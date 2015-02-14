CC = gcc
SRC_FILE = client.c main.c server.c socket.c

TestSocket: $(foreach v, $(SRC_FILE), src/$(v))
	$(CC) -g -o $@ $^ -lpthread
