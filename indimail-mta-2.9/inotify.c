/*
 * $Log: inotify.c,v $
 * Revision 1.6  2019-06-24 23:29:35+05:30  Cprogrammer
 * added notifications for open and delete
 *
 * Revision 1.5  2017-05-12 19:01:46+05:30  Cprogrammer
 * use compile time inotify(7) api
 *
 * Revision 1.4  2017-05-12 17:58:09+05:30  Cprogrammer
 * inotify indimail-mta version
 *
 * Revision 1.3  2015-04-16 17:49:28+05:30  Cprogrammer
 * new logic for handling timeouts
 *
 * Revision 1.2  2015-04-10 19:32:01+05:30  Cprogrammer
 * use select() to read events on fd 0 and event fd
 *
 *
 * This is the sample program to notify us for the file creation and file deletion takes place in “/tmp” directory
 */
#include "hasinotify.h"
#ifdef HASINOTIFY
#include <sys/inotify.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include "subfd.h"
#include "scan.h"
#include "strerr.h"
#include "env.h"
#include "sig.h"
#include "sgetopt.h"

#define EVENT_SIZE        ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define FATAL             "inotify: fatal: "
#define SELECTTIMEOUT     5

char           *usage = "usage: inotify [-n] path1..path2";

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

int             ifd, _soptind, _sargc, *wd;

void
sigterm()
{
	substdio_flush(subfdout);
	substdio_flush(subfderr);
 	/* removing the watched directory from the watch list.  */
	for (optind = _soptind; wd && optind < _sargc; optind++)
		inotify_rm_watch(ifd, wd[optind - _soptind]);
	_exit(1);
}

