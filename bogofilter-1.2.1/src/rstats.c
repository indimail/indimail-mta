/* $Id: rstats.c 6816 2009-02-21 21:11:01Z relson $ */

/*****************************************************************************

NAME:
   rstats.c -- routines for printing robinson data for debugging.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "bogofilter.h"
#include "listsort.h"
#include "msgcounts.h"
#include "prob.h"
#include "rstats.h"
#include "score.h"
#include "xmalloc.h"

typedef struct rstats_s rstats_t;
struct rstats_s {
    rstats_t *next;
    const word_t *token;
    u_int32_t	good;
    u_int32_t	bad;
    u_int32_t	msgs_good;
    u_int32_t	msgs_bad;
    bool   used;
    double prob;
};

typedef struct rhistogram_s rhistogram_t;
struct rhistogram_s {
    size_t count;
    double prob;
    double spamicity;
};

typedef struct header_s header_t;
struct header_s {
    rstats_t *list;
    uint      count;		/* words in list */
    uint      robn;		/* words in score */
    FLOAT     p;		/* Robinson's P */
    FLOAT     q;		/* Robinson's Q */
    double    min_dev;
    double    spamicity;
};

static header_t *stats_head = NULL;
static rstats_t *stats_tail = NULL;

/* Function Prototypes */

static void rstats_print_histogram(size_t robn, rstats_t *rstats_head);
static void rstats_print_rtable(rstats_t *rstats_head);

/* Function Definitions */

void rstats_init(void)
{
    if (stats_head == NULL) {
	stats_head = xcalloc(1, sizeof(header_t));
	stats_tail = (rstats_t *)xcalloc(1, sizeof(rstats_t));
	stats_head->list = stats_tail;
    }
}

void rstats_cleanup(void)
{
    rstats_t *p, *q;

    for (p = stats_head->list; p != NULL; p = q)
    {
      q = p->next;
      xfree(p);
    }
    xfree(stats_head);
    stats_head = NULL;
    stats_tail = NULL;
}

void rstats_add(const word_t *token, double prob, bool used, wordcnts_t *cnts)
{
    if (token == NULL)
	return;

    stats_head->count += 1;
    stats_tail->next  = NULL;

    /* Using externally controlled data;
       token must not be freed before calling rstats_cleanup()
    */
    stats_tail->token = token;
    stats_tail->prob  = prob;
    stats_tail->used  = used;
    stats_tail->good  = cnts->good;
    stats_tail->bad   = cnts->bad;
    stats_tail->msgs_good = cnts->msgs_good;
    stats_tail->msgs_bad = cnts->msgs_bad;

    stats_tail->next = (rstats_t *)xcalloc(1, sizeof(rstats_t));
    stats_tail = stats_tail->next;
}

/* compare_rstats_t - sort by ascending spamicity */

static int compare_rstats_t(const void *const pv1, const void *const pv2)
{
    const rstats_t *r1 = (const rstats_t *)pv1;
    const rstats_t *r2 = (const rstats_t *)pv2;

    if (r1->prob > r2->prob) return 1;
    if (r2->prob > r1->prob) return -1;

    return word_cmp(r1->token, r2->token);
}

#define	INTERVALS	10

void rstats_fini(size_t robn, FLOAT P, FLOAT Q, double spamicity)
{
    stats_head->robn      = robn;
    stats_head->p         = P;
    stats_head->q         = Q;
    stats_head->spamicity = spamicity;
}

void rstats_print(bool unsure)
{
    size_t robn  = stats_head->robn;

    /* sort by ascending spamicity */
    stats_head->list = (rstats_t *)listsort((element *)stats_head->list, (fcn_compare *)&compare_rstats_t);

    if (Rtable || verbose>=3)
	rstats_print_rtable(stats_head->list);
    else if (verbose==2 || (unsure && verbose))
	rstats_print_histogram(robn, stats_head->list);
}

