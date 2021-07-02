#pragma once

#include <stdbool.h>
#include "mjson.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mg_jsonrpc;
typedef struct mg_jsonrpc mg_jsonrpc_t;

mg_jsonrpc_t *mg_jsonrpc_new(const char *url);
mg_jsonrpc_t *mg_jsonrpc_ref(mg_jsonrpc_t *mgj);
void mg_jsonrpc_unref(mg_jsonrpc_t *mgj);

int mg_jsonrpc_init(mg_jsonrpc_t *mgj, struct jsonrpc_method *methods[]);
int mg_jsonrpc_deinit(mg_jsonrpc_t *mgj);

int mg_jsonrpc_start(mg_jsonrpc_t *mgj, bool sync);
int mg_jsonrpc_stop(mg_jsonrpc_t *mgj);

#ifdef __cplusplus
}
#endif
