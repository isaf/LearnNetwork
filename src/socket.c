#include "socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "poll.h"

unsigned int g_max_fd = 0;
int g_clinet_counter = 0;
CLIENT_INFO* c_head = NULL;
CLIENT_INFO* c_tail = NULL;	//point to tail node

static CLIENT_INFO* remove_sock_info(SOCKET fd);

//initialize the socket
int sock_init() {
#ifdef _WIN32
	struct WSAData wsaData;
	WSAStartup(0x0101, &wsaData);
#endif // _WIN32
	return 1;
}

SOCKET sock_listen(int port, int backlog) {
	SOCKET s;
	sockaddr_in local_address;
	int len;
	int ret;

	backlog = backlog ? backlog : 5;
	len = sizeof(sockaddr_in);
	memset(&local_address, 0, len);
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
	local_address.sin_port = htons(port);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s > g_max_fd)
		g_max_fd = s;
	if (s == INVALID_SOCKET) {
		printf("socket error: create socket failed.\n");
		return 0;
	}

	ret = bind(s, (SOCKADDR*)&local_address, sizeof(SOCKADDR));
	if(ret != 0) {
		printf("socket error: bind failed.\n");
		return 0;
	}
	if (listen(s, backlog) == SOCKET_ERROR) {
		printf("socket error: listen failed\n");
		return 0;
	}
	poll_start(s);
	return s;
}

SOCKET sock_connect(const char* szAddress, int nPort) {
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in remote_address;
	int len = sizeof(sockaddr_in);
	int ret;

	memset(&remote_address, 0, sizeof(remote_address));
	remote_address.sin_family = AF_INET;
	remote_address.sin_addr.s_addr = inet_addr(szAddress);
	remote_address.sin_port = htons(nPort);

	ret = connect(s, (SOCKADDR*)&remote_address, sizeof(SOCKADDR));
	if (!ret)
		return s;
	sock_close(s);
	return 0;
}

int sock_close(SOCKET sfd) {
	CLIENT_INFO* info = sock_get_fd_info(sfd);
	info->close = 1;
#ifdef _WIN32
	return closesocket(sfd);
#else
	return close(sfd);
#endif // _WIN32

}

int sock_wait(SOCKET lsfd) {
	sock_reset_client_info();
	return poll_wait(lsfd);
}

SOCK_MSG* sock_read(SOCKET fd) {
	CLIENT_INFO* info = sock_get_fd_info(fd);
	if (info) {
		SOCK_MSG* msg = info->hmsg;
		if (msg) {
			info->hmsg = info->hmsg->next;	// drop the first msg
			return msg;
		}
	}
	return NULL;
}

int sock_write(SOCKET fd, const char* data) {
	send(fd, data, MAX_BUFFER_SIZE, 0);
	return 0;
}



int sock_uninit() {
	poll_close();
#ifdef _WIN32
	WSACleanup();
#endif // _WIN32
	return 1;
}

int sock_init_client_queue() {
	if (!c_tail) {	//initialize
		CLIENT_INFO* cur = NULL;
		c_head = (CLIENT_INFO*)malloc(sizeof(CLIENT_INFO));
		memset(c_head, 0, sizeof(CLIENT_INFO));
		c_tail = c_head;
		cur = c_head->next;
		return 1;
	}
	return 0;
}


CLIENT_INFO* sock_get_fd_info(SOCKET sfd) {
	CLIENT_INFO* cur = c_head->next;
	while (cur) {
		if (cur->sock == sfd)
			return cur;
		else
			cur = cur->next;
	}
	return NULL;
}

CLIENT_INFO* sock_get_client_queue() {
	return c_head->next;
}

int sock_push_msg(int msgtype, SOCKET fd, const char* data) {
	CLIENT_INFO* info = sock_get_fd_info(fd);
	if (info && !info->close) {
		SOCK_MSG* msg = (SOCK_MSG*)malloc(sizeof(SOCK_MSG));
		msg->type = msgtype;
		if (msgtype == SOCK_TYPE_DATA)
			memcpy(msg->buff, data, MAX_BUFFER_SIZE);
		msg->next = NULL;
		msg->pre = info->lmsg;

		if (!info->hmsg)
			info->hmsg = msg;

		info->lmsg = msg;

		return 1;
	}
	return 0;
}

CLIENT_INFO* sock_add_client_info(SOCKET fd, sockaddr_in* cs_info) {
	CLIENT_INFO* info = (CLIENT_INFO*)malloc(sizeof(CLIENT_INFO));

	info->sock = fd;
	info->id = ++g_clinet_counter;
	info->sockinfo = cs_info;
	info->next = NULL;
	info->pre = c_tail;
	info->close = 0;
	info->hmsg = NULL;
	info->lmsg = NULL;

	c_tail->next = info;
	c_tail = info;

	return info;
}

void sock_reset_client_info() {
	CLIENT_INFO* cur = c_head->next;
	while (cur) {
		if (cur->close)
			cur = remove_sock_info(cur->sock);
		else
			cur = cur->next;
	}
}

static CLIENT_INFO* remove_sock_info(SOCKET fd) {
	CLIENT_INFO* cur = c_head->next;
	CLIENT_INFO* next = NULL;
	while (cur) {
		if (cur->sock == fd) {
			if (cur == c_tail)
				c_tail = cur->pre;
			else
				cur->next->pre = cur->pre;

			next = cur->next;
			cur->pre->next = next;
			free(cur);
			return next;
		}
		else
			cur = cur->next;
	}
	return NULL;
}
