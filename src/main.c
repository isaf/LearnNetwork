#include <stdio.h>
#include "client.h"
#include "server.h"

unsigned int g_server_port = 8888;

int main(int Argc, char ** Argv) {
	if (Argc >= 2) {
		if (strcmp(Argv[1], "c") == 0) {
			const char* address = Argc >= 3 ? Argv[2] : "127.0.0.1";
			if (!cli_start(address, g_server_port))
				printf("client start fail!\n");
		}
		else if (strcmp(Argv[1], "s") == 0)
			svr_start(g_server_port);
		else
			printf("error argument!must be \"c\" or \"s\".\n");
	}
	else
		svr_start(g_server_port);
	return 1;
}