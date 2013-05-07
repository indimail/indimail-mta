/* $Id: collect.c 6766 2009-01-12 04:27:36Z relson $ */

/* collect.c -- tokenize input and cap word frequencies, return a wordhash */

#include "common.h"

#include <assert.h>
#include <stdlib.h>

#include "charset.h"
#include "mime.h"
#include "wordhash.h"
#include "token.h"

#include "collect.h"

void wordprop_init(void *vwordprop)
{
    wordprop_t *wp = vwordprop;
    memset(wp, 0, sizeof(*wp));
}

void wordcnts_init(void *vwordcnts)
{
    wordcnts_t *wc = vwordcnts;
    memset(wc, 0, sizeof(*wc));
}

void wordcnts_incr(wordcnts_t *w1, wordcnts_t *w2)
{
    w1->good += w2->good;
    w1->bad  += w2->bad;
}

/* Tokenize input text and save words in the wordhash_t hash table.
 *
 * Returns:  true if the EOF token has not been read.
 */
void collect_words(wordhash_t *wh)
{
    if (DEBUG_WORDLIST(2)) fprintf(dbgout, "### collect_words() begins\n");

    lexer_init();

    for (;;){
	wordprop_t *wp;
	word_t token;
	token_t cls = get_token( &token );

	if (cls == NONE)
	    break;

	if (cls == BOGO_LEX_LINE)
	{
	    char *beg = (char *)token.u.text+1;	/* skip leading quote mark */
	    char *end = strchr(beg, '"');
	    assert(end);
	    token.leng = end - beg;
	    memmove(token.u.text, token.u.text + 1, token.leng + 1);
	    token.u.text[token.leng] = '\0';	/* ensure nul termination */
	}

	wp = wordhash_insert(wh, &token, sizeof(wordprop_t), &wordprop_init);
	if (wh->type != WH_CNTS)
	    wp->freq = 1;

/******* EK **********/

#ifdef	CP866
/* mime charset hack */
	{
	    static bool hasCharset=false;
	    if (hasCharset)  /* prev token == charset */
	    {
		if (token.leng > 5 &&
		    !strncmp(token.text, "mime:", 5))
		    set_charset(token.text+5);
	    }
	    hasCharset = 0;
	    if (token.leng == 5+7)
	    {
		if (!strncmp(token.text, "mime:", 5) &&
		    !strncasecmp(token.text+5, "charset", 7))
		    hasCharset = true;
	    }
	}
#endif

/******* end of EK addition **********/

	if (DEBUG_WORDLIST(3)) {
	    fprintf(dbgout, "%3d ", (int) wh->count);
	    word_puts(&token, 0, dbgout);
	    fputc('\n', dbgout);
	}

	if (cls == BOGO_LEX_LINE)
	{
	    char *s = (char *)token.u.text;
	    s += token.leng + 2;
	    wp->cnts.bad = atoi(s);
	    s = strchr(s+1, ' ') + 1;
	    wp->cnts.good = atoi(s);
	    wp->cnts.msgs_good = msgs_good;
	    wp->cnts.msgs_bad = msgs_bad;
	}
    }
    
    if (DEBUG_WORDLIST(2)) fprintf(dbgout, "### collect_words() ends\n");

    return;
}
