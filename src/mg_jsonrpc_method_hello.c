#include "mongoose.h"
#include "mg_jsonrpc_methods.h"

static void method_cb(struct jsonrpc_request *req) {
    jsonrpc_return_success(req, "%Q", "mongoose jsonrpc v0.01");
    return;
}

#define METHOD "hello"
struct jsonrpc_method mgj_hello = {
    .method = METHOD,
    .method_sz = sizeof(METHOD) - 1,
    .cb = method_cb,
};
