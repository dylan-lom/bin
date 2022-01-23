.PHONY: all clean

CC=gcc
BINS=countdown license scratch stopwatch
CFLAGS=-ggdb -Wall -Wextra -Werror -pedantic

all: $(BINS)

clean:
	rm -f $(BINS)

binday: binday.c
	$(CC) $(CFLAGS) -o binday binday.c
countdown: countdown.c
	$(CC) $(CFLAGS) -o countdown countdown.c

license: license.c
	$(CC) $(CFLAGS) -o license license.c

scratch: scratch.c
	$(CC) $(CFLAGS) -o scratch scratch.c

stopwatch: stopwatch.c
	$(CC) $(CFLAGS) -o stopwatch stopwatch.c

