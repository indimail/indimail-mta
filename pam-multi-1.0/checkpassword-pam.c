/*
 * $Log: $
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include "indimail.h"
#include "pam-support.h"

#ifndef MAX_BUFF
#define MAX_BUFF 512
#endif

#ifndef lint
static char     sccsid[] = "$Id: $";
#endif

int             authlen = 512;
static const char     *short_options = "dehs:HV";

enum { OPT_STDOUT = 1 };

static struct option long_options[] = {
	{"debug", no_argument, NULL, 'd'},
	{"help", no_argument, NULL, 'h'},
	{"noenv", no_argument, NULL, 'e'},
	{"no-chdir-home", no_argument, NULL, 'H'},
	{"service", required_argument, NULL, 's'},
	{"stdout", no_argument, NULL, OPT_STDOUT},
	{"version", no_argument, NULL, 'V'},
	{NULL, 0, NULL, 0}
};

static const char *usage =
	"Usage: " PACKAGE " [OPTION]... -- prog...\n"
	"\n"
	"Authenticate using PAM and the checkpassword protocol:\n"
	"\t<URL:http://cr.yp.to/checkpwd/interface.html>\n"
	"and run the program specified as 'prog'\n"
	"\n"
	"Options are:\n"
	"  -d, --debug\t\tturn on debugging output\n"
	"  -h, --help\t\tdisplay this help and exit\n"
	"  -e, --noenv\t\tdo not set uid, gid, environment variables,\n"
	"\t\t\tand home directory\n"
	"  -H, --no-chdir-home\tdo not change to home directory\n"
	"  -s, --service=SERVICE\tspecify PAM service name to use\n"
	"\t\t\t(by default use the contents of $PAM_SERVICE)\n"
	"  -V, --version\t\tdisplay version information and exit\n";

int
initialize(char *username, int opt_dont_chdir_home, int debug)
{
	struct passwd  *pw;
	/*- switch to proper uid/gid/groups */
	if (!(pw = getpwnam(username))) {
		if (debug)
			fprintf(stderr, "Error getting information about %s from /etc/passwd: %s\n", username, strerror(errno));
		exit(111);
	}
	/*- set supplementary groups */
	if (initgroups(username, pw->pw_gid) == -1) {
		fprintf(stderr, "Error setting supplementary groups for user %s: %s\n", username, strerror(errno));
		exit(111);
	}
	/*- set gid */
	if (setgid(pw->pw_gid) == -1) {
		fprintf(stderr, "setgid(%d) error: %s\n", pw->pw_gid, strerror(errno));
		exit(111);
	}
	/*- set uid */
	if (setuid(pw->pw_uid) == -1) {
		fprintf(stderr, "setuid(%d) error: %s\n", pw->pw_uid, strerror(errno));
		exit(111);
	}
	/*- switch to user home directory */
	if (!opt_dont_chdir_home) {
		if (chdir(pw->pw_dir) == -1) {
			fprintf(stderr, "Error changing directory %s: %s\n", pw->pw_dir, strerror(errno));
			exit(111);
		}
	}
	/*- set $USER */
	if (setenv("USER", username, 1) == -1) {
		fprintf(stderr, "Error setting $USER to %s: %s\n", username, strerror(errno));
		exit(111);
	}
	/*- set $HOME */
	if (setenv("HOME", pw->pw_dir, 1) == -1) {
		fprintf(stderr, "Error setting $HOME to %s: %s\n", pw->pw_dir, strerror(errno));
		exit(111);
	}
	/*- set $SHELL */
	if (setenv("SHELL", pw->pw_shell, 1) == -1) {
		fprintf(stderr, "Error setting $SHELL to %s: %s\n", pw->pw_shell, strerror(errno));
		exit(111);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	char           *ptr, *tmpbuf, *login, *response, *challenge;
	char            buf[MAX_BUFF];
	int             opt_use_stdout = 0, opt_dont_set_env = 0,
					opt_dont_chdir_home = 0, debug = 0, c, count, offset, status;
	char           *service_name = 0;
	int             option_index = 0;

	if(argc < 2)
		_exit(2);
	/*- process command line options */
	opterr = 0;
	while (1) {
		c = getopt_long(argc, argv, short_options, long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case OPT_STDOUT:
			opt_use_stdout = 1;
			break;
		case 'd':
			debug = 1;
			break;
		case 'H':
			opt_dont_chdir_home = 1;
			break;
		case 'e':
			opt_dont_set_env = 1;
			break;
		case 'h':
			puts(usage);
			exit(0);
		case 's':
			service_name = optarg;
			break;
		case 'V':
			puts(PACKAGE " " VERSION);
			exit(2);
		case '?':
			fprintf(stderr, "Invalid command line, see --help\n");
			exit(2);
		}
	}
	if (optind + 1 != argc)
		exit(2);
	if (!service_name && !(service_name = getenv("PAM_SERVICE"))) {
		fprintf(stderr, "PAM service name not specified\n");
		exit(2);
	}
	if(!(tmpbuf = calloc(1, (authlen + 1) * sizeof(char))))
	{
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		fprintf(stderr, "malloc-%d: %s\n", authlen + 1, strerror(errno));
		_exit(111);
	}
	for (offset = 0;;)
	{
		do
		{
			count = read(3, tmpbuf + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while(count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while(count == -1 && errno == EINTR);
#endif
		if (count == -1)
		{
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			fprintf(stderr, "read: %s\n", strerror(errno));
			_exit(111);
		}
		else
		if(!count)
			break;
		offset += count;
		if(offset >= (authlen + 1))
			_exit(2);
	}
	if (debug)
		fprintf(stderr, "read %d bytes\n", offset);
	count = 0;
	login = tmpbuf + count; /*- username */
	for(;tmpbuf[count] && count < offset;count++);
	if(count == offset || (count + 1) == offset)
	{
		if (debug)
			fprintf(stderr, "no username\n");
		_exit(2);
	}
	count++;
	challenge = tmpbuf + count; /*- challenge */
	for(;tmpbuf[count] && count < offset;count++);
	if(count == offset || (count + 1) == offset)
	{
		if (debug)
			fprintf(stderr, "no challenge\n");
		_exit(2);
	}
	response = tmpbuf + count + 1; /*- response */
	if((ptr = strchr(login, '@'))) /*- For stupid Netscape */
		*ptr = 0;
	if (debug)
		fprintf(stderr, "%s: login [%s] challenge [%s] response [%s]\n", 
			argv[0], login, challenge, response);
	/*- authenticate using PAM */
	status = 0;
	if ((status = authenticate_using_pam(service_name, login, challenge, debug)))
	{
		pipe_exec(argv, tmpbuf, offset);
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
	if ((ptr = (char *) getenv("POSTAUTH")) && !access(ptr, X_OK))
	{
		snprintf(buf, MAX_BUFF, "%s %s", ptr, login);
		status = runcmmd(buf, 0);
	}
	_exit(status);
	/*- Not reached */
	return(0);
}

void
getversion_checkpassword_pam_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
