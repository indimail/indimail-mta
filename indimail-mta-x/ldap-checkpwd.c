/*-
 * $Log: ldap-checkpwd.c,v $
 * Revision 1.14  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.13  2023-07-13 02:43:28+05:30  Cprogrammer
 * replaced out() with subprintf()
 *
 * Revision 1.12  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.11  2021-05-29 23:44:46+05:30  Cprogrammer
 * replace str_chr with str_rchr to get domain correctly from email address
 *
 * Revision 1.10  2021-01-27 18:56:32+05:30  Cprogrammer
 * use env variable NATIVE_CHECKPASSWORD to comply with checkpassword protocol
 *
 * Revision 1.9  2021-01-27 16:54:02+05:30  Cprogrammer
 * added dovecot support
 *
 * Revision 1.8  2019-12-24 07:19:24+05:30  Cprogrammer
 * use LDAP_FIELD_xxx environment variables to get value of any ldap field
 *
 * Revision 1.7  2019-12-21 00:53:51+05:30  Cprogrammer
 * fixed multiple bugs
 *
 * Revision 1.6  2014-01-29 14:00:56+05:30  Cprogrammer
 * BUG - removed extra semicolon
 *
 * Revision 1.5  2011-01-18 21:16:33+05:30  Cprogrammer
 * conditional compilation of ldap code in ldap-checkpwd.c
 *
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
#include "hasldap.h"
#ifdef HASLDAP
#include <unistd.h>
#include <string.h>
#include <ldap.h>
#include <sys/types.h>
#include <error.h>
#include <pathexec.h>
#include <str.h>
#include <env.h>
#include <scan.h>
#include <subfd.h>
#include <substdio.h>
#include <qprintf.h>
#include <strerr.h>
#include <stralloc.h>
#include <noreturn.h>

/*-
 * Customise these or set these as environment variables
 * LDAP_HOST hostname of your LDAP server
 * LDAP_BASE base under which entry must exist
 * LDAP_SCOPE search scope relative to base
 * LDAP_BIND_DN dn of the account who has the good permissions to request search
 * LDAP_BIND_PASSWD password of the account who has the good permissions to request search
 * LDAP_FILTER filter to use, %u gets replaced by login & %h gets replaced by domain
 * LDAP_FIELD_xxx where xxx is name of a LDAP field like uid, gid, home, user password
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
#define LDAP_HOME_FIELD   "homeDirectory"
#define LDAP_UID_FIELD    "uidNumber"
#define LDAP_GID_FIELD    "gidNumber"
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
logerr(const char *s)
{
	if (substdio_puts(subfderr, s) == -1)
		_exit(111);
}

void
logerrf(const char *s)
{
	if (substdio_puts(subfderr, s) == -1)
		_exit(111);
	if (substdio_flush(subfderr) == -1)
		_exit(111);
}

static int      debug;

no_return void
my_error(const char *s1, const char *s2, int exit_val)
{
	logerr(s1);
	if (s2) {
		logerr(": ");
		logerr(s2);
	}
	if (exit_val < 0) {
		logerr(": ");
		logerr(error_str(errno));
	}
	logerrf("\n");
	_exit(exit_val > 0 ? exit_val : 111);
}

static char     up[513];
static stralloc buf = {0};
#ifdef EXTENDED_ATTRIBUTES
static stralloc homedir = {0};
#endif

extern int      snprintf(char *, size_t, const char *, /*args*/ ...);
int             ldap_lookup(const char *, const char *, const char *error[], uid_t *, gid_t *);

int
main(int argc, char *argv[])
{
	char           *login, *password, *ptr;
	const char     *error = NULL;
	int             uplen, i, r, native_checkpassword;
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
			my_error("read", 0, -1);
		if (r == 0)
			break;
		uplen += r;
		if (uplen >= sizeof (up))
			my_error("read", "data too big", 100);
	}
	close(3);
	i = 0;
	if (i >= uplen)
		my_error("invalid data", 0, 2);
	login = up + i;
	while (up[i++])
		if (i >= uplen)
			my_error("invalid data", 0, 2);
	password = up + i;
	if (i >= uplen)
		my_error("invalid data", 0, 2);
	while (up[i++])
		if (i >= uplen)
			my_error("invalid data", 0, 2);
	if (!*login || !*password)
		my_error("invalid data", 0, 2);
	native_checkpassword = (env_get("NATIVE_CHECKPASSWORD") || env_get("DOVECOT_VERSION")) ? 1 : 0;
	if (native_checkpassword) {
		if (!env_unset("userdb_uid") || !env_unset("userdb_gid") ||
				!env_unset("EXTRA"))
			my_error("out of mem", 0, -1);
	}
	if (debug) {
		subprintf(subfdout, "login [%s] password [%s]\n", login, password);
		flush();
	}
	if ((i = ldap_lookup(login, password, &error, &uid, &gid)) == -1)
		my_error("ldap lookup error", error, 111);
