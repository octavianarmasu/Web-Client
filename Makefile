CC=gcc
CFLAGS=-g -I.

client: client.c requests.c helpers.c buffer.c parson.c
	$(CC) -o client client.c requests.c buffer.c helpers.c parson.c -Wall

run: client
	./client

clean:
	rm -f *.o client
