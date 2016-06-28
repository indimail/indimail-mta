/*
 * $Log: authindi.c,v $
 * Revision 2.25  2016-05-25 08:58:10+05:30  Cprogrammer
 * use LIBEXECDIR for authmodule directory
 *
 * Revision 2.24  2014-02-05 01:49:18+05:30  Cprogrammer
 * changes to error messages if inquery/vauth_open fails
 *
 * Revision 2.23  2013-09-04 12:49:37+05:30  Cprogrammer
 * added cases for cram_sha256, cram_sha512 and cram_ripemd
 *
 * Revision 2.22  2011-11-19 21:58:29+05:30  Cprogrammer
 * fix imapd getting executed for pop3 service on auth failure
 * use auth_method variable for cram-md5, cram-sha1
 *
 * Revision 2.21  2011-10-30 09:01:41+05:30  Cprogrammer
 * free base64 decoded strings
 *
 * Revision 2.20  2011-10-28 14:14:45+05:30  Cprogrammer
 * send the auth method
 *
 * Revision 2.19  2011-10-25 20:46:47+05:30  Cprogrammer
 * added cram-md5 authentication
 *
 * Revision 2.18  2010-05-05 14:47:24+05:30  Cprogrammer
 * no need of using service:port format for service variable
 *
 * Revision 2.17  2010-04-12 12:59:18+05:30  Cprogrammer
 * use domain limits for account/password expiry
 * use inquery() for domain limits if QUERY_CACHE is defined
 *
 * Revision 2.16  2010-04-09 13:59:08+05:30  Cprogrammer
 * added env variable AUTHSERVICE to denote imap, pop3 or webmail
 *
 * Revision 2.15  2010-03-07 09:28:06+05:30  Cprogrammer
 * check return value of is_distributed_domain for error
 *
 * Revision 2.14  2010-02-28 13:26:48+05:30  Cprogrammer
 * use value of DEBUG_LOGIN
 *
 * Revision 2.13  2009-11-18 14:21:31+05:30  Cprogrammer
 * more readable code
 *
 * Revision 2.12  2009-10-14 20:41:24+05:30  Cprogrammer
 * check parse_quota for return status
 *
 * Revision 2.11  2009-02-18 21:24:00+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 2.10  2009-02-06 11:36:22+05:30  Cprogrammer
 * ignore return values of write on stderr
 *
 * Revision 2.9  2008-12-19 11:51:59+05:30  Cprogrammer
 * changed PASSWD_CACHE to QUERY_CACHE
 * restrict authentication to local users
 *
 * Revision 2.8  2008-12-16 18:35:37+05:30  Cprogrammer
 * restrict authentication to local users only for distributed domains
 *
 * Revision 2.7  2008-11-21 11:59:32+05:30  Cprogrammer
 * added environment variable MAILDIR
 *
 * Revision 2.6  2008-11-20 22:15:45+05:30  Cprogrammer
 * exit if EXIT_ONERROR is defined and POSTAUTH returns exit code 3
 *
 * Revision 2.5  2008-11-13 19:22:22+05:30  Cprogrammer
 * removed extra close_connection()
 *
 * Revision 2.4  2008-11-06 15:02:01+05:30  Cprogrammer
 * changed PASSWD_CACHE to QUERY_CACHE
 *
 * Revision 2.3  2008-08-28 21:52:50+05:30  Cprogrammer
 * close mysql connection on exit
 *
 * Revision 2.2  2008-08-24 17:43:44+05:30  Cprogrammer
 * added code to return error for password changes
 *
 * Revision 2.1  2008-08-24 14:44:56+05:30  Cprogrammer
 * courier-imap authmodule for IndiMail
 *
 */
#include "indimail.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

#ifndef lint
static char     sccsid[] = "$Id: authindi.c,v 2.25 2016-05-25 08:58:10+05:30 Cprogrammer Exp mbhangui $";
#endif
#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#endif

int             authlen = AUTH_SIZE;
static int      exec_local(char **, char *, char *, struct passwd *, char *);

