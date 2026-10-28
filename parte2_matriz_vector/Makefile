CC = gcc
CFLAGS = -O3 -Wall -Wextra -pthread
LDFLAGS = -lm -lpthread

TARGETS = matvec_secuencial matvec_pthreads

all: $(TARGETS)

matvec_secuencial: matvec_secuencial.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

matvec_pthreads: matvec_pthreads.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
