#include <pthread.h>

#include "mongoose.h"
#include "mg_jsonrpc.h"

#include "mg_log.h"
#include "mg_jsonrpc_methods.h"
#include "mg_jsonrpc_instance.h"

static pthread_mutex_t g_mgj_lock = PTHREAD_MUTEX_INITIALIZER;
static mg_jsonrpc_t *g_mgj;

mg_jsonrpc_t *mg_jsonrpc(void) {
    pthread_mutex_lock(&g_mgj_lock);
    do {
        if (g_mgj) break;

        mg_jsonrpc_t *mgj = mg_jsonrpc_new(MG_JSONRPC_INSTANCE_URL);
        if (mgj == NULL) {
            LOGD("mg_jsonrpc_new fail");
            break;
        }

        mg_jsonrpc_init(mgj, mg_jsonrpc_methods());
        g_mgj = mgj;
    } while (0);
    pthread_mutex_unlock(&g_mgj_lock);

    return g_mgj;
}
