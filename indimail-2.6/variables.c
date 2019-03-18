/*
 * $Log: variables.c,v $
 * Revision 2.59  2019-03-16 19:27:21+05:30  Cprogrammer
 * removed mailing list table
 *
 * Revision 2.58  2017-11-09 10:15:15+05:30  Cprogrammer
 * fixed paths
 *
 * Revision 2.57  2017-10-12 13:26:24+05:30  Cprogrammer
 * removed indiversion
 *
 * Revision 2.56  2017-03-13 14:11:43+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.55  2017-03-08 13:52:21+05:30  Cprogrammer
 * removed vconvert
 *
 * Revision 2.54  2014-01-02 23:54:50+05:30  Cprogrammer
 * set delayed insert to off for MySQL delayed inserts
 *
 * Revision 2.53  2012-08-03 08:25:51+05:30  Cprogrammer
 * removed duplicate X-Mailer
 *
 * Revision 2.52  2011-05-18 12:54:19+05:30  Cprogrammer
 * added vserverinfo
 *
 * Revision 2.51  2011-03-30 18:57:14+05:30  Cprogrammer
 * removed ncursed based program 'shit' & execmysql program. added svstat, resetquota form
 * admin commands list
 *
 * Revision 2.50  2010-05-01 15:12:24+05:30  Cprogrammer
 * removed editor from list of rfc_ids
 *
 * Revision 2.49  2010-04-13 08:16:52+05:30  Cprogrammer
 * renamed vmoddomlimits to vlimit
 *
 * Revision 2.48  2010-02-16 09:28:47+05:30  Cprogrammer
 * added mailer-daemon to rfc ids
 *
 * Revision 2.47  2009-09-17 09:52:59+05:30  Cprogrammer
 * removed duplicate command entry for ipchange in adminCommands
 *
 * Revision 2.46  2009-08-11 21:29:35+05:30  Cprogrammer
 * added svctool, clearopensmtp, hostsync, inquerytest, ipchange, vmoddomain
 *
 * Revision 2.45  2008-08-02 09:09:39+05:30  Cprogrammer
 * removed verror_str
 *
 * Revision 2.44  2008-07-18 19:08:34+05:30  Cprogrammer
 * renamed getversion_variables_c to getversion_indivariables_c
 *
 * Revision 2.43  2008-07-13 19:49:17+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.42  2008-06-24 21:59:36+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.41  2008-06-13 10:35:49+05:30  Cprogrammer
 * moved mailing_list table under vfilter
 *
 * Revision 2.40  2008-06-13 08:22:17+05:30  Cprogrammer
 * corrections made for conditional compilation of CLUSTERED_SITE, VALIAS, IP_ALIAS_DOMAINS, VFILTER, ENABLE_DOMAIN_LIMITS
 *
 * Revision 2.39  2008-06-05 16:21:52+05:30  Cprogrammer
 * moved execmysql, vdeloldusers, vreorg, updatefile, ipchange to sbin
 *
 * Revision 2.38  2008-05-28 16:38:33+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.37  2008-05-28 15:22:25+05:30  Cprogrammer
 * removed ldap, cdb modules
 *
 * Revision 2.36  2008-05-27 22:35:29+05:30  Cprogrammer
 * new structure for all indimail tables
 *
 * Revision 2.35  2004-06-19 00:23:08+05:30  Cprogrammer
 * renamed vfilter id to prefilt
 *
 * Revision 2.34  2004-05-22 23:48:44+05:30  Cprogrammer
 * added ipchange
 *
 * Revision 2.33  2004-05-17 00:51:02+05:30  Cprogrammer
 * added hostcntrl program
 *
 * Revision 2.32  2004-04-08 13:04:15+05:30  Cprogrammer
 * added postfilt id
 *
 * Revision 2.31  2004-03-23 21:24:19+05:30  Cprogrammer
 * added numerical expressions
 *
 * Revision 2.30  2003-11-03 15:16:23+05:30  Cprogrammer
 * compatibility with indimail-8.0.0
 *
 * Revision 2.29  2003-10-25 01:24:58+05:30  Cprogrammer
 * added vmoddomlimits
 *
 * Revision 2.28  2003-10-03 01:23:23+05:30  Cprogrammer
 * added X-Spam-Rating
 *
 * Revision 2.27  2003-10-01 12:57:38+05:30  Cprogrammer
 * added ids that should not be purged
 *
 * Revision 2.26  2003-09-16 12:34:35+05:30  Cprogrammer
 * added program vpriv
 *
 * Revision 2.25  2003-08-24 16:04:56+05:30  Cprogrammer
 * reordered ldap_fields and vldap_attrs
 *
 * Revision 2.24  2003-07-04 11:35:29+05:30  Cprogrammer
 * added program vatrn
 *
 * Revision 2.23  2003-06-22 10:52:57+05:30  Cprogrammer
 * added updatefile
 *
 * Revision 2.22  2003-06-18 23:42:11+05:30  Cprogrammer
 * added Received header
 *
 * Revision 2.21  2003-05-26 12:58:59+05:30  Cprogrammer
 * added vgroup
 *
 * Revision 2.20  2003-04-12 00:21:18+05:30  Cprogrammer
 * replaced admin_command with structure ADMINCOMMAND
 *
 * Revision 2.19  2003-02-11 23:11:31+05:30  Cprogrammer
 * organized programs according to clustered/non-clustered
 *
 * Revision 2.18  2003-02-02 23:50:02+05:30  Cprogrammer
 * added program vmatrix
 *
 * Revision 2.17  2003-01-26 18:30:08+05:30  Cprogrammer
 * added variable use_vfilter
 *
 * Revision 2.16  2003-01-22 16:02:35+05:30  Cprogrammer
 * added large range of programs
 *
 * Revision 2.15  2002-11-28 00:47:00+05:30  Cprogrammer
 * added variable admin_command - list of indisrvr admin commands
 *
 * Revision 2.14  2002-10-18 23:53:16+05:30  Cprogrammer
 * conditional compilation of Regular Expression
 *
 * Revision 2.13  2002-10-18 14:55:56+05:30  Cprogrammer
 * added headers
 *
 * Revision 2.12  2002-10-18 01:18:12+05:30  Cprogrammer
 * added array for definition of RFC821 ids
 *
 * Revision 2.11  2002-10-14 20:58:27+05:30  Cprogrammer
 * added new comparsion in vfilter_comparision - RegExp for matching using regular expressions
 *
 * Revision 2.10  2002-10-11 01:03:59+05:30  Cprogrammer
 * added h_mailinglist array
 *
 * Revision 2.9  2002-10-10 23:37:33+05:30  Cprogrammer
 * added structure i_header
 *
 * Revision 2.8  2002-10-03 14:00:03+05:30  Cprogrammer
 * added headers
 *
 * Revision 2.7  2002-09-30 16:04:36+05:30  Cprogrammer
 * added VFILTER definitions
 *
 * Revision 2.6  2002-08-30 23:29:31+05:30  Cprogrammer
 * use isopen_vauthinit for checking multiple mysql connections
 *
 * Revision 2.5  2002-08-25 22:35:00+05:30  Cprogrammer
 * new variable for determining autoturn or etrn domains
 *
 * Revision 2.4  2002-07-22 19:39:10+05:30  Cprogrammer
 * added fields servOwner and portCount
 *
 * Revision 2.3  2002-07-09 14:46:45+05:30  Cprogrammer
 * added ldapversion
 *
 * Revision 2.2  2002-07-01 13:32:26+05:30  Cprogrammer
 * added attribute mails
 *
 * Revision 2.1  2002-06-26 03:24:03+05:30  Cprogrammer
 * changes for USE_LDAP_PASSWD implemention
 *
 * Revision 1.14  2002-04-08 03:47:31+05:30  Cprogrammer
 * added variable is_overquota
 *
 * Revision 1.13  2002-03-27 00:08:51+05:30  Cprogrammer
 * added variables MdaMysql, RelayHosts
 *
 * Revision 1.12  2002-03-25 00:36:46+05:30  Cprogrammer
 * removed variable indimailLock
 *
 * Revision 1.11  2002-03-13 11:39:14+05:30  Cprogrammer
 * added variable indimaillock for mysql locking
 *
 * Revision 1.10  2002-02-24 03:26:24+05:30  Cprogrammer
 * added variable CurCount to hold number of mails lying in Maildir
 *
 * Revision 1.9  2001-12-11 11:33:08+05:30  Cprogrammer
 * removed relay_table related variables
 *
 * Revision 1.8  2001-11-30 00:14:37+05:30  Cprogrammer
 * added variables cntrl_table and relay_table
 *
 * Revision 1.7  2001-11-29 20:56:03+05:30  Cprogrammer
 * changes for initiating only one connection for indimail, cntrl and relay if hosts are same
 *
 * Revision 1.6  2001-11-29 13:20:47+05:30  Cprogrammer
 * added variable verbose
 *
 * Revision 1.5  2001-11-24 12:20:44+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.4  2001-11-20 21:48:35+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.3  2001-11-20 10:56:35+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:26:10+05:30  Cprogrammer
 * mysql changed to array to hold two connections to two different
 * mysql databases
 *
 * Revision 1.1  2001-10-24 18:15:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: variables.c,v 2.59 2019-03-16 19:27:21+05:30 Cprogrammer Exp mbhangui $";
#endif

vdir_type       vdir;
char            dirlist[MAX_DIR_LIST] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int             create_flag;
int             site_size = SITE_SIZE;
int             fdm;
int             userNotFound = 0;
int             is_open = 0;
char            mysql_host[MAX_BUFF];
int             encrypt_flag;
char           *indi_port, *default_table, *inactive_table, *cntrl_table;
mdir_t          CurBytes, CurCount;
uid_t           indimailuid = -1;
gid_t           indimailgid = -1;
int             OptimizeAddDomain;
int             is_inactive;
int             is_overquota;
int             verbose = 0;
int             use_etrn;
#ifdef VFILTER
int             use_vfilter;
#endif
char           *rfc_ids[] = {
	"postmaster",
	"abuse",
	"mailer-daemon",
	"prefilt",
	"postfilt",
	"spam",
	"nonspam",
	"register-spam",
	"register-nonspam",
	0
};

/*
 * 0 Hostcntrl Slave/Master
 * 1 Vpopmail
 */
