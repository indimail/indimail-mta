#ifndef	maildirfilter_h
#define	maildirfilter_h

/*
** Copyright 2000-2004 Double Precision, Inc.
** See COPYING for distribution information.
*/


#include	"config.h"

#ifdef __cplusplus
extern "C" {
#endif

enum maildirfiltertype {
	startswith,
	endswith,
	contains,
	hasrecipient,
	mimemultipart,
	textplain,
	islargerthan,		/* Use negation for the opposite! */
	anymessage
	} ;

struct maildirfilterrule {
	struct maildirfilterrule *next, *prev;
	char *rulename_utf8;
	enum maildirfiltertype type;
	int flags;

#define	MFR_DOESNOT	1	/* Negates pretty much every condition */
#define	MFR_BODY	2	/* startswith/endswith/contains applied
				** to body.
				*/
#define	MFR_CONTINUE	4	/* Continue filtering (cc instead of to) */
#define MFR_PLAINSTRING	8	/* Pattern is a plain string, not a regex */

	char *fieldname_utf8;	/* Match this header */
	char *fieldvalue_utf8;	/* Match/search value */
	char *tofolder;		/* Destination folder, fwd address, err msg */
	char *fromhdr;		/* From: header on autoreplies. */
	} ;

struct maildirfilter {
	struct maildirfilterrule *first, *last;
	} ;

/****************************************************************************/
/*             Low-level filter access functions                            */
/****************************************************************************/

/*
** A maildirfilter structure is initialized simply by nulling it out, then: */

struct maildirfilterrule *maildir_filter_appendrule(struct maildirfilter *r,
					const char *name,
					enum maildirfiltertype type,
					int flags,
					const char *header,
					const char *value,
					const char *folder,
					const char *fromhdr,
					const char *rulecharset,
					int *errcode);	/* Append a new rule */

int maildir_filter_setautoreplyfrom(struct maildirfilter *, const char *);

/* Move a given rule up or down in the hierarchy */

void maildir_filter_ruleup(struct maildirfilter *, struct maildirfilterrule *);
void maildir_filter_ruledown(struct maildirfilter *, struct maildirfilterrule *);

/* Delete a given rule */

void maildir_filter_ruledel(struct maildirfilter *, struct maildirfilterrule *);

/* Update an existing rule */

int maildir_filter_ruleupdate(struct maildirfilter *, struct maildirfilterrule *,
		  const char *,
		  enum maildirfiltertype,
		  int,
		  const char *,
		  const char *,
		  const char *,
		  const char *,
		  const char *,
		  int *);

#define	maildir_filter_freerules(r)	do { \
	while ( (r)->first )	\
		maildir_filter_ruledel( (r), (r)->first );	\
	} while (0)


/*
** maildir_filter_appendrule and maildir_filter_ruleupdate set err_code to the following upon an error
** exit
*/

#define	MF_ERR_BADRULENAME	1
#define	MF_ERR_BADRULETYPE	2
#define	MF_ERR_BADRULEHEADER	3
#define	MF_ERR_BADRULEVALUE	4
#define	MF_ERR_BADRULEFOLDER	5
#define MF_ERR_BADFROMHDR	6
#define MF_ERR_EXISTS		7
#define MF_ERR_INTERNAL		100

/* Save/Load rules from the given file */

int maildir_filter_saverules(struct maildirfilter *,
		 const char *,		/* Filename */
		 const char *,		/* Maildir */
		 const char *,		/* Path to maildir from mailfilter */
		 const char *);		/* The return address */

int maildir_filter_loadrules(struct maildirfilter *, const char *);

#define	MF_LOADOK	0
#define	MF_LOADNOTFOUND	1
#define	MF_LOADFOREIGN	2
#define	MF_LOADERROR	3

/****************************************************************************/
/*             High-level filter access functions                            */
/****************************************************************************/

int maildir_filter_importmaildirfilter(const char *); /* Get the maildir filter */
int maildir_filter_loadmaildirfilter(struct maildirfilter *, const char *);
int maildir_filter_savemaildirfilter(struct maildirfilter *, const char *, const char *);
int maildir_filter_exportmaildirfilter(const char *);
						/* Commit the maildir filter */
int maildir_filter_hasmaildirfilter(const char *);		/* Is maildir filter defined? */

void maildir_filter_endmaildirfilter(const char *);		/* Remove the temp file */

	/*
	** A convenient structure to parse autoresponder parameters.
	*/

struct maildir_filter_autoresp_info {
	char *name;
	int dsnflag;
	unsigned days;
	int noquote;
} ;

int maildir_filter_autoresp_info_init_str(struct maildir_filter_autoresp_info *, const char *);
int maildir_filter_autoresp_info_init(struct maildir_filter_autoresp_info *, const char *);
void maildir_filter_autoresp_info_free(struct maildir_filter_autoresp_info *);
char *maildir_filter_autoresp_info_asstr(struct maildir_filter_autoresp_info *);

#ifdef __cplusplus
}
#endif

#endif
