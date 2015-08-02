/* $Id: system.h 6657 2007-03-22 15:24:47Z m-a $ */

/*****************************************************************************

NAME:
   system.h -- system definitions and prototypes for bogofilter

******************************************************************************/

/* parts were taken from autoconf.info */

#ifndef SYSTEM_H
#define SYSTEM_H

#ifndef	CONFIG_H
# define  CONFIG_H
# include "config.h"
#endif

#ifndef BFTYPES_H
# include "bftypes.h"
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#if defined(HAVE_LIMITS_H)
# include <limits.h>
#elif defined(HAVE_SYS_PARAM_H)
# include <sys/param.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include <signal.h>

#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif

#if HAVE_STRINGS_H
#include <strings.h>
#endif

#if !STDC_HEADERS
# if !HAVE_STRCHR
#  define strchr index
# endif
# if !HAVE_STRRCHR
#  define strrchr rindex
# endif

# if !HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
# endif
# if !HAVE_MEMMOVE
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(/*@out@*/ char *dst, const char *src, size_t size);
#endif

#ifndef HAVE_STRLCAT
size_t strlcat(/*@out@*/ char *dst, const char *src, size_t size);
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

/* dirent.h surroundings */
/* from autoconf.info */
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#ifdef __DGUX__
#undef EX_OK
#endif

/* Ignore __attribute__ if not using GNU CC */
#if	!defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(a)
#endif

/* system.c - function prototypes */

extern bool bf_abspath(const char *path);
extern int  bf_mkdir(const char *path, mode_t mode);
extern void bf_sleep(long delay);

#endif /* SYSTEM_H */
