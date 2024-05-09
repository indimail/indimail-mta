/*
 * $Log: yearcal.c,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2016-01-28 23:42:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "caldate.h"

const char     *montab[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             year;
	long            daystart;
	long            dayend;
	long            day;
	int             weekday;
	struct caldate  cd;

	while (*++argv) {
		year = atoi(*argv);
		cd.year = year;
		cd.month = 1;
		cd.day = 1;
		daystart = caldate_mjd(&cd);
		cd.year = year + 1;
		dayend = caldate_mjd(&cd);

		while ((daystart + 3) % 7)
			--daystart;
		while ((dayend + 3) % 7)
			++dayend;
		printf(" S  M  T  W  T  F  S\n");
		for (day = daystart; day < dayend; ++day) {
			caldate_frommjd(&cd, day, &weekday, (int *) 0);
			if (cd.year != year)
				printf("   ");
			else {
				if (cd.month & 1)
					if (cd.day < 10)
						printf(" %d%c%d ", cd.day % 10, 8, cd.day % 10);
					else
						printf("%d%c%d%d%c%d ", cd.day / 10, 8, cd.day / 10, cd.day % 10, 8, cd.day % 10);
				else
					printf("%2d ", cd.day);
				if (weekday == 6) {
					if ((cd.day >= 15) && (cd.day < 22))
						printf(" %s %d\n", montab[cd.month - 1], year);
					else
						printf("\n");
				}
			}
		}
		printf("\n");
	}
	return(0);
}

void
getversion_yearcal_c()
{
	const char     *x = "$Id: yearcal.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
