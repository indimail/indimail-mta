/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	"hmac.h"


struct hmac_hashinfo *hmac_list[]= {HMAC_LIST};

struct hhki {
	const struct hmac_hashinfo *hh;
	const char *k;
        size_t kl;
	unsigned char *kxopad;
	unsigned char *kxipad;

	void *context;
	} ;

static void dohashkey(unsigned char *, void *);
static void docalcc(void *, void *);

void hmac_hashkey(const struct hmac_hashinfo *hh, const char *k,
        size_t kl, unsigned char *kxopad, unsigned char *kxipad)
{
struct hhki i;

	i.hh=hh;
	i.k=k;
	i.kl=kl;
	i.kxopad=kxopad;
	i.kxipad=kxipad;

	(*hh->hh_allocacontext)( docalcc, (void *)&i );
}

static void dokeycalc(struct hhki *);

static void docalcc(void *c, void *v)
{
struct hhki *i=(struct hhki *)v;

	i->context=c;

	if (i->kl > i->hh->hh_B)
		(*i->hh->hh_allocaval)(dohashkey, (void *)i);
	else
		dokeycalc( i );
}

static void dohashkey(unsigned char *keybuf, void *v)
{
struct hhki *i=(struct hhki *)v;

	(*i->hh->hh_init)(i->context);
	(*i->hh->hh_hash)(i->context, i->k, i->kl);
	(*i->hh->hh_endhash)(i->context, i->kl);
	(*i->hh->hh_getdigest)(i->context, keybuf);
	i->k=(char *)keybuf;
	i->kl=i->hh->hh_L;
	dokeycalc(i);
}

static void dokeycalc(struct hhki *i)
{
char	buf[64];	/* Random guess :-) */
unsigned n;
unsigned l;

	(*i->hh->hh_init)(i->context);
	n=0;
	for (l=0; l<i->hh->hh_B; l++)
	{
		buf[n] = ( l < i->kl ? i->k[l]:0) ^ 0x5C;
		if ( ++n >= sizeof(buf))
		{
			(*i->hh->hh_hash)(i->context, buf, sizeof(buf));
			n=0;
		}
	}
	if (n)
		(*i->hh->hh_hash)(i->context, buf, n);
	(*i->hh->hh_getdigest)(i->context, i->kxopad);

	(*i->hh->hh_init)(i->context);
	n=0;
	for (l=0; l<i->hh->hh_B; l++)
	{
		buf[n] = ( l < i->kl ? i->k[l]:0) ^ 0x36;
		if ( ++n >= sizeof(buf))
		{
			(*i->hh->hh_hash)(i->context, buf, sizeof(buf));
			n=0;
		}
	}
	if (n)
		(*i->hh->hh_hash)(i->context, buf, n);
	(*i->hh->hh_getdigest)(i->context, i->kxipad);
}

struct hhko {
	const struct hmac_hashinfo *hh;
	const char *t;
        size_t tl;
	const unsigned char *kxopad;
	const unsigned char *kxipad;
	unsigned char *hash;
	} ;

static void docalch(void *, void *);

void hmac_hashtext (
        const struct hmac_hashinfo *hh,
        const char *t,
        size_t tl,
        const unsigned char *kxopad,
        const unsigned char *kxipad,
        unsigned char *hash)
{
struct hhko o;

	o.hh=hh;
	o.t=t;
	o.tl=tl;
	o.kxopad=kxopad;
	o.kxipad=kxipad;
	o.hash=hash;

	(*hh->hh_allocacontext)( docalch, (void *)&o );
}

static void docalch(void *c, void *v)
{
struct hhko *o=(struct hhko *)v;

	(o->hh->hh_setdigest)(c, o->kxipad);
	(o->hh->hh_hash)(c, o->t, o->tl);
	(o->hh->hh_endhash)(c, o->tl+o->hh->hh_B);
	(o->hh->hh_getdigest)(c, o->hash);

	(o->hh->hh_setdigest)(c, o->kxopad);
	(o->hh->hh_hash)(c, o->hash, o->hh->hh_L);
	(o->hh->hh_endhash)(c, o->hh->hh_L + o->hh->hh_B);
	(o->hh->hh_getdigest)(c, o->hash);
}
