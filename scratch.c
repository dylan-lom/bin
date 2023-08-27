/* cc -o scratch -Wall -Wextra -pedantic scratch.c */

#include <time.h>
#include <ctype.h>
#include <err.h>
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
	if (errno) err(1, "ERROR: Unable to parse offset");

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
			errx(1, "ERROR: Unknown date type: `%s`\n", endptr);
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
			if (len < 2) errx(1, "ERROR: Ambiguous weekday: `%s`\n", offset);

			weekday = 0;
		} else if (strncasecmp("monday", offset, len) == 0) {
			weekday = 1;
		} else if (strncasecmp("tuesday", offset, len) == 0) {
			if (len < 2) errx(1, "ERROR: Ambiguous weekday: `%s`\n", offset);
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
			errx(1, "ERROR: Couldn't parse offset: `%s`\n", offset);
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
scratchfile(const char *category, time_t date)
{
	size_t filelen = strlen("YYYY-mm-dd.md") + 1;
	if (category != NULL) filelen += strlen(category) + 1;
	char *file = calloc(filelen, sizeof(*file));

	size_t ret = 0;
	if (category != NULL) {
		ret = snprintf(file, filelen, "%s-", category);
		assert(ret < filelen);
	}

	ret = strftime(file + ret, filelen - ret, "%Y-%m-%d.md", localtime(&date));
	assert(ret > 0); // Indicates an error
	return file;
}

int streq(const char *s1, const char *s2)
{
	return strcmp(s1, s2) == 0;
}

/* Open a "scratch" document for the day */
int
main(int argc, const char *argv[])
{
	int dry = 0;
	const char *cat = NULL;

	while (argc > 1) {
		if (streq(argv[1], "-n")) {
			dry = 1;
		} else if (streq(argv[1], "-c")) {
			if (argc < 3) errx(1, "-c option requires an argument");

			cat = argv[2];
			argc--; argv++;
		} else {
			break;
		}

		argc--; argv++;
	}


	const char *editor = getenv("VISUAL");
	if (!dry && editor == NULL) {
		errx(1, "ERROR: VISUAL environment variable is not set.\n");
	}

	const char *home = getenv("HOME");
	if (home == NULL) {
		errx(1, "ERROR: HOME environment variable is not set.\n");
	}

	time_t day = time(0);
	if (argc > 1) day = dateadd(day, argv[1]);

	char *path = scratchpath(home);
	char *file = scratchfile(cat, day);

	int exitcode = 0;

	if (dry) {
		printf("%s/%s\n", path, file);
	} else {
		// Make ~/.scratch if it does not exist
		struct stat sb;
		if (
			(stat(path, &sb) == -1 || !S_ISDIR(sb.st_mode)) // path doesn't exist
			&& mkdir(path, S_IRWXU) == -1 // failed to make it!
		) errx(1, "ERROR: Unable to create directory %s", path);

		char *cmd = scratchcmd(editor, path, file);
		exitcode = system(cmd);
		if (exitcode == -1) errx(1, "ERROR: Unable to start editor");
		free(cmd);
	}

	free(path);
	free(file);

	return exitcode;
}

