#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>

#include "config.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif


#define DEFAULT_MATCHSLEEP 5
#define DEFAULT_MATCHCOUNT 1
#define DEFAULT_RETRY      5

#define MAXBUFFSIZE 2048

#ifndef FALSE
#define FALSE                           0
#define TRUE                            1
#endif

#define EQS( s1, s2 )          ( strcasecmp( s1, s2 ) == 0 )
#define EQC( c1, c2 )          ( c1 == c2 )
#define SCP( s1, s2 )           ( strcpy( s1, s2 ) )

typedef enum { FAILED = 0, OK } status_e ;
typedef enum { NO = 0, YES } boolean_e ;

typedef void (*voidfunc)() ;
typedef status_e (*statfunc)() ;


#endif   /* DEFS_H */
