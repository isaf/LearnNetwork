#if defined(linux) && defined(USE_EPOLL)

#include "socket.h"

void poll_start(SOCKET lsfd) {

}

int poll_wait(SOCKET lsfd) {
	return 1;
}

void poll_close() {
	
}

#endif // linux