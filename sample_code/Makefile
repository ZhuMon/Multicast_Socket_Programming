CC=gcc
S=multicast_server.c
C=multicast_client.c

all: server client

server: 
	@${CC} ${S} -o server

client: 
	@${CC} ${C} -o client

clean:
	@rm client server

