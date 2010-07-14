/* $Id: bogohist.c 6766 2009-01-12 04:27:36Z relson $ */

/*****************************************************************************

NAME:
   bogohist.c -- print bogofilter histogram

AUTHOR:
   Gyepi Sam <gyepi@praxis-sw.com>

******************************************************************************/

#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#include "bogohist.h"
#include "prob.h"
#include "datastore.h"
#include "msgcounts.h"
#include "word.h"
#include "wordlists.h"
#include "xmalloc.h"

static uint ham_only,  ham_hapax;
static uint spam_only, spam_hapax;

static uint mgood, mbad;

#define	INTERVALS	20
#define PCT(n)		100.0 * n / count

typedef struct rhistogram_s rhistogram_t;
struct rhistogram_s {
    uint32_t count[INTERVALS];
};

/* Function Prototypes */

/* Function Definitions */

static int ds_histogram_hook(/*@unused@*/ word_t *key, dsv_t *data,
			     void *userdata)
/* returns 0 if ok, 1 if not ok */
{
    rhistogram_t *hist = userdata;

    double fw = calc_prob(data->goodcount, data->spamcount, mgood, mbad);
    uint idx = min(fw * INTERVALS, INTERVALS-1);

    /* ignore meta-tokens */
    if (*key->u.text == (byte) '.')
	return 0;

    hist->count[idx] += 1;

    if (data->spamcount == 0) {
	ham_only += 1;
	if (data->goodcount == 1)
	    ham_hapax += 1;
    }

    if (data->goodcount == 0) {
	spam_only += 1;
	if (data->spamcount == 1)
	    spam_hapax += 1;
    }

    return 0;
}

static int print_histogram(rhistogram_t *hist)
{
    uint i, r;
    uint maxcnt = 0;
    uint count = 0;

    if (verbose == 0)
	(void)printf("Histogram\n");

    if (verbose == 1) {
	hist->count[0]           -= ham_hapax;
	hist->count[INTERVALS-1] -= spam_hapax;
	(void)printf("Histogram without hapaxes\n");
    }

    if (verbose == 2) {
	hist->count[0]           -= ham_only;
	hist->count[INTERVALS-1] -= spam_only;
	(void)printf("Histogram without pure ham and spam\n");
    }

    (void)printf("%5s%8s  %3s  %s\n", "score", "count", "pct", "histogram");

    for (i=0; i<INTERVALS; i+=1)
    {
	uint32_t cnt = hist->count[i];
	if (cnt > maxcnt) 
	    maxcnt = cnt;
	count += cnt;
    }

    /* Print histogram */
    for (i=0; i<INTERVALS; i+=1)
    {
	uint32_t cnt = hist->count[i];
	double beg = 1.0 * i / INTERVALS;
	double pct = PCT(cnt);

	/* print interval, count, probability, percent, and spamicity */
	(void)printf("%3.2f %8u %5.2f ", beg, cnt, pct);

	/* scale histogram to 48 characters */
	if (maxcnt>48) cnt = (cnt * 48 + maxcnt - 1) / maxcnt;

	/* display histogram */
	for (r=0; r<cnt; r+=1)
	    (void)fputc( '#', stdout);
	(void)fputc( '\n', stdout);
    }

    (void)printf("tot  %8u\n", count);

    return count;
}

ex_t histogram(bfpath *bfp)
{
    ex_t rc;
    uint count;
    void *dsh, *dbe;
    dsv_t val;

    rhistogram_t hist;

    dbe = ds_init(bfp);
    if (dbe == NULL)
	return EX_ERROR;

    dsh = ds_open(dbe, bfp, DS_READ);
    if (dsh == NULL)
	return EX_ERROR;

    if (DST_OK != ds_txn_begin(dsh)) {
	ds_close(dsh);
	ds_cleanup(dbe);
	fprintf(stderr, "cannot begin transaction!\n");
	return EX_ERROR;
    }

    ds_get_msgcounts(dsh, &val);
    mgood = val.goodcount;
    mbad = val.spamcount;

    memset(&hist, 0, sizeof(hist));
    rc = ds_foreach(dsh, ds_histogram_hook, &hist);

    if (DST_OK != ds_txn_commit(dsh)) {
	ds_close(dsh);
	ds_cleanup(dbe);
	fprintf(stderr, "cannot commit transaction!\n");
	return EX_ERROR;
    }

    ds_close(dsh);
    ds_cleanup(dbe);

    count = print_histogram(&hist);

    if (verbose > 0) {
	printf("hapaxes:  ham %7u, spam %7u\n", ham_hapax, spam_hapax);
	printf("   pure:  ham %7u, spam %7u\n", ham_only,  spam_only);
    }
    else {
	printf("hapaxes:  ham %7u (%5.2f%%), spam %7u (%5.2f%%)\n", ham_hapax, PCT(ham_hapax), spam_hapax, PCT(spam_hapax));
	printf("   pure:  ham %7u (%5.2f%%), spam %7u (%5.2f%%)\n", ham_only,  PCT(ham_only),  spam_only,  PCT(spam_only));
    }

    return rc;
}

/* for a standalone program:
**
**	cc -o bogohist.prog.o -DMAIN -c bogohist.c
**	cc -o bogohist bogohist.prog.o libbogofilter.a strlcpy.o strlcat.o -ldb  -lm
*/

#ifdef	MAIN
const char *progname = "bogohist";

int main(int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "usage: %s BOGOFILTER_DIR\n", progname);
	exit(1);
    }
    else {
	const char *path = argv[1];
	int rc = histogram(path);
	exit(rc);
    }
}
#endif
