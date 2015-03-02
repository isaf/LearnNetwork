#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#if defined(USE_KQUEUE)

#include "socket.h"
#include <stdlib.h>
#include <sys/event.h>

typedef struct kevent kevent;
#define MAX_EVENS 1
static int g_pollfd;
kevent* g_pevents = NULL;

void poll_start(SOCKET lsfd) {
	g_pollfd = kqueue();
	kevent ev;
	ev.ident = lsfd;
	ev.filter = EVFILT_READ;
	ev.flags = EV_ADD;
	ev.fflags = 0;
	ev.data = 0;
	ev.udata = NULL;
	// you can use EV_SET(&ev, lsfd, EVFILT_READ, EV_ADD, 0, 0, NULL) to replace it
	kevent(g_pollfd, &ev, 1, NULL, 0, NULL);
	g_pevents = (kevent*)calloc(MAX_EVENS, sizeof(kevent));	//calloc: alloc and initialize
}

int poll_wait(SOCKET lsfd) {
	CLIENT_INFO* info = NULL;
	sockaddr_in* cs_info = NULL;
	SOCKET cs = 0;
	int len = sizeof(sockaddr_in);
	int n = kevent(g_pollfd, NULL, 0, g_pevents, MAX_EVENS, 10);
	int ret;
	int i;
	for (i = 0; i < n; ++i) {
		int fd = g_pevents[i].ident;
		if (fd == lsfd) {
			cs_info = (sockaddr_in*)malloc(sizeof(sockaddr_in));
			cs = accept(lsfd, (struct sockaddr *)cs_info, &len);

			if (cs != INVALID_SOCKET) {
				kevent ev;
				info = sock_add_client_info(cs, cs_info);
				if (cs > g_max_fd)
					g_max_fd = cs;
				if (info)
					sock_push_msg(SOCK_TYPE_CONNECT, cs, 0);

				EV_SET(&ev, cs, EVFILT_READ, EV_ADD, 0, 0, NULL);
				kevent(g_pollfd, &ev, 1, NULL, 0, NULL);
			}

		}
		else if (g_pevents[i].filter == EVFILT_READ) {
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

#endif
#endif