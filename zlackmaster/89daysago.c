/* cc -Wall -Wextra -pedantic -o 89daysago 89daysago.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* I cannot believe how fucking hard it is to build a project from github.com */
int
main(void)
{
	time_t t = time(NULL);
	struct tm *tm = gmtime(&t);
	tm->tm_mday -= 89;
	printf("%lld\n", mktime(tm));
	return 0;
}