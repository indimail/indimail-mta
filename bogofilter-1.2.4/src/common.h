/* $Id: common.h 6795 2009-02-14 18:29:46Z relson $ */

/*****************************************************************************

NAME:
   common.h -- common definitions and prototypes for bogofilter

******************************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include "system.h"

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#include <stdio.h>

#include "debug.h"

#ifdef	ENABLE_MEMDEBUG
#include "memdebug.h"
#else
#define	MEMDISPLAY  do { } while(0)
#endif

/* for easier debugging - can be disabled */
#if	0
#define	D	0	/* size adjustment */
#define	Z(n)		/* mark end of string */
#else
#define	D	1	/* size adjustment */
#define	Z(n) n=(byte)'\0' /* mark end of string */
#endif

/* length of token between these values */
#define MIN_TOKEN_LEN		 3	/* default value of min-token-len       */
#define MAX_TOKEN_LEN		30	/* default value of max-token-len       */
#define MAX_MULTI_TOKEN_LEN	30	/* default value of max-multi-token-len */
#define	MAX_PREFIX_LEN	 	 5	/* maximum length of prefix		*/
#define	MUL_TOKEN_CNT		 1	/* default value of multi-token-count   */

typedef enum sh_e { IX_SPAM = 0, 	/* index for SPAM */
		    IX_GOOD = 1, 	/* index for GOOD */
		    IX_SIZE = 2, 	/* array size     */
		    IX_UNDF = 3 	/* ... undefined  */
} sh_t;

#define max(x, y)	(((x) > (y)) ? (x) : (y))
#define min(x, y)	(((x) < (y)) ? (x) : (y))

#define	NL	"\n"
#define	CRLF	"\r\n"

#if defined(PATH_MAX)
#define PATH_LEN PATH_MAX
#elif defined(MAXPATHLEN)
#define PATH_LEN MAXPATHLEN
#else
#define PATH_LEN 1024
#endif

/** Default database file mode */
#define	DS_MODE 	(mode_t) 0664

/** Default directory */
#define	DIR_MODE	(mode_t) 0775

#define COUNTOF(array)	(uint) (sizeof(array)/sizeof(array[0]))

typedef unsigned char byte;

enum dbmode_e { DS_READ = 1, DS_WRITE = 2, DS_LOAD = 8 };
typedef enum dbmode_e dbmode_t;

#define BIT(n)	(1 << n)

typedef enum rc_e { RC_SPAM	= 0,
		    RC_HAM	= 1,
		    RC_UNSURE	= 2,
		    RC_OK,
		    RC_MORE	}  rc_t;

typedef enum ex_e { EX_SPAM	= RC_SPAM,
		    EX_HAM	= RC_HAM,
		    EX_UNSURE	= RC_UNSURE,
		    EX_OK	= 0,
		    EX_ERROR	= 3 } ex_t;

typedef enum run_e {
    RUN_UNKNOWN= 0,
    RUN_NORMAL = BIT(0),
    RUN_UPDATE = BIT(1),
    REG_SPAM   = BIT(2),
    REG_GOOD   = BIT(3),
    UNREG_SPAM = BIT(4),
    UNREG_GOOD = BIT(5)
} run_t;
extern run_t run_type;
extern bool  run_classify;
extern bool  run_register;

typedef struct {
    double mant;
    int    exp;
} FLOAT;

typedef enum priority_e {
    PR_NONE,		/* 0 */
    PR_ENV_HOME,	/* 1 */
    PR_CFG_SITE,	/* 2 */
    PR_CFG_USER,	/* 3 */
    PR_CFG_UPDATE,	/* 4 */
    PR_ENV_BOGO,	/* 5 */
    PR_COMMAND		/* 6 */
} priority_t;

typedef enum bulk_e {
    B_NORMAL,
    B_CMDLINE,
    B_STDIN
} bulk_t;

/* for transaction flag */

typedef	enum {
    T_ERROR    = -1,	/* -1 for error */
    T_DISABLED =  0,	/*  0 for no transactions - 0 must mean T_DISABLE
			    for compatibility with dummy functions */
    T_ENABLED  =  1,	/*  1 for transactions */
    T_DEFAULT_OFF =  2, /*  2 for off, unless explicity specified */
    T_DEFAULT_ON  =  3, /*  3 for on, unless explicity specified */
    T_DONT_KNOW		/*  4 for don't know */
} e_txn;

/* for encoding (unicode) flag */

typedef	enum {
    E_UNKNOWN = 0,	/* 0 for not set  */
    E_RAW     = 1,	/* 1 for raw text */
    E_UNICODE = 2,	/* 2 for unicode  */
#ifdef	ENABLE_UNICODE
    E_DEFAULT = E_UNICODE
#else
    E_DEFAULT = E_RAW
#endif
} e_enc;

#include "globals.h"

/* Represents the secondary data for a word key */
typedef struct {
    u_int32_t	good;
    u_int32_t	bad;
    u_int32_t	msgs_good;
    u_int32_t	msgs_bad;
} wordcnts_t;

typedef struct {
    wordcnts_t  cnts;
    double 	prob;
    int		freq;
    bool	used;
} wordprop_t;

extern void bf_exit(void);

#define internal_error do { fprintf(stderr, "Internal error in %s:%lu\n", __FILE__, (unsigned long)__LINE__); abort(); } while(0)

typedef enum e_wordlist_version {
    ORIGINAL_VERSION = 0,
    IP_PREFIX = 20040500	/* when IP prefixes were added */
} t_wordlist_version;

#define	CURRENT_VERSION	IP_PREFIX

/* for bogoutil.c and datastore_db_trans.c */

typedef enum { M_NONE, M_DUMP, M_LOAD, M_WORD, M_MAINTAIN, M_ROBX, M_HIST,
    M_LIST_LOGFILES, M_LEAFPAGES,
    M_RECOVER, M_CRECOVER, M_PURGELOGS, M_VERIFY, M_REMOVEENV, M_CHECKPOINT,
    M_PAGESIZE }
    cmd_t;

#endif
