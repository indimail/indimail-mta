#ifndef	proxy_h
#define	proxy_h

/*
** Copyright 2004 Double Precision, Inc.
** See COPYING for distribution information.
*/


struct proxyinfo {
	const char *host;
	int port;

	int (*connected_func)(int, const char *, void *);
	void *void_arg;
};

int connect_proxy(struct proxyinfo *);
void proxyloop(int);

struct proxybuf {
	char buffer[256];
	char *bufptr;
	size_t bufleft;
};

int proxy_readline(int fd, struct proxybuf *pb,
		   char *linebuf,
		   size_t linebuflen,
		   int imapmode);
int proxy_write(int fd, const char *hostname,
		const char *buf, size_t buf_len);

#endif
