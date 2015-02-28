USE_TYPE ?= USE_SELECT
epoll: USE_TYPE = USE_EPOLL
kqueue: USE_TYPE = USE_KQUEUE
CC = gcc
SRC_FILE = client.c main.c server.c socket.c epoll_poll.c iocp_poll.c kqueue_poll.c select_poll.c thread.c

TestSocket: $(foreach v, $(SRC_FILE), src/$(v))
	$(CC) -D$(USE_TYPE) -g -o $@ $^ -lpthread 
clean:
	rm TestSocket -f
