/*
** Copyright 1998 - 2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include "rfc2045_config.h"
#endif
#include	"rfc2045.h"
#include	"rfc2045charset.h"
#include	"rfc822/encode.h"
#include	"rfc822/rfc822hdr.h"
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include       <string.h>
#if    HAVE_STRINGS_H
#include       <strings.h>
#endif


static char *rw_boundary_root;
static int rw_boundary_cnt;
static const char *rw_appname;

static int fdout;
static int (*fdout_func)(const char *, int, void *);
static void *fdout_arg;

static char fdout_buf[512];
static char *fdout_ptr;
static size_t fdout_left;
static int conv_err;

static int fdout_flush()
{
int	n=fdout_ptr-fdout_buf;
int	i=0;
char	*p=fdout_buf;

	while (n)
	{
		i=fdout_func ? (*fdout_func)(p, n, fdout_arg):
				write(fdout, p, n);
		if (i <= 0)	return (-1);
		p += i;
		n -= i;
	}
	fdout_ptr=fdout_buf;
	fdout_left=sizeof(fdout_buf);
	return (0);
}

static int fdout_add(const char *p, size_t cnt)
{
	while (cnt)
	{
		if (cnt < fdout_left)
		{
			memcpy(fdout_ptr, p, cnt);
			fdout_ptr += cnt;
			fdout_left -= cnt;
			return (0);
		}
		if (fdout_left == 0)
		{
			if (fdout_flush())	return (-1);
			continue;
		}
		memcpy(fdout_ptr, p, fdout_left);
		p += fdout_left;
		cnt -= fdout_left;
		fdout_ptr += fdout_left;
		fdout_left=0;
	}
	return (0);
}

static int do_8bit(const char *p, size_t cnt, void *ptr)
{
	if (fdout_add(p, cnt))
		conv_err=1;
	return (0);
}

static int fdout_autoconverted(const char *oldte, const char *newte)
{
	if (fdout_add("X-Mime-Autoconverted: from ", 27) ||
		fdout_add(oldte, strlen(oldte)) ||
		fdout_add(" to ", 4) ||
		fdout_add(newte, strlen(newte)) ||
		(rw_appname && (fdout_add(" by ", 4) ||
			fdout_add(rw_appname, strlen(rw_appname)))) ||
		fdout_add("\n", 1))	return (-1);
	return (0);
}

static int fdout_value(const char *);

static int fdout_attr(const struct rfc2045attr *a)
{
	if (fdout_add(a->name, strlen(a->name)))	return (-1);
	if (a->value && (fdout_add("=", 1) || fdout_value(a->value)))
		return (-1);
	return (0);
}

static int fdout_value(const char *v)
{
size_t i,j;

	for (i=0; v[i]; i++)
	{
		if ( !isalnum((int)(unsigned char)v[i]) && v[i] != '-')
		{
			if (fdout_add("\"", 1))	return (-1);
			for (j=i=0; v[i]; i++)
				if (v[i] == '\\' || v[i] == '"')
				{
					if (fdout_add(v+j, i-j) ||
						fdout_add("\\", 1))
						return (-1);
					j=i;
				}
			if (fdout_add(v+j, i-j) || fdout_add("\"", 1))
				return (-1);
			return (0);
		}
	}
	return (fdout_add(v, i));
}

static int fdout_add_qp(const char *ptr, size_t cnt, void *dummy)
{
	return (fdout_add(ptr, cnt));
}

static int qpe_do(const char *p, size_t i, void *ptr)
{
	return libmail_encode( (struct libmail_encode_info *)ptr, p, i);
}

#define	TE(p)	((p)->rw_transfer_encoding ? \
		(p)->rw_transfer_encoding: (p)->content_transfer_encoding)

static int rwmime(struct rfc2045 *p)
{
static char mimever[]="Mime-Version: 1.0\n";
const char *te;
struct	rfc2045attr *a;

	if (!p->parent)
		if (fdout_add(mimever, sizeof(mimever)-1))	return (-1);

	if (p->content_type)
	{
		if (fdout_add("Content-Type: ", 14) ||
			fdout_add(p->content_type, strlen(p->content_type)))
			return (-1);

		for (a=p->content_type_attr; a; a=a->next)
		{
			if (!a->name || strcmp(a->name, "boundary") == 0)
				continue;
			if ( fdout_add("; ", 2) ||
				fdout_attr(a))	return (-1);
		}
	}

	if (p->firstpart
		&& p->firstpart->next /* ADDED 8/30/99, see below */)
	{
	char	buf[80];

		++rw_boundary_cnt;
		sprintf(buf, "-%d", rw_boundary_cnt);
		if ( fdout_add("; boundary=\"", 12) ||
			fdout_add(rw_boundary_root, strlen(rw_boundary_root)) ||
			fdout_add(buf, strlen(buf)) ||
			fdout_add("\"", 1))	return (-1);
	}
	if (fdout_add("\n", 1))	return (-1);

	/*
	** Show content transfer encoding, unless this is a multipart
	** section.
	*/

	te=TE(p);
	if (te && p->firstpart == NULL)
	{
		if (fdout_add("Content-Transfer-Encoding: ", 27) ||
			fdout_add(te, strlen(te)) ||
			fdout_add("\n", 1))	return (-1);
	}
	return (0);
}

