/*
** Copyright 2000-2018 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"courier-unicode.h"
#include	<string.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<iconv.h>
#include	<errno.h>
#include	<locale.h>
#if	USE_LIBCHARSET
#if	HAVE_LOCALCHARSET_H
#include	<localcharset.h>
#elif	HAVE_LIBCHARSET_H
#include	<libcharset.h>
#endif	/* HAVE_LOCALCHARSET_H */
#else
#include	<langinfo.h>
#endif	/* USE_LIBCHARSET */

static char default_chset_buf[32];
static const char *fix_charset(const char *);

const char *unicode_locale_chset()
{
	const char *c;

#if	USE_LIBCHARSET
	c=locale_charset();
#else
	c=nl_langinfo(CODESET);
#endif
	return fix_charset(c);
}

#if HAVE_LANGINFO_L
const char *unicode_locale_chset_l(locale_t l)
{
	return fix_charset(nl_langinfo_l(CODESET, l));
}
#endif

static const char *fix_charset(const char *c)
{
	if (!c)
		c="US-ASCII";

	if (c &&

	    /* Map GNU libc iconv oddity to us-ascii */

	    (strcmp(c, "ANSI_X3.4") == 0 ||
	     strncmp(c, "ANSI_X3.4-", 10) == 0))
		c="US-ASCII";
	return c;
}

static void init_default_chset()
{
	const char *old_locale=NULL;
	const char *chset=NULL;
	char *locale_cpy=NULL;
	char buf[sizeof(default_chset_buf)];

	chset=getenv("MM_CHARSET");

	if (chset == NULL)
		chset=getenv("CHARSET");

	if (chset == NULL)
	{
		old_locale=setlocale(LC_ALL, "");
		locale_cpy=old_locale ? strdup(old_locale):NULL;
		chset=unicode_locale_chset();
	}

	memset(buf, 0, sizeof(buf));

	strncat(buf, chset, sizeof(buf)-1);

	memcpy(default_chset_buf, buf, sizeof(buf));

	if (locale_cpy)
	{
		setlocale(LC_ALL, locale_cpy);
		free(locale_cpy);
	}
}

const char *unicode_default_chset()
{
	if (default_chset_buf[0] == 0)
		init_default_chset();

	return default_chset_buf;
}


/*****************************************************************************/

const char unicode_u_ucs4_native[]=
#if WORDS_BIGENDIAN
	"UCS-4BE"
#else
	"UCS-4LE"
#endif
	;

const char unicode_u_ucs2_native[]=
#if WORDS_BIGENDIAN
	"UCS-2BE"
#else
	"UCS-2LE"
#endif
	;

/* A stack of conversion modules */

struct unicode_convert_hdr {

	int (*convert_handler)(void *ptr,
			       const char *text, size_t cnt);
	int (*deinit_handler)(void *ptr, int *errptr);
	void *ptr;

	struct unicode_convert_hdr *next;
};

/* Decoding table for modified UTF7-encoding as used in imap */

static const char mbase64_lookup[]={
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,63,-1,-1,-1,
	52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
	-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
	-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

static const char mbase64[]=
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

/*
** Conversion wrapper for converting to modified-utf7 IMAP encoding.
**
** This is done by converting to UCS2, then stacking on a module that
** takes that and converts UCS2 to modified-UTF7.
**
** init_nottoimaputf7() returns an opaque stack for converting to ucs2.
*/

static unicode_convert_handle_t
init_nottoimaputf7(const char *src_chset,
		   const char *dst_chset,
		   int (*output_func)(const char *, size_t, void *),
		   void *convert_arg);

/*
** The to modified UTF7 module
*/

struct unicode_convert_toimaputf7 {

	struct unicode_convert_hdr hdr;

	/* Accumulated output buffer */

	char utf7encodebuf[1024];
	size_t utf7encodebuf_cnt;

	/* Accumulated bits for base64 encoding */
	uint32_t utf7bits;

	/* How many bits in utf7bits */
	uint16_t utf7bitcount;

	/* Flag: in base64mode */
	uint16_t utfmode;

	int errflag;

	/* Any extra characters that should be munged */

	char smapmunge[16];

	/* Remembered output function */

	int (*output_func)(const char *, size_t, void *);

	/* Remembered arg to the output function */
	void *convert_arg;
};

/* Macro - flush the output buffer */
#define toimaputf7_encode_flush(p) do {					\
		int rc;							\
									\
		rc=(*(p)->output_func)((p)->utf7encodebuf,		\
				       (p)->utf7encodebuf_cnt,		\
				       (p)->convert_arg);		\
		if (rc)							\
			return ((p)->errflag=(rc));			\
									\
		(p)->utf7encodebuf_cnt=0;				\
	} while (0)

static int toimaputf7_encode_flushfinal(struct unicode_convert_toimaputf7 *p)
{
	if (p->utf7encodebuf_cnt > 0)
		toimaputf7_encode_flush(p);
	return 0;
}

/* Macro - add one char to the output buffer */

#define toimaputf7_encode_add(p,c) do {					\
		if ((p)->utf7encodebuf_cnt >= sizeof((p)->utf7encodebuf)) \
			toimaputf7_encode_flush((p));			\
									\
		(p)->utf7encodebuf[(p)->utf7encodebuf_cnt++]=(c);	\
	} while (0);

static int deinit_toimaputf7(void *ptr, int *errptr);

static int do_convert_toutf7(const char *text, size_t cnt, void *arg);
static int convert_utf7_handler(void *ptr, const char *text, size_t cnt);

/*
** Conversion wrapper for converting to modified-utf8 SMAP encoding.
**
** This is done by converting to UTF-8, then stacking on a module that
** takes that and converts UTF-8 to modified-UTF8.
**
** init_nottosmaputf8() returns an opaque stack for converting to modified
** UTF-8.
*/

static unicode_convert_handle_t
init_nottosmaputf8(const char *src_chset,
		   const char *dst_chset,
		   int (*output_func)(const char *, size_t, void *),
		   void *convert_arg);

/*
** The to modified UTF8 module
*/

struct unicode_convert_tosmaputf8 {

	struct unicode_convert_hdr hdr;

