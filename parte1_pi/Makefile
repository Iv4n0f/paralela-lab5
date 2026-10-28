CC = gcc
CFLAGS = -O3 -Wall -Wextra -pthread
LDFLAGS = -lm -lpthread

TARGETS = pi_secuencial pi_pthreads

all: $(TARGETS)

pi_secuencial: pi_secuencial.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

pi_pthreads: pi_pthreads.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