static int dorw(struct rfc2045 *p, struct rfc2045src *src)
{
	int seen_mime=0;
	char	buf[BUFSIZ];
	struct libmail_encode_info qp_encode;
	int	bcnt;
	struct rfc2045headerinfo *hdr;
	char *name, *value;

	/* Slurp the untouchable portion of multipart/signed */

#if 1
	if (p->parent && p->parent->content_type &&
	    strcasecmp(p->parent->content_type, "multipart/signed") == 0 &&
	    p->next)
	{
		off_t ps=p->startpos;

		if ((*src->seek_func)(ps, src->arg) == (off_t)-1)
			return (-1);

		while (ps < p->endbody)
		{
			ssize_t n;

			if (p->endbody - ps > sizeof(buf))
				n=sizeof(buf);
			else	n=p->endbody-ps;
			n=(*src->read_func)(buf, n, src->arg);
			if (n <= 0)	return (-1);

			if (fdout_add(buf, n))	return (-1);
			ps += n;
		}
		return (0);
	}
#endif

	if (p->parent)
	{
		seen_mime=1;
		if (rwmime(p))	return (-1);
	}

	hdr=rfc2045header_start(src, p);

	while (1)
	{
		if (rfc2045header_get(hdr, &name, &value,
				      RFC2045H_NOLC | RFC2045H_KEEPNL) < 0)
		{
			rfc2045header_end(hdr);
			return (-1);
		}

		if (name == NULL)
			break;

		if (RFC2045_ISMIME1DEF(p->mime_version) &&
		    rfc822hdr_namecmp(name, "mime-version") == 0 &&
			!seen_mime)
		{
			seen_mime=1;
			rwmime(p);
			continue;
		}

		if (!RFC2045_ISMIME1DEF(p->mime_version) ||
		    (rfc822hdr_namecmp(name, "mime-version") &&
		     rfc822hdr_namecmp(name, "content-type") &&
		     rfc822hdr_namecmp(name, "content-transfer-encoding")))
		{
			if (fdout_add(name, strlen(name)) ||
			    fdout_add(": ", 2) ||
			    fdout_add(value, strlen(value)) ||
			    fdout_add("\n", 1))
			{
				rfc2045header_end(hdr);
				return (-1);
			}
		}
	}
	rfc2045header_end(hdr);

	if (RFC2045_ISMIME1DEF(p->mime_version))
	{
		if (!seen_mime)
			if (rwmime(p))	return (-1);

		if (!p->firstpart && p->rw_transfer_encoding)
			if (fdout_autoconverted(p->content_transfer_encoding,
				p->rw_transfer_encoding))	return (-1);
	}

	if (fdout_add("\n", 1))	return (-1);

	if ((*src->seek_func)(p->startbody, src->arg) == (off_t)-1)
		return (-1);

	/* For non-multipart section, just print the body */

	if (!p->firstpart)
	{
	off_t	ps=p->startbody;
	int	convmode=0;

		if (p->rw_transfer_encoding)
		{
			if ( strcasecmp(p->rw_transfer_encoding,
				"quoted-printable") == 0)
				convmode=RFC2045_RW_7BIT;
			else
				convmode=RFC2045_RW_8BIT;
		}

		conv_err=0;
		if (convmode == RFC2045_RW_7BIT)
		{
			libmail_encode_start(&qp_encode,
					     "quoted-printable",
					     fdout_add_qp, NULL);
			rfc2045_cdecode_start(p, &qpe_do, &qp_encode);
		}

		if (convmode == RFC2045_RW_8BIT)
		{
			rfc2045_cdecode_start(p, &do_8bit, 0);
		}

		while (ps < p->endbody)
		{
		int	n;

			if (p->endbody - ps > sizeof(buf))
				n=sizeof(buf);
			else	n=p->endbody-ps;
			n=(*src->read_func)(buf, n, src->arg);
			if (n <= 0)	return (-1);
			if (convmode)
				rfc2045_cdecode(p, buf, n);
			else	if (fdout_add(buf, n))	conv_err=1;
			ps += n;
			if (conv_err)	break;
		}
		if (convmode == RFC2045_RW_7BIT)
		{
			rfc2045_cdecode_end(p);
			if (libmail_encode_end(&qp_encode))
				conv_err=1;
		}
		if (convmode == RFC2045_RW_8BIT)
		{
			rfc2045_cdecode_end(p);
		}
		if (conv_err)	return (-1);
		return (0);
	}

	bcnt=rw_boundary_cnt;

	/* Sam 8/30/99 fix - handle message/rfc822:

            --boundary
            Content-Type: message/rfc822

         --><-- we're here, DON'T add RFC2045MIMEMSG and rest of crap here
	*/
	if (p->firstpart->next == 0)
	{
	int	rc;

		p->firstpart->parent=0;
		rc=dorw(p->firstpart, src);
		p->firstpart->parent=p;
		return (rc);
	}

	if (fdout_add(RFC2045MIMEMSG, sizeof(RFC2045MIMEMSG)-1))
		return (-1);
	for (p=p->firstpart; p; p=p->next)
	{
		if (p->isdummy)	continue;
		sprintf(buf, "\n--%s-%d\n", rw_boundary_root, bcnt);
		if (fdout_add(buf, strlen(buf)))	return (-1);
		if (dorw(p, src) != 0)	return(-1);
	}
	sprintf(buf, "\n--%s-%d--\n", rw_boundary_root, bcnt);
	if (fdout_add(buf, strlen(buf)))	return (-1);
	return (0);
}

