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

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/ws/jsonrpc")) {
            // Upgrade to websocket. From now on, a connection is a full-duplex
            // Websocket connection, which will receive MG_EV_WS_MSG events.
            mg_ws_upgrade(c, hm, NULL);
        }
    } else if (ev == MG_EV_WS_MSG) {
        // Got websocket frame. Received data is wm->data. Echo it back!
        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
        mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
        mg_iobuf_delete(&c->recv, c->recv.len);
    }
    (void) fn_data;
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

    LOG(LL_INFO, ("mg thread exiting\n"));
    mg_mgr_free(&mgj->mgr);
    return NULL;
}

int mg_jsonrpc_start(mg_jsonrpc_t *mgj, bool sync) {
    if (mgj == NULL) return -EINVAL;
    if (mgj->running) return -EALREADY;

    mgj->running = true;
    int ret = pthread_create(&mgj->mg_thread.thread, NULL, mg_thread, mgj);
    if (ret != 0) {
        LOG(LL_ERROR, ("pthread_create fail:%s\n", strerror(ret)));
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
