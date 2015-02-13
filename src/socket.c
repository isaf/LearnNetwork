#include "socket.h"
#include <stdio.h>

static int g_clinet_counter = 0;
static CLIENT_INFO* c_head = NULL;
static CLIENT_INFO* c_tail = NULL;	//point to tail node

static void init_fd_set(SOCKET lsfd, fd_set* pset);
static int push_socket_msg(int msgtype, SOCKET fd, const char* data);
static CLIENT_INFO* add_sock_info(SOCKET fd, sockaddr_in* cs_info);
static CLIENT_INFO* remove_sock_info(SOCKET fd);
static int reset_fd_queue();

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
	local_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
	local_address.sin_port = htons(port);

	s = socket(AF_INET, SOCK_STREAM, 0);
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
	return s;
}

SOCKET sock_connect(const char* szAddress, int nPort) {
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in remote_address;
	int len = sizeof(sockaddr_in);
	int ret;

	memset(&remote_address, 0, sizeof(remote_address));
	remote_address.sin_family = AF_INET;
	remote_address.sin_addr.S_un.S_addr = inet_addr(szAddress);
	remote_address.sin_port = htons(nPort);

	ret = connect(s, (SOCKADDR*)&remote_address, sizeof(SOCKADDR));
	if (!ret)
		return s;
	closesocket(s);
	return 0;
}

int sock_close(SOCKET sfd) {
	return closesocket(sfd);
}

int sock_wait(SOCKET lsfd, fd_set* pset) {
	sockaddr_in* cs_info;
	SOCKET cs = 0;

	int ret;
	struct timeval tv = {1, 0};
	int len;
	char recv_buff[MAX_BUFFER_SIZE] = {0};
	unsigned int i;

	len = sizeof(sockaddr_in);
	reset_fd_queue();
	init_fd_set(lsfd, pset);
	ret = select(0, pset, NULL, NULL, &tv);
	if (ret > 0) {
		if (FD_ISSET(lsfd, pset)) {
			FD_CLR(lsfd, pset);
			cs_info = (sockaddr_in*)malloc(sizeof(sockaddr_in));
			cs = accept(lsfd, (struct sockaddr *)cs_info, &len);
			if (cs != INVALID_SOCKET) {
				CLIENT_INFO* info = add_sock_info(cs, cs_info);
				if (info)
					push_socket_msg(SOCK_TYPE_CONNECT, cs, 0);
			}
		}
		for (i = 0; i < pset->fd_count; ++i) {
			SOCKET fd = pset->fd_array[i];
			CLIENT_INFO* info = sock_get_fd_info(fd);
			ret = recv(fd, recv_buff, sizeof(recv_buff), 0);
			if (ret > 0) {
				if (info && !info->close)
					push_socket_msg(SOCK_TYPE_DATA, fd, recv_buff);
				}
			else if (ret <= 0) {
				info->close = 1;
				push_socket_msg(SOCK_TYPE_CLOSE, fd, 0);
			}
		}
		return 1;
	}

	return 0;
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

CLIENT_INFO* sock_get_fd_queue() {
	return c_head->next;
}

int sock_uninit() {
#ifdef _WIN32
	WSACleanup();
#endif // _WIN32
	return 1;
}

int sock_init_fd_queue() {
	if (!c_tail) {	//initialize
		c_head = (CLIENT_INFO*)malloc(sizeof(CLIENT_INFO));
		memset(c_head, 0, sizeof(CLIENT_INFO));
		c_tail = c_head;
		return 1;
	}
	return 0;
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

static int push_socket_msg(int msgtype, SOCKET fd, const char* data) {
	CLIENT_INFO* info = sock_get_fd_info(fd);
	if (info) {
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

static CLIENT_INFO* add_sock_info(SOCKET fd, sockaddr_in* cs_info) {
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

static int reset_fd_queue() {
	CLIENT_INFO* cur = c_head->next;
	while (cur) {
		if (cur->close)
			cur = remove_sock_info(cur->sock);
		else
			cur = cur->next;
	}
}