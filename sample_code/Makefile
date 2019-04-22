CC=gcc
S=multicast_server.c
C=multicast_client.c

all: clean server client

server: 
	@${CC} ${S} -o server

client: 
	@${CC} ${C} -o client

clean:
	@rm -f client server

