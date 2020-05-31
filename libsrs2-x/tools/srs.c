#ifdef HAVE_CONFIG_H
# include "../config.h"
#else
/* Let's take a wild guess */
# define HAVE_SYS_SOCKET_H
# define HAVE_SYS_TIME_H
# define HAVE_UNISTD_H
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#include <sys/un.h>
#endif

#include "../libsrs2/srs2.h"


#define SRS_DIE() do{fprintf(stderr, "Exiting.\n");exit(255);}while(0)

#define SRS_DIR_FORWARD	0
#define SRS_DIR_REVERSE	1

#define DF_DAEMON	1
#define DF_NOFORK	2

#define SRS_DIE_UNLESS(x) \
	do { int _ret = x; \
		if (_ret != SRS_SUCCESS) die (_ret); \
	} while(0)

#ifdef HAVE_GETOPT_LONG
static const struct option	longopts[] = {
		{ "daemon",			no_argument,		NULL, 'd', },
		{ "nofork",			no_argument,		NULL, 'x', },
		{ "socket",			required_argument,	NULL, 's', },
		{ "separator",		required_argument,	NULL, 'e', },
		{ "address",		required_argument,	NULL, 'a', },
		{ "alwaysrewrite",	no_argument,		NULL, 'r', },
		{ "secret",			required_argument,	NULL, 'k', },
		{ "secretfile",		required_argument,	NULL, 'f', },
		{ "forward",		no_argument,		NULL, 'o', },
		{ "reverse",		no_argument,		NULL, 'v', },
		{ "alias",			required_argument,	NULL, 'i', },
		{ "hashlength",		required_argument,	NULL, 'l', },
		{ "help",			no_argument,		NULL, 'h', },
		{ 0, 0, 0, 0 },
	};
	int		 idx;
#define DOC_LONGOPT(l, v, t, p1) do { \
		fprintf(stderr, "    --%s%c%s%*s" t "\n", \
			l, (v ? '=' : ' '),  (v ? v : ""), p1, ""); \
	} while(0)
#else
#define DOC_LONGOPT(l, v, t, p1) do { } while(0)
#endif
static const char	*shortopts = "dxs:e:a:k:f:ovi:l:hr";

#define DOC_OPT(s, l, v, t, p0, p1) do { \
		fprintf(stderr, "    -%c%c%s%*s" t "\n", \
			s, (v ? ' ' : ' '), (v ? v : ""), p0, ""); \
		DOC_LONGOPT(l, v, t, p1); \
	} while(0)

static void
die(int code)
{
	fprintf(stderr, "Error %d: %s\n", code, srs_strerror(code));
	SRS_DIE();
}

static void
usage(char *name)
{
	fprintf(stderr, "Sender Rewriting Scheme - "
					"command/daemon interface - "
				"http://www.libsrs2.org/\n");
	fprintf(stderr, "Usage: %s [options] <address> [...]\n", name);
	DOC_OPT('e', "separator", "<sep>",
			"Set the initial SRS separator.", 18, 9);
	DOC_OPT('a', "address", "<address>",
			"Provide an address to translate.", 14, 7);
	DOC_OPT('k', "secret", "<secret>",
			"Provide a cryptographic secret.", 15, 9);
	DOC_OPT('f', "secretfile", "<filename>",
			"Provide a file containing cryptographic secrets.", 13, 3);
	DOC_OPT('o', "forward", NULL,
			"Perform forwards rewriting.", 23, 16);
	DOC_OPT('v', "reverse", NULL,
			"Perform reverse rewriting.", 23, 16);
	DOC_OPT('i', "alias", "<domain>",
			"Specify the alias for forwards rewriting.", 15, 10);
	DOC_OPT('l', "hashlength", "<int>",
			"Specify the hash length for forwards rewriting.", 18, 8);
	DOC_OPT('r', "alwaysrewrite", NULL,
			"Force rewriting even when optional.", 23, 10);
	DOC_OPT('d', "daemon", NULL,
			"Become a rewriting daemon.", 23, 17);
	DOC_OPT('x', "nofork", NULL,
			"Do not fork daemon into background.", 23, 17);
	DOC_OPT('s', "socket", NULL,
			"Specify the socket for the rewriting daemon.", 23, 17);
	DOC_OPT('h', "help", NULL,
			"Display this help.", 23, 19);
}

