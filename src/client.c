#include "client.h"
#include "thread.h"
#ifndef _WIN32
#include <pthread.h>
#endif // !_WIN32

static CALL_BACK recv_process(void* sockfd);

int cli_start(const char* address, int port) {
	printf("client starting...\n");
	if (sock_init()) {
		SOCKET s = sock_connect(address, port);

		if (s > 0) {
			char send_buff[MAX_BUFFER_SIZE] = {0};

			thread_create(recv_process, (void*)s);
			while (TRUE) {
				printf("input sth:\n");
				scanf("%s", send_buff);
				send(s, send_buff, sizeof(send_buff), 0);
			}
			return 1;
		}
	}
	sock_uninit();
	return 0;
}

static CALL_BACK recv_process(void* data)
{
	SOCKET s = (SOCKET)data;
	int ret = 0;
	char buff[MAX_BUFFER_SIZE] = {0};

	while (TRUE) {
		ret = recv(s, buff, sizeof(buff), 0);
		if (ret > 0)
			printf("recv %s", buff);
	}
#ifdef _WIN32
	return 1;
#endif
}