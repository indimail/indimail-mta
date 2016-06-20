/*
 * $Log: hier.c,v $
 * Revision 1.219  2016-06-20 08:31:49+05:30  Cprogrammer
 * added man pages for mbox2maildir, qmail-popbull
 *
 * Revision 1.218  2016-06-17 20:15:39+05:30  Cprogrammer
 * moved qmailanalog scripts to libexec dir
 *
 * Revision 1.217  2016-06-17 18:29:01+05:30  Cprogrammer
 * FHS compliance - moved internal binaries to libexec dir
 *
 * Revision 1.216  2016-06-17 17:20:42+05:30  Cprogrammer
 * added conf-libexec, auto_libexec variable for libexecdir (internal binaries)
 *
 * Revision 1.215  2016-06-17 09:30:47+05:30  Cprogrammer
 * permissions fixes for rpmlint
 *
 * Revision 1.214  2016-06-15 19:21:14+05:30  Cprogrammer
 * removed old documentation
 *
 * Revision 1.213  2016-06-15 11:58:04+05:30  Cprogrammer
 * removed qmail-scanner-queue.pl, stripmime.pl
 *
 * Revision 1.212  2016-06-14 09:11:20+05:30  Cprogrammer
 * added README.licenses
 *
 * Revision 1.211  2016-06-14 09:06:42+05:30  Cprogrammer
 * aded GPLv3 license
 *
 * Revision 1.210  2016-06-09 14:15:04+05:30  Cprogrammer
 * relaxed permissions on few executables
 *
 * Revision 1.209  2016-06-09 12:59:27+05:30  Cprogrammer
 * renamed rmail, sendmail, newaliases to fix problems with setting mta alternatives
 *
 * Revision 1.208  2016-06-08 10:47:38+05:30  Cprogrammer
 * fix permissions for control control/domainkeys etc/indimail/users control/inquery
 *
 * Revision 1.207  2016-06-07 10:56:01+05:30  Cprogrammer
 * moved error.3 to qerror.3 to avoid conflict with GNU error(3) extension
 *
 * Revision 1.206  2016-06-06 17:05:24+05:30  Cprogrammer
 * added indimail-mta.fc
 *
 * Revision 1.205  2016-06-05 13:18:38+05:30  Cprogrammer
 * moved relaytest, splogger, dns*, qmail-qmqpc, qmail-tcpto, qmail-tcpok, qmail-queue, qmail-nullqueue, qmail-multi to sbin
 *
 * Revision 1.204  2016-06-03 13:33:18+05:30  Cprogrammer
 * moved qmail-daemon, qmail-start, qmail-lspawn, qmail-rspawn, qmail-clean, qmail-send, qmail-todo to sbin
 *
 * Revision 1.203  2016-06-03 09:56:37+05:30  Cprogrammer
 * moved non-user programs to sbin
 *
 * Revision 1.202  2016-06-02 19:04:09+05:30  Cprogrammer
 * moved whois to sbin
 *
 * Revision 1.201  2016-06-02 18:05:38+05:30  Cprogrammer
 * added selinux policy file indimail-mta.te
 *
 * Revision 1.200  2016-06-02 17:40:36+05:30  Cprogrammer
 * renamed base64 to qbase64
 *
 * Revision 1.199  2016-05-30 20:24:49+05:30  Cprogrammer
 * removed _hier()
 *
 * Revision 1.198  2016-05-29 20:04:40+05:30  Cprogrammer
 * moved instcheck to sbin
 *
 * Revision 1.197  2016-05-27 20:47:07+05:30  Cprogrammer
 * FHS compliance
 *
 * Revision 1.196  2016-05-17 23:13:31+05:30  Cprogrammer
 * fixed permission of ratelimit directory
 *
 * Revision 1.195  2016-05-15 22:45:15+05:30  Cprogrammer
 * renamed qmail-smtpd.so to qmail_smtpd.so
 *
 * Revision 1.194  2016-04-05 01:17:16+05:30  Cprogrammer
 * added control/ratelimit directory
 *
 * Revision 1.193  2016-04-03 18:09:24+05:30  Cprogrammer
 * copy qmail-smtpd.so only if LOAD_SHARED_OBJECTS is defined
 *
 * Revision 1.192  2016-02-08 21:37:44+05:30  Cprogrammer
 * added qmail-smtpd.so
 *
 * Revision 1.191  2016-01-28 23:56:29+05:30  Cprogrammer
 * added yearcal, nowutc
 *
 * Revision 1.190  2016-01-28 10:28:17+05:30  Cprogrammer
 * fixed installation of leapsecs.txt, leapsecs.dat
 *
 * Revision 1.189  2016-01-28 09:03:53+05:30  Cprogrammer
 * install leapsecs.txt in etc
 *
 * Revision 1.188  2016-01-28 09:00:04+05:30  Cprogrammer
 * added leapsecs program and leapsecs.dat
 *
 * Revision 1.187  2016-01-02 19:22:18+05:30  Cprogrammer
 * organized as per package daemontools, ucspi-tcp, etc
 *
 * Revision 1.186  2015-08-19 16:38:01+05:30  Cprogrammer
 * added whois program
 *
 * Revision 1.185  2015-04-10 19:41:41+05:30  Cprogrammer
 * added udplogger, udpclient
 *
 * Revision 1.184  2014-06-13 17:58:01+05:30  Cprogrammer
 * removed qmail-queue-print
 *
 * Revision 1.183  2014-05-28 16:06:35+05:30  Cprogrammer
 * removed /var/indimail/queue directory
 *
 * Revision 1.182  2014-04-17 12:01:30+05:30  Cprogrammer
 * ignore man pages error
 *
 * Revision 1.181  2014-01-22 20:37:12+05:30  Cprogrammer
 * added hassrs.h
 *
 * Revision 1.180  2013-12-05 18:09:02+05:30  Cprogrammer
 * added qaes
 *
 * Revision 1.179  2013-09-03 22:58:04+05:30  Cprogrammer
 * added drate man page and cidr
 *
 * Revision 1.178  2013-08-29 18:27:49+05:30  Cprogrammer
 * added drate
 *
 * Revision 1.177  2013-08-25 19:44:22+05:30  Cprogrammer
 * added README.srs
 *
 * Revision 1.176  2013-08-25 18:38:34+05:30  Cprogrammer
 * added srsfilter
 *
 * Revision 1.175  2013-05-07 16:51:35+05:30  Cprogrammer
 * skip qmail-sql for non-indimail installation
 *
 * Revision 1.174  2012-11-24 08:20:16+05:30  Cprogrammer
 * added rrt
 *
 * Revision 1.173  2011-11-27 12:00:43+05:30  Cprogrammer
 * added qnotify
 *
 * Revision 1.172  2011-07-29 09:28:25+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.171  2011-07-21 13:17:19+05:30  Cprogrammer
 * added service script for systemd
 *
 * Revision 1.170  2011-07-15 11:49:35+05:30  Cprogrammer
 * added surblqueue
 *
 * Revision 1.169  2011-07-14 14:45:17+05:30  Cprogrammer
 * added README.surbl
 *
 * Revision 1.168  2011-07-13 22:02:40+05:30  Cprogrammer
 * added surblfilter
 *
 * Revision 1.167  2011-07-08 13:47:06+05:30  Cprogrammer
 * added dnsbl plugin
 *
 * Revision 1.166  2011-06-18 11:42:39+05:30  Cprogrammer
 * added README.filters, README.indimail
 *
 * Revision 1.165  2011-06-04 14:03:45+05:30  Cprogrammer
 * removed dkim.8 man page
 *
 * Revision 1.164  2011-05-17 21:15:46+05:30  Cprogrammer
 * added man page for qmail-nullqueue
 *
 * Revision 1.163  2011-04-23 11:12:19+05:30  Cprogrammer
 * rearranged binaries
 *
 * Revision 1.162  2011-04-18 22:22:38+05:30  Cprogrammer
 * added plugtest
 *
 * Revision 1.161  2011-04-13 22:11:50+05:30  Cprogrammer
 * added plugin_init man page
 *
 * Revision 1.160  2011-04-13 19:43:22+05:30  Cprogrammer
 * added smtpd-plugin.so plugin
 *
 * Revision 1.159  2011-02-26 11:11:18+05:30  Cprogrammer
 * added man page for qmail-cat
 *
 * Revision 1.158  2011-02-12 16:04:27+05:30  Cprogrammer
 * added qarf man page
 *
 * Revision 1.157  2011-02-11 23:41:37+05:30  Cprogrammer
 * added qarf
 *
 * Revision 1.156  2011-01-29 22:20:47+05:30  Cprogrammer
 * changes for courier-imap-4.8.2
 *
 * Revision 1.155  2011-01-11 10:28:03+05:30  Cprogrammer
 * added balance_outgoing
 *
 * Revision 1.154  2010-08-08 22:29:09+05:30  Cprogrammer
 * removed shared libs from checks
 *
 * Revision 1.153  2010-07-14 15:57:56+05:30  Cprogrammer
 * fixed libindimail-1.7.8
 *
 * Revision 1.152  2010-07-14 15:37:04+05:30  Cprogrammer
 * commented out repo files for instcheck
 *
 * Revision 1.151  2010-07-14 15:11:54+05:30  Cprogrammer
 * removed obsolete files
 *
 * Revision 1.150  2010-07-10 10:25:55+05:30  Cprogrammer
 * added portable echo
 *
 * Revision 1.149  2010-07-08 11:21:53+05:30  Cprogrammer
 * added all indimail programs to fix debian stupidity
 *
 * Revision 1.148  2010-06-19 16:49:00+05:30  Cprogrammer
 * added rmail man page
 *
 * Revision 1.147  2010-06-19 15:52:11+05:30  Cprogrammer
 * Dummy UUCP rmail command for postfix/qmail systems
 *
 * Revision 1.146  2010-06-11 16:30:03+05:30  Cprogrammer
 * added sendmail man page
 *
 * Revision 1.145  2010-05-20 11:28:29+05:30  Cprogrammer
 * removed qmail-qstat, qmail-qmHandle
 *
 * Revision 1.144  2010-05-19 18:56:38+05:30  Cprogrammer
 * moved qmail-rm man page to section 1
 *
 * Revision 1.143  2010-05-04 10:00:40+05:30  Cprogrammer
 * added cdbgetm
 *
 * Revision 1.142  2010-04-22 15:20:02+05:30  Cprogrammer
 * added checkpassword modules sys-checkpwd and ldap-checkpwd
 *
 * Revision 1.141  2010-04-18 16:26:26+05:30  Cprogrammer
 * added qmail-sql
 *
 * Revision 1.140  2010-04-16 09:08:51+05:30  Cprogrammer
 * added qmtp man page
 *
 * Revision 1.139  2010-04-02 17:46:41+05:30  Cprogrammer
 * added swaks
 *
 * Revision 1.138  2010-03-26 08:37:19+05:30  Cprogrammer
 * added dkim man page
 *
 * Revision 1.137  2010-03-09 14:40:47+05:30  Cprogrammer
 * added cdb man pages
 * added man pages for predate, datemail, logselect
 * removed envconf
 *
 * Revision 1.136  2010-03-08 22:04:30+05:30  Cprogrammer
 * renamed qmail-autoresponder as autoresponder to shorten path
 *
 * Revision 1.135  2010-03-08 15:38:50+05:30  Cprogrammer
 * added qmailctl man page
 *
 * Revision 1.134  2010-03-03 11:13:00+05:30  Cprogrammer
 * added base64 man page
 *
 * Revision 1.133  2010-03-03 11:01:46+05:30  Cprogrammer
 * merged base64 encoding/decoding in one utility - base64
 *
 * Revision 1.132  2010-02-28 11:44:26+05:30  Cprogrammer
 * added directory etc
 *
 * Revision 1.131  2010-02-25 10:41:32+05:30  Cprogrammer
 * added man page for uacl
 *
 * Revision 1.130  2010-01-21 08:57:23+05:30  Cprogrammer
 * added program uacl
 *
 * Revision 1.129  2009-11-28 20:46:04+05:30  Cprogrammer
 * renamed config to qmailconfig
 * added hostname command
 *
 * Revision 1.128  2009-11-11 09:31:14+05:30  Cprogrammer
 * added queue-fix man page
 *
 * Revision 1.127  2009-10-19 10:13:04+05:30  Cprogrammer
 * added qmail-daemon.8
 *
 * Revision 1.126  2009-09-08 12:33:23+05:30  Cprogrammer
 * compilation of control/domainkeys to be default
 *
 * Revision 1.125  2009-09-01 21:58:56+05:30  Cprogrammer
 * added batv tester
 *
 * Revision 1.124  2009-08-30 17:32:21+05:30  Cprogrammer
 * added greydaemon
 *
 * Revision 1.123  2009-08-29 20:47:15+05:30  Cprogrammer
 * removed greydaemon
 *
 * Revision 1.122  2009-08-29 15:25:52+05:30  Cprogrammer
 * added qmail-greyd
 *
 * Revision 1.121  2009-08-24 11:05:49+05:30  Cprogrammer
 * added README.greylist
 *
 * Revision 1.120  2009-08-23 21:10:42+05:30  Cprogrammer
 * added greydaemon
 *
 * Revision 1.119  2009-08-05 14:46:58+05:30  Cprogrammer
 * added qmail-poppass
 *
 * Revision 1.118  2009-06-25 12:39:08+05:30  Cprogrammer
 * renamed maildirmake to qmaildirmake to avoid conflict with courier-imap's maildirmake
 *
 * Revision 1.117  2009-06-17 14:14:18+05:30  Cprogrammer
 * added StartupParameters.plist and indimail.plist for MacOS startup
 *
 * Revision 1.116  2009-04-29 11:21:05+05:30  Cprogrammer
 * renamed qmail-spamdb as qmail-cdb
 *
 * Revision 1.115  2009-04-24 23:49:45+05:30  Cprogrammer
 * added qmail-qfilter
 *
 * Revision 1.114  2009-04-23 13:48:08+05:30  Cprogrammer
 * removed qmail-qfilter
 *
 * Revision 1.113  2009-04-22 15:23:39+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.112  2009-04-20 10:06:43+05:30  Cprogrammer
 * added dk-filter
 *
 * Revision 1.111  2009-04-19 13:39:39+05:30  Cprogrammer
 * removed echo program
 *
 * Revision 1.110  2009-04-10 20:24:17+05:30  Cprogrammer
 * added all README files
 *
 * Revision 1.109  2009-04-07 20:17:05+05:30  Cprogrammer
 * added domainkeys directory
 *
 * Revision 1.108  2009-04-06 16:38:51+05:30  Cprogrammer
 * removed function cd()
 *
 * Revision 1.107  2009-04-03 13:42:20+05:30  Cprogrammer
 * changed permission of qmail-dk, qmail-dkim to 555
 *
 * Revision 1.106  2009-04-02 20:35:22+05:30  Cprogrammer
 * added qmail-qfilter
 *
 * Revision 1.105  2009-04-02 14:56:09+05:30  Cprogrammer
 * added dk-filter
 *
 * Revision 1.104  2009-03-30 22:25:36+05:30  Cprogrammer
 * added rfc-4871.5
 *
 * Revision 1.103  2009-03-27 16:48:16+05:30  Cprogrammer
 * added qmail-dkim man page
 *
 * Revision 1.102  2009-03-19 10:09:39+05:30  Cprogrammer
 * added qmail-dkim
 *
 * Revision 1.101  2009-03-09 15:49:24+05:30  Cprogrammer
 * renamed FAQ to QMAILFAQ
 *
 * Revision 1.100  2009-02-21 15:05:31+05:30  Cprogrammer
 * added maildirmake
 *
 * Revision 1.99  2009-02-15 17:24:22+05:30  Cprogrammer
 * changed perm for upstart to 444
 *
 * Revision 1.98  2009-02-10 09:29:14+05:30  Cprogrammer
 * changed perm of domain direcotory to 775
 *
 * Revision 1.97  2009-02-08 10:08:25+05:30  Cprogrammer
 * changed owner of qmail-dk to root
 *
 * Revision 1.96  2009-02-01 00:00:23+05:30  Cprogrammer
 * added rpmattr
 *
 * Revision 1.95  2009-01-26 15:18:14+05:30  Cprogrammer
 * moved upstart to boot
 *
 * Revision 1.94  2008-09-16 08:25:09+05:30  Cprogrammer
 * added cdbmake package
 *
 * Revision 1.93  2008-07-27 15:26:39+05:30  Cprogrammer
 * renamed svscan.upstart to upstart
 *
 * Revision 1.92  2008-07-25 16:50:44+05:30  Cprogrammer
 * removed qbiff
 *
 * Revision 1.91  2008-06-25 20:38:04+05:30  Cprogrammer
 * added upstart
 *
 * Revision 1.90  2008-06-17 11:34:52+05:30  Cprogrammer
 * removed duplicate update_tmprsadh
 *
 * Revision 1.89  2008-06-06 14:32:12+05:30  Cprogrammer
 * added mbox2maildir
 *
 * Revision 1.88  2008-06-06 10:24:30+05:30  Cprogrammer
 * added rfc-1845 man page
 *
 * Revision 1.87  2008-06-05 10:57:07+05:30  Cprogrammer
 * moved to more secure permissions
 *
 * Revision 1.86  2008-06-01 15:31:28+05:30  Cprogrammer
 * added logselect
 *
 * Revision 1.85  2008-05-27 19:31:57+05:30  Cprogrammer
 * added instcheck binary to distribution
 *
 * Revision 1.84  2008-05-26 22:22:02+05:30  Cprogrammer
 * added commandline argument for configurable qmail home
 * moved documents to doc
 *
 * Revision 1.83  2008-05-22 19:46:34+05:30  Cprogrammer
 * added auto_uido, auto_gidq perms
 *
 * Revision 1.82  2008-02-05 15:28:48+05:30  Cprogrammer
 * added mlmatchup
 *
 * Revision 1.81  2007-12-20 12:44:25+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.80  2005-07-04 18:04:48+05:30  Cprogrammer
 * bug fix for TCPTO_BUFSIZ
 *
 * Revision 1.79  2005-06-29 20:53:49+05:30  Cprogrammer
 * Size of tcpto changed to TCPTO_BUFSIZ
 *
 * Revision 1.78  2005-05-31 15:43:36+05:30  Cprogrammer
 * added update_tmprsadh
 *
 * Revision 1.77  2005-04-26 23:31:54+05:30  Cprogrammer
 * added plugins directory
 *
 * Revision 1.76  2005-04-24 22:30:55+05:30  Cprogrammer
 * added qhpsi executable
 *
 * Revision 1.75  2005-04-05 12:03:23+05:30  Cprogrammer
 * added relaytest
 *
 * Revision 1.74  2005-03-28 09:38:38+05:30  Cprogrammer
 * added qmail-internals
 *
 * Revision 1.73  2005-03-03 14:35:58+05:30  Cprogrammer
 * added stripmime
 *
 * Revision 1.72  2005-01-22 00:38:29+05:30  Cprogrammer
 * removed 822headers
 *
 * Revision 1.71  2004-10-29 00:09:27+05:30  Cprogrammer
 * added domain-keys man page
 *
 * Revision 1.70  2004-10-27 17:52:00+05:30  Cprogrammer
 * added rfc 3798
 *
 * Revision 1.69  2004-10-24 22:25:59+05:30  Cprogrammer
 * added recordio, argv0, addcr, delcr,fixcrio
 *
 * Revision 1.68  2004-10-22 20:25:44+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.67  2004-10-22 14:42:10+05:30  Cprogrammer
 * removed man pages for ucspi
 *
 * Revision 1.66  2004-10-20 20:03:07+05:30  Cprogrammer
 * install qmail-dk if DOMAIN_KEYS is defined
 *
 * Revision 1.65  2004-10-09 23:23:41+05:30  Cprogrammer
 * added daemontools
 *
 * Revision 1.64  2004-09-29 23:18:09+05:30  Cprogrammer
 * added dknewkey and dktest man pages
 *
 * Revision 1.63  2004-09-27 15:30:45+05:30  Cprogrammer
 * added substdio man pages
 *
 * Revision 1.62  2004-09-23 21:02:40+05:30  Cprogrammer
 * changed group permission of qscanq to qscand
 *
 * Revision 1.61  2004-09-21 23:46:07+05:30  Cprogrammer
 * added recipient extension
 *
 * Revision 1.60  2004-09-19 22:47:29+05:30  Cprogrammer
 * added qscanq virus scanner
 *
 * Revision 1.59  2004-08-28 01:05:58+05:30  Cprogrammer
 * added qmail-dk
 *
 * Revision 1.58  2004-08-25 13:24:09+05:30  Cprogrammer
 * added RFC 2476
 *
 * Revision 1.57  2004-08-15 20:22:47+05:30  Cprogrammer
 * added dktest, dknewkey spfquery dnstxt
 *
 * Revision 1.56  2004-07-30 12:52:26+05:30  Cprogrammer
 * added update_tmprsadh for TLS
 *
 * Revision 1.55  2004-07-27 22:57:56+05:30  Cprogrammer
 * added qlogtools
 *
 * Revision 1.54  2004-06-18 22:41:12+05:30  Cprogrammer
 * added qtools
 *
 * Revision 1.53  2004-06-16 01:20:42+05:30  Cprogrammer
 * added mess822
 *
 * Revision 1.52  2004-05-14 00:45:47+05:30  Cprogrammer
 * added serialmail
 *
 * Revision 1.51  2004-02-13 14:49:29+05:30  Cprogrammer
 * added rspamstat, rspamhist
 *
 * Revision 1.50  2004-01-08 00:30:30+05:30  Cprogrammer
 * added qmail-cat, rfc-2505, rfc-2635, spam stats scripts
 *
 * Revision 1.49  2004-01-05 14:03:10+05:30  Cprogrammer
 * added smtp-matchup
 *
 * Revision 1.48  2004-01-04 23:15:32+05:30  Cprogrammer
 * *** empty log message ***
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include "stralloc.h"
#include "strerr.h"
#include "auto_qmail.h"
#include "auto_control.h"
#include "auto_assign.h"
#include "auto_shared.h"
#include "auto_sysconfdir.h"
#include "auto_prefix.h"
#include "auto_libexec.h"
#include "auto_uids.h"
#include "qmail-todo.h"
#include "fmt.h"
#include "fifo.h"
#include "tcpto.h"
#include "hasindimail.h"
#include "hassrs.h"

void            d(char *, char *, int, int, int);
void            h(char *, int, int, int);
void            l(char *, char *, char *, int);
void            c(char *, char *, char *, int, int, int);
char           *getdirname(char *, char **);

stralloc        a1 = { 0 };
stralloc        a2 = { 0 };
stralloc        a3 = { 0 };
stralloc        a4 = { 0 };
stralloc        a5 = { 0 };
char            buf[100 + FMT_ULONG];
extern int      lsb;

static int
str_diff(s, t)
	register char  *s;
	register char  *t;
{
	register char   x;

	for (;;)
	{
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
		x = *s;
		if (x != *t)
			break;
		if (!x)
			break;
		++s;
		++t;
	}
	return ((int) (unsigned int) (unsigned char) x) - ((int) (unsigned int) (unsigned char) *t);
}

void
hier(inst_dir, fatal)
	char           *inst_dir, *fatal;
{
	char           *auto_cntrl_base, *auto_cntrl_dir, *auto_assgn_base, *auto_assgn_dir;
	char           *auto_libexec_base, *auto_libexec_dir, *auto_qmail_home = auto_qmail;
	char           *mandir;
	mode_t          moder_d, moder_f, moder_x, moder_s, moder_t, moder_u;
	uid_t           uidr;
	gid_t           gidr;

	if (inst_dir && *inst_dir)
		auto_qmail_home = inst_dir;
	/* Directories */
	if (!str_diff(auto_qmail, "/var/indimail") || !str_diff(auto_qmail, "/var/qmail")) {
		h(auto_qmail_home, auto_uido, auto_gidq, 0555);
	}

	auto_cntrl_dir = getdirname(auto_control, &auto_cntrl_base);
	if (!stralloc_copys(&a1, auto_cntrl_dir))
		strerr_die2sys(111, fatal, "out of memory: ");
	if (!stralloc_0(&a1))
		strerr_die2sys(111, fatal, "out of memory: ");
	auto_cntrl_dir = a1.s;
	if (!str_diff(auto_qmail, auto_cntrl_dir))
		d(auto_qmail_home, "control", auto_uidv, auto_gidq, 0775);
	else {
		h(auto_cntrl_dir, 0, 0, 0755);
		d(auto_cntrl_dir, "control", auto_uidv, auto_gidq, 0775);
		/*- l(auto_qmail_home, "control", auto_cntrl_dir); -*/
		l(auto_qmail_home, "control", auto_control, 0);
	}

	auto_assgn_dir = getdirname(auto_assign, &auto_assgn_base);
	if (!stralloc_copys(&a2, auto_assgn_dir))
		strerr_die2sys(111, fatal, "out of memory: ");
	if (!stralloc_0(&a2))
		strerr_die2sys(111, fatal, "out of memory: ");
	auto_assgn_dir = a2.s;
	if (!str_diff(auto_qmail, auto_assgn_dir))
		d(auto_qmail_home, "users", auto_uidv, auto_gidq, 0775);
	else {
		h(auto_assgn_dir, 0, 0, 0755);
		d(auto_assgn_dir, "users", auto_uidv, auto_gidq, 0775);
		/*- l(auto_qmail_home, "users", auto_assgn_dir); -*/
		l(auto_qmail_home, "users", auto_assign, 0);
	}

	/*- shared directory for boot, doc, man */
	if (str_diff(auto_qmail, auto_shared)) {
		mandir = getdirname(auto_shared, 0);
		if (!stralloc_copys(&a3, mandir))
			strerr_die2sys(111, fatal, "out of memory: ");
		if (!stralloc_0(&a3))
			strerr_die2sys(111, fatal, "out of memory: ");
		mandir = a3.s;
		h(auto_shared, auto_uido, auto_gidq, 0555);
	} else
		mandir = auto_qmail_home;

	/*- libexecdir directory for internal binaries */
	auto_libexec_dir = getdirname(auto_libexec, &auto_libexec_base);
	if (!stralloc_copys(&a4, auto_libexec_dir))
		strerr_die2sys(111, fatal, "out of memory: ");
	if (!stralloc_0(&a4))
		strerr_die2sys(111, fatal, "out of memory: ");
	if (!stralloc_copys(&a5, auto_libexec_base + 1))
		strerr_die2sys(111, fatal, "out of memory: ");
	if (!stralloc_0(&a5))
		strerr_die2sys(111, fatal, "out of memory: ");
	auto_libexec_dir = a4.s;
	auto_libexec_base = a5.s;
	if (!str_diff(auto_qmail, auto_libexec)) {
		d(auto_qmail_home, auto_libexec_base, auto_uidv, auto_gidq, 0555);
	} else {
		h(auto_libexec, 0, 0, 0555);
		l(auto_qmail_home, "libexec", auto_libexec, 0);
	}

	/*- sysconf directory for control, config files */
	if (str_diff(auto_qmail, auto_sysconfdir))
		h(auto_sysconfdir, 0, 0, 0755);

	if (str_diff(auto_qmail, auto_prefix)) {
		uidr = 0;
		gidr = 0;
		moder_d = 0755;
		moder_f = 0644;
		moder_x = 0755;
		moder_s = 0755;
		moder_t = 0751;
		moder_u = 0755;
		if (access(auto_prefix, F_OK))
			h(auto_prefix, uidr, gidr, 0755);
		l(auto_qmail_home, "bin", auto_prefix, 1);
		l(auto_qmail_home, "sbin", auto_prefix, 1);
	} else {
		uidr = auto_uido;
		gidr = auto_gidq;
		moder_d = 0555;
		moder_f = 0444;
		moder_x = 0555;
		moder_s = 0555;
		moder_t = 0550;
		moder_u = 0551;
	}
	d(auto_prefix,     "bin", uidr, gidr, 0555);
	d(auto_prefix,     "sbin", uidr, gidr, 0555);
	d(auto_qmail_home, "domains", auto_uido, auto_gidv, 0775);
	d(auto_sysconfdir, "etc", auto_uido, auto_gidq, 0775);
	d(auto_qmail_home, "qscanq", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "qscanq/root", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "qscanq/root/scanq", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "alias", auto_uida, auto_gidq, 02775);
	d(auto_qmail_home, "autoturn", auto_uidv, auto_gidq, 02775);
	d(auto_cntrl_dir,  "control/domainkeys", auto_uidv, auto_gidq, 0775);
	d(auto_cntrl_dir,  "control/ratelimit", auto_uidr, auto_gidq, 02775);
	d(auto_cntrl_dir,  "control/defaultqueue", auto_uidv, auto_gidq, 0775);
