/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"maildir/maildirkeywords.h"

static struct libmail_kwHashtable h;

int smapflag=0;

static int count_flags(struct libmail_keywordEntry *dummy1, void *dummy)
{
	++*(size_t *)dummy;

	return 0;
}

static struct libmail_kwMessage *msgs[3];
static const char * const flags[]={"apple", "banana", "pear", "grape"};


static int dump()
{
	size_t cnt=0;

	if (libmail_kwEnumerate(&h, &count_flags, &cnt))
		return -1;

	printf("%d flags\n", (int)cnt);

	for (cnt=0; cnt<sizeof(msgs)/sizeof(msgs[0]); cnt++)
	{
		struct libmail_kwMessageEntry *e;

		printf("%d:", (int)cnt);

		for (e=msgs[cnt]->firstEntry; e; e=e->next)
			printf(" %s", keywordName(e->libmail_keywordEntryPtr));
		printf("\n");
	}
	return 0;

}

int main()
{
	size_t i;

	libmail_kwhInit(&h);

	for (i=0; i<sizeof(msgs)/sizeof(msgs[0]); i++)
	{
		if ((msgs[i]=libmail_kwmCreate()) == NULL)
		{
			perror("malloc");
			exit(1);
		}

		msgs[i]->u.userNum=i;
	}

	if (libmail_kwmSetName(&h, msgs[0], flags[0]) >= 0 &&
	    libmail_kwmSetName(&h, msgs[1], flags[1]) >= 0 &&
	    libmail_kwmSetName(&h, msgs[2], flags[2]) >= 0 &&
	    libmail_kwmSetName(&h, msgs[0], flags[0]) >= 0 &&
	    libmail_kwmSetName(&h, msgs[0], flags[1]) >= 0 &&
	    libmail_kwmSetName(&h, msgs[1], flags[2]) >= 0 &&
	    libmail_kwmSetName(&h, msgs[2], flags[3]) >= 0)
	{

		if (dump() == 0)
		{
			libmail_kwmClearName(msgs[2], flags[3]);
			libmail_kwmClearName(msgs[2], flags[3]);
			libmail_kwmClearName(msgs[0], flags[1]);

			if (dump() == 0)
				exit(0);
		}

	}

	perror("ERROR");
	exit(1);
	return 0;
}