static int
listen_socket(char *path)
{
	struct sockaddr_un	 addr;
	int	 				 sock;

	if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		SRS_DIE();
	}
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path);
	if (unlink(path) < 0) {
		if (errno != ENOENT) {
			perror("unlink");
			SRS_DIE();
		}
	}
	if (bind(sock, (struct sockaddr *)(&addr), sizeof(addr)) < 0) {
		perror("bind");
		SRS_DIE();
	}
	if (listen(sock, 5) < 0) {
		perror("listen");
		SRS_DIE();
	}
	fprintf(stderr, "Accepting connections on %s\n", path);

	return sock;
}

static void
rundaemon(srs_t *srs, char *path, int daemonflags)
{
	fd_set				 readfds_sv;
	fd_set				 readfds;
	struct sockaddr_un	 addr;
	socklen_t			 addrlen;
	int	 				 sock;
	int					 maxfd;
	int					 fd;
	char				 line[BUFSIZ];
	int					 len;
	char				 buf[BUFSIZ];
	char				*cp;
	char				*address;
	char				*alias;
	int					 ret;

	sock = listen_socket(path);

	/* daemon() */

	FD_ZERO(&readfds_sv);
	FD_SET(sock, &readfds_sv);
	maxfd = sock + 1;

#define FINISH(fd) close(fd); FD_CLR(fd, &readfds_sv)
#define SKIPWHITE(cp) while (isspace(*cp)) cp++;

	if (!(daemonflags & DF_NOFORK)) {
		if (daemon(0, 0) < 0)
			perror("daemon");
	}

	for (;;) {
		memcpy(&readfds, &readfds_sv, sizeof(fd_set));

		select(maxfd, &readfds, NULL, NULL, NULL);
		for (fd = 0; fd < maxfd; fd++) {
			if (FD_ISSET(fd, &readfds)) {
				if (fd == sock) {
					fd = accept(sock, (struct sockaddr*)&addr,&addrlen);
					if (fd < 0) {
						perror("accept");
						continue;
					}
					FD_SET(fd, &readfds_sv);
					if (fd >= maxfd)
						maxfd = fd + 1;
					fprintf(stderr, "Accept %d\n", fd);
					continue;
				}
				else {
					len = read(fd, line, BUFSIZ);
					if (len <= 0) {
						if (len < 0)
							perror("read");
						fprintf(stderr, "Close %d\n", fd);
						FINISH(fd);
						continue;
					}
					line[strcspn(line, "\r\n")] = '\0';
					fprintf(stderr, "%d: %s\n", fd, line);

					if (strncasecmp(line, "forward ", 8) == 0) {
						cp = line + 8;
						while (isspace(*cp))
							cp++;
						address = cp;
						cp = strchr(address, ' ');
						if (cp == NULL) {
							fprintf(stderr, "No alias in %s on %d\n",
											line, fd);
							FINISH(fd);
							continue;
						}
						*cp++ = '\0';
						while (isspace(*cp))
							cp++;
						alias = cp;
						ret = srs_forward(srs, buf, BUFSIZ,
										address, alias);
						if (ret != SRS_SUCCESS) {
							fprintf(stderr, "SRS error: %s\n",
											srs_strerror(ret));
							FINISH(fd);
							continue;
						}
						fprintf(stderr, "Forward %s, %s -> %s\n",
										address, alias, buf);
						write(fd, buf, strlen(buf));
						write(fd, "\n", 1);
						FINISH(fd);
						continue;
					}
					else if (strncasecmp(line, "reverse ", 8) == 0) {
						cp = line + 8;
						while (isspace(*cp))
							cp++;
						address = cp;
						ret = srs_reverse(srs, buf, BUFSIZ, address);
						if (ret != SRS_SUCCESS) {
							fprintf(stderr, "SRS error: %s\n",
											srs_strerror(ret));
							FINISH(fd);
							continue;
						}
						fprintf(stderr, "Reverse %s -> %s\n",
										address, buf);
						write(fd, buf, strlen(buf));
						write(fd, "\n", 1);
						FINISH(fd);
						continue;
					}
					else {
						fprintf(stderr, "Unknown command %s on %d\n",
										line, fd);
						FINISH(fd);
						continue;
					}
				}
			}
		}
	}
}

