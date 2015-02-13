#include "client.h"
#include <stdio.h>
#ifndef _WIN32
#include <pthread.h>
#endif // !_WIN32

#ifdef _WIN32
typedef DWORD THREAD_ID;
static DWORD WINAPI recv_process(void* sockfd);
#else
typedef pthread_t THREAD_ID;
void recv_process(void* sockfd);
#endif // _WIN32


int cli_start(const char* address, int port) {
	printf("client starting...\n");
	if (sock_init()) {
		SOCKET s = sock_connect(address, port);
		if (s > 0) {
			THREAD_ID thread_id;
			char send_buff[1024] = {0};
#ifdef _WIN32
			CreateThread(NULL, 0, recv_process, (void*)s, 0, &thread_id);
#else
			pthread_create(&thread_id, NULL, (void *)recv_process, NULL);
#endif
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
#ifdef _WIN32
static DWORD WINAPI recv_process(void* sockfd)
#else
void recv_process(void* sockfd)
#endif
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