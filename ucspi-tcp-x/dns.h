/*
 * $Log: dns.h,v $
 * Revision 1.9  2021-05-12 20:59:50+05:30  Cprogrammer
 * define arguments as array subscripts to fix gcc 11 warnings
 *
 * Revision 1.8  2020-04-30 22:04:56+05:30  Cprogrammer
 * define dns_resolve_tx as extern in dns_resolve.c
 *
 * Revision 1.7  2020-04-30 17:59:29+05:30  Cprogrammer
 * define function prototypes as extern
 *
 * Revision 1.6  2017-03-30 22:43:48+05:30  Cprogrammer
 * renamed dns_txt() to rbl_dns_txt() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
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

extern struct dns_transmit dns_resolve_tx;

extern void     dns_random_init(char data[128]);
extern unsigned int dns_random(unsigned int);
extern void     dns_sortip(char *, unsigned int);
extern void     dns_domain_free(char **);
extern int      dns_domain_copy(char **, char *);
extern unsigned int dns_domain_length(char *);
extern int      dns_domain_equal(char *, char *);
extern int      dns_domain_suffix(char *, char *);
extern int      dns_domain_fromdot(char **, char *, unsigned int);
extern int      dns_domain_todot_cat(stralloc *, char *);
extern unsigned int dns_packet_copy(char *, unsigned int, unsigned int, char *, unsigned int);
extern unsigned int dns_packet_getname(char *, unsigned int, unsigned int, char **);
extern unsigned int dns_packet_skipname(char *, unsigned int, unsigned int);
extern int      dns_transmit_start(struct dns_transmit *, char servers[256], int, char *, char qtype[2], unsigned char localip[16]);
extern void     dns_transmit_free(struct dns_transmit *);
extern void     dns_transmit_io(struct dns_transmit *, iopause_fd *, struct taia *);
extern int      dns_transmit_get(struct dns_transmit *, iopause_fd *, struct taia *);
extern int      dns_resolvconfip(char s[256]);
extern int      dns_resolve(char *, char qtype[2]);
extern int      dns_ip4_packet(stralloc *, char *, unsigned int);
extern int      dns_ip4(stralloc *, stralloc *);
extern int      dns_name_packet(stralloc *, char *, unsigned int);
extern void     dns_name4_domain(char name[DNS_NAME4_DOMAIN], char ip[4]);
extern int      dns_name4(stralloc *, char ip[4]);
extern int      dns_txt_packet(stralloc *, char *, unsigned int);
extern int      rbl_dns_txt(stralloc *, stralloc *);
extern int      dns_mx_packet(stralloc *, char *, unsigned int);
extern int      dns_mx(stralloc *, stralloc *);
extern int      dns_resolvconfrewrite(stralloc *);
extern int      dns_ip4_qualify_rules(stralloc *, stralloc *, stralloc *, stralloc *);
extern int      dns_ip4_qualify(stralloc *, stralloc *, stralloc *);
extern int      dns_packet_nameequal(char *, unsigned int, unsigned int, char *, unsigned int, unsigned int);

#ifdef LIBC_HAS_IP6
extern int      dns_name6(stralloc *, char ip[16]);
extern int      dns_name6_domain(char name[DNS_NAME6_DOMAIN], char ip[16]);
extern void     dns_sortip6(char *, unsigned int);
extern unsigned int dns_domain_suffixpos(char *, char *);
extern int      dns_ip6_packet(stralloc *, char *, unsigned int);
extern int      dns_ip6(stralloc *, stralloc *);
extern int      dns_ip6_qualify_rules(stralloc *, stralloc *, stralloc *, stralloc *);
extern int      dns_ip6_qualify(stralloc *, stralloc *, stralloc *);
#endif
#endif
