/*
 * $Log: hier.c,v $
 * Revision 1.254  2019-02-21 14:14:22+05:30  Cprogrammer
 * added qfilters directory in /usr/libexec/indimail
 * added qfrontend script for qmail-qfilter
 * added smtp ratelimit qmail-qfilter script
 *
 * Revision 1.253  2018-11-11 17:35:10+05:30  Cprogrammer
 * added maildirsize man page
 *
 * Revision 1.252  2018-11-11 14:07:37+05:30  Cprogrammer
 * added maildirsize program
 *
 * Revision 1.251  2018-09-10 12:14:28+05:30  Cprogrammer
 * added tcp subdir for tcpserver access control files
 *
 * Revision 1.250  2018-09-02 13:51:52+05:30  Cprogrammer
 * conditional compilation of qmail-daned, qdane, dnstlsarr
 *
 * Revision 1.249  2018-06-25 13:43:23+05:30  Cprogrammer
 * changed group to root for binaries in bin, sbin
 *
 * Revision 1.248  2018-05-30 20:12:35+05:30  Cprogrammer
 * added man page for dnstlsarr
 *
 * Revision 1.247  2018-05-28 19:55:51+05:30  Cprogrammer
 * install qmail-daned, dnstlsarr regardless of HASTLSA
 *
 * Revision 1.246  2018-05-27 14:12:03+05:30  Cprogrammer
 * removed daneprog script
 *
 * Revision 1.245  2018-05-27 11:17:34+05:30  mbhangui
 * fixed dnstlsarr path
 *
 * Revision 1.244  2018-05-26 15:46:13+05:30  Cprogrammer
 * added dnstlsarr program
 *
 * Revision 1.243  2018-05-21 07:22:37+05:30  Cprogrammer
 * added foxhole_all.cdb file
 *
 * Revision 1.242  2018-05-20 09:56:00+05:30  Cprogrammer
 * renamed qmail-dane to qmail-daned
 *
 * Revision 1.241  2018-04-26 11:39:02+05:30  Cprogrammer
 * added qdane, qdane man page, tlsacheck(), greylist(), qmail-dane man pages
 *
 * Revision 1.240  2018-04-25 21:44:27+05:30  Cprogrammer
 * added qmail-dane, daneprog
 *
 * Revision 1.239  2018-02-05 12:40:29+05:30  Cprogrammer
 * updated doc list
 *
 * Revision 1.238  2018-01-31 14:36:52+05:30  Cprogrammer
 * moved qmail-popbull, multilog, udplogger to sbin
 *
 * Revision 1.237  2018-01-31 14:18:56+05:30  Cprogrammer
 * moved qmail-pop3d, qmail-qmqpd, qmail-qmtpd, qmail-smtpd, qmail-poppass, qmail-dk, qmail-dkim, qmail-popup, ofmipd to sbin
 *
 * Revision 1.236  2018-01-31 12:04:51+05:30  Cprogrammer
 * moved system binaries to sbin
 *
 * Revision 1.235  2018-01-25 16:16:09+05:30  Cprogrammer
 * inquery, domains directory are part of indimail package
 *
 * Revision 1.234  2018-01-09 11:41:09+05:30  Cprogrammer
 * renamed cronlist to cronlist.q
 * added nodnscheck control file
 * moved leapsecs.dat, leapsecs.txt to /etc/indimail
 * added software licenses for yahoo domainkeys
 *
 * Revision 1.233  2017-05-12 18:04:36+05:30  Cprogrammer
 * added inotify - monitor file system events
 *
 * Revision 1.232  2017-04-23 18:28:54+05:30  Cprogrammer
 * added cronlist
 *
 * Revision 1.231  2017-04-16 13:06:55+05:30  Cprogrammer
 * moved surbfilter, surblqueue, qmail-greyd, greydaemon to sbin
 *
 * Revision 1.230  2017-04-13 00:26:47+05:30  Cprogrammer
 * updated attribution, licenses for indimail-mta package
 *
 * Revision 1.229  2017-04-12 14:52:15+05:30  Cprogrammer
 * report programs moved to libexecdir
 *
 * Revision 1.228  2017-04-12 13:21:50+05:30  Cprogrammer
 * create queue base directory
 *
 * Revision 1.227  2017-04-11 18:26:51+05:30  Cprogrammer
 * sysconfdir permissions were getting set multiple times
 *
 * Revision 1.226  2017-03-27 08:51:20+05:30  Cprogrammer
 * moved inquery to /var/indimail
 *
 * Revision 1.225  2017-03-21 15:39:11+05:30  Cprogrammer
 * added certs directory for certificates
 *
 * Revision 1.224  2017-01-08 19:02:27+05:30  Cprogrammer
 * skip development man pages if dev_package is 0
 *
 * Revision 1.223  2017-01-04 15:46:35+05:30  Cprogrammer
 * moved svscanboot to libexecdir
 *
 * Revision 1.222  2016-06-24 16:37:25+05:30  Cprogrammer
 * added spfquery man page
 *
 * Revision 1.221  2016-06-24 13:26:18+05:30  Cprogrammer
 * added srsfilter, qmail-srs man page
 *
 * Revision 1.220  2016-06-20 08:41:25+05:30  Cprogrammer
 * moved batv to libexecdir
 *
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
hier(inst_dir, fatal, dev_package)
	char           *inst_dir, *fatal;
	int             dev_package;
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
		/* create indimail/qmail home directory (/var/indimail or /var/qmail */
		h(auto_qmail_home, auto_uido, auto_gidq, 0555);
	}
	/*- sysconf directory for control, config files */
	if (str_diff(auto_qmail, auto_sysconfdir))
		h(auto_sysconfdir, auto_uido, auto_gidq, 02755); /*- create /etc/indimail */

	/*-
	 * auto_control    = /etc/indimail/control
	 * auto_cntrl_dir  = /etc/indimail
	 * auto_cntrl_base = control
	 */
	auto_cntrl_dir = getdirname(auto_control, &auto_cntrl_base);
	if (!stralloc_copys(&a1, auto_cntrl_dir))
		strerr_die2sys(111, fatal, "out of memory: ");
	if (!stralloc_0(&a1))
		strerr_die2sys(111, fatal, "out of memory: ");
	auto_cntrl_dir = a1.s;
	if (!str_diff(auto_qmail, auto_cntrl_dir))
		/* create control directory (/var/indimail/control */
		d(auto_qmail_home, "control", auto_uidv, auto_gidq, 0775);
	else {
		/* create control directory (/etc/indimail */
		if (str_diff(auto_sysconfdir, auto_cntrl_dir))
			h(auto_cntrl_dir, auto_uido, auto_gidq, 02755);
		/* create control directory (/etc/indimail/control */
		d(auto_cntrl_dir, "control", auto_uidv, auto_gidq, 02775);
		/*- link /var/indimail/control to /etc/indimail/control */
		l(auto_qmail_home, "control", auto_control, 0);
	}

	/*-
	 * auto_assign     = /etc/indimail/users
	 * auto_assgn_dir  = /etc/indimail
	 * auto_assgn_base = users
	 */
	auto_assgn_dir = getdirname(auto_assign, &auto_assgn_base);
	if (!stralloc_copys(&a2, auto_assgn_dir))
		strerr_die2sys(111, fatal, "out of memory: ");
	if (!stralloc_0(&a2))
		strerr_die2sys(111, fatal, "out of memory: ");
	auto_assgn_dir = a2.s;
	if (!str_diff(auto_qmail, auto_assgn_dir))
		d(auto_qmail_home, "users", auto_uidv, auto_gidq, 0775);
	else {
		if (str_diff(auto_sysconfdir, auto_assgn_dir))
			h(auto_assgn_dir, auto_uido, auto_gidq, 02755);
		d(auto_assgn_dir, "users", auto_uidv, auto_gidq, 02775);
		/*- link /var/indimail/users to /etc/indimail/users */
		l(auto_qmail_home, "users", auto_assign, 0);
	}

	/*- shared directory for boot, doc, man */
	if (str_diff(auto_qmail, auto_shared)) {
		/*- autoshared = /usr/share/indimail */
		mandir = getdirname(auto_shared, 0);
		if (!stralloc_copys(&a3, mandir))
			strerr_die2sys(111, fatal, "out of memory: ");
		if (!stralloc_0(&a3))
			strerr_die2sys(111, fatal, "out of memory: ");
		mandir = a3.s; /* /usr/share */
		h(auto_shared, auto_uido, 0, 0555);
	} else
		mandir = auto_qmail_home;

	/*-
	 * libexecdir directory for internal binaries
	 * auto_libexec      = /usr/libexec/indimail
	 * auto_libexec_dir  = /usr/libexec
	 * auto_libexec_base = indimail
	 */
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
		/*- link /var/indimail/libexec to /usr/libexec/indimail */
		l(auto_qmail_home, "libexec", auto_libexec, 0);
	}

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

	d(auto_qmail_home, "queue", auto_uido, auto_gidq, 0755);
	d(auto_libexec,    "qfilters", auto_uido, 0, 0755);

	d(auto_sysconfdir, "certs", auto_uidv, auto_gidq, 02775);
	d(auto_sysconfdir, "tcp", auto_uidv, auto_gidq, 02775);

	d(auto_qmail_home, "qscanq", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "qscanq/root", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "qscanq/root/scanq", auto_uidc, auto_gidc, 0750);
	d(auto_qmail_home, "alias", auto_uida, auto_gidq, 02775);
	d(auto_qmail_home, "autoturn", auto_uidv, auto_gidq, 02775);

	d(auto_cntrl_dir,  "control/domainkeys", auto_uidv, auto_gidq, 02755);
	d(auto_cntrl_dir,  "control/ratelimit", auto_uidr, auto_gidq, 02775);
	d(auto_cntrl_dir,  "control/defaultqueue", auto_uidv, auto_gidq, 0755);

	d(auto_shared,     "boot", auto_uido, 0, 0555);
	d(auto_shared,     "doc", auto_uido, 0, 0555);

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
	c(auto_shared,     "boot", "home", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "home+df", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "proc", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "proc+df", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "binm1", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "binm1+df", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "binm2", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "binm2+df", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "binm3", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "binm3+df", auto_uido, 0, 0755);
	c(auto_shared,     "boot", "upstart", auto_uido, 0, 0444);
	c(auto_shared,     "boot", "systemd", auto_uido, 0, 0444);
