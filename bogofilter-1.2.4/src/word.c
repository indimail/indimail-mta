/* $Id: word.c 6976 2012-12-02 20:49:49Z relson $ */

/** \file word.c
 * support for strings of arbitrary content, implementation
 *
 * \author David Relson <relson@osagesoftware.com>
 */

#include "common.h"

#include <stdarg.h>

#include "word.h"
#include "xmalloc.h"

/* Function Definitions */

word_t  *word_news(const char *cstring)
{
    return word_new((const byte *)cstring, strlen(cstring));
}

word_t *word_new(const byte *text, uint len)
{
    /* to lessen malloc/free calls, allocate struct and data in one block */
    word_t *self = xmalloc(sizeof(word_t)+len+1);
    self->leng = len;
    self->u.text = (byte *)((char *)self+sizeof(word_t));
    if (text != NULL) {
	memcpy(self->u.text, text, len);
	self->u.text[len] = '\0';			/* ensure nul termination */
    } else {
	self->u.text[0] = '\0';			/* ditto for text == NULL */
    }
    return self;
}

int word_cmp(const word_t *w1, const word_t *w2)
{
	int r;
	uint l;
	if (w2) {
    	l = min(w1->leng, w2->leng);
    	r = memcmp((const char *)w1->u.text, (const char *)w2->u.text, l);
	} else
		return -1;
    if (r) return r;
    if (w1->leng > w2->leng) return 1;
    if (w1->leng < w2->leng) return -1;
    return 0;
}

int word_cmps(const word_t *w, const char *s)
{
    word_t w2;
    w2.leng = strlen(s);
    w2.u.ctext = s;
    return word_cmp(w, &w2);
}

word_t *word_concat(const word_t *w1, const word_t *w2)
{
    uint len = w1->leng + w2->leng;
    word_t *ans = word_new(NULL, len);
    memcpy(ans->u.text, w1->u.text, w1->leng);
    memcpy(ans->u.text+w1->leng, w2->u.text, w2->leng);
    Z(ans->u.text[ans->leng]);		/* for easier debugging - removable */
    return ans;
}

void word_puts(const word_t *word, uint width, FILE *fp)
{
    /* width = 0 - output all of the word
    **       > 0 - use 'width' as count, 
    **		   blank fill if 'width' < length
    */
    uint l = (width == 0) ? word->leng : min(width, word->leng);
    (void) fwrite(word->u.text, 1, l, fp);
    if (l < width)
	(void) fprintf(fp, "%*s", (int)(width - l), " ");
}
