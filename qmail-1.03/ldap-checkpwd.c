/*-
 * $Log: ldap-checkpwd.c,v $
 * Revision 1.4  2010-06-02 15:20:37+05:30  Cprogrammer
 * fix for users without '@' sign
 *
 * Revision 1.3  2010-06-02 14:55:05+05:30  Cprogrammer
 * added anon bind
 *
 * Revision 1.2  2010-05-05 11:48:11+05:30  Cprogrammer
 * fixed calls to my_error()
 *
 * Revision 1.1  2010-04-22 14:27:09+05:30  Cprogrammer
 * Initial revision
 *
 *
 * ldap-checkpwd.c
 *
 * checkpassword implementation that searches an LDAP database
 * for all the necessary parameter.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ldap.h>
#include <sys/types.h>
#include "error.h"
#include "pathexec.h"
#include "str.h"
#include "env.h"
#include "scan.h"
/*----*/
#include "subfd.h"
#include "substdio.h"
#include "strerr.h"
#include "stralloc.h"

/*-
 * Customise these or set these as environment variables
 * LDAP_HOST hostname of your LDAP server
 * LDAP_BASE base under which entry must exist
 * LDAP_SCOPE search scope relative to base
 * LDAP_BIND_DN dn of the account who has the good permissions to request search
 * LDAP_BIND_PASSWD password of the account who has the good permissions to request search
 * LDAP_FILTER filter to use, %u gets replaced by login & %h gets replaced by domain
 * LDAP_UID_FIELD name of the field containing the system uid
 * LDAP_GID_FIELD name of the field containing the system gid
 * LDAP_HOME_FIELD name of field containing the directory of the mailbox
 *
 * #define LDAP_SCOPE_BASE			((ber_int_t) 0x0000)
 * #define LDAP_SCOPE_ONELEVEL		((ber_int_t) 0x0001)
 * #define LDAP_SCOPE_SUBTREE		((ber_int_t) 0x0002)
 * #define LDAP_SCOPE_SUBORDINATE	((ber_int_t) 0x0003)

 * #define LDAP_SCOPE_BASEOBJECT	LDAP_SCOPE_BASE
 * #define LDAP_SCOPE_ONE			LDAP_SCOPE_ONELEVEL
 * #define LDAP_SCOPE_SUB			LDAP_SCOPE_SUBTREE
 * #define LDAP_SCOPE_CHILDREN		LDAP_SCOPE_SUBORDINATE
 */

#define LDAP_HOST         "localhost"
#define LDAP_BASE         "ou=mail,o=enterprise"
#define LDAP_SCOPE        LDAP_SCOPE_SUBTREE
#define LDAP_FILTER       "(&(uid=%u)(dc=%h))"
#define LDAP_HOME_FIELD   "mailMessageStore"
#define LDAP_UID_FIELD    "uid"
#define LDAP_GID_FIELD    "gid"
#define LDAP_BIND_DN      "uid=auth,ou=mail,o=enterprise"
#define LDAP_BIND_PASSWD  "password"

#define PROTOCOL_LEN      512
#define FATAL "ldap-checkpwd: fatal: "

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
logerr(char *s)
{
	if (substdio_puts(subfderr, s) == -1)
		_exit(111);
}

void
logerrf(char *s)
{
	if (substdio_puts(subfderr, s) == -1)
		_exit(111);
	if (substdio_flush(subfderr) == -1)
		_exit(111);
}

static int      debug;

void
my_error(char *s1, char *s2, int exit_val)
{
	if (!debug)
		_exit(exit_val);
	logerr(s1);
	logerr(": ");
	if (s2) {
		logerr(s2);
		logerr(": ");
	}
	if (exit_val > 0)
		logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val > 0 ? exit_val : -exit_val);
}

static char     up[513];
#ifdef EXTENDED_ATTRIBUTES
static stralloc homedir = {0};
#endif

extern int      snprintf(char *, size_t, const char *, /*args*/ ...);
int             ldap_lookup(char *, char *, char **, uid_t *, gid_t *);