	int errflag;

	/* Remembered output function */

	int (*output_func)(const char *, size_t, void *);

	/* Remembered arg to the output function */
	void *convert_arg;
};



/*
** Create a conversion module stack
*/

unicode_convert_handle_t
unicode_convert_init(const char *src_chset,
		       const char *dst_chset,
		       int (*output_func)(const char *, size_t, void *),
		       void *convert_arg)
{
	struct unicode_convert_toimaputf7 *toutf7;
	unicode_convert_handle_t h;
	const char *smapmunge;
	size_t l=strlen(unicode_x_imap_modutf7);

	if (strncmp(dst_chset, unicode_x_imap_modutf7, l) == 0 &&
	    (dst_chset[l] == 0 || dst_chset[l] == ' '))
	{
		smapmunge=dst_chset + l;

		if (*smapmunge)
			++smapmunge;
	}
	else
		return init_nottoimaputf7(src_chset, dst_chset,
					     output_func,
					     convert_arg);

	toutf7=malloc(sizeof(struct unicode_convert_toimaputf7));

	if (!toutf7)
		return NULL;

	memset(toutf7, 0, sizeof(*toutf7));

	h=unicode_convert_init(src_chset, unicode_u_ucs2_native,
			       do_convert_toutf7, toutf7);
	if (!h)
	{
		free(toutf7);
		return (NULL);
	}

	toutf7->output_func=output_func;
	toutf7->convert_arg=convert_arg;

	strncat(toutf7->smapmunge, smapmunge, sizeof(toutf7->smapmunge)-1);

	toutf7->hdr.convert_handler=convert_utf7_handler;
	toutf7->hdr.deinit_handler=deinit_toimaputf7;
	toutf7->hdr.ptr=toutf7;
	toutf7->hdr.next=h;
	return &toutf7->hdr;
}

/* Passthrough to the wrapped stack */

static int convert_utf7_handler(void *ptr, const char *text, size_t cnt)
{
	struct unicode_convert_toimaputf7 *toutf7=
		(struct unicode_convert_toimaputf7 *)ptr;

	return (*toutf7->hdr.next->convert_handler)(toutf7->hdr.next->ptr,
						    text, cnt);
}

static int utf7off(struct unicode_convert_toimaputf7 *toutf7)
{
	if (!toutf7->utfmode)
		return 0;
	toutf7->utfmode=0;

	if (toutf7->utf7bitcount > 0)
		toimaputf7_encode_add(toutf7,
				      mbase64[(toutf7->utf7bits
					       << (6-toutf7->utf7bitcount))
					      & 63]);
	toimaputf7_encode_add(toutf7, '-');
	return 0;
}


static int do_convert_toutf7(const char *text, size_t cnt, void *arg)
{
	struct unicode_convert_toimaputf7 *toutf7=
		(struct unicode_convert_toimaputf7 *)arg;

	/* We better be getting UCS-2 here! */

	const uint16_t *utext=(const uint16_t *)text;
	cnt /= 2;

	while (cnt)
	{
		if (toutf7->errflag)
			return toutf7->errflag;

		if (*utext >= 0x20 && *utext <= 0x7F
		    && strchr( toutf7->smapmunge, (char)*utext) == NULL)

			/*
			  && (!toutf7->smapmunge || (*utext != '.' && *utext != '/' &&
			  *utext != '~' && *utext != ':')))
			  */
		{
			if (utf7off(toutf7))
				return toutf7->errflag;

			toimaputf7_encode_add(toutf7, *utext);

			if (*utext == '&')
				toimaputf7_encode_add(toutf7, '-');

			++utext;
			--cnt;
			continue;
		}

		if (!toutf7->utfmode)
		{
			toutf7->utfmode=1;
			toutf7->utf7bitcount=0;
			toimaputf7_encode_add(toutf7, '&');
			continue;
		}

		toutf7->utf7bits = (toutf7->utf7bits << 16) |
			(((uint32_t)*utext) & 0xFFFF);
		toutf7->utf7bitcount += 16;

		++utext;
		--cnt;

		/* If there's at least 6 bits, output base64-encoded char */

		while (toutf7->utf7bitcount >= 6)
		{
			uint32_t v;
			int n;

			if (toutf7->errflag)
				return toutf7->errflag;

			v=toutf7->utf7bits;
			n=toutf7->utf7bitcount-6;
			toutf7->utf7bitcount -= 6;

			if (n > 0)
				v >>= n;

			toimaputf7_encode_add(toutf7, mbase64[v & 63]);
		}
	}

	return 0;
}

static int deinit_toimaputf7(void *ptr, int *errptr)
{
	int rc;

	struct unicode_convert_toimaputf7 *toutf7=
		(struct unicode_convert_toimaputf7 *)ptr;

	/* Flush out the downstream stack */
	rc=(*toutf7->hdr.next->deinit_handler)(toutf7->hdr.next->ptr, errptr);

	/* Make sure we're out of modified base64 */

	if (rc == 0)
		rc=utf7off(toutf7);

	if (rc == 0 && toutf7->utf7encodebuf_cnt > 0)
		rc=toimaputf7_encode_flushfinal(toutf7);

	free(toutf7);
	return rc;
}

/*
** Convert to unicode_x_smap_modutf8.
*/

static int deinit_tosmaputf8(void *ptr, int *errptr);
static int do_convert_tosmaputf8(const char *text, size_t cnt, void *arg);
static int convert_utf8_handler(void *ptr, const char *text, size_t cnt);

static unicode_convert_handle_t
init_nottoimaputf7(const char *src_chset,
		   const char *dst_chset,
		   int (*output_func)(const char *, size_t, void *),
		   void *convert_arg)
{
	struct unicode_convert_tosmaputf8 *toutf8;
	unicode_convert_handle_t h;

	if (strcmp(dst_chset, unicode_x_smap_modutf8))
		return init_nottosmaputf8(src_chset, dst_chset,
					  output_func,
					  convert_arg);

	toutf8=malloc(sizeof(struct unicode_convert_tosmaputf8));

	if (!toutf8)
		return NULL;

	memset(toutf8, 0, sizeof(*toutf8));

	h=unicode_convert_init(src_chset, "utf-8",
			       do_convert_tosmaputf8,
			       toutf8);
	if (!h)
	{
		free(toutf8);
		return (NULL);
	}

	toutf8->output_func=output_func;
	toutf8->convert_arg=convert_arg;

	toutf8->hdr.convert_handler=convert_utf8_handler;
	toutf8->hdr.deinit_handler=deinit_tosmaputf8;
	toutf8->hdr.ptr=toutf8;
	toutf8->hdr.next=h;
	return &toutf8->hdr;
}

