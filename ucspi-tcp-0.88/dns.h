/*
 * $Log: dns.h,v $
 * Revision 1.5  2007-06-10 10:13:52+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2005-06-10 11:06:30+05:30  Cprogrammer
 * removed blank lines
 *
 * Revision 1.3  2005-06-10 09:05:09+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef DNS_H
#define DNS_H

#include "haveip6.h"
#include "stralloc.h"
#include "iopause.h"
#include "taia.h"

#define DNS_C_IN "\0\1"
#define DNS_C_ANY "\0\377"

#define DNS_T_A "\0\1"
#define DNS_T_NS "\0\2"
#define DNS_T_CNAME "\0\5"
#define DNS_T_SOA "\0\6"
#define DNS_T_PTR "\0\14"
#define DNS_T_HINFO "\0\15"
#define DNS_T_MX "\0\17"
#define DNS_T_TXT "\0\20"
#define DNS_T_RP "\0\21"
#define DNS_T_SIG "\0\30"
#define DNS_T_KEY "\0\31"
#define DNS_T_AAAA "\0\34"
#define DNS_T_AXFR "\0\374"
#define DNS_T_ANY "\0\377"
#define DNS_NAME4_DOMAIN 31
#ifdef LIBC_HAS_IP6
#define DNS_NAME6_DOMAIN (4*16+11)
#endif

struct dns_transmit
{
	char           *query;		/*- 0, or dynamically allocated */
	unsigned int    querylen;
	char           *packet;		/*- 0, or dynamically allocated */
	unsigned int    packetlen;
	int             s1;			/*- 0, or 1 + an open file descriptor */
	int             tcpstate;
	unsigned int    udploop;
	unsigned int    curserver;
	struct taia     deadline;
	unsigned int    pos;
	char           *servers;
#ifdef LIBC_HAS_IP6
	char            localip[16];
	unsigned int    scope_id;
#else
	char            localip[4];
#endif
	char            qtype[2];
};
struct dns_transmit dns_resolve_tx;

void            dns_random_init(char *);
unsigned int    dns_random(unsigned int);
void            dns_sortip(char *, unsigned int);
void            dns_domain_free(char **);
int             dns_domain_copy(char **, char *);
unsigned int    dns_domain_length(char *);
int             dns_domain_equal(char *, char *);
int             dns_domain_suffix(char *, char *);
int             dns_domain_fromdot(char **, char *, unsigned int);
int             dns_domain_todot_cat(stralloc *, char *);
unsigned int    dns_packet_copy(char *, unsigned int, unsigned int, char *, unsigned int);
unsigned int    dns_packet_getname(char *, unsigned int, unsigned int, char **);
unsigned int    dns_packet_skipname(char *, unsigned int, unsigned int);
int             dns_transmit_start(struct dns_transmit *, char *, int, char *, char *, unsigned char *);
void            dns_transmit_free(struct dns_transmit *);
void            dns_transmit_io(struct dns_transmit *, iopause_fd *, struct taia *);
int             dns_transmit_get(struct dns_transmit *, iopause_fd *, struct taia *);
int             dns_resolvconfip(char *);
int             dns_resolve(char *, char *);
int             dns_ip4_packet(stralloc *, char *, unsigned int);
int             dns_ip4(stralloc *, stralloc *);
int             dns_name_packet(stralloc *, char *, unsigned int);
void            dns_name4_domain(char *, char *);
int             dns_name4(stralloc *, char *);
int             dns_txt_packet(stralloc *, char *, unsigned int);
int             dns_txt(stralloc *, stralloc *);
int             dns_mx_packet(stralloc *, char *, unsigned int);
int             dns_mx(stralloc *, stralloc *);
int             dns_resolvconfrewrite(stralloc *);
int             dns_ip4_qualify_rules(stralloc *, stralloc *, stralloc *, stralloc *);
int             dns_ip4_qualify(stralloc *, stralloc *, stralloc *);
int             dns_packet_nameequal(char *, unsigned int, unsigned int, char *, unsigned int, unsigned int);

#ifdef LIBC_HAS_IP6
int             dns_name6(stralloc *, char *);
int             dns_name6_domain(char *, char *);
void            dns_sortip6(char *, unsigned int);
unsigned int    dns_domain_suffixpos(char *, char *);
int             dns_ip6_packet(stralloc *, char *, unsigned int);
int             dns_ip6(stralloc *, stralloc *);
int             dns_ip6_qualify_rules(stralloc *, stralloc *, stralloc *, stralloc *);
int             dns_ip6_qualify(stralloc *, stralloc *, stralloc *);
#endif
#endif
