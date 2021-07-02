#pragma once

#include "mg_jsonrpc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MG_JSONRPC_INSTANCE_URL "http://localhost:8000"

mg_jsonrpc_t *mg_jsonrpc(void);

#ifdef __cplusplus
}
#endif
