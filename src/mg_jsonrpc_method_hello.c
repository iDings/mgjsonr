#include "mongoose.h"
#include "mg_jsonrpc_methods.h"
#include "mg_jsonrpc.h"

static void method_hello(struct jsonrpc_request *req) {
    jsonrpc_return_success(req, "%Q", "mongoose jsonrpc v0.01");
    mg_jsonrpc_t *mgj = req->userdata;
    mg_jsonrpc_send_notification(mgj, "hello.noti", "{\"version\":\"v0.01\"}");
    return;
}

MG_JSONRPC_METHOD_DEF(hello, "hello");

static struct jsonrpc_method *methods[] = {
    &hello,
    NULL
};
MG_JSONRPC_METHODS_INIT(hello, methods);
