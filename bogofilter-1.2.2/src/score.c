/* $Id: score.c 6890 2010-03-17 23:52:48Z m-a $ */

/*****************************************************************************

NAME:
   score.c -- implements Fisher variant on Robinson algorithm.

******************************************************************************/

#include "common.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "bogoconfig.h"
#include "bogofilter.h"
#include "collect.h"
#include "datastore.h"
#include "listsort.h"
#include "msgcounts.h"
#include "prob.h"
#include "rand_sleep.h"
#include "rstats.h"
#include "score.h"
#include "wordhash.h"
#include "wordlists.h"

#if defined(HAVE_GSL_10) && !defined(HAVE_GSL_14)
/* HAVE_GSL_14 implies HAVE_GSL_10
 * if we have neither, we'll use our included GSL 1.4, which knows CDFs
 * if we have both, we have GSL 1.4, which knows CDFs
 *
 * in other cases, we need to integrate the PDF to get the CDF
 */
#define GSL_INTEGRATE_PDF
#include "gsl/gsl_randist.h"
#include "gsl/gsl_integration.h"
#include "gsl/gsl_errno.h"
#else
#include "gsl/gsl_cdf.h"
#endif

/* Structure Definitions */

typedef struct probnode_t {
    hashnode_t * node;
    double	 prob;
} probnode_t;

/* struct for saving stats for printing. */
typedef struct score_s {
    double min_dev;
    double spamicity;
    u_int32_t robn;
    double p_ln;	/* Robinson P, as a log*/
    double q_ln;	/* Robinson Q, as a log*/
    double p_pr;	/* Robinson P */
    double q_pr;	/* Robinson Q */
} score_t;

/* Function Prototypes */

static	double	get_spamicity(size_t robn, FLOAT P, FLOAT Q);
static	bool	need_scoring_boundary(size_t count);
static	double	find_scoring_boundary(wordhash_t *wh);
static	size_t	compute_count_and_scores(wordhash_t *wh);
static	size_t	compute_count_and_spamicity(wordhash_t *wh, FLOAT *P, FLOAT *Q, bool need_stats);
static	int	compare_hashnode_t(const void *const pv1, const void *const pv2);

/* Static Variables */

static score_t	score;

/* Function Definitions */

double msg_spamicity(void)
{
    return score.spamicity;
}

rc_t msg_status(void)
{
    if (score.spamicity >= spam_cutoff)
	return RC_SPAM;

    if ((ham_cutoff < EPS) ||
	(score.spamicity <= ham_cutoff))
	return RC_HAM;

    return RC_UNSURE;
}

void msg_print_stats(FILE *fp)
{
    bool unsure = unsure_stats && (msg_status() == RC_UNSURE) && verbose;

    (void)fp;

    if (quiet)
	return;

    if (Rtable || unsure || verbose >= 2)
	rstats_print(unsure);
}

/** search token in all lists according to precedence, summing up the
 * counts (all lists at same precedence are used); if found, set cnts
 * accordingly. */
static int lookup(const word_t *token, wordcnts_t *cnts)
{
    int override=0;
    wordlist_t* list;

    if (fBogotune) {
	wordprop_t *wp = wordhash_search_memory(token);
	if (wp) {
	    cnts->good = wp->cnts.good;
	    cnts->bad  = wp->cnts.bad;
	}
	return 0;
    }

    cnts->msgs_bad = cnts->msgs_good = 0;

    for (list=word_lists; list != NULL; list=list->next)
    {
	dsv_t val;
	int ret;

	if (override > list->override)	/* if already found */
	    break;

	ret = ds_read(list->dsh, token, &val);

	/* check if we have the token */
	switch (ret) {
	    case 0:
		/* token found, pass on */
		break;
	    case 1:
		/* token not found, clear counts */
		val.count[IX_GOOD] = 0;
		val.count[IX_SPAM] = 0;
		break;
	    case DS_ABORT_RETRY:
		/* sleep, reinitialize and start over */
		rand_sleep(1000,1000000);
		begin_wordlist(list);
		/* FALLTHROUGH */
	    default:
		return ret;
	}

	if (ret == 0 && list->type == WL_IGNORE) {	/* if found on ignore list */
	    cnts->good = cnts->bad = 0;
	    break;
	}

	override=list->override;

	if (DEBUG_ALGORITHM(2)) {
	    fprintf(dbgout, "%6d %5u %5u %5u %5u list=%s,%c,%d ",
		    ret, (uint)val.count[IX_GOOD], (uint)val.count[IX_SPAM],
		    (uint)list->msgcount[IX_GOOD], (uint)list->msgcount[IX_SPAM],
		    list->listname, list->type, list->override);
	    word_puts(token, 0, dbgout);
	    fputc('\n', dbgout);
	}

	cnts->good += val.count[IX_GOOD];
	cnts->bad += val.count[IX_SPAM];
	cnts->msgs_good += list->msgcount[IX_GOOD];
	cnts->msgs_bad += list->msgcount[IX_SPAM];
    }

    if (DEBUG_ALGORITHM(1)) {
	fprintf(dbgout, "%5u %5u ", (uint)cnts->bad, (uint)cnts->good);
	word_puts(token, 0, dbgout);
	fputc('\n', dbgout);
    }

    return 0;
}


