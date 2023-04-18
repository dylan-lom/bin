/* cc -o scratch -Wall -Wextra -pedantic scratch.c */

#include <time.h>
#include <ctype.h>
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

    if (offset != endptr) {
        switch (tolower(*endptr)) {
        case '\0':
        case 'd':
            tm->tm_mday += (int)l;
            break;
        case 'w':
            tm->tm_mday += (int)7*l;
            break;
        case 'm':
            tm->tm_mon += (int)l;
            break;
        case 'y':
            tm->tm_year += (int)l;
            break;
        default:
            fprintf(stderr, "ERROR: Unknown date type: `%s`\n", endptr);
            exit(1);
        }
    } else {
        // strtol coulnd't parse anything -- check if we got a DOTW.
        int sign = 1;
        if (offset[0] == '-') {
            sign = -1;
            offset++;
        } else if (offset[0] == '+') {
            offset++;
        }

        size_t len = strlen(offset);
        int weekday;
        if (strncasecmp("sunday", offset, len) == 0) {
            if (len < 2) {
                fprintf(stderr, "ERROR: Ambiguous weekday: `%s`\n", offset);
                exit(1);
            }
            weekday = 0;
        } else if (strncasecmp("monday", offset, len) == 0) {
            weekday = 1;
        } else if (strncasecmp("tuesday", offset, len) == 0) {
            if (len < 2) {
                fprintf(stderr, "ERROR: Ambiguous weekday: `%s`\n", offset);
                exit(1);
            }
            weekday = 2;
        } else if (strncasecmp("wednesday", offset, len) == 0) {
            weekday = 3;
        } else if (strncasecmp("thursday", offset, len) == 0) {
            weekday = 4;
        } else if (strncasecmp("friday", offset, len) == 0) {
            weekday = 5;
        } else if (strncasecmp("saturday", offset, len) == 0) {
            weekday = 6;
        } else {
            fprintf(stderr, "ERROR: Couldn't parse offset: `%s`\n", offset);
            exit(1);
        }

        do {
            tm->tm_mday += 1 * sign;
            mktime(tm);
        } while (tm->tm_wday != weekday);
    }

    return mktime(tm);
}

/* Open a "scratch" document for the day */
int
main(int argc, const char *argv[])
{
    int dry = 0;
    if (argc > 1 && strcmp("-n", argv[1]) == 0) {
        dry = 1;
        argc--;
        argv++;
    }

    const char *editor = getenv("VISUAL");
    if (!dry && editor == NULL) {
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

    /* TODO: This should be extracted + refactored at this point. */
    // Construct the command string
    {
        int cmdsize = sizeof(cmd);
        char *cmdend = cmd;
        int n = -1;
        if (dry) {
            n = snprintf(cmdend, cmdsize, "%s/.scratch/", home);
        } else {
            n = snprintf(cmdend, cmdsize, "%s %s/.scratch/", editor, home);
        }
        cmdend += n;
        cmdsize -= n;
        if (cmdsize <= 0) {
            assert(0 && "Ran out of space while constructing command.");
        }

        if (!dry) {
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
        }

        n = strftime(cmdend, cmdsize, "%Y-%m-%d.md", localtime(&day));
        cmdend += n;
        cmdsize -= n;
        if (n == 0 || cmdsize <= 0) {
            assert(0 && "Ran out of space while constructing command.");
        }
    }

    if (dry) {
        printf("%s\n", cmd);
        exit(0);
   }

    int exitcode;
    if ((exitcode = system(cmd)) == -1) {
        fprintf(stderr, "ERROR: Unable to start editor: %s\n", strerror(errno));
        exit(1);
    }

    return exitcode;
}

