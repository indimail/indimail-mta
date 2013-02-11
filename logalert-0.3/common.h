/*
 * $Log: $
 */
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
typedef int64_t mdir_t;
typedef uint64_t umdir_t;
#else
typedef long long mdir_t;
typedef unsigned long long umdir_t;
#define PRId64 "lld"
#define PRIu64 "llu"
#define SCNd64 "lld"
#define SCNu64 "llu"
#endif