/* do wordlist lookups for the words in the wordhash
 */
void lookup_words(wordhash_t *wh)
{
    int ret;
    hashnode_t *node;

    if (msg_count_file)	/* if mc file, already done */
	return;

retry:
    for (node = wordhash_first(wh); node != NULL; node = wordhash_next(wh))
    {
	word_t *token     = node->key;
	wordprop_t *props = (wordprop_t *) node->data;
	wordcnts_t *cnts  = &props->cnts;
	ret = lookup(token, cnts);
	if (ret == DS_ABORT_RETRY)
	    /* start all over, the message counts may have changed
	     * lookup handles reinitializing the wordlist */
	    goto retry;
    }

    return;
}

/** selects the best spam/non-spam indicators and calculates Robinson's S,
 * \return -1.0 for error, S otherwise */
double msg_compute_spamicity(wordhash_t *wh) /*@globals errno@*/
{
    FLOAT P = {1.0, 0};		/* Robinson's P */
    FLOAT Q = {1.0, 0};		/* Robinson's Q */

    double spamicity;
    size_t robn = 0;

    bool need_stats = !fBogotune && (Rtable || passthrough || (verbose > 0));

    if (DEBUG_ALGORITHM(2)) fprintf(dbgout, "### msg_compute_spamicity() begins\n");

    if (DEBUG_ALGORITHM(2)) fprintf(dbgout, "min_dev: %f, robs: %f, robx: %f\n", 
				    min_dev, robs, robx);

    /* compute scores for the wordhash's tokens */
    robn = compute_count_and_scores(wh);

    /* recalculate min_dev if necessary to satisfy token_count settings */
    if (!need_scoring_boundary(robn))
	score.min_dev = min_dev;
    else
	score.min_dev = find_scoring_boundary(wh);

    /* compute message spamicity from the wordhash's scores */
    robn = compute_count_and_spamicity(wh, &P, &Q, need_stats);

    /* Robinson's P, Q and S
    ** S = (P - Q) / (P + Q)			    [combined indicator]
    */
    spamicity = get_spamicity(robn, P, Q);

    if (need_stats && robn != 0)
	rstats_fini(robn, P, Q, spamicity);

    if (DEBUG_ALGORITHM(2)) fprintf(dbgout, "### msg_compute_spamicity() ends\n");

    return spamicity;
}

/*
** compute_count_and_scores()
**	compute the token probabilities from the linked list of tokens
*/
static size_t compute_count_and_scores(wordhash_t *wh)
{
    size_t count = 0;
    hashnode_t *node;

    if (fBogotune)
	return count;

    for (node = wordhash_first(wh); node != NULL; node = wordhash_next(wh))
    {
	wordcnts_t *cnts;
	wordprop_t *props;

	props = (wordprop_t *) node->data;
	cnts  = &props->cnts;
	props->prob = calc_prob(cnts->good, cnts->bad,
				cnts->msgs_good, cnts->msgs_bad);
	props->used = fabs(props->prob - EVEN_ODDS) > min_dev;
	if (props->used)
	    count += 1;
    }

    return count;
}