MYSQL           mysql[2];
MYSQL         **MdaMysql;
DBINFO        **RelayHosts;
#ifdef VFILTER
char           *vfilter_comparision[] = {
	"Equals",
	"Contains",
	"Does not contain",
	"Starts with",
	"Ends with",
	"Sender Not in Address Book",
	"My id not in To, CC, Bcc",
	"Numerical Logical Expression",
#ifdef HAVE_FNMATCH
	"RegExp",
#endif
	0
};

char           *vfilter_header[] = {
	"Return-Path",
	"From",
	"Subject",
	"To",
	"Cc",
	"Bcc",
	"Reply-To",
	"Date",
	"Sender",
	"User-Agent",
	"Message-Id",
	"MIME-Version",
	"Content-Type",
	"Content-Transfer-Encoding",
	"Precedence",
	"Organization",
	"Errors-To",
	"List-Id",
	"Mailing-List",
	"X-Sender",
	"X-Mailing-List",
	"X-ML-Name",
	"X-List",
	"X-Loop",
	"X-BeenThere",
	"X-Sequence",
	"X-Mailer",
	"Importance",
	"X-Priority",
	"X-Spam-Status",
	"X-Spam-Rating",
	"Received",
	0
};

char           *i_headers[] = {
	"To",
	"Return-Path",
	"Delivered-To",
	"Cc",
	"Bcc",
	"Reply-To",
	"From",
	"Sender",
	"Reply-To",
	"Errors-To",
	"Disposition-Notification-To",
	0
};

