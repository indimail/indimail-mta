/*
 * $Id: qmail.h,v 1.12 2024-01-23 01:22:42+05:30 Cprogrammer Exp mbhangui $
 */
#ifndef QMAIL_H
#define QMAIL_H

#include "substdio.h"
#include "buffer_defs.h"

#ifndef CUSTOM_ERR_FD
#define CUSTOM_ERR_FD 2
#endif

#define DEATH         86400  /*- 24 hours; must be below qmail-send's OSSIFIED (36 hours) */
#define OSSIFIED      129600 /*- 36 hours; must exceed   qmail-queue's DEATH   (24 hours) */

struct qmail
{
	int             flagerr;
	unsigned long   pid;
	int             fdm; /*- fd message */
	int             fde; /*- fd envelope */
	int             fdc; /*- fd custom */
	substdio        ss;
	char            buf[BUFSIZE_MESS];
};

int             qmail_open(struct qmail *);
void            qmail_put(struct qmail *, const char *, unsigned int);
void            qmail_puts(struct qmail *, const char *);
void            qmail_from(struct qmail *, const char *);
void            qmail_to(struct qmail *, const char *);
void            qmail_fail(struct qmail *);
const char     *qmail_close(struct qmail *);
unsigned long   qmail_qp(struct qmail *);

#define QQ_OK                  0
#define QQ_COMPAT             115

/*-
 * if you add new errors relook
 * at QQ_MIN_PERM and QQ_MAX_PERM
 */
#define QQ_MIN_PERM           11
#define QQ_MAX_PERM           40
#define perm_error(x)         ((x) == QQ_COMPAT || ((x) >= QQ_MIN_PERM && (x) <= QQ_MAX_PERM))
#define temp_error(x)         (((x) && (x) < QQ_MIN_PERM) || ((x) > QQ_MAX_PERM && (x) != QQ_COMPAT))

/*- permanent errors QQ_MIN_PERM to QQ_MAX_PERM */
#define QQ_ENVELOPE_TOO_LONG  11
#define QQ_PERM_MSG_REJECT    31
#define QQ_SPAM_THRESHOLD     32
#define QQ_VIRUS_IN_MSG       33
#define QQ_BANNED_ATTACHMENT  34
#define QQ_NO_PRIVATE_KEY     35

/*- define temporary errors > QQ_MAX_PERM */
/*- temporary errors */
#define QQ_DUP_ERR            41
#define QQ_VIRUS_SCANNER_PRIV 50
#define QQ_OUT_OF_MEMORY      51
#define QQ_TIMEOUT            52
#define QQ_WRITE_ERR          53
#define QQ_READ_ERR           54
#define QQ_CONFIG_ERR         55
#define QQ_NETWORK            56
#define QQ_OPEN_SHARED_OBJ    57
#define QQ_RESOLVE_SHARED_SYM 58
#define QQ_CLOSE_SHARED_OBJ   59
#define QQ_PIPE_SOCKET        60
#define QQ_CHDIR              61
#define QQ_MESS_FILE          62
#define QQ_CD_ROOT            63
#define QQ_FSYNC_ERR          64
#define QQ_INTD_FILE          65
#define QQ_LINK_TODO_INTD     66
#define QQ_LINK_MESS_PID      67
#define QQ_TMP_FILES          68
#define QQ_SYNCDIR_ERR        69
#define QQ_PID_FILE           70
#define QQ_TEMP_MSG_REJECT    71
#define QQ_CONN_TIMEOUT       72
#define QQ_CONN_REJECT        73
#define QQ_CONN_FAILED        74
#define QQ_EXEC_FAILED        75
#define QQ_TEMP_SPAM_FILTER   76
#define QQ_QHPSI_TEMP_ERR     77
#define QQ_GET_UID_GID        78
#define QQ_ENVELOPE_FMT_ERR   79
#define QQ_REMOVE_INTD_ERR    80
#define QQ_INTERNAL_BUG       81
#define QQ_REMOVE_PID_ERR     83
#define QQ_SYSTEM_MISCONFIG   87
#define QQ_CUSTOM_ERR         88
#define QQ_EXEC_QMAILQUEUE   120
#define QQ_FORK_ERR          121
#define QQ_WAITPID_SURPRISE  122
#define QQ_CRASHED           123

#endif

/*
 * $Log: qmail.h,v $
 * Revision 1.12  2024-01-23 01:22:42+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.11  2024-01-20 23:27:50+05:30  Cprogrammer
 * increased qmail-queue buffer to 8192 for performance
 *
 * Revision 1.10  2023-12-25 09:30:13+05:30  Cprogrammer
 * added defintion for DEATH and OSSIFIED
 *
 * Revision 1.9  2023-02-08 18:47:31+05:30  Cprogrammer
 * added perm_error, temp_error macro to evaluate perm/temp errors
 *
 * Revision 1.8  2022-10-17 19:44:36+05:30  Cprogrammer
 * define qmail-queue exit codes
 *
 * Revision 1.7  2022-10-04 23:43:51+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.6  2020-05-12 12:14:13+05:30  Cprogrammer
 * fix integer signedness error in qmail_put() (CVE-2005-1515)
 *
 * Revision 1.5  2009-04-21 14:27:24+05:30  Cprogrammer
 * define CUSTOM_ERR_FD
 *
 * Revision 1.4  2005-05-31 15:45:23+05:30  Cprogrammer
 * added fdc for passing custom error message to qmail-queue
 *
 * Revision 1.3  2004-10-11 13:57:51+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:26+05:30  Cprogrammer
 * added RCS log
 *
 */