#ifdef INDIMAIL
	d(auto_cntrl_dir,  "control/inquery", auto_uidv, auto_gidq, 0775);
#endif
	d(auto_shared,     "boot", auto_uido, auto_gidq, 0555);
	d(auto_shared,     "doc", auto_uido, auto_gidq, 0555);
	d(mandir,          "man", uidr, gidr, moder_d);
	d(mandir,          "man/cat1", uidr, gidr, moder_d);
	d(mandir,          "man/cat5", uidr, gidr, moder_d);
	d(mandir,          "man/cat7", uidr, gidr, moder_d);
	d(mandir,          "man/cat8", uidr, gidr, moder_d);
	d(mandir,          "man/man1", uidr, gidr, moder_d);
	d(mandir,          "man/man3", uidr, gidr, moder_d);
	d(mandir,          "man/man5", uidr, gidr, moder_d);
	d(mandir,          "man/man7", uidr, gidr, moder_d);
	d(mandir,          "man/man8", uidr, gidr, moder_d);

	/* Boot files */
	c(auto_shared,     "boot", "home", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "home+df", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "proc", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "proc+df", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "binm1", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "binm1+df", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "binm2", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "binm2+df", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "binm3", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "binm3+df", auto_uido, auto_gidq, 0755);
	c(auto_shared,     "boot", "upstart", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "boot", "systemd", auto_uido, auto_gidq, 0444);
