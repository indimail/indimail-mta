/* $Id: robx.c 6875 2010-02-14 20:57:34Z relson $ */

/*****************************************************************************

NAME:
   robx.c -- computes robx value by reading wordlist.db

AUTHOR:
   David Relson - C version
   Greg Lous - perl version
   
******************************************************************************/

#include <errno.h>

#include "common.h"

#include "datastore.h"
#include "rand_sleep.h"
#include "robx.h"
#include "wordlists.h"

/* Function Prototypes */

/* Function Definitions */

typedef struct robhook_data {
    double   sum;
    uint32_t count;
    uint32_t spam_cnt;
    uint32_t good_cnt;
    dsh_t    *dsh;
    double   scalefactor;
} rhd_t;

static void robx_accum(rhd_t *rh, 
		       word_t *key,
		       dsv_t *data)
{
    uint32_t goodness = data->goodcount;
    uint32_t spamness = data->spamcount;
    double prob = spamness / (goodness * rh->scalefactor + spamness);
    bool doit = goodness + spamness >= 10;

    if (doit) {
	rh->sum += prob;
	rh->count += 1;
    }

    /* print if -vvv and token in both word lists, or -vvvv */
    if ((verbose > 2 && doit) || verbose > 3) {
	fprintf(dbgout, "cnt: %4lu,  sum: %11.6f,  ratio: %9.6f,"
		"  sp: %3lu,  gd: %3lu,  p: %9.6f,  t: %.*s\n", 
		(unsigned long)rh->count, rh->sum, rh->sum / rh->count,
		(unsigned long)spamness, (unsigned long)goodness, prob,
		CLAMP_INT_MAX(key->leng), key->u.text);
    }
}

static int robx_hook(word_t *key, dsv_t *data, 
		     void *userdata)
{
    struct robhook_data *rh = userdata;

    /* ignore system meta-data */
    if (*key->u.text != '.')
	robx_accum(rh, key, data);

    return 0;
}

/** returns negative for failure.
 * used by bogoutil and bogotune */
double compute_robinson_x(void)
{
    int ret;
    double rx;
    dsh_t *dsh;
    wordlist_t *wordlist;

    struct robhook_data rh;

    open_wordlists(DS_READ);
    wordlist = get_default_wordlist(word_lists);

    dsh = wordlist->dsh;

    rh.spam_cnt = wordlist->msgcount[IX_SPAM];
    rh.good_cnt = wordlist->msgcount[IX_GOOD];

    if (rh.spam_cnt == 0 || rh.good_cnt == 0)
	wordlist_error(ENOENT);
    
    rh.scalefactor = (double)rh.spam_cnt/(double)rh.good_cnt;

    rh.dsh = dsh;
    rh.sum = 0.0;
    rh.count = 0;

    do {
	ret = ds_foreach(dsh, robx_hook, &rh);
	if (ret == DS_ABORT_RETRY) {
	    rand_sleep(1000, 1000000);
	    begin_wordlist(wordlist);
	}
    } while (ret == DS_ABORT_RETRY);

    rx = rh.sum/rh.count;
    if (rh.count == 0)
	ret = -1;
    if (verbose > 2)
	printf("%s: %u, %u, scale: %f, sum: %f, cnt: %6d, .ROBX: %f\n",
	       MSG_COUNT, rh.spam_cnt, rh.good_cnt,
	       rh.scalefactor, rh.sum, (int)rh.count, rx);

    close_wordlists(true);

    return ret ? -1 : rx;
}
