/*-
 * $Id: dkim_functions.c,v 1.2 2026-11-10 15:11:37+05:30 Cprogrammer Exp mbhangui $
 *
 * dkim_functions.c - DKIM (RFC 6376 / RFC 8463) signing and verification
 *
 * Plain C, depends only on libc and OpenSSL (>= 1.1.1).
 *
 * Design notes
 * ------------
 * - One streaming parser (message_feed) splits the message into a list of
 *   header fields and a body. Both signer and verifier sit on top of it.
 * - A body hasher (bodyhash) implements simple and relaxed body
 *   canonicalization (RFC 6376 3.4.3/3.4.4) one byte at a time, so it never
 *   needs the body in memory.
 * - Signer: the body is hashed once for each of the four combinations of
 *   {simple, relaxed} x {sha1, sha256}. dkim_sign_get() can then be called
 *   any number of times with different keys/options without re-reading the
 *   message. That is what makes multiple signatures safe.
 * - Verifier: every DKIM-Signature header gets its own body hasher (own
 *   canonicalization, own algorithm, own l= limit) and its own result.
 */
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <stralloc.h>
#include <qprintf.h>
#include <unistd.h>
#include <openssl/opensslv.h>
#include "dkim_functions.h"

/*-
 * Compiled with every OpenSSL version
 */
void
dkim_free(void *p)
{
	free(p);
}

/*-
 * Call at start up. With OpenSSL older than 1.1.1 (no Ed25519, and none of the
 * EVP calls used below) this does _exit(code); otherwise it returns 0.
 */
int
exit_if_old(int code)
{
#if OPENSSL_VERSION_NUMBER < 0x10101000L
	_exit(code);
#else
	(void) code;
	return 0;
#endif
}

/* ------------------------------------------------------------------ */
/* status text                                                         */
/* ------------------------------------------------------------------ */
const char     *
dkim_status_text(int code, const char **enhanced)
{
	const char     *t, *c = "X.7.0";

	switch (code)
	{
	case DKIM_SUCCESS:                       t = "good        "; break;
	case DKIM_FINISHED_BODY:                 t = "process result: no more message body is needed"; break;
	case DKIM_PARTIAL_SUCCESS:               t = "verify result: at least none but not all signatures verified"; break;
	case DKIM_NEUTRAL:                       t = "verify result: no signatures verified but message is not suspicious"; break;
	case DKIM_SUCCESS_BUT_EXTRA:             t = "signature result: signature verified but it did not include all of the body"; break;
	case DKIM_3PS_SIGNATURE:                 t = "signature result: 3rd party signature"; break;
	case DKIM_FAIL:                          t = "DKIM Signature verification failed"; break;
	case DKIM_BAD_SYNTAX:                    t = "signature error: DKIM-Signature could not parse or has bad tags/values"; c = "X.7.5"; break;
	case DKIM_SIGNATURE_BAD:                 t = "signature error: RSA/ED25519 verify failed"; c = "X.7.5"; break;
	case DKIM_SIGNATURE_BAD_BUT_TESTING:     t = "signature error: RSA/ED25519 verify failed but testing"; c = "X.7.5"; break;
	case DKIM_SIGNATURE_EXPIRED:             t = "signature error: x= is old"; c = "X.7.5"; break;
	case DKIM_SELECTOR_INVALID:              t = "signature error: selector doesn't parse or contains invalid values"; c = "X.7.5"; break;
	case DKIM_SELECTOR_GRANULARITY_MISMATCH: t = "signature error: selector g= doesn't match i="; c = "X.7.5"; break;
	case DKIM_SELECTOR_KEY_REVOKED:          t = "signature error: selector p= empty"; c = "X.7.5"; break;
	case DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG: t = "signature error: selector domain name too long to request"; break;
	case DKIM_SELECTOR_DNS_TEMP_FAILURE:     t = "signature error: temporary dns failure requesting selector"; break;
	case DKIM_SELECTOR_DNS_PERM_FAILURE:     t = "signature error: permanent dns failure requesting selector"; break;
	case DKIM_SELECTOR_PUBLIC_KEY_INVALID:   t = "signature error: selector p= value invalid or wrong format"; c = "X.7.5"; break;
	case DKIM_NO_SIGNATURES:                 t = "no signatures"; c = "X.7.5"; break;
	case DKIM_NO_VALID_SIGNATURES:           t = "no valid signatures"; c = "X.7.5"; break;
	case DKIM_BODY_HASH_MISMATCH:            t = "signature verify error: message body does not hash to bh value"; c = "X.7.7"; break;
	case DKIM_SELECTOR_ALGORITHM_MISMATCH:   t = "signature error: selector h= doesn't match signature a="; c = "X.7.7"; break;
	case DKIM_STAT_INCOMPAT:                 t = "signature error: incompatible v="; c = "X.7.6"; break;
	case DKIM_UNSIGNED_FROM:                 t = "signature error: not all message's From headers in signature"; c = "X.7.7"; break;
	case DKIM_BAD_IDENTITY:                  t = "signature error: invalid identify in signature"; c = "X.7.7"; break;
	case DKIM_OUT_OF_MEMORY:                 t = "memory allocation failed"; c = "X.3.0"; break;
	case DKIM_INVALID_CONTEXT:               t = "DKIMContext structure invalid for this operation"; c = "X.3.0"; break;
	case DKIM_NO_SENDER:                     t = "signing error: Could not find From: or Sender: header in message"; c = "X.1.7"; break;
	case DKIM_BAD_PRIVATE_KEY:               t = "signing error: private key not parsable, or key type does not match the -z algorithm (RSA key needs -z 1/2/3, Ed25519 key needs -z 4)"; c = "X.7.5"; break;
	case DKIM_BUFFER_TOO_SMALL:              t = "signing error: Buffer passed in is not large enough"; c = "X.3.0"; break;
	case DKIM_EVP_SIGN_FAILURE:              t = "signing error: evp signing failure"; c = "X.3.0"; break;
	case DKIM_EVP_DIGEST_FAILURE:            t = "signing error: evp digest failure"; c = "X.3.0"; break;
	case DKIM_OPENSSL_TOO_OLD:               t = "OpenSSL version too old on this system"; c = "X.3.0"; break;
	default:                                 t = "error"; c = "X.3.0"; break;
	}
	if (enhanced)
		*enhanced = c;
	return t;
}

const char     *
dkim_code_name(int code)
{
	switch (code)
	{
	case DKIM_SUCCESS:                       return "SUCCESS";
	case DKIM_FINISHED_BODY:                 return "FINISHED BODY";
	case DKIM_PARTIAL_SUCCESS:               return "PARTIAL SUCCESS";
	case DKIM_NEUTRAL:                       return "NEUTRAL";
	case DKIM_SUCCESS_BUT_EXTRA:             return "SUCCESS(BUT EXTRA)";
	case DKIM_3PS_SIGNATURE:                 return "3rd PARTY SIGNATURE";
	case DKIM_FAIL:                          return "FAIL";
	case DKIM_BAD_SYNTAX:                    return "BAD SYNTAX";
	case DKIM_SIGNATURE_BAD:                 return "SIGNATURE BAD";
	case DKIM_SIGNATURE_BAD_BUT_TESTING:     return "SIGNATURE BAD (TESTING)";
	case DKIM_SIGNATURE_EXPIRED:             return "SIGNATURE EXPIRED";
	case DKIM_SELECTOR_INVALID:              return "SELECTOR INVALID";
	case DKIM_SELECTOR_GRANULARITY_MISMATCH: return "SELECTOR GRANULARITY MISMATCH";
	case DKIM_SELECTOR_KEY_REVOKED:          return "SELECTOR KEY REVOKED";
	case DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG: return "DOMAIN NAME TOO LONG";
	case DKIM_SELECTOR_DNS_TEMP_FAILURE:     return "DNS TEMP FAILURE";
	case DKIM_SELECTOR_DNS_PERM_FAILURE:     return "DNS PERM FAILURE";
	case DKIM_SELECTOR_PUBLIC_KEY_INVALID:   return "PUBLIC KEY INVALID";
	case DKIM_NO_SIGNATURES:                 return "NO SIGNATURES";
	case DKIM_NO_VALID_SIGNATURES:           return "NO VALID SIGNATURES";
	case DKIM_BODY_HASH_MISMATCH:            return "BODY HASH MISMATCH";
	case DKIM_SELECTOR_ALGORITHM_MISMATCH:   return "ALGORITHM MISMATCH";
	case DKIM_STAT_INCOMPAT:                 return "STAT INCOMPAT";
	case DKIM_UNSIGNED_FROM:                 return "UNSIGNED FROM";
	case DKIM_BAD_IDENTITY:                  return "BAD IDENTITY";
	case DKIM_OPENSSL_TOO_OLD:               return "OPENSSL TOO OLD";
	}
	return "Unknown error";
}

