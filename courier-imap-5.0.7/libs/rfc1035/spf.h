/*
** Copyright 2004 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifndef	rfc1035_spf_h
#define	rfc1035_spf_h


#include "rfc1035/rfc1035.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*
   An SPF client evaluates an SPF record and produces one of seven
   results:

     None: The domain does not publish SPF data.

     Neutral (?): The SPF client MUST proceed as if a domain did not
     publish SPF data.  This result occurs if the domain explicitly
     specifies a "?" value, or if processing "falls off the end" of
     the SPF record.

     Pass (+): the message meets the publishing domain's definition of
     legitimacy.  MTAs proceed to apply local policy and MAY accept or
     reject the message accordingly.

     Fail (-): the message does not meet a domain's definition of
     legitimacy.  MTAs MAY reject the message using a permanent
     failure reply code.  (Code 550 is RECOMMENDED.  See [RFC2821]
     section 7.1.)

     Softfail (~): the message does not meet a domain's strict
     definition of legitimacy, but the domain cannot confidently state
     that the message is a forgery.  MTAs SHOULD accept the message
     but MAY subject it to a higher transaction cost, deeper scrutiny,
     or an unfavourable score.

   There are two error conditions, one temporary and one permanent.

     Error: indicates an error during lookup; an MTA SHOULD reject the
     message using a transient failure code, such as 450.

     Unknown: indicates incomplete processing: an MTA MUST proceed as
     if a domain did not publish SPF data.
*/

#define SPF_NONE 0
#define SPF_NEUTRAL '?'
#define SPF_PASS '+'
#define SPF_FAIL '-'
#define SPF_SOFTFAIL '~'
#define SPF_ERROR '4'

	/* Everything else is SPF_UNKNOWN */
#define SPF_UNKNOWN '5'

char rfc1035_spf_lookup(const char *mailfrom,
			const char *tcpremoteip,
			const char *tcpremotehost,
			const char *helodomain,
			const char *mydomain,
			char *errmsg_buf,
			size_t errmsg_buf_size);

#ifdef  __cplusplus
}
#endif

#endif
