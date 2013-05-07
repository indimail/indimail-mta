/* $Id: maint.c 6862 2009-08-05 08:43:04Z m-a $ */

/*****************************************************************************

NAME:
   maint.c -- wordlist maintenance functions

AUTHOR:
   David Relson

******************************************************************************/

#include "common.h"

#include <assert.h>
#include <stdlib.h>

#include "buff.h"
#include "datastore.h"
#include "error.h"
#include "charset.h"
#ifndef	DISABLE_UNICODE
#include "convert_unicode.h"
#include "iconvert.h"
#endif
#include "maint.h"
#include "transaction.h"
#include "wordlists.h"
#include "xmalloc.h"
#include "xstrdup.h"

uint32_t thresh_count = 0;
YYYYMMDD thresh_date  = 0;
size_t	 size_min = 0;
size_t	 size_max = 0;
bool     timestamp_tokens = true;
bool	 upgrade_wordlist_version = false;

#ifndef	DISABLE_UNICODE
e_enc	 old_encoding;
e_enc	 new_encoding;
#define DEFAULT_OR_UNICODE(enc) (enc == E_RAW) ? charset_default : charset_unicode
#endif

/* Function Prototypes */

/* Function Definitions */

/* Keep high counts */
static bool keep_count(uint32_t count)
{
    bool ok = count > thresh_count;
    if (count > 0 && DEBUG_DATABASE(1))
	fprintf(dbgout, "keep_count:  %lu > %lu -> %c\n",
		(unsigned long)count, (unsigned long)thresh_count,
		ok ? 't' : 'f' );
    return ok;
}

/* Keep recent dates */
static bool keep_date(YYYYMMDD date)
{
    bool ok = thresh_date < date;
    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "keep_date: %ld < %ld -> %c\n",
		(long)thresh_date, (long)date, ok ? 't' : 'f' );
    return ok;
}

/* Keep sizes within bounds */
static bool keep_size(size_t size)
{
    bool ok = (size_min <= size) && (size <= size_max);
    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "keep_size:  %lu <= %lu <= %lu -> %c\n", 
		(unsigned long)size_min, (unsigned long)size, (unsigned long)size_max, 
		ok ? 't' : 'f' );
    return ok;
}

static void merge_tokens(const word_t *old_token, const word_t *new_token, dsv_t *in_val, ta_t *transaction, void *vhandle)
{
    int	  ret;
    dsv_t old_tmp;

    /* delete original token */
    ta_delete(transaction, vhandle, old_token);

    /* retrieve and update nonascii token*/
    ret = ta_read(transaction, vhandle, new_token, &old_tmp);

    if (ret == EX_OK) {
	in_val->spamcount += old_tmp.spamcount;
	in_val->goodcount += old_tmp.goodcount;
	in_val->date       = max(old_tmp.date, in_val->date);	/* date in form YYYYMMDD */
    }
    set_date(in_val->date);	/* set timestamp */
    ta_write(transaction, vhandle, new_token, in_val);
    set_date(0);
}

static void replace_token(const word_t *old_token, const word_t *new_token, dsv_t *in_val, ta_t *transaction, void *vhandle)
{
    /* delete original token */
    ta_delete(transaction, vhandle, old_token);	

    /* retrieve and update nonascii token*/
    set_date(in_val->date);	/* set timestamp */
    ta_write(transaction, vhandle, new_token, in_val);
    set_date(0);
}

/* Keep token if at least one user given constraint should be kept */
/* Discard if all user given constraints are satisfied */

bool discard_token(word_t *token, dsv_t *in_val)
{
    bool discard;

    if (token->u.text[0] == '.') {	/* keep .ENCODING, .MSG_COUNT, and .ROBX */
	if (strcmp((const char *)token->u.text, MSG_COUNT) == 0)
	    return false;
	if (strcmp((const char *)token->u.text, ROBX_W) == 0)
	    return false;
	if (strcmp((const char *)token->u.text, WORDLIST_ENCODING) == 0)
	    return false;
    }

    discard = (thresh_count != 0) || (thresh_date != 0) || (size_min != 0) || (size_max != 0);

    if (discard) {
	if (thresh_count != 0 &&
		(keep_count(in_val->spamcount) || keep_count(in_val->goodcount)))
	    discard = false;
	if (thresh_date != 0 && keep_date(in_val->date))
	    discard = false;
	if ((size_min != 0 || size_max != 0) &&
		keep_size(token->leng))
	    discard = false;
    }

    return discard;
}

bool do_replace_nonascii_characters(register byte *str, register size_t len)
{
    bool change = false;
    assert(str != NULL);
    while (len--) {
	if (*str & 0x80) {
	    *str = '?';
	    change = true;
	}
	str++;
    }
    return change;
}

struct userdata_t {
    void *vhandle;
    ta_t *transaction;
};

