CC = gcc

all: server client

server: server.o multmodulo.o
	$(CC) -o server server.o multmodulo.o -pthread 

client: client.o
	$(CC) -o client client.o

multmodulo.o: multmodulo.h multmodulo.c
	$(CC) -c multmodulo.c

server.o: server.c 
	$(CC) -c server.c

client.o: client.c 
	$(CC) -c client.c

clean: 
	rm server client server.o client.o multmodulo.o