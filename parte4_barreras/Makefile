CC = gcc
CFLAGS = -O3 -Wall -Wextra -pthread -D_GNU_SOURCE
LDFLAGS = -lpthread

TARGETS = barrera_busywait barrera_semaforos barrera_condvar

all: $(TARGETS)

barrera_busywait: barrera_busywait.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

barrera_semaforos: barrera_semaforos.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

barrera_condvar: barrera_condvar.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