#ifdef DARWIN
	c(auto_shared,     "boot", "StartupParameters.plist", auto_uido, 0, 0444);
	c(auto_shared,     "boot", "indimail.plist", auto_uido, 0, 0444);
#endif

	c(auto_sysconfdir, "control", "nodnscheck", auto_uidv, auto_gidv, 0644);
	c(auto_sysconfdir, ".", "leapsecs.dat", auto_uido, auto_gidq, 0644);
	c(auto_sysconfdir, ".", "leapsecs.txt", auto_uido, auto_gidq, 0644);
	c(auto_sysconfdir, ".", "cronlist.q", auto_uido, 0, 0444);
	c(auto_sysconfdir, ".", "foxhole_all.cdb", auto_uido, auto_gidq, 0644);

	/* Binaries */
	c(auto_prefix, "bin", "qmail-inject", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qmail-showctl", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qmail-qread", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "sendmail", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "tcp-env", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qreceipt", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "forward", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "preline", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "condredirect", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "bouncesaying", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "except", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qmaildirmake", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "maildir2mbox", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "mbox2maildir", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "maildirwatch", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "predate", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "datemail", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "mailsubj", auto_uido, 0, moder_x);

	c(auto_prefix, "bin", "qmail-qfilter", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "drate", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "cidr", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qmail-cat", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "irmail", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "rrforward", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "maildirdeliver", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qmail-rm", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "autoresponder", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qnotify", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "rrt", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qarf", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "inotify", auto_uido, 0, moder_x);
