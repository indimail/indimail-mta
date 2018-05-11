#ifndef __QUERY_GETDNS_H__
#define __QUERY_GETDNS_H__

#include <stdlib.h>
#include <stdint.h>

/*-
 * tlsa_rdata: structure to hold TLSA record rdata.
 * insert_tlsa_rdata(): insert node at tail of linked list of tlsa_rdata.
 * free_tlsa(): free memory in the linked list.
 */
typedef struct tlsa_rdata {
	uint8_t         usage;
	uint8_t         selector;
	uint8_t         mtype;
	unsigned long   data_len;
	uint8_t        *data;
	struct tlsa_rdata *next;
} tlsa_rdata;

/*-
 * qinfo: structure to hold query information to be passed to
 * callback functions.
 */
typedef struct qinfo {
	const char     *qname;
	uint16_t        qtype;
	uint16_t        port;
} qinfo;

typedef struct addrinfo addrinf;

enum AUTH_MODE {
    MODE_BOTH=0,
    MODE_DANE,
    MODE_PKIX
};

tlsa_rdata     *insert_tlsa_rdata(tlsa_rdata **, tlsa_rdata *, tlsa_rdata *);
void            free_tlsa(tlsa_rdata *);
void            print_tlsa(tlsa_rdata *);
addrinf        *insert_addrinfo(addrinf *, addrinf *);
int             do_dns_queries(const char *, uint16_t, int);
char           *bin2hexstring(uint8_t *, size_t);

extern addrinf *addresses;
extern int      dns_bogus_or_indeterminate;
extern int      address_authenticated;
extern int      v4_authenticated;
extern int      v6_authenticated;
extern int      tlsa_authenticated;
extern size_t   tlsa_count;
extern      int danetlsa_error;
extern    char *danetlsa_err_str;
extern enum AUTH_MODE auth_mode;
extern tlsa_rdata *tlsa_rdata_list;

#define GETDNS_NO_ERR                  0
#define GETDNS_MEM_ERR                 1
#define GETDNS_TIMEOUT_ERR             2
#define GETDNS_CALLBACK_ERR            3
#define GETDNS_CALLBACKCANCEL_ERR      4
#define GETDNS_NO_EXIST_DOMAIN_ERR     5
#define GETDNS_NO_SECURE_ANSWER_ERR    6
#define GETDNS_ALL_BOGUS_ANSWER_ERR    7
#define GETDNS_RES_INDETERMINATE_ERR   8
#define GETDNS_DICT_RESPONSE_ERR       9
#define GETDNS_DICT_ANSWER_ERR         10
#define GETDNS_ZERO_REPLY_ERR          11
#define GETDNS_DNSSEC_STATUS_ERR       12
#define GETDNS_DNSSEC_INSECURE_ERR     13

#endif /*- __QUERY_GETDNS_H__ */
