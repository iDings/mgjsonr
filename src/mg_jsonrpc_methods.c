#include <stdbool.h>
#include <string.h>

#include "mg_jsonrpc_methods.h"
#include "mg_log.h"

struct jsonrpc_method *g_mgj_methods = NULL;

// TODO: empty array maybe some more check
static bool rpc_list_filter(const char *method, const char *params, int params_len) {
    if (!params || !params_len || !strncmp(params, "[]", 2))
        return true;

    int koff, klen, voff, vlen, vtype, off = 0;
    bool match = false;
    do {
        off = mjson_next(params, params_len, off, &koff, &klen, &voff, &vlen, &vtype);
        if (off == 0) {
            break;
        }

        LOGD("method: %s\n", method);
        LOGD("key: %.*s, value: %.*s vtype:%d\n", klen, params + koff, vlen, params + voff, vtype);
        if (vtype != MJSON_TOK_STRING)
            continue;

        const char *p = params + voff;
        // if `klen` holds `0`, we're iterating over an array, otherwise over an object.
        if (!klen && (mjson_globmatch(p + 1, vlen - 2 ,method, strlen(method)) > 0)) {
            match = true;
            break;
        }
    } while (1);

    return match;
}

static int jsonrpc_print_methods(mjson_print_fn_t fn, void *fndata, va_list *ap) {
    struct jsonrpc_ctx *ctx = va_arg(*ap, struct jsonrpc_ctx *);
    const char *params = va_arg(*ap, const char *);
    int params_len = va_arg(*ap, int);

    struct jsonrpc_method *m;
    int len = 0;
    LOGD("params:%.*s params_len:%d\n", params_len, params, params_len);
    for (m = ctx->methods; m != NULL; m = m->next) {
        bool match = rpc_list_filter(m->method, params, params_len);
        if (match) {
            if (m != ctx->methods) len += mjson_print_buf(fn, fndata, ",", 1);
            len += mjson_print_str(fn, fndata, m->method, (int) strlen(m->method));
        }
    }
    return len;
}

static void method_rpc_list(struct jsonrpc_request *r) {
    jsonrpc_return_success(r, "[%M]", jsonrpc_print_methods, r->ctx, r->params, r->params_len);
}

MG_JSONRPC_METHOD_DEF(rpc_list, "rpc.list");
static struct jsonrpc_method *methods[] = {
    &rpc_list,
    NULL
};
MG_JSONRPC_METHODS_INIT(rpc, methods);

struct jsonrpc_method *mg_jsonrpc_methods(void) {
    return g_mgj_methods;
}

void mg_jsonrpc_methods_register(struct jsonrpc_method *methods[]) {
    for (int i = 0; methods[i]; i++) {
        methods[i]->next = g_mgj_methods;
        g_mgj_methods = methods[i];
    }
}