#if defined(HASDKIM) || defined(DOMAIN_KEYS)
	c(auto_prefix, "bin", "dk-filter", auto_uido, 0, moder_x);
#endif
	c(auto_prefix, "bin", "queue-fix", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qmailctl", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "uacl", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qbase64", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "swaks", auto_uido, 0, moder_x);
#ifdef HASTLSA
	c(auto_prefix, "bin", "qdane", auto_uido, 0, moder_x);
#endif
	c(auto_prefix, "bin", "qaes", auto_uido, 0, moder_x);
#ifdef USE_SPF
	c(auto_prefix, "bin", "spfquery", auto_uido, 0, moder_x);
#endif
#ifdef HAVESRS
	c(auto_prefix ,"bin", "srsfilter", auto_uido, 0, moder_x);
#endif
#ifdef DOMAIN_KEYS
	c(auto_prefix, "bin", "dknewkey", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "dktest", auto_uido, 0, moder_x);
#endif
	c(auto_prefix, "bin", "recordio", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "argv0", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "addcr", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "delcr", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "fixcrio", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "maildirsize", auto_uido, 0, moder_x);

#ifdef DOMAIN_KEYS
	c(auto_prefix, "sbin", "qmail-dk", auto_uido, 0, moder_x);
#endif
#ifdef HASDKIM
	c(auto_prefix, "sbin", "qmail-dkim", auto_uido, 0, moder_x);
