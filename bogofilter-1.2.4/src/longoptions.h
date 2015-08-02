/* $Id: longoptions.h 6993 2013-06-28 21:54:23Z m-a $ */

/*****************************************************************************

NAME:
   longoptions.h -- definitions for longoptions.c

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#ifndef OPTIONS_H
#define OPTIONS_H

#include "getopt.h"

/* Definitions */

/** has_arg symbolic values */
enum field_e {
    N = 0,	/* no_argument		(or 0) if the option does not take an argument, */
    R = 1,	/* required_argument	(or 1) if the option requires an argument, */
    O = 2	/* optional_argument 	(or 2) if the option takes an optional argument. */
};

typedef enum longopts_e {
    O_BLOCK_ON_SUBNETS = 1000,
    O_CHARSET_DEFAULT,
    O_CONFIG_FILE,
    O_DB_CHECKPOINT,
    O_DB_LIST_LOGFILES,
    O_DB_PRINT_LEAFPAGE_COUNT,
    O_DB_PRINT_PAGESIZE,
    O_DB_PRUNE,
    O_DB_RECOVER,
    O_DB_RECOVER_HARDER,
    O_DB_REMOVE_ENVIRONMENT,
    O_DB_VERIFY,
    O_DB_LOG_AUTOREMOVE,
    O_DB_TRANSACTION,
    O_DB_TXN_DURABLE,
    O_NS_ESF,
    O_SP_ESF,
    O_HAM_CUTOFF,
    O_HAM_TRUE,
    O_HEADER_FORMAT,
    O_LOG_HEADER_FORMAT,
    O_LOG_UPDATE_FORMAT,
    O_MIN_DEV,
    O_MIN_TOKEN_LEN,
    O_MAX_TOKEN_LEN,
    O_MAX_MULTI_TOKEN_LEN,
    O_MULTI_TOKEN_COUNT,
    O_REPLACE_NONASCII_CHARACTERS,
    O_ROBS,
    O_ROBX,
    O_SPAM_CUTOFF,
    O_SPAM_HEADER_NAME,
    O_SPAM_HEADER_PLACE,
    O_SPAM_SUBJECT_TAG,
    O_SPAMICITY_FORMATS,
    O_SPAMICITY_TAGS,
    O_STATS_IN_HEADER,
    O_TERSE,
    O_TERSE_FORMAT,
    O_THRESH_UPDATE,
    O_TOKEN_COUNT_FIX,
    O_TOKEN_COUNT_MIN,
    O_TOKEN_COUNT_MAX,
    O_TIMESTAMP,
    O_UNICODE,
    O_UNSURE_SUBJECT_TAG,
    O_USER_CONFIG_FILE,
    O_WORDLIST
} longopts_t;

#ifndef	DISABLE_UNICODE
#define	UNICODE_OPTION	\
    { "unicode",			R, 0, O_UNICODE },
#else
#define	UNICODE_OPTION
#endif

/* common options */
#define LONGOPTIONS_COMMON \
    { "config-file",			R, 0, O_CONFIG_FILE }, \
    { "no-config-file",			N, 0, 'C' }, \
    { "help",				N, 0, 'h' }, \
    { "input-file",			R, 0, 'I' }, \
    { "output-file",			R, 0, 'O' }, \
    { "min-token-len",			R, 0, O_MIN_TOKEN_LEN }, \
    { "max-token-len",			R, 0, O_MAX_TOKEN_LEN }, \
    { "max-multi-token-len",		R, 0, O_MAX_MULTI_TOKEN_LEN }, \
    { "multi-token-count",		R, 0, O_MULTI_TOKEN_COUNT }, \
       UNICODE_OPTION \
    { "version",			N, 0, 'V' }, \
    { "verbosity",			N, 0, 'v' },

/* options for bogofilter */
#define LONGOPTIONS_MAIN \
    { "ham-true"	,		N, 0, O_HAM_TRUE },

/* options for bogofilter */
#define LONGOPTIONS_MAIN_TUNE \
    { "token-count"     ,               R, 0, O_TOKEN_COUNT_FIX }, \
    { "token-count-min" ,               R, 0, O_TOKEN_COUNT_MIN }, \
    { "token-count-max" ,               R, 0, O_TOKEN_COUNT_MAX },

/* options for bogofilter and bogolexer */
#define LONGOPTIONS_LEX \
    { "block-on-subnets",		R, 0, O_BLOCK_ON_SUBNETS }, \
    { "charset-default",		R, 0, O_CHARSET_DEFAULT }, \
    { "user-config-file",		R, 0, O_USER_CONFIG_FILE }, \
    { "replace-nonascii-characters",	R, 0, O_REPLACE_NONASCII_CHARACTERS },

/* options for bogolexer and bogoutil */
#define LONGOPTIONS_LEX_UTIL

/* options for bogofilter and bogoutil - some preprocessor workarounds here */
#define lo1
#define lo2
#ifdef	HAVE_DECL_DB_CREATE
 #undef lo1
 #define lo1 \
    { "db-log-autoremove",		R, 0, O_DB_LOG_AUTOREMOVE },
 #ifdef	FUTURE_DB_OPTIONS
  #undef lo2
  #define lo2 \
    { "db-txn-durable",			R, 0, O_DB_TXN_DURABLE },
 #endif
#endif

#define LONGOPTIONS_DB \
    { "db-transaction",			R, 0, O_DB_TRANSACTION }, \
    { "timestamp-date",			R, 0, 'y' }, \
    lo1 lo2

extern int getopt_long_chk(int argc, char * const argv[], char const
	*optstring, const struct option *longopts, int *longindex);

#endif
