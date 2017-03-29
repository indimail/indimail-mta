#ifndef	imapd_h
#define	imapd_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/


#define	HIERCH	'.'		/* Hierarchy separator char */
#define	HIERCHS	"."		/* Hierarchy separator char */

#define	NEWMSG_FLAG	'*'	/* Prefixed to mimeinfo to indicate new msg */


#define	is_sharedsubdir(dir) \
	(strncmp((dir), SHAREDSUBDIR "/", \
		 sizeof (SHAREDSUBDIR "/")-1) == 0)

#define	SUBSCRIBEFILE	"courierimapsubscribed"

extern void check_rights(const char *mailbox,
			 char *rights_buf);

#define CHECK_RIGHTSM(mailbox, varname, rights) \
	char varname[sizeof(rights)]; \
	strcpy(varname, rights); \
	check_rights(mailbox, varname);


#endif
