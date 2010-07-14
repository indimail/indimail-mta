/* $Id: wordlists.h 6875 2010-02-14 20:57:34Z relson $ */

/*  constants and declarations for wordlists */

#ifndef	WORDLISTS_H
#define	WORDLISTS_H

#include "bftypes.h"
#include "wordlists_base.h"

extern const char *aCombined[];
extern size_t	   cCombined;
extern const char *aSeparate[];
extern size_t	   cSeparate;

typedef	char FILEPATH[PATH_LEN];

/*@null@*/
extern wordlist_t *word_lists;

void incr_wordlist_mode(void);
void set_wordlist_mode(const char *filepath);
bool configure_wordlist(const char *val);

/**
 * initialize wordlist the same way as open_wordlists does, like
 * beginning a transaction and reading message counts
 */
void begin_wordlist(wordlist_t *list);

void open_wordlists(dbmode_t mode);
bool close_wordlists(bool commit);
bool query_wordlists_closed(void);

void set_list_active_status(bool status);
void set_wordlist_directory(void);

void wordlist_error(int err);

#endif	/* WORDLISTS_H */
