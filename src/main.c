#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#include "mg_jsonrpc.h"

static const char *s_listen_on = "http://localhost:8000";
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

    mg_jsonrpc_t *mgj = mg_jsonrpc_new(s_listen_on);
    mg_jsonrpc_start(mgj, true);

    while (running) {
        sleep(1);
    }

    mg_jsonrpc_unref(mgj);
    mgj = NULL;
    return 0;
}