static int deinit_tosmaputf8(void *ptr, int *errptr)
{
	int rc;

	struct unicode_convert_tosmaputf8 *toutf8=
		(struct unicode_convert_tosmaputf8 *)ptr;

	/* Flush out the downstream stack */
	rc=(*toutf8->hdr.next->deinit_handler)(toutf8->hdr.next->ptr, errptr);

	if (toutf8->errflag && rc == 0)
		rc=toutf8->errflag;

	free(toutf8);
	return rc;
}

static int do_convert_tosmaputf8(const char *text, size_t cnt, void *arg)
{
	struct unicode_convert_tosmaputf8 *toutf8=
		(struct unicode_convert_tosmaputf8 *)arg;
	int rc;
	size_t i;
	char octal[4];

	while (cnt)
	{
		if (toutf8->errflag)
			return toutf8->errflag;

		for (i=0; i<cnt; ++i)
			if ((unsigned char)text[i] < ' ' ||
			    strchr("./~:\\", text[i]))
				break;
		if (i)
		{
			rc= (*toutf8->output_func)(text, i,
						   toutf8->convert_arg);

			if (rc)
			{
				toutf8->errflag=rc;
				return rc;
			}
			text += i;
			cnt -= i;
		}

		if (cnt)
		{
			char c= *text;

			octal[0]='\\';
			octal[3]= (c & 7)+'0'; c /= 8;
			octal[2]= (c & 7)+'0'; c /= 8;
			octal[1]= (c & 7)+'0';
			rc= (*toutf8->output_func)(octal, 4,
						   toutf8->convert_arg);
			if (rc)
			{
				toutf8->errflag=rc;
				return rc;
			}
			++text;
			--cnt;
		}
	}
	return 0;
}

static int convert_utf8_handler(void *ptr, const char *text, size_t cnt)
{
	struct unicode_convert_tosmaputf8 *toutf8=
		(struct unicode_convert_tosmaputf8 *)ptr;

	return (*toutf8->hdr.next->convert_handler)(toutf8->hdr.next->ptr,
						    text, cnt);
}

/************/

/*
** Convert from modified-utf7 IMAP encoding.
**
** This module converts it to UCS-2, then this is attached to a stack that
** converts UCS-2 to the requested charset.
*/

static unicode_convert_handle_t
init_notfromimaputf7(const char *src_chset,
		     const char *dst_chset,
		     int (*output_func)(const char *, size_t, void *),
		     void *convert_arg);

struct unicode_convert_fromimaputf7 {

	struct unicode_convert_hdr hdr;

	/* Accumulated UCS-2 stream */
	uint16_t convbuf[512];
	size_t convbuf_cnt;

	/* Accumulated base64 bits */
	uint32_t modbits;

	/* How many bits extracted from a base64 stream */

	short modcnt;

	/* Flag: seen the & */
	char seenamp;

	/* Flag: seen the &, and the next char wasn't - */

	char inmod;
	int errflag;
	int converr;
};

/* Flush the accumulated UCS-2 stream */

#define convert_fromutf7_flush(p) do {					\
		int rc=(*(p)->hdr.next->convert_handler)		\
			((p)->hdr.next->ptr,				\
			 (const char *)(p)->convbuf,			\
			 (p)->convbuf_cnt *				\
			 sizeof((p)->convbuf[0]));			\
									\
		if (rc)							\
			(p)->errflag=rc;				\
									\
		(p)->convbuf_cnt=0;					\
	} while (0)

/* Accumulated a UCS-2 char */

#define convert_fromutf7_add(p,c) do {					\
		if ((p)->convbuf_cnt >=					\
		    sizeof((p)->convbuf)/sizeof((p)->convbuf[0]))	\
			convert_fromutf7_flush((p));			\
		(p)->convbuf[(p)->convbuf_cnt++]=(c);			\
	} while (0)


static int convert_fromutf7(void *ptr,
			    const char *text, size_t cnt);
static int deinit_fromutf7(void *ptr, int *errptr);

static unicode_convert_handle_t
init_nottosmaputf8(const char *src_chset,
		   const char *dst_chset,
		   int (*output_func)(const char *, size_t, void *),
		   void *convert_arg)
{
	struct unicode_convert_fromimaputf7 *fromutf7;
	unicode_convert_handle_t h;
	size_t l=strlen(unicode_x_imap_modutf7);

	if (strncmp(src_chset, unicode_x_imap_modutf7, l) == 0 &&
	    (src_chset[l] == 0 || src_chset[l] == ' '))
		;
	else
		return init_notfromimaputf7(src_chset, dst_chset,
					    output_func,
					    convert_arg);

	fromutf7=(struct unicode_convert_fromimaputf7 *)
		malloc(sizeof(struct unicode_convert_fromimaputf7));

	if (!fromutf7)
		return NULL;

	memset(fromutf7, 0, sizeof(*fromutf7));

	/* Create a stack for converting UCS-2 to the dest charset */

	h=unicode_convert_init(unicode_u_ucs2_native, dst_chset,
			       output_func, convert_arg);

	if (!h)
	{
		free(fromutf7);
		return (NULL);
	}

	fromutf7->hdr.next=h;
	fromutf7->hdr.convert_handler=convert_fromutf7;
	fromutf7->hdr.deinit_handler=deinit_fromutf7;
	fromutf7->hdr.ptr=fromutf7;
	return &fromutf7->hdr;
}

