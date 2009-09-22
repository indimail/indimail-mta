/*
** Copyright 2002-2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "config.h"
#include "maildirsearch.h"

static const char rcsid[]="$Id: maildirsearchC.cpp,v 1.2 2003/01/05 04:22:50 mrsam Exp $";

mail::Search::Search()
{
	maildir_search_init(&sei);
}

mail::Search::~Search()
{
	maildir_search_destroy(&sei);
}