/*
** compute_count_and_score()
**	compute the spamicity from the linked list of tokens using
**	min_dev to select tokens
*/
static size_t compute_count_and_spamicity(wordhash_t *wh, 
					  FLOAT *P, FLOAT *Q,
					  bool need_stats)
{
    size_t count = 0;

    hashnode_t *node;

    for (node = wordhash_first(wh); node != NULL; node = wordhash_next(wh))
    {
	bool useflag;
	double prob;
	word_t *token;
	wordcnts_t *cnts;
	wordprop_t *props;

	if (!fBogotune) {
	    token = node->key;
	    props = (wordprop_t *) node->data;
	    cnts  = &props->cnts;
	    prob = props->prob;
	    useflag = props->used;
	} else {
	    token = NULL;
	    cnts = (wordcnts_t *) node;
	    prob = calc_prob(cnts->good, cnts->bad,
			     cnts->msgs_good, cnts->msgs_bad);
	    useflag = fabs(prob - EVEN_ODDS) > score.min_dev;
	}

	if (need_stats)
	    rstats_add(token, prob, useflag, cnts);

	/* Robinson's P and Q; accumulation step */
	/*
	 * P = 1 - ((1-p1)*(1-p2)*...*(1-pn))^(1/n)	[spamminess]
	 * Q = 1 - (p1*p2*...*pn)^(1/n)			[non-spamminess]
	 */
	if (useflag ) {
	    int e;

	    P->mant *= 1-prob;
	    if (P->mant < 1.0e-200) {
		P->mant = frexp(P->mant, &e);
		P->exp += e;
	    }

	    Q->mant *= prob;
	    if (Q->mant < 1.0e-200) {
		Q->mant = frexp(Q->mant, &e);
		Q->exp += e;
	    }
	    count += 1;
	}

	if (DEBUG_ALGORITHM(3)) {
	    (void)fprintf(dbgout, "%3lu %3lu %f ",
			  (unsigned long)count, (unsigned long)count, prob);
	    (void)word_puts(token, 0, dbgout);
	    (void)fputc('\n', dbgout);
	}
    }

    return count;
}

/* need_scoring_boundary( )
**	determine if min_dev gives a count fitting the token count limits
**	return True if so; False if not
*/
static bool need_scoring_boundary(size_t count)
{
    /* Early out if no token count limits are set */
    if (((token_count_min == 0) || (token_count_min <= count)) &&
	((token_count_max == 0) || (token_count_max >= count)) &&
	((token_count_fix == 0) || (token_count_fix == count)))
	return false;
    else
	return true;
}

/* find_scoring_boundary( )
**	determine the token score that gives the desired token count
**	for scoring the message.
*/
static double find_scoring_boundary(wordhash_t *wh)
{
    size_t count = 0;

    double min_prob = (token_count_max == 0.0) ? min_dev : 1.0;

    hashnode_t *node;

    /* sort by ascending score difference (from 0.5) */
    wh->iter_head = (hashnode_t *)listsort((element *)wh->iter_head, (fcn_compare *)&compare_hashnode_t);

    count = max(token_count_fix, max(token_count_min, token_count_max));

    for (node = wordhash_first(wh); node != NULL; node = wordhash_next(wh)) {
	wordcnts_t *cnts;
	wordprop_t *props = NULL;
	double prob;
	double dev;

	if (!fBogotune) {
	    props = (wordprop_t *) node->data;
	    cnts  = &props->cnts;
	} else {
	    cnts = (wordcnts_t *) node;
	}
	prob = calc_prob(cnts->good, cnts->bad,
			 cnts->msgs_good, cnts->msgs_bad);
	dev = fabs(prob - EVEN_ODDS);

	if (count > 0) {
	    count -= 1;
	    if (!fBogotune)
		props->used = true;
	    min_prob = dev;
	}
	else if (dev >= min_prob) {
	    if (!fBogotune)
		props->used = true;
	}
	else {
	    if (!fBogotune)
		props->used = false;
	}
    }

    return min_prob;
}

/* compare_hashnode_t - sort by ascending score difference (from 0.5) */

static int compare_hashnode_t(const void *const pv1, const void *const pv2)
{
    double d1;
    double d2;

    if (!fBogotune) {
	const hashnode_t *hn1 = (const hashnode_t *const)pv1;
	const hashnode_t *hn2 = (const hashnode_t *const)pv2;
	d1 = fabs(((wordprop_t *) hn1->data)->prob - EVEN_ODDS);
	d2 = fabs(((wordprop_t *) hn2->data)->prob - EVEN_ODDS);
    } else {
	const wordcnts_t *cnts;
	double prob;
	cnts = (const wordcnts_t *) pv1;
	prob = calc_prob(cnts->good, cnts->bad,
			 cnts->msgs_good, cnts->msgs_bad);
	d1 = fabs(prob - EVEN_ODDS);

	cnts = (const wordcnts_t *) pv2;
	prob = calc_prob(cnts->good, cnts->bad,
			 cnts->msgs_good, cnts->msgs_bad);
	d2 = fabs(prob - EVEN_ODDS);
    }

    if (d1 < d2)
	return +1;
    if (d1 > d2)
	return -1;

    return 0;
}

