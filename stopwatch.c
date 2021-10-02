/* stopwatch.c v0.1.0
 * Copyright (c) 2020 Dylan Lom <djl@dylanlom.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <poll.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

const char* argv0;

void usage()
{
    fprintf(stderr, "usage: %s\n", argv0);
    exit(1);
}

void
printtime(int t)
{
    printf("\r%d", t);
    fflush(stdout);
}

int
main(int argc, char* argv[])
{
    argv0 = *(argv++);
    argc--;

    if (argc > 0) usage();

    time_t t = 0;
    int pollrs = 0;

    /* Use non-canoncial mode on stdin */
    struct termios old, new;
    tcgetattr(0, &old);
    tcgetattr(0, &new);
    new.c_lflag &= ~(ICANON | ECHO);
    new.c_cc[VMIN] = 0;
    new.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new);

    struct pollfd fds[1] = {{ .fd = 0, .events = POLLIN }};
    while (pollrs < 1) {
        printtime(t);
        t++;
        pollrs = poll(fds, 1, 1000);
    }
    puts("");
    tcsetattr(0, TCSANOW, &old);

    return 0;
}
