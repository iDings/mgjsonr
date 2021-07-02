#include "mongoose.h"
#include "mg_jsonrpc_methods.h"

static void method_cb(struct jsonrpc_request *req) {
    jsonrpc_return_success(req, "%Q", "mongoose jsonrpc v0.01");
    return;
}

MG_JSONRPC_METHOD_DEF(hello, "hello", method_cb);