static int convert_fromutf7(void *ptr,
			    const char *text, size_t cnt)
{
	struct unicode_convert_fromimaputf7 *fromutf7=
		(struct unicode_convert_fromimaputf7 *)ptr;
	int bits;

	while (cnt)
	{
		if (fromutf7->errflag)
			return fromutf7->errflag;

		if (!fromutf7->seenamp && *text == '&')
		{
			fromutf7->seenamp=1;
			fromutf7->inmod=0;
			fromutf7->modcnt=0;
			++text;
			--cnt;
			continue;
		}

		if (fromutf7->seenamp)
		{
			if (*text == '-')
			{
				convert_fromutf7_add(fromutf7, '&');
				++text;
				--cnt;
				fromutf7->seenamp=0;
				continue;
			}
			fromutf7->seenamp=0;
			fromutf7->inmod=1;
		}

		if (!fromutf7->inmod)
		{
			/* Not in the base64 encoded stream */

			convert_fromutf7_add(fromutf7,
					     ((uint16_t)*text) & 0xFFFF);
			++text;
			--cnt;
			continue;
		}

		if (*text == '-')
		{
			/* End of the base64 encoded stream */
			fromutf7->inmod=0;
			++text;
			--cnt;
			continue;
		}

		/* Got 6 more bits */

		bits=mbase64_lookup[(unsigned char)*text];

		++text;
		--cnt;

		if (bits < 0)
		{
			errno=EILSEQ;
			return fromutf7->errflag=-1;
		}

		fromutf7->modbits = (fromutf7->modbits << 6) | bits;
		fromutf7->modcnt += 6;

		if (fromutf7->modcnt >= 16)
		{
			/* Got a UCS-2 char */

			int shiftcnt=fromutf7->modcnt - 16;
			uint32_t v=fromutf7->modbits;

			if (shiftcnt)
				v >>= shiftcnt;

			fromutf7->modcnt -= 16;

			convert_fromutf7_add(fromutf7, v);
		}
	}
	return 0;
}

static int deinit_fromutf7(void *ptr, int *errptr)
{
	struct unicode_convert_fromimaputf7 *fromutf7=
		(struct unicode_convert_fromimaputf7 *)ptr;
	int rc;

	if (fromutf7->seenamp || fromutf7->inmod)
	{
		if (fromutf7->errflag == 0)
		{
			fromutf7->errflag= -1;
			errno=EILSEQ;
		}
	}

	if (fromutf7->convbuf_cnt)
		convert_fromutf7_flush(fromutf7);

	rc=fromutf7->hdr.next->deinit_handler(fromutf7->hdr.next->ptr, errptr);

	if (fromutf7->errflag && rc == 0)
		rc=fromutf7->errflag;

	if (errptr && fromutf7->converr)
		*errptr=1;

	free(fromutf7);
	return rc;
}

/************/

/*
** Convert from modified-utf8 SMAP encoding.
**
** This module converts it to UTF-8, then this is attached to a stack that
** converts UTF-8 to the requested charset.
*/

static unicode_convert_handle_t
init_notfromsmaputf8(const char *src_chset,
		     const char *dst_chset,
		     int (*output_func)(const char *, size_t, void *),
		     void *convert_arg);

struct unicode_convert_fromsmaputf8 {

	struct unicode_convert_hdr hdr;

	/* Convert a backslash escape */

	int in_escape;

	/* The escaped character */

	unsigned char escape_char;

	int errflag;
	int converr;
};

static int convert_fromutf8(void *ptr,
			    const char *text, size_t cnt);
static int deinit_fromutf8(void *ptr, int *errptr);

static unicode_convert_handle_t
init_notfromimaputf7(const char *src_chset,
		     const char *dst_chset,
		     int (*output_func)(const char *, size_t, void *),
		     void *convert_arg)
{
	struct unicode_convert_fromsmaputf8 *fromutf8;
	unicode_convert_handle_t h;

	if (strcmp(src_chset, unicode_x_smap_modutf8))
		return init_notfromsmaputf8(src_chset, dst_chset,
					    output_func, convert_arg);

	fromutf8=(struct unicode_convert_fromsmaputf8 *)
		malloc(sizeof(struct unicode_convert_fromsmaputf8));

	if (!fromutf8)
		return NULL;

	memset(fromutf8, 0, sizeof(*fromutf8));

	/* Create a stack for converting UTF-8 to the dest charset */

	h=unicode_convert_init("utf-8", dst_chset,
			       output_func, convert_arg);

	if (!h)
	{
		free(fromutf8);
		return (NULL);
	}

	fromutf8->hdr.next=h;
	fromutf8->hdr.convert_handler=convert_fromutf8;
	fromutf8->hdr.deinit_handler=deinit_fromutf8;
	fromutf8->hdr.ptr=fromutf8;
	return &fromutf8->hdr;
}

static int convert_fromutf8(void *ptr,
			    const char *text, size_t cnt)
{
	struct unicode_convert_fromsmaputf8 *fromutf8=
		(struct unicode_convert_fromsmaputf8 *)ptr;
	size_t i;

	while (cnt)
	{
		if (fromutf8->errflag)
			return fromutf8->errflag;

		if (fromutf8->in_escape)
		{
			if (*text < '0' || *text > '7')
			{
				errno=EILSEQ;
				return fromutf8->errflag=-1;
			}
			fromutf8->escape_char <<= 3;
			fromutf8->escape_char |= *text - '0';
			if (--fromutf8->in_escape == 0)
			{
				fromutf8->errflag=(*fromutf8->hdr.next
						   ->convert_handler)
					(fromutf8->hdr.next->ptr,
					 (const char *)&fromutf8->escape_char,
					 1);
			}
			++text;
			--cnt;
			continue;
		}

		for (i=0; i<cnt; ++i)
			if (text[i] == '\\')
				break;

		if (i)
		{
			fromutf8->errflag=(*fromutf8->hdr.next
					   ->convert_handler)
				(fromutf8->hdr.next->ptr, text, i);
			text += i;
			cnt -= i;
		}

		if (cnt)
		{
			fromutf8->escape_char=0;
			fromutf8->in_escape=3;
			++text;
			--cnt;
		}
	}
	return 0;
}