int
main(int argc, char **argv)
{

	char	 c;
	FILE	*fp;
	char	 line[BUFSIZ];

	srs_t	*srs;
	int		 direction = SRS_DIR_FORWARD;
	int		 daemonflags = 0;
	int		 secrets = 0;
	char	*socketpath = "/tmp/srsd";
	char	*alias = NULL;
	char	**addrs = alloca(argc * sizeof(char *));
	char	**addre = addrs;

	char	**addrp;
	int		 i;

	srs = srs_new();

	while ((c =
#ifdef HAVE_GETOPT_LONG
		getopt_long(argc, argv, shortopts, longopts, &idx)
#else
		getopt(argc, argv, shortopts)
#endif
			) != -1) {
		switch (c) {
			case 'd':
				daemonflags |= DF_DAEMON;
				break;
			case 'x':
				daemonflags |= DF_NOFORK;
				break;
			case 's':
				socketpath = optarg;
				break;
			case 'e':
				SRS_DIE_UNLESS(srs_set_separator(srs, optarg[0]));
				break;
			case 'a':
				*addre++ = optarg;
				break;
			case 'k':
				SRS_DIE_UNLESS(srs_add_secret(srs, optarg));
				secrets++;
				break;
			case 'r':
				SRS_DIE_UNLESS(srs_set_alwaysrewrite(srs, TRUE));
				break;
			case 'f':
				fp = fopen(optarg, "r");
				if (fp == NULL) {
					fprintf(stderr, "Failed to open %s: %s\n",
									optarg, strerror(errno));
					SRS_DIE();
				}
				while (fgets(line, BUFSIZ, fp) != NULL) {
					line[strcspn(line, "\r\n")] = '\0';
					if (line[0] == '#')
						continue;
					if (line[0] == '\0')
						continue;
					SRS_DIE_UNLESS(srs_add_secret(srs, line));
					secrets++;
				}
				fclose(fp);
				break;
			case 'o':
				direction = SRS_DIR_FORWARD;
				break;
			case 'v':
				direction = SRS_DIR_REVERSE;
				break;
			case 'i':
				alias = optarg;
				break;
			case 'l':
				SRS_DIE_UNLESS(srs_set_hashlength(srs, atol(optarg)));
				break;
			case 'h':
			case '?':
				usage(argv[0]);
				SRS_DIE();
			default:
				fprintf(stderr, "Unrecognised option %c\n", c);
				SRS_DIE();
		}
	}
	argv += optind;
	argc -= optind;

	if (secrets == 0) {
		fprintf(stderr, "No secrets!\n");
		SRS_DIE();
	}

	if (daemonflags & DF_DAEMON) {
		rundaemon(srs, socketpath, daemonflags);
		SRS_DIE();
	}

	for (i = 0; i < argc; i++)
		*addre++ = argv[i];

	if ((direction == SRS_DIR_FORWARD) && (alias == NULL)) {
		fprintf(stderr, "No alias given for forwards translation. "
						"Please use --alias or -i.\n");
		SRS_DIE();
	}

	if (addre == addrs) {
		fprintf(stderr, "No alias given to translate.\n");
		SRS_DIE();
	}

	for (addrp = addrs; addrp < addre; addrp++) {
		if (direction == SRS_DIR_FORWARD)
			SRS_DIE_UNLESS(srs_forward(srs, line, BUFSIZ, *addrp, alias));
		else
			SRS_DIE_UNLESS(srs_reverse(srs, line, BUFSIZ, *addrp));
		printf("%s\n", line);
	}

	return 0;
}
