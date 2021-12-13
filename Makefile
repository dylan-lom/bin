.PHONY: all clean

CC=gcc
BINS=countdown license scratch stopwatch
CFLAGS=-Wall -Wextra -Werror -pedantic

all: $(BINS)

clean:
	rm -f $(BINS)

countdown: countdown.c
	$(CC) $(CFLAGS) -o countdown countdown.c

license: license.c
	$(CC) $(CFLAGS) -o license license.c

scratch: scratch.c
	$(CC) $(CFLAGS) -o scratch scratch.c

stopwatch: stopwatch.c
	$(CC) $(CFLAGS) -o stopwatch stopwatch.c