static int rfc2045_rewrite_common(struct rfc2045 *, struct rfc2045src *src,
				  const char *);

int rfc2045_rewrite(struct rfc2045 *p, struct rfc2045src *src, int fdout_arg,
		    const char *appname)
{
	fdout=fdout_arg;
	fdout_func=0;
	return (rfc2045_rewrite_common(p, src, appname));
}

int rfc2045_rewrite_func(struct rfc2045 *p, struct rfc2045src *src,
	int (*funcarg)(const char *, int, void *), void *funcargarg,
	const char *appname)
{
	fdout= -1;
	fdout_func=funcarg;
	fdout_arg=funcargarg;
	return (rfc2045_rewrite_common(p, src, appname));
}

static int rfc2045_rewrite_common(struct rfc2045 *p,
				   struct rfc2045src *src, const char *appname)
{
	int	rc;

	rw_appname=appname;

	fdout_ptr=fdout_buf;
	fdout_left=sizeof(fdout_buf);

	rw_boundary_root=rfc2045_mk_boundary(p, src);
	if (rw_boundary_root == 0)
		rc= -1;
	else
	{
		rw_boundary_cnt=1;
		rc=dorw(p, src);
		free(rw_boundary_root);
	}
	if (rc == 0 && fdout_ptr > fdout_buf
	    && fdout_ptr[-1] != '\n')
	{
		fdout_add("\n", 1);
	}
	if (rc == 0 && fdout_ptr > fdout_buf)
		rc=fdout_flush();
	return (rc);
}
