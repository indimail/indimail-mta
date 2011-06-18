/*-
 * Copyright (C) 2004 Ben Goodwin
 * This file is part of the nsvs package
 *
 * The nsvs package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The nsvs package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with the nsvs package; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * $Id: sock.c,v 1.1 2011-06-18 11:38:50+05:30 Cprogrammer Exp mbhangui $ 
 */
#include "common.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include "nsvs.h"

// FIXME sig handler testing ...
#define HANDLE_SIGNALS 1
#if HANDLE_SIGNALS
#include <signal.h>
typedef void    (*sighandler_t) (int);
#endif

static int
_open_socket(int *sock)
{
	char            buf[256];
	char           *socket_file;
	struct sockaddr_un sock_addr;

	getEnvConfigStr(&socket_file, "NSSD_SOCKET", _PATH_NSVSD_SOCKET);
	if ((*sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		strerror_r(errno, buf, sizeof(buf));
		nsvs_log(LOG_ALERT, "%s: sock: %s", __FUNCTION__, buf);
		return 0;
	}
	sock_addr.sun_family = AF_UNIX;
	strcpy(sock_addr.sun_path, socket_file);
	if (connect(*sock, (struct sockaddr *) &sock_addr, sizeof (sock_addr)) < 0) {
		strerror_r(errno, buf, sizeof(buf));
		nsvs_log(LOG_ALERT, "%s: connect: %s: (errno %d) %s", __FUNCTION__, socket_file, errno, buf);
		close(*sock);
		return 0;
	}
	if (fcntl(*sock, F_SETFL, O_NONBLOCK) == -1) {
		strerror_r(errno, buf, sizeof(buf));
		nsvs_log(LOG_ALERT, "%s: fcntl: %s", __FUNCTION__, buf);
		close(*sock);
		return 0;
	}
	return 1;
}

static int
_send_request(int sock, int32_t type, const char *request)
{
	ssize_t         bytes;
	request_header_t req;

	req.version = INTERFACE_VERSION;
	req.type = type;
	req.key_len = request ? strlen(request) + 1 : 0;

#if HANDLE_SIGNALS
	{
		sighandler_t    previous;
		previous = signal(SIGPIPE, SIG_IGN);
#endif
		bytes = write_wt(sock, &req, sizeof (req), WRITE_TIMEOUT);
		bytes += write_wt(sock, request, req.key_len, WRITE_TIMEOUT);
#if HANDLE_SIGNALS
		signal(SIGPIPE, previous);
	}
#endif

	if (bytes == sizeof (req) + req.key_len)
		return 1;

	return 0;
}

static int
_read_response_header(int sock, response_header_t * response_header, int timeout)
{
	ssize_t         to_read = sizeof (response_header_t);
	ssize_t         r;

#if HANDLE_SIGNALS
	{
		sighandler_t    previous;
		previous = signal(SIGPIPE, SIG_IGN);
#endif
		r = read_wt(sock, response_header, to_read, timeout);
#if HANDLE_SIGNALS
		signal(SIGPIPE, previous);
	}
#endif
	if (r == to_read)
		return 1;
	return 0;
}

static int
_read_response(int sock, response_header_t response_header, struct response_data *data, int timeout)
{
	ssize_t         to_read = response_header.response_size;
	ssize_t         r;

#if HANDLE_SIGNALS
	{
		sighandler_t    previous;
		previous = signal(SIGPIPE, SIG_IGN);
#endif
		r = read_wt(sock, data, to_read, timeout);
#if HANDLE_SIGNALS
		signal(SIGPIPE, previous);
	}
#endif
	if (r == to_read)
		return 1;
	return 0;
}

NSS_STATUS
_get_response_data(int32_t type, const char *key, response_header_t * response_header, struct response_data ** data, int timeout)
{
	int             sock;

	if (!_open_socket(&sock))
		return NSS_UNAVAIL;

	if (!_send_request(sock, type, key)) {
		close(sock);
		return NSS_UNAVAIL;
	}

	if (!_read_response_header(sock, response_header, timeout)) {
		close(sock);
		return NSS_UNAVAIL;
	}

	if (response_header->count == 0) {
		close(sock);
		return NSS_NOTFOUND;
	}

	*data = (struct response_data *) malloc(response_header->response_size);
	if (!*data) {
		close(sock);
		return NSS_UNAVAIL;
	}

	memset(*data, 0, response_header->response_size);

	if (!_read_response(sock, *response_header, *data, timeout)) {
		XFREE(*data);
		close(sock);
		return NSS_UNAVAIL;
	}
	close(sock);
	return NSS_SUCCESS;
}
