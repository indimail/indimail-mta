#include <sys/types.h>
#include <errno.h>
#if defined (sun)
#include <nss_common.h>
#include <nss_dbdefs.h>
typedef nss_status_t NSS_STATUS;
#define NSS_ARGS(args)  ((nss_XbyY_args_t *)args)
#else
#include <nss.h>
#define NSS_SUCCESS     NSS_STATUS_SUCCESS
#define NSS_NOTFOUND    NSS_STATUS_NOTFOUND
#define NSS_UNAVAIL     NSS_STATUS_UNAVAIL
#define NSS_TRYAGAIN    NSS_STATUS_TRYAGAIN
typedef enum nss_status NSS_STATUS;
#endif

#define SINGLE_READ_TIMEOUT 1
#define MULTI_READ_TIMEOUT 5
#define WRITE_TIMEOUT 1

/* Alignment code adapted from padl.com's nss_ldap */
#ifdef __GNUC__
#define alignof(ptr) __alignof__(ptr)
#else
#define alignof(ptr) (sizeof(char *))
#endif

#define align(ptr, blen, TYPE)                                               \
do                                                                           \
  {                                                                          \
    char *qtr = ptr;                                                         \
    ptr += alignof(TYPE) - 1;                                                \
    ptr -= ((ptr - (char *)NULL) % alignof(TYPE));                           \
    blen -= (ptr - qtr);                                                     \
  } while (0)

/*-
 * Linux and Solaris handle buffer exhaustion differently.
 * Linux sets errno to ERANGE and returns TRYAGAIN, which results in
 * the NSS system trying with a buffer twice as big.
 * Solaris, however, doesn't seem to retry.  I've checked the Solaris 8
 * code for files/ldap/nisplus NSS and they all set NSS_ARGS(args)->erange
 * to 1 and return NOTFOUND.  Note that this macro sets *errnop to 1, but
 * it's not really errnop, it's erange - see the calling functions.
 * In fact, my tests reveal that if you return TRYAGAIN, Solaris will try
 * over and over, without increasing the buffer - AKA infinite (or long)
 * loop.
 */
#ifdef sun
#define EXHAUSTED_BUFFER                                                     \
do                                                                           \
  {                                                                          \
    if (errnop)                                                              \
      *errnop = 1;                                                           \
    return (NSS_NOTFOUND);                                                   \
  } while (0);
#else
#define EXHAUSTED_BUFFER                                                     \
do                                                                           \
  {                                                                          \
    *errnop = ERANGE;                                                        \
    return (NSS_TRYAGAIN);                                                   \
  } while (0);
#endif

/* s.c */
NSS_STATUS _get_response_data (int32_t type, const char *key,
                               response_header_t *response_header,
                               struct response_data **data, int timeout);
void nsvs_log (int prio, const char *fmt, ...);
#if defined (sun)
NSS_STATUS _nss_nssd_default_destr (nss_backend_t *be, void *args);
#endif
