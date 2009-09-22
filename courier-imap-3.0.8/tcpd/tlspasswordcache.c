/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "config.h"
#include "tlspasswordcache.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <md5/md5.h>

#define PASSFILEFORMAT 1

#if HAVE_OPENSSL097

static void sslerror(const char *pfix)
{
        char errmsg[256];
        int errnum=ERR_get_error();
 
        ERR_error_string_n(errnum, errmsg, sizeof(errmsg)-1);

	fprintf(stderr, "%s: %s\n", pfix, errmsg);
}

int tlspassword_init()
{
	return 1;
}

static int save_string(EVP_CIPHER_CTX *,
		       const char *, char *,
		       int (*)(const char *, size_t, void *),
		       void *);

int tlspassword_save( const char * const *urls,
		      const char * const *pwds,
		      const char *mpw,
		      int (*writefunc)(const char *, size_t, void *),
		      void *writefuncarg)
{
	char buf[BUFSIZ];
	char *p;
	int l;
	int wl;

	char iv1_buf[16];
	char iv2_buf[16];
	MD5_DIGEST md5_password;
	int iv_len, key_len;
	EVP_CIPHER_CTX ctx;
	const EVP_CIPHER *des=EVP_des_cbc();

	md5_digest(mpw, strlen(mpw), md5_password);

	EVP_CIPHER_CTX_init(&ctx);
	iv_len=EVP_CIPHER_iv_length(des);
	key_len=EVP_CIPHER_key_length(des);

	if (RAND_pseudo_bytes(iv1_buf, sizeof(iv1_buf)) < 0 ||
	    RAND_pseudo_bytes(iv2_buf, sizeof(iv2_buf)) < 0)
	{
		fprintf(stderr,
			"tlspassword_save: internal error - "
			"RAND_pseudo_bytes() failed.\n");
		EVP_CIPHER_CTX_cleanup(&ctx);
		errno=EIO;
		return -1;
	}

	if (iv_len + key_len > sizeof(iv1_buf)
	    || key_len > sizeof(md5_password)-1)
	{
		fprintf(stderr,
			"tlspassword_save: internal error - "
			"key sizes too large.\n");
		EVP_CIPHER_CTX_cleanup(&ctx);
		errno=EIO;
		return -1;
	}

	p=buf+3;

	if (!EVP_EncryptInit_ex(&ctx, des, NULL,
				(unsigned char *)md5_password,
				(unsigned char *)&iv1_buf) ||
	    !EVP_EncryptUpdate(&ctx, p, &l,
			       (unsigned char *)md5_password + key_len,
			       sizeof(md5_password)-key_len) ||
	    !EVP_EncryptUpdate(&ctx, p += l, &l,
			       (unsigned char *)&iv2_buf,
			       iv_len + key_len) ||
	    !EVP_EncryptFinal_ex(&ctx, p += l, &l))

	{
		sslerror("EVP_EncryptInit_ex");
		EVP_CIPHER_CTX_cleanup(&ctx);
		errno=EIO;
		return -1;
	}

	p += l;

	wl= p - buf - 3;

	buf[0]=PASSFILEFORMAT;
	buf[1]= wl / 256;
	buf[2]= wl % 256;

	l=(*writefunc)(buf, 3, writefuncarg);

	if (l == 0)
		l=(*writefunc)(iv1_buf, iv_len, writefuncarg);

	if (l == 0)
		l=(*writefunc)(buf+3, wl, writefuncarg);

	if (l)
		return l;

#if 0
	{
		int i;

		printf("KEY: ");

		for (i=0; i<key_len + iv_len; i++)
			printf("%02X", (int)(unsigned char)iv2_buf[i]);
		printf("\n");
	}
#endif

	if (!EVP_EncryptInit_ex(&ctx, des, NULL,
				(unsigned char *)&iv2_buf,
				(unsigned char *)&iv2_buf + key_len))
	{
		sslerror("EVP_EncryptInit_ex");
		EVP_CIPHER_CTX_cleanup(&ctx);
		errno=EIO;
		return -1;
	}

	for (l=0; urls[l]; l++)
	{
		int n=save_string(&ctx, urls[l], buf, writefunc, writefuncarg);

		if (n)
			return n;

		n=save_string(&ctx, pwds[l], buf, writefunc, writefuncarg);

		if (n)
			return n;
	}

	if (!EVP_EncryptFinal_ex(&ctx, buf, &l))
	{
		sslerror("EVP_EncryptInit_ex");
		EVP_CIPHER_CTX_cleanup(&ctx);
		errno=EIO;
		return -1;
	}

	if (l)
		l=(*writefunc)(buf, l, writefuncarg);

	EVP_CIPHER_CTX_cleanup(&ctx);
	return l;
}

