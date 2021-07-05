#pragma once

#include <stdio.h>
#include "mjson.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MG_JSONRPC_METHOD_CONSTRUCTOR

#define MG_JSONRPC_METHOD_DEF(name, method) \
    struct jsonrpc_method name = {method, sizeof(method) - 1, method_##name, 0}

#ifdef MG_JSONRPC_METHOD_CONSTRUCTOR
#define MG_JSONRPC_METHODS_INIT(name, methods_tbl) \
    __attribute__((constructor)) static void methods_##name(void) { \
        mg_jsonrpc_methods_register(methods_tbl); \
    }
#else
#define MG_JSONRPC_METHODS_INIT(name, methods_tbl)
#endif

void mg_jsonrpc_methods_register(struct jsonrpc_method *methods[]);
struct jsonrpc_method *mg_jsonrpc_methods(void);

// NOTE: http://alturl.com/f8vv9
// gcc construct can't work on static library
// need explicit call init
void mg_jsonrpc_methods_init(void);

#ifdef __cplusplus
}
#endif
