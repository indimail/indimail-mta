/* $Id: passthrough.c 6926 2010-07-06 21:52:54Z m-a $ */

/*****************************************************************************

NAME:
   passthrough.c -- handle writing out message in passthrough ('-p') mode.

******************************************************************************/

#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#include "passthrough.h"
#include "bogofilter.h"
#include "bogoreader.h"		/* for is_eol() */
#include "fgetsl.h"
#include "format.h"
#include "textblock.h"
#include "xmalloc.h"

#include "lexer.h" /* need have_body */

static const char *eol;
char msg_register[256];
#ifdef INDIMAIL
char            msg_bogofilter[256];
char            msg_spam_header[256];
#else
static char     msg_bogofilter[256];
static char     msg_spam_header[256];
#endif
size_t msg_register_size = sizeof(msg_register);

/* Function Definitions */

static void cleanup_exit(ex_t exitcode, int killfiles)
    __attribute__ ((noreturn));
static void cleanup_exit(ex_t exitcode, int killfiles) {
    if (fpo) {
	output_cleanup();
    }
    if (killfiles && outfname[0] != '\0') unlink(outfname);
    exit(exitcode);
}

#define	ISSPACE(ch)	(isspace(ch) || (ch == '\b'))

/* check for non-empty blank line */
static bool is_blank_line(const char *line, size_t len)
{
    if (have_body)
	return len == 0;

    while (len-- > 0) {
	byte ch = *line++;
	if (!ISSPACE(ch))
	    return false;
    }

    return true;
}

/** check if the line we're looking at is a header/body delimiter */
static bool is_hb_delim(const char *line, size_t len, bool strict_body)
{
    if (strict_body)
	return len == 0;

    return is_blank_line(line, len);
}

static int read_mem(char **out, void *in) {
    textdata_t **text = in;
    if ((*text)->next) {
	int s = (*text)->size;
	*out = (char *)(*text)->data;
	*text = (*text)->next;
	return s;
    }
    return 0;
}

typedef int (*readfunc_t)(char **, void *);

static void write_spam_info(void)
{
    if (eol == NULL)	/* special treatment of empty input */
	eol = NL;

    /* print spam-status at the end of the header
     * then mark the beginning of the message body */
    if (passthrough || verbose || terse)
	{
#ifdef INDIMAIL
		if (*msg_register)
			fprintf(fpo, "%s, %s%s", msg_spam_header, msg_register, eol);
		else
			fprintf(fpo, "%s%s", msg_spam_header, eol);
#else
		fprintf(fpo, "%s%s", msg_spam_header, eol);
#endif
	}

    if (passthrough || verbose || Rtable) {
	verbose += passthrough;
	print_stats( fpo );
	verbose -= passthrough;
    }
}

static bool write_header(rc_t status, readfunc_t rf, void *rfarg)
{
    ssize_t rd;
#ifdef INDIMAIL
	char           *out, *ptr;
#else
	char           *out;
#endif

    bool hadlf = true;
    bool seen_subj = false;

    bool seen_place_header = false;

    int bogolen = strlen(spam_header_name);
    const char *subjstr = "Subject:";
    int subjlen = strlen(subjstr);

    eol = NULL;

    /* print headers */
    while ((rd = rf(&out, rfarg)) > 0)
    {
	if (eol == NULL) {
	    if (memcmp(out+rd-1, NL, 1) == 0)
		eol = NL;
	    if (rd >=2 && memcmp(out+rd-2, CRLF, 2) == 0)
		eol = CRLF;
	}

	/* skip over spam_header_name ("X-Bogosity:") lines */
	while (rd >= bogolen && 
	       memcmp(out, spam_header_name, bogolen) == 0) {
	    while (((rd = rf(&out, rfarg)) > 0) && 
		   (out[0] == ' ' || out[0] == '\t') )
		/* empty loop */ ;
	}

	/* detect spam_header_place line */
	if ( !seen_place_header &&
	     *spam_header_place != '\0' &&
	    memcmp(out, spam_header_place, strlen(spam_header_place)) == 0)
	{
	    seen_place_header = true;
	    write_spam_info();
	}

	/* detect end of headers */
	if (is_eol(out, rd) ||
	    is_hb_delim(out, rd, have_body))
	    /* check for non-empty blank line */
	    break;

	/* rewrite "Subject: " line */
	if (!seen_subj && rd >= subjlen) {
	    const char *tag = NULL;

	    if (status == RC_SPAM && spam_subject_tag != NULL)
		tag = spam_subject_tag;

	    if (status == RC_UNSURE && unsure_subject_tag != NULL)
		tag = unsure_subject_tag;

	    if (tag != NULL && strncasecmp(out, subjstr, subjlen) == 0) {
		seen_subj = true;
#ifdef INDIMAIL
		for (ptr = out + subjlen; *ptr && isspace((int) *ptr); ptr++);
		if (strstr(ptr, tag))
			(void) fwrite(out, 1, rd, fpo);	/*- do not tag twice */
		else		 /*- adding tag will break DomainKeys */
		{
			(void) fprintf(fpo, "%.*s %s", subjlen, out, tag);
			if (out[subjlen] != ' ')
				fputc(' ', fpo);
			(void) fwrite(out + subjlen, 1, rd - subjlen, fpo);
		}
#else
		(void) fprintf(fpo, "%.*s %s", subjlen, out, tag);
		if (out[subjlen] != ' ')
		    fputc(' ', fpo);
		(void) fwrite(out + subjlen, 1, rd - subjlen, fpo);
#endif
		continue;
	    }
	}

	hadlf = (out[rd-1] == '\n');
	(void) fwrite(out, 1, rd, fpo);
	if (ferror(fpo)) cleanup_exit(EX_ERROR, 1);
    }

    if (!seen_place_header)
	write_spam_info();

    if (!hadlf)
	(void) fputs(eol, fpo);

    return seen_subj;
}