void
close_connection()
{
#ifdef QUERY_CACHE
	if (!getenv("QUERY_CACHE"))
		vclose();
#else /*- Not QUERY_CACHE */
	vclose();
#endif
}

static unsigned char encoding_table[] = { 
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

static char    *decoding_table = NULL;

void
build_decoding_table()
{
	int i;

	decoding_table = (char *) malloc(256);
	for (i = 0; i < 0x40; i++)
		decoding_table[encoding_table[i]] = i;
}

static char    *
b64_decode(const unsigned char *data, size_t input_length, size_t * output_length)
{
	int i, j;

	if (decoding_table == NULL)
		build_decoding_table();

	if (input_length % 4 != 0)
		return NULL;

	*output_length = input_length / 4 * 3;
	if (data[input_length - 1] == '=')
		(*output_length)--;
	if (data[input_length - 2] == '=')
		(*output_length)--;

	char           *decoded_data = malloc(*output_length);
	if (decoded_data == NULL)
		return NULL;

	for (i = 0, j = 0; i < input_length;) {

		uint32_t        sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t        sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t        sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
		uint32_t        sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

		uint32_t        triple = (sextet_a << 3 * 6)
			+ (sextet_b << 2 * 6)
			+ (sextet_c << 1 * 6)
			+ (sextet_d << 0 * 6);

		if (j < *output_length)
			decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < *output_length)
			decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < *output_length)
			decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
	}
	return decoded_data;
}

void
base64_cleanup()
{
	free(decoding_table);
}

