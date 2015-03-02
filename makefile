USE_TYPE ?= USE_SELECT

none :
	@echo "Please do command as follow:"
	@echo "'make select' to test select in linux."
	@echo "'make epoll' to test epoll in linux."
	@echo "'make kqueue' to test kqueue in macosx."
	@echo "'make clean' to clean the project."

select : USE_TYPE = USE_SELECT
epoll : USE_TYPE = USE_EPOLL
kqueue : USE_TYPE = USE_KQUEUE
CC = gcc
SRC_FILE = client.c main.c server.c socket.c epoll_poll.c iocp_poll.c kqueue_poll.c select_poll.c thread.c

select epoll kqueue : clean TestSocket

TestSocket: $(foreach v, $(SRC_FILE), src/$(v))
	$(CC) -D$(USE_TYPE) -g -o $@ $^ -lpthread 
clean:
	rm TestSocket -f
