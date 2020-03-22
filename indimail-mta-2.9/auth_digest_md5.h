/*
 * $Log: auth_digest_md5.h,v $
 * Revision 1.2  2011-12-05 15:07:16+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2011-10-29 20:41:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _AUTH_DIGEST_MD5_H
#define _AUTH_DIGEST_MD5_H 1

#ifndef	lint
static char     sccsidauthdigestmd5h[] = "$Id: auth_digest_md5.h,v 1.2 2011-12-05 15:07:16+05:30 Cprogrammer Stab mbhangui $";
#endif

char *digest_md5(char *, int, char *, int, char *, int, char *, char *, int, char *, int, char *, char *, char *);

#endif