static int deinit_fromutf8(void *ptr, int *errptr)
{
	struct unicode_convert_fromsmaputf8 *fromutf8=
		(struct unicode_convert_fromsmaputf8 *)ptr;
	int rc;

	if (fromutf8->in_escape)
	{
		fromutf8->errflag= -1;
		errno=EILSEQ;
	}

	rc=fromutf8->hdr.next->deinit_handler(fromutf8->hdr.next->ptr, errptr);

	if (fromutf8->errflag && rc == 0)
		rc=fromutf8->errflag;

	if (errptr && fromutf8->converr)
		*errptr=1;

	free(fromutf8);
	return rc;
}

/************/

/* A real conversion module, via iconv */

struct unicode_convert_iconv {

	struct unicode_convert_hdr hdr;

	iconv_t h;
	int errflag; /* Accumulated errors */

	int (*output_func)(const char *, size_t, void *);
	void *convert_arg;

	char buffer[1024]; /* Input buffer */
	size_t bufcnt; /* Accumulated input in buffer */
	char skipcnt; /* Skip this many bytes upon encountering EILSEQ */
	char skipleft; /* How many bytes are currently left to skip */
	char converr; /* Flag - an EILSEQ was encountered */
} ;

static int init_iconv(struct unicode_convert_iconv *h,
		      const char *src_chset,
		      const char *dst_chset,
		      int (*output_func)(const char *, size_t, void *),
		      void *convert_arg);

static unicode_convert_handle_t
init_notfromsmaputf8(const char *src_chset,
		     const char *dst_chset,
		     int (*output_func)(const char *, size_t, void *),
		     void *convert_arg)
{


	struct unicode_convert_iconv *h=
		malloc(sizeof(struct unicode_convert_iconv));

	if (!h)
		return NULL;

	memset(h, 0, sizeof(*h));

	if (init_iconv(h, src_chset, dst_chset, output_func, convert_arg))
	{
		free(h);
		return NULL;
	}
	return &h->hdr;
}

/* Run the stack */

int unicode_convert(unicode_convert_handle_t h,
		      const char *text, size_t cnt)
{
	return (*h->convert_handler)(h->ptr, text, cnt);
}

/* Destroy the stack */

int unicode_convert_deinit(unicode_convert_handle_t h, int *errptr)
{
	return (*h->deinit_handler)(h, errptr);
}

static int deinit_iconv(void *ptr, int *errptr);
static int convert_iconv(void *ptr,
			 const char *text, size_t cnt);

/* Initialize a single conversion module, in the stack */

static int init_iconv(struct unicode_convert_iconv *h,
		      const char *src_chset,
		      const char *dst_chset,
		      int (*output_func)(const char *, size_t, void *),
		      void *convert_arg)
{
	if ((h->h=iconv_open(dst_chset, src_chset)) == (iconv_t)-1)
		return -1;

	h->hdr.convert_handler=convert_iconv;
	h->hdr.deinit_handler=deinit_iconv;
	h->hdr.ptr=h;

	h->output_func=output_func;
	h->convert_arg=convert_arg;

	/* Heuristically determine how many octets to skip upon an EILSEQ */

	h->skipcnt=1;
	switch (src_chset[0]) {
	case 'u':
	case 'U':
		switch (src_chset[1]) {
		case 'c':
		case 'C':
			switch (src_chset[2]) {
			case 's':
			case 'S':
				if (src_chset[3] == '-')
					switch (src_chset[4]) {
					case '4':
						/* UCS-4 */
						h->skipcnt=4;
						break;
					case '2':
						/* UCS-2 */
						h->skipcnt=2;
						break;
					}
			}
			break;
		case 't':
		case 'T':
			switch (src_chset[2]) {
			case 'f':
			case 'F':
				if (src_chset[3] == '-')
					switch (src_chset[4]) {
					case '3':
						/* UTF-32 */
						h->skipcnt=4;
						break;
					case '1':
						/* UTF-16 */
						h->skipcnt=2;
						break;
					}
			}
		}
	}

	return 0;
}

static void convert_flush(struct unicode_convert_iconv *);
static void convert_flush_iconv(struct unicode_convert_iconv *, const char **,
				size_t *);

/*
** iconv conversion module. Accumulate input in an input buffer. When the
** input buffer is full, invoke convert_flush().
*/

static int convert_iconv(void *ptr,
			 const char *text, size_t cnt)
{
	struct unicode_convert_iconv *h=(struct unicode_convert_iconv *)ptr;

	while (cnt && h->errflag == 0)
	{
		if (h->bufcnt >= sizeof(h->buffer)-1)
		{
			convert_flush(h);

			if (h->errflag)
				break;
		}

		h->buffer[h->bufcnt++]= *text++;
		--cnt;
	}

	return h->errflag;
}

/*
** Finish an iconv conversion module. Invoke convert_flush() to flush any
** buffered input. Invoke convert_flush_iconv() to return state to the initial
** conversion state.
*/

static int deinit_iconv(void *ptr, int *errptr)
{
	int rc;
	int converr;
	struct unicode_convert_iconv *h=(struct unicode_convert_iconv *)ptr;
	unicode_convert_handle_t next;

	if (h->errflag == 0)
		convert_flush(h);

	if (h->bufcnt && h->errflag == 0)
		h->converr=1;

	if (h->errflag == 0)
		convert_flush_iconv(h, NULL, NULL);

	rc=h->errflag;
	converr=h->converr != 0;
	iconv_close(h->h);
	next=h->hdr.next;
	free(h);
	if (errptr)
		*errptr=converr;

	/* If there's another module in the stack, clean that up */

	if (next)
	{
		int converrnext;
		int rcnext=unicode_convert_deinit(next, &converrnext);

		if (converrnext && errptr && *errptr == 0)
			*errptr=converr;

		if (rcnext && rc == 0)
			rc=rcnext;
	}
	return rc;
}

/*
** Invoke convert_flush_iconv() to flush the input buffer. If there's
** unconverted text remaining, reposition it at the beginning of the input
** buffer.
*/

