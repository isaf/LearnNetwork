#if !defined(USE_IOCP) && !defined(USE_EPOLL) && !defined(USE_KQUEUE)
#include "socket.h"
#include <stdlib.h>
#ifndef _WIN32
#include <sys/select.h>
#endif

static fd_set g_fdset;

static void init_fd_set(SOCKET lsfd, fd_set* pset);

void poll_start(SOCKET lsfd) {

}

int poll_wait(SOCKET lsfd) {
	sockaddr_in* cs_info = NULL;
	SOCKET cs = 0;
	CLIENT_INFO* info = NULL;
	fd_set* pset = &g_fdset;

	int ret;
	struct timeval tv = {2, 0};
	int len;
	char recv_buff[MAX_BUFFER_SIZE] = {0};

	len = sizeof(sockaddr_in);
	init_fd_set(lsfd, pset);
	ret = select(g_max_fd + 1, pset, NULL, NULL, &tv);	//the first arg must be max fd plus 1 in linux.
	if (ret > 0) {
		if (FD_ISSET(lsfd, pset)) {
			FD_CLR(lsfd, pset);
			cs_info = (sockaddr_in*)malloc(sizeof(sockaddr_in));
			cs = accept(lsfd, (struct sockaddr *)cs_info, &len);
			if (cs != INVALID_SOCKET) {
				info = sock_add_client_info(cs, cs_info);
				if (cs > g_max_fd)
					g_max_fd = cs;
				if (info)
					sock_push_msg(SOCK_TYPE_CONNECT, cs, 0);
			}
		}

		info = sock_get_client_queue();
		while (info) {
			SOCKET fd = info->sock;
			if (FD_ISSET(fd, pset)) {
				ret = recv(fd, recv_buff, sizeof(recv_buff), 0);
				if (ret > 0)
					sock_push_msg(SOCK_TYPE_DATA, fd, recv_buff);
				else if (ret <= 0)
					sock_push_msg(SOCK_TYPE_CLOSE, fd, 0);
			}
			info = info->next;
		}
		return 1;
	}
	return ret;
}

void poll_close() {

}

static void init_fd_set(SOCKET lsfd, fd_set* pset) {
	CLIENT_INFO* cur = c_head->next;

	FD_ZERO(pset);
	FD_SET(lsfd, pset);
	while (cur) {
		FD_SET(cur->sock, pset);
		cur = cur->next;
	}
}
#endif // !USE_IOCP
