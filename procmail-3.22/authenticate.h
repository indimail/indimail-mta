/*
 * $Id: authenticate.h,v 1.5 1999/04/19 06:36:59 guenther Exp $
 */

/*
 * Generic authentication interface, substitute a suitable module to
 * accomodate arbitrary other authentication databases 
 */
#include <sys/types.h>
typedef struct auth_identity auth_identity;

#ifndef P
#define P(x)	x
#define Q(x)	()
#endif

auth_identity *auth_finduser P((char *const user, const int sock));
auth_identity *auth_finduid Q((const uid_t uid, const int sock));
auth_identity *auth_newid P((void));
int auth_checkpassword P((const auth_identity * const pass, const char *const pw, const int allowemptypw));
int auth_filledid P((const auth_identity * pass));
const char     *auth_getsecret P((const auth_identity * const pass));
const char     *auth_mailboxname P((auth_identity * const pass));
const char     *auth_homedir P((const auth_identity * const pass));
const char     *auth_shell P((const auth_identity * const pass));
const char     *auth_username P((const auth_identity * const pass));
uid_t           auth_whatuid P((const auth_identity * const pass));
uid_t           auth_whatgid P((const auth_identity * const pass));
void auth_copyid P((auth_identity * newpass, const auth_identity * oldpass));
void auth_freeid P((auth_identity * pass));
void auth_end   P((void));