static void convert_flush(struct unicode_convert_iconv *h)
{
	const char *p;
	size_t n;

	if (h->bufcnt == 0 || h->errflag)
		return;

	p=h->buffer;
	n=h->bufcnt;

	convert_flush_iconv(h, &p, &n);

	if (h->errflag)
		return;

	if (h->bufcnt == n)
		n=0; /* Unexpected error, dunno what to do, punt */

	h->bufcnt=0;

	while (n)
	{
		h->buffer[h->bufcnt]= *p;

		++h->bufcnt;
		++p;
		--n;
	}
}

/*
** Convert text via iconv.
*/

static void convert_flush_iconv(struct unicode_convert_iconv *h,
				const char **inbuf, size_t *inbytesleft)
{
	int save_errno;

	while (1)
	{
		char outbuf[1024];
		char *outp;
		size_t outleft;
		size_t n;
		size_t origin=0;

		if (inbytesleft)
		{
			if ((origin=*inbytesleft) == 0)
				return;

			if (inbuf && h->skipleft && origin)
			{
				/* Skipping after an EILSEQ */

				--h->skipleft;
				--*inbytesleft;
				++*inbuf;
				continue;
			}

		}

		if (h->errflag)
		{
			/* Quietly eat everything after a previous error */

			if (inbytesleft)
				*inbytesleft=0;

			return;
		}

		outp=outbuf;
		outleft=sizeof(outbuf);

		n=iconv(h->h, (char **)inbuf, inbytesleft, &outp, &outleft);

		save_errno=errno;

		/* Anything produced by iconv() gets pushed down the stack */

		if (outp > outbuf)
		{
			int rc=(*h->output_func)(outbuf, outp-outbuf,
						 h->convert_arg);
			if (rc)
			{
				h->errflag=rc;
				return;
			}
		}

		if (n != (size_t)-1)
		{
			/* iconv(3) reason #2 */

			break;
		}

		if (inbytesleft == 0)
		{
			/*
			** An error when generating the shift sequence to
			** return to the initial state. We don't know what to
			** do, now.
			*/

			errno=EINVAL;
			h->errflag= -1;
			return;
		}

		/*
		** convert_flush() gets invoked when the 1024 char input buffer
		** fills or to convert input that has been buffered when
		** convert_chset_end() gets invoked.
		**
		** A return code of EINVAL from iconv() is iconv() encountering
		** an incomplete multibyte sequence.
		**
		** If iconv() failed without consuming any input:
		**
		** - iconv(3) reason #1, EILSEQ, invalid multibyte sequence
		** that starts at the beginning of the string we wish to
		** convert. Discard one character, and try again.
		**
		** - iconv(3) reason #3, EINVAL, incomplete multibyte sequence.
		** If it's possible to have an incomplete 1024 character long
		** multibyte sequence, we're in trouble. Or we've encountered
		** an EINVAL when flushing out the remaining buffered input,
		** in convert_chset_end(). In either case, it's ok to sicard
		** one character at a time, until we either reach the end,
		** or get some other result.
		**
		** - iconv(3) reason #4, E2BIG. If the 1024 character output
		** buffer, above, is insufficient to produce the output from a
		** single converted character, we're in trouble.
		*/

		if (*inbytesleft == origin)
		{
			h->skipleft=h->skipcnt;
			h->converr=1;
		}

		/*
		** Stopped at an incomplete multibyte sequence, try again on
		** the next round.
		*/
		else if (save_errno == EINVAL)
			break;

		if (save_errno == EILSEQ)
			h->converr=1; /* Another possibility this can happen */

		/*
		** If we get here because of iconv(3) reason #4, filled out
		** the output buffer, we should continue with the conversion.
		** Otherwise, upon encountering any other error condition,
		** reset the conversion state.
		*/
		if (save_errno != E2BIG)
			iconv(h->h, NULL, NULL, NULL, NULL);
	}
}

/*****************************************************************************/

/*
** A wrapper for unicode_convert() that collects the converted character
** text into a buffer. This is done by passing an output function to
** unicode_convert() that saves converted text in a linked-list
** of buffers.
**
** Then, in the deinitialization function, the buffers get concatenated into
** the final character buffer.
*/

struct unicode_convert_cbuf {
	struct unicode_convert_cbuf *next;
	char *fragment;
	size_t fragment_size;
};

struct unicode_convert_tocbuf {
	struct unicode_convert_hdr hdr;

	char **cbufptr_ret;
	size_t *cbufsize_ret;
	int errflag;
	size_t tot_size;
	int nullterminate;

	struct unicode_convert_cbuf *first, **last;
};

static int save_tocbuf(const char *, size_t, void *);
static int convert_tocbuf(void *ptr,
			  const char *text, size_t cnt);
static int deinit_tocbuf(void *ptr, int *errptr);

unicode_convert_handle_t
unicode_convert_tocbuf_init(const char *src_chset,
			      const char *dst_chset,
			      char **cbufptr_ret,
			      size_t *cbufsize_ret,
			      int nullterminate
			      )
{
	struct unicode_convert_tocbuf *p=
		malloc(sizeof(struct unicode_convert_tocbuf));
	unicode_convert_handle_t h;

	if (!p)
		return NULL;

	memset(p, 0, sizeof(*p));

	h=unicode_convert_init(src_chset, dst_chset, save_tocbuf, p);

	if (!h)
	{
		free(p);
		return NULL;
	}

	p->cbufptr_ret=cbufptr_ret;
	p->cbufsize_ret=cbufsize_ret;
	p->last= &p->first;
	p->nullterminate=nullterminate;
	p->hdr.next=h;
	p->hdr.convert_handler=convert_tocbuf;
	p->hdr.deinit_handler=deinit_tocbuf;
	p->hdr.ptr=p;
	return &p->hdr;
}

/* Capture the output of the conversion stack */

static int save_tocbuf(const char *text, size_t cnt, void *ptr)
{
	struct unicode_convert_tocbuf *p=
		(struct unicode_convert_tocbuf *)ptr;
	struct unicode_convert_cbuf *fragment=
		malloc(sizeof(struct unicode_convert_cbuf)+cnt);
	size_t tot_size;

	if (!fragment)
	{
		p->errflag=1;
		return 1;
	}

	fragment->next=NULL;
	fragment->fragment=(char *)(fragment+1);
	if ((fragment->fragment_size=cnt) > 0)
		memcpy(fragment->fragment, text, cnt);

	*(p->last)=fragment;
	p->last=&fragment->next;

	tot_size=p->tot_size + cnt; /* Keep track of the total size saved */

	if (tot_size < p->tot_size) /* Overflow? */
	{
		errno=E2BIG;
		return 1;
	}
	p->tot_size=tot_size;
	return 0;
}

