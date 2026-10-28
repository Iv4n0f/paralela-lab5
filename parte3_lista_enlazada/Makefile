CC = gcc
CFLAGS = -O3 -Wall -Wextra -pthread -D_GNU_SOURCE
LDFLAGS = -lpthread

TARGETS = lista_sin_sync lista_mutex lista_rwlock

all: $(TARGETS)

lista_sin_sync: lista_sin_sync.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

lista_mutex: lista_mutex.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

lista_rwlock: lista_rwlock.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
