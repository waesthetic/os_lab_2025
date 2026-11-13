CC = gcc
CFLAGS = -std=c11 -Wall -Wextra
LDLIBS = -pthread

.PHONY: all clean

all: client server

client: client.c modulo.h libmodulo.a
	$(CC) $(CFLAGS) client.c libmodulo.a -o client $(LDLIBS)

server: server.c modulo.h libmodulo.a
	$(CC) $(CFLAGS) server.c libmodulo.a -o server $(LDLIBS)

libmodulo.a: modulo.o
	ar rcs libmodulo.a modulo.o

modulo.o: modulo.c modulo.h
	$(CC) $(CFLAGS) -c modulo.c -o modulo.o

clean:
	rm -f client server modulo.o libmodulo.o