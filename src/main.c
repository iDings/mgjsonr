#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#include "mg_jsonrpc.h"
#include "mg_jsonrpc_methods.h"

static bool running = true;

static void sigcatch(int signum) {
	static int force_eixt = 0;
	fprintf(stderr, "quiting: by signal %d\n", signum);
	running = false;

	if (++force_eixt > 5) {
		fprintf(stderr, "force exiting\n");
		exit(127);
	}
}

int main(int argc, char **argv) {
	signal(SIGINT, sigcatch);
	signal(SIGTERM, sigcatch);

    const char *url = "ws://0.0.0.0:9898";
    if (argc >= 2)
        url = argv[1];

    mg_jsonrpc_t *mgj = mg_jsonrpc_new(url);
    assert(mgj != NULL);

#ifndef MG_JSONRPC_METHOD_CONSTRUCTOR
    mg_jsonrpc_methods_init();
#endif
    mg_jsonrpc_init(mgj, mg_jsonrpc_methods());
    mg_jsonrpc_start(mgj, true);

    while (running) {
        sleep(1);
    }

    mg_jsonrpc_unref(mgj);
    return 0;
}
