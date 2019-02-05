/*
** Copyright 2003-2007 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include "config.h"
#include "tlspasswordcache.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <md5/md5.h>

#define PASSFILEFORMAT 1

#if HAVE_OPENSSL097
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

static void sslerror(EVP_CIPHER_CTX *ctx, const char *pfix)
{
        char errmsg[256];
        int errnum=ERR_get_error();

        ERR_error_string_n(errnum, errmsg, sizeof(errmsg)-1);

	fprintf(stderr, "%s: %s\n", pfix, errmsg);
}


#endif

#if HAVE_OPENSSL110
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

static void sslerror(EVP_CIPHER_CTX *ctx, const char *pfix)
{
        char errmsg[256];
        int errnum=ERR_get_error();

        ERR_error_string_n(errnum, errmsg, sizeof(errmsg)-1);

	fprintf(stderr, "%s: %s\n", pfix, errmsg);
}


#endif

#if HAVE_GCRYPT

#include <gcrypt.h>

#define RAND_pseudo_bytes(a,b) (gcry_create_nonce((a),(b)), 0)

typedef struct {
	enum gcry_cipher_algos algo;
	enum gcry_cipher_modes mode;
} EVP_CIPHER;

#define EVP_MAX_IV_LENGTH 256

const EVP_CIPHER *EVP_des_cbc()
{
	static const EVP_CIPHER des_cbc={GCRY_CIPHER_DES,
					 GCRY_CIPHER_MODE_CBC};

	return &des_cbc;
}

typedef struct {
	const EVP_CIPHER *cipher;
	gcry_error_t err;
	gcry_cipher_hd_t handle;

	int padding;
	char *blkbuf;
	size_t blksize;

	size_t blkptr;

} EVP_CIPHER_CTX;

static void sslerror(EVP_CIPHER_CTX *ctx, const char *pfix)
{
	fprintf(stderr, "%s: %s\n", pfix, gcry_strerror(ctx->err));
}

static void EVP_CIPHER_CTX_init(EVP_CIPHER_CTX *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
}

static void EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *ctx)
{
	if (ctx->handle)
	{
		gcry_cipher_close(ctx->handle);
		ctx->handle=NULL;
	}

	if (ctx->blkbuf)
	{
		free(ctx->blkbuf);
		ctx->blkbuf=NULL;
	}
}

static int EVP_CIPHER_iv_length(const EVP_CIPHER *cipher)
{
	size_t l=0;

	gcry_cipher_algo_info(cipher->algo, GCRYCTL_GET_BLKLEN, NULL, &l);
	return l;
}

static int EVP_CIPHER_key_length(const EVP_CIPHER *cipher)
{
	size_t l=0;

	gcry_cipher_algo_info(cipher->algo, GCRYCTL_GET_KEYLEN, NULL, &l);
	return l;
}

static int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
			      void *impl, unsigned char *key, unsigned char *iv)
{
	EVP_CIPHER_CTX_cleanup(ctx);
	ctx->cipher=cipher;
	ctx->err=gcry_cipher_open(&ctx->handle,
				  cipher->algo,
				  cipher->mode, 0);

	if (!ctx->err)
		ctx->err=gcry_cipher_setkey(ctx->handle, key,
					    EVP_CIPHER_key_length(cipher));

	if (!ctx->err)
		ctx->err=gcry_cipher_setiv(ctx->handle, iv,
					   (ctx->blksize=
					    EVP_CIPHER_iv_length(cipher)));

	if (!ctx->err)
		if ((ctx->blkbuf=malloc(ctx->blksize)) == NULL)
			ctx->err=gpg_err_code_from_errno(errno);

	ctx->blkptr=0;
	ctx->padding=1;

	return ctx->err == 0;
}

static int EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out,
			     int *outl, unsigned char *in, int inl)
{
	*outl=0;

	while (inl > 0)
	{
		size_t cp= (size_t)inl < (ctx->blksize - ctx->blkptr)
			? (size_t)inl:(ctx->blksize - ctx->blkptr);

		if (ctx->blkptr == 0 && inl > ctx->blksize*2)
		{
			cp=(inl / ctx->blksize - 1) * ctx->blksize;

			if ((ctx->err=gcry_cipher_encrypt(ctx->handle,
							  out, cp,
							  in, cp))
			    != 0)
				return 0;

			out += cp;
			*outl += cp;
			in += cp;
			inl -= cp;
			continue;
		}

		memcpy(ctx->blkbuf + ctx->blkptr, in, cp);

		in += cp;
		inl -= cp;

		ctx->blkptr += cp;

		if (ctx->blkptr == ctx->blksize)
		{
			if ((ctx->err=gcry_cipher_encrypt(ctx->handle,
							  out, ctx->blksize,
							  ctx->blkbuf,
							  ctx->blksize)) != 0)
				return 0;
			out += ctx->blksize;
			*outl += ctx->blksize;
			ctx->blkptr=0;
		}
	}
	return 1;
}

static int EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out,
			       int *outl)
{
	if (ctx->padding)
	{
		unsigned char pad=ctx->blksize - ctx->blkptr;

		*outl=0;

		if (pad == 0)
			pad=ctx->blksize;

		do
		{
			int n_outl;

			if (!EVP_EncryptUpdate(ctx, out, &n_outl, &pad, 1))
				return 0;

			out += n_outl;
			*outl += n_outl;
		}
		while (ctx->blkptr);
	}
	else if (ctx->blksize != ctx->blkptr)
	{
		ctx->err=GPG_ERR_BAD_DATA;
		return 0;
	}

	return 1;
}

static int EVP_DecryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *type,
			      void *impl, unsigned char *key,
			      unsigned char *iv)
{
	return EVP_EncryptInit_ex(ctx, type, impl, key, iv);
}

static int EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out,
			     int *outl, unsigned char *in, int inl)
{
	*outl=0;

	while (inl > 0)
	{
		size_t cp;

		if (ctx->blkptr == 0 && inl > ctx->blksize * 3)
		{
			cp=(inl / ctx->blksize - 2) * ctx->blksize;

			if ((ctx->err=gcry_cipher_decrypt(ctx->handle,
							  out, cp,
							  in, cp))
			    != 0)
				return 0;

			out += cp;
			*outl += cp;
			in += cp;
			inl -= cp;
			continue;
		}

		if (ctx->blkptr == ctx->blksize)
		{
			if ((ctx->err=gcry_cipher_decrypt(ctx->handle,
							  out, ctx->blksize,
							  ctx->blkbuf,
							  ctx->blksize)) != 0)
				return 0;
			out += ctx->blksize;
			*outl += ctx->blksize;
			ctx->blkptr=0;
		}

		cp= (size_t)inl < (ctx->blksize - ctx->blkptr)
			? (size_t)inl:(ctx->blksize - ctx->blkptr);

		memcpy(ctx->blkbuf + ctx->blkptr, in, cp);

		in += cp;
		inl -= cp;

		ctx->blkptr += cp;

	}
	return 1;
}

static int EVP_DecryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *outm,
			       int *outl)
{
	unsigned char lastval;
	int cnt;

	if (ctx->blkptr != ctx->blksize)
	{
		ctx->err=GPG_ERR_BAD_DATA;
		return 0;
	}

	if ((ctx->err=gcry_cipher_decrypt(ctx->handle,
					  ctx->blkbuf,
					  ctx->blksize,
					  NULL, 0)) != 0)
		return 0;

	if (ctx->padding)
	{
		lastval=ctx->blkbuf[ctx->blksize-1];

		if (lastval > 0 && lastval <= ctx->blksize)
		{
			char n;

			for (n=0; n<lastval; n++)
				if (ctx->blkbuf[ctx->blksize-1-n] != lastval)
					lastval=0;
		}
		else
			lastval=0;

		if (!lastval)
		{
			ctx->err=GPG_ERR_BAD_DATA;
			return 0;
		}
	}
	else
	{
		lastval=0;
	}

	cnt=ctx->blksize-lastval;
	if (cnt)
		memcpy(outm, ctx->blkbuf, cnt);
	*outl=cnt;
	return 1;
}


#define HAVE_OPENSSL097 1
#endif


#if HAVE_OPENSSL110

#define RANDOM_BYTES RAND_bytes

typedef EVP_CIPHER_CTX *CIPHER_CONTEXT;

#define CIPHER_INIT(p) (p=EVP_CIPHER_CTX_new())
#define CIPHER_CLEANUP(p) (EVP_CIPHER_CTX_free(p))
#define HAVE_OPENSSL097 1
#define RANDOM_BYTES RAND_bytes
#define CONTEXT(ctx) (*(ctx))
#else

typedef EVP_CIPHER_CTX CIPHER_CONTEXT;

#define CIPHER_INIT(p) EVP_CIPHER_CTX_init(&p)
#define CIPHER_CLEANUP(p) EVP_CIPHER_CTX_cleanup(&p)
#define RANDOM_BYTES RAND_pseudo_bytes

#define CONTEXT(ctx) (ctx)
#endif


#if HAVE_OPENSSL097

#if BUFSIZ < 8192
#undef BUFSIZ
#define BUFSIZ 8192
#endif

int tlspassword_init()
{
	return 1;
}

static int save_string(CIPHER_CONTEXT *,
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

	unsigned char iv1_buf[16];
	unsigned char iv2_buf[16];
	MD5_DIGEST md5_password;
	int iv_len, key_len;
	CIPHER_CONTEXT ctx;
	const EVP_CIPHER *des=EVP_des_cbc();

	md5_digest(mpw, strlen(mpw), md5_password);

	CIPHER_INIT(ctx);
	iv_len=EVP_CIPHER_iv_length(des);
	key_len=EVP_CIPHER_key_length(des);

	if (RANDOM_BYTES(iv1_buf, sizeof(iv1_buf)) < 0 ||
	    RANDOM_BYTES(iv2_buf, sizeof(iv2_buf)) < 0)
	{
		fprintf(stderr,
			"tlspassword_save: internal error - "
			"RANDOM_BYTES() failed.\n");
		CIPHER_CLEANUP(ctx);
		errno=EIO;
		return -1;
	}

	if (iv_len + key_len > sizeof(iv1_buf)
	    || iv_len + key_len != sizeof(iv2_buf)
	    || key_len != sizeof(md5_password)/2)
	{
		fprintf(stderr,
			"tlspassword_save: internal error - "
			"unexpected key sizes.\n");
		CIPHER_CLEANUP(ctx);
		errno=EIO;
		return -1;
	}

	p=buf+3;

	if (!EVP_EncryptInit_ex(CONTEXT(&ctx), des, NULL,
				(unsigned char *)md5_password,
				iv1_buf) ||
	    !EVP_EncryptUpdate(CONTEXT(&ctx), (unsigned char *)p, &l,
			       (unsigned char *)md5_password + key_len,
			       sizeof(md5_password)-key_len) ||
	    !EVP_EncryptUpdate(CONTEXT(&ctx), (unsigned char *)(p += l), &l,
			       iv2_buf,
			       iv_len + key_len) ||
	    !EVP_EncryptFinal_ex(CONTEXT(&ctx), (unsigned char *)(p += l), &l))

	{
		sslerror(CONTEXT(&ctx), "EVP_EncryptInit_ex");
		CIPHER_CLEANUP(ctx);
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
		l=(*writefunc)((const char *)iv1_buf, iv_len, writefuncarg);

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

	if (!EVP_EncryptInit_ex(CONTEXT(&ctx), des, NULL,
				(unsigned char *)&iv2_buf,
				(unsigned char *)&iv2_buf + key_len))
	{
		sslerror(CONTEXT(&ctx), "EVP_EncryptInit_ex");
		CIPHER_CLEANUP(ctx);
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

	if (!EVP_EncryptFinal_ex(CONTEXT(&ctx), (unsigned char *)buf, &l))
	{
		sslerror(CONTEXT(&ctx), "EVP_EncryptInit_ex");
		CIPHER_CLEANUP(ctx);
		errno=EIO;
		return -1;
	}

	if (l)
		l=(*writefunc)(buf, l, writefuncarg);

	CIPHER_CLEANUP(ctx);
	return l;
}

static int save_string(CIPHER_CONTEXT *ctx,
		       const char *str, char *buf,
		       int (*writefunc)(const char *, size_t, void *),
		       void *writefuncarg)
{
	int l;
	size_t len=strlen(str);
	unsigned char b[2];

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

	if (!EVP_EncryptUpdate(CONTEXT(ctx), (unsigned char *)buf, &l, b, 2))
	{
		sslerror(CONTEXT(ctx), "EVP_EncryptUpdate");
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

		if (!EVP_EncryptUpdate(CONTEXT(ctx), (unsigned char *)buf, &l,
				       (unsigned char *)str, n))
		{
			sslerror(CONTEXT(ctx), "EVP_EncryptUpdate");
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
	CIPHER_CONTEXT ctx;
	const EVP_CIPHER *des=EVP_des_cbc();
	struct tlspassword_readinfo readinfo;
	char header[3];
	size_t l;
	char iv1_buf[EVP_MAX_IV_LENGTH];
	struct tempstring_list *tl;
	const char **urls, **pws;

	readinfo.bufleft=0;
	readinfo.readfunc=callback;
	readinfo.readfuncarg=callback_arg;
	readinfo.tl_list=NULL;
	readinfo.tl_last=NULL;

	md5_digest(mpw, strlen(mpw), md5_password);

	CIPHER_INIT(ctx);
	iv_len=EVP_CIPHER_iv_length(des);
	key_len=EVP_CIPHER_key_length(des);

	if (tlspassword_read(&readinfo, header, 3) ||
	    tlspassword_read(&readinfo, iv1_buf, iv_len))
	{
		CIPHER_CLEANUP(ctx);
		return -1;
	}
	if (header[0] != PASSFILEFORMAT)
	{
		errno=EINVAL;
		CIPHER_CLEANUP(ctx);
		return -1;
	}

	if ((l=(size_t)(unsigned char)header[1] * 256
	     + (unsigned char)header[2]) > sizeof(buf) / 4)
	{
		errno=EINVAL;
		CIPHER_CLEANUP(ctx);
		return -1;
	}

	if (tlspassword_read(&readinfo, buf, l))
		return -1;

	p=buf + sizeof(buf)/2;
	if (!EVP_DecryptInit_ex(CONTEXT(&ctx), des, NULL,
				(unsigned char *)md5_password,
				(unsigned char *)&iv1_buf) ||
	    !EVP_DecryptUpdate(CONTEXT(&ctx), (unsigned char *)p, &outl,
			       (unsigned char *)buf, l) ||
	    !EVP_DecryptFinal_ex(CONTEXT(&ctx), (unsigned char *)(p += outl), &outl))
	{
		errno=EINVAL;
		CIPHER_CLEANUP(ctx);
		return -1;
	}

	p += outl;

	if (p - (buf +sizeof(buf)/2) != sizeof(md5_password) + iv_len
	    || memcmp(buf + sizeof(buf)/2, (char *)(&md5_password) + key_len,
		      sizeof(md5_password)-key_len))
	{
		errno=EINVAL;
		CIPHER_CLEANUP(ctx);
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

	if (!EVP_DecryptInit_ex(CONTEXT(&ctx), des, NULL,
				(unsigned char *)(p-iv_len-key_len),
				(unsigned char *)(p-iv_len)))
	{
		errno=EINVAL;
		CIPHER_CLEANUP(ctx);
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
				CIPHER_CLEANUP(ctx);
				return -1;
			}

			readinfo.bufptr=readinfo.buf;
			readinfo.bufleft=outl;
		}

		if (!EVP_DecryptUpdate(CONTEXT(&ctx), (unsigned char *)buf, &outl,
				       (unsigned char *)
				       readinfo.bufptr, readinfo.bufleft))
		{
			tlspassword_readcleanup(&readinfo);
			errno=EINVAL;
			CIPHER_CLEANUP(ctx);
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
				CIPHER_CLEANUP(ctx);
				return -1;
			}

			p += n;
			outl -= n;
		}
	}

	if (!EVP_DecryptFinal_ex(CONTEXT(&ctx), (unsigned char *)buf, &outl))
	{
		tlspassword_readcleanup(&readinfo);
		errno=EINVAL;
		CIPHER_CLEANUP(ctx);
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
			CIPHER_CLEANUP(ctx);
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
		CIPHER_CLEANUP(ctx);
		return (-1);
	}

	if ((urls=malloc((readinfo.nstrings+1) * sizeof(char *))) == NULL ||
	    (pws=malloc((readinfo.nstrings+1) * sizeof(char *))) == NULL)
	{
		if (urls)
			free(urls);

		tlspassword_readcleanup(&readinfo);
		CIPHER_CLEANUP(ctx);
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
	CIPHER_CLEANUP(ctx);
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
