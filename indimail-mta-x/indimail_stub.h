/*
 * $Log: indimail_stub.h,v $
 * Revision 1.6  2021-02-07 23:12:58+05:30  Cprogrammer
 * use inquery(), parse_email() from libindimail
 *
 * Revision 1.5  2019-04-20 19:50:44+05:30  Cprogrammer
 * changed interface for loadLibrary(), closeLibrary() and getlibObject()
 *
 * Revision 1.4  2019-04-17 00:00:23+05:30  Cprogrammer
 * added parse_email() function
 *
 * Revision 1.3  2018-07-01 11:49:21+05:30  Cprogrammer
 * renamed getFunction() to getlibObject()
 *
 * Revision 1.2  2018-03-24 23:34:45+05:30  Cprogrammer
 * removed hardcoded definition of VIRTUAL_PKG_LIB
 *
 * Revision 1.1  2018-01-10 11:30:04+05:30  Cprogrammer
 * Initial revision
 *
 */
/* gid flags */
#define NO_PASSWD_CHNG 0x01
#define NO_POP         0x02
#define NO_WEBMAIL     0x04
#define NO_IMAP        0x08
#define BOUNCE_MAIL    0x10
#define NO_RELAY       0x20
#define NO_DIALUP      0x40
#define QA_ADMIN       0x80
#define V_OVERRIDE     0x100
#define NO_SMTP        0x200
#define V_USER0        0x400
#define V_USER1        0x800
#define V_USER2        0x1000
#define V_USER3        0x2000

/*- inquery types */
#define USER_QUERY     1
#define RELAY_QUERY    2
#define PWD_QUERY      3
#define HOST_QUERY     4
#define ALIAS_QUERY    5
#define LIMIT_QUERY    6
#define DOMAIN_QUERY   7
#define MAX_BUFF       300
#define CNTRL_HOST     "localhost"
#define MASTER_HOST    "localhost"
#define MYSQL_HOST     "localhost"

void           *loadLibrary(void **, const char *, int *, const char *err[]);
void           *getlibObject(const char *, void **, const char *, const char *err[]);
void            closeLibrary(void **);
