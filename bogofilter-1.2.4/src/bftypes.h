/* $Id: bftypes.h 6023 2005-06-04 12:47:16Z relson $ */

/*****************************************************************************/
/** \file bftypes.h
 * \brief Type definitions for bogofilter.
 *
 * This file shall define the bool and uint32_t types.
 * it shall include inttypes.h and stdbool.h if present.
 *
 * Parts were taken from autoconf.info.
 */
/*****************************************************************************/

#ifndef BFTYPES_H
#define BFTYPES_H

#ifndef	CONFIG_H
# define  CONFIG_H
# include "config.h"
#endif

/** Define C99 style _Bool type for C89 compilers. */
#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
# if ! HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
typedef unsigned char _Bool;
#  endif
# endif
/** alias C99-standard _Bool type to bool */
# define bool _Bool
/** default value for false */
# define false 0
/** default value for true */
# define true 1
/* internal - marker that we have defined true/false */
# define __bool_true_false_are_defined 1
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#elif HAVE_STDINT_H
#include <stdint.h>
#endif

/** unsigned 32-bit integer */
#ifndef HAVE_UINT32_T
#ifdef HAVE_U_INT32_T
typedef u_int32_t uint32_t;
#elif SIZEOF_LONG == 4
typedef unsigned long uint32_t;
#elif SIZEOF_INT == 4
typedef unsigned int uint32_t;
#elif SIZEOF_SHORT == 4
typedef unsigned short uint32_t;
#else
#error we do not know how to define uint32_t
#endif
#endif /* HAVE_UINT32_T */
#ifndef HAVE_U_INT32_T
typedef uint32_t u_int32_t;
#endif

/** signed 32-bit integer */
#ifndef HAVE_INT32_T
#if SIZEOF_LONG == 4
typedef signed long int32_t;
#elif SIZEOF_INT == 4
typedef signed int int32_t;
#elif SIZEOF_SHORT == 4
typedef signed short int32_t;
#else
#error we do not know how to define int32_t
#endif
#endif /* HAVE_INT32_T */

/** unsigned 16-bit integer */
#ifndef HAVE_UINT16_T
#if SIZEOF_SHORT == 2
typedef unsigned short uint16_t;
#else
#error we do not know how to define uint16_t
#endif
#endif
#ifndef HAVE_U_INT16_T
typedef uint16_t u_int16_t;
#endif

/** signed 16-bit integer */
#ifndef HAVE_INT16_T
#if SIZEOF_SHORT == 2
typedef signed short int16_t;
#else
#error we do not know how to define int16_t
#endif
#endif

/** unsigned 8-bit integer */
#ifndef HAVE_U_INT8_T
typedef unsigned char u_int8_t;
#endif

/** alias for unsigned long */
#ifndef HAVE_ULONG
typedef unsigned long ulong;
#endif

/** alias for unsigned int */
#ifndef HAVE_UINT
typedef unsigned int uint;
#endif

#if !defined(HAVE_SSIZE_T)
typedef int ssize_t;
#endif

/** type for getrlimit/setrlimit functions (some systems don't define this
 * type) */
#ifndef HAVE_RLIM_T
typedef int rlim_t;
#endif

/** prevent db.h from redefining the types above */
#undef	__BIT_TYPES_DEFINED__
#define	__BIT_TYPES_DEFINED__ 1

/* splint crutch */
#ifdef __LCLINT__
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef uint8_t u_int8_t;
#define false 0
#define true 1
#endif

/** Data type for a date stamp in YYYY*10000 + MM*100 + DD format */
typedef uint32_t YYYYMMDD;

/* sanity check */
#ifdef HAVE_SIZE_T
#if SIZEOF_INT > SIZEOF_SIZE_T
#error "int is wider than size_t. The current code is not designed to work on such systems and needs review."
#endif
#endif

#endif /* BFTYPES_H */
