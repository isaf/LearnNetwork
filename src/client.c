#include "client.h"
#include <stdio.h>

static DWORD WINAPI recv_process(void* sockfd);

int cli_start(const char* address, int port) {
	printf("client starting...\n");
	if (sock_init()) {
		SOCKET s = sock_connect(address, port);
		if (s > 0) {
			DWORD thread_id;
			char send_buff[1024] = {0};
			CreateThread(NULL, 0, recv_process, (void*)s, 0, &thread_id);
			while (1) {
				printf("input sth:\n");
				scanf("%s", send_buff);
				send(s, send_buff, sizeof(send_buff), 0);
			}
		}
		return 1;
	}
	sock_uninit();
	return 0;
}

static DWORD WINAPI recv_process(void* sockfd)
{
	SOCKET s = (SOCKET)sockfd;
	int ret = 0;
	char buff[1024];
	while (1) {
		ret = recv(s, buff, sizeof(buff), 0);
		if (ret != SOCKET_ERROR)
			printf("recv %s", buff);
	}
	return 1;
}