.PHONY: all clean

CC=gcc
BINS=countdown license stopwatch
CFLAGS=-Wall -Wextra -pedantic

all: $(BINS)

clean:
	rm -f $(BINS)

countdown: countdown.c
	$(CC) $(CFLAGS) -o countdown countdown.c

license: license.c
	$(CC) $(CFLAGS) -o license license.c

stopwatch: stopwatch.c
	$(CC) $(CFLAGS) -o stopwatch stopwatch.c

