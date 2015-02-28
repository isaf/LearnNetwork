#include "thread.h"
#ifndef _WIN32
#include <pthread.h>
#endif // !_WIN32

THREAD_ID thread_create(THREAD_PROC proc, void* data) {
	THREAD_ID thread_id;
#ifdef _WIN32
	CreateThread(NULL, 0, proc, data, 0, &thread_id);
#else
	pthread_create(&thread_id, NULL, proc, data);
#endif
	return thread_id;
}