static int save_string(EVP_CIPHER_CTX *ctx,
		       const char *str, char *buf,
		       int (*writefunc)(const char *, size_t, void *),
		       void *writefuncarg)
{
	int l;
	size_t len=strlen(str);
	char b[2];

	if (len >= 256 * 256)
	{
		fprintf(stderr,
			"tlspassword_save: internal error - "
			"key sizes too large.\n");
		errno=EINVAL;
		return -1;
	}

	b[0]=len / 256;
	b[1]=len % 256;

	if (!EVP_EncryptUpdate(ctx, buf, &l, b, 2))
	{
		sslerror("EVP_EncryptUpdate");
		return -1;
	}

	if (l)
	{
		l=(*writefunc)(buf, l, writefuncarg);

		if (l)
			return l;
	}

	while (len)
	{
		size_t n=len;

		if (n > BUFSIZ / 4)
			n=BUFSIZ/4;

		if (!EVP_EncryptUpdate(ctx, buf, &l, str, n))
		{
			sslerror("EVP_EncryptUpdate");
			return -1;
		}

		if (l)
		{
			l=(*writefunc)(buf, l, writefuncarg);

			if (l)
				return l;
		}

		str += n;
		len -= n;
	}

	return 0;
}

struct tempstring_list {
	struct tempstring_list *next;
	char *url;
	char *pw;
};

struct tlspassword_readinfo {
	char buf[BUFSIZ / 2];
	char *bufptr;
	size_t bufleft;

	int (*readfunc)(char *, size_t, void *);
	void *readfuncarg;

	struct tempstring_list *tl_list, *tl_last;

	int (*readhandler)(struct tlspassword_readinfo *, char *, int);

	unsigned int stringhi;
	char *stringptr;
	size_t stringleft;
	size_t nstrings;
};


static int tlspassword_read(struct tlspassword_readinfo *p,
			    char *buf,
			    size_t nbytes)
{
	while (nbytes)
	{
		size_t c;

		if (p->bufleft == 0)
		{
			int n= (*p->readfunc)(p->buf, sizeof(p->buf),
					      p->readfuncarg);

			if (n <= 0)
				return -1;
			p->bufptr=p->buf;
			p->bufleft=n;
		}

		c=nbytes;

		if (c > p->bufleft)
			c=p->bufleft;

		memcpy(buf, p->bufptr, c);
		p->bufptr += c;
		p->bufleft -= c;
		nbytes -= c;
	}

	return 0;
}

static void tlspassword_readcleanup(struct tlspassword_readinfo *p)
{
	while (p->tl_list)
	{
		struct tempstring_list *t=p->tl_list;

		p->tl_list=t->next;
		if (t->url)
			free(t->url);
		if (t->pw)
			free(t->pw);
		free(t);
	}
}

static int read_stringhi(struct tlspassword_readinfo *, char *, int);