static int maintain_hook(word_t *w_key, dsv_t *in_val,
	void *userdata)
{
    size_t len;
    word_t token;
    void *vhandle = ((struct userdata_t *) userdata)->vhandle;
    ta_t *transaction = ((struct userdata_t *) userdata)->transaction;

    token.u.text = w_key->u.text;
    token.leng = w_key->leng;

    len = strlen(MSG_COUNT);
    if (len == token.leng && 
	    strncmp((char *)token.u.text, MSG_COUNT, token.leng) == 0)
	return EX_OK;

    if (discard_token(&token, in_val)) {
	int ret = ta_delete(transaction, vhandle, &token);
	if (DEBUG_DATABASE(0))
	    fprintf(dbgout, "deleting '%.*s'\n", (int)min(INT_MAX, token.leng), (char *)token.u.text);
	return ret;
    }

    if (replace_nonascii_characters)
    {
	word_t new_token;
	new_token.u.text = (byte *)xmalloc(token.leng + 1);
	memcpy(new_token.u.text, token.u.text, token.leng);
	new_token.leng = token.leng;
	new_token.u.text[new_token.leng] = '\0';
	if (do_replace_nonascii_characters(new_token.u.text, new_token.leng))
	    merge_tokens(&token, &new_token, in_val, transaction, vhandle);
	xfree(new_token.u.text);
    }

#ifndef	DISABLE_UNICODE
    if (old_encoding != new_encoding)
    {
	buff_t new_buff;
	buff_t old_buff;

	old_buff.read = 0;
	old_buff.size = token.leng;
	old_buff.t.u.text = token.u.text;
	old_buff.t.leng = token.leng;

	new_buff.read = 0;
	new_buff.size = token.leng * 6;
	new_buff.t.leng = 0;
	new_buff.t.u.text = (byte *)xmalloc(new_buff.size);

	iconvert(&old_buff, &new_buff);

	if (old_buff.t.leng != new_buff.t.leng ||
	    memcmp(old_buff.t.u.text, new_buff.t.u.text, new_buff.t.leng) != 0) {
	    if (DEBUG_ICONV(2)) {
		fputs("***  ", dbgout); word_puts(&old_buff.t, 0, dbgout); fputs( "\n", dbgout);
		fputs("***  ", dbgout); word_puts(&new_buff.t, 0, dbgout); fputs( "\n", dbgout);

	    }
	    merge_tokens(&old_buff.t, &new_buff.t, in_val, transaction, vhandle);
	}

	xfree(new_buff.t.u.text);
    }
#endif

    if (upgrade_wordlist_version)
    {
	switch (wordlist_version)
	{
	    case IP_PREFIX:
		{
		    /* up-to-date - nothing to do */
		    break;
		}
	    case 0:
		{
		    /* update to "ip:" prefix level */

		    const char  *url_hdr = "url:";
		    size_t       url_len = strlen(url_hdr);
		    const char  *ip_hdr  = "ip:";
		    size_t       ip_len  = strlen(ip_hdr);

		    if (token.leng > url_len && memcmp(token.u.text, url_hdr, url_len) == 0)
		    {
			word_t new_token;
			new_token.leng = token.leng + ip_len -  url_len;
			new_token.u.text = (byte *)xmalloc(new_token.leng + 1);
			memcpy(new_token.u.text, ip_hdr, ip_len);
			memcpy(new_token.u.text+ip_len, token.u.text+url_len, token.leng - url_len);
			new_token.u.text[new_token.leng] = '\0';
			replace_token(&token, &new_token, in_val, transaction, vhandle);
			xfree(new_token.u.text);
		    }
		    break;
		}
	}
    }

    return EX_OK;
}

static bool check_wordlist_version(dsh_t *dsh)
{
    dsv_t val;
    ds_get_wordlist_version(dsh, &val);
    if (val.count[0] >= CURRENT_VERSION)
	return true;
    else
	return false;
}

static ex_t maintain_wordlist(void *database)
{
    ta_t *transaction = ta_init();
    struct userdata_t userdata;
    ex_t ret;
    bool done = false;

    userdata.vhandle = database;
    userdata.transaction = transaction;

    if (DST_OK == ds_txn_begin(database)) {
#ifndef	DISABLE_UNICODE
	dsv_t val;
	int rc = ds_get_wordlist_encoding(database, &val);
	new_encoding = encoding;
	if (rc == 0)
	    old_encoding = val.spamcount;	/* found */
	else
	    old_encoding = E_RAW;		/* not found */
	if (old_encoding != new_encoding) {
	    const char *from_charset = DEFAULT_OR_UNICODE(old_encoding);
	    const char *to_charset   = DEFAULT_OR_UNICODE(new_encoding);
	    init_charset_table_iconv(from_charset, to_charset);
	}
#endif
	ret = ds_foreach(database, maintain_hook, &userdata);
    } else
	ret = EX_ERROR;

    if (upgrade_wordlist_version) {
	done = check_wordlist_version(database);
	if (!done)
	    fprintf(dbgout, "Upgrading wordlist.\n");
	else
	    fprintf(dbgout, "Wordlist has already been upgraded.\n");
    }

    if (!done && upgrade_wordlist_version)
    {
	dsv_t val;
	val.count[0] = CURRENT_VERSION;
	val.count[1] = 0;
	ds_set_wordlist_version(database, &val);
    }

#ifndef	DISABLE_UNICODE
    if (old_encoding != new_encoding) {
	dsv_t val;
	word_t enco;

	enco.u.text = (byte *)xstrdup(WORDLIST_ENCODING);
	enco.leng = strlen(WORDLIST_ENCODING);
	val.count[0] = new_encoding;
	val.count[1] = 0;
	val.date     = 0;

	ds_write(database, &enco, &val);
	xfree(enco.u.text);
    }
#endif

    if (ta_commit(transaction) != TA_OK)
	ret = EX_ERROR;

    if (DST_OK != ds_txn_commit(database))
	ret = EX_ERROR;

    return ret;
}

ex_t maintain_wordlist_file(bfpath *bfp)
{
    ex_t rc;
    dsh_t *dsh;
    void *dbe;

    dbe = ds_init(bfp);

    dsh = ds_open(dbe, bfp, DS_WRITE);

    if (dsh == NULL)
	return EX_ERROR;

    rc = maintain_wordlist(dsh);

    ds_close(dsh);
    ds_cleanup(dbe);

    return rc;
}