#if OPENSSL_VERSION_NUMBER >= 0x10101000L

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#define MAX_HEADER_BYTES (16UL * 1024 * 1024)
#define MAX_TAGS         32

/* ------------------------------------------------------------------ */
/* growable buffer                                                     */
/* ------------------------------------------------------------------ */
typedef struct {
	char           *s;
	size_t          len;
	size_t          cap;
} dbuf;

static int
dbuf_grow(dbuf *b, size_t add)
{
	size_t          need = b->len + add + 1;
	char           *n;

	if (need <= b->cap)
		return 0;
	if (!(b->cap)) b->cap = 256;
	while (b->cap < need)
		b->cap *= 2;
	if (!(n = realloc(b->s, b->cap)))
		return -1;
	b->s = n;
	return 0;
}

static int
dbuf_catb(dbuf *b, const char *s, size_t n)
{
	if (dbuf_grow(b, n))
		return -1;
	memcpy(b->s + b->len, s, n);
	b->len += n;
	b->s[b->len] = 0;
	return 0;
}

static int
dbuf_cats(dbuf *b, const char *s)
{
	return dbuf_catb(b, s, strlen(s));
}

static int
dbuf_catc(dbuf *b, char c)
{
	return dbuf_catb(b, &c, 1);
}

static void
dbuf_reset(dbuf *b)
{
	b->len = 0;
	if (b->s)
		b->s[0] = 0;
}

static void
dbuf_free(dbuf *b)
{
	free(b->s);
	b->s = NULL;
	b->len = b->cap = 0;
}

/* ------------------------------------------------------------------ */
/* small helpers                                                       */
/* ------------------------------------------------------------------ */
#define IS_WSP(c) ((c) == ' ' || (c) == '\t')
#define IS_FWS(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

static char    *
xstrdup(const char *s)
{
	size_t          n = strlen(s) + 1;
	char           *p = malloc(n);

	if (p)
		memcpy(p, s, n);
	return p;
}

static int
b64_decode(const char *in, unsigned char **out, size_t *outlen)
{
	unsigned char  *o;
	unsigned int    acc = 0;
	int             bits = 0, pad = 0;
	size_t          n = 0;
	const char     *p;
	static signed char tab[256];
	static int      init;

	if (!init) {
		const char     *a = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		int             i;

		memset(tab, -1, sizeof(tab));
		for (i = 0; i < 64; i++)
			tab[(unsigned char) a[i]] = i;
		init = 1;
	}
	if (!(o = malloc(strlen(in) / 4 * 3 + 4)))
		return -1;
	for (p = in; *p; p++) {
		if (IS_FWS(*p))
			continue;
		if (*p == '=') {
			pad++;
			continue;
		}
		if (pad || tab[(unsigned char) *p] < 0) {
			free(o);
			return -2;
		}
		acc = (acc << 6) | tab[(unsigned char) *p];
		bits += 6;
		if (bits >= 8) {
			bits -= 8;
			o[n++] = (acc >> bits) & 0xff;
		}
	}
	if (pad > 2) {
		free(o);
		return -2;
	}
	*out = o;
	*outlen = n;
	return 0;
}

