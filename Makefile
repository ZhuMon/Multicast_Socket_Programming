CC=gcc
S=lab2_server.c
C=lab2_client.c

all: clean server client
	@cp client ..

server: 
	@${CC} ${S} -o server

client: 
	@${CC} ${C} -o client

clean:
	@rm -f client server

