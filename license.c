/* Copyright (c) 2021 Dylan Lom <djl@dylanlom.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *zerobsd = "Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.\n\nTHE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.\n";

const char *mit = "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\nThe above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n";

const char *wtf = "            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE\n   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n\n     0. You just DO WHAT THE FUCK YOU WANT TO.\n";

const char *progname;

void
usage(void)
{
    fprintf(stderr, "USAGE: %s [-s [name [email]]] 0bsd|zerobsd|mit|wtf\n", progname);
    fprintf(stderr, "NOTES: Only first character of license name required.\n");
    fprintf(stderr, "       License name is case-insensitive.\n");
    fprintf(stderr, "       -s flag will print Copyright (c) <year> <name> [<<email>>].\n");
    fprintf(stderr, "       -s flag will user login for name if not provided.\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    progname = *argv++;
    argc--;

    if (argc > 1) {
        if (strcmp("-s", *argv++) != 0) {
            usage();
        }
        argc--;

        char *name, *email = NULL;
        if (argc > 1) {
            name = *argv++;
            argc--;

            if (argc > 1) {
                email = *argv++;
                argc--;
            }
        } else {
            name = getlogin();
        }

        time_t now = time(NULL);
        struct tm *tm = localtime(&now);

        printf("Copyright (c) %d %s", tm->tm_year + 1900, name);
        if (email) printf(" <%s> ", email);
        printf("\n\n");
    }

    if (argc != 1) {
        usage();
    }

    switch (**argv) {
    case '0':
    case 'z':
    case 'Z':
        printf(zerobsd);
        break;
    case 'm':
    case 'M':
        printf(mit);
        break;
    case 'w':
    case 'W':
        printf(wtf);
        break;
    default:
        usage();
    }
}
