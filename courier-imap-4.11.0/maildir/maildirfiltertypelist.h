#ifndef	maildirfiltertypelist_h
#define	maildirfiltertypelist_h

/*
** Copyright 2000-2002 Double Precision, Inc.
** See COPYING for distribution information.
*/


#include	"config.h"

static struct {
	enum maildirfiltertype type;
	const char *name;
	} typelist[] = {
		{startswith, "startswith"},
		{endswith, "endswith"},
		{contains, "contains"},
		{hasrecipient, "hasrecipient"},
		{mimemultipart, "mimemultipart"},
		{textplain, "textplain"},
		{islargerthan, "islargerthan"},
		{anymessage, "anymessage"},
		{ 0, 0}};

#endif
