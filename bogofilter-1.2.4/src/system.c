/* $Id: system.c 6484 2006-05-29 14:28:00Z relson $ */

/*****************************************************************************

NAME:
   system.c -- bogofilter covers for OS/compiler dependent functions.

******************************************************************************/

#include "common.h"

#if defined(__IBMC__) || defined(__IBMCPP__) || defined(__WATCOMC__)
#define _OS2_
/* OS/2 stores the configuration file in the same directory as the
 * programs are run from.
 * Requested by Evgeny Kotsuba <evgen@shatura.laser.ru> */
const char * const system_config_file = "bogofilter.cf";
#include "direct.h"
#endif

#ifdef __riscos__
/* static symbols that trigger UnixLib behaviour */
#include <unixlib/local.h> /* __RISCOSIFY_NO_PROCESS */
int __riscosify_control = __RISCOSIFY_NO_PROCESS;
int __feature_imagefs_is_file = 1;
const char *const system_config_file = "<Bogofilter$Dir>.bogofilter/cf";
#endif

/* import select() */
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

/** Checks if \a path is absolute. */
bool bf_abspath(const char *path /** path to check for absoluteness */)
{
#if defined(__OS2__)
    return (bool) strchr(path, ':') || (bool) (*path == '\\') || (bool) (*path == '/');
#elif defined(__riscos__)
    return (bool) (strchr(path, ':') || strchr(path, '$') || strchr(path, '#') ||
		   strchr(path, '@') || strchr(path, '%') || strchr(path, '&'));
#else /* POSIX and similar*/
    return (bool) (*path == DIRSEP_C);
#endif
}

/** sleep for \a delay microseconds (rounded to nearest millisecond on
 * OS/2) */
void bf_sleep(long delay /** microseconds to wait */)
{
#ifndef _OS2_
    struct timeval timeval;
    timeval.tv_sec  = delay / 1000000;
    timeval.tv_usec = delay % 1000000;
    select(0,NULL,NULL,NULL,&timeval);
#else
/*APIRET DosSleep(ULONG  msec )  */
    DosSleep((delay+500)/1000);
#endif
}

/** Create a new directory \a path with permissions \a mode, modified by
 * the current umask. \a mode is ignored on OS/2. */
int bf_mkdir(const char *path /** new directory to be created */,
	mode_t mode /** permissions for new directory, subject to umask restrictions */)
{
    int rc;
#ifndef _OS2_ /* POSIX and similar */
    rc = mkdir(path, mode);
#else
    rc = mkdir((unsigned char *)path);
#endif
    return rc;
}
