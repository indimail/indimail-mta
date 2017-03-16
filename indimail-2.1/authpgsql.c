/*
 * $Log: authpgsql.c,v $
 * Revision 2.2  2014-01-30 13:59:55+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 2.1  2013-05-15 01:09:57+05:30  Cprogrammer
 * pgsql authmodule
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef ENABLE_DOMAIN_LIMITS
#include <time.h>
#endif
#define _XOPEN_SOURCE
#include <unistd.h>
#include <errno.h>

#ifndef lint
static char     sccsid[] = "$Id: authpgsql.c,v 2.2 2014-01-30 13:59:55+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_PGSQL
#include <libpq-fe.h>			/* required pgsql front-end headers */

#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#endif

int             authlen = AUTH_SIZE;
PGconn         *pgc; /* pointer to pgsql connection */

struct passwd  *
pg_getpw(char *user, char *domain)
{
	static struct passwd   pwent;
	static char     IUser[MAX_BUFF], IPass[MAX_BUFF], IGecos[MAX_BUFF];
	static char     IDir[MAX_BUFF], IShell[MAX_BUFF];
	char            SqlBufRead[SQL_BUF_SIZE];
	char           *table_name, *select_stmt;
	PGresult       *pgres;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif

	lowerit(user);
	lowerit(domain);
	if (!(table_name = (char *) getenv("PG_TABLE_NAME")))
		table_name = "indimail";
	if (!(select_stmt = (char *) getenv("SELECT_STATEMENT")))
		select_stmt = "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, \
		pw_gecos, pw_dir, pw_shell from %s where pw_name = \"%s\" \
		and pw_domain = \"%s\"";
	snprintf(SqlBufRead, SQL_BUF_SIZE, select_stmt, table_name, user, domain);
	pgres = PQexec(pgc, SqlBufRead);
	if (!pgres || PQresultStatus(pgres) != PGRES_TUPLES_OK) {
		if (pgres)
			PQclear(pgres);
		return NULL;
	}
	if (PQntuples(pgres) <= 0) {	/* rows count */
		PQclear(pgres);
		return NULL;
	}
	pwent.pw_name = IUser;
	pwent.pw_passwd = IPass;
	pwent.pw_gecos = IGecos;
	pwent.pw_dir = IDir;
	pwent.pw_shell = IShell;

	strncpy(pwent.pw_name, PQgetvalue(pgres, 0, 0), MAX_BUFF);
	strncpy(pwent.pw_passwd, PQgetvalue(pgres, 0, 1), MAX_BUFF);
	pwent.pw_uid = atoi(PQgetvalue(pgres, 0, 2));
	pwent.pw_gid = atoi(PQgetvalue(pgres, 0, 3));
	strncpy(pwent.pw_gecos, PQgetvalue(pgres, 0, 4), MAX_BUFF);
	strncpy(pwent.pw_dir, PQgetvalue(pgres, 0, 5), MAX_BUFF);
	strncpy(pwent.pw_shell, PQgetvalue(pgres, 0, 6), MAX_BUFF);
#ifdef ENABLE_DOMAIN_LIMITS
	if (getenv("DOMAIN_LIMITS") && !(pwent.pw_gid & V_OVERRIDE))
	{
		if (!vget_limits(domain, &limits))
			pwent.pw_gid |= vlimits_get_flag_mask(&limits);
		else
			return ((struct passwd *) 0);
	}
#endif
	return (&pwent);
}