void score_initialize(void)
{
    word_t *word_robx = word_news(ROBX_W);

    wordlist_t *list = get_default_wordlist(word_lists);

    if (fabs(min_dev) < EPS)
	min_dev = MIN_DEV;
    if (spam_cutoff < EPS)
	spam_cutoff = SPAM_CUTOFF;

    /*
    ** If we're classifying messages, we need to compute the scalefactor 
    ** (from the .MSG_COUNT values)
    ** If we're registering tokens, we needn't get .MSG_COUNT
    */

    if (fabs(robs) < EPS)
	robs = ROBS;

    if (fabs(robx) < EPS)
    {
	/* Assign default value in case there's no wordlist
	 * or no wordlist entry */
	robx = ROBX;
	if (list->dsh != NULL)
	{
	    int ret;
	    dsv_t val;

	    /* Note: .ROBX is scaled by 1000000 in the wordlist */
	    ret = ds_read(list->dsh, word_robx, &val);
	    if (ret != 0)
		robx = ROBX;
	    else {
		/* If found, unscale; else use predefined value */
		uint l_robx = val.count[IX_SPAM];
		robx = l_robx ? (double)l_robx / 1000000 : ROBX;
	    }
	}
    }

    if (robx < 0.0 || 1.0 < robx) {
	fprintf(stderr, "Invalid robx value (%f).  Must be between 0.0 and 1.0\n", robx);
	exit(EX_ERROR);
    }

    word_free(word_robx);

    return;
}

void score_cleanup(void)
{
/*    rstats_cleanup(); */
}

#ifdef GSL_INTEGRATE_PDF
static double chisq(double x, void *p) {
     return(gsl_ran_chisq_pdf(x, *(double *)p));
}

inline static double prbf(double x, double df) {
    gsl_function chi;
    int status;
    double p, abserr;
    const int intervals = 15;
    const double eps = 1000 * DBL_EPSILON;

    gsl_integration_workspace *w;
    chi.function = chisq;
    chi.params = &df;
    gsl_set_error_handler_off();
    w = gsl_integration_workspace_alloc(intervals);
    if (!w) {
	fprintf(stderr, "Out of memory! %s:%d\n", __FILE__, __LINE__);
	exit(EX_ERROR);
    }
    status = gsl_integration_qag(&chi, 0, x, eps, eps,
	    intervals, GSL_INTEG_GAUSS41, w, &p, &abserr);
    if (status && status != GSL_EMAXITER) {
	fprintf(stderr, "Integration error: %s\n", gsl_strerror(status));
	exit(EX_ERROR);
    }
    gsl_integration_workspace_free(w);
    p = max(0.0, 1.0 - p);
    return(min(1.0, p));
}
#else
inline static double prbf(double x, double df)
{
    double r = gsl_cdf_chisq_Q(x, df);
    return (r < DBL_EPSILON) ? 0.0 : r;
}
#endif

static double get_spamicity(size_t robn, FLOAT P, FLOAT Q)
{
    if (robn == 0)
    {
	score.spamicity = robx;
    }
    else
    {
	double sp_df = 2.0 * robn * sp_esf;
	double ns_df = 2.0 * robn * ns_esf;
	double ln2 = log(2.0);					/* ln(2) */

	score.robn = robn;

	/* convert to natural logs */
	score.p_ln = (log(P.mant) + P.exp * ln2) * sp_esf;	/* invlogsum */
	score.q_ln = (log(Q.mant) + Q.exp * ln2) * ns_esf;	/* logsum */

	score.p_pr = prbf(-2.0 * score.p_ln, sp_df);		/* compute P */
	score.q_pr = prbf(-2.0 * score.q_ln, ns_df);		/* compute Q */
  
	if (!fBogotune && sp_esf >= 1.0 && ns_esf >= 1.0) {
	    score.spamicity = (1.0 + score.q_pr - score.p_pr) / 2.0;
	} else if (score.q_pr < DBL_EPSILON && score.p_pr < DBL_EPSILON) {
	    score.spamicity = 0.5;
	} else {
	    score.spamicity = score.q_pr / (score.q_pr + score.p_pr);
	}
    }

    return score.spamicity;
}

void msg_print_summary(const char *pfx)
{
    if (!Rtable) {
	(void)fprintf(fpo, "%s%-*s %6lu %9.6f %9.6f %9.6f\n",
		      pfx, max_token_len+2, "N_P_Q_S_s_x_md", (unsigned long)score.robn, 
		      score.p_pr, score.q_pr, score.spamicity);
	(void)fprintf(fpo, "%s%-*s  %9.6f %9.6f %9.6f\n",
		      pfx, max_token_len+2+6, " ", robs, robx, score.min_dev);
    }
    else {
	/* Trim token to 22 characters to accomodate R's default line length of 80 */
	(void)fprintf(fpo, "%s%-24s %6lu %9.2e %9.2e %9.2e %9.2e %9.2e %5.3f\n",
		      pfx, "N_P_Q_S_s_x_md", (unsigned long)score.robn,
		      score.p_pr, score.q_pr, score.spamicity, robs, robx, score.min_dev);
    }
}

/* Done */
