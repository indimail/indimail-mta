/*
 * $Log: $
 */
#include <stddef.h>
#include "query-getdns.h"
/*
 * Flags: dns bogus or indeterminate; authenticate responses
 */
int             dns_bogus_or_indeterminate;
int             address_authenticated;
int             v4_authenticated;
int             v6_authenticated;
int             mx_authenticated;
int             srv_authenticated;
int             tlsa_authenticated;

/*
 * addresses: (head of) linked list of addrinfo structures
 */
size_t          address_count;
struct addrinfo *addresses;

/*-
 * tlsa_count: count of TLSA records.
 */
size_t          tlsa_count;
tlsa_rdata     *tlsa_rdata_list;

/*- enum AUTH_MODE  auth_mode = MODE_BOTH; -*/

/*- error handling */
int             danetlsa_error;
char           *danetlsa_err_str;