int
main(int argc, char *argv[])
{
	char           *login, *password, *error = 0;
	int             uplen, i, r;
	uid_t           uid;
	gid_t           gid;

	if (argc < 2)
		_exit(2);
	if (env_get("DEBUG"))
		debug = 1;
	uplen = 0;
	for (;;) {
		do
			r = read(3, up + uplen, sizeof (up) - uplen);
		while ((r == -1) && (errno == error_intr));
		if (r == -1)
			my_error("read", 0, 111);
		if (r == 0)
			break;
		uplen += r;
		if (uplen >= sizeof (up))
			my_error("read", "data too big", -1);
	}
	close(3);
	i = 0;
	if (i >= uplen)
		my_error("invalid data", 0, -2);
	login = up + i;
	while (up[i++])
		if (i >= uplen)
			my_error("invalid data", 0, -2);
	password = up + i;
	if (i >= uplen)
		my_error("invalid data", 0, -2);
	while (up[i++])
		if (i >= uplen)
			my_error("invalid data", 0, -2);
	if (!*login || !*password)
		my_error("invalid data", 0, -2);
	if (debug) {
		out("login [");
		out(login);
		out("] password [");
		out(password);
		out("]\n");
		flush();
	}
	i = ldap_lookup(login, password, &error, &uid, &gid);
	if (debug)
		flush();
	if (i == -1)
		my_error("ldap error", error, 111);
	else
	if (!i)
		_exit(0);
#ifdef EXTENDED_ATTRIBUTES
	if (homedir.len && chdir(homedir.s) != 0)
		my_error("chdir", homedir.s, 111);
#endif
	execvp(argv[1], argv + 1);
	my_error("execvp", argv[1], 111);
	/*- Not reached */
	_exit(111);
}

int
ldap_lookup(char *login, char *password, char **error, uid_t *userId, gid_t *groupId)
{
	char           *attrs[] = { NULL };
	char           *host, *dn, *port, *ptr;
	char           *ldap_host, *ldap_bind_dn, *ldap_bind_passwd = 0, *ldap_filter,
				   *ldap_base, *ldap_scope;
#ifdef EXTENDED_ATTRIBUTES
	char           *ldap_home_field, *ldap_uid_field, *ldap_gid_field;
	char          **values;
#endif
	static stralloc filter = {0};
	LDAP           *ld;
	LDAPMessage    *res, *res0;
	int             ret, ldap_port, scope, at;

	if (!(ldap_host = env_get("LDAP_HOST")))
		ldap_host = LDAP_HOST;
	if (!(port = env_get("LDAP_PORT")))
		ldap_port = LDAP_PORT;
	else
		scan_int(port, &ldap_port);
	if (!(ld = (LDAP *) ldap_init(ldap_host, ldap_port))) {
		if (error)
			*error = error_str(errno);
		return(-1);
	}
	if (login[at = str_chr(login, '@')] == 0)
		host = "";
	else
		host = login + at + 1;
	/*
	 * The bind DN. This is always used for simple authentication (although it may be a
	 * zero-length string for anonymous simple authentication), and is generally not used
	 * for SASL authentication.
	 *
	 * The credentials. The type of credentials provided vary based on the authentication
	 * type.
	 *
	 *  For simple authentication, the credentials should be the password for the target
	 *  bind DN, or an empty string for anonymous simple authentication.
	 *
	 *  For SASL authentication, the credentials should include the name of the SASL
	 *  mechanism to use, and may optionally include encoded credential information
	 *  appropriate for the SASL mechanism. 
	 */
	if (!(ldap_bind_dn = env_get("LDAP_BIND_DN"))) {
		ldap_bind_dn = "";
		ldap_bind_passwd = "";
	} else
		ldap_bind_passwd = env_get("LDAP_BIND_PASSWD");
	switch ((ret = ldap_simple_bind_s(ld, ldap_bind_dn, ldap_bind_passwd)))
	{
	case LDAP_SUCCESS:
		break;
	case LDAP_INVALID_CREDENTIALS:
		ldap_unbind_s(ld);
		return(1);
	default:
		if (error)
			*error = ldap_err2string(ret);
		ldap_unbind_s(ld);
		return -1;
	}
	if (!(ldap_filter = env_get("LDAP_FILTER")))
		ldap_filter = LDAP_FILTER;
	for (ptr = ldap_filter;*ptr;ptr++) {
		if (*ptr == '%' && *(ptr + 1)) {
			switch (*(ptr + 1))
			{
			case '%':
				if (!stralloc_catb(&filter, "%", 1)); {
					if (error)
						*error = "out of memory";
					return(-1);
				}
				ptr++;
				break;
			case 'e':
				if (!stralloc_cats(&filter, login)) {
					if (error)
						*error = "out of memory";
					return(-1);
				}
				ptr++;
				break;
			case 'u':
				if (!stralloc_catb(&filter, login, at)) {
					if (error)
						*error = "out of memory";
					return(-1);
				}
				ptr++;
				break;
			case 'h':
				if (!stralloc_cats(&filter, host)) {
					if (error)
						*error = "out of memory";
					return(-1);
				}
				ptr++;
				break;
			}
		} else
			stralloc_catb(&filter, ptr, 1);
	}
	if (!stralloc_0(&filter)) {
		if (error)
			*error = "out of memory";
		return(-1);
	}
	if (!(ldap_base = env_get("LDAP_BASE")))
		ldap_base = LDAP_BASE;
	if (!(ldap_scope = env_get("LDAP_SCOPE")))
		scope = LDAP_SCOPE;
	else {
		if (!str_diffn(ldap_scope, "base", 4))
			scope = LDAP_SCOPE_BASE;
		else
		if (!str_diffn(ldap_scope, "one", 3))
			scope = LDAP_SCOPE_ONELEVEL;
		else
		if (!str_diffn(ldap_scope, "sub", 3))
			scope = LDAP_SCOPE_SUBTREE;
		else
		if (!str_diffn(ldap_scope, "children", 3))
			scope = LDAP_SCOPE_SUBORDINATE;
		else {
			if (error)
				*error = "invalid scope";
			errno = EINVAL;
			return(-1);
		}
	}
	if (debug)
	{
		out("ldap_search: base[");
		out(ldap_base);
		out("] filter [");
		out(filter.s);
		out("]\n");
		flush();
	}
	if ((ret = ldap_search_s(ld, ldap_base, scope, filter.s, attrs, 0, &res))) {
		if (error)
			*error = ldap_err2string(ret);
		ldap_unbind_s(ld);
		return(-1);
	}
	if (!(res0 = ldap_first_entry(ld, res))) {
		if (error) {
			ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &ret);
			*error = ldap_err2string(ret);
		}
		ldap_msgfree(res);
		ldap_unbind_s(ld);
		return(1);
	}
	if (!(dn = ldap_get_dn(ld, res))) {
		if (error) {
			ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &ret);
			*error = ldap_err2string(ret);
		}
		ldap_msgfree(res);
		ldap_unbind_s(ld);
		return(-1);
	}
	if (debug) {
		out("dn=[");
		out(dn);
		out("]\n");
		flush();
	}
	if (debug) {
		out("ldap_simple_bind: password=[");
		out(password);
		out("]\n");
		flush();
	}
	switch ((ret = ldap_simple_bind_s(ld, dn, password)))
	{
	case LDAP_SUCCESS:
		break;
	case LDAP_INVALID_CREDENTIALS:
		ldap_msgfree(res);
		ldap_unbind_s(ld);
		return(1);
	default:
		if (error)
			*error = ldap_err2string(ret);
		ldap_msgfree(res);
		ldap_unbind_s(ld);
		return -1;
	}
	ldap_memfree(dn);
