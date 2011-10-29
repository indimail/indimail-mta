#ifndef MISC_TYPES_H
#define MISC_TYPES_H

#include <inttypes.h>
#include <sys/types.h>

/* used for crypto routines */
#ifdef uint32_t
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t   u8;

typedef int32_t i32;
typedef int16_t i16;
typedef int8_t   i8;
#else
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef signed int i32;
typedef signed short i16;
typedef signed char i8;
#endif

#endif
