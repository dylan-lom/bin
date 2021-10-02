.PHONY: all clean

CC=gcc
BINS=countdown stopwatch
CFLAGS=-Wall -Wextra -pedantic

all: $(BINS)

clean:
	rm -f $(BINS)

countdown: countdown.c
	$(CC) $(CFLAGS) -o countdown countdown.c

stopwatch: stopwatch.c
	$(CC) $(CFLAGS) -o stopwatch stopwatch.c

