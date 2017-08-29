/** \file tls.c - collect common TLS functionality 
 * \author Matthias Andree
 * \date 2006
 */

#include "fetchmail.h"

#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

/** return true if user allowed opportunistic STARTTLS/STLS */
int maybe_starttls(struct query *ctl) {
#ifdef SSL_ENABLE
         /* opportunistic  or forced TLS */
    return (!ctl->sslproto || strlen(ctl->sslproto))
	&& !ctl->use_ssl;
#else
    (void)ctl;
    return 0;
#endif
}

/** return true if user requires STARTTLS/STLS, note though that this
 * code must always use a logical AND with maybe_tls(). */
int must_starttls(struct query *ctl) {
#ifdef SSL_ENABLE
    return maybe_starttls(ctl)
	&& (ctl->sslfingerprint || ctl->sslcertck
		|| (ctl->sslproto && !strcasecmp(ctl->sslproto, "tls1")));
#else
    (void)ctl;
    return 0;
#endif
}