char           *h_mailinglist[] = {
	"Precedence"
	"List-Id"
	"List-Post"
	"List-Help"
	"List-Unsubscribe"
	"List-Subscribe",
	"Mailing-List",
	"X-Mailing-List",
	"X-ML-Name",
	"X-List",
	"X-Loop",
	"X-BeenThere",
	"X-Sequence",
	0
};
#endif
#ifdef CLUSTERED_SITE
int             isopen_cntrl = 0;
int             delayed_insert = 0;
int             isopen_vauthinit[2] = {0, 0};
char            cntrl_host[MAX_BUFF];
char           *cntrl_port;
#endif /*- #ifdef CLUSTERED_SITE */
ADMINCOMMAND adminCommands[] = {
	{PREFIX"/bin/vadduser", "Add a user to a Virtual Domain"},
	{PREFIX"/bin/vpasswd", "Change Password for a Mail User"},
	{PREFIX"/bin/vdeluser", "Delete Mail User from a Virtual Domain"},
	{PREFIX"/bin/vsetuserquota", "Set Filesystem Quota for a Mail user"},
	{PREFIX"/bin/vbulletin", "Send out Bulletins"},
	{PREFIX"/bin/vmoduser", "Modify Mail User Characteristics"},
	{PREFIX"/bin/valias", "Add an Alias"},
	{PREFIX"/bin/vuserinfo", "Mail User Information"},
	{PREFIX"/bin/vipmap", "Add/Modify/Delete IP Maps"},
	{PREFIX"/bin/vacation", "Add Mail Vacation Autoresponder"},
	{PREFIX"/bin/vmoveuser", "Move a user between different file systems"},
	{PREFIX"/bin/vrenameuser", "Rename a user"},
	{PREFIX"/bin/crc", "Calculate Checksums of files/directories"},
	{PREFIX"/bin/vcfilter", "Create Filters"},
	{PREFIX"/bin/vsmtp", "Add/Modify/Delete SMTP Routes"},
	{PREFIX"/bin/dbinfo", "Add/Modify/Delete Mail Control Defination"},
	{PREFIX"/bin/vhostid", "Add/Modify/Delete Host IDs"},
	{PREFIX"/bin/printdir", "Print Mail Hash Directory Info"},
	{PREFIX"/bin/svstat", "Get Service Status for IndiMail Services"},
	{PREFIX"/bin/vaddaliasdomain", "Add an Alias Domain"},
	{PREFIX"/bin/vadddomain", "Add a Virtual Domain"},
	{PREFIX"/bin/vcalias", "Convert .qmail files to valias format"},
	{PREFIX"/bin/vcaliasrev", "Convert Alias to .qmail format"},
	{PREFIX"/bin/vdeldomain", "Delete a Virtual Domain"},
	{PREFIX"/bin/vrenamedomain", "Rename a Virtual Domain"},
	{PREFIX"/bin/vdominfo", "Domain Information"},
	{PREFIX"/bin/vgroup", "Add/Modify/Delete Groups "},
	{PREFIX"/bin/vatrn", "Add ATRN Maps for ODMR"},
	{PREFIX"/bin/vpriv", "Add Privileges to Program for IndiSrvr"},
	{PREFIX"/bin/vlimit", "Administer Domain Wide Limits"},
	{PREFIX"/bin/hostcntrl", "Administer Hostcntrl Entries"},
	{PREFIX"/sbin/resetquota", "Reset/Correct quota for a Maildir"},
	{PREFIX"/sbin/vreorg", "Reorganize Mail Database"},
	{PREFIX"/sbin/vdeloldusers", "Delete Old Mail Users"},
	{PREFIX"/sbin/ipchange", "Change/Update IP Address changes in IndiMail"},
	{PREFIX"/sbin/svctool", "Service Configuration tool"},
	{PREFIX"/sbin/clearopensmtp", "Clear Open SMTP session"},
	{PREFIX"/sbin/hostsync", "Sync Hostcntrl Information"},
	{PREFIX"/sbin/inquerytest", "Test inlookup queries"},
	{PREFIX"/sbin/vmoddomain", "Modify Domain Information"},
	{PREFIX"/sbin/vserverinfo", "Mail Server Information"},
	{PREFIX"/sbin/mgmtpass", "Manage Admin Client Passwords"},
	{PREFIX"/sbin/vfstab", "Add/Modify/Delete Filesystem Balancing for Mail filesystems"},
	{LIBEXECDIR"/updatefile", "Update Control Files"},
	{(char *) NULL, (char *) NULL}
};