#ifdef EXTENDED_ATTRIBUTES
	if (homedir.len && chdir(homedir.s) != 0)
		my_error("chdir", homedir.s, -1);
	if (!env_put2("HOME", homedir.s))
		my_error("out of mem", 0, -1);
#endif
	if (!i) {
		if (native_checkpassword) { /*- support dovecot checkpassword */
			if (!env_put2("userdb_uid", "indimail") ||
					!env_put2("userdb_gid", "indimail"))
				my_error("out of memory", 0, -1);
			if ((ptr = env_get("EXTRA"))) {
				if (!stralloc_copyb(&buf, "userdb_uid userdb_gid ", 22) ||
						!stralloc_cats(&buf, ptr) || !stralloc_0(&buf))
				my_error("out of memory", 0, -1);
			} else
			if (!stralloc_copyb(&buf, "userdb_uid userdb_gid", 21) ||
					!stralloc_0(&buf))
				my_error("out of memory", 0, -1);
			if (!env_put2("EXTRA", buf.s))
				my_error("out of memory", 0, -1);
			execv(argv[1], argv + 1);
			my_error("execv", argv[1], -1);
			_exit (111);
		}
		_exit (0);
	}
	/*- authenticaion did not succeed */
	if (native_checkpassword)
		_exit (1);
	execv(argv[1], argv + 1);
	my_error("execv", argv[1], -1);
	/*- Not reached */
	_exit (111);
}

int
ldap_lookup(const char *login, const char *password, const char *error[], uid_t *userId, gid_t *groupId)
{
	char           *attrs[] = { NULL };
	const char     *host, *dn, *port, *ptr;
	const char     *ldap_host, *ldap_bind_dn, *ldap_bind_passwd = 0, *ldap_filter,
				   *ldap_base, *ldap_scope;
#ifdef EXTENDED_ATTRIBUTES
	const char     *ldap_field;
	char          **values;
#endif
	static stralloc filter = {0}, errbuf = {0};
	LDAP           *ld;
	LDAPMessage    *res, *res0;
	int             i, ret, ldap_port, scope, at;

	if (!(ldap_host = env_get("LDAP_HOST")))
		ldap_host = LDAP_HOST;
	if (!(port = env_get("LDAP_PORT")))
		ldap_port = LDAP_PORT;
	else
		scan_int(port, &ldap_port);
	if (!(ld = (LDAP *) ldap_init(ldap_host, ldap_port))) {
		if (error)
			*error = error_str(errno);
		return (-1);
	}
	if (login[at = str_rchr(login, '@')] == 0)
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
	ret = 3;
	if (ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ret) != LDAP_OPT_SUCCESS) {
		if (!stralloc_copyb(&errbuf, "ldap_set_option: LDAP_OPT_PROTOCOL_VERSION=3", 44) ||
				!stralloc_0(&errbuf)) {
			if (error)
				*error = "out of memory";
			return (-1);
		}
		if (error)
			*error = errbuf.s;
		return (-1);
	}
	switch ((ret = ldap_simple_bind_s(ld, ldap_bind_dn, ldap_bind_passwd)))
	{
	case LDAP_SUCCESS:
		break;
	case LDAP_INVALID_CREDENTIALS:
		if (error)
			*error = "invalid credentials for bind DN";
		ldap_unbind_s(ld);
		return (-1);
	default:
		if (!stralloc_copyb(&errbuf, "ldap_simple_bind_s: ", 20) ||
				!stralloc_cats(&errbuf, ldap_err2string(ret)) ||
				!stralloc_0(&errbuf)) {
			ldap_unbind_s(ld);
			if (error)
				*error = "out of memory";
			return (-1);
		}
		ldap_unbind_s(ld);
		if (error)
			*error = errbuf.s;
		return (-1);
	}
	if (!(ldap_filter = env_get("LDAP_FILTER")))
		ldap_filter = LDAP_FILTER;
	for (ptr = ldap_filter;*ptr;ptr++) {
		if (*ptr == '%' && *(ptr + 1)) {
			switch (*(ptr + 1))
			{
			case '%':
				if (!stralloc_catb(&filter, "%", 1)) {
					if (error)
						*error = "out of memory";
					return (-1);
				}
				ptr++;
				break;
			case 'e':
				if (!stralloc_cats(&filter, login)) {
					if (error)
						*error = "out of memory";
					return (-1);
				}
				ptr++;
				break;
			case 'u':
				if (!stralloc_catb(&filter, login, at)) {
					if (error)
						*error = "out of memory";
					return (-1);
				}
				ptr++;
				break;
			case 'h':
				if (!stralloc_cats(&filter, host)) {
					if (error)
						*error = "out of memory";
					return (-1);
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
		return (-1);
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
			return (-1);
		}
	}
	if (debug)
	{
		subprintf(subfdout, "ldap_search: base=[%s] filter=[%s]\n", ldap_base, filter.s);
		flush();
	}
	if ((ret = ldap_search_s(ld, ldap_base, scope, filter.s, attrs, 0, &res))) {
		if (!stralloc_copyb(&errbuf, "ldap_search_s: ", 15) ||
				!stralloc_cats(&errbuf, ldap_err2string(ret)) ||
				!stralloc_0(&errbuf)) {
			ldap_msgfree(res);
			ldap_unbind_s(ld);
			if (error)
				*error = "out of memory";
			return (-1);
		}
		ldap_msgfree(res);
		ldap_unbind_s(ld);
		if (error)
			*error = errbuf.s;
		return (-1);
	}
	if (!(res0 = ldap_first_entry(ld, res))) {
		ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &ret);
		if (!stralloc_copyb(&errbuf, "ldap_first_entry: ", 18) ||
				!stralloc_cats(&errbuf, ldap_err2string(ret)) ||
				!stralloc_0(&errbuf)) {
			ldap_msgfree(res);
			ldap_unbind_s(ld);
			if (error)
				*error = "out of memory";
			return (-1);
		}
		ldap_msgfree(res);
		ldap_unbind_s(ld);
		if (error)
			*error = errbuf.s;
		return (-1);
	}
	if (!(dn = ldap_get_dn(ld, res))) {
		ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &ret);
		if (!stralloc_copyb(&errbuf, "ldap_get_dn: ", 13) ||
				!stralloc_cats(&errbuf, ldap_err2string(ret)) ||
				!stralloc_0(&errbuf)) {
			ldap_msgfree(res);
			ldap_unbind_s(ld);
			if (error)
				*error = "out of memory";
			return (-1);
		}
		ldap_msgfree(res);
		ldap_unbind_s(ld);
		if (error)
			*error = errbuf.s;
		return (-1);
	}
	if (debug) {
		subprintf(subfdout, "ldap_get_dn: dn=[%s]\n", dn);
		flush();
	}
	if (debug) {
		subprintf(subfdout, "ldap_simple_bind: password=[%s]\n", password);
		flush();
	}
	/*- bind with the user dn to test the password */
	switch ((ret = ldap_simple_bind_s(ld, dn, password)))
	{
	case LDAP_SUCCESS:
		break;
	case LDAP_INVALID_CREDENTIALS:
		ldap_msgfree(res);
		ldap_unbind_s(ld);
		return (1);
	default:
		ldap_msgfree(res);
		if (!stralloc_copyb(&errbuf, "ldap_simple_bind_s: ", 20) ||
				!stralloc_cats(&errbuf, ldap_err2string(ret)) ||
				!stralloc_0(&errbuf)) {
			ldap_unbind_s(ld);
			if (error)
				*error = "out of memory";
			return (-1);
		}
		ldap_unbind_s(ld);
		if (error)
			*error = errbuf.s;
		return -1;
	}
	ldap_memfree((char *) dn);
