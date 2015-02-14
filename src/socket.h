#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
typedef unsigned int SOCKET;
typedef struct sockaddr SOCKADDR;
#define SOCKET_ERROR -1
#define INVALID_SOCKET (SOCKET)(~0)
#endif //_WIN32

#define MAX_BUFFER_SIZE 1024

#define SOCK_TYPE_CONNECT 0
#define SOCK_TYPE_DATA 1
#define SOCK_TYPE_CLOSE 2

typedef struct SOCK_MSG {
	int type;
	char buff[MAX_BUFFER_SIZE];
	struct SOCK_MSG* pre;
	struct SOCK_MSG* next;
}SOCK_MSG;

typedef struct sockaddr_in sockaddr_in;

typedef struct CLIENT_INFO {
	int id;
	short close;
	SOCKET sock;
	SOCK_MSG* hmsg;	// point to the first msg
	SOCK_MSG* lmsg;	// point to the last msg
	struct sockaddr_in* sockinfo;
	struct CLIENT_INFO* pre;
	struct CLIENT_INFO* next;
}CLIENT_INFO;

int sock_init();
SOCKET sock_listen(int port, int backlog);
SOCKET sock_connect(const char* address, int port);
int sock_close(SOCKET sfd);
int sock_uninit();
int sock_wait(SOCKET lsfd, fd_set* pset);
SOCK_MSG* sock_read(SOCKET fd);
int sock_write(SOCKET fd, const char* data);
CLIENT_INFO* sock_get_fd_info(SOCKET sfd);
CLIENT_INFO* sock_get_fd_queue();
int sock_init_fd_queue();

#endif //__SOCKET_H__