IndiMAILTable IndiMailTable[] = {
	{ON_LOCAL, "atrn_map",     ATRN_MAP_LAYOUT},
	{ON_LOCAL, "bulkmail",     BULKMAIL_TABLE_LAYOUT},
	{ON_LOCAL, "fstab",        FSTAB_TABLE_LAYOUT},
#ifdef IP_ALIAS_DOMAINS
	{ON_LOCAL, "ip_alias_map", IP_ALIAS_TABLE_LAYOUT},
#endif
#ifdef ENABLE_AUTH_LOGGING
	{ON_LOCAL, "lastauth",     LASTAUTH_TABLE_LAYOUT},
	{ON_LOCAL, "userquota",    USERQUOTA_TABLE_LAYOUT},
#endif
#ifdef VALIAS
	{ON_LOCAL, "valias",       VALIAS_TABLE_LAYOUT},
#endif
#ifdef VFILTER
	{ON_LOCAL, "vfilter",      FILTER_TABLE_LAYOUT},
#endif
#ifdef ENABLE_DOMAIN_LIMITS
	{ON_LOCAL, "vlimits",      LIMITS_TABLE_LAYOUT},
#endif
#ifdef ENABLE_MYSQL_LOGGING
	{ON_LOCAL, "vlog",         VLOG_TABLE_LAYOUT},
#endif
#ifdef CLUSTERED_SITE
#ifdef VALIAS
	{ON_MASTER, "aliasdomain", ALIASDOMAIN_TABLE_LAYOUT},
#endif
	{ON_MASTER, "dbinfo",      DBINFO_TABLE_LAYOUT},
	{ON_MASTER, "fstab",       FSTAB_TABLE_LAYOUT},
	{ON_MASTER, "host_table",  HOST_TABLE_LAYOUT},
	{ON_MASTER, "mgmtaccess",  MGMT_TABLE_LAYOUT},
	{ON_MASTER, "smtp_port",   SMTP_TABLE_LAYOUT},
	{ON_MASTER, "vpriv",       PRIV_CMD_LAYOUT},
	{ON_MASTER, "spam",        SPAM_TABLE_LAYOUT},
	{ON_MASTER, "badmailfrom", BADMAILFROM_TABLE_LAYOUT},
	{ON_MASTER, "badrcptto",   BADMAILFROM_TABLE_LAYOUT},
	{ON_MASTER, "spamdb",      BADMAILFROM_TABLE_LAYOUT},
#endif
	{0,         (char *) NULL, (char *) NULL}
};

void
getversion_indivariables_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