#ifdef EXTENDED_ATTRIBUTES
	if ((ldap_home_field = env_get("LDAP_HOME_FIELD")))
	{
		if (!(values = (char **) ldap_get_values(ld, res0, ldap_home_field))) {
			if (error) {
				ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &ret);
				*error = ldap_err2string(ret);
			}
			ldap_msgfree(res);
			ldap_unbind_s(ld);
			return(-1);
		}
		if (values && values[0]) {
			if (!stralloc_copys(&homedir, values[0])) {
				ldap_value_free(values);
				ldap_msgfree(res);
				ldap_unbind_s(ld);
				return(-1);
			}
			if (!stralloc_0(&homedir)) {
				ldap_value_free(values);
				ldap_msgfree(res);
				ldap_unbind_s(ld);
				return(-1);
			}
		}
		ldap_value_free(values);
	}
	if ((ldap_uid_field = env_get("LDAP_UID_FIELD")))
	{
		if (!(values = ldap_get_values(ld, res0, ldap_uid_field))) {
			if (error) {
				ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &ret);
				*error = ldap_err2string(ret);
			}
			ldap_msgfree(res);
			ldap_unbind_s(ld);
			return(-1);
		}
		if (values && values[0])
			scan_ulong(values[0], (unsigned long *) userId);
		ldap_value_free(values);
	}
	if ((ldap_gid_field = env_get("LDAP_GID_FIELD")))
	{
		if (!(values = ldap_get_values(ld, res0, ldap_gid_field))) {
			if (error) {
				ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &ret);
				*error = ldap_err2string(ret);
			}
			ldap_msgfree(res);
			ldap_unbind_s(ld);
			return(-1);
		}
		if (values && values[0])
			scan_ulong(values[0], (unsigned long *) groupId);
		ldap_value_free(values);
	}
#endif
	ldap_msgfree(res);
	ldap_unbind(ld);
	return(0);
}

void
getversion_ldap_checkpwd_c()
{
	static char    *x = "$Id: ldap-checkpwd.c,v 1.4 2010-06-02 15:20:37+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
