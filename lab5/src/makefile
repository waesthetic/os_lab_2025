CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O0

all: mutex_without mutex_with factorial deadlock

mutex_without: mutex.c
	$(CC) $(CFLAGS) mutex.c -o mutex_without -pthread

mutex_with: mutex.c
	$(CC) $(CFLAGS) -DUSE_MUTEX mutex.c -o mutex_with -pthread

factorial: factorial.c
	$(CC) $(CFLAGS) factorial.c -o factorial -pthread

deadlock: deadlock.c
	$(CC) $(CFLAGS) deadlock.c -o deadlock -pthread

clean:
	rm -f mutex_without mutex_with factorial deadlock

.PHONY: all clean
