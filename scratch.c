/* scratch.c v0.1.0
 * Copyright (c) 2021 Dylan Lom <djl@dylanlom.com>
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

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

char cmd[1024];

time_t dateadd(time_t start, const char *offset) {
    struct tm *tm = localtime(&start);

    if (offset[0] == '+' && offset[1] == '\0') {
        tm->tm_mday += 1;
        return mktime(tm);
    } else if (offset[0] == '-' && offset[1] == '\0') {
        tm->tm_mday -= 1;
        return mktime(tm);
    }

    errno = 0;
    char *endptr;
    long l = strtol(offset, &endptr, 10);
    if (errno) {
        fprintf(stderr, "ERROR: Unable to parse offset: %s.\n",
                strerror(errno));
        exit(1);
    }

    switch (*endptr) {
    case '\0':
    case 'd':
    case 'D':
        tm->tm_mday += (int)l;
        break;
    case 'w':
        tm->tm_mday += (int)7*l;
        break;
    case 'm':
    case 'M':
        tm->tm_mon += (int)l;
        break;
    case 'y':
    case 'Y':
        tm->tm_year += (int)l;
        break;
    }

    return mktime(tm);
}

/* Open a "scratch" document for the day */
int
main(int argc, const char *argv[])
{
    const char *editor = getenv("VISUAL");
    if (editor == NULL) {
        fprintf(stderr, "ERROR: VISUAL environment variable is not set.\n");
        exit(1);
    }

    const char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "ERROR: HOME environment variable is not set.\n");
        exit(1);
    }

    time_t day = time(0);
    if (argc > 1) day = dateadd(day, argv[1]);

    // Construct the command string
    {
        int cmdsize = sizeof(cmd);
        char *cmdend = cmd;

        int n = snprintf(cmdend, cmdsize, "%s %s/.scratch/", editor, home);
        cmdend += n;
        cmdsize -= n;
        if (cmdsize <= 0) {
            assert(0 && "Ran out of space while constructing command.");
        }

        // Make ~/.scratch if it does not exist
        struct stat sb;
        char *pathname = cmd + strlen(editor) + 1;
        if (stat(pathname, &sb) == -1 || !S_ISDIR(sb.st_mode)) {
            if (mkdir(pathname, S_IRWXU) == -1) {
                fprintf(stderr,
                        "ERROR: Unable to create directory %s: %s.\n",
                        pathname, strerror(errno));
            }
        }

        n = strftime(cmdend, cmdsize, "%Y-%m-%d.md", localtime(&day));
        cmdend += n;
        cmdsize -= n;
        if (n == 0 || cmdsize <= 0) {
            assert(0 && "Ran out of space while constructing command.");
        }
    }

    int exitcode;
    if ((exitcode = system(cmd)) == -1) {
        fprintf(stderr, "ERROR: Unable to start editor: %s\n", strerror(errno));
        exit(1);
    }

    return exitcode;
}
