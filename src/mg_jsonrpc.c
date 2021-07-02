#include <errno.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>

#include "mg_jsonrpc.h"
#include "mongoose.h"

struct mg_jsonrpc {
    int ref_count;
    struct mg_mgr mgr;

    char *ws_url;
    bool running;
    struct {
        pthread_t thread;
        pthread_cond_t cond;
        pthread_mutex_t lock;
    } mg_thread;
};

static const char *ev2str(int ev) {
    if (ev == MG_EV_ERROR)
        return "mg_ev_error";
    if (ev == MG_EV_POLL)
        return "mg_ev_poll";
    if (ev == MG_EV_RESOLVE)
        return "mg_ev_resolve";
    if (ev == MG_EV_CONNECT)
        return "mg_ev_connect";
    if (ev == MG_EV_ACCEPT)
        return "mg_ev_accept";
    if (ev == MG_EV_READ)
        return "mg_ev_read";
    if (ev == MG_EV_WRITE)
        return "mg_ev_write";
    if (ev == MG_EV_CLOSE)
        return "mg_ev_close";
    if (ev == MG_EV_HTTP_MSG)
        return "mg_ev_http_msg";
    if (ev == MG_EV_HTTP_CHUNK)
        return "mg_ev_http_chunk";
    if (ev == MG_EV_WS_OPEN)
        return "mg_ev_ws_open";
    if (ev == MG_EV_WS_MSG)
        return "mg_ev_ws_msg";
    if (ev == MG_EV_WS_CTL)
        return "mg_ev_ws_ctl";
    if (ev == MG_EV_MQTT_CMD)
        return "mg_ev_mqtt_cmd";
    if (ev == MG_EV_MQTT_MSG)
        return "mg_ev_mqtt_msg";
    if (ev == MG_EV_MQTT_OPEN)
        return "mg_ev_mqtt_open";
    if (ev == MG_EV_SNTP_TIME)
        return "mg_ev_sntp_time";
    if (ev == MG_EV_USER)
        return "mg_ev_user";

    return "mg_ev_unkown";
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    mg_jsonrpc_t *mgj = (mg_jsonrpc_t *)fn_data;

    LOG(LL_VERBOSE_DEBUG, ("receive ev: %s", ev2str(ev)));
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/ws/jsonrpc")) {
            // Upgrade to websocket. From now on, a connection is a full-duplex
            // Websocket connection, which will receive MG_EV_WS_MSG events.
            mg_ws_upgrade(c, hm, NULL);
        } else {
        }
    } else if (ev == MG_EV_WS_MSG) {
        // Got websocket frame. Received data is wm->data. Echo it back!
        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
        mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
        mg_iobuf_delete(&c->recv, c->recv.len);
    }
}

mg_jsonrpc_t *mg_jsonrpc_new(const char *url) {
    if (url == NULL) return NULL;

    mg_jsonrpc_t *mgj = calloc(1, sizeof(mg_jsonrpc_t));
    if (mgj == NULL) {
        LOG(LL_ERROR, ("calloc %zu fail", sizeof(mg_jsonrpc_t)));
        return NULL;
    }

    mgj->ws_url = strdup(url);
    if (mgj->ws_url == NULL) {
        LOG(LL_ERROR, ("strdup %s fail", url));
        free(mgj);
        mgj = NULL;
        return NULL;
    }

    pthread_cond_init(&mgj->mg_thread.cond, NULL);
    pthread_mutex_init(&mgj->mg_thread.lock, NULL);
    mgj->ref_count = 1;

    return mgj;
}

static void mg_jsonrpc_freep(mg_jsonrpc_t **mgjp) {
    if (mgjp && *mgjp) {
        mg_jsonrpc_t *mgj = *mgjp;
        *mgjp = NULL;

        mg_jsonrpc_stop(mgj);

        pthread_mutex_destroy(&mgj->mg_thread.lock);
        pthread_cond_destroy(&mgj->mg_thread.cond);
        if (mgj->ws_url)
            free(mgj->ws_url);
        free(mgj);
    }
}

mg_jsonrpc_t *mg_jsonrpc_ref(mg_jsonrpc_t *mgj) {
    if (mgj == NULL) return NULL;
    if (mgj->ref_count <= 0) return NULL;

    ++mgj->ref_count;
    return mgj;
}

void mg_jsonrpc_unref(mg_jsonrpc_t *mgj) {
    if (mgj == NULL) return;
    if (mgj->ref_count <= 0) return;

    if (--mgj->ref_count == 0)
        mg_jsonrpc_freep(&mgj);
}

static void *mg_thread(void *udata) {
    mg_jsonrpc_t *mgj = (mg_jsonrpc_t *)udata;

    mg_log_set("3");
    mg_mgr_init(&mgj->mgr);
    mg_http_listen(&mgj->mgr, mgj->ws_url, fn, mgj);
    pthread_cond_broadcast(&mgj->mg_thread.cond);

    while (mgj->running)
        mg_mgr_poll(&mgj->mgr, 500);

    LOG(LL_INFO, ("mg thread exiting"));
    mg_mgr_free(&mgj->mgr);
    return NULL;
}

int mg_jsonrpc_start(mg_jsonrpc_t *mgj, bool sync) {
    if (mgj == NULL) return -EINVAL;
    if (mgj->running) return -EALREADY;

    mgj->running = true;
    int ret = pthread_create(&mgj->mg_thread.thread, NULL, mg_thread, mgj);
    if (ret != 0) {
        LOG(LL_ERROR, ("pthread_create fail:%s", strerror(ret)));
        mgj->running = false;
        return ret;
    }

    if (sync)
        pthread_cond_wait(&mgj->mg_thread.cond, &mgj->mg_thread.lock);
    return 0;
}

int mg_jsonrpc_stop(mg_jsonrpc_t *mgj) {
    if (mgj == NULL) return -EINVAL;
    if (!mgj->running) return -EALREADY;

    mgj->running = false;
    pthread_join(mgj->mg_thread.thread, NULL);
    return 0;
}
