#if defined(linux) && defined(USE_EPOLL)

#include "socket.h"
#include <stdlib.h>
#include <sys/epoll.h>

typedef struct epoll_event epoll_event;
#define MAX_EVENS 1	//is useless after linux kernel 2.6.8
static int g_pollfd;
epoll_event* g_pevents = NULL;

void poll_start(SOCKET lsfd) {
	g_pollfd = epoll_create(MAX_EVENS);
	epoll_event event;
	event.data.fd = lsfd;
	event.events = EPOLLIN;
	epoll_ctl(g_pollfd, EPOLL_CTL_ADD, lsfd, &event);
	g_pevents = (epoll_event*)calloc(MAX_EVENS, sizeof(epoll_event));	//calloc: alloc and initialize
}

int poll_wait(SOCKET lsfd) {
	CLIENT_INFO* info = NULL;
	sockaddr_in* cs_info = NULL;
	SOCKET cs = 0;
	int len = sizeof(sockaddr_in);
	int n = epoll_wait(g_pollfd, g_pevents, MAX_EVENS, 10);
	int ret;
	int i;

	for (i = 0; i < n; ++i) {
		int fd = g_pevents[i].data.fd;
		if (fd == lsfd) {
			cs_info = (sockaddr_in*)malloc(sizeof(sockaddr_in));
			cs = accept(lsfd, (struct sockaddr *)cs_info, &len);

			if (cs != INVALID_SOCKET) {
				epoll_event event;
				info = sock_add_client_info(cs, cs_info);
				if (cs > g_max_fd)
					g_max_fd = cs;
				if (info)
					sock_push_msg(SOCK_TYPE_CONNECT, cs, 0);

				event.data.fd = cs;
				event.events = EPOLLIN;
				epoll_ctl(g_pollfd, EPOLL_CTL_ADD, cs, &event);
			}

		}
		else if (g_pevents[i].events & EPOLLIN) {
			char recv_buff[MAX_BUFFER_SIZE] = {0};
			ret = recv(fd, recv_buff, sizeof(recv_buff), 0);
			if (ret > 0)
				sock_push_msg(SOCK_TYPE_DATA, fd, recv_buff);
			else if (ret <= 0)
				sock_push_msg(SOCK_TYPE_CLOSE, fd, 0);
		}
	}
	return 1;
}

void poll_close() {
	close(g_pollfd);
	free(g_pevents);
}

#endif // linux