int
main(int argc, char **argv)
{
	char           *buf, *tmpbuf, *login, *challenge, *response, *crypt_pass, *ptr,
				   *real_domain, *prog_name, *service, *auth_type, *auth_data;
	char            user[AUTH_SIZE], domain[AUTH_SIZE], Email[MAX_BUFF];
	int             count, offset, auth_method;
	size_t          cram_md5_len, out_len;
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;
	char           *(imapargs[]) = { INDIMAILDIR"/sbin/imaplogin", LIBEXECDIR"/authlib/authindi",
					INDIMAILDIR"/bin/imapd", "Maildir", 0 };
	char           *(pop3args[]) = { INDIMAILDIR"/sbin/pop3login", LIBEXECDIR"/authlib/authindi",
					INDIMAILDIR"/bin/pop3d", "Maildir", 0 };
#ifdef ENABLE_DOMAIN_LIMITS
	time_t          curtime;
	struct vlimits  limits;
#endif

	if ((prog_name = strrchr(argv[0], '/')))
		prog_name++;
	else
		prog_name = argv[0];
	if (argc < 3)
	{
		fprintf(stderr, "%s: no more modules will be tried\n", prog_name);
		return (1);
	}
	if (!(tmpbuf = calloc(1, (authlen + 1) * sizeof(char))))
	{
		fprintf(stderr, "%s: malloc-%d: %s\n", prog_name, authlen + 1, strerror(errno));
		return (1);
	}
	/*-
	 * Courier-IMAP authmodules Protocol (authindi /var/indimail/bin/imapd Maildir < /tmp/input 3<&0)
	 * imap\n
	 * login\n
	 * postmaster@test.com\n ---> username or challenge
	 * pass\n                ---> plain text / response
	 * newpass\n             ---> auth_data
	 * argv[0]=/var/indimail/libexec/authlib/try
	 * argv[1]=/var/indimail/libexec/authlib/authpam
	 * argv[2]=/var/indimail/bin/imapd
	 * argv[3]=Maildir
	 */
	for (offset = 0;;)
	{
		do
		{
			count = read(3, tmpbuf + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while (count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while (count == -1 && errno == EINTR);
#endif
		if (count == -1)
		{
			fprintf(stderr, "read: %s\n", strerror(errno));
			return (1);
		} else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
		{
			fprintf(stderr, "%s: auth data too long\n", prog_name);
			return (2);
		}
	}
	if (!(buf = calloc(1, (offset + 1) * sizeof(char))))
	{
		fprintf(stderr, "%s: malloc-%d: %s\n", prog_name, authlen + 1, strerror(errno));
		return (1);
	}
	memcpy(buf, tmpbuf, offset);
	count = 0;
	service = tmpbuf + count; /*- service */
	for (;tmpbuf[count] != '\n' && count < offset;count++);
	if (count == offset || (count + 1) == offset)
	{
		fprintf(stderr, "%s: auth data too short\n", prog_name);
		return (2);
	}
	tmpbuf[count++] = 0;

	auth_type = tmpbuf + count; /* type (login, plain, cram-md5, cram-sha1 or pass) */
	for (;tmpbuf[count] != '\n' && count < offset;count++);
	if (count == offset || (count + 1) == offset)
	{
		fprintf(stderr, "%s: auth data too short\n", prog_name);
		return (2);
	}
	tmpbuf[count++] = 0;
	if (!strncmp(auth_type, "pass", 5))
		auth_method = -1;
	else
	if (!strncmp(auth_type, "login", 6))
		auth_method = AUTH_LOGIN;
	else
	if (!strncmp(auth_type, "plain", 6))
		auth_method = AUTH_PLAIN;
	else
	if (!strncmp(auth_type, "cram-md5", 9))
		auth_method = AUTH_CRAM_MD5;
	else
	if (!strncmp(auth_type, "cram-sha1", 10))
		auth_method = AUTH_CRAM_SHA1;
	else
	if (!strncmp(auth_type, "cram-sha256", 12))
		auth_method = AUTH_CRAM_SHA256;
	else
	if (!strncmp(auth_type, "cram-sha512", 12))
		auth_method = AUTH_CRAM_SHA512;
	else
	if (!strncmp(auth_type, "cram-ripemd", 12))
		auth_method = AUTH_CRAM_RIPEMD;
	else
		auth_method = 0;

	login = tmpbuf + count; /*- username or challenge */
	for (cram_md5_len = 0;tmpbuf[count] != '\n' && count < offset;count++, cram_md5_len++);
	if (count == offset || (count + 1) == offset)
	{
		fprintf(stderr, "%s: auth data too short\n", prog_name);
		return (2);
	}
	tmpbuf[count++] = 0;
	if (auth_method > 2) {
		if (!(ptr = b64_decode((unsigned char *) login, cram_md5_len, &out_len)))
		{
			fprintf(stderr, "b64_decode: %s\n", strerror(errno));
			pipe_exec(argv, buf, offset);
			return (1);
		}
		challenge = ptr;
	} else
		challenge = 0;

	auth_data = tmpbuf + count; /*- (plain text password or cram-md5 response) */
	for (cram_md5_len = 0;tmpbuf[count] != '\n' && count < offset;count++, cram_md5_len++);
	tmpbuf[count++] = 0;
	if (auth_method > 2) {
		if (!(ptr = b64_decode((unsigned char *) auth_data, cram_md5_len, &out_len)))
		{
			if (challenge)
				free (challenge);
			fprintf(stderr, "b64_decode: %s\n", strerror(errno));
			pipe_exec(argv, buf, offset);
			return (1);
		}
		for (login = ptr;*ptr && !isspace(*ptr);ptr++);
		*ptr = 0;
		response = ptr + 1;
	} else
		response = 0;
	if (!*auth_data) {
		auth_data = tmpbuf + count; /* in case of auth login, auth plain */
		for (;tmpbuf[count] != '\n' && count < offset;count++);
		tmpbuf[count++] = 0;
	}
	if (!strncmp(auth_type, "pass", 5))
	{
		fprintf(stderr, "%s: Password Change not supported\n", prog_name);
		if (auth_method > 2)
		{
			if (challenge)
				free (challenge);
			free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	}
	if (parse_email(login, user, domain, MAX_BUFF))
	{
		fprintf(stderr, "%s: could not parse email [%s]\n", prog_name, login);
		if (auth_method > 2)
		{
			if (challenge)
				free (challenge);
			free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	}
	if (!vget_assign(domain, 0, 0, &uid, &gid)) 
	{
		fprintf(stderr, "%s: domain %s does not exist\n", prog_name, domain);
		if (auth_method > 2)
		{
			if (challenge)
				free (challenge);
			free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	}
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	snprintf(Email, MAX_BUFF, "%s@%s", user, real_domain);
#ifdef CLUSTERED_SITE
	if ((count = is_distributed_domain(real_domain)) == -1)
	{
		fprintf(stderr, "%s: is_distributed_domain failed\n", real_domain);
		if (auth_method > 2)
		{
			if (challenge)
				free (challenge);
			free (login);
		}
		return (1);
	} else
	if (count)
	{
		char           *mailstore, *ptr;

#ifdef QUERY_CACHE
		if (getenv("QUERY_CACHE"))
			mailstore = inquery(HOST_QUERY, Email, 0);
		else
			mailstore = findhost(Email, 2);
#else
		mailstore = findhost(Email, 2);
#endif
		if (!mailstore)
		{
			if (!userNotFound)
				fprintf(stderr, "No mailstore for %s\n", Email);
			if (auth_method > 2)
			{
				if (challenge)
					free (challenge);
				free (login);
			}
			pipe_exec(argv, buf, offset);
			return (1);
		}
		if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
			*ptr = 0;
		for (;*mailstore && *mailstore != ':';mailstore++);
		mailstore++;
		if (!islocalif(mailstore))
		{
			fprintf(stderr, "%s not on local (mailstore %s)\n", Email, mailstore);
			if (auth_method > 2)
			{
				if (challenge)
					free (challenge);
				free (login);
			}
			return (1);
		}
	}
#endif /*- CLUSTERED_SITE */
#ifdef QUERY_CACHE
	if (getenv("QUERY_CACHE"))
		pw = inquery(PWD_QUERY, Email, 0);
	else
	{
		if (vauth_open((char *) 0))
		{
			if(!userNotFound)
				fprintf(stderr, "%s: %s: %s: %s\n", prog_name, Email, 
					getenv("QUERY_CACHE") ? "inquery" : "vauth_open", errno ? strerror(errno) : "AUTHFAILURE");
			if (auth_method > 2)
			{
				if (challenge)
					free (challenge);
				free (login);
			}
			pipe_exec(argv, buf, offset);
			return (1);
		}
		pw = vauth_getpw(user, real_domain);
	}
#else
	if (vauth_open((char *) 0))
	{
		if(!userNotFound)
			fprintf(stderr, "%s: vauth_open: %s\n", prog_name, errno ? strerror(errno) : "AUTHFAILURE");
		if (auth_method > 2)
		{
			if (challenge)
				free (challenge);
			free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	}
	pw = vauth_getpw(user, real_domain);
#endif
	if (!pw)
	{
		if(!userNotFound)
#ifdef QUERY_CACHE
			fprintf(stderr, "%s: %s: %s: %s\n", prog_name, Email, 
				getenv("QUERY_CACHE") ? "inquery" : "vauth_open", errno ? strerror(errno) : "AUTHFAILURE");
#else
			fprintf(stderr, "%s: vauth_open: %s@%s: %s\n", prog_name, user, real_domain,
				errno ? strerror(errno) : "");
#endif
		if (auth_method > 2)
		{
			if (challenge)
				free (challenge);
			free (login);
		}
		pipe_exec(argv, buf, offset);
		close_connection();
		return (1);
	}
	/*
	 * Look at what type of connection we are trying to auth.
	 * And then see if the user is permitted to make this type
	 * of connection
	 */
	if (strcmp("webmail", service) == 0)
	{
		if (pw->pw_gid & NO_WEBMAIL)
		{
			fprintf(stderr, "%s: webmail disabled for this account", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			if (auth_method > 2)
			{
				if (challenge)
					free (challenge);
				free (login);
			}
			execv(!strcmp("pop3", service) ? *pop3args : *imapargs, argv);
			fprintf(stderr, "execv %s: %s", !strcmp("pop3", service) ? *pop3args : *imapargs,
				strerror(errno));
			return (1);
		}
	} else
	if (strcmp("pop3", service) == 0)
	{
		if (pw->pw_gid & NO_POP)
		{
			fprintf(stderr, "%s: pop3 disabled for this account", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			if (auth_method > 2)
			{
				if (challenge)
					free (challenge);
				free (login);
			}
			execv(!strcmp("pop3", service) ? *pop3args : *imapargs, argv);
			fprintf(stderr, "execv %s: %s", !strcmp("pop3", service) ? *pop3args : *imapargs,
				strerror(errno));
			return (1);
		}
	} else
	if (strcmp("imap", service) == 0)
	{
		if (pw->pw_gid & NO_IMAP)
		{
			fprintf(stderr, "%s: imap disabled for this account", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			if (auth_method > 2)
			{
				if (challenge)
					free (challenge);
				free (login);
			}
			execv(!strcmp("pop3", service) ? *pop3args : *imapargs, argv);
			fprintf(stderr, "execv %s: %s", !strcmp("pop3", service) ? *pop3args : *imapargs,
				strerror(errno));
			return (1);
		}
	}
	crypt_pass = pw->pw_passwd;
	if ((ptr = getenv("DEBUG_LOGIN")) && *ptr > '0')
	{
		if (response)
			fprintf(stderr, "service[%s] authmeth [%d] login [%s] challenge [%s] response [%s] pw_passwd [%s]\n", 
				service, auth_method, login, challenge, response, crypt_pass);
		else
			fprintf(stderr, "service[%s] authmeth [%d] login [%s] auth [%s] pw_passwd [%s]\n", 
				service, auth_method, login, auth_data, crypt_pass);
	}
	if (pw_comp((unsigned char *) login, (unsigned char *) crypt_pass,
		(unsigned char *) (auth_method > 2 ? challenge : 0),
		(unsigned char *) (auth_method > 2 ? response : auth_data), auth_method))
	{
		if (argc == 3)
		{
			fprintf(stderr, "%s: no more modules will be tried\n", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			execv(!strcmp("pop3", service) ? *pop3args : *imapargs, argv);
			fprintf(stderr, "execv %s: %s", !strcmp("pop3", service) ? *pop3args : *imapargs,
				strerror(errno));
			return (1);
		}
		close_connection();
		if (auth_method > 2)
		{
			if (challenge)
				free (challenge);
			free (login);
		}
		pipe_exec(argv, buf, offset);
		return (1);
	}
	if (auth_method > 2)
	{
		if (challenge)
			free (challenge);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (getenv("DOMAIN_LIMITS"))
	{
		struct vlimits *lmt;
#ifdef QUERY_CACHE
		if (!getenv("QUERY_CACHE"))
		{
			if (vget_limits(real_domain, &limits))
			{
				fprintf(stderr, "%s: unable to get domain limits for for %s\n", prog_name, real_domain);
				close_connection();
				if (auth_method > 2)
					free (login);
				pipe_exec(argv, buf, offset);
				return (1);
			}
			lmt = &limits;
		} else
			lmt = inquery(LIMIT_QUERY, login, 0);
#else
		if (vget_limits(real_domain, &limits))
		{
			fprintf(stderr, "%s: unable to get domain limits for for %s\n", prog_name, real_domain);
			close_connection();
			if (auth_method > 2)
				free (login);
			pipe_exec(argv, buf, offset);
			return (1);
		}
		lmt = &limits;
#endif
		curtime = time(0);
		if (lmt->domain_expiry > -1 && curtime > lmt->domain_expiry)
		{
			fprintf(stderr, "%s: Sorry, your domain has expired\n", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			if (auth_method > 2)
				free (login);
			execv(!strcmp("pop3", service) ? *pop3args : *imapargs, argv);
			fprintf(stderr, "execv %s: %s", !strcmp("pop3", service) ? *pop3args : *imapargs,
				strerror(errno));
			return (1);
		} else
		if (lmt->passwd_expiry > -1 && curtime > lmt->passwd_expiry)
		{
			fprintf(stderr, "%s: Sorry, your password has expired\n", prog_name);
			if (write(2, "AUTHFAILURE\n", 12) == -1) ;
			close_connection();
			if (auth_method > 2)
				free (login);
			execv(!strcmp("pop3", service) ? *pop3args : *imapargs, argv);
			fprintf(stderr, "execv %s: %s", !strcmp("pop3", service) ? *pop3args : *imapargs,
				strerror(errno));
		} 
	}
#endif
	exec_local(argv + argc - 2, login, real_domain, pw, service);
	if (auth_method > 2)
		free (login);
	return (0);
}

static int
exec_local(char **argv, char *userid, char *TheDomain, struct passwd *pw, char *service)
{
	char            Maildir[MAX_BUFF], authenv1[MAX_BUFF], authenv2[MAX_BUFF],
					authenv3[MAX_BUFF], authenv4[MAX_BUFF], authenv5[MAX_BUFF],
					authenv6[MAX_BUFF], authenv7[MAX_BUFF], TheUser[MAX_BUFF],
					TmpBuf[MAX_BUFF];
	char           *ptr, *cptr;
	int             status;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit;
#endif

	for (cptr = TheUser, ptr = userid;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (Check_Login(service, TheDomain, pw->pw_gecos))
	{
		fprintf(stderr, "Login not permitted for %s\n", service);
		close_connection();
		return (1);
	}
	snprintf(authenv1, MAX_BUFF, "AUTHENTICATED=%s", userid);
	snprintf(authenv2, MAX_BUFF, "AUTHADDR=%s@%s", TheUser, TheDomain);
	snprintf(authenv3, MAX_BUFF, "AUTHFULLNAME=%s", pw->pw_gecos);
#ifdef USE_MAILDIRQUOTA	
	if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1)
	{
		fprintf(stderr, "parse_quota: %s: %s\n", pw->pw_shell, strerror(errno));
		close_connection();
		return (1);
	}
	snprintf(authenv4, MAX_BUFF, "MAILDIRQUOTA=%"PRIu64"S,%"PRIu64"C", size_limit, count_limit);
#else
	snprintf(authenv4, MAX_BUFF, "MAILDIRQUOTA=%sS", pw->pw_shell);
#endif
	snprintf(authenv5, MAX_BUFF, "HOME=%s", pw->pw_dir);
	snprintf(authenv6, MAX_BUFF, "AUTHSERVICE=%s", service);
	putenv(authenv1);
	putenv(authenv2);
	putenv(authenv3);
	putenv(authenv4);
	putenv(authenv5);
	putenv(authenv6);
	switch ((status = Login_Tasks(pw, userid, service)))
	{
		case 2:
			if (!(ptr = getenv("TMP_MAILDIR")))
				snprintf(Maildir, MAX_BUFF, "%s", pw->pw_dir);
			else
				snprintf(Maildir, MAX_BUFF, "%s", ptr);
			break;
		case 3:
			if ((ptr = getenv("EXIT_ONERROR")))
			{
				if ((ptr = getenv("MSG_ONERROR")) && !access(ptr, F_OK))
				{
					FILE *fp;

					if ((fp = fopen(ptr, "r")))
					{
						for (;;)
						{
							if (!fgets(TmpBuf, sizeof(TmpBuf) - 2, fp))
								break;
							fprintf(stdout, "%s", TmpBuf);
						}
						fflush(stdout);
						fclose(fp);
						/*- Flow through */
					} else
						fprintf(stderr, "%s: %s\n", ptr, strerror(errno));
				}
				fprintf(stderr, "POSTAUTH: Error on Exit\n");
				close_connection();
				return (1);
			}
			if (!(ptr = getenv("TMP_MAILDIR")))
				snprintf(Maildir, MAX_BUFF, "%s", pw->pw_dir);
			else
				snprintf(Maildir, MAX_BUFF, "%s", ptr);
			break;
		default:
			snprintf(Maildir, MAX_BUFF, "%s", pw->pw_dir);
			break;
	}
	close_connection();
	if (chdir(Maildir))
	{
		fprintf(stderr, "authindi: chdir: %s: %s\n", Maildir, strerror(errno));
		return (1);
	}
	snprintf(authenv7, MAX_BUFF, "MAILDIR=%s/Maildir", Maildir);
	putenv(authenv7);
	execv(argv[0], argv);
	return (1);
}

void
getversion_authindi_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
