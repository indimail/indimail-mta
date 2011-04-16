/* $Id: bogofilter.c 6789 2009-02-12 01:12:53Z relson $ */

/*****************************************************************************

NAME:
   bogofilter.c -- detect spam and bogons presented on standard input.

AUTHORS:
   Eric S. Raymond <esr@thyrsus.com>
   David Relson    <relson@osagesoftware.com>
   Matthias Andree <matthias.andree@gmx.de>
   Greg Louis      <glouis@dynamicro.on.ca>

THEORY:

   Originally implemented as Paul Graham's variant of Bayes filtering,
   as described in 

     "A Plan For Spam", http://www.paulgraham.com/spam.html

   Updated in accordance with Gary Robinson's proposed modifications,
   as described at

    http://radio.weblogs.com/0101454/stories/2002/09/16/spamDetection.html

******************************************************************************/

#include "common.h"

#include <string.h>
#include <stdlib.h>

#include "bogofilter.h"
#include "bogoconfig.h"
#include "bogoreader.h"
#include "collect.h"
#include "format.h"
#include "passthrough.h"
#include "register.h"
#include "rstats.h"
#include "score.h"
#ifdef INDIMAIL
#include <unistd.h>
#include <errno.h>
#include "format.h"
#endif

/*
**	case B_NORMAL:		
**	case B_STDIN:		* '-b' - streaming (stdin) mode *
**	case B_CMDLINE:		* '-B' - command line mode *
**
**loop:
**    read & parse a message
**	if -p, save textblocks
**    register if -snSN && -pe
**    classify if -pue && ! -snSN
**    register if -u
**    write    if -p
**    if (-snSN && -pe) || -u
**	free tokens
**    else
**	accumulate tokens	
**
**end:	register if -snSN && ! -pe
*/

/* Function Definitions */

void print_stats(FILE *fp)
{
    msg_print_stats(fp);
}

#ifdef INDIMAIL
#if 0
extern float    unknown_count, total_count;
#endif
extern char     msg_bogofilter[];
extern char     msg_register[];

void            write_fifolog(rc_t);

void
write_fifolog(rc_t status)
{
	char           *fifo_name, *ptr;
	FILE           *logfp;
	pid_t           pid;
	int             logfifo, spamfd = 255;

	fifo_name = getenv("LOGFILTER");
	if (!fifo_name || !*fifo_name)
		return;
	if (*fifo_name != '/')
		return;
	format_log_header(msg_bogofilter, 256);
#if 0
	sprintf(msg_bogofilter + strlen(msg_bogofilter), ", ratio %0.2f", unknown_count / total_count);
#endif
	/*- write to qmail-smtpd fifo */
	if ((ptr = getenv("SPAMFD")))
		spamfd = atoi(ptr);
	write(spamfd, msg_bogofilter, strlen(msg_bogofilter));
	if (logflag)
		return;
	if ((logfifo = open(fifo_name, O_NDELAY | O_WRONLY)) == -1)
		return;
	if (!(logfp = fdopen(logfifo, "w")))
		return;
	pid = getpid();
	format_log_header(msg_bogofilter, 256);
	switch (run_type)
	{
	case RUN_NORMAL:
		fprintf(logfp, "bogofilter: pid %d, %s\n", pid, msg_bogofilter);
		break;
	case RUN_UPDATE:
		if (status == RC_UNSURE || msg_register[0] == '\0')
			fprintf(logfp, "bogofilter: pid %d, %s\n", pid, msg_bogofilter);
		else
		{
			fprintf(logfp, "bogofilter: pid %d, %s, %s\n", pid, msg_bogofilter, msg_register);
			msg_register[0] = '\0';
		}
		break;
	default:
		fprintf(logfp, "bogofilter: pid %d, %s\n", pid, msg_register);
		msg_register[0] = '\0';
	}
	fclose(logfp);
}
#endif

rc_t bogofilter(int argc, char **argv)
{
    uint msgcount = 0;
    rc_t status = RC_OK;
    bool register_opt = (run_type & (REG_SPAM | UNREG_SPAM | REG_GOOD | UNREG_GOOD)) != 0;
    bool register_bef = register_opt && passthrough;
    bool register_aft = ((register_opt && !passthrough) || (run_type & RUN_UPDATE)) != 0;
    bool write_msg    = passthrough || Rtable;
    bool classify_msg = write_msg || ((run_type & (RUN_NORMAL | RUN_UPDATE))) != 0;

    wordhash_t *words;

    score_initialize();			/* initialize constants */

    if (query)
	return query_config();

    words = register_aft ? wordhash_new() : NULL;

    bogoreader_init(argc, (const char * const *) argv);

    while ((*reader_more)()) {
	double spamicity;
	wordhash_t *w = wordhash_new();

	rstats_init();
	passthrough_setup();

	collect_words(w);
	wordhash_sort(w);
	msgcount += 1;

	format_set_counts(w->count, msgcount);

        if (!passthrough_keepopen())
            bogoreader_close_ifeof();
        
	if (register_opt && DEBUG_REGISTER(1))
	    fprintf(dbgout, "Message #%ld\n", (long) msgcount);
	if (register_bef)
	    register_words(run_type, w, 1);
	if (register_aft)
	    wordhash_add(words, w, &wordprop_init);

	if (classify_msg || write_msg) {
	    lookup_words(w);			/* This reads the database */
	    spamicity = msg_compute_spamicity(w);
	    status = msg_status();
	    if (run_type & RUN_UPDATE)		/* Note: don't register if RC_UNSURE */
	    {
		if (status == RC_SPAM && spamicity <= 1.0 - thresh_update)
		    register_words(REG_SPAM, w, msgcount);
		if (status == RC_HAM && spamicity >= thresh_update)
		    register_words(REG_GOOD, w, msgcount);
	    }

	    if (verbose && !passthrough && !quiet) {
		const char *filename = (*reader_filename)();
		if (filename)
		    fprintf(fpo, "%s ", filename); 
	    }

	    write_message(status);		/* passthrough */
	    if (logflag && !register_opt) {
		write_log_message(status);
		msgcount = 0;
	    }
	}
	wordhash_free(w);

	passthrough_cleanup();
	rstats_cleanup();

	if (DEBUG_MEMORY(2))
	    MEMDISPLAY;

	if (fDie)
	    exit(EX_ERROR);
    }

    bogoreader_fini();

    if (DEBUG_MEMORY(1))
	MEMDISPLAY;

    if (register_aft && ((run_type & RUN_UPDATE) == 0)) {
	wordhash_sort(words);
	register_words(run_type, words, msgcount);
    }

    score_cleanup();

    if (logflag && register_opt)
	write_log_message(status);
#ifdef INDIMAIL
	write_fifolog(status);
#endif

    wordhash_free(words);

    if (DEBUG_MEMORY(1))
	MEMDISPLAY;

    return status;
}

/* Done */
