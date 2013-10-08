/*
** Copyright 2000-2013 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"maildirfilter.h"
#include	"maildirfiltertypelist.h"
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdlib.h>

int main(int argc, char **argv)
{
const char *name;
enum maildirfiltertype type;
const char *header;
const char *value;
const char *folder;
int errcode, i;
struct maildirfilter mf;
struct maildirfilterrule *r;
int flags=0;
const char *charset=getenv("CHARSET");

	if (argc < 6)
	{
		fprintf(stderr, "Invalid args\n");
		return (1);
	}

	name=argv[1];

	value=argv[2];
	if (*value == '-')
	{
		flags |= MFR_DOESNOT;
		++value;
	}
	if (*value == '/')
	{
		flags |= MFR_BODY;
		++value;
	}

	for (i=0; typelist[i].name; i++)
		if (strcasecmp(typelist[i].name, value) == 0)
			break;
	if (!typelist[i].name)
	{
		fprintf(stderr, "unknown op: %s\n", argv[2]);
		return (1);
	}
	type=typelist[i].type;
	header=argv[3];
	value=argv[4];
	folder=argv[5];

	memset(&mf, 0, sizeof(mf));

	errcode=maildir_filter_loadrules(&mf, "testrules");
	if (errcode && errcode != MF_LOADNOTFOUND)
	{
		fprintf(stderr, "Error loading testrules: %d\n", errcode);
		return (1);
	}

	if (!charset)
		charset="utf-8";

	r=maildir_filter_appendrule(&mf, name, type, flags,
				    header, value, folder, "", charset,
				    &errcode);

	if (!r)
	{
		fprintf(stderr, "Error appending %s: %d\n", name, errcode);
		return (1);
	}

	unlink("maildirsize");
	errcode=maildir_filter_saverules(&mf, "testrules2", ".", "Maildir", "nobody@example.com");
	if (errcode)
	{
		fprintf(stderr, "Error saving testrules2: %d\n", errcode);
		return (1);
	}
	rename("testrules2", "testrules");
	printf("Added %s\n", r->rulename_utf8);
	return (0);
}
