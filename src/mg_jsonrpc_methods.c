#include "mg_jsonrpc_methods.h"

extern struct jsonrpc_method mgj_hello;

struct jsonrpc_method *g_mgj_methods[] = {
    &mgj_hello,
    NULL,
};