static void rstats_print_histogram(size_t robn, rstats_t *rstats_head)
{
    size_t i;
    size_t maxcnt=0;
    rstats_t *cur=rstats_head;
    rhistogram_t hist[INTERVALS];

    double invn = (double) robn;

    double invlogsum = 0.0;	/* Robinson's P */
    double logsum = 0.0;	/* Robinson's Q */

    if (!stats_in_header)
	(void)fprintf(fpo, "\n" );

    /* Compute histogram */
    for (i=0; i<INTERVALS; i+=1)
    {
	rhistogram_t *h = &hist[i];
	double fin = 1.0*(i+1)/INTERVALS;
	size_t cnt = 0;

	h->prob = 0.0;
	h->spamicity=0.0;

	while (cur != NULL)
	{
	    double prob = cur->prob;
	    if (prob >= fin)
		break;

	    if (cur->used)
	    {
		cnt += 1;
		h->prob += prob;
		invlogsum += log(1.0 - prob);
		logsum += log(prob);
	    }

	    cur = cur->next;
	}

	if (robn == 0)
	    h->spamicity = robx;
	else
	{
	    double invproduct, product;
	    invproduct = 1.0 - exp(invlogsum / invn);
	    product = 1.0 - exp(logsum / invn);
	    h->spamicity = (invproduct + product < EPS)
		? 0.0
		: (1.0 + (invproduct - product) / (invproduct + product)) / 2.0;
	}
	h->count=cnt;
	maxcnt = max(maxcnt, cnt);
    }

    (void)fprintf(fpo, "%s%4s %4s %6s  %9s %s\n", stats_prefix, "int", "cnt", "prob", "spamicity", "histogram" );

    /* Print histogram */
    for (i=0; i<INTERVALS; i+=1)
    {
	double beg = 1.0*i/INTERVALS;
	rhistogram_t *h = &hist[i];
	size_t cnt = h->count;
	double prob = cnt ? h->prob/cnt : 0.0;

	/* print interval, count, probability, and spamicity */
	(void)fprintf(fpo, "%s%3.2f %4lu %f %f ", stats_prefix, beg, (unsigned long)cnt, prob, h->spamicity );

	/* scale histogram to 48 characters */
	if (maxcnt>48) cnt = (cnt * 48 + maxcnt - 1) / maxcnt;

	/* display histogram */
	while (cnt-- > 0)
	    (void)fputc( '#', fpo);
	(void)fputc( '\n', fpo);
    }
}

static void rstats_print_rtable(rstats_t *rstats_head)
{
    const char *pfx = !stats_in_header ? "" : "  ";

    rstats_t *cur;

    /* print header */
    if (!Rtable)
	(void)fprintf(fpo, "%s%*s %6s    %-6s    %-6s    %-6s %s\n",
		      pfx, max_token_len+2, "", "n", "pgood", "pbad", "fw", "U");
    else
	(void)fprintf(fpo, "%s%*s %6s    %-6s    %-6s    %-6s  %-6s    %-6s %s\n",
		      pfx, max_token_len+2, "", "n", "pgood", "pbad", "fw", "invfwlog", "fwlog", "U");

    /* Print 1 line per token */
    for (cur=rstats_head->next; cur != NULL; cur=cur->next)
    {
	int len = (cur->token->leng >= max_token_len) ? 0 : (max_token_len - cur->token->leng);
	double fw = calc_prob(cur->good, cur->bad, cur->msgs_good, cur->msgs_bad);
	char flag = cur->used ? '+' : '-';

	(void)fprintf(fpo, "%s\"", pfx);
	(void)word_puts(cur->token, 0, fpo);

	if (cur->msgs_good == 0 && cur->msgs_bad == 0)
	{
	    flag = 'i';
	    (void)fprintf(fpo, "\"%*s %6lu  %8s  %8s  %8.6f",
			  len, " ", (unsigned long)(cur->good + cur->bad),
			  "--------", "--------",
			  fw);
	}
	else
	    (void)fprintf(fpo, "\"%*s %6lu  %8.6f  %8.6f  %8.6f",
			  len, " ", (unsigned long)(cur->good + cur->bad),
			  (double)cur->good / cur->msgs_good,
			  (double)cur->bad  / cur->msgs_bad,
			  fw);

	if (Rtable)
	    (void)fprintf(fpo, "%s%10.5f%10.5f",
			  pfx, log(1.0 - fw), log(fw));
	(void)fprintf(fpo, " %c\n", flag);
    }

    /* print trailer */
    msg_print_summary(pfx);
}