static void write_body(readfunc_t rf, void *rfarg)
{
    ssize_t rd;
    char   *out;

    int hadlf = 1;
    /* If the message terminated early (without body or blank
     * line between header and body), enforce a blank line to
     * prevent anything past us from choking. */
    (void) fputs(eol, fpo);

    /* print body */
    while ((rd = rf(&out, rfarg)) > 0)
    {
	(void) fwrite(out, 1, rd, fpo);
	hadlf = (out[rd-1] == '\n');
	if (ferror(fpo)) cleanup_exit(EX_ERROR, 1);
    }

    if (!hadlf) (void) fputs(eol, fpo);
}

static void build_spam_header(void)
{
    if (passthrough || verbose || terse) {
	typedef char *formatter(char *buff, size_t size);
	formatter *fcn = terse ? format_terse : format_header;
	(*fcn)(msg_spam_header, sizeof(msg_spam_header));
    }
}

void write_message(rc_t status)
{
    readfunc_t rf = NULL;	/* assignment to quench warning */
    void *rfarg = 0;		/* assignment to quench warning */
    textdata_t *text;
    bool seen_subj = false;

    build_spam_header();

    if (!passthrough)
    {
	write_spam_info();
    }
    else
    {
	eol = NULL;
	/* initialize */
	rf = read_mem;
	text = textblock_head();
	rfarg = &text;

	seen_subj = write_header(status, rf, rfarg);

	if (!seen_subj) {
	    if (status == RC_SPAM && spam_subject_tag != NULL)
		(void) fprintf(fpo, "Subject: %s%s", spam_subject_tag, eol);
	    if (status == RC_UNSURE && unsure_subject_tag != NULL)
		(void) fprintf(fpo, "Subject: %s%s", unsure_subject_tag, eol);
	}

	write_body(rf, rfarg);

	if (verbose || Rtable) {
	    if (fflush(fpo) || ferror(fpo))
		cleanup_exit(EX_ERROR, 1);
	}
    }

    return;
}

void write_log_message(rc_t status)
{
#ifdef HAVE_SYSLOG_H
    format_log_header(msg_bogofilter, sizeof(msg_bogofilter));

    switch (run_type) {
    case RUN_NORMAL:
	syslog(LOG_INFO, "%s\n", msg_bogofilter);
	break;
    case RUN_UPDATE:
	if (status == RC_UNSURE || msg_register[0] == '\0')
	    syslog(LOG_INFO, "%s\n", msg_bogofilter);
	else {
	    syslog(LOG_INFO, "%s, %s\n", msg_bogofilter, msg_register);
	    msg_register[0] = '\0';
	}
	break;
    default:
	syslog(LOG_INFO, "%s", msg_register);
	msg_register[0] = '\0';
    }

    closelog();
#endif
}

/* we need to take care not to fclose() a shared file descriptor, if,
 * for instance we've fdopen()ed STDOUT_FILENO, becase we'd prevent
 * other streams accessing the same file descriptor from flushing their
 * caches. Prominent symptom: t.bogodir failure.
 */
static bool fpo_mayclose;	/* if output_cleanup can close the file */

void output_setup(void)
{
    assert(fpo == NULL);

    if (*outfname && passthrough) {
	fpo = fopen(outfname,"w");
	fpo_mayclose = true;
    } else {
	fpo = fdopen(STDOUT_FILENO, "w");
	fpo_mayclose = false;
    }

    if (fpo == NULL) {
	if (*outfname)
	    fprintf(stderr, "Cannot open %s: %s\n",
		    outfname, strerror(errno));
	else
	    fprintf(stderr, "Cannot fdopen STDOUT: %s\n", strerror(errno));

	exit(EX_ERROR);
    }

    /* if we're not in passthrough mode, set line buffered mode just in
     * case some program that calls uses waits for our output in -T mode */
    if (!passthrough) {
	setvbuf(fpo, NULL, _IOLBF, BUFSIZ);
    }
}

void output_cleanup(void) {
    int rc;

    rc = fflush(fpo);
    if (fpo_mayclose)
	rc |= fclose(fpo);
    fpo = NULL;
    if (rc)
	cleanup_exit(EX_ERROR, 1);
}

void passthrough_setup(void)
{
    if (!passthrough)
	return;

    textblock_init();
}

int passthrough_keepopen(void)
{
    return false;
}

void passthrough_cleanup(void)
{
    if (!passthrough)
	return;

    textblock_free();
}

/* End */
