#ifndef __THREAD_H__
#define __THREAD_H__

#ifdef _WIN32
#include<windows.h>
#define CALL_BACK DWORD WINAPI
#else
#include <pthread.h>
#define CALL_BACK void*
#endif

#ifdef _WIN32
typedef DWORD THREAD_ID;
typedef LPTHREAD_START_ROUTINE THREAD_PROC;
#else
typedef pthread_t THREAD_ID;
typedef void* (*THREAD_PROC)(void*);
#endif

THREAD_ID thread_create(THREAD_PROC proc, void* args);

#endif //__THREAD_H__