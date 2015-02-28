#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#if defined(USE_KQUEUE)

#include "socket.h"

void poll_start(SOCKET lsfd) {

}

int poll_wait(SOCKET lsfd) {
	return 1;
}

void poll_close() {

}

#endif
#endif