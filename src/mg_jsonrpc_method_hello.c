#include "mongoose.h"
#include "mg_jsonrpc_methods.h"

static void method_hello(struct jsonrpc_request *req) {
    jsonrpc_return_success(req, "%Q", "mongoose jsonrpc v0.01");
    return;
}

MG_JSONRPC_METHOD_DEF(hello, "hello");

static struct jsonrpc_method *methods[] = {
    &hello,
    NULL
};
MG_JSONRPC_METHODS_INIT(hello, methods);
