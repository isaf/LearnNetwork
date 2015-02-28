#ifndef __POLL_H__
#define __POLL_H__

void poll_start(SOCKET lsfd);
int poll_wait(SOCKET lsfd);
void poll_close();

#endif //__POLL_H__