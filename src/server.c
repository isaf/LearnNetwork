#include "server.h"
#include <stdio.h>

static void broadcast_message(SOCKET sfd, const char* msg);
static void message_loop(SOCKET lsfd);
//==============================================================
int svr_start(int port) {
	int ret = 0;
	printf("server starting...\n");
	if (sock_init()) {
		SOCKET lsfd = sock_listen(port, 0);
		sock_init_fd_queue();
		if (lsfd > 0) {
			ret = 1;
			printf("server start ok!\n");
			message_loop(lsfd);
			sock_close(lsfd);
		}
	}
	printf("server exit!\n");
	sock_uninit();
	return ret;
}

static void message_loop(SOCKET lsfd) {
	fd_set rfds;
	char send_buff[MAX_BUFFER_SIZE] = {0};
	while (1) {
		if (sock_wait(lsfd, &rfds)) {
			CLIENT_INFO* info = sock_get_fd_queue();
			while (info) {
				SOCKET fd = info->sock;
				while (1) {
					SOCK_MSG* msg = sock_read(fd);
					if (msg) {
						switch (msg->type)
						{
						case SOCK_TYPE_CONNECT: 
							printf("%s login.\n", inet_ntoa(info->sockinfo->sin_addr));
							sprintf(send_buff, "login success, your id: %d.\n", info->id);
							sock_write(fd, send_buff);
							break;
						case SOCK_TYPE_DATA:
							printf("[%s:%d]say: %s.\n", inet_ntoa(info->sockinfo->sin_addr), info->id, msg->buff);
							broadcast_message(fd, (const char*)msg->buff);
							break;
						case SOCK_TYPE_CLOSE:
							printf("[%s:%d]disconnect!\n", inet_ntoa(info->sockinfo->sin_addr), info->id);
							sock_close(fd);
							break;
						default:
							break;
						}
						free(msg);
					}
					else
						break;
				}
				info = info->next;
			}
		}
	}
}

static void broadcast_message(SOCKET sfd, const char* msg) {

}