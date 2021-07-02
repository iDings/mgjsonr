#pragma once

#include "mjson.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO:
#define MG_JSONRPC_METHOD_DEF(name, method, method_fn) \
    struct jsonrpc_method mgj_##name = {method, sizeof(method) - 1, method_fn, 0}

// end with NULL
extern struct jsonrpc_method *g_mgj_methods[];

#ifdef __cplusplus
}
#endif
