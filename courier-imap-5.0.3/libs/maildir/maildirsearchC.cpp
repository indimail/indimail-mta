/*
** Copyright 2002-2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "config.h"
#include "maildirsearch.h"


mail::Search::Search()
{
	maildir_search_init(&sei);
}

mail::Search::~Search()
{
	maildir_search_destroy(&sei);
}

