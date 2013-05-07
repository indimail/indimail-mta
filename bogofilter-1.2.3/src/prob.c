/* $Id: prob.c 6733 2008-06-11 11:15:34Z relson $ */

/*****************************************************************************

NAME:
   prob.c -- calculate token's spamicity

AUTHORS:
   David Relson <relson@osagesoftware.com>
   Matthias Andree <matthias.andree@gmx.de>

******************************************************************************/

#include "globals.h"
#include "prob.h"

double calc_prob(uint good, uint bad, uint goodmsgs, uint badmsgs)
{
    uint n = good + bad;
    double fw, pw;

    /* http://www.linuxjournal.com/article.php?sid=6467 */

    /* robs is Robinson's s parameter, the "strength of background info" */
    /* robx is Robinson's x parameter, the assumed probability that
     * a word we don't have enough info about will be spam */
    /* n is the number of messages that contain the word w */

    if (n == 0
#ifdef EXTRA_DOMAIN_CHECKING
	    /* we had this in place while the ignore lists caused the
	     * token to have "nan" counts because score.c left the
	     * message counts at zero - #ifdef'd out for speed */
	    || badmsgs == 0 || goodmsgs == 0
#endif
	    ) {
	/* in these cases, pw would be undefined and return NaN
	 * we substitute "we don't know", the x parameter */
	fw = robx;
    } else {
	/* The original version of this code has four divisions.
	pw = ((bad / badmsgs) / (bad / badmsgs + good / goodmsgs));
	*/

	/* This modified version, with 1 division, is considerably% faster. */
	pw =   bad * (double)goodmsgs
	    / (bad * (double)goodmsgs + good * (double)badmsgs);

	fw = (robs * robx + n * pw) / (robs + n);
    }

    return fw;
}
