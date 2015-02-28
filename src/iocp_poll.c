#if defined(_WIN32) && defined(USE_IOCP)
#include "socket.h"
#include "thread.h"

HANDLE g_iocp_handle;
typedef struct {
	SOCKET s;
}COMP_KEY;

typedef struct {
	OVERLAPPED over_lap;
	WSABUF buffer;
	char data[MAX_BUFFER_SIZE];
	DWORD datalen;
}IO_DATA; 

static HANDLE create_iocp();
static void bind_iocp(SOCKET s, COMP_KEY* key);


void poll_start(SOCKET lsfd) {
	COMP_KEY* key = (COMP_KEY*)malloc(sizeof(COMP_KEY));
	g_iocp_handle = create_iocp();
	key->s = lsfd;
	bind_iocp(lsfd, key);
}
int poll_wait(SOCKET lsfd) {
	CLIENT_INFO* info = NULL;
	sockaddr_in* cs_info = NULL;
	SOCKET cs = 0;
	int len = sizeof(sockaddr_in);
	DWORD flags = 0;
	int ret;
	fd_set fdset;
	struct timeval tv = {0, 0};

	FD_ZERO(&fdset);
	FD_SET(lsfd, &fdset);
	ret = select(g_max_fd, &fdset, NULL, NULL, &tv);
	if (ret > 0) {
		COMP_KEY* key = (COMP_KEY*)malloc(sizeof(COMP_KEY));
		IO_DATA* io_data = (IO_DATA*)malloc(sizeof(IO_DATA));
		cs_info = (sockaddr_in*)malloc(sizeof(sockaddr_in));
		cs = accept(lsfd, (struct sockaddr *)cs_info, &len);

		if (cs != INVALID_SOCKET) {
			info = sock_add_client_info(cs, cs_info);
			if (cs > g_max_fd)
				g_max_fd = cs;
			if (info)
				sock_push_msg(SOCK_TYPE_CONNECT, cs, 0);
		}
		key->s = cs;
		bind_iocp(cs, key);

		ZeroMemory(&io_data->over_lap, sizeof(io_data->over_lap));
		ZeroMemory(io_data->data, MAX_BUFFER_SIZE);
		io_data->buffer.len = MAX_BUFFER_SIZE;
		io_data->buffer.buf = io_data->data;
		WSARecv(cs, &io_data->buffer, 1, &io_data->datalen, &flags, &io_data->over_lap, NULL);
	}
	return 1;
}
void poll_close() {
	CloseHandle(g_iocp_handle);
}

static DWORD WINAPI iocp_work_proc(void* data);

static HANDLE create_iocp() {
	SYSTEM_INFO sys_into;
	HANDLE iocp_handle;
	unsigned int i;
	unsigned int thread_cout;

	GetSystemInfo(&sys_into);
	thread_cout = 1/*sys_into.dwNumberOfProcessors * 2*/;
	iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, thread_cout);
	for (i = 0; i < thread_cout; ++i)
		thread_create(iocp_work_proc, (void*)iocp_handle);

	return iocp_handle;
}

static DWORD WINAPI iocp_work_proc(void* data) {
	HANDLE iocp_handle = (HANDLE)data;
	DWORD trans_byte;
	COMP_KEY* key = NULL;
	IO_DATA* io_data = NULL;
	DWORD flags = 0;

	while (TRUE)
	{
		BOOL bRet = GetQueuedCompletionStatus(iocp_handle, &trans_byte, (PULONG_PTR)&key, (LPOVERLAPPED*)&io_data, INFINITE);
		if (bRet == 0 || trans_byte == 0) {
			sock_push_msg(SOCK_TYPE_CLOSE, key->s, 0);
			continue;
		}
		else {
			sock_push_msg(SOCK_TYPE_DATA, key->s, io_data->data);
		}
		ZeroMemory(&io_data->over_lap, sizeof(io_data->over_lap));
		ZeroMemory(io_data->data, MAX_BUFFER_SIZE);
		io_data->buffer.len = MAX_BUFFER_SIZE;
		io_data->buffer.buf = io_data->data;
		WSARecv(key->s, &io_data->buffer, 1, &io_data->datalen, &flags, &io_data->over_lap, NULL);
	}
	return 1;
}

static void bind_iocp(SOCKET s, COMP_KEY* key) {
	CreateIoCompletionPort((HANDLE)s, g_iocp_handle, (ULONG_PTR)key, 0);
}

#endif