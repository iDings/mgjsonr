#pragma once

#include <stdio.h>
#include "mjson.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MG_JSONRPC_METHOD_DEF(name, method) \
    struct jsonrpc_method name = {method, sizeof(method) - 1, method_##name, 0}

#define MG_JSONRPC_METHODS_INIT(name, methods_tbl) \
    __attribute__((constructor)) static void methods_##name(void) { \
        mg_jsonrpc_methods_register(methods_tbl); \
    }

void mg_jsonrpc_methods_register(struct jsonrpc_method *methods[]);
struct jsonrpc_method *mg_jsonrpc_methods(void);

#ifdef __cplusplus
}
#endif
