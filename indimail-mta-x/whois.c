/*
 * $Log: whois.c,v $
 * Revision 1.7  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2023-07-13 02:49:30+05:30  Cprogrammer
 * replaced logerr(), logerrf() with subprintf()
 *
 * Revision 1.5  2022-05-10 20:56:35+05:30  Cprogrammer
 * replaced strstr with str_str from libqmail
 *
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2018-05-30 12:10:23+05:30  Cprogrammer
 * fixed newline getting appended to whois server variable
 *
 * Revision 1.2  2015-08-20 18:49:34+05:30  Cprogrammer
 * modified str_whois function
 *
 * Revision 1.1  2015-08-19 16:30:30+05:30  Cprogrammer
 * Initial revision
 *
 *
 * @brief whois client program
 *
 * @details This program shall perform whois for a domain and get you the whois
 * data of that domain
 *
 * @author Silver Moon ( m00n.silv3r@gmail.com )
 */
#include <unistd.h>
#include <sys/socket.h>
#include <subfd.h>
#include <sgetopt.h>
#include <strerr.h>
#include <str.h>
#include <stralloc.h>
#include <noreturn.h>
#include <qprintf.h>
#include <tcpopen.h>

#define FATAL "whois: fatal: "

int		get_whois_data(char *);
int		whois_query(const char *, char *, stralloc *);

static int      verbose;
static stralloc whois_server = {0}, response_1 = {0},
				response_2 = {0}, message = {0};

void
out(const char *str)
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

no_return void
die_nomem()
{
	substdio_flush(subfdout);
	substdio_puts(subfderr, "out of memory\n");
	substdio_flush(subfderr);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *arg;
	int             opt;

	while ((opt = getopt(argc, argv, "v")) != opteof) {
		switch (opt) {
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_die2x(111, FATAL, "usage: whois [-v] domain");
		}
	}
	if (optind + 1 != argc)
		strerr_die2x(111, FATAL, "usage: whois [-v] domain");
	arg = argv[optind++];
	return (get_whois_data(arg));
}

char *
str_whois(char *resp, int resp_len)
{
	char		   *pch, *wch;
	int             len;

	whois_server.len = 0;
	if ((pch = str_str(resp, "whois."))) {
		for (len = 0, wch = pch; len < resp_len; len++, wch++) {
			if (*wch == '\n' || *wch == '\r') {
				if (!stralloc_copyb(&whois_server, pch, len))
					die_nomem();
				if (!stralloc_0(&whois_server))
					die_nomem();
				break;
			}
		}
	}
	return (pch ? whois_server.s : 0);
}

/*
 * Get the whois data of a domain
 */
int
get_whois_data(char *domain)
{
	char		*ext, *ptr, *wch;
	int          i ;

	/* remove "http://" and "www." */
	if ((ptr = str_str(domain, "www.")))
		ptr += 4;
	else
	if ((ptr = str_str(domain, "http://")))
		ptr += 7;
	else
		ptr = domain;

	i = str_chr(ptr, '.');
	if (!ptr[i]) {
		subprintf(subfderr, "invalid domain %s\n", ptr);
		substdio_flush(subfderr);
		_exit (100);
	} else
		ext = ptr + i + 1;
	/*
	 * This will tell the whois server for the particular TLD like com ,
	 * org
	 */
	if (whois_query("whois.iana.org", ext, &response_1)) {
		subprintf(subfderr, "whois query failed\n");
		substdio_flush(subfderr);
		_exit (100);
	}

	wch = str_whois(response_1.s, response_1.len);
	/*
	 * Now we have the TLD whois server in wch , query again This will
	 * provide minimal whois information along with the parent whois
	 * server of the specific domain :)
	 */
	if (!wch) {
		subprintf(subfderr, "TLD whois server for %s not found\n", ext);
		substdio_flush(subfderr);
		_exit(100);
	}
	out("TLD whois server is: ");
	out(wch);
	out(", domain ");
	out(ptr);
	out("\n");
	if (whois_query(wch, ptr, &response_2)) {
		subprintf(subfderr, "whois query failed\n");
		substdio_flush(subfderr);
		_exit (100);
	}
	/* Again search for a whois server in this response. :) */
	wch = str_whois(response_2.s, response_2.len);

	/*- If a registrar whois server is found then query it */
	if (wch) {
		/*
		 * Now we have the registrar whois server , this has the
		 * direct full information of the particular domain so lets
		 * query again
		 */

		out("Registrar whois server is: ");
		out(wch);
		out(", domain ");
		out(ptr);
		out("\n");
		if (whois_query(wch, ptr, &response_1)) {
			subprintf(subfderr, "whois query failed\n");
			substdio_flush(subfderr);
			_exit (100);
		}
		out(response_1.s);
		out("\n");
	} else {
		/*- otherwise echo the output from the previous whois result */
		out(response_2.s);
		out("\n");
	}
	flush();
	return 0;
}

/*
 * Perform a whois query to a server and record the response
 */
int
whois_query(const char *server, char *query, stralloc *response)
{
	char            buffer[1500];
	int	            sock, read_size;

	if ((sock = tcpopen(server, (char *) "nicname", 0)) == -1)
		strerr_die4sys(111, FATAL, "tcpopen: ", server, ": ");
	if (verbose) {
		subprintf(subfderr, "querying %s for %s\n", server, query);
		substdio_flush(subfderr);
	}
	if (!stralloc_copys(&message, query))
		die_nomem();
	if (!stralloc_cats(&message, "\r\n"))
		die_nomem();
	if (send(sock, message.s, message.len, 0) < 0)
		strerr_die2sys(111, FATAL, "send: ");
	response->len = 0;
	while ((read_size = recv(sock, buffer, sizeof(buffer), 0))) {
		if (!stralloc_catb(response, buffer, read_size))
			die_nomem();
	}
	if (!stralloc_0(response))
		die_nomem();
	response->len--;
	close(sock);
	return 0;
}

void
getversion_whois_c()
{
	const char     *x = "$Id: whois.c,v 1.7 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
