#ifndef	maildir_autoresponse_h
#define	maildir_autoresponse_h

/*
** Copyright 2001-2003 Double Precision, Inc.
** See COPYING for distribution information.
*/


#include	"config.h"
#include	<stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

	/* Return a list of available autoresponses, NULL if error */

extern char **maildir_autoresponse_list(const char *);
extern void maildir_autoresponse_list_free(char **);

	/* Validate the autoresponse name */

extern int maildir_autoresponse_validate(const char *, const char *);

	/* Delete/Create/Open autoresponse text */

extern void maildir_autoresponse_delete(const char *, const char *);
extern FILE *maildir_autoresponse_create(const char *, const char *);
extern int maildir_autoresponse_create_finish(const char *, const char *, FILE *);

extern FILE *maildir_autoresponse_open(const char *, const char *);

#ifdef __cplusplus
}
#endif

#endif
