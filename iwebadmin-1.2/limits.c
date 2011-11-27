/*
 * $Id: limits.c,v 1.3 2011-11-17 22:10:48+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#include <indimail_config.h>
#undef PACKAGE
#undef VERSION
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include <indimail.h>
#undef PACKAGE
#undef VERSION
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include "config.h"

#include "iwebadmin.h"
#include "iwebadminx.h"
#include "limits.h"

void
load_limits()
{
	int             gid = 0;
	vget_limits(Domain, &Limits);
	MaxPopAccounts = Limits.maxpopaccounts;
	MaxAliases = Limits.maxaliases;
	MaxForwards = Limits.maxforwards;
	MaxAutoResponders = Limits.maxautoresponders;
	MaxMailingLists = Limits.maxmailinglists;
	gid |= vlimits_get_flag_mask(&Limits);
}

void
getversion_qalimit_c()
{
	printf("%s\n", sccsidh);
}
