/* $Id: register.c 6797 2009-02-14 21:13:00Z relson $ */

/* register.c -- read input with collect and register to persistent db */

#include "common.h"

#include <stdlib.h>

#include "bogofilter.h"
#include "datastore.h"
#include "collect.h"
#include "format.h"
#include "msgcounts.h"
#include "rand_sleep.h"
#include "register.h"
#include "wordhash.h"
#include "wordlists.h"

#define PLURAL(count) ((count == 1) ? "" : "s")

/*
 * tokenize text on stdin and register it to a specified list
 * and possibly out of another list
 */
void register_words(run_t _run_type, wordhash_t *h, u_int32_t msgcount)
{
    const char *r="",*u="";
    dsv_t val;
    hashnode_t *node;
    wordprop_t *wordprop;
    run_t save_run_type = run_type;
    int retrycount = 60;		/* we'll retry an aborted
					   registration five dozen times
					   before giving up. */
    bool first;

    u_int32_t wordcount = h->count;	/* use number of unique tokens */

    /* registrations always go to the default wordlist */
    wordlist_t *list = get_default_wordlist(word_lists);

    sh_t incr = IX_UNDF, decr = IX_UNDF;

    /* If update directory explicity supplied, setup the wordlists. */
    if (update_dir) {
	if (set_wordlist_dir(update_dir, PR_CFG_UPDATE) != 0) {
	    fprintf(stderr, "Can't find HOME or BOGOFILTER_DIR in environment.\n");
	    exit(EX_ERROR);
	}
    }

    if (_run_type & REG_SPAM)	{ r = "s"; incr = IX_SPAM; }
    if (_run_type & REG_GOOD)	{ r = "n"; incr = IX_GOOD; }
    if (_run_type & UNREG_SPAM)	{ u = "S"; decr = IX_SPAM; }
    if (_run_type & UNREG_GOOD)	{ u = "N"; decr = IX_GOOD; }

    if (wordcount == 0)
	msgcount = 0;

    format_set_counts(wordcount, msgcount);
    format_log_update(msg_register, msg_register_size, u, r);

    if (verbose)
	(void)fprintf(dbgout, "# %u word%s, %u message%s\n", 
		      wordcount, PLURAL(wordcount), msgcount, PLURAL(msgcount));

    /* When using auto-update with separate wordlists , 
       datastore.c needs to know which to update */

    run_type |= _run_type;

    first = true;

retry:
    if (first)
	first = false;
    else {
	if (verbose)
	    fprintf(stderr, "retrying registration after avoided deadlock...\n");
	begin_wordlist(list);
    }

    if (retrycount-- == 0) {
	fprintf(stderr, "retry count exceeded, giving up.\n");
	exit(EX_ERROR);
    }

    for (node = wordhash_first(h); node != NULL; node = wordhash_next(h))
    {
	wordprop = node->data;
	switch (ds_read(list->dsh, node->key, &val)) {
	    case DS_ABORT_RETRY:
		rand_sleep(4*1000,1000*1000);
		goto retry;
	    case 0:
	    case 1:
		break;
	    default:
		fprintf(stderr, "cannot read from data base.\n");
		exit(EX_ERROR);
	}
	if (incr != IX_UNDF) {
	    u_int32_t *counts = val.count;
	    counts[incr] += wordprop->freq;
	}
	if (decr != IX_UNDF) {
	    u_int32_t *counts = val.count;
	    counts[decr] = ((long)counts[decr] < wordprop->freq) ? 0 : counts[decr] - wordprop->freq;
	}
	switch (ds_write(list->dsh, node->key, &val)) {
	    case DS_ABORT_RETRY:
		rand_sleep(4*1000,1000*1000);
		goto retry;
	    case 0:
		break;
	    default:
		fprintf(stderr, "cannot write to data base.\n");
		exit(EX_ERROR);
	}
    }

    switch (ds_get_msgcounts(list->dsh, &val)) {
	case 0:
	case 1:
	    break;
	case DS_ABORT_RETRY:
	    rand_sleep(4 * 1000, 1000 * 1000);
	    goto retry;
	default:
	    fprintf(stderr, "cannot get message count values.\n");
	    exit(EX_ERROR);
    }
    list->msgcount[IX_SPAM] = val.spamcount;
    list->msgcount[IX_GOOD] = val.goodcount;

    if (incr != IX_UNDF)
	list->msgcount[incr] += msgcount;

    if (decr != IX_UNDF) {
	if (list->msgcount[decr] > msgcount)
	    list->msgcount[decr] -= msgcount;
	else
	    list->msgcount[decr] = 0;
    }

    val.spamcount = list->msgcount[IX_SPAM];
    val.goodcount = list->msgcount[IX_GOOD];

    switch (ds_set_msgcounts(list->dsh, &val)) {
	case 0:
	    break;
	case DS_ABORT_RETRY:
	    rand_sleep(4 * 1000, 1000 * 1000);
	    goto retry;
	default:
	    fprintf(stderr, "cannot set message count values\n");
	    exit(EX_ERROR);
    }

    if (DEBUG_REGISTER(1))
	(void)fprintf(dbgout, "bogofilter: list %s (%s) - %ul spam, %ul good\n",
		      list->listname, list->bfp->filepath, val.spamcount, val.goodcount);

    run_type = save_run_type;
}
