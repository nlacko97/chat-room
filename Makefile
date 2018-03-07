
CC = gcc
FL = -Wall

all: select-server client #server

# server: server.c
# 	$(CC) $(FL) -o $@ -pthread server.c

client: client.c
	$(CC) $(FL) -o $@ -pthread client.c

select-server: select-server.c
	$(CC) $(FL) -o $@ select-server.c

.PHONY: clean

clean:
	rm -f server client select-server *.o