#endif
	c(auto_prefix, "sbin", "qmail-popbull", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-popup", auto_uido, 0, moder_u);
	c(auto_prefix, "sbin", "qmail-poppass", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-pop3d", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-qmqpd", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-qmtpd", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-smtpd", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "spawn-filter", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-local", auto_uido, 0, moder_u);
	c(auto_prefix, "sbin", "qmail-remote", auto_uido, 0, moder_u);
	c(auto_prefix, "sbin", "recipient-cdb", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "qmail-getpw", auto_uido, 0, moder_u);
	c(auto_prefix, "sbin", "qmail-pw2u", auto_uido, 0, moder_u);
	c(auto_prefix, "sbin", "cdbdump", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "cdbstats", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "cdbtest", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "cdbget", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "cdbgetm", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "cdbmake", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-newmrh", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "relaytest", auto_uido, 0, moder_u);
	c(auto_prefix, "sbin", "splogger", auto_uido, 0, moder_u);
	c(auto_prefix, "sbin", "qmail-tcpto", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-tcpok", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "cleanq", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qscanq-stdin", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-qmqpc", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-daemon", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "qmail-start", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "qmail-lspawn", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "qmail-rspawn", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "qmail-clean", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "qmail-send", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "svscan", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "readproctitle", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-greyd", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "greydaemon", auto_uido, 0, moder_x);
#ifdef HASTLSA
	c(auto_prefix, "sbin", "qmail-daned", auto_uido, 0, moder_x);
#endif
	c(auto_prefix, "sbin", "surblfilter", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "surblqueue", auto_uido, 0, moder_x);
#ifdef EXTERNAL_TODO
	c(auto_prefix, "sbin", "qmail-todo", auto_uido,0,moder_t);
#endif
#ifdef SMTP_PLUGIN
	c(auto_prefix, "sbin", "plugtest", auto_uido, 0, moder_x);
#endif
	c(auto_prefix, "sbin", "svctool", auto_uido, 0, moder_x);

	c(auto_prefix, "sbin", "qmail-newu", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "qmail-cdb", auto_uido, 0, moder_t);
	c(auto_prefix, "sbin", "qmail-sql", auto_uido, 0, moder_t);

	/* mess822 */
	c(auto_prefix, "bin", "ofmipname", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "iftocc", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "iftoccfrom", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "ifaddr", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "new-inject", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822field", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822header", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822date", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822received", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822print", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822body", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822headerok", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822bodyfilter", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822headerfilter", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822addr", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "822fields", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "checkaddr", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "checkdomain", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "filterto", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "condtomaildir", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "replier", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "replier-config", auto_uido, 0, moder_x);

	c(auto_prefix, "sbin", "ofmipd", auto_uido, 0, moder_x);

	/* fastforward */
	c(auto_prefix, "bin", "dot-forward", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "fastforward", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "printforward", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "setforward", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "inewaliases", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "printmaillist", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "setmaillist", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "newinclude", auto_uido, 0, moder_x);

	/* daemontools */
	c(auto_prefix, "bin", "envdir", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "envuidgid", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "fghack", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "pgrphack", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "setlock", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "setuidgid", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "softlimit", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "supervise", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "svc", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "svok", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "svstat", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "tai64n", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "tai64nlocal", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "tai64nunix", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "tai2tai64n", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "tai64n2tai", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "spipe", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qfilelog", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "multipipe", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "teepipe", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "multitail", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "logselect", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "qlogselect", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "udpclient", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "udplogger", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "multilog", auto_uido, 0, moder_x);

	/* serialmail */
	c(auto_prefix, "bin", "serialcmd", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "serialqmtp", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "serialsmtp", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "maildircmd", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "maildirqmtp", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "maildirsmtp", auto_uido, 0, moder_x);
	c(auto_prefix, "bin", "maildirserial", auto_uido, 0, moder_x);

	/* setuid/setgid Programs */
	c(auto_prefix, "sbin", "qmail-queue", auto_uidq, auto_gidq, 06551);
	c(auto_prefix, "sbin", "qhpsi", auto_uidc, auto_gidq, 06551);
	c(auto_prefix, "sbin", "qscanq", auto_uidc, auto_gidc, 04551);
	c(auto_prefix, "sbin", "run-cleanq", auto_uido, auto_gidc, 02551);

	/*- misc */
	c(auto_prefix, "sbin", "qmail-multi", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "qmail-nullqueue", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "sys-checkpwd", auto_uido, 0, moder_x);
	c(auto_prefix, "sbin", "ldap-checkpwd", auto_uido, 0, moder_x);

#if defined(SMTP_PLUGIN) || defined(LOAD_SHARED_OBJECTS)
	d(auto_prefix, "lib/indimail/plugins", auto_uido, 0, 0555);
#endif
#ifdef SMTP_PLUGIN
	c(auto_prefix, "lib/indimail/plugins", "smtpd-plugin.so", auto_uido, 0, moder_s);
	c(auto_prefix, "lib/indimail/plugins", "smtpd-plugin0.so", auto_uido, 0, moder_s);
#endif
	c(auto_prefix, "lib/indimail/plugins", "generic.so", auto_uido, 0, moder_s);
#ifdef LOAD_SHARED_OBJECTS
	c(auto_prefix, "lib/indimail/plugins", "qmail_smtpd.so", auto_uido, 0, moder_s);
#endif

	c(auto_libexec, "qfilters", "qf-smtp-ratelimit", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qfrontend", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qpq", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qail", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "elq", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "pinq", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qmail-lint", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qsmhook", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "etrn", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "atrn", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qmail-lagcheck", auto_uido, 0, moder_x);
#ifdef TLS
	c(auto_libexec_dir, auto_libexec_base, "update_tmprsadh", auto_uido, 0, moder_x);
#endif
	c(auto_libexec_dir, auto_libexec_base, "rpmattr", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "ipmeprint", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "idedit", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "cdbmake-12", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "cdbmake-sv", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "testzero", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnscname", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnsptr", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnsip", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnsmxip", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "dnsfq", auto_uido, 0, moder_x);
#ifdef USE_SPF
	c(auto_libexec_dir, auto_libexec_base, "dnstxt", auto_uido, 0, moder_x);
#endif
#ifdef HASTLSA
	c(auto_libexec_dir, auto_libexec_base, "dnstlsarr", auto_uido, 0, moder_x);
#endif
	c(auto_libexec_dir, auto_libexec_base, "leapsecs", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "yearcal", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "nowutc", auto_uido, 0, moder_x);

	/* Report Programs */
	c(auto_libexec_dir, auto_libexec_base, "svscanboot", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "columnt", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zoverall", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsendmail", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "xsender", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "xrecipient", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "ddist", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "deferrals", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "failures", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "successes", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rhosts", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "recipients", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rxdelay", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "senders", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "suids", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zddist", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zdeferrals", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zfailures", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsuccesses", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zrhosts", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zrecipients", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zrxdelay", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsenders", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsuids", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "multilog-matchup", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "smtp-matchup", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "matchup", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "mlmatchup", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "xqp", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtp", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtpfailures", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtpsdomains", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtprdomains", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtpsenders", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rsmtprecipients", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zsmtp", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "zspam", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rspamrdomain", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rspamsdomain", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rspamstat", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "rspamhist", auto_uido, 0, moder_x);
#ifdef BATV
	c(auto_libexec_dir, auto_libexec_base, "batv", auto_uido, 0, moder_x);
#endif

	/*- misc */
	c(auto_libexec_dir, auto_libexec_base, "envmigrate", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "hostname", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "qmailconfig", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "config-fast", auto_uido, 0, moder_x);
	c(auto_libexec_dir, auto_libexec_base, "instcheck", auto_uido, 0, moder_t);
	c(auto_libexec_dir, auto_libexec_base, "whois", auto_uido, 0, moder_x);

	/* GPLv3 License, Man Pages, Documents */
	c(auto_shared,     "doc", "COPYING", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "LICENSE.libdkim", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "LICENSE.qhpsi", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "LICENSE.GPL-2.libsrs2", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "softwarelicense1-1.html", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "CREDITS", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "INSTALL-MINI", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "INSTALL.alias", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "INSTALL.control", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "INSTALL.maildir", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "INSTALL.mbox", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "INSTALL.vsm", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.licenses", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.clamav", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.greylist", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.logselect", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.srs", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.surbl", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.filters", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.recipients", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.indimail-mta", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.qhpsi", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.qregex", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.queue-fix", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.status", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.libdkim", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "README.EXTTODO", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "AUTHORS.libdkim", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "AUTHORS.libsrs2", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "FROMISP", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "TOISP", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "AUTOTURN", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "INTERNALS", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.local2alias", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.local2ext", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.local2local", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.local2rem", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.local2virt", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.nullclient", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.relaybad", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.relaygood", auto_uido, 0, 0444);
	c(auto_shared,     "doc", "PIC.rem2local", auto_uido, 0, 0444);

#ifdef HASTLSA
	c(mandir,          "man/man1", "qdane.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "dnstlsarr.1", uidr, gidr, moder_f);
#endif
	c(mandir,          "man/man1", "cidr.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "qmail-cat.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "predate.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "datemail.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "argv0.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "addcr.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "delcr.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "fixcrio.1", uidr, gidr, moder_f);
	c(mandir,          "man/man1", "maildirsize.1", uidr, gidr, moder_f);
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
	c(mandir,          "man/man1", "inotify.1", uidr, gidr, moder_f);
#ifdef HAVESRS
	c(mandir,          "man/man1", "srsfilter.1", uidr, gidr, moder_f);
#endif
#ifdef USE_SPF
	c(mandir,          "man/man1", "spfquery.1", uidr, gidr, moder_f);
#endif

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
#ifdef HAVESRS
	c(mandir,          "man/man5", "qmail-srs.5", uidr, gidr, moder_f);
#endif

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
	c(mandir,          "man/man8", "svctool.8", uidr, gidr, moder_f);

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
#ifdef HASTLSA
	c(mandir,          "man/man8", "qmail-daned.8", uidr, gidr, moder_f);
#endif
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

	if (!dev_package)
		return;
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
#ifdef HASTLSA
	c(mandir,          "man/man3", "tlsacheck.3", uidr, gidr, moder_f);
#endif
	c(mandir,          "man/man3", "greylist.3", uidr, gidr, moder_f);
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
	static char    *x = "$Id: hier.c,v 1.254 2019-02-21 14:14:22+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