int tlspassword_load( int (*callback)(char *, size_t, void *),
		      void *callback_arg,

		      const char *mpw,

		      void (*readfunc)(const char * const *,
				       const char * const *,
				       void *),
		      void *readfunc_arg)
{
	char buf[BUFSIZ];
	int outl;
	char *p;

	MD5_DIGEST md5_password;
	int iv_len, key_len;
	EVP_CIPHER_CTX ctx;
	const EVP_CIPHER *des=EVP_des_cbc();
	struct tlspassword_readinfo readinfo;
	char header[3];
	size_t l;
	unsigned char iv1_buf[EVP_MAX_IV_LENGTH];
	struct tempstring_list *tl;
	const char **urls, **pws;

	readinfo.bufleft=0;
	readinfo.readfunc=callback;
	readinfo.readfuncarg=callback_arg;
	readinfo.tl_list=NULL;
	readinfo.tl_last=NULL;


	md5_digest(mpw, strlen(mpw), md5_password);

	EVP_CIPHER_CTX_init(&ctx);
	iv_len=EVP_CIPHER_iv_length(des);
	key_len=EVP_CIPHER_key_length(des);

	if (tlspassword_read(&readinfo, header, 3) ||
	    tlspassword_read(&readinfo, iv1_buf, iv_len))
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}
	if (header[0] != PASSFILEFORMAT)
	{
		errno=EINVAL;
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	if ((l=(size_t)(unsigned char)header[1] * 256
	     + (unsigned char)header[2]) > sizeof(buf) / 4)
	{
		errno=EINVAL;
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	if (tlspassword_read(&readinfo, buf, l))
		return -1;

	p=buf + sizeof(buf)/2;
	if (!EVP_DecryptInit_ex(&ctx, des, NULL,
				(unsigned char *)md5_password,
				(unsigned char *)&iv1_buf) ||
	    !EVP_DecryptUpdate(&ctx, p, &outl, buf, l) ||
	    !EVP_DecryptFinal_ex(&ctx, p += outl, &outl))
	{
		errno=EINVAL;
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	p += outl;

	if (p - (buf +sizeof(buf)/2) != sizeof(md5_password) + iv_len
	    || memcmp(buf + sizeof(buf)/2, (char *)(&md5_password) + key_len,
		      sizeof(md5_password)-key_len))
	{
		errno=EINVAL;
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

#if 0
	{
		int i;

		printf("KEY: ");

		for (i=0; i<key_len + iv_len; i++)
			printf("%02X", (int)(unsigned char)(p-iv_len-key_len)[i]);
		printf("\n");
	}
#endif

	if (!EVP_DecryptInit_ex(&ctx, des, NULL,
				(unsigned char *)(p-iv_len-key_len),
				(unsigned char *)(p-iv_len)))
	{
		errno=EINVAL;
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	readinfo.nstrings=0;
	readinfo.readhandler= &read_stringhi;
	for (;;)
	{
		if (readinfo.bufleft == 0)
		{
			outl= (*readinfo.readfunc)(readinfo.buf,
						   sizeof(readinfo.buf),
						   readinfo.readfuncarg);

			if (outl == 0)
				break;

			if (outl < 0)
			{
				tlspassword_readcleanup(&readinfo);
				errno=EINVAL;
				EVP_CIPHER_CTX_cleanup(&ctx);
				return -1;
			}

			readinfo.bufptr=readinfo.buf;
			readinfo.bufleft=outl;
		}

		if (!EVP_DecryptUpdate(&ctx, buf, &outl,
				       readinfo.bufptr, readinfo.bufleft))
		{
			tlspassword_readcleanup(&readinfo);
			errno=EINVAL;
			EVP_CIPHER_CTX_cleanup(&ctx);
			return -1;
		}
		readinfo.bufleft=0;

		p=buf;
		while (outl)
		{
			int n= (*readinfo.readhandler)(&readinfo, p, outl);

			if (n < 0)
			{
				tlspassword_readcleanup(&readinfo);
				EVP_CIPHER_CTX_cleanup(&ctx);
				return -1;
			}

			p += n;
			outl -= n;
		}
	}

	if (!EVP_DecryptFinal_ex(&ctx, buf, &outl))
	{
		tlspassword_readcleanup(&readinfo);
		errno=EINVAL;
		EVP_CIPHER_CTX_cleanup(&ctx);
		return -1;
	}

	p=buf;
	while (outl)
	{
		int n= (*readinfo.readhandler)(&readinfo, p, outl);

		if (n < 0)
		{
			tlspassword_readcleanup(&readinfo);
			errno=EINVAL;
			EVP_CIPHER_CTX_cleanup(&ctx);
			return -1;
		}

		p += n;
		outl -= n;
	}

	if (readinfo.tl_list && readinfo.tl_list->pw == NULL)
		/* Odd # of strings -- no good */
	{
		tlspassword_readcleanup(&readinfo);
		errno=EINVAL;
		EVP_CIPHER_CTX_cleanup(&ctx);
		return (-1);
	}

	if ((urls=malloc((readinfo.nstrings+1) * sizeof(char *))) == NULL ||
	    (pws=malloc((readinfo.nstrings+1) * sizeof(char *))) == NULL)
	{
		if (urls)
			free(urls);

		tlspassword_readcleanup(&readinfo);
		EVP_CIPHER_CTX_cleanup(&ctx);
		return (-1);
	}

	l=0;
	for (tl=readinfo.tl_list; tl; tl=tl->next)
	{
		urls[l]=tl->url;
		pws[l]=tl->pw;
		l++;
	}

	urls[l]=NULL;
	pws[l]=NULL;

	(*readfunc)(urls, pws, readfunc_arg);

	free(urls);
	free(pws);

	tlspassword_readcleanup(&readinfo);
	EVP_CIPHER_CTX_cleanup(&ctx);
	return 0;
}

static int read_stringlo(struct tlspassword_readinfo *info,
			 char *p, int n);

static int read_string(struct tlspassword_readinfo *info,
		       char *p, int n);

static int read_stringhi(struct tlspassword_readinfo *info,
			 char *p, int n)
{
	info->stringhi=(unsigned char)*p;
	info->stringhi *= 256;

	info->readhandler=read_stringlo;
	return 1;
}

static int read_stringlo(struct tlspassword_readinfo *info,
			 char *p, int n)
{
	struct tempstring_list *t;

	info->readhandler=read_string;
	info->stringleft=info->stringhi + (unsigned char)*p;

	if (info->tl_last &&
	    info->tl_last->pw == NULL) /* This string is the pw */
	{
		info->tl_last->pw=malloc(info->stringleft+1);
		if (!info->tl_last->pw)
			return -1;

		info->stringptr=info->tl_last->pw;
		return 1;
	}

	if ((t=(struct tempstring_list *)malloc(sizeof(struct tempstring_list))
	     ) == NULL || (t->url=malloc(info->stringleft+1)) == NULL)
	{
		if (t) free(t);
		return -1;
	}

	if (info->tl_last)
		info->tl_last->next=t;
	else
		info->tl_list=t;
	info->tl_last=t;
	info->stringptr=t->url;
	t->next=NULL;
	t->pw=NULL;
	++info->nstrings;
	return 1;
}

static int read_string(struct tlspassword_readinfo *info, char *p, int n)
{
	if (n > info->stringleft)
		n=info->stringleft;

	memcpy(info->stringptr, p, n);
	info->stringptr += n;
	info->stringleft -= n;

	if (info->stringleft == 0)
	{
		info->readhandler=read_stringhi;
		*info->stringptr=0;
	}

	return n;
}

#else


int tlspassword_init()
{
	return 0;
}


int tlspassword_save( const char * const *urls,
		      const char * const *pwds,
		      const char *mpw,
		      int (*writefunc)(const char *, size_t, void *),
		      void *writefuncarg)
{
	errno=EIO;
	return -1;
}

int tlspassword_load( int (*readfunc)(char *, size_t, void *),
		      void *readfuncarg,

		      const char *mpw,
		      void (*callback)(const char * const *,
				       const char * const *,
				       void *),
		      void *callback_arg)
{
	errno=EIO;
	return -1;
}
#endif