static int
b64_encode(dbuf *out, const unsigned char *in, size_t n)
{
	static const char a[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t          i;

	for (i = 0; i + 2 < n; i += 3) {
		if (dbuf_catc(out, a[in[i] >> 2]) ||
				dbuf_catc(out, a[((in[i] & 3) << 4) | (in[i + 1] >> 4)]) ||
				dbuf_catc(out, a[((in[i + 1] & 15) << 2) | (in[i + 2] >> 6)]) ||
				dbuf_catc(out, a[in[i + 2] & 63]))
			return -1;
	}
	if (n - i == 1) {
		if (dbuf_catc(out, a[in[i] >> 2]) ||
				dbuf_catc(out, a[(in[i] & 3) << 4]) ||
				dbuf_catc(out, '=') || dbuf_catc(out, '='))
			return -1;
	} else
	if (n - i == 2) {
		if (dbuf_catc(out, a[in[i] >> 2]) ||
				dbuf_catc(out, a[((in[i] & 3) << 4) | (in[i + 1] >> 4)]) ||
				dbuf_catc(out, a[(in[i + 1] & 15) << 2]) ||
				dbuf_catc(out, '='))
			return -1;
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* tag=value list parser (DKIM-Signature and key records)              */
/* ------------------------------------------------------------------ */
typedef struct {
	char           *name;
	char           *value;
} tag_t;

/*-
 * Destructive. Returns number of tags or -1 on a syntax error
 * (missing '=', empty name, duplicate tag).
 */
static int
parse_tags(char *s, tag_t *tags, int max)
{
	char           *p = s, *e;
	int             n = 0, i;

	for (;;) {
		while (*p && IS_FWS(*p))
			p++;
		if (*p == ';') {
			p++;
			continue;
		}
		if (!*p)
			break;
		if (n == max)
			return -1;
		tags[n].name = p;
		while (*p && *p != '=' && *p != ';')
			p++;
		if (*p != '=')
			return -1;
		for (e = p; e > tags[n].name && IS_FWS(e[-1]); e--);
		if (e == tags[n].name)
			return -1;
		*e = 0;
		*p++ = 0;
		while (*p && IS_FWS(*p))
			p++;
		tags[n].value = p;
		while (*p && *p != ';')
			p++;
		for (e = p; e > tags[n].value && IS_FWS(e[-1]); e--);
		if (*p)
			p++;
		*e = 0;
		for (i = 0; i < n; i++)
			if (!strcmp(tags[i].name, tags[n].name))
				return -1;
		n++;
	}
	return n;
}

static const char *
tag_get(const tag_t *tags, int n, const char *name)
{
	int             i;

	for (i = 0; i < n; i++)
		if (!strcmp(tags[i].name, name))
			return tags[i].value;
	return NULL;
}

/* ------------------------------------------------------------------ */
/* header list and streaming message parser                            */
/* ------------------------------------------------------------------ */
typedef struct {
	char           *raw;      /*- "Name: value", continuation lines joined with CRLF, no final CRLF */
	size_t          len;
	size_t          name_len; /*- length of name, trailing WSP trimmed (0 if no colon) */
	size_t          colon;    /*- offset of ':' (0 if none) */
} hdr;

typedef struct {
	hdr            *v;
	int             n;
	int             cap;
	size_t          bytes;
} hdrlist;

typedef struct {
	hdrlist         hl;
	dbuf            line;
	dbuf            field;
	int             headers_done;
} message;

static int
hl_add(hdrlist *hl, const char *s, size_t len)
{
	hdr            *h;
	char           *c;
	size_t          i;

	if (hl->bytes + len > MAX_HEADER_BYTES)
		return -2;
	if (hl->n == hl->cap) {
		int             nc = hl->cap ? hl->cap * 2 : 32;
		hdr            *nv = realloc(hl->v, nc * sizeof(hdr));

		if (!nv)
			return -1;
		hl->v = nv;
		hl->cap = nc;
	}
	if (!(c = malloc(len + 1)))
		return -1;
	memcpy(c, s, len);
	c[len] = 0;
	h = hl->v + hl->n++;
	h->raw = c;
	h->len = len;
	h->colon = h->name_len = 0;
	for (i = 0; i < len; i++) {
		if (c[i] == ':') {
			h->colon = i;
			for (h->name_len = i; h->name_len && IS_WSP(c[h->name_len - 1]); h->name_len--);
			break;
		}
		if (c[i] == '\n' || c[i] == '\r')
			break;
	}
	hl->bytes += len;
	return 0;
}

static void
hl_free(hdrlist *hl)
{
	int             i;

	for (i = 0; i < hl->n; i++)
		free(hl->v[i].raw);
	free(hl->v);
	memset(hl, 0, sizeof(*hl));
}

static int
hdr_is(const hdr *h, const char *name)
{
	size_t          n = strlen(name);

	return h->name_len == n && !strncasecmp(h->raw, name, n);
}

static int
message_flush_field(message *m)
{
	int             r;

	if (!m->field.len)
		return 0;
	r = hl_add(&m->hl, m->field.s, m->field.len);
	dbuf_reset(&m->field);
	return r;
}

/*-
 * Consume header bytes. Sets *used to the number of bytes eaten. When the
 * blank line that ends the header has been eaten headers_done is set and
 * the rest of the buffer is body.
 */
static int
message_feed_headers(message *m, const char *buf, size_t len, size_t *used)
{
	size_t          i;
	char           *l;

	*used = 0;
	for (i = 0; i < len; i++) {
		if (buf[i] != '\n') {
			if (dbuf_catc(&m->line, buf[i]))
				return DKIM_OUT_OF_MEMORY;
			if (m->line.len + m->field.len > MAX_HEADER_BYTES)
				return DKIM_FAIL;
			continue;
		}
		l = m->line.s;
		if (m->line.len && l[m->line.len - 1] == '\r')
			m->line.len--;
		if (!m->line.len) { /*- blank line: end of header */
			if (message_flush_field(m))
				return DKIM_OUT_OF_MEMORY;
			m->headers_done = 1;
			*used = i + 1;
			return 0;
		}
		l[m->line.len] = 0;
		if (IS_WSP(l[0]) && m->field.len) {
			if (dbuf_cats(&m->field, "\r\n") || dbuf_catb(&m->field, l, m->line.len))
				return DKIM_OUT_OF_MEMORY;
		} else {
			if (message_flush_field(m))
				return DKIM_OUT_OF_MEMORY;
			if (dbuf_catb(&m->field, l, m->line.len))
				return DKIM_OUT_OF_MEMORY;
		}
		dbuf_reset(&m->line);
	}
	*used = len;
	return 0;
}

/* message ended inside the header block (no body at all) */
static int
message_end_headers(message *m)
{
	if (m->headers_done)
		return 0;
	if (m->line.len) {
		char           *l = m->line.s;

		if (l[m->line.len - 1] == '\r')
			m->line.len--;
		if (m->line.len) {
			l[m->line.len] = 0;
			if (IS_WSP(l[0]) && m->field.len) {
				if (dbuf_cats(&m->field, "\r\n") || dbuf_catb(&m->field, l, m->line.len))
					return DKIM_OUT_OF_MEMORY;
			} else {
				if (message_flush_field(m))
					return DKIM_OUT_OF_MEMORY;
				if (dbuf_catb(&m->field, l, m->line.len))
					return DKIM_OUT_OF_MEMORY;
			}
		}
	}
	if (message_flush_field(m))
		return DKIM_OUT_OF_MEMORY;
	m->headers_done = 1;
	return 0;
}

static void
message_free(message *m)
{
	hl_free(&m->hl);
	dbuf_free(&m->line);
	dbuf_free(&m->field);
}

/* ------------------------------------------------------------------ */
/* header canonicalization (RFC 6376 3.4.1, 3.4.2)                     */
/* ------------------------------------------------------------------ */
/* returns 1 if appended, 0 if header has no colon (skipped), -1 on ENOMEM */
static int
canon_header(dbuf *out, const hdr *h, int relaxed, int crlf)
{
	size_t          i;
	int             sp = 0, any = 0;

	if (!h->name_len)
		return 0;
	if (!relaxed) {
		if (dbuf_catb(out, h->raw, h->len) || (crlf && dbuf_cats(out, "\r\n")))
			return -1;
		return 1;
	}
	for (i = 0; i < h->name_len; i++)
		if (dbuf_catc(out, tolower((unsigned char) h->raw[i])))
			return -1;
	if (dbuf_catc(out, ':'))
		return -1;
	for (i = h->colon + 1; i < h->len; i++) {
		char            c = h->raw[i];

		if (c == '\r' || c == '\n')
			continue; /*- unfold */
		if (IS_WSP(c)) {
			sp = 1;
			continue;
		}
		if (sp && any && dbuf_catc(out, ' '))
			return -1;
		sp = 0;
		any = 1;
		if (dbuf_catc(out, c))
			return -1;
	}
	if (crlf && dbuf_cats(out, "\r\n"))
		return -1;
	return 1;
}

/* ------------------------------------------------------------------ */
/* body canonicalization + hashing (RFC 6376 3.4.3, 3.4.4)             */
/* ------------------------------------------------------------------ */
typedef struct {
	EVP_MD_CTX     *md;
	int             relaxed;
	int             line_has_content;
	int             pending_wsp;
	int             pending_cr;
	unsigned long   pending_empty;
	unsigned long long total_len;  /*- canonicalized body length */
	unsigned long long hashed_len; /*- bytes actually fed to the hash */
	unsigned long long limit;
	int             has_limit;
	int             failed;
	unsigned char   obuf[4096];
	size_t          olen;
	unsigned char   digest[EVP_MAX_MD_SIZE];
	unsigned int    dlen;
	int             done;
} bodyhash;

static int
bh_init(bodyhash *b, int use_sha1, int relaxed, int has_limit, unsigned long long limit)
{
	memset(b, 0, sizeof(*b));
	b->relaxed = relaxed;
	b->has_limit = has_limit;
	b->limit = limit;
	if (!(b->md = EVP_MD_CTX_new()))
		return -1;
	if (EVP_DigestInit_ex(b->md, use_sha1 ? EVP_sha1() : EVP_sha256(), NULL) != 1) {
		EVP_MD_CTX_free(b->md);
		b->md = NULL;
		return -1;
	}
	return 0;
}

static void
bh_free(bodyhash *b)
{
	if (b->md)
		EVP_MD_CTX_free(b->md);
	b->md = NULL;
}

static void
bh_flush(bodyhash *b)
{
	size_t          n = b->olen;

	if (b->has_limit) {
		if (b->hashed_len >= b->limit)
			n = 0;
		else
		if (b->hashed_len + n > b->limit)
			n = (size_t) (b->limit - b->hashed_len);
	}
	if (n) {
		if (EVP_DigestUpdate(b->md, b->obuf, n) != 1)
			b->failed = 1;
		b->hashed_len += n;
	}
	b->olen = 0;
}

static void
bh_putc(bodyhash *b, unsigned char c)
{
	if (b->olen == sizeof(b->obuf))
		bh_flush(b);
	b->obuf[b->olen++] = c;
	b->total_len++;
}

static void
bh_content(bodyhash *b, unsigned char c)
{
	if (b->relaxed && IS_WSP(c)) {
		b->pending_wsp = 1;
		return;
	}
	if (!b->line_has_content) {
		while (b->pending_empty) {
			bh_putc(b, '\r');
			bh_putc(b, '\n');
			b->pending_empty--;
		}
		b->line_has_content = 1;
	}
	if (b->relaxed && b->pending_wsp) {
		bh_putc(b, ' ');
		b->pending_wsp = 0;
	}
	bh_putc(b, c);
}

static void
bh_eol(bodyhash *b)
{
	if (b->line_has_content) {
		bh_putc(b, '\r');
		bh_putc(b, '\n');
		b->line_has_content = 0;
	} else
		b->pending_empty++;
	b->pending_wsp = 0;
}

static void
bh_feed(bodyhash *b, const unsigned char *p, size_t n)
{
	size_t          i;

	for (i = 0; i < n; i++) {
		unsigned char   c = p[i];

		if (b->pending_cr) {
			b->pending_cr = 0;
			if (c == '\n') {
				bh_eol(b);
				continue;
			}
			bh_content(b, '\r');
		}
		if (c == '\r') {
			b->pending_cr = 1;
			continue;
		}
		if (c == '\n') {
			bh_eol(b);
			continue;
		}
		bh_content(b, c);
	}
}

static int
bh_final(bodyhash *b)
{
	if (b->done)
		return b->failed ? -1 : 0;
	if (b->pending_cr) {
		b->pending_cr = 0;
		bh_content(b, '\r');
	}
	if (b->line_has_content)
		bh_eol(b);
	/* pending empty lines at the end of the body are dropped */
	if (!b->total_len && !b->relaxed) { /*- simple: empty body is a single CRLF */
		bh_putc(b, '\r');
		bh_putc(b, '\n');
	}
	bh_flush(b);
	if (EVP_DigestFinal_ex(b->md, b->digest, &b->dlen) != 1)
		b->failed = 1;
	b->done = 1;
	return b->failed ? -1 : 0;
}

/* ------------------------------------------------------------------ */
/* domain extraction from address headers                              */
/* ------------------------------------------------------------------ */
/* value = text after the colon. Copies domain of first address to out. */
static int
addr_domain(const char *val, char *out, size_t outsz)
{
	const char     *p, *start = val, *at, *e;
	size_t          n;

	if ((p = strchr(val, '<')))
		start = p + 1;
	if (!(at = strchr(start, '@')))
		return 0;
	at++;
	for (e = at; *e && !IS_FWS(*e) && *e != '>' && *e != ',' && *e != ';' && *e != '(' && *e != ')'; e++);
	n = e - at;
	if (!n || n >= outsz)
		return 0;
	memcpy(out, at, n);
	out[n] = 0;
	while (n && out[n - 1] == '.')
		out[--n] = 0;
	return n > 0;
}

static int
find_domain(const hdrlist *hl, const char *const *names, char *out, size_t outsz)
{
	int             k, i;

	for (k = 0; names[k]; k++) {
		for (i = 0; i < hl->n; i++) {
			if (hdr_is(&hl->v[i], names[k]) &&
					addr_domain(hl->v[i].raw + hl->v[i].colon + 1, out, outsz))
				return 1;
		}
	}
	return 0;
}

static int
domain_in(const char *dom, const char *parent) /*- dom == parent or subdomain of it */
{
	size_t          dl = strlen(dom), pl = strlen(parent);

	if (dl == pl)
		return !strcasecmp(dom, parent);
	return dl > pl && dom[dl - pl - 1] == '.' && !strcasecmp(dom + dl - pl, parent);
}

/* ================================================================== */
/* SIGNING                                                             */
/* ================================================================== */
struct DKIMSign {
	DKIMSignOptions opts;
	message         msg;
	bodyhash        bh[4];    /*- [relaxed_body * 2 + sha256] */
	int             bh_ready;
	int             finished;
	int             domain_done;
	char            domain[256];
};

static int
sign_begin_body(DKIMSign *s)
{
	int             i;

	if (s->bh_ready)
		return 0;
	for (i = 0; i < 4; i++) {
		if (bh_init(&s->bh[i], !(i & 1), i >> 1, 0, 0)) {
			while (--i >= 0)
				bh_free(&s->bh[i]);
			return DKIM_OUT_OF_MEMORY;
		}
	}
	s->bh_ready = 1;
	return 0;
}

DKIMSign       *
dkim_sign_new(const DKIMSignOptions *opts)
{
	DKIMSign       *s;

	if (!(s = calloc(1, sizeof(*s))))
		return NULL;
	if (opts)
		s->opts = *opts;
	return s;
}

int
dkim_sign_process(DKIMSign *s, const char *buf, size_t len)
{
	int             r, i;
	size_t          used;

	if (!s || s->finished)
		return DKIM_INVALID_CONTEXT;
	while (len) {
		if (!s->msg.headers_done) {
			if ((r = message_feed_headers(&s->msg, buf, len, &used)))
				return r;
			buf += used;
			len -= used;
			if (s->msg.headers_done && (r = sign_begin_body(s)))
				return r;
			continue;
		}
		for (i = 0; i < 4; i++)
			bh_feed(&s->bh[i], (const unsigned char *) buf, len);
		break;
	}
	return 0;
}

static int
sign_finish(DKIMSign *s)
{
	int             r, i;

	if (s->finished)
		return 0;
	if ((r = message_end_headers(&s->msg)) || (r = sign_begin_body(s)))
		return r;
	for (i = 0; i < 4; i++)
		if (bh_final(&s->bh[i]))
			return DKIM_EVP_DIGEST_FAILURE;
	s->finished = 1;
	return 0;
}

const char     *
dkim_sign_domain(DKIMSign *s)
{
	static const char *const names[] = { "From", "Sender", "Return-Path", "X-Bounced-Address", NULL };

	if (!s)
		return NULL;
	if (s->opts.szDomain[0])
		return s->opts.szDomain;
	if (!s->domain_done) {
		if (s->msg.headers_done || sign_finish(s) == 0) {
			if (!find_domain(&s->msg.hl, names, s->domain, sizeof(s->domain))) {
				if (s->opts.szDefaultDomain[0]) {
					strncpy(s->domain, s->opts.szDefaultDomain, sizeof(s->domain) - 1);
					s->domain[sizeof(s->domain) - 1] = 0;
				} else
					s->domain[0] = 0;
			}
			s->domain_done = 1;
		}
	}
	return s->domain[0] ? s->domain : NULL;
}

void
dkim_sign_free(DKIMSign *s)
{
	int             i;

	if (!s)
		return;
	if (s->bh_ready)
		for (i = 0; i < 4; i++)
			bh_free(&s->bh[i]);
	message_free(&s->msg);
	free(s);
}

/* --- signature header assembly --- */
static int
fold_new(dbuf *b, size_t *col)
{
	if (dbuf_cats(b, "\r\n\t"))
		return -1;
	*col = 8;
	return 0;
}

/* start of a tag: separate by a space or fold when the line gets long */
static int
tag_add(dbuf *b, size_t *col, const char *tag)
{
	size_t          n = strlen(tag);

	if (*col + 1 + n > 76 && *col > 8) {
		if (fold_new(b, col))
			return -1;
	} else {
		if (dbuf_catc(b, ' '))
			return -1;
		(*col)++;
	}
	if (dbuf_catb(b, tag, n))
		return -1;
	*col += n;
	return 0;
}

/* continuation piece of a long tag value; may start a new line */
static int
tag_more(dbuf *b, size_t *col, const char *piece)
{
	size_t          n = strlen(piece);

	if (*col + n > 76 && *col > 8 && fold_new(b, col))
		return -1;
	if (dbuf_catb(b, piece, n))
		return -1;
	*col += n;
	return 0;
}

/* dkim-quoted-printable (RFC 6376 2.11) of header value, for z= */
static int
dkim_qp(dbuf *out, const char *s, size_t n)
{
	size_t          i;
	static stralloc t;

	for (i = 0; i < n; i++) {
		unsigned char   c = s[i];

		if (c == '\r' || c == '\n')
			continue;
		if (c < 0x21 || c > 0x7e || c == ';' || c == '=' || c == '|') {
			if (qsprintf(&t, "=%02X", c) == -1)
				return -1;
			if (dbuf_cats(out, t.s))
				return -1;
		} else
		if (dbuf_catc(out, c))
			return -1;
	}
	return 0;
}

static int
no_passphrase_cb(char *buf, int size, int rwflag, void *u)
{
	(void) buf; (void) size; (void) rwflag; (void) u;
	return 0;
}

static int
sign_one(DKIMSign *s, const DKIMSignOptions *o, int hash, EVP_PKEY *pkey, dbuf *out)
{
	int             hc_relaxed, bc_relaxed, is_ed = (hash == DKIM_HASH_ED25519), i, r = 0, nsel = 0, from_ok = 0;
	const char     *alg;
	const char     *domain;
	char            numbuf[64];
	dbuf            sig = {0}, hdata = {0}, tmp = {0}, hnames = {0};
	bodyhash       *bh;
	size_t          col, siglen = 0, j;
	unsigned char  *sigbytes = NULL;
	EVP_MD_CTX     *mc = NULL;
	hdr             sh;
	int            *sel = NULL;

	if (!(domain = dkim_sign_domain(s)))
		return DKIM_NO_SENDER;
	for (i = 0; i < s->msg.hl.n; i++)
		if (hdr_is(&s->msg.hl.v[i], "from"))
			from_ok = 1;
	if (!from_ok && !s->opts.szDomain[0] && !s->opts.szDefaultDomain[0])
		return DKIM_NO_SENDER;
	if (o->szIdentity[0]) {
		const char     *at = strchr(o->szIdentity, '@');

		if (!at || !domain_in(at + 1, domain))
			return DKIM_BAD_IDENTITY;
	}
	switch (o->nCanon)
	{
	case DKIM_SIGN_SIMPLE:         hc_relaxed = 0; bc_relaxed = 0; break;
	case DKIM_SIGN_RELAXED:        hc_relaxed = 1; bc_relaxed = 1; break;
	case DKIM_SIGN_RELAXED_SIMPLE: hc_relaxed = 1; bc_relaxed = 0; break;
	case DKIM_SIGN_SIMPLE_RELAXED: hc_relaxed = 0; bc_relaxed = 1; break;
	default:                       return DKIM_FAIL;
	}
	switch (hash)
	{
	case DKIM_HASH_SHA1:    alg = "rsa-sha1"; break;
	case DKIM_HASH_SHA256:  alg = "rsa-sha256"; break;
	case DKIM_HASH_ED25519: alg = "ed25519-sha256"; break;
	default:                return DKIM_FAIL;
	}
	if ((is_ed && EVP_PKEY_base_id(pkey) != EVP_PKEY_ED25519) ||
			(!is_ed && EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA))
		return DKIM_BAD_PRIVATE_KEY;
	bh = &s->bh[bc_relaxed * 2 + (hash == DKIM_HASH_SHA1 ? 0 : 1)];

	/* choose headers, walking bottom-up so h= order matches verifier's selection */
	if (!(sel = calloc(s->msg.hl.n + 1, sizeof(int))))
		return DKIM_OUT_OF_MEMORY;
	for (i = s->msg.hl.n - 1; i >= 0; i--) {
		hdr            *h = &s->msg.hl.v[i];

		if (!h->name_len)
			continue;
		if (hdr_is(h, "from") || !s->opts.pfnHeaderCallback || s->opts.pfnHeaderCallback(h->raw))
			sel[nsel++] = i;
	}

	/* DKIM-Signature header up to and including "b=" */
	if (dbuf_cats(&sig, "DKIM-Signature:"))
		goto nomem;
	col = 15;
	if (tag_add(&sig, &col, "v=1;"))
		goto nomem;
	snprintf(numbuf, sizeof(numbuf), "a=%s;", alg);
	if (tag_add(&sig, &col, numbuf))
		goto nomem;
	snprintf(numbuf, sizeof(numbuf), "c=%s/%s;", hc_relaxed ? "relaxed" : "simple", bc_relaxed ? "relaxed" : "simple");
	if (tag_add(&sig, &col, numbuf))
		goto nomem;
	if (dbuf_cats(&tmp, "d=") || dbuf_cats(&tmp, domain) || dbuf_catc(&tmp, ';') || tag_add(&sig, &col, tmp.s))
		goto nomem;
	dbuf_reset(&tmp);
	if (dbuf_cats(&tmp, "s=") || dbuf_cats(&tmp, o->szSelector) || dbuf_catc(&tmp, ';') || tag_add(&sig, &col, tmp.s))
		goto nomem;
	dbuf_reset(&tmp);
	if (o->nIncludeTimeStamp) {
		snprintf(numbuf, sizeof(numbuf), "t=%lu;", (unsigned long) time(NULL));
		if (tag_add(&sig, &col, numbuf))
			goto nomem;
	}
	if (o->expireTime > 0) {
		snprintf(numbuf, sizeof(numbuf), "x=%lu;", (unsigned long) o->expireTime);
		if (tag_add(&sig, &col, numbuf))
			goto nomem;
	}
	if (o->nIncludeQueryMethod && tag_add(&sig, &col, "q=dns/txt;"))
		goto nomem;
	if (o->nIncludeBodyLengthTag) {
		snprintf(numbuf, sizeof(numbuf), "l=%llu;", bh->total_len);
		if (tag_add(&sig, &col, numbuf))
			goto nomem;
	}
	if (o->szIdentity[0]) {
		if (dbuf_cats(&tmp, "i=") || dbuf_cats(&tmp, o->szIdentity) || dbuf_catc(&tmp, ';') ||
				tag_add(&sig, &col, tmp.s))
			goto nomem;
		dbuf_reset(&tmp);
	}
	/* h= */
	for (i = 0; i < nsel; i++) {
		hdr            *h = &s->msg.hl.v[sel[i]];

		dbuf_reset(&tmp);
		if (dbuf_catb(&tmp, h->raw, h->name_len))
			goto nomem;
		for (j = 0; j < tmp.len; j++)
			tmp.s[j] = tolower((unsigned char) tmp.s[j]);
		if (dbuf_catc(&tmp, i + 1 < nsel ? ':' : ';'))
			goto nomem;
		if (i == 0) {
			dbuf_reset(&hnames);
			if (dbuf_cats(&hnames, "h=") || dbuf_cats(&hnames, tmp.s))
				goto nomem;
			if (tag_add(&sig, &col, hnames.s))
				goto nomem;
		} else
		if (tag_more(&sig, &col, tmp.s))
			goto nomem;
	}
	dbuf_reset(&tmp);
	/* z= */
	if (o->nIncludeCopiedHeaders) {
		for (i = nsel - 1, j = 0; i >= 0; i--, j++) {
			hdr            *h = &s->msg.hl.v[sel[i]];

			dbuf_reset(&tmp);
			if (j == 0 && dbuf_cats(&tmp, "z="))
				goto nomem;
			if (dbuf_catb(&tmp, h->raw, h->name_len) || dbuf_catc(&tmp, ':') ||
					dkim_qp(&tmp, h->raw + h->colon + 1, h->len - h->colon - 1) ||
					dbuf_catc(&tmp, i > 0 ? '|' : ';'))
				goto nomem;
			if (j == 0) {
				if (tag_add(&sig, &col, tmp.s))
					goto nomem;
			} else
			if (tag_more(&sig, &col, tmp.s))
				goto nomem;
		}
		dbuf_reset(&tmp);
	}
	/* bh= */
	if (dbuf_cats(&tmp, "bh=") || b64_encode(&tmp, bh->digest, bh->dlen) || dbuf_catc(&tmp, ';') ||
			tag_add(&sig, &col, tmp.s))
		goto nomem;
	dbuf_reset(&tmp);
	if (tag_add(&sig, &col, "b="))
		goto nomem;

	/* header hash input: selected headers + this header with empty b= and no final CRLF */
	for (i = 0; i < nsel; i++)
		if (canon_header(&hdata, &s->msg.hl.v[sel[i]], hc_relaxed, 1) < 0)
			goto nomem;
	memset(&sh, 0, sizeof(sh));
	sh.raw = sig.s;
	sh.len = sig.len;
	sh.colon = 14;
	sh.name_len = 14;
	if (canon_header(&hdata, &sh, hc_relaxed, 0) < 0)
		goto nomem;

	if (!(mc = EVP_MD_CTX_new()))
		goto nomem;
	if (is_ed) {
		unsigned char   dg[EVP_MAX_MD_SIZE];
		unsigned int    dl = 0;

		if (EVP_Digest(hdata.s, hdata.len, dg, &dl, EVP_sha256(), NULL) != 1) {
			r = DKIM_EVP_DIGEST_FAILURE;
			goto done;
		}
		if (EVP_DigestSignInit(mc, NULL, NULL, NULL, pkey) != 1 ||
				EVP_DigestSign(mc, NULL, &siglen, dg, dl) != 1 ||
				!(sigbytes = malloc(siglen)) ||
				EVP_DigestSign(mc, sigbytes, &siglen, dg, dl) != 1) {
			r = DKIM_EVP_SIGN_FAILURE;
			goto done;
		}
	} else {
		if (EVP_DigestSignInit(mc, NULL, hash == DKIM_HASH_SHA1 ? EVP_sha1() : EVP_sha256(), NULL, pkey) != 1 ||
				EVP_DigestSign(mc, NULL, &siglen, (unsigned char *) hdata.s, hdata.len) != 1 ||
				!(sigbytes = malloc(siglen)) ||
				EVP_DigestSign(mc, sigbytes, &siglen, (unsigned char *) hdata.s, hdata.len) != 1) {
			r = DKIM_EVP_SIGN_FAILURE;
			goto done;
		}
	}
	dbuf_reset(&tmp);
	if (b64_encode(&tmp, sigbytes, siglen))
		goto nomem;
	for (j = 0; j < tmp.len; j++) {
		if (col >= 76 && fold_new(&sig, &col))
			goto nomem;
		if (dbuf_catc(&sig, tmp.s[j]))
			goto nomem;
		col++;
	}
	/* CRLF -> LF for output; the verifier sees LF as CRLF so the hash is unchanged */
	for (j = 0; j < sig.len; j++) {
		if (sig.s[j] == '\r' && j + 1 < sig.len && sig.s[j + 1] == '\n')
			continue;
		if (dbuf_catc(out, sig.s[j]))
			goto nomem;
	}
	if (dbuf_catc(out, '\n'))
		goto nomem;
	goto done;
nomem:
	r = DKIM_OUT_OF_MEMORY;
done:
	if (mc)
		EVP_MD_CTX_free(mc);
	free(sigbytes);
	free(sel);
	dbuf_free(&sig);
	dbuf_free(&hdata);
	dbuf_free(&tmp);
	dbuf_free(&hnames);
	return r;
}

int
dkim_sign_get(DKIMSign *s, const DKIMSignOptions *o, const char *pem, char **sig)
{
	BIO            *bio;
	EVP_PKEY       *pkey;
	dbuf            out = {0};
	int             r;

	if (!s || !o || !pem || !sig)
		return DKIM_INVALID_CONTEXT;
	*sig = NULL;
	if ((r = sign_finish(s)))
		return r;
	if (!(bio = BIO_new_mem_buf(pem, -1)))
		return DKIM_OUT_OF_MEMORY;
	pkey = PEM_read_bio_PrivateKey(bio, NULL, no_passphrase_cb, NULL);
	BIO_free(bio);
	if (!pkey) {
		ERR_clear_error();
		return DKIM_BAD_PRIVATE_KEY;
	}
	if (o->nHash == DKIM_HASH_SHA1_AND_SHA256) {
		if (!(r = sign_one(s, o, DKIM_HASH_SHA1, pkey, &out)))
			r = sign_one(s, o, DKIM_HASH_SHA256, pkey, &out);
	} else
		r = sign_one(s, o, o->nHash, pkey, &out);
	EVP_PKEY_free(pkey);
	if (r) {
		dbuf_free(&out);
		return r;
	}
	*sig = out.s;
	return 0;
}

/* ================================================================== */
/* VERIFYING                                                           */
/* ================================================================== */
typedef struct {
	int             hdr_idx;
	char           *copy;        /*- header value (after colon), parsed in place */
	tag_t           tags[MAX_TAGS];
	int             ntags;
	int             hash;        /*- DKIM_HASH_SHA1 / SHA256 / ED25519 */
	int             hc_relaxed, bc_relaxed;
	int             has_l;
	unsigned long long l;
	int             result;
	int             bh_ready;
	bodyhash        bh;
	DKIMVerifyDetails detail;
} vsig;

struct DKIMVerify {
	DKIMVerifyOptions opts;
	message         msg;
	vsig            sigs[DKIM_MAX_VERIFY_SIGNATURES];
	DKIMVerifyDetails details[DKIM_MAX_VERIFY_SIGNATURES];
	int             nsigs;
	int             began;
	int             finished;
	char            author[256];
	int             have_author;
};

DKIMVerify     *
dkim_verify_new(const DKIMVerifyOptions *opts)
{
	DKIMVerify     *v;

	if (!(v = calloc(1, sizeof(*v))))
		return NULL;
	if (opts)
		v->opts = *opts;
	return v;
}

static int
all_digits(const char *s, unsigned long long *val)
{
	unsigned long long n = 0;

	if (!*s)
		return 0;
	for (; *s; s++) {
		if (*s < '0' || *s > '9')
			return 0;
		if (n > (~0ULL - 9) / 10)
			return 0;
		n = n * 10 + (*s - '0');
	}
	*val = n;
	return 1;
}

/* count occurrences of name in colon-separated h= list */
static int
h_count(const char *h, const char *name)
{
	size_t          nl = strlen(name);
	int             c = 0;
	const char     *p = h;

	while (*p) {
		const char     *s, *e;

		while (*p && (IS_FWS(*p) || *p == ':'))
			p++;
		s = p;
		while (*p && *p != ':')
			p++;
		for (e = p; e > s && IS_FWS(e[-1]); e--);
		if ((size_t) (e - s) == nl && !strncasecmp(s, name, nl))
			c++;
	}
	return c;
}

static void
verify_parse_sig(DKIMVerify *v, vsig *sg)
{
	const char     *a, *c, *d, *i, *h, *x, *t, *l;
	unsigned long long xv = 0, tv = 0;
	int             k, nfrom = 0;
	char           *cc;

	sg->result = 0;
	if ((sg->ntags = parse_tags(sg->copy, sg->tags, MAX_TAGS)) < 0) {
		sg->result = DKIM_BAD_SYNTAX;
		return;
	}
#define T(n) tag_get(sg->tags, sg->ntags, n)
	if (!(a = T("a")) || !T("b") || !T("bh") || !(d = T("d")) || !(h = T("h")) || !T("s") || !T("v")) {
		sg->result = DKIM_BAD_SYNTAX;
		return;
	}
	if (strcmp(T("v"), "1")) {
		sg->result = DKIM_STAT_INCOMPAT;
		return;
	}
	sg->detail.szDomain = xstrdup(d);
	sg->detail.szSelector = xstrdup(T("s"));
	if (!strcasecmp(a, "rsa-sha256"))
		sg->hash = DKIM_HASH_SHA256;
	else
	if (!strcasecmp(a, "rsa-sha1"))
		sg->hash = DKIM_HASH_SHA1;
	else
	if (!strcasecmp(a, "ed25519-sha256"))
		sg->hash = DKIM_HASH_ED25519;
	else {
		sg->result = DKIM_BAD_SYNTAX;
		return;
	}
	sg->detail.szAlgorithm = sg->hash == DKIM_HASH_SHA1 ? "rsa-sha1" :
		sg->hash == DKIM_HASH_SHA256 ? "rsa-sha256" : "ed25519-sha256";
	sg->hc_relaxed = sg->bc_relaxed = 0;
	if ((c = T("c"))) {
		if (!(cc = xstrdup(c))) {
			sg->result = DKIM_OUT_OF_MEMORY;
			return;
		}
		{
			char           *sl = strchr(cc, '/');
			const char     *hh = cc, *bb = "simple";

			if (sl) {
				*sl = 0;
				bb = sl + 1;
			}
			if (!strcasecmp(hh, "relaxed"))
				sg->hc_relaxed = 1;
			else
			if (strcasecmp(hh, "simple"))
				sg->result = DKIM_BAD_SYNTAX;
			if (!strcasecmp(bb, "relaxed"))
				sg->bc_relaxed = 1;
			else
			if (strcasecmp(bb, "simple"))
				sg->result = DKIM_BAD_SYNTAX;
		}
		free(cc);
		if (sg->result)
			return;
	}
	if (!*d || !*T("s") || !*h) {
		sg->result = DKIM_BAD_SYNTAX;
		return;
	}
	if ((i = T("i"))) {
		const char     *at = strchr(i, '@');

		if (!at || !domain_in(at + 1, d)) {
			sg->result = DKIM_BAD_IDENTITY;
			return;
		}
	}
	if ((l = T("l"))) {
		if (!all_digits(l, &sg->l)) {
			sg->result = DKIM_BAD_SYNTAX;
			return;
		}
		sg->has_l = 1;
	}
	if ((t = T("t")) && !all_digits(t, &tv)) {
		sg->result = DKIM_BAD_SYNTAX;
		return;
	}
	if ((x = T("x"))) {
		if (!all_digits(x, &xv) || (t && xv < tv)) {
			sg->result = DKIM_BAD_SYNTAX;
			return;
		}
		if ((unsigned long long) time(NULL) > xv) {
			sg->result = DKIM_SIGNATURE_EXPIRED;
			return;
		}
	}
	/* From: must be covered, every From: header of the message */
	for (k = 0; k < v->msg.hl.n; k++)
		if (hdr_is(&v->msg.hl.v[k], "from"))
			nfrom++;
	if (!v->opts.nAllowUnsignedFromHeaders) {
		int             hfrom = h_count(h, "from");

		if (hfrom < 1 || hfrom < nfrom) {
			sg->result = DKIM_UNSIGNED_FROM;
			return;
		}
	}
	if (v->opts.nSubjectRequired && !h_count(h, "subject")) {
		for (k = 0; k < v->msg.hl.n; k++) {
			if (hdr_is(&v->msg.hl.v[k], "subject")) {
				sg->result = DKIM_BAD_SYNTAX; /*- Subject present in message but not signed */
				return;
			}
		}
	}
#undef T
}

static int
verify_begin(DKIMVerify *v)
{
	static const char *const from_only[] = { "From", NULL };
	int             i;

	if (v->began)
		return 0;
	v->began = 1;
	v->have_author = find_domain(&v->msg.hl, from_only, v->author, sizeof(v->author));
	for (i = 0; i < v->msg.hl.n && v->nsigs < DKIM_MAX_VERIFY_SIGNATURES; i++) {
		hdr            *h = &v->msg.hl.v[i];
		vsig           *sg;

		if (!hdr_is(h, "dkim-signature"))
			continue;
		sg = &v->sigs[v->nsigs++];
		sg->hdr_idx = i;
		if (!(sg->copy = malloc(h->len - h->colon))) /*- len - (colon + 1) + 1 */
			return DKIM_OUT_OF_MEMORY;
		memcpy(sg->copy, h->raw + h->colon + 1, h->len - h->colon - 1);
		sg->copy[h->len - h->colon - 1] = 0;
		verify_parse_sig(v, sg);
		if (sg->result == DKIM_OUT_OF_MEMORY)
			return DKIM_OUT_OF_MEMORY;
		if (!sg->result) {
			if (bh_init(&sg->bh, sg->hash == DKIM_HASH_SHA1, sg->bc_relaxed,
						sg->has_l && v->opts.nHonorBodyLengthTag, sg->l))
				return DKIM_OUT_OF_MEMORY;
			sg->bh_ready = 1;
		}
	}
	return 0;
}

int
dkim_verify_process(DKIMVerify *v, const char *buf, size_t len)
{
	int             r, i;
	size_t          used;

	if (!v || v->finished)
		return DKIM_INVALID_CONTEXT;
	while (len) {
		if (!v->msg.headers_done) {
			if ((r = message_feed_headers(&v->msg, buf, len, &used)))
				return r;
			buf += used;
			len -= used;
			if (v->msg.headers_done && (r = verify_begin(v)))
				return r;
			continue;
		}
		for (i = 0; i < v->nsigs; i++)
			if (v->sigs[i].bh_ready)
				bh_feed(&v->sigs[i].bh, (const unsigned char *) buf, len);
		break;
	}
	return 0;
}

/* --- public key record --- */
static int
glob_match(const char *pat, const char *s) /*- g= : at most one '*' */
{
	const char     *star = strchr(pat, '*');
	size_t          pl, sl = strlen(s);

	if (!star)
		return !strcasecmp(pat, s);
	pl = star - pat;
	return sl >= pl + strlen(star + 1) &&
		!strncasecmp(pat, s, pl) &&
		!strcasecmp(star + 1, s + sl - strlen(star + 1));
}

static int
list_has(const char *list, const char *item, int sep)
{
	size_t          il = strlen(item);
	const char     *p = list;

	while (*p) {
		const char     *s, *e;

		while (*p && (IS_FWS(*p) || *p == sep))
			p++;
		s = p;
		while (*p && *p != sep)
			p++;
		for (e = p; e > s && IS_FWS(e[-1]); e--);
		if ((size_t) (e - s) == il && !strncasecmp(s, item, il))
			return 1;
	}
	return 0;
}

static int
verify_one(DKIMVerify *v, vsig *sg)
{
	const char     *d = sg->detail.szDomain, *sel = sg->detail.szSelector;
	char           *txt = NULL, *rec = NULL;
	static stralloc name = {0};
	tag_t           kt[MAX_TAGS];
	int             nk, r = 0, testing = 0, strict = 0;
	const char     *kv, *kk, *kh, *ks, *kp, *kg, *kt_;
	unsigned char  *pder = NULL, *sigbytes = NULL, *bhbytes = NULL;
	size_t          pl = 0, sl = 0, bl = 0;
	EVP_PKEY       *pkey = NULL;
	dbuf            hdata = {0}, sigcopy = {0};
	int            *used = NULL;
	const char     *hlist, *ival;
	int             k;

	/* 1. fetch the key record */
	if (v->opts.pszSelectorOverride)
		txt = xstrdup(v->opts.pszSelectorOverride);
	else {
		size_t          nl = strlen(sel) + strlen("._domainkey.") + strlen(d) + 1;

		if (nl > 254)
			return DKIM_SELECTOR_DOMAIN_NAME_TOO_LONG;
		if (qsprintf(&name, "%s._domainkey.%s", sel, d) == -1)
			return DKIM_OUT_OF_MEMORY;
		if (!v->opts.pfnDnsCallback)
			return DKIM_SELECTOR_DNS_TEMP_FAILURE;
		rec = v->opts.pfnDnsCallback(name.s);
		if (!rec)
			return DKIM_SELECTOR_DNS_TEMP_FAILURE;
		if (!strcmp(rec, "e=temp;")) {
			OPENSSL_free(rec);
			return DKIM_SELECTOR_DNS_TEMP_FAILURE;
		}
		if (!strcmp(rec, "e=perm;")) {
			OPENSSL_free(rec);
			return DKIM_SELECTOR_DNS_PERM_FAILURE;
		}
		txt = xstrdup(rec);
		OPENSSL_free(rec);
	}
	if (!txt)
		return DKIM_OUT_OF_MEMORY;
	if ((nk = parse_tags(txt, kt, MAX_TAGS)) < 0) {
		r = DKIM_SELECTOR_INVALID;
		goto done;
	}
	kv = tag_get(kt, nk, "v");
	kk = tag_get(kt, nk, "k");
	kh = tag_get(kt, nk, "h");
	ks = tag_get(kt, nk, "s");
	kp = tag_get(kt, nk, "p");
	kg = tag_get(kt, nk, "g");
	kt_ = tag_get(kt, nk, "t");
	if (kv && strcmp(kv, "DKIM1")) {
		r = DKIM_SELECTOR_INVALID;
		goto done;
	}
	if (!kp) {
		r = DKIM_SELECTOR_INVALID;
		goto done;
	}
	if (!*kp) {
		r = DKIM_SELECTOR_KEY_REVOKED;
		goto done;
	}
	if (ks && !list_has(ks, "*", ':') && !list_has(ks, "email", ':')) {
		r = DKIM_SELECTOR_INVALID;
		goto done;
	}
	if (kt_) {
		testing = list_has(kt_, "y", ':');
		strict = list_has(kt_, "s", ':');
	}
	if (kk && strcasecmp(kk, "rsa") && strcasecmp(kk, "ed25519")) {
		r = DKIM_SELECTOR_INVALID;
		goto done;
	}
	if ((sg->hash == DKIM_HASH_ED25519) != (kk && !strcasecmp(kk, "ed25519"))) {
		r = DKIM_SELECTOR_ALGORITHM_MISMATCH;
		goto done;
	}
	if (kh && !list_has(kh, sg->hash == DKIM_HASH_SHA1 ? "sha1" : "sha256", ':')) {
		r = DKIM_SELECTOR_ALGORITHM_MISMATCH;
		goto done;
	}
	ival = tag_get(sg->tags, sg->ntags, "i");
	if (strict && ival) {
		const char     *at = strchr(ival, '@');

		if (!at || strcasecmp(at + 1, d)) {
			r = DKIM_BAD_IDENTITY;
			goto done;
		}
	}
	if (kg && strcmp(kg, "*")) {
		const char     *at = ival ? strchr(ival, '@') : NULL;
		char           *lp = xstrdup(at ? ival : "");

		if (!lp) {
			r = DKIM_OUT_OF_MEMORY;
			goto done;
		}
		if (at)
			lp[at - ival] = 0;
		k = glob_match(kg, lp);
		free(lp);
		if (!k) {
			r = DKIM_SELECTOR_GRANULARITY_MISMATCH;
			goto done;
		}
	}
	if (b64_decode(kp, &pder, &pl) || !pl) {
		r = DKIM_SELECTOR_PUBLIC_KEY_INVALID;
		goto done;
	}
	if (sg->hash == DKIM_HASH_ED25519) {
		const unsigned char *q = pder;

		if (pl == 32)
			pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pder, pl);
		else
		if ((pkey = d2i_PUBKEY(NULL, &q, (long) pl)) && EVP_PKEY_base_id(pkey) != EVP_PKEY_ED25519) {
			EVP_PKEY_free(pkey);
			pkey = NULL;
		}
	} else {
		const unsigned char *q = pder;

		pkey = d2i_PUBKEY(NULL, &q, (long) pl);
		if (!pkey) {
			q = pder;
			pkey = d2i_PublicKey(EVP_PKEY_RSA, NULL, &q, (long) pl);
		}
		if (pkey && (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA || EVP_PKEY_bits(pkey) < 1024)) {
			EVP_PKEY_free(pkey);
			pkey = NULL;
		}
	}
	ERR_clear_error();
	if (!pkey) {
		r = DKIM_SELECTOR_PUBLIC_KEY_INVALID;
		goto done;
	}

	/* 2. body hash */
	if (bh_final(&sg->bh)) {
		r = DKIM_EVP_DIGEST_FAILURE;
		goto done;
	}
	if (b64_decode(tag_get(sg->tags, sg->ntags, "bh"), &bhbytes, &bl)) {
		r = DKIM_BAD_SYNTAX;
		goto done;
	}
	if (bl != sg->bh.dlen || memcmp(bhbytes, sg->bh.digest, bl)) {
		r = DKIM_BODY_HASH_MISMATCH;
		goto done;
	}

	/* 3. header hash and signature */
	if (b64_decode(tag_get(sg->tags, sg->ntags, "b"), &sigbytes, &sl) || !sl) {
		r = DKIM_BAD_SYNTAX;
		goto done;
	}
	if (!(used = calloc(v->msg.hl.n + 1, sizeof(int)))) {
		r = DKIM_OUT_OF_MEMORY;
		goto done;
	}
	hlist = tag_get(sg->tags, sg->ntags, "h");
	{
		const char     *p = hlist;

		while (*p) {
			const char     *s0, *e;
			size_t          nl;
			int             i;

			while (*p && (IS_FWS(*p) || *p == ':'))
				p++;
			s0 = p;
			while (*p && *p != ':')
				p++;
			for (e = p; e > s0 && IS_FWS(e[-1]); e--);
			nl = e - s0;
			if (!nl)
				continue;
			for (i = v->msg.hl.n - 1; i >= 0; i--) {
				hdr            *h = &v->msg.hl.v[i];

				if (!used[i] && h->name_len == nl && !strncasecmp(h->raw, s0, nl)) {
					used[i] = 1;
					if (canon_header(&hdata, h, sg->hc_relaxed, 1) < 0) {
						r = DKIM_OUT_OF_MEMORY;
						goto done;
					}
					break;
				}
			}
		}
	}
	/* the DKIM-Signature header itself with the value of b= removed */
	{
		hdr            *h = &v->msg.hl.v[sg->hdr_idx];
		size_t          i = h->colon + 1, vs = 0, ve = 0;
		int             found = 0;
		hdr             tmp;

		while (i <= h->len && !found) {
			size_t          ts, te;

			while (i < h->len && (IS_FWS(h->raw[i]) || h->raw[i] == ';'))
				i++;
			if (i >= h->len)
				break;
			ts = i;
			while (i < h->len && h->raw[i] != '=' && h->raw[i] != ';')
				i++;
			te = i;
			while (te > ts && IS_FWS(h->raw[te - 1]))
				te--;
			if (i < h->len && h->raw[i] == '=') {
				i++;
				vs = i;
				while (i < h->len && h->raw[i] != ';')
					i++;
				ve = i;
				if (te - ts == 1 && h->raw[ts] == 'b')
					found = 1;
			}
		}
		if (!found) {
			r = DKIM_BAD_SYNTAX;
			goto done;
		}
		if (dbuf_catb(&sigcopy, h->raw, vs) || dbuf_catb(&sigcopy, h->raw + ve, h->len - ve)) {
			r = DKIM_OUT_OF_MEMORY;
			goto done;
		}
		memset(&tmp, 0, sizeof(tmp));
		tmp.raw = sigcopy.s;
		tmp.len = sigcopy.len;
		tmp.colon = h->colon;
		tmp.name_len = h->name_len;
		if (canon_header(&hdata, &tmp, sg->hc_relaxed, 0) < 0) {
			r = DKIM_OUT_OF_MEMORY;
			goto done;
		}
	}
	{
		EVP_MD_CTX     *mc = EVP_MD_CTX_new();
		int             ok = 0;

		if (!mc) {
			r = DKIM_OUT_OF_MEMORY;
			goto done;
		}
		if (sg->hash == DKIM_HASH_ED25519) {
			unsigned char   dg[EVP_MAX_MD_SIZE];
			unsigned int    dl = 0;

			if (EVP_Digest(hdata.s, hdata.len, dg, &dl, EVP_sha256(), NULL) == 1 &&
					EVP_DigestVerifyInit(mc, NULL, NULL, NULL, pkey) == 1)
				ok = EVP_DigestVerify(mc, sigbytes, sl, dg, dl) == 1;
		} else {
			if (EVP_DigestVerifyInit(mc, NULL, sg->hash == DKIM_HASH_SHA1 ? EVP_sha1() : EVP_sha256(), NULL, pkey) == 1)
				ok = EVP_DigestVerify(mc, sigbytes, sl, (unsigned char *) hdata.s, hdata.len) == 1;
		}
		EVP_MD_CTX_free(mc);
		ERR_clear_error();
		if (!ok) {
			r = testing ? DKIM_SIGNATURE_BAD_BUT_TESTING : DKIM_SIGNATURE_BAD;
			goto done;
		}
	}
	/* verified */
	if (sg->has_l && v->opts.nHonorBodyLengthTag && sg->l < sg->bh.total_len)
		r = DKIM_SUCCESS_BUT_EXTRA;
	else
	if (v->have_author && !domain_in(v->author, d))
		r = DKIM_3PS_SIGNATURE;
	else
		r = DKIM_SUCCESS;
done:
	free(txt);
	free(pder);
	free(sigbytes);
	free(bhbytes);
	free(used);
	if (pkey)
		EVP_PKEY_free(pkey);
	dbuf_free(&hdata);
	dbuf_free(&sigcopy);
	return r;
}

int
dkim_verify_finish(DKIMVerify *v, int *nsigs, DKIMVerifyDetails **details)
{
	int             i, r;

	if (!v)
		return DKIM_INVALID_CONTEXT;
	if (!v->finished) {
		if ((r = message_end_headers(&v->msg)) || (r = verify_begin(v)))
			return r;
		for (i = 0; i < v->nsigs; i++) {
			vsig           *sg = &v->sigs[i];

			if (!sg->result) {
				sg->result = verify_one(v, sg);
				if (sg->result == DKIM_OUT_OF_MEMORY)
					return DKIM_OUT_OF_MEMORY;
			}
			sg->detail.nResult = sg->result;
		}
		v->finished = 1;
	}
	for (i = 0; i < v->nsigs; i++)
		v->details[i] = v->sigs[i].detail; /*- strings stay owned by the context */
	*nsigs = v->nsigs;
	*details = v->details;
	return 0;
}

const char     *
dkim_verify_author_domain(DKIMVerify *v)
{
	if (!v)
		return NULL;
	if (!v->msg.headers_done) {
		/* headers not complete yet: nothing known */
		return NULL;
	}
	return v->have_author ? v->author : NULL;
}

void
dkim_verify_free(DKIMVerify *v)
{
	int             i;

	if (!v)
		return;
	for (i = 0; i < v->nsigs; i++) {
		free(v->sigs[i].copy);
		free(v->sigs[i].detail.szDomain);
		free(v->sigs[i].detail.szSelector);
		if (v->sigs[i].bh_ready)
			bh_free(&v->sigs[i].bh);
	}
	message_free(&v->msg);
	free(v);
}

int
dkim_verify_status(int nsigs, const DKIMVerifyDetails *details)
{
	int             i, ret = DKIM_FAIL, failed = 0, extra = 0;

	if (!nsigs || !details)
		return DKIM_NO_SIGNATURES;
	for (i = 0; i < nsigs; i++) {
		/*-
		 * DKIM_3PS_SIGNATURE counts as a good signature (that is what the old
		 * code did); change it here if you want DKIMVERIFY=F to take effect.
		 */
		if (details[i].nResult == DKIM_SUCCESS || details[i].nResult == DKIM_3PS_SIGNATURE)
			return DKIM_SUCCESS;
		if (details[i].nResult == DKIM_SUCCESS_BUT_EXTRA) {
			extra = 1; /*- verified, but l= left part of the body uncovered */
			continue;
		}
		if (!failed) {
			ret = details[i].nResult;
			failed = 1;
		}
	}
	return extra ? DKIM_SUCCESS_BUT_EXTRA : ret;
}

#else /*- OPENSSL_VERSION_NUMBER < 0x10101000L */

#warning "dkim_functions.c needs OpenSSL 1.1.1 or later (Ed25519)"

/*-
 * Old OpenSSL: nothing can be signed or verified. All functions fail with
 * DKIM_OPENSSL_TOO_OLD. Functions that return a pointer return NULL.
 */
DKIMSign       *dkim_sign_new(const DKIMSignOptions *opts) { (void) opts; return NULL; }
int             dkim_sign_process(DKIMSign *s, const char *buf, size_t len) { (void) s; (void) buf; (void) len; return DKIM_OPENSSL_TOO_OLD; }
const char     *dkim_sign_domain(DKIMSign *s) { (void) s; return NULL; }
int             dkim_sign_get(DKIMSign *s, const DKIMSignOptions *opts, const char *privkey_pem, char **sig)
{
	(void) s; (void) opts; (void) privkey_pem;
	if (sig)
		*sig = NULL;
	return DKIM_OPENSSL_TOO_OLD;
}
void            dkim_sign_free(DKIMSign *s) { (void) s; }
DKIMVerify     *dkim_verify_new(const DKIMVerifyOptions *opts) { (void) opts; return NULL; }
int             dkim_verify_process(DKIMVerify *v, const char *buf, size_t len) { (void) v; (void) buf; (void) len; return DKIM_OPENSSL_TOO_OLD; }
int             dkim_verify_finish(DKIMVerify *v, int *nsigs, DKIMVerifyDetails **details)
{
	(void) v;
	if (nsigs)
		*nsigs = 0;
	if (details)
		*details = NULL;
	return DKIM_OPENSSL_TOO_OLD;
}
const char     *dkim_verify_author_domain(DKIMVerify *v) { (void) v; return NULL; }
void            dkim_verify_free(DKIMVerify *v) { (void) v; }
int             dkim_verify_status(int nsigs, const DKIMVerifyDetails *details) { (void) nsigs; (void) details; return DKIM_OPENSSL_TOO_OLD; }

#endif /*- OPENSSL_VERSION_NUMBER >= 0x10101000L */

/*-
 * $Log: dkim_functions.c,v $
 * Revision 1.2  2026-11-10 15:11:37+05:30  Cprogrammer
 * fix compile (albeit with stub) on older openssl versions
 *
 * Revision 1.1  2026-11-09 23:21:22+05:30  Cprogrammer
 * Initial revision
 *
 */
