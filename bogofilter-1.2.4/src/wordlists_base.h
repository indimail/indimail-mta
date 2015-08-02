/* $Id: wordlists_base.h 6058 2005-06-17 03:35:11Z relson $ */

/*  constants and declarations for wordlists_base.c */

#ifndef	WORDLISTS_CORE_H
#define	WORDLISTS_CORE_H

#ifndef	DATASTORE_H
typedef void *dsh_t;
#endif

#include "paths.h"

typedef enum e_WL_TYPE {
    WL_REGULAR =	'R',	/**< list contains regular tokens */
    WL_IGNORE  =	'I'	/**< list contains tokens that are skipped */
} WL_TYPE;

/** type of a wordlist node */
typedef struct wordlist_s wordlist_t;
/** structure of a wordlist node - singly linked priority queue */
struct wordlist_s
{
    /*@null@*/  wordlist_t *next;	/**< link to next queue node */
    /*@owned@*/ char *listname;		/**< resource name (for debug/verbose messages) */
    /*@owned@*/ bfpath *bfp;		/**< resource path info */
    /*@owned@*/ dsh_t *dsh;		/**< datastore handle */
    u_int32_t	msgcount[IX_SIZE];	/**< count of messages in wordlist. */
    WL_TYPE	type;			/**< datastore type */
    int		override;		/**< priority in queue */
    e_enc	encoding;		/**< encoding */
};

void wordlists_set_bogohome(void);

/** Initialize a wordlist node and insert into the right place of the
 * priority queue, \return -1 for error, 0 for success */
void init_wordlist(const char* name, 
		   const char *path,
		   int override, WL_TYPE type);

/** Print wordlists to stdout, prefixing it by \a fmt which is
 * considered a printf format string and provided one constant argument,
 * "wordlist" (literally).
 */
void display_wordlists(wordlist_t *list, const char *fmt);

/** Free resources of all nodes in the list */
void free_wordlists(void);

/** Get default wordlist for registering messages, finding robx, etc */
wordlist_t *get_default_wordlist(wordlist_t *list);

#endif	/* WORDLISTS_CORE_H */