int
main(int argc, char **argv)
{
	char           *tmpbuf, *login, *ologin, *response, *challenge, *crypt_pass, *ptr, *cptr;
	char            user[AUTH_SIZE], fquser[AUTH_SIZE], domain[AUTH_SIZE], buf[MAX_BUFF];
	int             count, offset, norelay = 0, status, auth_method;
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	time_t          curtime;
	struct vlimits  limits;
#endif

	if (argc < 2)
		_exit(2);
	if (!(tmpbuf = calloc(1, (authlen + 1) * sizeof (char)))) {
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		fprintf(stderr, "malloc-%d: %s\n", authlen + 1, strerror(errno));
		_exit(111);
	}
	for (offset = 0;;) {
		do {
			count = read(3, tmpbuf + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while (count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while (count == -1 && errno == EINTR);
#endif
		if (count == -1) {
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			fprintf(stderr, "read: %s\n", strerror(errno));
			_exit(111);
		} else if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
			_exit(2);
	}
	count = 0;
	login = tmpbuf + count;	/*- username */
	for (; tmpbuf[count] && count < offset; count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);

	count++;
	challenge = tmpbuf + count;	/*- challenge (or plain text) */
	for (; tmpbuf[count] && count < offset; count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);

	count++;
	response = tmpbuf + count; /*- response (cram-md5, cram-sha1, etc) */
	for (; tmpbuf[count] && count < offset; count++);
	if (count == offset || (count + 1) == offset)
		auth_method = 0;
	else
		auth_method = tmpbuf[count + 1];

	for (cptr = user, ptr = login; *ptr && *ptr != '@'; *cptr++ = *ptr++);
	*cptr = 0;
	ologin = login;
	if (*ptr) {
		ptr++;
		for (cptr = domain; *ptr; *cptr++ = *ptr++);
		*cptr = 0;
	} else {
/*- no @ in the login */

		if (auth_method == AUTH_DIGEST_MD5) { /*- for handling dumb programs like
												outlook written by dumb programmers */
			for (cptr = buf, ptr = response; *ptr; *cptr++ = *ptr++) {
				if (*ptr == '\n') {
					*cptr = 0;
					ptr++;
					if (!strncmp("realm=", buf, 6)) {
						ptr = buf + 6;
						for (cptr = domain; *ptr; ptr++) {
							if (isspace(*ptr) || *ptr == '\"')
								continue;
							*cptr++ = *ptr;
						}
						*cptr = 0;
						break;
					} else
						cptr = buf;
				}
			}
			*cptr = 0;
			if (!strncmp("realm=", buf, 6)) {
				ptr = buf + 6;
				for (cptr = domain; *ptr && *ptr != ','; ptr++) {
					if (isspace(*ptr) || *ptr == '\"')
						continue;
					*cptr++ = *ptr;
				}
				*cptr = 0;
			}
		}
		if (!*domain && parse_email(login, user, domain, AUTH_SIZE)) {
			getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
			for (cptr = domain; *ptr; *cptr++ = *ptr++);
			*cptr = 0;
		}
		snprintf(fquser, AUTH_SIZE, "%s@%s", user, domain);
		login = fquser;
	}
	pgc = PQconnectdb("user=postgress,database=indimail");
	if (PQstatus(pgc) == CONNECTION_BAD) {
		printf("454-failed to connect to database (%s) (#4.3.0)\r\n", PQerrorMessage(pgc));
		fflush(stdout);
		_exit(111);
	}
	pw = pg_getpw(user, domain);
    PQfinish(pgc);
	if (!pw) {
		if (userNotFound)
			pipe_exec(argv, tmpbuf, offset);
		else
			fprintf(stderr, "vchkpass: inquery: %s\n", strerror(errno));
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit(111);
	} else if (pw->pw_gid & NO_SMTP) {
		printf("553-Sorry, this account cannot use SMTP (#5.7.1)\r\n");
		fflush(stdout);
		_exit(1);
	} else if (is_inactive && !getenv("ALLOW_INACTIVE")) {
		printf("553-Sorry, this account is inactive (#5.7.1)\r\n");
		fflush(stdout);
		_exit(1);
	} else if (pw->pw_gid & NO_RELAY)
		norelay = 1;
	crypt_pass = pw->pw_passwd;
	if (getenv("DEBUG")) {
		fprintf(stderr, "%s: login [%s] challenge [%s] response [%s] pw_passwd [%s] method[%d]\n", argv[0], login, challenge,
				response, crypt_pass, auth_method);
	}
	if (pw_comp
		((unsigned char *) ologin, (unsigned char *) crypt_pass, (unsigned char *) (*response ? challenge : 0),
		 (unsigned char *) (*response ? response : challenge), auth_method)) {
		pipe_exec(argv, tmpbuf, offset);
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit(111);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (getenv("DOMAIN_LIMITS")) {
		struct vlimits *lmt;
#ifdef QUERY_CACHE
		if (!getenv("QUERY_CACHE")) {
			if (vget_limits(domain, &limits)) {
				fprintf(stderr, "vchkpass: unable to get domain limits for for %s\n", domain);
				printf("454-unable to get domain limits for %s\r\n", domain);
				fflush(stdout);
				_exit(111);
			}
			lmt = &limits;
		} else
			lmt = inquery(LIMIT_QUERY, login, 0);
#else
		if (vget_limits(domain, &limits)) {
			fprintf(stderr, "vchkpass: unable to get domain limits for for %s\n", domain);
			printf("454-unable to get domain limits for %s\r\n", domain);
			fflush(stdout);
			_exit(111);
		}
		lmt = &limits;
#endif
		curtime = time(0);
		if (lmt->domain_expiry > -1 && curtime > lmt->domain_expiry) {
			printf("553-Sorry, your domain has expired (#5.7.1)\r\n");
			fflush(stdout);
			_exit(1);
		} else if (lmt->passwd_expiry > -1 && curtime > lmt->passwd_expiry) {
			printf("553-Sorry, your password has expired (#5.7.1)\r\n");
			fflush(stdout);
			_exit(1);
		}
	}
#endif
	status = 0;
	if ((ptr = (char *) getenv("POSTAUTH")) && !access(ptr, X_OK)) {
		snprintf(buf, MAX_BUFF, "%s %s", ptr, login);
		status = runcmmd(buf, 0);
	}
	_exit(norelay ? 3 : status);
	/*- Not reached */
	return (0);
}

#else
#include <string.h>
#warning "not compiled with -DHAVE_PGSQL"

int
main(int argc, char **argv)
{
	execvp(argv[1], argv + 1);
	printf("454-%s (#4.3.0)\r\n", strerror(errno));
	fflush(stdout);
	fprintf(stderr, "execvp: %s: %s\n", argv[1], strerror(errno));
	_exit(111);
}
#endif

void
getversion_authpgsql_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
