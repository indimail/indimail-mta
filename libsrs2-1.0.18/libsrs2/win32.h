#ifndef __WIN32_H__
#define __WIN32_H__

#ifdef _WIN32
/* Copyright (c) 2004 Shevek (srs@anarres.org)
 * All rights reserved.
 *
 * This file is a part of libsrs2 from http://www.libsrs2.org/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, under the terms of either the GNU General Public
 * License version 2 or the BSD license, at the discretion of the
 * user. Copies of these licenses have been included in the libsrs2
 * distribution. See the the file called LICENSE for more
 * information.
 */

#define __BEGIN_DECLS
#define __END_DECLS

#define HAVE_STDARG_H

#define HAVE_TIME_H

#define HAVE_STRING_H

#define SIZEOF_UNSIGNED_LONG 4

#define alloca _alloca

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#endif

#endif
