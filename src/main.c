#include <stdio.h>
#include "client.h"
#include "server.h"

int main(int Argc, char ** Argv) {
	if (Argc == 2) {
		if (strcmp(Argv[1], "c") == 0)
			//cli_start("127.0.0.1", 8888);
			cli_start("10.20.100.23", 8888);
		else if (strcmp(Argv[1], "s") == 0)
			svr_start(8888);
	}
	else
		printf("error argument!must be \"c\" or \"s\".\n");
	return 1;
}