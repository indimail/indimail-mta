/* $Id: score.h 6777 2009-02-01 03:49:48Z relson $ */
/*  constants and declarations for robinson-fisher method */

#ifndef	FISHER_H
#define	FISHER_H

#include "wordhash.h"

#define MAX_REPEATS	1	/* cap on word frequency per message */
#define GOOD_BIAS	1.0	/* don't give good words more weight */

extern	void	lookup_words(wordhash_t *wh);
extern	void	score_initialize(void);
extern	void	score_cleanup(void);

extern	double	msg_compute_spamicity(wordhash_t *wordhash) /*@globals errno@*/;
extern	double	msg_spamicity(void);
extern	rc_t	msg_status(void);
extern	void	msg_print_stats(FILE *fp);
extern	void	msg_print_summary(const char *pfx);

extern	void	print_summary(void);

#endif
