/*
 * $Log: vtable.c,v $
 * Revision 2.3  2011-07-29 09:26:43+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 2.2  2011-05-21 19:20:49+05:30  Cprogrammer
 * skip comments
 *
 * Revision 2.1  2011-05-20 21:16:54+05:30  Cprogrammer
 * create MySQL table from a template
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <mysql.h>

#ifndef	lint
static char     sccsid[] = "$Id: vtable.c,v 2.3 2011-07-29 09:26:43+05:30 Cprogrammer Stab mbhangui $";
#endif

int             verbose;

#ifdef HAVE_STDARG_H
#include <stdarg.h>
char           *
mysql_stack(const char *fmt, ...)
#else
#include <varargs.h>
char           *
mysql_stack(va_alist)
va_dcl
#endif
{
#ifndef HAVE_STDARG_H
	char           *fmt;
#endif
	va_list         ap;
	int             len, i;
	static int      mylen;
	static char    *mysqlQueryStr;
	char           *ptr, *mysqlstr;

#ifndef HAVE_STDARG_H
	va_start(ap);
	fmt = va_arg(ap, char *);
#endif
	if (fmt && *fmt) {
#ifdef HAVE_STDARG_H
		va_start(ap, fmt);
#endif
		if (vasprintf(&mysqlstr, fmt, ap) == -1) {
			fprintf(stderr, "mysql_stack: vasprintf: %s\n", strerror(errno));
			return ((char *) 0);
		}
		va_end(ap);
		len = strlen(mysqlstr) + 1;
		if (!(mysqlQueryStr = realloc(mysqlQueryStr, mylen + len + 1))) {	/*- The man page is wierd on Mac OS */
			fprintf(stderr, "%s", mysqlstr);
			free(mysqlstr);
			fprintf(stderr, "mysql_stack: realloc: %s\n", strerror(errno));
			fflush(stderr);
			return ((char *) 0);
		}
		strncpy(mysqlQueryStr + mylen, mysqlstr, len);
		free(mysqlstr);
		mysqlQueryStr[mylen + len - 1] = 0;
		mylen += len;
		return (mysqlQueryStr);
	} else {
		if (!mysqlQueryStr)
			return ((char *) 0);
		for (ptr = mysqlQueryStr, i = len = 0; len < mylen; len++, ptr++) {
			if (*ptr == 0) {
				/*- execute mysql here */
				*ptr = ' ';
				i = len + 1;
				if (verbose)
					fprintf(stderr, "%s", mysqlQueryStr + i);
			}
		}
		mysqlQueryStr[mylen - 3] = ';';
		mysqlQueryStr[mylen - 1] = 0;
		mylen = 0;
		ptr = mysqlQueryStr;
		mysqlQueryStr = (char *) 0;
		return (ptr);
	}
}

void
execute_stack(void)
{
	mysql_stack(0);
	return;
}

static void
usage()
{
	fprintf(stderr, "usage: vtable [options] vtable_file\n");
	fprintf(stderr, "options: -v verbose\n");
	fprintf(stderr, "         -S MySQL Server IP\n");
	fprintf(stderr, "         -p MySQL Port\n");
	fprintf(stderr, "         -s MySQL socket\n");
	fprintf(stderr, "         -D MySQL Database Name\n");
	fprintf(stderr, "         -U MySQL User Name\n");
	fprintf(stderr, "         -P MySQL Password\n");
	return;
}

static int
get_options(int argc, char **argv, char **mysql_server, char **mysql_socket, char **mysql_port,
		char **mysql_database, char **mysql_user, char **mysql_pass, char ***filename)
{
	int             c;

	*mysql_port = *mysql_server = *mysql_database = *mysql_socket = *mysql_user = *mysql_pass = 0;
	while ((c = getopt(argc, argv, "vS:p:s:D:U:P:")) != -1) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'S':
			*mysql_server = optarg;
			break;
		case 'p':
			*mysql_port = optarg;
			break;
		case 's':
			*mysql_socket = optarg;
			break;
		case 'D':
			*mysql_database = optarg;
			break;
		case 'U':
			*mysql_user = optarg;
			break;
		case 'P':
			*mysql_pass = optarg;
			break;
		default:
			usage();
			return (1);
		} /*- switch(c) */
	} /*- while ((c = getopt(argc, argv, "vS:p:s:D:U:P:")) != -1) */
	if (optind < argc)
		*filename = argv + optind;
	else {
		usage();
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	char           *mysql_host, *mysql_db, *port, *mysql_sock, *mysql_user, *mysql_pass, *ptr;
	char          **filename, **fptr;
	char            buffer[2048];
	FILE           *fp;
	int             mysql_port, errors;
	MYSQL           mysql;

	if (get_options(argc, argv, &mysql_host, &mysql_sock, &port, &mysql_db,
		&mysql_user, &mysql_pass, &filename))
		return (1);
	if (verbose)
		printf("connecting to mysql %s:%s db %s\n", mysql_host,
			port ? port : mysql_sock, mysql_db);
	mysql_init(&mysql);
	mysql_port = port ? atoi(port) : 0;
	if (!(mysql_real_connect(&mysql, mysql_host, mysql_user, mysql_pass,
		mysql_db, mysql_port, mysql_sock, 0))) {
		fprintf(stderr, "mysql_real_connect: %s: %s\n", mysql_host, mysql_error(&mysql));
		return (1);
	}
	for (errors = 0, fptr = filename;*fptr;fptr++) {
		if (verbose)
			printf("processing %s\n", *fptr);
		if (!(fp = fopen(*fptr, "r")))
		{
			perror(*fptr);
			errors++;
			continue;
		}
		mysql_stack("create table IF NOT EXISTS ");
		for(;;) {
			if (!fgets(buffer, sizeof(buffer) - 2, fp))
				break;
			if ((ptr = strchr(buffer, '#')) || (ptr = strrchr(buffer, '\n')))
				*ptr = '\0';
			for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
			if (!*ptr)
				continue;
			mysql_stack(buffer);
			mysql_stack(" ");
		}
		if (!(ptr = mysql_stack(0))) {
			fprintf(stderr, "mysql_stack returned NULL\n");
		} else {
			if (verbose)
				printf("%s\n", ptr);
			if (mysql_query(&mysql, ptr))
				fprintf(stderr, "mysql_query: %s: %s\n", ptr, mysql_error(&mysql));
			free((void *) ptr);
		}
		fclose(fp);
	}
	mysql_close(&mysql);
	return (errors);
}

void
getversion_vtable_c()
{
	printf("%s\n", sccsid);
}