#ifdef EXTENDED_ATTRIBUTES
	for (i = 0; environ[i]; ++i) {
		if (str_diffn(environ[i], "LDAP_FIELD_", 11))
			continue;
		for (ldap_field = environ[i] + 11;*ldap_field && *ldap_field != '=';ldap_field++);
		ldap_field++;
		if (!(values = (char **) ldap_get_values(ld, res0, ldap_field))) {
			if (error) {
				ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &ret);
				*error = ldap_err2string(ret);
			}
			ldap_msgfree(res);
			ldap_unbind_s(ld);
			return (-1);
		}
		if (!str_diffn(environ[i] + 11, "HOME", 4)) {
			if (values && values[0]) {
				if (!stralloc_copys(&homedir, values[0])) {
					ldap_value_free(values);
					ldap_msgfree(res);
					ldap_unbind_s(ld);
					return (-1);
				}
				if (!stralloc_0(&homedir)) {
					ldap_value_free(values);
					ldap_msgfree(res);
					ldap_unbind_s(ld);
					return (-1);
				}
			}
		} else
		if (!str_diffn(environ[i] + 11, "UID", 3)) {
			if (values && values[0])
				scan_ulong(values[0], (unsigned long *) userId);
		}
		if (!str_diffn(environ[i] + 11, "GID", 3)) {
			if (values && values[0])
				scan_ulong(values[0], (unsigned long *) groupId);
		}
		if (debug && values && values[0]) {
			subprintf(subfdout, "%s=[%s]\n", environ[i], values[0]);
			flush();
		}
		ldap_value_free(values);
	} /*- for (i = 0; environ[i]; ++i) */
#endif
	ldap_msgfree(res);
	ldap_unbind(ld);
	return (0);
}
#else
#warning "not compiled with -DHASLDAP or ldap libraries absent"
#include "substdio.h"
#include <unistd.h>

static char     sserrbuf[512];
struct substdio sserr;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	substdio_fdbuf(&sserr, write, 2, sserrbuf, sizeof(sserrbuf));
	substdio_puts(&sserr, "not compiled with -DHASDKIM or ldap libraries absent\n");
	substdio_flush(&sserr);
	_exit(111);
}
#endif

void
getversion_ldap_checkpwd_c()
{
	const char     *x = "$Id: ldap-checkpwd.c,v 1.14 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
