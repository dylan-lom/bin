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

char *
scratchcmd(const char *editor, const char *path, const char *file)
{
	size_t bufflen = strlen(editor) + 1 + strlen(path) + 1 + strlen(file) + 1;
	char *buff = calloc(bufflen, sizeof(*buff));

	size_t ret = snprintf(buff, bufflen, "%s %s/%s", editor, path, file);
	assert(ret < bufflen);
	return buff;
}

char *
scratchpath(const char *home)
{
	const char *dir = "/.scratch";
	size_t pathlen = strlen(home) + strlen(dir) + 1;
	char *path = calloc(pathlen, sizeof(*path));

	size_t ret = snprintf(path, pathlen, "%s%s", home, dir);
	assert(ret < pathlen); // Indicates we truncated something!
	return path;
}

char *
scratchfile(time_t date)
{
	size_t filelen = strlen("YYYY-mm-dd.md") + 1;
	char *file = calloc(filelen, sizeof(*file));

	size_t ret = strftime(file, filelen, "%Y-%m-%d.md", localtime(&date));
	assert(ret > 0); // Indicates an error
	return file;
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

	char *path = scratchpath(home);
	char *file = scratchfile(day);
	char *cmd = scratchcmd(editor, path, file);

	int exitcode = 0;

	if (dry) {
		printf("%s/%s\n", path, file);
	} else {
		// Make ~/.scratch if it does not exist
		struct stat sb;
		if (stat(path, &sb) == -1 || !S_ISDIR(sb.st_mode)) {
			if (mkdir(path, S_IRWXU) == -1) {
				fprintf(stderr,
						"ERROR: Unable to create directory %s: %s.\n",
						path, strerror(errno));
			}
		}

		if ((exitcode = system(cmd)) == -1) {
			fprintf(stderr, "ERROR: Unable to start editor: %s\n", strerror(errno));
			exit(1);
		}
	}

	free(path);
	free(file);
	free(cmd);

	return exitcode;
}