/* Punt converted text down the stack */

static int convert_tocbuf(void *ptr, const char *text, size_t cnt)
{
	struct unicode_convert_tocbuf *p=
		(struct unicode_convert_tocbuf *)ptr;

	return unicode_convert(p->hdr.next, text, cnt);
}

/*
** Destroy the conversion stack. Destroy the downstream, then assemble the
** final array.
*/

static int deinit_tocbuf(void *ptr, int *errptr)
{
	struct unicode_convert_tocbuf *p=
		(struct unicode_convert_tocbuf *)ptr;
	int rc=unicode_convert_deinit(p->hdr.next, errptr);
	struct unicode_convert_cbuf *bufptr;

	if (rc == 0 && p->nullterminate)
	{
		char zero=0;

		rc=save_tocbuf( &zero, sizeof(zero), p->hdr.ptr);
	}

	if (rc == 0)
	{
		if (((*p->cbufptr_ret)=malloc(p->tot_size ? p->tot_size:1)) !=
		    NULL)
		{
			size_t i=0;

			for (bufptr=p->first; bufptr; bufptr=bufptr->next)
			{
				if (bufptr->fragment_size)
					memcpy(&(*p->cbufptr_ret)[i],
					       bufptr->fragment,
					       bufptr->fragment_size);
				i += bufptr->fragment_size;
			}
			(*p->cbufsize_ret)=i;
		}
		else
		{
			rc= -1;
		}
	}

	for (bufptr=p->first; bufptr; )
	{
		struct unicode_convert_cbuf *b=bufptr;

		bufptr=bufptr->next;

		free(b);
	}
	free(p);

	return rc;
}

unicode_convert_handle_t
unicode_convert_tocbuf_toutf8_init(const char *src_chset,
				     char **cbufptr_ret,
				     size_t *cbufsize_ret,
				     int nullterminate
				     )
{
	return unicode_convert_tocbuf_init(src_chset, "utf-8",
					     cbufptr_ret, cbufsize_ret,
					     nullterminate);
}

unicode_convert_handle_t
unicode_convert_tocbuf_fromutf8_init(const char *dst_chset,
				       char **cbufptr_ret,
				       size_t *cbufsize_ret,
				       int nullterminate
				       )
{
	return unicode_convert_tocbuf_init("utf-8", dst_chset,
					     cbufptr_ret, cbufsize_ret,
					     nullterminate);
}

char *unicode_convert_toutf8(const char *text,
			       const char *charset,
			       int *error)
{
	char *cbufptr;
	size_t cbufsize;
	unicode_convert_handle_t h=
		unicode_convert_tocbuf_toutf8_init(charset,
						     &cbufptr,
						     &cbufsize, 1);

	if (!h)
		return NULL;

	unicode_convert(h, text, strlen(text));

	if (unicode_convert_deinit(h, error) == 0)
		return cbufptr;

	return NULL;
}

char *unicode_convert_fromutf8(const char *text,
				 const char *charset,
				 int *error)
{
	char *cbufptr;
	size_t cbufsize;
	unicode_convert_handle_t h=
		unicode_convert_tocbuf_fromutf8_init(charset,
						       &cbufptr,
						       &cbufsize, 1);

	if (!h)
		return NULL;

	unicode_convert(h, text, strlen(text));

	if (unicode_convert_deinit(h, error) == 0)
		return cbufptr;

	return NULL;
}

char *unicode_convert_tobuf(const char *text,
			      const char *charset,
			      const char *dstcharset,
			      int *error)
{
	char *cbufptr;
	size_t cbufsize;
	unicode_convert_handle_t h=
		unicode_convert_tocbuf_init(charset,
					      dstcharset,
					      &cbufptr,
					      &cbufsize, 1);

	if (!h)
		return NULL;

	unicode_convert(h, text, strlen(text));

	if (unicode_convert_deinit(h, error) == 0)
		return cbufptr;

	return NULL;
}

/*****************************************************************************/

/*
** Convert text to char32_ts. Same basic approach as
** unicode_convert_tocbuf_init(). The output character set gets specified
** as UCS-4, the final output size is divided by 4, and the output buffer gets
** typed as a char32_t array.
*/

struct unicode_convert_buf {
	struct unicode_convert_buf *next;
	char32_t *fragment;
	size_t fragment_size;
	size_t max_fragment_size;
};

struct unicode_convert_tou {
	struct unicode_convert_hdr hdr;

	char32_t **ucptr_ret;
	size_t *ucsize_ret;
	int errflag;
	size_t tot_size;
	int nullterminate;

	struct unicode_convert_buf *first, *tail, **last;
};

static int save_unicode(const char *, size_t, void *);
static int convert_tounicode(void *ptr,
			 const char *text, size_t cnt);
static int deinit_tounicode(void *ptr, int *errptr);

unicode_convert_handle_t
unicode_convert_tou_init(const char *src_chset,
			   char32_t **ucptr_ret,
			   size_t *ucsize_ret,
			   int nullterminate
			   )
{
	struct unicode_convert_tou *p=
		malloc(sizeof(struct unicode_convert_tou));
	unicode_convert_handle_t h;

	if (!p)
		return NULL;

	memset(p, 0, sizeof(*p));

	h=unicode_convert_init(src_chset, unicode_u_ucs4_native,
				 save_unicode, p);

	if (!h)
	{
		free(p);
		return NULL;
	}

	p->ucptr_ret=ucptr_ret;
	p->ucsize_ret=ucsize_ret;
	p->last= &p->first;
	p->nullterminate=nullterminate;
	p->hdr.next=h;
	p->hdr.convert_handler=convert_tounicode;
	p->hdr.deinit_handler=deinit_tounicode;
	p->hdr.ptr=p;
	return &p->hdr;
}