int
main(int argc, char **argv)
{
	int             opt, length, i = 0, dataTimeout = -1, retval, read_stdin = 1, _soptind;
	struct timeval  timeout;
	struct timeval *tptr;
	time_t          last_timeout;
	char            buffer[EVENT_BUF_LEN];
	char           *ptr;
	fd_set          rfds;	/*- File descriptor mask for select -*/

	if (argc < 2)
		strerr_die1x(100, usage);
	_sargc = argc;
	sig_termcatch(sigterm);
	while ((opt = getopt(argc, argv, "nd:")) != opteof) {
		switch (opt) {
		case 'n':
			read_stdin = 0;
			break;
		case 'd':
			scan_int(optarg, &dataTimeout);
			break;
		}
	}
	if (optind == argc)
		strerr_die1x(100, usage);
	if (!(wd = (int *) malloc(sizeof(int) * (argc - optind))))
		strerr_die2sys(111, FATAL, "out of mem");
	/*- create a INOTIFY instance */
	if ((ifd = inotify_init()) < 0)
		strerr_die2sys(111, FATAL, "inotify_init: ");
	for (_soptind = optind; optind < argc; optind++) {
		if (access(argv[optind], F_OK))
			strerr_die2sys(111, FATAL, argv[optind]);
		/*- adding a directory into watch list.  */
		if ((wd[optind - _soptind] = inotify_add_watch(ifd, argv[optind], IN_CREATE | IN_OPEN| IN_CLOSE_WRITE| IN_DELETE)) == -1)
			strerr_die4sys(111, FATAL, "inotify_add_watch: ", argv[optind], ": ");
	}
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (dataTimeout == -1) {
		if (!(ptr = env_get("DATA_TIMEOUT")))
			ptr = "1800";
		scan_int(ptr, &dataTimeout);
	}
	if (dataTimeout > 0)
		timeout.tv_sec = (dataTimeout > SELECTTIMEOUT) ? dataTimeout : SELECTTIMEOUT;
	else
		timeout.tv_sec = 0 - dataTimeout;
	timeout.tv_usec = 0;
	tptr = (timeout.tv_sec ? &timeout : (struct timeval *) 0);

	last_timeout = timeout.tv_sec;
	for (;;) {
		FD_ZERO(&rfds);
		if (read_stdin)
			FD_SET(0, &rfds);
		FD_SET(ifd, &rfds);
		i = 0;
		if ((retval = select(ifd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, tptr)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
		}
		if (!retval) /*- timeout occurred */
		{
			if (dataTimeout > 0) {
				last_timeout += 2 * last_timeout;
				timeout.tv_sec = last_timeout;
			} else {
				timeout.tv_sec = 0 - dataTimeout;
				last_timeout = timeout.tv_sec;
			}
			timeout.tv_usec = 0;
		} else {
			if (dataTimeout > 0)
				last_timeout = timeout.tv_sec = (dataTimeout > SELECTTIMEOUT) ? dataTimeout : SELECTTIMEOUT;
			else
				timeout.tv_sec = 0 - dataTimeout;
			timeout.tv_usec = 0;
		}
		if (read_stdin && FD_ISSET(0, &rfds)) { /*- data from socket/stdin and display it */
			if ((length = read(0, buffer, sizeof(buffer))) == -1)
				strerr_die2sys(111, FATAL, "read-network: ");
			if (!length) {
				if (substdio_puts(subfderr, "shutting down\n") == -1)
					strerr_die2sys(111, FATAL, "write: ");
				if (substdio_flush(subfderr) == -1)
					strerr_die2sys(111, FATAL, "write: ");
				break;
			} else {
				if (substdio_put(subfderr, buffer, length) == -1)
					strerr_die2sys(111, FATAL, "write: ");
				if (substdio_flush(subfderr) == -1)
					strerr_die2sys(111, FATAL, "write: ");
			}
		}
		if (!FD_ISSET(ifd, &rfds))
			continue;
		/*
	 	 * read to determine the event change happens on a directory. Actually 
	 	 * this read blocks until the change event occurs
	 	 */
		if ((length = read(ifd, buffer, EVENT_BUF_LEN)) < 0)
			strerr_die2sys(111, FATAL, "read-event: ");

		/*-
	 	 * actually read return the list of change events happens.
		 * Here, read the change event one by one and process it accordingly.
	 	 */
		while (i < length) {
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			if (event->len) {
				if (event->mask & IN_CREATE) {
					if (event->mask & IN_ISDIR) {
						out("dir  ");
						out(event->name);
						out(" created\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					} else {
						out("file ");
						out(event->name);
						out(" created\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					}
				} else
				if (event->mask & IN_DELETE) {
					if (event->mask & IN_ISDIR) {
						out("dir  ");
						out(event->name);
						out(" deleted\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					} else {
						out("file ");
						out(event->name);
						out(" deleted\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					}
				} else
				if (event->mask & IN_OPEN) {
					if (event->mask & IN_ISDIR) {
						out("dir  ");
						out(event->name);
						out(" opened\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					} else {
						out("file ");
						out(event->name);
						out(" opened\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					}
				} else
				if (event->mask & IN_DELETE) {
					if (event->mask & IN_ISDIR) {
						out("dir  ");
						out(event->name);
						out(" deleted\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					} else {
						out("file ");
						out(event->name);
						out(" deleted\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					}
				} else
				if (event->mask & IN_CLOSE_WRITE) {
					if (event->mask & IN_ISDIR) {
						out("dir  ");
						out(event->name);
						out(" closed\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					} else {
						out("file ");
						out(event->name);
						out(" closed\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					}
				} else {
					if (event->mask & IN_ISDIR) {
						out("dir ");
						out(event->name);
						out("\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					} else {
						out("file ");
						out(event->name);
						out("\n");
						if (substdio_flush(subfdout) == -1)
							strerr_die2sys(111, FATAL, "write: ");
					}
				}
			}
			i += EVENT_SIZE + event->len;
		}
	}
 	/* removing the watched directory from the watch list.  */
	for (optind = _soptind; optind < argc; optind++)
		inotify_rm_watch(ifd, wd[optind - _soptind]);
	close(ifd);
	_exit(0);
}
#else
#warning "this system does not support inotify(7)"
#include <unistd.h>
#include "subfd.h"
int
main(int argc, char **argv)
{
	substdio_puts(subfderr, "inotify is missing on your system\n");
	substdio_flush(subfderr);
	_exit(111);
}
#endif

void
getversion_inotify_c()
{
	static char    *x = "$Id: inotify.c,v 1.6 2019-06-24 23:29:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
