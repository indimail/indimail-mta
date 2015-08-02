/* $Id: maint.h 6053 2005-06-13 00:59:18Z relson $ */

#ifndef MAINT_H
#define MAINT_H

#include <time.h>

#include "datastore.h"

#define	AGE_IS_YDAY
#undef	AGE_IS_YDAY
#undef	AGE_IS_YYYYMMDD
#define	AGE_IS_YYYYMMDD

extern	uint32_t thresh_count;
extern	YYYYMMDD thresh_date;
extern	size_t   size_min, size_max;
extern	bool     timestamp_tokens;
extern	bool     replace_nonascii_characters;
extern	bool     upgrade_wordlist_version;

/* Function Prototypes */
ex_t maintain_wordlist_file(bfpath *bfp);

bool discard_token(word_t *token, dsv_t *info);

bool do_replace_nonascii_characters(byte *, size_t);

void set_today(void);
void set_date(YYYYMMDD date);
time_t long_to_date(long l);
YYYYMMDD string_to_date(const char *s);

#endif	/* MAINT_H */