unicode_convert_handle_t
unicode_convert_fromu_init(const char *dst_chset,
			     char **cbufptr_ret,
			     size_t *csize_ret,
			     int nullterminate
			     )
{
	return unicode_convert_tocbuf_init(unicode_u_ucs4_native,
					     dst_chset,
					     cbufptr_ret,
					     csize_ret,
					     nullterminate);
}

int unicode_convert_uc(unicode_convert_handle_t handle,
			 const char32_t *text,
			 size_t cnt)
{
	return unicode_convert(handle, (const char *)text,
				 cnt * sizeof(*text));
}

/* Capture the output of the conversion stack */

static int save_unicode(const char *text, size_t cnt, void *ptr)
{
	struct unicode_convert_tou *p=
		(struct unicode_convert_tou *)ptr;
	struct unicode_convert_buf *fragment;
	size_t tot_size;

	cnt /= sizeof(char32_t);

	tot_size=p->tot_size + cnt*sizeof(char32_t);
	/* Keep track of the total size saved */

	if (p->tail)
	{
		size_t n=p->tail->max_fragment_size-p->tail->fragment_size;

		if (n > cnt)
			n=cnt;

		if (n)
		{
			memcpy(p->tail->fragment+p->tail->fragment_size,
			       text, n*sizeof(char32_t));

			cnt -= n;
			text += n*sizeof(char32_t);
			p->tail->fragment_size += n;
		}
	}

	if (cnt > 0)
	{
		size_t cnt_alloc=cnt;

		if (cnt_alloc < 16)
			cnt_alloc=16;

		if ((fragment=malloc(sizeof(struct unicode_convert_buf)
				     +cnt_alloc*sizeof(char32_t)))
		    == NULL)
		{
			p->errflag=1;
			return 1;
		}

		fragment->next=NULL;
		fragment->fragment=(char32_t *)(fragment+1);
		fragment->max_fragment_size=cnt_alloc;
		fragment->fragment_size=cnt;
		memcpy(fragment->fragment, text, cnt*sizeof(char32_t));

		*(p->last)=fragment;
		p->last=&fragment->next;
		p->tail=fragment;
	}

	if (tot_size < p->tot_size) /* Overflow? */
	{
		errno=E2BIG;
		return 1;
	}
	p->tot_size=tot_size;
	return 0;
}

/* Punt converted text down the stack */

static int convert_tounicode(void *ptr,
			     const char *text, size_t cnt)
{
	struct unicode_convert_tou *p=
		(struct unicode_convert_tou *)ptr;

	return unicode_convert(p->hdr.next, text, cnt);
}

/*
** Destroy the conversion stack. Destroy the downstream, then assemble the
** final array.
*/

static int deinit_tounicode(void *ptr, int *errptr)
{
	struct unicode_convert_tou *p=
		(struct unicode_convert_tou *)ptr;
	int rc=unicode_convert_deinit(p->hdr.next, errptr);
	struct unicode_convert_buf *bufptr;

	if (rc == 0 && p->nullterminate)
	{
		char32_t zero=0;

		rc=save_unicode( (const char *)&zero, sizeof(zero),
				 p->hdr.ptr);
	}

	if (rc == 0)
	{
		if (((*p->ucptr_ret)=malloc(p->tot_size ? p->tot_size:1)) !=
		    NULL)
		{
			size_t i=0;

			for (bufptr=p->first; bufptr; bufptr=bufptr->next)
			{
				if (bufptr->fragment_size)
					memcpy(&(*p->ucptr_ret)[i],
					       bufptr->fragment,
					       bufptr->fragment_size
					       *sizeof(*bufptr->fragment));
				i += bufptr->fragment_size;
			}
			(*p->ucsize_ret)=i;
		}
		else
		{
			rc= -1;
		}
	}

	for (bufptr=p->first; bufptr; )
	{
		struct unicode_convert_buf *b=bufptr;

		bufptr=bufptr->next;

		free(b);
	}
	free(p);

	return rc;
}

int unicode_convert_tou_tobuf(const char *text,
				size_t text_l,
				const char *charset,
				char32_t **uc,
				size_t *ucsize,
				int *err)
{
	unicode_convert_handle_t h;

	if ((h=unicode_convert_tou_init(charset, uc, ucsize, 0)) == NULL)
		return -1;

	if (unicode_convert(h, text, text_l) < 0)
	{
		unicode_convert_deinit(h, NULL);
		return -1;
	}

	if (unicode_convert_deinit(h, err))
		return -1;

	return 0;
}

int unicode_convert_fromu_tobuf(const char32_t *utext,
				  size_t utext_l,
				  const char *charset,
				  char **c,
				  size_t *csize,
				  int *err)
{
	unicode_convert_handle_t h;

	if (utext_l == (size_t)-1)
	{
		for (utext_l=0; utext[utext_l]; ++utext_l)
		     ;
	}

	if ((h=unicode_convert_fromu_init(charset, c, csize, 1)) == NULL)
		return -1;

	if (unicode_convert_uc(h, utext, utext_l) < 0)
	{
		unicode_convert_deinit(h, NULL);
		return -1;
	}

	if (unicode_convert_deinit(h, err))
		return -1;

	return 0;
}

char *unicode_convert_tocase(const char *str,
			       const char *charset,
			       char32_t (*first_char_func)(char32_t),
			       char32_t (*char_func)(char32_t))
{
	char32_t *uc;
	size_t ucsize;
	size_t i;
	int err;
	char *c;
	size_t csize;

	if (unicode_convert_tou_tobuf(str, strlen(str),
					charset, &uc, &ucsize, &err))
		return NULL;

	if (err)
	{
		free(uc);
		return NULL;
	}

	for (i=0; i<ucsize; ++i)
	{
		uc[i]=(*first_char_func)(uc[i]);

		if (char_func)
			first_char_func=char_func;
	}

	if (unicode_convert_fromu_tobuf(uc, ucsize,
					  charset,
					  &c, &csize, &err))
	{
		free(uc);
		return NULL;
	}

	free(uc);

	if (err)
	{
		free(c);
		return NULL;
	}

	return c;
}