#ifdef DARWIN
	c(auto_shared,     "boot", "StartupParameters.plist", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "boot", "indimail.plist", auto_uido, auto_gidq, 0444);
#endif
	c(auto_sysconfdir, "etc", "leapsecs.dat", auto_uido, auto_gidq, 0644);
	c(auto_sysconfdir, "etc", "leapsecs.txt", auto_uido, auto_gidq, 0644);
#ifndef INDIMAIL
	c(auto_sysconfdir, "etc/..", "indimail-mta.te", auto_uido, auto_gidq, 0644);
	c(auto_sysconfdir, "etc/..", "indimail-mta.fc", auto_uido, auto_gidq, 0644);
#endif

	/* Binaries */
	c(auto_qmail_home, "bin", "qmail-inject", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-showctl", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-qread", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-pop3d", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-popbull", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-qmqpd", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-qmtpd", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-smtpd", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "sendmail", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "tcp-env", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qreceipt", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "forward", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "preline", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "condredirect", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "bouncesaying", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "except", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmaildirmake", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "maildir2mbox", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "mbox2maildir", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "maildirwatch", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "predate", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "datemail", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "mailsubj", auto_uido, auto_gidq, moder_x);

	c(auto_qmail_home, "bin", "qmail-qfilter", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "surblfilter", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "surblqueue", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "drate", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "cidr", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "spawn-filter", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-cat", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-poppass", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-greyd", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "greydaemon", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "irmail", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "rrforward", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "maildirdeliver", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-rm", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "autoresponder", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qnotify", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "rrt", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qarf", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qpq", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qail", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "elq", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "pinq", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qmail-lint", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qsmhook", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "etrn", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "atrn", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qmail-lagcheck", auto_uido, auto_gidq, moder_x);
#if defined(HASDKIM) || defined(DOMAIN_KEYS)
	c(auto_qmail_home, "bin", "dk-filter", auto_uido, auto_gidq, moder_x);
#endif
	c(auto_qmail_home, "bin", "queue-fix", auto_uido, auto_gidq, moder_x);

	c(auto_qmail_home, "bin", "qmailctl", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "uacl", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qbase64", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "swaks", auto_uido, auto_gidq, moder_x);

	c(auto_qmail_home, "bin", "qmail-newu", auto_uido, auto_gidq, moder_t);
	c(auto_qmail_home, "bin", "qmail-newmrh", auto_uido, auto_gidq, moder_t);
	c(auto_qmail_home, "bin", "recipient-cdb", auto_uido, auto_gidq, moder_t);
	c(auto_qmail_home, "bin", "qmail-cdb", auto_uido, auto_gidq, moder_t);
#ifdef INDIMAIL
	c(auto_qmail_home, "bin", "qmail-sql", auto_uido, auto_gidq, moder_t);
#endif
	c(auto_qmail_home, "bin", "cdbmake", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "cdbget", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "cdbgetm", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "cdbdump", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "cdbstats", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "cdbtest", auto_uido, auto_gidq, moder_x);
#ifdef TLS
	c(auto_libexec_dir, auto_libexec_base, "update_tmprsadh", auto_uido, auto_gidq, moder_x);
#endif
	c(auto_libexec_dir, auto_libexec_base, "rpmattr", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "ipmeprint", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "idedit", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "cdbmake-12", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "cdbmake-sv", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "testzero", auto_uido, auto_gidq, moder_x);

	c(auto_qmail_home, "bin", "qaes", auto_uido, auto_gidq, moder_x);
#ifdef USE_SPF
	c(auto_qmail_home, "bin", "spfquery", auto_uido, auto_gidq, moder_x);
#endif

#ifdef HAVESRS
	c(auto_qmail_home ,"bin", "srsfilter", auto_uido, auto_gidq, moder_x);
#endif
#ifdef DOMAIN_KEYS
	c(auto_qmail_home, "bin", "dknewkey", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "dktest", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-dk", auto_uido, auto_gidq, moder_x);
#endif
#ifdef HASDKIM
	c(auto_qmail_home, "bin", "qmail-dkim", auto_uido, auto_gidq, moder_x);
#endif

	c(auto_qmail_home, "bin", "recordio", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "argv0", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "addcr", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "delcr", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "fixcrio", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qmail-getpw", auto_uido, auto_gidq, moder_u);
	c(auto_qmail_home, "bin", "qmail-local", auto_uido, auto_gidq, moder_u);
	c(auto_qmail_home, "bin", "qmail-remote", auto_uido, auto_gidq, moder_u);
	c(auto_qmail_home, "bin", "qmail-pw2u", auto_uido, auto_gidq, moder_u);
	c(auto_qmail_home, "bin", "qmail-popup", auto_uido, auto_gidq, moder_u);

	c(auto_qmail_home, "sbin", "relaytest", auto_uido, auto_gidq, moder_u);
	c(auto_qmail_home, "sbin", "splogger", auto_uido, auto_gidq, moder_u);
	c(auto_qmail_home, "sbin", "qmail-tcpto", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "qmail-tcpok", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "cleanq", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "qscanq-stdin", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "qmail-qmqpc", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "qmail-daemon", auto_uido, auto_gidq, moder_t);
	c(auto_qmail_home, "sbin", "qmail-start", auto_uido, auto_gidq, moder_t);
	c(auto_qmail_home, "sbin", "qmail-lspawn", auto_uido, auto_gidq, moder_t);
	c(auto_qmail_home, "sbin", "qmail-rspawn", auto_uido, auto_gidq, moder_t);
	c(auto_qmail_home, "sbin", "qmail-clean", auto_uido, auto_gidq, moder_t);
	c(auto_qmail_home, "sbin", "qmail-send", auto_uido, auto_gidq, moder_t);
#ifdef EXTERNAL_TODO
	c(auto_qmail_home, "sbin", "qmail-todo", auto_uido,auto_gidq,moder_t);
#endif
#ifdef SMTP_PLUGIN
	c(auto_qmail_home, "sbin", "plugtest", auto_uido, auto_gidq, moder_x);
#endif
	c(auto_libexec_dir, auto_libexec_base, "dnscname", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnsptr", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnsip", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnsmxip", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnsfq", auto_uido, auto_gidq, moder_x);
#ifdef USE_SPF
	c(auto_libexec_dir, auto_libexec_base, "dnstxt", auto_uido, auto_gidq, moder_x);
#endif
	c(auto_libexec_dir, auto_libexec_base, "leapsecs", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "yearcal", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "nowutc", auto_uido, auto_gidq, moder_x);

	/* mess822 */
	c(auto_qmail_home, "bin", "ofmipd", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "ofmipname", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "iftocc", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "iftoccfrom", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "ifaddr", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "new-inject", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822field", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822header", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822date", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822received", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822print", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822body", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822headerok", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822bodyfilter", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822headerfilter", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822addr", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "822fields", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "checkaddr", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "checkdomain", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "filterto", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "condtomaildir", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "replier", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "replier-config", auto_uido, auto_gidq, moder_x);

	/* fastforward */
	c(auto_qmail_home, "bin", "dot-forward", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "fastforward", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "printforward", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "setforward", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "inewaliases", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "printmaillist", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "setmaillist", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "newinclude", auto_uido, auto_gidq, moder_x);

	/* daemontools */
	c(auto_qmail_home, "bin", "envdir", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "envuidgid", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "fghack", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "multilog", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "pgrphack", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "setlock", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "setuidgid", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "softlimit", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "supervise", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "svc", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "svok", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "svstat", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "tai64n", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "tai64nlocal", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "tai64nunix", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "tai2tai64n", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "tai64n2tai", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "spipe", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qfilelog", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "multipipe", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "teepipe", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "multitail", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "logselect", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "qlogselect", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "udplogger", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "udpclient", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "svscan", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "svscanboot", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "readproctitle", auto_uido, auto_gidq, moder_x);

	/* serialmail */
	c(auto_qmail_home, "bin", "serialcmd", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "serialqmtp", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "serialsmtp", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "maildircmd", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "maildirqmtp", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "maildirsmtp", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "maildirserial", auto_uido, auto_gidq, moder_x);

	/* Report Programs */
	c(auto_qmail_home, "bin", "matchup", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "mlmatchup", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "columnt", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "bin", "xqp", auto_uido, auto_gidq, moder_x);

	c(auto_libexec_dir, auto_libexec_base, "zoverall", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsendmail", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "xsender", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "xrecipient", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "ddist", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "deferrals", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "failures", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "successes", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rhosts", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "recipients", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rxdelay", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "senders", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "suids", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zddist", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zdeferrals", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zfailures", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsuccesses", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zrhosts", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zrecipients", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zrxdelay", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsenders", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsuids", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "multilog-matchup", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "smtp-matchup", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtp", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtpfailures", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtpsdomains", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtprdomains", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtpsenders", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtprecipients", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsmtp", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zspam", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rspamrdomain", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rspamsdomain", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rspamstat", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rspamhist", auto_uido, auto_gidq, moder_x);

	/* setuid/setgid Programs */
	c(auto_qmail_home, "sbin", "qmail-queue", auto_uidq, auto_gidq, 06551);
	c(auto_qmail_home, "sbin", "qhpsi", auto_uidc, auto_gidq, 06551);
	c(auto_qmail_home, "sbin", "qscanq", auto_uidc, auto_gidc, 04551);
	c(auto_qmail_home, "sbin", "run-cleanq", auto_uido, auto_gidc, 02551);

	/*- misc */
#ifdef BATV
	c(auto_qmail_home, "sbin", "batv", auto_uido, auto_gidq, moder_x);
#endif
	c(auto_libexec_dir, auto_libexec_base, "envmigrate", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "hostname", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qmailconfig", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "config-fast", auto_uido, auto_gidq, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "instcheck", auto_uido, auto_gidq, moder_t);
	c(auto_libexec_dir, auto_libexec_base, "whois", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "qmail-multi", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "qmail-nullqueue", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "sys-checkpwd", auto_uido, auto_gidq, moder_x);
	c(auto_qmail_home, "sbin", "ldap-checkpwd", auto_uido, auto_gidq, moder_x);

#if defined(SMTP_PLUGIN) || defined(LOAD_SHARED_OBJECTS)
	d(auto_prefix,     "lib/indimail/plugins", auto_uido, auto_gidq, 0555);
#endif
#ifdef SMTP_PLUGIN
	c(auto_prefix,     "lib/indimail/plugins", "smtpd-plugin.so", auto_uido, auto_gidq, moder_s);
	c(auto_prefix,     "lib/indimail/plugins", "smtpd-plugin0.so", auto_uido, auto_gidq, moder_s);
#endif
	c(auto_prefix,     "lib/indimail/plugins", "generic.so", auto_uido, auto_gidq, moder_s);
#ifdef LOAD_SHARED_OBJECTS
	c(auto_prefix,     "lib/indimail/plugins", "qmail_smtpd.so", auto_uido, auto_gidq, moder_s);
#endif

	/* GPLv3 License, Man Pages, Documents */
	c(auto_shared,     "doc", "COPYING", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.licenses", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "CREDITS", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "FROMISP", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "TOISP", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "AUTOTURN", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "EXTTODO", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "INTERNALS", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.qmail", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.clamav", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.greylist", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.logselect", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.srs", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.surbl", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.filters", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.moreipme", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.recipients", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.local2alias", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.local2ext", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.local2local", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.local2rem", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.local2virt", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.nullclient", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.relaybad", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.relaygood", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "PIC.rem2local", auto_uido, auto_gidq, 0444);
#if 0
	c(auto_shared,     "doc", "README.indimail", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "QMAILFAQ", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "UPGRADE", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "SENDMAIL", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "INSTALL.qmail", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "INSTALL.alias", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "INSTALL.ctl", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "INSTALL.ids", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "INSTALL.maildir", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "INSTALL.mbox", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "INSTALL.vsm", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "TEST.deliver", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "TEST.receive", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "REMOVE.sendmail", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "REMOVE.binmail", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.auth", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.newline", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.qhpsi", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.qregex", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.queue-fix", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.remote-auth", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.spamcontrol", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.starttls", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.status", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.tls", auto_uido, auto_gidq, 0444);
	c(auto_shared,     "doc", "README.wildmat", auto_uido, auto_gidq, 0444);
#endif
	c(mandir,          "man/man1", "cidr.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qmail-cat.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "predate.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "datemail.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "argv0.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "addcr.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "delcr.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "fixcrio.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "recordio.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "uacl.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qbase64.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "swaks.1", uidr, gidr, moder_f);
	c(mandir,          "man/man7", "forgeries.7", uidr, gidr, moder_f);
	c(mandir,          "man/cat7", "forgeries.0", uidr, gidr, moder_f);
	c(mandir,          "man/man7", "qmail-limits.7", uidr, gidr, moder_f);
	c(mandir,          "man/cat7", "qmail-limits.0", uidr, gidr, moder_f);
	c(mandir,          "man/man7", "qmail.7", uidr, gidr, moder_f);
	c(mandir,          "man/cat7", "qmail.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "forward.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "forward.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "rrforward.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "rrforward.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "condredirect.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "condredirect.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "bouncesaying.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "bouncesaying.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "except.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "except.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qmaildirmake.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "qmaildirmake.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "maildir2mbox.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "maildir2mbox.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "mbox2maildir.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "maildirwatch.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "maildirwatch.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "mailsubj.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "mailsubj.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qreceipt.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "qreceipt.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "preline.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "preline.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "tcp-env.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "tcp-env.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "relaytest.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "maildirdeliver.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "maildirdeliver.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "autoresponder.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "autoresponder.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qnotify.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "rrt.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qarf.1", uidr, gidr, moder_f);

	/* serialmail */
	c(mandir,          "man/man1", "serialcmd.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "serialcmd.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "serialqmtp.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "serialqmtp.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "serialsmtp.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "serialsmtp.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "maildircmd.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "maildircmd.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "maildirqmtp.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "maildirqmtp.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "maildirsmtp.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "maildirsmtp.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "maildirserial.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "maildirserial.0", uidr, gidr, moder_f);

	/*- report programs */
	c(mandir,          "man/man1", "matchup.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "matchup.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "mlmatchup.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "mlmatchup.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "xqp.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "xqp.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "xsender.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "xsender.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "xrecipient.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "xrecipient.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "columnt.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "columnt.0", uidr, gidr, moder_f);

	c(mandir,          "man/man1", "cdbget.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "cdbgetm.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "cdbtest.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "cdbstats.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "cdbmake.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "cdbdump.1", uidr, gidr, moder_f);

	c(mandir,          "man/man1", "qaes.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qmail-rm.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "qmail-rm.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "plugtest.1", uidr, gidr, moder_f);

	c(mandir,          "man/man5", "addresses.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "addresses.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "envelopes.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "envelopes.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "maildir.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "maildir.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "mbox.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "mbox.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "dot-qmail.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "dot-qmail.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "qmail-control.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "qmail-control.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "qmail-header.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "qmail-header.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "qmail-log.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "qmail-log.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "qmail-users.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "qmail-users.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "tcp-environ.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "tcp-environ.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rewriting.5", uidr, gidr, moder_f);
	c(mandir,          "man/cat5", "rewriting.0", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "qmtp.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-822.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-1845.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-2821.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-1893.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-1894.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-1985.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-1321.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-2104.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-2645.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-2554.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-2505.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-2635.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-2476.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-3834.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-3798.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-4871.5", uidr, gidr, moder_f);
	c(mandir,          "man/man5", "rfc-4870.5", uidr, gidr, moder_f);

	/*- fastforward */
	c(mandir,          "man/man1", "dot-forward.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "dot-forward.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "fastforward.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "fastforward.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "printforward.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "printforward.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "setforward.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "setforward.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "inewaliases.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "newaliases.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "printmaillist.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "printmaillist.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "setmaillist.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "setmaillist.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "newinclude.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "newinclude.0", uidr, gidr, moder_f);

	/*- mess822 */
	c(mandir,          "man/man8", "ofmipd.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "ofmipd.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "ofmipname.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "ofmipname.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "iftocc.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "iftocc.0", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "iftoccfrom.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "iftoccfrom.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "ifaddr.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "ifaddr.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "new-inject.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "new-inject.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822field.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822field.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822header.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822header.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822date.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822date.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822received.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822received.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822print.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822print.0", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822body.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822body.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822headerok.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822headerok.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822bodyfilter.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822bodyfilter.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822headerfilter.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822headerfilter.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822addr.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822addr.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "822fields.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "822fields.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "checkaddr.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "checkaddr.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "checkdomain.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "checkdomain.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "filterto.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "filterto.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "condtomaildir.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "condtomaildir.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "replier.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "replier.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "replier-config.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "replier-config.1", uidr, gidr, moder_f);

	/*- daemontools */
	c(mandir,          "man/man8", "envdir.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "envuidgid.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "fghack.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "multilog.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "pgrphack.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "readproctitle.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "setlock.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "setuidgid.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "softlimit.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "supervise.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "svc.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "svok.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "svscan.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "svstat.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "tai64n.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "tai64nlocal.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "tai64nunix.8", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "spipe.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qfilelog.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "multipipe.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "teepipe.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "multitail.1", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "logselect.8", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qlogselect.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "tai2tai64n.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "tai64n2tai.1", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "svscanboot.8", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "udpclient.1", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "udplogger.8", uidr, gidr, moder_f);

	/*- qmail */
	c(mandir,          "man/man8", "qmailctl.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-local.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-local.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-lspawn.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-lspawn.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-getpw.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-getpw.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-remote.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-remote.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-rspawn.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-rspawn.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-clean.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-clean.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-send.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-send.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-todo.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-todo.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-daemon.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-start.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-start.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "splogger.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "splogger.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-internals.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-queue.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qhpsi.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "surblqueue.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-queue.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-nullqueue.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-popbull.8", uidr, gidr, moder_f);

#ifdef HASDKIM
	c(mandir,          "man/man8", "qmail-dkim.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "dk-filter.8", uidr, gidr, moder_f);
#endif
#ifdef DOMAIN_KEYS
	c(mandir,          "man/man8", "qmail-dk.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-dk.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "dknewkey.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "dktest.8", uidr, gidr, moder_f);
#endif
	c(mandir,          "man/man8", "qmail-multi.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-multi.0", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "drate.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qmail-qfilter.1", uidr, gidr, moder_f);
	c(mandir,          "man/cat1", "qmail-qfilter.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "surblfilter.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "spawn-filter.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "spawn-filter.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-inject.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-inject.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "isendmail.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "irmail.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-showctl.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-showctl.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-newmrh.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-newmrh.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "recipient-cdb.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "recipient-cdb.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-sql.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-cdb.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-cdb.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-newu.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-newu.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-pw2u.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-pw2u.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-qread.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-qread.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "queue-fix.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "queue-fix.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-tcpok.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-tcpok.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-tcpto.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-tcpto.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-pop3d.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-pop3d.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-popup.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-popup.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-qmqpc.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-qmqpc.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-qmqpd.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-qmqpd.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-qmtpd.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-qmtpd.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-smtpd.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-smtpd.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-greyd.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-greyd.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "greydaemon.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "greydaemon.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-poppass.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-poppass.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qmail-command.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qmail-command.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "sys-checkpwd.8", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "ldap-checkpwd.8", uidr, gidr, moder_f);

	c(mandir,          "man/man8", "qscanq.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qscanq.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "qscanq-stdin.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "qscanq-stdin.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "cleanq.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "cleanq.0", uidr, gidr, moder_f);
	c(mandir,          "man/man8", "run-cleanq.8", uidr, gidr, moder_f);
	c(mandir,          "man/cat8", "run-cleanq.0", uidr, gidr, moder_f);

	/*- library */
	c(mandir,          "man/man3", "alloc.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "case.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "cdb.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "coe.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "datetime.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "direntry.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "env.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "qerror.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "error_str.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "error_temp.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "fd_copy.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "fd_move.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "fifo_make.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "getln.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "getln2.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "now.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "sgetopt.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "stralloc.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "byte.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "substdio.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "substdio_in.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "substdio_out.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "substdio_copy.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "subgetopt.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "wait.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "caldate.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "caldate_mjd.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "caltime.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "caltime_tai.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "config.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "leapsecs.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "tai.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "tai_now.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "tai_pack.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "rewritehost.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "mess822.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "mess822_addr.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "mess822_date.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "mess822_fold.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "mess822_quote.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "mess822_token.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "mess822_when.3", uidr, gidr, moder_f);
	c(mandir,          "man/man3", "plugin_init.3", uidr, gidr, moder_f);
}

void
getversion_install_big_c()
{
	static char    *x = "$Id: hier.c,v 1.219 2016-06-20 08:31:49+05:30 Cprogrammer Exp mbhangui $";

#ifdef INDIMAIL
	if (x)
		x = sccsidh;
#else
	if (x)
		x++;
#endif
}
