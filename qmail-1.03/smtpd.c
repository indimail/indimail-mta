/*
 * $Log: smtpd.c,v $
 * Revision 1.173  2013-08-06 11:15:25+05:30  Cprogrammer
 * use SPFIPV6 env variable to use spf ipv6 code
 *
 * Revision 1.172  2013-07-16 20:27:14+05:30  Cprogrammer
 * fix size getting clobbered by strnum in log_spam() function
 *
 * Revision 1.171  2013-06-09 17:03:49+05:30  Cprogrammer
 * shortened variable declartion in addrparse() function
 *
 * Revision 1.170  2013-05-16 23:33:16+05:30  Cprogrammer
 * added checkrecipient as part of non-indimail code
 *
 * Revision 1.169  2013-01-02 08:59:12+05:30  Cprogrammer
 * use FORCE_TLS env variable to enforce STARTTLS during AUTH
 *
 * Revision 1.168  2012-06-26 19:17:21+05:30  Cprogrammer
 * fix infinite loop in blast() function on EOF
 *
 * Revision 1.167  2012-04-26 18:06:35+05:30  Cprogrammer
 * removed memory leaks
 *
 * Revision 1.166  2012-04-10 20:32:39+05:30  Cprogrammer
 * use ipv4 address for spf
 *
 * Revision 1.165  2012-01-19 16:40:44+05:30  Cprogrammer
 * allow restricting of MASQUERADE to the value set by MASQUERADE env variable
 *
 * Revision 1.164  2011-12-22 12:02:30+05:30  Cprogrammer
 * use AUTH methods defines from indimail.h
 *
 * Revision 1.163  2011-12-18 11:17:25+05:30  Cprogrammer
 * fixed abnormal exit in DIGEST-MD5 for non-existing users
 *
 * Revision 1.162  2011-12-10 15:23:20+05:30  Cprogrammer
 * added CRAM-SHA256 auth
 *
 * Revision 1.161  2011-11-17 20:31:09+05:30  Cprogrammer
 * handle the case when recipients.cdb is not present
 *
 * Revision 1.160  2011-11-16 16:19:49+05:30  Cprogrammer
 * fix errStr, errstr variables
 *
 * Revision 1.159  2011-11-07 09:32:17+05:30  Cprogrammer
 * removed unused variable qop
 *
 * Revision 1.158  2011-10-29 20:40:58+05:30  Cprogrammer
 * compute len directly instead of using str_len()
 *
 * Revision 1.157  2011-10-28 17:58:09+05:30  Cprogrammer
 * added AUTH CRAM-SHA1, CRAM-RIPEMD, DIGEST-MD5
 *
 * Revision 1.156  2011-10-25 21:16:20+05:30  Cprogrammer
 * option to disable LOGIN, PLAIN, CRAM-MD5 using env variables
 *
 * Revision 1.155  2011-07-29 09:30:05+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.154  2011-07-08 22:04:00+05:30  Cprogrammer
 * added logging of plugin errors to fd 2
 *
 * Revision 1.153  2011-07-08 13:48:08+05:30  Cprogrammer
 * execute plugins towards the end of MAIL, RCPT, DATA functions
 *
 * Revision 1.152  2011-04-17 18:31:32+05:30  Cprogrammer
 * multiple plugin feature
 *
 * Revision 1.151  2011-04-16 14:43:47+05:30  Cprogrammer
 * spamfd can be confugured using SPAMFD env variable
 *
 * Revision 1.150  2011-04-13 21:27:15+05:30  Cprogrammer
 * added env variable DISABLE_PLUGIN to disable loading of smtp plugins
 *
 * Revision 1.149  2011-04-13 20:45:21+05:30  Cprogrammer
 * added smtp plugin feature
 *
 * Revision 1.148  2010-11-05 06:28:45+05:30  Cprogrammer
 * moved mail_acl() function to mail_acl.c
 *
 * Revision 1.147  2010-10-06 21:59:24+05:30  Cprogrammer
 * fixed wrong return value passed to log_rules()
 *
 * Revision 1.146  2010-09-11 15:26:25+05:30  Cprogrammer
 * reopen all setup control files after envrules
 *
 * Revision 1.145  2010-09-11 10:20:50+05:30  Cprogrammer
 * fixed few environment variables not geting set by envrules
 *
 * Revision 1.144  2010-07-13 16:26:50+05:30  Cprogrammer
 * fix accesslist problems occurring with multiple rcpt
 *
 * Revision 1.143  2010-07-02 16:18:29+05:30  Cprogrammer
 * new function msg_notify() for short email notifications to recipients
 * notify recipients when message size exceeds databyte limits
 *
 * Revision 1.142  2010-04-29 12:09:53+05:30  Cprogrammer
 * fixed cdb search for control files badip, badhelo, authdomains & chkrcptdomains
 *
 * Revision 1.141  2010-04-23 19:20:52+05:30  Cprogrammer
 * added badip functionality
 * fixed setup_state with badhost functionality
 *
 * Revision 1.140  2010-03-30 16:43:44+05:30  Cprogrammer
 * fix for IPV6 hosts
 *
 * Revision 1.139  2010-03-30 16:25:32+05:30  Cprogrammer
 * use ipv4 address for greylisting
 *
 * Revision 1.138  2010-03-30 09:09:12+05:30  Cprogrammer
 * greylisting error was being sent to client instead of log files
 *
 * Revision 1.137  2010-02-10 08:59:41+05:30  Cprogrammer
 * trivial change for #ifdef INDIMAIL
 *
 * Revision 1.136  2010-01-20 11:26:46+05:30  Cprogrammer
 * new improved logic for accesslist
 *
 * Revision 1.135  2009-12-05 19:47:54+05:30  Cprogrammer
 * ansic conversion
 *
 * Revision 1.134  2009-12-05 11:26:43+05:30  Cprogrammer
 * added badhost check
 * display value of TCPPARANOID, REQPTR in error messages
 *
 * Revision 1.133  2009-11-12 19:28:50+05:30  Cprogrammer
 * do antispoofing in smtp_mail()
 * bypass antispoofing if relayclient is set
 *
 * Revision 1.132  2009-09-08 12:34:21+05:30  Cprogrammer
 * removed dependency of INDIMAIL on spam filtering
 *
 * Revision 1.131  2009-09-07 13:56:54+05:30  Cprogrammer
 * disable sqlmatch if INDIMAIL is not defined
 *
 * Revision 1.130  2009-09-05 22:47:55+05:30  Cprogrammer
 * made servercert, clientca, clientca filename configurable using
 * environment varables SERVERCERT, CLIENTCA, CLIENTCRL
 *
 * Revision 1.129  2009-09-01 21:58:36+05:30  Cprogrammer
 * added Bounce Address Tag Validation (BATV)
 *
 * Revision 1.128  2009-08-24 11:06:06+05:30  Cprogrammer
 * added greylisting
 *
 * Revision 1.127  2009-08-05 14:47:08+05:30  Cprogrammer
 * removed extra flush() function calls
 *
 * Revision 1.126  2009-05-26 12:23:01+05:30  Cprogrammer
 * added PTR check and Paranoid check
 *
 * Revision 1.125  2009-05-03 22:47:04+05:30  Cprogrammer
 * restore_env() now returns void
 *
 * Revision 1.124  2009-05-01 12:46:05+05:30  Cprogrammer
 * use constants from qregex.h for envrules return status
 *
 * Revision 1.123  2009-05-01 12:41:05+05:30  Cprogrammer
 * removed exit() from err_addressmatch()
 *
 * Revision 1.122  2009-05-01 10:43:12+05:30  Cprogrammer
 * added errstr argument to address_match(), matchregex() and envrules()
 *
 * Revision 1.121  2009-04-30 22:18:09+05:30  Cprogrammer
 * removed chkrcptdomains from cdb and MySQL
 *
 * Revision 1.120  2009-04-30 18:51:22+05:30  Cprogrammer
 * removed authdomains from cdb and MySQL
 *
 * Revision 1.119  2009-04-29 21:03:32+05:30  Cprogrammer
 * check address_match() for failure
 *
 * Revision 1.118  2009-04-29 16:22:48+05:30  Cprogrammer
 * bypass badrcptto, badrcptpatterns for addresses listed in goodrcptto, goodrcptpatterns
 *
 * Revision 1.117  2009-04-29 15:16:32+05:30  Cprogrammer
 * goodrcptto overrides badrcpto
 *
 * Revision 1.116  2009-04-29 10:34:36+05:30  Cprogrammer
 * added cdb lookup for spamignore, blackholedsender, badmailfrom,relaymailfrom,authdomains,
 * badrcptto,chkrcptdomains,goodrcptto,blackholedrcpt
 *
 * Revision 1.115  2009-04-17 21:09:53+05:30  Cprogrammer
 * added SMTP error message for temporary auth failure
 *
 * Revision 1.114  2009-04-14 11:31:38+05:30  Cprogrammer
 * set rcptcount to zero in each session
 *
 * Revision 1.113  2009-04-10 12:32:00+05:30  Cprogrammer
 * added RECIPIENTS extension (0.5.19) by Erwin Hoffmann
 *
 * Revision 1.112  2009-03-30 21:42:31+05:30  Cprogrammer
 * added #ifdef for goodrcptto
 *
 * Revision 1.111  2009-03-30 16:33:02+05:30  Cprogrammer
 * added goodrcptto, goodrcptpatterns
 *
 * Revision 1.110  2009-03-21 12:36:52+05:30  Cprogrammer
 * conditional compilation of check_recipient code
 *
 * Revision 1.109  2009-03-11 20:21:40+05:30  Cprogrammer
 * modified checkrecipient extension
 *
 * Revision 1.108  2009-01-06 20:45:34+05:30  Cprogrammer
 * added greetdelay functionality
 * fixed addrparse() function
 * fixed sigscheck()
 *
 * Revision 1.107  2008-07-15 20:06:14+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.106  2008-06-12 08:40:46+05:30  Cprogrammer
 * added rulesfile argument
 *
 * Revision 1.105  2008-06-03 22:03:38+05:30  Cprogrammer
 * reject helo if relayclient is not set and helo hostname is a local host
 * reject if errors equals/exceeds MAX_RCPT_ERRCOUNT
 *
 * Revision 1.104  2008-06-01 15:39:52+05:30  Cprogrammer
 * Frederik Vermeulen <qmail-tls akrul inoa.net> 20070408Frederik Vermeulen <qmail-tls akrul inoa.net> 20070408 TLS patch
 *
 * Revision 1.103  2007-12-21 13:58:06+05:30  Cprogrammer
 * fix null termination bug of address
 *
 * Revision 1.102  2007-12-20 13:51:13+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.101  2006-03-02 20:46:28+05:30  Cprogrammer
 * initialize message size for each SMTP session
 *
 * Revision 1.100  2005-12-29 14:01:53+05:30  Cprogrammer
 * prevent NULL rcptto
 *
 * Revision 1.99  2005-08-25 23:58:49+05:30  Cprogrammer
 * gcc 4 compliance
 * BUG Fix - QREGEX was by default enabled
 * avoid confusion with precedence in mailfrom_size()
 *
 * Revision 1.98  2005-06-17 21:50:36+05:30  Cprogrammer
 * replaced struct ip_address with a shorter typedef
 *
 * Revision 1.97  2005-06-11 21:27:06+05:30  Cprogrammer
 * ipv6 address support
 * accept mails only from email having fqdn
 *
 * Revision 1.96  2005-05-31 15:46:30+05:30  Cprogrammer
 * fixed null termination problem with tls code
 *
 * Revision 1.95  2005-04-05 12:05:36+05:30  Cprogrammer
 * reject mails if OPENRELAY is set
 *
 * Revision 1.94  2005-04-02 23:06:23+05:30  Cprogrammer
 * replaced wildmat with wildmat_internal
 *
 * Revision 1.93  2005-02-14 23:08:07+05:30  Cprogrammer
 * added qregex control file
 * added RFC 3848
 * use regex/wildmat for matching senders/recipients in accesslist control file
 *
 * Revision 1.92  2005-01-22 00:39:37+05:30  Cprogrammer
 * BUG - Fix for ANTISPOOFING
 *
 * Revision 1.91  2005-01-18 13:48:53+05:30  Cprogrammer
 * BUG - Added case for NO_RELAY for LOGIN, PLAIN, CRAM-MD5 authentication
 *
 * Revision 1.90  2005-01-17 16:05:37+05:30  Cprogrammer
 * prevent DOS in CUGMAIL
 * added new case in smtp_auth() for NO_RELAY
 *
 * Revision 1.89  2004-10-22 20:11:12+05:30  Cprogrammer
 * removed readwrite.h
 * added RCS id
 * standardized "Unable to queue messages"
 *
 * Revision 1.88  2004-10-11 14:20:42+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.87  2004-10-09 23:32:38+05:30  Cprogrammer
 * BUG Fixes - Corrected typos
 *
 * Revision 1.86  2004-09-26 00:00:59+05:30  Cprogrammer
 * added access list for email transactions
 *
 * Revision 1.85  2004-09-22 22:27:21+05:30  Cprogrammer
 * replaced atoi() with scan_int/scan_ulong
 *
 * Revision 1.84  2004-09-22 16:55:53+05:30  Cprogrammer
 * added recipient extension
 * expanded meaning of VIRUSCHECK environment variable
 *
 * Revision 1.83  2004-08-28 01:04:50+05:30  Cprogrammer
 * added message submission port
 *
 * Revision 1.82  2004-08-14 02:27:35+05:30  Cprogrammer
 * removed compilation warning
 *
 * Revision 1.81  2004-08-14 01:47:31+05:30  Cprogrammer
 * added SPF code
 *
 * Revision 1.80  2004-08-13 00:15:50+05:30  Cprogrammer
 * blackhole code for virus check and content filter
 *
 * Revision 1.79  2004-07-30 18:46:56+05:30  Cprogrammer
 * removed sig_alarmcatch() as it is handled in ssl_timeoutio
 *
 * Revision 1.78  2004-07-30 18:40:46+05:30  Cprogrammer
 * TLS code overhaul - Fredrik Vermeulen 20040519
 *
 * Revision 1.77  2004-07-13 22:42:22+05:30  Cprogrammer
 * added control file maxcmdlen - max length a smtp command may have
 *
 * Revision 1.76  2004-06-15 22:47:32+05:30  Cprogrammer
 * concatenate split header lines before passing it to bodycheck()
 *
 * Revision 1.75  2004-05-29 21:04:59+05:30  Cprogrammer
 * chkusrdomains renamed to chkrcptdomains
 * CHECKSENDER renamed to CUGMAIL
 *
 * Revision 1.74  2004-05-27 22:59:25+05:30  Cprogrammer
 * fixed problem of Null string environment variables
 * made badmailpatterns, badrcptto, badrcptpatterns,
 * spamignore, spamignorepatterns, signatures, bodycheck
 * configurable through environment variables
 *
 * Revision 1.73  2004-05-26 09:34:03+05:30  Cprogrammer
 * ability to run sigscheck selectively on header/body/both
 *
 * Revision 1.72  2004-05-23 22:18:05+05:30  Cprogrammer
 * added envrules filename as argument
 *
 * Revision 1.71  2004-05-19 23:14:58+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.70  2004-05-19 11:49:04+05:30  Cprogrammer
 * added host access control
 *
 * Revision 1.69  2004-05-07 10:30:31+05:30  Cprogrammer
 * use original environment variables in each session. - envrules bug
 *
 * Revision 1.68  2004-04-29 22:41:20+05:30  Cprogrammer
 * multiline greeting capability added
 * unset NULLQUEUE before starting mail session
 *
 * Revision 1.67  2004-03-17 12:17:25+05:30  Cprogrammer
 * fixed bug with virus scanning and pattern scanning with multiple
 * transaction in a single SMTP session
 *
 * Revision 1.66  2004-02-17 14:07:49+05:30  Cprogrammer
 * added envrules
 * added bodycheck
 * check real domain in authenticated smtp
 *
 * Revision 1.65  2004-01-20 06:56:02+05:30  Cprogrammer
 * give syntax error if arg passed to rset
 *
 * Revision 1.64  2004-01-20 02:11:21+05:30  Cprogrammer
 * ISO C conformance
 *
 * Revision 1.63  2004-01-20 01:53:37+05:30  Cprogrammer
 * removed badheader code
 * prevent overflow of pos variable in blast()
 * fixed call to err_nogateway()
 * replaced helocheck() with address_match()
 *
 * Revision 1.62  2004-01-10 16:10:43+05:30  Cprogrammer
 * incorporated Erwin Hoffman's changes to sizelimit()
 * added Paul Gregg's rejection of HELO commands without dot
 *
 * Revision 1.61  2004-01-08 00:32:24+05:30  Cprogrammer
 * use TMPDIR environment variable for temporary directory
 * send spam to central spam logger
 *
 * Revision 1.60  2004-01-05 19:23:34+05:30  Cprogrammer
 * tmpFile was not initialized
 *
 * Revision 1.59  2004-01-04 23:16:23+05:30  Cprogrammer
 * standardization of error reporting for rsmtp reporting scripts
 *
 * Revision 1.58  2003-12-30 00:35:53+05:30  Cprogrammer
 * use address_match() generic functions to search control files
 *
 * Revision 1.57  2003-12-25 15:38:12+05:30  Cprogrammer
 * blackhole code fools the client that message has been accepted
 * added anti-spoofing code
 *
 * Revision 1.56  2003-12-24 17:28:17+05:30  Cprogrammer
 * changed isspam() to cdbstr()
 *
 * Revision 1.55  2003-12-22 18:32:25+05:30  Cprogrammer
 * moved regex match routines to qregex.c
 * renamed pattern_find() to address_match()
 *
 * Revision 1.54  2003-12-22 10:04:31+05:30  Cprogrammer
 * generalized function pattern_find()
 *
 * Revision 1.53  2003-12-20 13:53:43+05:30  Cprogrammer
 * added qregex for pattern functionality in badmailfrom
 * and badrcptto
 * changed atoi to strtol
 *
 * Revision 1.52  2003-12-20 01:49:25+05:30  Cprogrammer
 * added checking for spamdb, spamdb.cdb file
 * use stralloc for preparing control file
 *
 * Revision 1.51  2003-12-15 13:50:31+05:30  Cprogrammer
 * renamed QMAILFILTER to SPAMFILTER
 *
 * Revision 1.50  2003-12-07 13:07:25+05:30  Cprogrammer
 * conditional compilation for INDIMAIL
 *
 * Revision 1.49  2003-11-29 23:45:09+05:30  Cprogrammer
 * spamignore control file added
 *
 * Revision 1.48  2003-10-30 00:15:13+05:30  Cprogrammer
 * vclose() did not get called if atrn was rejected
 *
 * Revision 1.47  2003-10-28 20:02:49+05:30  Cprogrammer
 * conditional compilation for INDIMAIL
 *
 * Revision 1.46  2003-10-27 00:52:03+05:30  Cprogrammer
 * added CHECKSENDER code
 *
 * Revision 1.45  2003-10-23 01:28:21+05:30  Cprogrammer
 * modified err_queue() to log full transaction
 * fixed compilation warnings
 *
 * Revision 1.44  2003-10-18 18:33:08+05:30  Cprogrammer
 * removed SPAMTHROTTLE
 *
 * Revision 1.43  2003-10-18 00:14:32+05:30  Cprogrammer
 * close mysql connections in smtp_atrn()
 *
 * Revision 1.42  2003-10-17 21:13:58+05:30  Cprogrammer
 * added ETRN, ATRN capability in help
 *
 * Revision 1.41  2003-10-16 01:21:18+05:30  Cprogrammer
 * corrected printing of negative queue suffix values in queueforward
 *
 * Revision 1.40  2003-10-13 18:31:16+05:30  Cprogrammer
 * made turning on of badhelo through env variable
 * LOGFILTER to turn on additional status retrieval from QMAILQUEUE programs
 *
 * Revision 1.39  2003-10-11 00:13:44+05:30  Cprogrammer
 * added badhelo control file
 * corrected call to vshow_atrn_map()
 *
 * Revision 1.38  2003-10-03 11:49:31+05:30  Cprogrammer
 * moved qforward code to qforward.c
 * addrparse() function bug
 *
 * Revision 1.37  2003-09-29 14:09:37+05:30  Cprogrammer
 * fd 3 was wrongly closed causing failure if morercpthosts was opened
 *
 * Revision 1.36  2003-09-27 21:12:12+05:30  Cprogrammer
 * change in rcpthosts() function
 *
 * Revision 1.35  2003-09-21 18:55:15+05:30  Cprogrammer
 * change in comment
 *
 * Revision 1.34  2003-09-16 18:00:56+05:30  Cprogrammer
 * correctness of RFC
 *
 * Revision 1.33  2003-07-20 17:16:12+05:30  Cprogrammer
 * added startls
 * corrected reading of badheaders
 * env variable REQUIREAUTH added
 *
 * Revision 1.32  2003-07-10 15:38:48+05:30  Cprogrammer
 * RFC-2554, RFC-2222 compliance
 *
 * Revision 1.31  2003-07-07 00:06:19+05:30  Cprogrammer
 * added list of HELP in ehlo
 *
 * Revision 1.30  2003-07-05 18:16:07+05:30  Cprogrammer
 * added odmr, RFC 2645 support
 *
 * Revision 1.29  2003-06-18 01:24:38+05:30  Cprogrammer
 * added code to forward bounces
 * added AUTH_DOMAINS functionality
 *
 * Revision 1.28  2003-06-09 22:47:42+05:30  Cprogrammer
 * added new spam control features
 * badheaders, queueforward, blackholedpatterns
 *
 * Revision 1.27  2002-12-11 00:09:20+05:30  Cprogrammer
 * added termination check
 *
 * Revision 1.26  2002-11-24 16:11:12+05:30  Cprogrammer
 * check relaying for all domains if AUTH_ALL is set
 *
 * Revision 1.25  2002-10-29 00:24:21+05:30  Cprogrammer
 * added control files maxrecipients and blackholedsender
 *
 * Revision 1.24  2002-10-25 17:03:04+05:30  Cprogrammer
 * made name of badmailfrom configurable
 *
 * Revision 1.23  2002-10-19 17:59:10+05:30  Cprogrammer
 * added missing else
 *
 * Revision 1.22  2002-09-30 22:57:18+05:30  Cprogrammer
 * included spamdef.h
 *
 * Revision 1.21  2002-09-15 15:19:55+05:30  Cprogrammer
 * added datetime.h
 *
 * Revision 1.20  2002-09-11 15:41:24+05:30  Cprogrammer
 * added option to masquerade for authenticated SMTP
 *
 * Revision 1.19  2002-09-10 20:42:49+05:30  Cprogrammer
 * corrections in DSNs
 *
 * Revision 1.18  2002-09-10 20:06:37+05:30  Cprogrammer
 * added env variable SHUTDOWN to indicate shutdown
 * initialize during helo command
 * removed redundant code under #ifdef VIOLATE_RFC
 *
 * Revision 1.17  2002-09-09 01:29:42+05:30  Cprogrammer
 * error messages changed to match with RFC 1893 DSNs
 *
 * Revision 1.16  2002-09-04 20:25:40+05:30  Cprogrammer
 * enhanced smtp logging
 * added code to allow aliases in User Status Check
 *
 * Revision 1.15  2002-09-04 04:31:38+05:30  Cprogrammer
 * do not advertise auth if correct arguments are not supplied
 *
 * Revision 1.14  2002-09-04 01:50:30+05:30  Cprogrammer
 * conditional compilation of indimail code
 * added selective checking of user status for domains through control file chkusrdomains
 * set remotehost to remoteip if REMOTEHOST environment variable is not set
 *
 * Revision 1.13  2002-09-02 17:57:40+05:30  Cprogrammer
 * removed stupid oops in error messages
 * removed unused variables
 * put parenthesis around b64decode() return values
 *
 * Revision 1.12  2002-09-02 16:44:05+05:30  Cprogrammer
 * organized global variables
 * enhanced log_trans to log the auth method
 *
 * Revision 1.11  2002-08-31 16:26:40+05:30  Cprogrammer
 * added case -1 in smtp_auth
 * added time in smtp greeting
 *
 * Revision 1.10  2002-08-31 14:24:02+05:30  Cprogrammer
 * missing break statement added
 *
 * Revision 1.9  2002-08-30 23:33:35+05:30  Cprogrammer
 * added more error messages
 *
 * Revision 1.8  2002-08-25 19:45:27+05:30  Cprogrammer
 * etrn support modified
 * dnscheck() function modified
 *
 * Revision 1.7  2002-08-25 03:27:51+05:30  Cprogrammer
 * added etrn support
 *
 * Revision 1.6  2002-08-15 19:43:46+05:30  Cprogrammer
 * changes for configurable control dir
 *
 * Revision 1.5  2002-08-15 00:50:12+05:30  Cprogrammer
 * do not print size when databytes is 0
 *
 * Revision 1.4  2002-07-14 10:36:22+05:30  Cprogrammer
 * extensive RFC support added
 *
 * Revision 1.3  2002-04-17 13:59:40+05:30  Cprogrammer
 * added maxhops control file
 * changed err_log() to log_trans()
 *
 * Revision 1.2  2002-04-09 13:53:21+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2002-04-08 04:46:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "sig.h"
#include "stralloc.h"
#include "substdio.h"
#include "alloc.h"
#include "auto_qmail.h"
#include "control.h"
#include "received.h"
#include "constmap.h"
#include "case.h"
#include "error.h"
#include "ipme.h"
#include "ip.h"
#include "qmail.h"
#include "str.h"
#include "fmt.h"
#include "scan.h"
#include "byte.h"
#include "env.h"
#include "now.h"
#include "exit.h"
#include "rcpthosts.h"
#include "recipients.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "commands.h"
#include "wait.h"
#include "fd.h"
#include "dns.h"
#include "etrn.h"
#include "datetime.h"
#include "date822fmt.h"
#include "base64.h"
#include "greylist.h"
#include "variables.h"
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "hasindimail.h"
#include "envrules.h"
#include "matchregex.h"
#include "tablematch.h"
#include "bodycheck.h"
#include "getln.h"
#include "qregex.h"
#include "mail_acl.h"
#ifdef TLS
#include "tls.h"
#include "ssl_timeoutio.h"
#endif
#ifdef USE_SPF
#include "spf.h"
#endif
#ifndef INDIMAIL
#include "auth_cram.h"
#endif

#ifdef BATV
#include "cdb.h"
#define BATVLEN 3		/* number of bytes */
#define BATVSTALE 7		/* accept for a week */
#include <openssl/md5.h>
#endif
#ifdef SMTP_PLUGIN
#include "smtp_plugin.h"
#include <dlfcn.h>
#endif

#define MAXHOPS   100
#define SMTP_PORT  25
#define ODMR_PORT 366 /*- On Demand Mail Relay Protocol RFC 2645 */
#define SUBM_PORT 587 /*- Message Submission Port RFC 2476 */

#ifdef TLS
void            tls_init();
int             tls_verify();
void            tls_nogateway();
#endif
ssize_t         safewrite(int, char *, int);
ssize_t         saferead(int, char *, int);
int             auth_login(char *);
int             auth_plain(char *);
int             auth_cram_md5();
int             auth_cram_sha1();
int             auth_cram_sha256();
int             auth_cram_ripemd();
int             auth_digest_md5();
int             err_noauth();
int             addrrelay();
void            smtp_greet(char *);
int             atrn_queue(char *, char *);
int             wildmat_internal(char *, char *);

#ifdef TLS
int             ssl_rfd = -1, ssl_wfd = -1;	/*- SSL_get_Xfd() are broken */
char           *servercert, *clientca, *clientcrl;
#endif
char           *revision = "$Revision: 1.173 $";
char           *protocol = "SMTP";
stralloc        proto = { 0 };
static stralloc Revision = { 0 };
static stralloc greeting = { 0 };
#ifdef USE_SPF
stralloc        spflocal = { 0 };
stralloc        spfguess = { 0 };
stralloc        spfexp = { 0 };
int             flagbarfspf;
unsigned int    spfbehavior = 0;
unsigned int    spfipv6 = 0;
static stralloc spfbarfmsg = { 0 };
#endif
stralloc        helohost = { 0 };
stralloc        addr = { 0 }; /*- will be 0-terminated, if addrparse returns 1 */
static stralloc liphost = { 0 };
static stralloc mailfrom = { 0 };
static stralloc rcptto = { 0 };

static stralloc authin = { 0 };
static stralloc user = { 0 };
static stralloc pass = { 0 };
static stralloc resp = { 0 };
static stralloc slop = { 0 };

#ifdef BATV
stralloc        signkey = {0};
stralloc        nosign = {0};
char            batvok;
int             signkeystale = 7; /*- accept signkey for a week */
struct constmap mapnosign;
char            isbounce;
#endif

/*- SpaceNet - maex */
static char     strnum[FMT_ULONG];
static char     accept_buf[FMT_ULONG];

char           *remoteip, *remotehost, *remoteinfo, *local, *relayclient,
			   *nodnscheck, *msgsize, *fakehelo, *hostname, *bouncemail, *requireauth,
			   *localip, *greyip;
#ifdef IPV6
char           *remoteip4;
#endif
char          **childargs;

static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(saferead, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);
static char     sserrbuf[512];
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
static char     upbuf[128];
static substdio ssup;

unsigned long   databytes = 0;
unsigned long   msg_size = 0;
unsigned long   BytesToOverflow = 0;

int             liphostok = 0;
int             maxhops = MAXHOPS;
int             ctl_maxcmdlen;	  /*- max length a smtp command may have */
int             timeout = 1200;
int             authd = 0;
int             seenhelo = 0;
int             authenticated;
int             seenmail = 0;
int             setup_state = 0;
int             rcptcount;
int             dsn;
int             qregex = 0;
int             rcpt_errcount = 0;
int             max_rcpt_errcount = 1;

struct qmail    qqt;

int             greetdelay = 0;
char           *errStr = 0;
/*- badmailfrom */
int             bmfok = 0;
static stralloc bmf = { 0 };
struct constmap mapbmf;
int             bmpok = 0;
static stralloc bmp = { 0 };
char           *bmfFn = 0;
/*- blackholedrcpt */
int             bhrcpok = 0;
static stralloc bhrcp = { 0 };
struct constmap mapbhrcp;
int             bhbrpok = 0;
static stralloc bhbrp = { 0 };
char           *bhrcpFn = 0;
/*- BLACKHOLE Sender Check Variables */
int             bhfok = 0;
stralloc        bhf = { 0 };
struct constmap mapbhf;
int             bhpok = 0;
static stralloc bhp = { 0 };
char           *bhsndFn = 0;
/*- badrcptto */
int             rcpok = 0;
static stralloc rcp = { 0 };
struct constmap maprcp;
int             brpok = 0;
static stralloc brp = { 0 };
char           *rcpFn = 0;
/*- accesslist */
int             acclistok = 0;
static stralloc acclist = { 0 };
/*- RELAYCLIENT Check Variables */
int             relayclientsok = 0;
static stralloc relayclients = { 0 };
struct constmap maprelayclients;
/*- RELAYDOMAIN Check Variables */
int             relaydomainsok = 0;
static stralloc relaydomains = { 0 };
struct constmap maprelaydomains;
/*- RELAYMAILFROM Check Variables */
int             rmfok = 0;
static stralloc rmf = { 0 };
struct constmap maprmf;
/*- NODNSCHECK Check Variables */
int             nodnschecksok = 0;
static stralloc nodnschecks = { 0 };
struct constmap mapnodnschecks;
/*- badip Check */
char           *dobadipcheck = (char *) 0;
char           *badipfn = (char *) 0;
int             briok = 0;
stralloc        bri = {0};
stralloc        ipaddr = {0};
struct constmap mapbri;
/*- badhost Check */
char           *dobadhostcheck = (char *) 0;
int             brhok = 0;
stralloc        brh = {0};
struct constmap mapbrh;
/*- Helo Check */
char           *dohelocheck = (char *) 0;
char           *badhelofn = (char *) 0;
int             badhelook = 0;
stralloc        badhelo = { 0 };
struct constmap maphelo;
/*- authdomains */
int             chkdomok;
static stralloc chkdom = { 0 };
struct constmap mapchkdom;
/*- goodrcptto */
int             chkgrcptok = 0;
static stralloc grcpt = { 0 };
struct constmap mapgrcpt;
int             chkgrcptokp = 0;
static stralloc grcptp = { 0 };
char           *grcptFn = 0;
/*- SPAM Ingore Sender Check Variables */
int             spfok = 0;
stralloc        spf = { 0 };
struct constmap mapspf;
int             sppok = 0;
static stralloc spp = { 0 };
char           *spfFn = 0;
/*-
 * check recipients using inquery
 * chkrcptdomains
 */
int             chkrcptok = 0;
static stralloc chkrcpt = { 0 };
struct constmap mapchkrcpt;
/*- TARPIT Check Variables */
int             tarpitcount = 0;
int             tarpitdelay = 5;
/*- MAXRECIPIENTS Check Variable */
int             maxrcptcount = 0;
/*- Russel Nelson's Virus Patch */
int             sigsok = 0;
int             sigsok_orig = 0;
stralloc        sigs = { 0 };
char           *virus_desc;
int             bodyok = 0;
int             bodyok_orig = 0;
stralloc        body = { 0 };
char           *content_desc;
#ifdef SMTP_PLUGIN
PLUGIN         **plug = (PLUGIN **) 0;
void           **handle;
int              plugin_count;
#endif
int             spamfd = 255;

struct authcmd
{
	char           *text;
	int             (*fun) ();
} authcmds[] =
{
	{"login", auth_login},
	{"plain", auth_plain},
	{"cram-md5", auth_cram_md5},
	{"cram-sha1", auth_cram_sha1},
	{"cram-sha256", auth_cram_sha256},
	{"cram-ripemd", auth_cram_ripemd},
	{"digest-md5", auth_digest_md5},
	{0, err_noauth}
};

int             smtp_port;

extern char   **environ;

/*- dummy function for mail_acl */
void
die_qregex(char *str)
{
	return;
}

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
logerrpid()
{
	strnum[fmt_ulong(strnum, getpid())] = 0;
	logerr("pid ");
	logerr(strnum);
	logerr(" from ");
}

ssize_t
safewrite(int fd, char *buf, int len)
{
	int             r;

#ifdef TLS
	if (ssl && fd == ssl_wfd)
		r = ssl_timeoutwrite(timeout, ssl_rfd, ssl_wfd, ssl, buf, len);
	else
		r = timeoutwrite(timeout, fd, buf, len);
#else
	r = timeoutwrite(timeout, fd, buf, len);
#endif
	if (r <= 0)
		_exit(1);
	return r;
}

void
flush()
{
	substdio_flush(&ssout);
}

void
out(char *s)
{
	substdio_puts(&ssout, s);
}

void
die_nohelofqdn(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" non-FQDN HELO: ");
	logerr(arg);
	logerrf("\n");
	out("451 unable to accept non-FQDN HELO (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_lcmd()
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" command t00 long!\n");
	out("553 sorry, the given command is t00 long! (#5.5.2)\r\n");
	flush();
	_exit(1);
}

void
die_read()
{
	out("451 Requested action aborted: read error (#4.4.2)\r\n");
	flush();
	_exit(1);
}

void
die_alarm()
{
	out("451 Requested action aborted: timeout (#4.4.2)\r\n");
	flush();
	_exit(1);
}

void
die_regex()
{
	out("451 Requested action aborted: regex compilation failed (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_nomem()
{
	out("451 Requested action aborted: out of memory (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_control()
{
	out("451 Requested action aborted: unable to read controls (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_ipme()
{
	out("451 Requested action aborted: unable to figure out my IP addresses (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_plugin(char *arg1, char *arg2, char *arg3, char *arg4)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(": ");
	out("451 ");
	if (arg1)
	{
		logerr(arg1);
		out(arg1);
	}
	if (arg2)
	{
		logerr(arg2);
		out(arg2);
	}
	if (arg3)
	{
		logerr(arg3);
		out(arg3);
	}
	if (arg4)
	{
		logerr(arg4);
		out(arg4);
	}
	logerrf("\n");
	out(" (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_logfilter()
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" unable create temporary files: ");
	logerr(error_str(errno));
	logerrf("\n");
	out("451 Requested action aborted: unable to create temporary files (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
die_addressmatch(char *errstr)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" address_match: ");
	logerr(errstr);
	logerrf("\n");
	out("451 Requested action aborted: ");
	out(errstr);
	out(" (#4.3.0)\r\n");
	flush();
	_exit (1);
}

void
err_addressmatch(char *errstr)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" address_match: ");
	logerr(errstr);
	logerrf("\n");
	out("451 Requested action aborted: local system failure (#4.3.0)\r\n");
}

void
straynewline()
{
	out("451 Requested action aborted: Bare LF received. (#4.6.0)\r\n");
	flush();
	_exit(1);
}

int
addrallowed(char *rcpt)
{
	int             r;

	if ((r = rcpthosts(rcpt, str_len(rcpt), 0)) == -1)
		die_control();
#ifdef TLS
	if (r == 0 && tls_verify())
		r = -2;
#endif
	return r;
}

void
log_spam(char *arg1, char *arg2, unsigned long size, stralloc *line)
{
	int             logfifo, match;
	char           *fifo_name;
	struct stat     statbuf;
	static char     spambuf[256], inbuf[1024];
	static substdio spamin;
	static substdio spamout;

	if (!env_get("SPAMFILTER"))
		return;
	fifo_name = env_get("LOGFILTER");
	if (!fifo_name || !*fifo_name)
		return;
	if (*fifo_name != '/')
		return;
	if ((logfifo = open(fifo_name, O_NDELAY | O_WRONLY)) == -1)
	{
		if (errno == ENXIO)
			return;
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr(fifo_name);
		logerr(": ");
		logerr(error_str(errno));
		logerrf("\n");
		out("451 Unable to queue messages (#4.3.0)\r\n");
		flush();
		_exit(1);
	}
	/*-
	 * write the SMTP transaction line to LOGFILTER fifo. All lines written
	 * to this fifo will be read by the qmail-cat spamlogger service
	 */
	substdio_fdbuf(&spamout, write, logfifo, spambuf, sizeof(spambuf));
	if (substdio_puts(&spamout, "qmail-smtpd: ") == -1)
	{
		close(logfifo);
		return;
	}
	if (substdio_puts(&spamout, "pid ") == -1)
	{
		close(logfifo);
		return;
	}
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (substdio_puts(&spamout, strnum) == -1)
	{
		close(logfifo);
		return;
	}
	if (substdio_puts(&spamout, " MAIL from <") == -1)
	{
		close(logfifo);
		return;
	}
	if (substdio_puts(&spamout, arg1) == -1)
	{
		close(logfifo);
		return;
	}
	if (substdio_puts(&spamout, "> RCPT <") == -1)
	{
		close(logfifo);
		return;
	}
	if (substdio_puts(&spamout, arg2) == -1)
	{
		close(logfifo);
		return;
	}
	if (substdio_puts(&spamout, "> Size: ") == -1)
	{
		close(logfifo);
		return;
	}
	strnum[fmt_ulong(strnum, msg_size)] = 0;
	if (substdio_puts(&spamout, strnum) == -1)
	{
		close(logfifo);
		return;
	}
	/*
	 * Read X-Bogosity line from bogofilter on spamfd. spamfd would have already
	 * been opened before qmail_open() by create_logfiler() function
	 */
	if (!fstat(spamfd, &statbuf) && statbuf.st_size > 0 && !lseek(spamfd, 0, SEEK_SET))
	{
		if (substdio_puts(&spamout, " ") == -1)
		{
			close(logfifo);
			close(spamfd);
			return;
		}
		substdio_fdbuf(&spamin, read, spamfd, inbuf, sizeof(inbuf));
		if (getln(&spamin, line, &match, '\n') == -1)
		{
			logerr("qmail-smtpd: read error: ");
			logerr(error_str(errno));
			logerrf("\n");
			close(spamfd);
			return;
		}
		close(spamfd);
		if (!stralloc_0(line))
			die_nomem();
		if (line->len)
		{
			if (substdio_puts(&spamout, line->s) == -1)
			{
				logerr("qmail-smtpd: write error: ");
				logerr(error_str(errno));
				logerrf("\n");
			}
		}
	}
	if (substdio_puts(&spamout, "\n") == -1)
	{
		logerr("qmail-smtpd: write error: ");
		logerr(error_str(errno));
		logerrf("\n");
	}
	if (substdio_flush(&spamout) == -1)
	{
		close(logfifo);
		return;
	}
	close(logfifo);
	return;
}

void
log_trans(char *arg1, char *arg2, char *arg3, int len, char *arg4, int notify)
{
	char           *ptr;
	int             idx;
	stralloc        tmpLine = { 0 };

	for (ptr = arg3 + 1, idx = 0; idx < len; idx++)
	{
		if (!arg3[idx])
		{
			/*
			 * write data to spamlogger and get X-Bogosity line in tmpLine
			 */
			if (!notify)
				log_spam(arg2, ptr, msg_size, &tmpLine);
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(arg1);
			if (!notify)
			{
				logerr(" HELO <");
				logerr(helohost.s);
				logerr("> ");
			} else
				logerr(" NOTIFY: ");
			logerr("MAIL from <");
			logerr(arg2);
			logerr("> RCPT <");
			logerr(ptr);
			if (!notify)
			{
				logerr("> AUTH <");
				if (arg4 && *arg4)
				{
					logerr(arg4);
					switch (authd)
					{
					case 0:
						break;
					case 1:
						logerr(": AUTH LOGIN");
						break;
					case 2:
						logerr(": AUTH PLAIN");
						break;
					case 3:
						logerr(": AUTH CRAM-MD5");
						break;
					case 4:
						logerr(": AUTH CRAM-SHA1");
						break;
					case 5:
						logerr(": AUTH CRAM-RIPEMD");
						break;
					case 6:
						logerr(": AUTH DIGEST-MD5");
						break;
					default:
						logerr(": AUTH unknown");
						break;
					}
				}
				if (addrallowed(ptr))
				{
					if (arg4 && *arg4)
						logerr(": ");
					logerr("local-rcpt");
				} else
				if (!arg4 || !*arg4)
					logerr("pop-bef-smtp");
			}
			logerr("> Size: ");
			strnum[fmt_ulong(strnum, msg_size)] = 0;
			logerr(strnum);
			if (!notify && tmpLine.len)
			{
				logerr(" ");
				logerr(tmpLine.s);
			}
			logerr("\n");
			ptr = arg3 + idx + 2;
		}
	}
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
err_queue(char *arg1, char *arg2, char *arg3, int len, char *arg4, char *qqx,
	int permanent, unsigned long qp)
{
	char           *ptr;
	int             idx;
	stralloc        tmpLine = { 0 };
	char            size[FMT_ULONG];

	accept_buf[fmt_ulong(accept_buf, qp)] = 0;
	size[fmt_ulong(size, msg_size)] = 0;
	for (ptr = arg3 + 1, idx = 0; idx < len; idx++)
	{
		if (!arg3[idx])
		{
			/*
			 * write data to spamlogger 
			 */
			log_spam(arg2, ptr, msg_size, &tmpLine);
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(arg1);
			logerr(" ");
			logerr(qqx);
			if (permanent)
				logerr(" (permanent): ");
			else
				logerr(" (temporary): ");
			logerr("HELO <");
			logerr(helohost.s);
			logerr("> MAIL from <");
			logerr(arg2);
			logerr("> RCPT <");
			logerr(ptr);
			logerr("> AUTH <");
			if (arg4 && *arg4)
			{
				logerr(arg4);
				switch (authd)
				{
				case 0:
					break;
				case 1:
					logerr(": AUTH LOGIN");
					break;
				case 2:
					logerr(": AUTH PLAIN");
					break;
				case 3:
					logerr(": AUTH CRAM-MD5");
					break;
				case 4:
					logerr(": AUTH CRAM-SHA1");
					break;
				case 5:
					logerr(": AUTH CRAM-RIPEMD");
					break;
				case 6:
					logerr(": AUTH DIGEST-MD5");
					break;
				default:
					logerr(": AUTH unknown");
					break;
				}
			}
			if (addrallowed(ptr))
			{
				if (arg4 && *arg4)
					logerr(": ");
				logerr("local-rcpt");
			} else
			if (!arg4 || !*arg4)
				logerr("pop-bef-smtp");
			logerr("> Size: ");
			logerr(size);
			if (tmpLine.len)
			{
				logerr(" ");
				logerr(tmpLine.s); /*- X-Bogosity line */
			}
			logerr(" qp ");
			logerr(accept_buf);
			logerr("\n");
			ptr = arg3 + idx + 2;
		}
	}
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
msg_notify()
{
	unsigned long   qp;
	char           *qqx;
	char            buf[DATE822FMT];
	struct datetime dt;

	if (qmail_open(&qqt) == -1)
	{
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr(remoteip);
		logerrf(" qqt failure");
		return;
	}
	qp = qmail_qp(&qqt); /*- pid of queue process */
	if (proto.len)
	{
		if (!stralloc_0(&proto))
			die_nomem();
		protocol = proto.s;
	}
	datetime_tai(&dt, now());
	received(&qqt, (char *) protocol, local, remoteip, remotehost, remoteinfo, fakehelo);
	strnum[fmt_ulong(strnum, msg_size)] = 0;
	qmail_puts(&qqt, "X-size-Notification: ");
	qmail_puts(&qqt, "size=");
	qmail_puts(&qqt, strnum);
	qmail_puts(&qqt, ",");
	qmail_put(&qqt, buf, date822fmt(buf, &dt));
	qmail_puts(&qqt, "To: do-not-reply\nFrom: ");
	qmail_put(&qqt, mailfrom.s, mailfrom.len);
	qmail_puts(&qqt, "\nSubject: Notification Message size ");
	qmail_puts(&qqt, strnum);
	qmail_puts(&qqt, " exceeds data limit\n");
	qmail_puts(&qqt, "Date: ");
	qmail_put(&qqt, buf, date822fmt(buf, &dt));
	qmail_from(&qqt, mailfrom.s);
	qmail_put(&qqt, rcptto.s, rcptto.len);
	qqx = qmail_close(&qqt);
	if (!*qqx) /*- mail is now in queue */
	{
		log_trans(remoteip, mailfrom.s, rcptto.s, rcptto.len, 0, 1);
		return;
	}
	err_queue(remoteip, mailfrom.s, rcptto.s, rcptto.len,
		authd ? remoteinfo : 0, qqx + 1, *qqx == 'D', qp);
}

void
err_smf()
{
	out("451 Requested action aborted: DNS temporary failure (#4.4.3)\r\n");
}

void
err_size(char *rip, char *mailfrom, char *rcpt, int len)
{
	int             idx;
	char           *ptr;

	out("552 sorry, that message size exceeds my databytes limit (#5.3.4)\r\n");
	if (env_get("DATABYTES_NOTIFY"))
		msg_notify();
	for (ptr = rcpt + 1, idx = 0; idx < len; idx++)
	{
		if (!rcpt[idx])
		{
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(rip);
			logerr(" data size exceeded: MAIL from <");
			logerr(mailfrom);
			logerr("> RCPT <");
			logerr(ptr);
			logerr("> Size: ");
			strnum[fmt_ulong(strnum, msg_size)] = 0;
			logerr(strnum);
			logerr("\n");
			ptr = rcpt + idx + 2;
		}
	}
}

void
err_hops()
{
	out("554 too many hops, this message is looping (#5.4.6)\r\n");
}

void
err_hmf(char *arg1, char *arg2, int arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	if (arg3)
		logerr(" Non-existing DNS_MX: MAIL ");
	else
		logerr(" Non-existing DNS_MX: HELO ");
	logerr(arg2);
	logerrf("\n");
	if (arg3)
		out("553 Bad sender's system address (#5.1.8)\r\n");
	else
		out("553 sorry, helo domain must exist (#5.1.8)\r\n");
}

void
err_badhelo(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid HELO greeting: HELO <");
	logerr(arg2);
	logerr("> FQDN <");
	logerr(arg3);
	logerrf(">\n");
	out("553 sorry, your HELO/EHLO greeting is in my badhelo list (#5.7.1)\r\n");
#ifdef QUITASAP
	flush();
	_exit(1);
#endif
}

void
err_nogateway(char *arg1, char *arg2, char *arg3, int flag)
{
	char           *x;

	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid RELAY client: MAIL from <");
	logerr(arg2);
	if (arg3 && *arg3) {
		logerr("> RCPT <");
		logerr(arg3);
	}
	logerr(">");
	if (authd) {
		logerr(", Auth <");
		logerr(remoteinfo);
		logerr(">");
		x = env_get("MASQUERADE");
		if (x && *x) {
			logerr(", MASQUERADE <");
			logerr(x);
			logerr(">");
		}
	}
	logerrf("\n");
	if (flag)
		out("553 sorry, this MTA does not accept masquerading/forging ");
	else
		out("553 sorry, that domain isn't allowed to be relayed thru this MTA without authentication ");
	if (authd) {
		out(", auth <");
		out(remoteinfo);
		out("> ");
	}
#ifdef TLS
	tls_nogateway();
#endif
	out("#5.7.1\r\n");
}

void
err_badbounce()
{
	out("553 sorry, bounce messages should have a single envelope recipient (#5.7.1)\r\n");
}

void
err_bmf(char *arg1, char *arg2)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid SENDER address: MAIL from <");
	logerr(arg2);
	logerrf(">\n");
	out("553 sorry, your envelope sender has been denied (#5.7.1)\r\n");
}

#ifdef USE_SPF
void
err_spf()
{
	int             i, j;

	for (i = 0; i < spfbarfmsg.len; i = j + 1)
	{
		j = byte_chr(spfbarfmsg.s + i, spfbarfmsg.len - i, '\n') + i;
		if (j < spfbarfmsg.len)
		{
			out("550-");
			spfbarfmsg.s[j] = 0;
			out(spfbarfmsg.s);
			spfbarfmsg.s[j] = '\n';
			out("\r\n");
		} else
		{
			out("550 ");
			out(spfbarfmsg.s);
			out(" (#5.7.1)\r\n");
		}
	}
}
#endif

void
err_hostaccess(char *arg1, char *arg2)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid SENDER host IP address: MAIL from <");
	logerr(arg2);
	logerrf(">\n");
	out("553 sorry, your host has been denied (#5.7.1)\r\n");
}

void
log_virus(char *arg1, char *arg2, char *arg3, char *arg4, int len, int blackhole)
{
	int             idx;
	char           *ptr;

	for (ptr = arg4 + 1, idx = 0; idx < len; idx++)
	{
		if (!arg4[idx])
		{
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(arg1);
			logerr(" virus/banned content: ");
			logerr(arg2);
			logerr(": MAIL from <");
			logerr(arg3);
			logerr("> RCPT <");
			logerr(ptr);
			logerr("> Size: ");
			strnum[fmt_ulong(strnum, msg_size)] = 0;
			logerr(strnum);
			logerr("\n");
			ptr = arg4 + idx + 2;
		}
	}
	if (substdio_flush(&sserr) == -1)
		_exit(1);
	if (!blackhole)
	{
		out("552-we don't accept email with the below content (#5.3.4)\r\n");
		out("552 Further Information: ");
		out(arg2);
		out("\r\n");
		flush();
	}
}

void
err_acl(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid RECIPIENT address: MAIL from <");
	logerr(arg2);
	logerr("> RCPT ");
	logerr(arg3);
	logerrf("\n");
	out("553 sorry, sites access list denies transaction (#5.7.1)\r\n");
	return;
}

void
err_rcp(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Invalid RECIPIENT address: MAIL from <");
	logerr(arg2);
	logerr("> RCPT ");
	logerr(arg3);
	logerrf("\n");
	out("553 sorry, your envelope recipient has been denied (#5.7.1)\r\n");
	return;
}

void
smtp_badip(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" BAD IP client\n");
	sleep(5);
	out("421 sorry, your IP (");
	out(arg);
	out(") is temporarily denied (#4.7.1)\r\n");
	return;
}

void
smtp_badhost(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" BAD HOST ");
	logerr(remotehost);
	logerrf("\n");
	sleep(5);
	out("553 sorry, your host (");
	out(remotehost);
	out(") has been denied (#5.7.1)\r\n");
}

void
smtp_relayreject(char *arg)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" OPEN RELAY client\n");
	sleep(5);
	out("553 No mail accepted from an open relay (");
	out(remoteip);
	out("); check your server configs (#5.7.1)\r\n");
	return;
}

void
smtp_paranoid(char *arg)
{
	char           *ptr;

	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" PTR (reverse DNS) record points to wrong hostname\n");
	sleep(5);
	ptr = env_get("TCPPARANOID");
	out("553 sorry, your IP address (");
	out(arg);
	if (ptr && *ptr)
	{
		out(") PTR (reverse DNS) record points to wrong hostname ");
		out(ptr);
		out(" (#5.7.1)\r\n");
	} else
		out(") PTR (reverse DNS) record points to wrong hostname (#5.7.1)\r\n");
	return;
}

void
smtp_ptr(char *arg)
{
	char           *ptr;

	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" unable to obain PTR (reverse DNS) record\n");
	sleep(5);
	ptr = env_get("REQPTR");
	out("553 ");
	if (*ptr)
	{
		out(ptr);
		out(": from ");
		out(arg);
		out(": (#5.7.1)\r\n");
	} else
	{
		out(" Sorry, no PTR (reverse DNS) record for (");
		out(arg);
		out(") (#5.7.1)\r\n");
	}
	return;
}

void
log_rules(char *arg1, char *arg2, char *arg3, int arg4)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Setting Rule No ");
	strnum[fmt_ulong(strnum, arg4)] = 0;
	logerr(strnum);
	logerr(": MAIL from <");
	logerr(arg2);
	if (authd)
	{
		switch (authd)
		{
		case 1:
			logerr("> AUTH LOGIN <");
			break;
		case 2:
			logerr("> AUTH PLAIN <");
			break;
		case 3:
			logerr("> AUTH CRAM-MD5 <");
			break;
		case 4:
			logerr("> AUTH CRAM-SHA1 <");
			break;
		case 5:
			logerr("> AUTH CRAM-SHA256 <");
			break;
		case 6:
			logerr("> AUTH CRAM-RIPEMD <");
			break;
		case 7:
			logerr("> AUTH DIGEST-MD5 <");
			break;
		default:
			logerr("> AUTH unknown <");
			break;
		}
		logerr(arg3);
	}
	logerrf(">\n");
}

void
err_relay()
{
	out("550 we don't relay (#5.7.1)\r\n");
}

void
err_unimpl(char *arg)
{
	if (!case_diffs(arg, "unimplemented"))
		out("502 unimplemented (#5.5.1)\r\n");
	else
	{
		out("500 command ");
		out(arg);
		out(" not recognized (#5.5.2)\r\n");
	}
}

void
err_syntax()
{
	out("555 syntax error in address (#5.1.3)\r\n");
}

void
err_wantmail()
{
	out("503 MAIL first (#5.5.1)\r\n");
}

void
err_wantrcpt()
{
	out("503 RCPT first (#5.5.1)\r\n");
}

void
err_bhf(char *arg1, char *arg2)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Blackholed SENDER address: MAIL ");
	logerr(arg2);
	logerrf("\n");
	if (!env_put("NULLQUEUE=1"))
		die_nomem();
}

void
err_bhrcp(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Blackholed RECIPIENT address: MAIL from <");
	logerr(arg2);
	logerr("> RCPT ");
	logerr(arg3);
	logerrf("\n");
	if (!env_put("NULLQUEUE=1"))
		die_nomem();
}

void
err_maps(char *m, char *ip, char *from)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(ip);
	logerr(" Blackholed SENDER address: MAIL ");
	logerr(from);
	logerr(" Reason ");
	logerr(m);
	logerrf("\n");
	out("553 ");
	out(m);
	out(" (#5.7.1)\r\n");
	flush();
	_exit(1);
}

void
err_mrc(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	logerr(" Too many RECIPIENTS: MAIL <");
	logerr(arg2);
	logerr("> Last RCPT <");
	logerr(arg3);
	logerrf(">\n");
	out("557 sorry, too many recipients (#5.7.1)\r\n");
}

void
smtp_noop(char *arg)
{
	if (arg && *arg)
	{
		out("501 invalid parameter syntax (#5.3.2)\r\n");
		return;
	}
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	out("250 ok\r\n");
	return;
}

void
smtp_vrfy(char *arg)
{
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	out("252 Cannot VRFY user, but will accept message and attempt delivery (#2.7.0)\r\n");
}

void
err_qqt(char *arg)
{
	out("451 Requested action aborted: qqt failure (#4.3.0)\r\n");
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg);
	logerrf(" qqt failure");
}

int
err_child()
{
	out("451 Requested action aborted: problem with child and I can't auth (#4.3.0)\r\n");
	return -1;
}

int
err_fork()
{
	out("451 Requested action aborted: child won't start and I can't auth (#4.3.0)\r\n");
	return -1;
}

int
err_pipe()
{
	out("451 Requested action aborted: unable to open pipe and I can't auth (#4.3.0)\r\n");
	return -1;
}

int
err_write()
{
	out("451 Requested action aborted: unable to write pipe and I can't auth (#4.3.0)\r\n");
	return -1;
}

void
err_authd()
{
	out("503 you're already authenticated (#5.5.0)\r\n");
}

void
err_authrequired()
{
	out("530 authentication required (#5.7.1)\r\n");
}

void
err_transaction(char *arg)
{
	out("503 no ");
	out(arg);
	out(" during mail transaction (#5.5.0)\r\n");
}

int
err_noauth()
{
	out("504 auth type unimplemented (#5.5.1)\r\n");
	return -1;
}

int
err_authabrt()
{
	out("501 auth exchange cancelled (#5.0.0)\r\n");
	return -1;
}

int
err_input()
{
	out("501 malformed auth input (#5.5.4)\r\n");
	return -1;
}

void
err_mailbox(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" Invalid RECIPIENT address: MAIL from <");
	logerr(arg1);
	logerr("> RCPT <");
	logerr(arg2);
	logerr("> state <");
	logerr(arg3);
	logerrf(">\n");
	out("550 sorry, ");
	out(arg1);
	out(" mailbox <");
	out(arg2);
	out("> ");
	out(arg3);
	out("\r\n");
	return;
}

void
err_rcpt_errcount(char *arg1, int count)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" Too many Invalid RECIPIENTS (");
	strnum[fmt_ulong(strnum, count)] = 0;
	logerr(strnum);
	logerr("): MAIL from <");
	logerr(arg1);
	logerrf(">\n");
	out("421 too many invalid addresses, goodbye (#4.7.1)\r\n");
}

void
err_greytimeout()
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerrf(" Timeout (no response from greylisting server)\n");
	out("451 greylist temporary failure - Timeout (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
err_grey_tmpfail()
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" greylisting temporary failure: ");
	logerr(error_str(errno));
	logerrf("\n");
	out("451 greylist temporary failure (#4.3.0)\r\n");
	flush();
	_exit(1);
}

void
err_grey()
{
	char           *arg, *ptr;
	int             idx;

	arg = rcptto.s;
	for (ptr = arg + 1, idx = 0; idx < rcptto.len; idx++)
	{
		if (!arg[idx])
		{
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" HELO <");
			logerr(helohost.s);
			logerr("> MAIL from <");
			logerr(mailfrom.s);
			logerr("> RCPT <");
			logerr(ptr);
			logerrf(">\n");
			ptr = arg + idx + 2;
		}
	}
	logerr("greylist ");
	logerr(remoteip);
	logerr(" <");
	logerr(mailfrom.s);
	logerr("> to <");
	logerr(arg + 1);
	logerr(">");
	if (rcptcount > 1)
		logerr("..."); /* > 1 address sent for greylist check */
	logerr("\n");
	out("450 try again later (#4.3.0)\r\n");
	return;
}

int             flagblackhole;
stralloc        Desc = { 0 };

int
sigscheck(stralloc *line, char **desc, int in_header)
{
	int             i, j, k, len, pos1, pos2, header_check, body_check;
	char           *ptr;

	*desc = "unknown";
	for (i = j = 0; i < sigs.len; i++)
	{
		header_check = body_check = flagblackhole = 0;
		if (!sigs.s[i])
		{
			for (k = i, len = 0; sigs.s[k] != ':' && k > j; k--)
				len++;
			pos1 = pos2 = 0;
			if (sigs.s[k] == ':')
			{
				sigs.s[k] = 0;
				pos2 = k;
				for (ptr = sigs.s + k + 1;*ptr && isspace(*ptr);ptr++);
				if (!str_diffn(ptr, "-headerblackhole", 16))
					header_check = flagblackhole = 1;
				else
				if (!str_diffn(ptr, "-header", 7))
					header_check = 1;
				else
				if (!str_diffn(ptr, "-bodyblackhole", 14))
					body_check = flagblackhole = 1;
				else
				if (!str_diffn(ptr, "-body", 5))
					body_check = 1;
				if (header_check || flagblackhole || body_check)
				{
					for (;sigs.s[k] != ':' && k > j; k--)
						len++;
					if (sigs.s[k] == ':')
					{
						pos1 = k;
						sigs.s[k] = 0;
						if (!stralloc_copys(&Desc, sigs.s + pos1 + 1))
							die_nomem();
					} else
					{
						if (!stralloc_copys(&Desc, sigs.s + j))
							die_nomem();
						len -= (pos2 - k);
					}
				} else
				{
					if (!stralloc_copys(&Desc, sigs.s + pos2 + 1))
						die_nomem();
				}
			} else
			{
				if (!stralloc_copys(&Desc, sigs.s + j))
					die_nomem();
				len = 0; /*- handle signatures without comments */
			}
			if (pos1)
				sigs.s[pos1] = ':';
			if (pos2)
				sigs.s[pos2] = ':';
			if ((body_check && in_header) || (header_check && !in_header))
			{
				j = i + 1;
				continue;
			}
			if ((i - j - len) < line->len && !str_diffn(line->s, sigs.s + j, i - j - len))
			{
				if (!stralloc_0(&Desc))
					die_nomem();
				*desc = Desc.s;
				return 1;
			}
			j = i + 1;
		}	  /*- if (!sigs.s[i]) */
	} /*- for (;;) */
	return 0;
}

/*
 * This function returns
 *  0: If user not found
 * >0: if user is found
 */
int
check_recipient_cdb(char *rcpt)
{
	int             r;

	r = recipients(rcpt, str_len(rcpt));
	switch (r)
	{
	case -1:
		die_control();
		break;
	case -2:
		die_nomem();
		break;
	case 10: /*- recipient cdb does not exist */
		return 0;
	case -3:
	case 111:
		out("451 unable to check recipients (#4.3.2)\r\n");
		logerr("qmail-smtpd: ");
		logerrpid();
		logerrf("recipients database error\n");
		flush();
		_exit(1);
		/*- Not Reached */
	}
	return r;
}

#ifdef INDIMAIL
/*
 * This function returns
 *  0: User is fine
 *  1: User is not present
 *  2: User is Inactive
 *  3: User is overquota
 * -1: System Error
 */
int
check_recipient_sql(char *rcpt)
{
	char           *ptr;

	if ((ptr = inquery(USER_QUERY, rcpt, 0)))
	{
		if (*ptr == 4) /*- allow aliases */
			return (0);
		return (*ptr);
	} 
	if (userNotFound)
		return (1);
	out("451 Requested action aborted: database error (#4.3.2)\r\n");
	logerr("qmail-smtpd: ");
	logerrpid();
	logerrf("sql database error\n");
	flush();
	_exit(1);
	/*- Not Reached */
	return (0);
}
#endif

int
dnscheck(char *addr, int len, int paranoid)
{
	stralloc        sa = { 0 };
	ipalloc         ia = { 0 };
	unsigned int    random;
	int             j;

	if (str_equal(addr, "#@[]") || !len)
		return (0);
	if (nodnschecksok)
	{
		if (constmap(&mapnodnschecks, addr, len))
			return 0;
		if ((j = byte_rchr(addr, len, '@')) < (len - 1))
		{
			if (constmap(&mapnodnschecks, addr + j, len - j))
				return 0;
		}
	}
	random = now() + (getpid() << 16);
	if ((j = byte_rchr(addr, len, '@')) < (len - 1))
	{
		if (!stralloc_copys(&sa, addr + j + 1))
			die_nomem();
		dns_init(0);
		if ((j = dns_mxip(&ia, &sa, random)) < 0)
			return j;
	} else
	if (paranoid)
	{
		if (!stralloc_copys(&sa, addr))
			die_nomem();
		dns_init(0);
		if ((j = dns_mxip(&ia, &sa, random)) < 0)
			return j;
	} else
		return (DNS_HARD);
	return (0);
}

void
log_etrn(char *arg1, char *arg2, char *arg3)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	if (arg3)
	{
		logerr(" ");
		logerr(arg3);
	}
	if (arg2)
	{
		logerr(" ETRN ");
		logerr(arg2);
	}
	logerrf("\n");
}

#ifdef INDIMAIL
void
log_atrn(char *arg1, char *arg2, char *arg3, char *arg4)
{
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(arg1);
	if (arg2)
	{
		logerr(" ");
		logerr(arg2);
	}
	if (arg3)
	{
		logerr(" ATRN ");
		logerr(arg3);
	}
	if (arg4)
	{
		logerr(": ");
		logerr(arg4);
	}
	logerrf("\n");
}
#endif

void
sigterm()
{
	smtp_greet("421 ");
	out(" Service not available, closing tranmission channel (#4.3.2)\r\n");
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerr(" going down on SIGTERM");
	logerrf("\n");
	flush();
	_exit(1);
}

void
esmtp_print()
{
	char           *ptr;
	char            buf[DATE822FMT];
	int             i;
	struct datetime dt;

	substdio_puts(&ssout, " (NO UCE) ESMTP IndiMail ");
	for (ptr = (revision + 11); *ptr; ptr++)
	{
		if (*ptr == ' ')
		{
			out(" ");
			break;
		}
		substdio_put(&ssout, ptr, 1);
	}
	datetime_tai(&dt, now());
	i = date822fmt(buf, &dt);
	buf[i - 1] = 0;
	out(buf);
}

void
smtp_greet(char *code)
{
	int             i, j, esmtp;

	if (code[3] != ' ')
	{
		substdio_puts(&ssout, code);
		substdio_puts(&ssout, greeting.s);
		return;
	}
	if (code[0] == '2' && code[1] == '2' && code[2] == '0')
		esmtp = 1;
	else
		esmtp = 0;
	for (i = 0, j = 0; i < greeting.len - 1; i++)
	{
		if (greeting.s[i] == '\0')
		{
			substdio_put(&ssout, code, 3);
			substdio_puts(&ssout, "-");
			substdio_put(&ssout, greeting.s + j, i - j);
			if (esmtp)
			{
				esmtp_print();
				esmtp = 0;
			}
			substdio_puts(&ssout, "\r\n");
			j = i + 1;
		}
	}
	substdio_puts(&ssout, code);
	substdio_put(&ssout, greeting.s + j, greeting.len - 1 - j);
	if (esmtp)
	{
		esmtp_print();
		esmtp = 0;
	}
}

void
smtp_help(char *arg)
{
	char           *ptr;

	ptr = revision + 11;
	if (*ptr)
	{
		out("214-This is IndiMail SMTP Version ");
		for (; *ptr; ptr++)
		{
			if (*ptr == ' ')
				break;
			substdio_put(&ssout, ptr, 1);
		}
	}
	out("\r\n");
	out("214-This server supports the following commands:\r\n");
	switch (smtp_port)
	{
#ifdef INDIMAIL
	case ODMR_PORT: /*- RFC 2645 */
		out("214 HELO EHLO AUTH ATRN HELP QUIT\r\n");
		break;
#endif
	case SUBM_PORT: /*- RFC 2476 */
		if (hostname && *hostname && childargs && *childargs)
			out("214 HELO EHLO RSET NOOP MAIL RCPT DATA AUTH VRFY HELP QUIT\r\n");
		else
			out("214 HELO EHLO RSET NOOP MAIL RCPT DATA VRFY HELP QUIT\r\n");
		break;
	default:
		if (hostname && *hostname && childargs && *childargs)
		{
			out("214 HELO EHLO RSET NOOP MAIL RCPT DATA AUTH VRFY ETRN ");
#ifdef INDIMAIL
			out("ATRN ");
#endif
			out("HELP QUIT\r\n");
		}
		else
		{
			out("214 HELO EHLO RSET NOOP MAIL RCPT DATA VRFY ETRN ");
#ifdef INDIMAIL
			out("ATRN ");
#endif
			out("HELP QUIT\r\n");
		}
		break;
	}
}

void
smtp_quit(char *arg)
{
#ifdef SMTP_PLUGIN
	int             i;
#endif

	smtp_greet("221 ");
	out(" closing connection\r\n");
	flush();
#ifdef SMTP_PLUGIN
	for (i = 0;i < plugin_count;i++)
		dlclose(handle[i]);
#endif
	_exit(0);
}

int
badipcheck(char *arg)
{
	/*- badip */
	if (!stralloc_copys(&ipaddr, arg))
		die_nomem();
	if (!stralloc_0(&ipaddr))
		die_nomem();
	switch (address_match((badipfn && *badipfn) ? badipfn : "badip", 
		&ipaddr, briok ? &bri : 0, briok ? &mapbri : 0, 0, &errStr))
	{
	case 1: 
		return (1);
	case 0:
		return (0);
	case -1:
		die_nomem();
	default:
		die_addressmatch(errStr);
		return (-1);
	}
	return (0);
}

/*
 * $TCPREMOTEHOST check against regular expressions depending on $RELAYCLIENT:
 * domain-based blacklist/right-hand-side blackhole list (RHSBL)
 *
 * Author: Jörg Backschues
 * switch (address_match(0, &addr, brhok ? &brh : 0, brhok ? &mapbrh : 0, 0, &errStr))
 */
int
badhostcheck()
{
	int             i = 0, j = 0, x = 0, negate = 0;
	stralloc        curregex = { 0 };

	while (j < brh.len) {
		i = j;
		while ((brh.s[i] != '\0') && (i < brh.len))
			i++;
		if (brh.s[j] == '!') {
			negate = 1;
			j++;
		}
		stralloc_copyb(&curregex, brh.s + j, (i - j));
		stralloc_0(&curregex);
		x = matchregex(remotehost, curregex.s, 0);
		if ((negate) && (x == 0))
			return 1;
		if (!(negate) && (x > 0))
			return 1;
		j = i + 1;
		negate = 0;
	}
	return 0;
}

void
dohelo(char *arg)
{
	int             i;

	seenhelo = 0;
	if (!stralloc_copys(&helohost, arg))
		die_nomem();
	if (!stralloc_0(&helohost))
		die_nomem();
	if (!relayclient) /*- turn on if user not authenticated */
	{
		if (env_get("ENFORCE_FQDN_HELO"))
		{
			i = str_chr(arg, '.');
			if (!arg[i])
				die_nohelofqdn(arg);
		}
		if (local && case_equals(local, helohost.s))
			return;
		if (localip && case_equals(localip, helohost.s))
			return;
	}
	/*- badhelo */
	if (dohelocheck)
	{
		switch (address_match((badhelofn && *badhelofn) ? badhelofn : "badhelo", 
			&helohost, badhelook ? &badhelo : 0, badhelook ? &maphelo : 0, 0, &errStr))
		{
		case 1:
			err_badhelo(remoteip, helohost.s, remotehost);
			return;
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr);
			return;
		}
	}
	if ((fakehelo = case_diffs(remotehost, helohost.s) ? helohost.s : 0))
	{
		if (dohelocheck && !nodnscheck)
		{
			switch (dnscheck(helohost.s, helohost.len - 1, 1))
			{
			case DNS_HARD:
				err_hmf(remoteip, arg, 0);
				return;
			case DNS_SOFT:
				err_smf();
				return;
			case DNS_MEM:
				die_nomem();
			}
		}
		seenhelo = 1;
		return;
	}
	seenhelo = 1;
}

void
greetdelay_check(int delay)
{
	int             r;

	if (delay > 0)
	{
		sleep (delay);
		return;
	} else
		delay = -delay;
	if ((r = timeoutread(delay, 0, ssinbuf, sizeof ssinbuf)) == -1)
	{
  		if (errno == error_timeout)
			return; /* Timeout ==> No early talking */
	}
	if (r <= 0)
	{
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr(remoteip);
		logerr(" read error: ");
		logerr(!r ? "EOF" : error_str(errno));
		logerrf("\n");
		die_read();
	}
	logerr("qmail-smtpd: ");
	logerrpid();
	logerr(remoteip);
	logerrf(" SMTP Protocol violation - Early Talking\n");
   	out("554 SMTP protocol violation. Polite people say hello after the server greets them (#5.7.1)\r\n");
	flush();
	_exit(1);
}


/*
 * This function gets called after envrules
 * Set any environment variables again using envrules
 */
void
databytes_setup()
{
	char           *x;

	if (!(x = env_get("DATABYTES")))
	{
		if (control_readulong(&databytes, "databytes") == -1)
			die_control();
	} else
		scan_ulong(x, &databytes);
	if (!(databytes + 1))
		--databytes;
}

void
post_setup()
{
	char           *x;

	/*- BADMAILFROM */
	if ((bmfok = control_readfile(&bmf, bmfFn = ((x = env_get("BADMAILFROM")) && *x ? x : "badmailfrom"), 0)) == -1)
		die_control();
	if (bmfok && !constmap_init(&mapbmf, bmf.s, bmf.len, 0))
		die_nomem();
	if ((bmpok = control_readfile(&bmp, (x = env_get("BADMAILPATTERNS")) && *x ? x : "badmailpatterns", 0)) == -1)
		die_control();
	/*
	 * BLACKHOLE Sender Patch - include Control file 
	 */
	if ((bhfok = control_readfile(&bhf, bhsndFn = ((x = env_get("BLACKHOLEDSENDER")) && *x ? x : "blackholedsender"), 0)) == -1)
		die_control();
	if (bhfok && !constmap_init(&mapbhf, bhf.s, bhf.len, 0))
		die_nomem();
	if ((bhpok = control_readfile(&bhp, (x = env_get("BLACKHOLEDPATTERNS")) && *x ? x : "blackholedpatterns", 0)) == -1)
		die_control();
	/*
	 * BLACKHOLE RECIPIENT Patch - include Control file 
	 */
	if ((bhrcpok = control_readfile(&bhrcp, bhrcpFn = ((x = env_get("BLACKHOLERCPT")) && *x ? x : "blackholedrcpt"), 0)) == -1)
		die_control();
	if (bhrcpok && !constmap_init(&mapbhrcp, bhrcp.s, bhrcp.len, 0))
		die_nomem();
	if ((bhbrpok = control_readfile(&bhbrp, (x = env_get("BLACKHOLERCPTPATTERNS")) && *x ? x : "blackholedrcptpatterns", 0)) == -1)
		die_control();
	/*
	 * RECIPIENT Patch - include Control file 
	 */
	if ((rcpok = control_readfile(&rcp, rcpFn = ((x = env_get("BADRCPTTO")) && *x ? x : "badrcptto"), 0)) == -1)
		die_control();
	if (rcpok && !constmap_init(&maprcp, rcp.s, rcp.len, 0))
		die_nomem();
	if ((brpok = control_readfile(&brp, (x = env_get("BADRCPTPATTERNS")) && *x ? x : "badrcptpatterns", 0)) == -1)
		die_control();
	/*
	 * goodrcpt
	 */
	if ((chkgrcptok = control_readfile(&grcpt, grcptFn = ((x = env_get("GOODRCPTTO")) && *x ? x : "goodrcptto"), 0)) == -1)
		die_control();
	if (chkgrcptok && !constmap_init(&mapgrcpt, grcpt.s, grcpt.len, 0))
		die_nomem();
	if ((chkgrcptokp = control_readfile(&grcptp, (x = env_get("GOODRCPTPATTERNS")) && *x ? x : "goodrcptpatterns", 0)) == -1)
		die_control();
	/*
	 * Spam Ignore Patch - include Control file 
	 */
	if (env_get("SPAMFILTER"))
	{
		if ((spfok = control_readfile(&spf, spfFn = ((x = env_get("SPAMIGNORE")) && *x ? x : "spamignore"), 0)) == -1)
			die_control();
		if (spfok && !constmap_init(&mapspf, spf.s, spf.len, 0))
			die_nomem();
		if ((sppok = control_readfile(&spp, (x = env_get("SPAMIGNOREPATTERNS")) && *x ? x : "spamignorepatterns", 0)) == -1)
			die_control();
	}
	/*
	 * DNSCHECK Patch - include Control file 
	 */
	if (!(nodnscheck = env_get("NODNSCHECK")))
	{
		/*- Look up "MAIL from:" addresses to skip for DNS check in control/nodnscheck. */
		if ((nodnschecksok = control_readfile(&nodnschecks, "nodnscheck", 0)) == -1)
			die_control();
		if (nodnschecksok && !constmap_init(&mapnodnschecks, nodnschecks.s, nodnschecks.len, 0))
			die_nomem();
	}
#ifdef BATV
	if ((batvok = control_readline(&signkey, (x = env_get("SIGNKEY")) ? x : "signkey")) == -1)
		die_control();
	if (batvok)
	{
		switch (control_readfile(&nosign, "nosignhosts",0))
		{
		case -1:
			die_control();
		case 0:
			if (!constmap_init(&mapnosign, "", 0, 1))
				die_nomem();
			break;
		case 1:
			if (!constmap_init(&mapnosign, nosign.s, nosign.len, 0))
				die_nomem();
			break;
		}
		if (control_readint(&signkeystale, "signkeystale") == -1) /*- Joerg Backschues */
			die_control();
		if ((x = env_get("SIGNKEYSTALE")))
			scan_int(x, &signkeystale);
	}
#endif
	if ((acclistok = control_readfile(&acclist, (x = env_get("ACCESSLIST")) && *x ? x : "accesslist", 0)) == -1)
		die_control();
	if ((x = env_get("BODYCHECK")))
	{
		if ((bodyok = control_readfile(&body, (x = env_get("BODYCHECK")) && *x ? x : "bodycheck", 0)) == -1)
			die_control();
		bodyok_orig = bodyok;
	}
#ifdef USE_SPF
	if ((x = env_get("SPFBEHAVIOR")))
		scan_int(x, (int *) &spfbehavior);
	else
	if (control_readint((int *) &spfbehavior, "spfbehavior") == -1)
		die_control();
	if ((x = env_get("SPFIPV6")))
		scan_int(x, (int *) &spfipv6);
	else
	if (control_readint((int *) &spfipv6, "spfipv6") == -1)
		die_control();
#endif
	/*
	 * TARPIT Patch - include Control Files 
	 */
	if (!(x = env_get("TARPITCOUNT")))
	{
		if (control_readint(&tarpitcount, "tarpitcount") == -1)
			die_control();
		if (tarpitcount < 0)
			tarpitcount = 0;
	} else
		scan_int(x, &tarpitcount);
	if (!(x = env_get("TARPITDELAY")))
	{
		if (control_readint(&tarpitdelay, "tarpitdelay") == -1)
			die_control();
		if (tarpitdelay < 0)
			tarpitdelay = 0;
	} else
		scan_int(x, &tarpitdelay);
	/*- MAXRECPIENTS - include Control Files */
	if (!(x = env_get("MAXRECIPIENTS")))
	{
		if (control_readint(&maxrcptcount, "maxrecipients") == -1)
			die_control();
		if (maxrcptcount < 0)
			maxrcptcount = 0;
	} else
		scan_int(x, &maxrcptcount);
	if ((x = env_get("VIRUSCHECK")))
	{
		unsigned long   u;
		if (!*x)
			x = "1";
		scan_ulong(x, &u);
		switch (u)
		{
			case 1: /*- Virus Scanner (Internal) */
			case 2: /*- Virus Scanner (Internal + External) */
			case 3: /*- Virus Scanner (Internal) + Bad Attachment Scan */
			case 4: /*- Virus Scanner (Internal + External) + Bad Attachment Scan */
				if ((sigsok = control_readfile(&sigs, (x = env_get("SIGNATURES")) && *x ? x : "signatures", 0)) == -1)
					die_control();
				sigsok_orig = sigsok;
				break;
			case 5: /*- Virus Scanner (External) + Bad Attachment Scan*/
			case 6: /*- Virus Scanner (External) */
			case 7: /*- Bad Attachment scan */
				break;
		}
	}
	if ((x = env_get("QREGEX")))
		scan_int(x, &qregex);
	else
	{
		if (control_readint(&qregex, "qregex") == -1)
			die_control();
		if (qregex && !env_put("QREGEX=1"))
			die_nomem();
	}
	return;
}

void
setup()
{
	unsigned int    i;
	unsigned        len;
	char           *x;

	if (!stralloc_copys(&Revision, revision + 11))
		die_nomem();
	if (!stralloc_0(&Revision))
		die_nomem();
	for (x = Revision.s; *x && *x != ' '; x++);
	if (*x == ' ')
		*x = 0;
	if (control_init() == -1)
		die_control();
	if (control_readfile(&greeting, "smtpgreeting", 1) != 1)
		die_control();
	if ((liphostok = control_rldef(&liphost, "localiphost", 1, (char *) 0)) == -1)
		die_control();
	if (control_readint(&timeout, "timeoutsmtpd") == -1)
		die_control();
	if (timeout <= 0)
		timeout = 1;
	if (control_readint(&maxhops, "maxhops") == -1)
		die_control();
	if (maxhops <= 0)
		maxhops = MAXHOPS;
	/*
	 * buffer limit for commands 
	 */
	if (control_readint(&ctl_maxcmdlen, "maxcmdlen") == -1)
		die_control();
	if (ctl_maxcmdlen < 0)
		ctl_maxcmdlen = 0;
	databytes_setup();
	if (rcpthosts_init() == -1)
		die_control();
	if (recipients_init() == -1)
		die_control();
	if ((x = env_get("GREETDELAY")))
		scan_int(x, &greetdelay);
	else
	{
		if (control_readint(&greetdelay, "greetdelay") == -1)
			die_control();
	}
	/*
	 * Enable badip if
	 * BADIPCHECK is defined (default control file badip)
	 * or
	 * BADIP (control file defined by BADIP env variable)
	 * is defined
	 */
	if ((dobadipcheck = (env_get("BADIPCHECK") ? "" : env_get("BADIP"))))
	{
		if ((briok = control_readfile(&bri, (badipfn = env_get("BADIP")) && *badipfn ? badipfn : "badip", 0)) == -1)
			die_control();
		if (briok && !constmap_init(&mapbri, bri.s, bri.len, 0))
			die_nomem();
	}
	/*
	 * Enable badhost if
	 * BADHOSTCHECK is defined (default control file badhost)
	 * or
	 * BADHOST (control file defined by BADHOST env variable)
	 * is defined
	 */
	if ((dobadhostcheck = (env_get("BADHOSTCHECK") ? "" : env_get("BADHOST"))))
	{
		if ((brhok = control_readfile(&brh, (x = env_get("BADHOST")) && *x ? x : "badhost", 0)) == -1)
			die_control();
		if (brhok && !constmap_init(&mapbrh, brh.s, brh.len, 0))
			die_nomem();
	}
	/*
	 * Enable badhelo if
	 * BADHELOCHECK is defined (default control file badhelo)
	 * or
	 * BADHELO (control file defined by BADHELO env variable)
	 * is defined
	 */
	if ((dohelocheck = (env_get("BADHELOCHECK") ? "" : env_get("BADHELO"))))
	{
		if ((badhelook = control_readfile(&badhelo, 
				(badhelofn = env_get("BADHELO")) && *badhelofn ? badhelofn : "badhelo", 0)) == -1)
			die_control();
		if (badhelook && !constmap_init(&maphelo, badhelo.s, badhelo.len, 0))
			die_nomem();
	}
	/*
	 * RELAYMAILFROM Patch - include Control File 
	 */
	if ((rmfok = control_readfile(&rmf, "relaymailfrom", 0)) == -1)
		die_control();
	if (rmfok && !constmap_init(&maprmf, rmf.s, rmf.len, 0))
		die_nomem();
#ifdef INDIMAIL
	if ((chkrcptok = control_readfile(&chkrcpt, "chkrcptdomains", 0)) == -1)
		die_control();
	if (chkrcptok && !constmap_init(&mapchkrcpt, chkrcpt.s, chkrcpt.len, 0))
		die_nomem();
#endif
	if ((chkdomok = control_readfile(&chkdom, "authdomains", 0)) == -1)
		die_control();
	if (chkdomok && !constmap_init(&mapchkdom, chkdom.s, chkdom.len, 0))
		die_nomem();
#ifdef IPV6
	if (!(remoteip4 = env_get("TCPREMOTEIP")))
		remoteip4 = "unknown";
	if (!(remoteip = env_get("TCP6REMOTEIP")) && !(remoteip = remoteip4))
		remoteip = "unknown";
	if (!(localip = env_get("TCP6LOCALIP")))
		localip = env_get("TCPLOCALIP");
	if (!(local = env_get("TCPLOCALHOST")))
		local = localip;
#else
	if (!(remoteip = env_get("TCPREMOTEIP")))
		remoteip = "unknown";
	localip = env_get("TCPLOCALIP");
	if (!(local = env_get("TCPLOCALHOST")))
		local = localip;
#endif
	if (!local && hostname && *hostname)
		local = hostname;
	if (!local)
		local = "unknown";
	if (!(remotehost = env_get("TCPREMOTEHOST")))
		remotehost = "unknown";
	remoteinfo = env_get("TCPREMOTEINFO");
	relayclient = env_get("RELAYCLIENT");
	greyip = env_get("GREYIP");
	if (!greyip || !*greyip)
		greyip = (char *) 0; /*- Disable greylisting if GREYIP="" */
	if (!relayclient)
	{
		/*- Attempt to look up the IP number in control/relayclients. */
		if ((relayclientsok = control_readfile(&relayclients, "relayclients", 0)) == -1)
			die_control();
		if (relayclientsok)
		{
			if (!constmap_init(&maprelayclients, relayclients.s, relayclients.len, 0))
				die_nomem();
			for (i = len = str_len(remoteip); i > 0; i--)
			{
				if ((i == len) || (remoteip[i - 1] == '.'))
				{
					if ((relayclient = constmap(&maprelayclients, remoteip, i)))
						break;
				}
			}
		}
	}
	if (!relayclient)
	{
		/*- Attempt to look up the host name in control/relaydomains. */
		if ((relaydomainsok = control_readfile(&relaydomains, "relaydomains", 0)) == -1)
			die_control();
		if (relaydomainsok)
		{
			if (!constmap_init(&maprelaydomains, relaydomains.s, relaydomains.len, 0))
				die_nomem();
			for (i = 0, len = str_len(remotehost); i <= len; i++)
			{
				if ((i == 0) || (i == len) || (remotehost[i] == '.'))
				{
					if ((relayclient = constmap(&maprelaydomains, remotehost + i, len - i)))
						break;
				}
			}
		}
	}
	post_setup();
#ifdef USE_SPF
	if (control_readline(&spflocal, "spfrules") == -1)
		die_control();
	if (spflocal.len && !stralloc_0(&spflocal))
		die_nomem();
	if (control_readline(&spfguess, "spfguess") == -1)
		die_control();
	if (spfguess.len && !stralloc_0(&spfguess))
		die_nomem();
	if (control_rldef(&spfexp, "spfexp", 0, SPF_DEFEXP) == -1)
		die_control();
	if (!stralloc_0(&spfexp))
		die_nomem();
#endif
#ifdef TLS
	if (env_get("SMTPS"))
	{
		smtps = 1;
		tls_init();
	} else
		dohelo(remotehost);
#else
	dohelo(remotehost);
#endif
}

int
addrparse(char *arg)
{
	int             i, flagesc, flagquoted;
	char            ch, terminator;
	ip_addr         ip;

	terminator = '>';
	i = str_chr(arg, '<');
	if (arg[i])
		arg += i + 1;
	else {	/*- partner should go read rfc 821 */
		terminator = ' ';
		arg += str_chr(arg, ':');
		if (*arg == ':')
			++arg;
		if (!*arg)
			return (0);
		while (*arg == ' ')
			++arg;
	}
	/*- strip source route */
	if (*arg == '@') {
		while (*arg) {
			if (*arg++ == ':')
				break;
		}
	}
	if (!stralloc_copys(&addr, ""))
		die_nomem();
	flagesc = 0;
	flagquoted = 0;
	for (i = 0; (ch = arg[i]); ++i) { /*- copy arg to addr, stripping quotes */
		if (flagesc) {
			if (!stralloc_append(&addr, &ch))
				die_nomem();
			flagesc = 0;
		} else {
			if (!flagquoted && ch == terminator)
				break;
			switch (ch)
			{
			case '\\':
				flagesc = 1;
				break;
#ifdef STRIPSINGLEQUOTES
			case '\'':
				flagquoted = !flagquoted;
				break;
#endif
			case '"':
				flagquoted = !flagquoted;
				break;
			default:
				if (!stralloc_append(&addr, &ch))
					die_nomem();
			}
		}
	}
	/*- could check for termination failure here, but why bother? */
	if (!stralloc_append(&addr, ""))
		die_nomem();
	if (liphostok) {
		if ((i = byte_rchr(addr.s, addr.len, '@')) < addr.len) {
			/*- if not, partner should go read rfc 821 */
			if (addr.s[i + 1] == '[') {
				if (!addr.s[i + 1 + ip_scanbracket(addr.s + i + 1, &ip)]) {
					if (ipme_is(&ip)) {
						addr.len = i + 1;
						if (!stralloc_cat(&addr, &liphost))
							die_nomem();
						if (!stralloc_0(&addr))
							die_nomem();
					}
				}
			}
		}
	}
	if (addr.len > 900)
		return 0;
	return 1;
}

void
smtp_helo(char *arg)
{
	seenmail = 0;
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	smtp_greet("250 ");
	if (!arg || !*arg)
	{
		out(" [");
		out(remoteip);
		out("]");
	}
	out("\r\n");
	if (!arg || !*arg)
		dohelo(remoteip);
	else
		dohelo(arg);
}

void
smtp_ehlo(char *arg)
{
	char            size_buf[FMT_ULONG]; /*- needed for SIZE CMD */

	seenmail = 0;
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	smtp_greet("250-");
	if (!arg || !*arg)
	{
		out(" [");
		out(remoteip);
		out("]");
	}
	out("\r\n");
	if (hostname && *hostname && childargs && *childargs)
	{
		char *no_auth_login, *no_auth_plain, *no_cram_md5,
			 *no_cram_sha1, *no_cram_sha256, *no_cram_ripemd,
			 *no_digest_md5;

		no_auth_login = env_get("DISABLE_AUTH_LOGIN");
		no_auth_plain = env_get("DISABLE_AUTH_PLAIN");
		no_cram_md5 = env_get("DISABLE_CRAM_MD5");
		no_cram_sha1= env_get("DISABLE_CRAM_SHA1");
		no_cram_sha256= env_get("DISABLE_CRAM_SHA256");
		no_cram_ripemd= env_get("DISABLE_CRAM_RIPEMD");
		no_digest_md5= env_get("DISABLE_DIGEST_MD5");

		if (!no_auth_login && !no_auth_plain && !no_cram_md5 && !no_cram_sha1
			&& !no_cram_sha256 && !no_cram_ripemd && !no_digest_md5)
		{
			out("250-AUTH LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA256 CRAM-RIPEMD DIGEST-MD5\r\n");
			out("250-AUTH=LOGIN PLAIN CRAM-MD5 CRAM-SHA1 CRAM-SHA256 CRAM-RIPEMD DIGEST-MD5\r\n");
		} else 
		if (!no_auth_login || !no_auth_plain || !no_cram_md5 || !no_cram_sha1 ||
			!no_cram_sha256 || !no_cram_ripemd)
		{
			int flag = 0;

			out("250-AUTH");
			if (!no_auth_login)
				out(" LOGIN");
			if (!no_auth_plain)
				out(" PLAIN");
			if (!no_cram_md5)
				out(" CRAM_MD5");
			out("\r\n");
			if (!no_auth_login)
			{
				out(flag++ == 0 ? "250-AUTH=" : " ");
				out("LOGIN");
			}
			if (!no_auth_plain)
			{
				out(flag++ == 0 ? "250-AUTH=" : " ");
				out("PLAIN");
			}
			if (!no_cram_md5)
			{
				out(flag++ == 0 ? "250-AUTH=" : " ");
				out("CRAM-MD5");
			}
			if (!no_cram_sha1)
			{
				out(flag++ == 0 ? "250-AUTH=" : " ");
				out("CRAM-SHA1");
			}
			if (!no_cram_sha256)
			{
				out(flag++ == 0 ? "250-AUTH=" : " ");
				out("CRAM-SHA256");
			}
			if (!no_cram_ripemd)
			{
				out(flag++ == 0 ? "250-AUTH=" : " ");
				out("CRAM-RIPEMD");
			}
			if (!no_digest_md5)
			{
				out(flag++ == 0 ? "250-AUTH=" : " ");
				out("DIGEST_MD5");
			}
			out("\r\n");
		}
	} 
#ifdef INDIMAIL
	if (smtp_port != ODMR_PORT)
	{
		out("250-PIPELINING\r\n");
		out("250-8BITMIME\r\n");
		if (databytes)
		{
			size_buf[fmt_ulong(size_buf, (unsigned long) databytes)] = 0;
			out("250-SIZE ");
			out(size_buf);
			out("\r\n");
		}
		if (smtp_port != SUBM_PORT)
			out("250-ETRN\r\n");
	} else
		out("250-ATRN\r\n");
#else
	out("250-PIPELINING\r\n");
	out("250-8BITMIME\r\n");
	if (databytes)
	{
		size_buf[fmt_ulong(size_buf, (unsigned long) databytes)] = 0;
		out("250-SIZE ");
		out(size_buf);
		out("\r\n");
	}
	if (smtp_port != SUBM_PORT)
		out("250-ETRN\r\n");
#endif
#ifdef TLS
	if (!ssl && env_get("STARTTLS"))
	{
		stralloc        filename = {0};
		struct stat     st;

		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
		}
		if (!stralloc_copys(&filename, controldir))
			die_nomem();
		if (!stralloc_catb(&filename, "/", 1))
			die_nomem();
		servercert = ((servercert = env_get("SERVERCERT")) ? servercert : "servercert.pem");
		if (!stralloc_cats(&filename, servercert))
			die_nomem();
		if (!stralloc_0(&filename))
			die_nomem();
		if (!stat(filename.s, &st))
			out("250-STARTTLS\r\n");
		alloc_free(filename.s);
	}
#endif
	out("250 HELP\r\n");
	if (!arg || !*arg)
		dohelo(remoteip);
	else
		dohelo(arg);
}

void
smtp_rset(char *arg)
{
	seenmail = 0;
	if (arg && *arg)
	{
		out("501 invalid parameter syntax (#5.3.2)\r\n");
		return;
	}
	if (!stralloc_copys(&rcptto, ""))
		die_nomem();
	if (!stralloc_copys(&mailfrom, ""))
		die_nomem();
	if (!stralloc_copys(&addr, ""))
		die_nomem();
	if (!stralloc_0(&rcptto))
		die_nomem();
	if (!stralloc_0(&mailfrom))
		die_nomem();
	if (!stralloc_0(&addr))
		die_nomem();
	out("250 flushed\r\n");
}

#ifdef BATV
int
check_batv_sig()
{
	int             daynumber = (now() / 86400) % 1000;
	int             i, md5pos, atpos, slpos;
	char            kdate[] = "0000";
	MD5_CTX         md5;
	unsigned char   md5digest[MD5_DIGEST_LENGTH];
	unsigned long   signday;

	if (addr.len >= (11 + 2 * BATVLEN) && stralloc_starts(&addr, "prvs=")) {
		atpos = str_rchr(addr.s, '@');
		addr.s[atpos] = 0;	/*- just for a moment */
		slpos = str_rchr(addr.s, '='); /*- prefer an = sign */
		addr.s[atpos] = '@';
		byte_copy(kdate, 4, addr.s + 5);
		md5pos = 9;
	} else
		return 0; /*- no BATV */
	if (kdate[0] != '0')
		return 0;				/* not known format 0 */
	if (scan_ulong(kdate + 1, &signday) != 3)
		return 0;
	if ((unsigned) (daynumber - signday) > signkeystale)
		return 0; /*- stale bounce */
	MD5_Init(&md5);
	MD5_Update(&md5, kdate, 4);
	addr.len--;
	MD5_Update(&md5, addr.s + slpos + 1, addr.len - slpos - 1);
	MD5_Update(&md5, signkey.s, signkey.len);
	MD5_Final(md5digest, &md5);
	for (i = 0; i < BATVLEN; i++) {
		int             c, x;
		c = addr.s[md5pos + 2 * i];
		if (isdigit(c))
			x = c - '0';
		else
		if (c >= 'a' && c <= 'f')
			x = 10 + c - 'a';
		else
		if (c >= 'A' && c <= 'F')
			x = 10 + c - 'A';
		else
			return 0;
		x <<= 4;
		c = addr.s[md5pos + 2 * i + 1];
		if (isdigit(c))
			x += c - '0';
		else
		if (c >= 'a' && c <= 'f')
			x += 10 + c - 'a';
		else
		if (c >= 'A' && c <= 'F')
			x += 10 + c - 'A';
		else
			return 0;
		if (x != md5digest[i])
			return 0;
	}
	/*- peel off the signature */
	addr.len -= slpos;
	byte_copy(addr.s, addr.len, addr.s + slpos + 1);
	return 1;
}
#endif /*- #ifdef BATV */

#ifdef INDIMAIL
int
pop_bef_smtp(char *mailfrom)
{
	char           *ptr;

	if ((ptr = inquery(RELAY_QUERY, mailfrom, remoteip)))
	{
		if ((authenticated = *ptr))
			relayclient = "";
	} else
	{
		if (userNotFound)
			authenticated = -1;
		else
		{
			out("451 Requested action aborted: database error (#4.3.2)\r\n");
			logerr("qmail-smtpd: ");
			logerrpid();
			logerrf("Database error\n");
			return (1);
		}
	}
	return (0);
}

/*
 * Allow aliasdomains or DEFAULT_DOMAIN to be matched with a real
 * domain in the mailfrom or the authenticated username.
 */
int
domain_compare(char *dom1, char *dom2)
{
	char           *tmpdom1, *tmpdom2;

	if (str_diff(dom1, dom2))
	{
		if (!(tmpdom1 = inquery(DOMAIN_QUERY, dom1, 0)))
		{
			if (userNotFound)
				tmpdom1 = dom1;
			else
			{
				out("451 Requested action aborted: database error (#4.3.2)\r\n");
				logerr("qmail-smtpd: ");
				logerrpid();
				logerrf("Database error\n");
				return (-1);
			}
		}
		if (!(tmpdom2 = inquery(DOMAIN_QUERY, dom2, 0)))
		{
			if (userNotFound)
				tmpdom2 = dom2;
			else
			{
				out("451 Requested action aborted: database error (#4.3.2)\r\n");
				logerr("qmail-smtpd: ");
				logerrpid();
				logerrf("Database error\n");
				return (-1);
			}
		}
		if (str_diff(tmpdom1, tmpdom2))
		{
			err_nogateway(remoteip, mailfrom.s, 0, 1);
			return (1);
		}
	}
	return (0);
}
#endif

int             flagsize = 0;
stralloc        mfparms = { 0 };

int
mailfrom_size(char *arg)
{
	long            r;
	unsigned long   sizebytes = 0;

	scan_ulong(arg, (unsigned long *) &r);
	sizebytes = r;
	msg_size = r;
	if (databytes && (sizebytes > databytes))
		return 1;
	return 0;
}

void
mailfrom_auth(char *arg, int len)
{
	if (authd)
		return;
	if (!stralloc_copys(&user, ""))
		die_nomem();
	if (case_starts(arg, "<>"))
	{
		if (!stralloc_cats(&user, "unknown"))
			die_nomem();
	} else
	{
		while (len)
		{
			if (*arg == '+')
			{
				if (case_starts(arg, "+3D"))
				{
					arg = arg + 2;
					len = len - 2;
					if (!stralloc_cats(&user, "="))
						die_nomem();
				}
				if (case_starts(arg, "+2B"))
				{
					arg = arg + 2;
					len = len - 2;
					if (!stralloc_cats(&user, "+"))
						die_nomem();
				}
			} else
			if (!stralloc_catb(&user, arg, 1))
				die_nomem();
			arg++;
			len--;
		}
	}
	if (!stralloc_0(&user))
		die_nomem();
	if (!remoteinfo)
	{
		remoteinfo = user.s;
		if (!env_unset("TCPREMOTEINFO"))
			die_nomem();
		if (!env_put2("TCPREMOTEINFO", remoteinfo))
			die_nomem();
	}
}

void
mailfrom_parms(char *arg)
{
	int             i;
	int             len;

	len = str_len(arg);
	if (!stralloc_copys(&mfparms, ""))
		die_nomem();
	i = byte_chr(arg, len, '>');
	if (i > 4 && i < len)
	{
		while (len)
		{
			arg++;
			len--;
			if (*arg == ' ' || *arg == '\0')
			{
				if (case_starts(mfparms.s, "SIZE=") && mailfrom_size(mfparms.s + 5))
				{
					flagsize = 1;
					return;
				}
				if (case_starts(mfparms.s, "AUTH="))
					mailfrom_auth(mfparms.s + 5, mfparms.len - 5);
				if (!stralloc_copys(&mfparms, ""))
					die_nomem();
			} else
			if (!stralloc_catb(&mfparms, arg, 1))
				die_nomem();
		}
	}
}

void
smtp_mail(char *arg)
{
#ifdef INDIMAIL
	struct passwd  *pw;
#endif
	char           *x;
	int             ret, envret = 0;
#ifdef SMTP_PLUGIN
	char           *mesg;
	int             i;
#endif
#ifdef USE_SPF
	int             r;
#endif

	/*
	 * If this is the second session
	 * restore original environment.
	 * This is because envrules() could
	 * have modified the environmnent
	 * in the previous session
	 */
	restore_env();
	if (envret)
	{
		post_setup();
		envret = 0;
	}
	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	if (!env_put2("SPFRESULT", "unknown"))
		die_nomem();
	if (!seenhelo)
	{
		out("503 Polite people say hello first (#5.5.4)\r\n");
		return;
	}
	if (!stralloc_copys(&proto, "ESMTP"))
		die_nomem();
#ifdef TLS
	if (ssl && !stralloc_append(&proto, "S"))
		die_nomem();
#endif
	if (authd && !stralloc_append(&proto, "A"))
		die_nomem();
	if (!addrparse(arg))
	{
		err_syntax();
		return;
	}
#if BATV
	if (batvok)
		(void) check_batv_sig(); /*- unwrap in case it's ours */
#endif
	switch ((envret = envrules(addr.s, "from.envrules", "FROMRULES", &errStr)))
	{
	case AM_MEMORY_ERR:
		die_nomem();
	case AM_FILE_ERR:
		die_control();
	case AM_REGEX_ERR:
		die_regex();
	case 0:
		break;
	default:
		if (envret > 0)
		{
			/*
			 * No point in setting Following environment variables as they have been
			 * read in the function setup() which gets called only once
			 * SMTPS, NODNSCHECK, RELAYCLIENT, TCPREMOTEINFO, TCPREMOTEHOST, TCPLOCALIP
			 * TCPLOCALHOST, TCPREMOTEIP, GREETDELAY, BADHELO, BADHELOCHECK
			 */
			databytes_setup(); /*- so that it is possible to set DATABYTES again using envrules */
			post_setup();
			log_rules(remoteip, addr.s, authd ? remoteinfo : 0, envret);
		}
		break;
	}
	if (!(ret = tablematch("hostaccess", remoteip, addr.s + str_chr(addr.s, '@'))))
	{
		err_hostaccess(remoteip, addr.s);
		return;
	} else
	if (ret == -1)
		die_control();
	else
	if (ret == -2)
		die_nomem();
	flagsize = 0;
	mailfrom_parms(arg);
	if ((requireauth = env_get("REQUIREAUTH")) && !authd)
	{
		err_authrequired();
		return;
	}
	/*- Terminate SMTP Session immediatly if BLACKHOLED Sender is seen */
	switch (address_match(bhsndFn, &addr, bhfok ? &bhf : 0, bhfok ? &mapbhf : 0, bhpok ? &bhp : 0, &errStr))
	{
	case 1: /*- flow through */
		err_bhf(remoteip, addr.s); /*- This sets NULLQUEUE */
	case 0:
		break;
	case -1:
		die_nomem();
	default:
		err_addressmatch(errStr);
		return;
	}
	/*- badmailfrom, badmailpatterns */
	switch (address_match(bmfFn, &addr, bmfok ? &bmf : 0, bmfok ? &mapbmf : 0, bmpok ? &bmp : 0, &errStr))
	{
	case 1:
		err_bmf(remoteip, addr.s);
		return;
	case 0:
		break;
	case -1:
		die_nomem();
	default:
		err_addressmatch(errStr);
		return;
	}

	if (!nodnscheck)
	{
		switch (dnscheck(addr.s, addr.len - 1, 0))
		{
		case DNS_HARD:
			err_hmf(remoteip, arg, 1);
			return;
		case DNS_SOFT:
			err_smf();
			return;
		case DNS_MEM:
			die_nomem();
		}
	}
	if ((bouncemail = env_get("BOUNCEMAIL")))
	{
		err_maps(bouncemail, remoteip, addr.s);
		return;
	}
	if (env_get("SPAMFILTER"))
	{
		/*- spamignore, spamignorepatterns */
		switch (address_match(spfFn, &addr, spfok ? &spf : 0, spfok ? &mapspf : 0, sppok ? &spp : 0, &errStr))
		{
		case 1:
			if (!env_unset("SPAMFILTER"))
				die_nomem();
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr);
			return;
		}
	}
	if (!stralloc_copys(&rcptto, ""))
		die_nomem();
	if (!stralloc_copys(&mailfrom, addr.s))
		die_nomem();
	if (!stralloc_0(&mailfrom))
		die_nomem();
	if ((x = env_get("MAX_RCPT_ERRCOUNT")))
		scan_int(x, &max_rcpt_errcount);
	else
		max_rcpt_errcount = -1;
	rcptcount = rcpt_errcount = 0;
	/*- relaymailfrom */
	if (!relayclient)
	{
		switch (address_match("relaymailfrom", &mailfrom, rmfok ? &rmf : 0, rmfok ? &maprmf : 0, 0, &errStr))
		{
		case 1:
			relayclient = "";
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr);
			return;
		}
	}
#ifdef INDIMAIL
	/*
	 * closed user group mailing
	 * allow only sender domains listed in rcpthosts to
	 * send mails.
	 */
	if (env_get("CUGMAIL"))
	{
		if (!addrallowed(addr.s))
		{
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" Invalid SENDER address: MAIL from <");
			logerr(mailfrom.s);
			logerrf(">\n");
			out("553 SMTP Access denied (#5.7.1)\r\n");
			return;
		}
		if (!(pw = inquery(PWD_QUERY, mailfrom.s, 0)))
		{
			if (userNotFound)
			{
				/*- 
				 * Accept the mail as denial could be stupid
				 * like the vrfy command
				 */
				logerr("qmail-smtpd: ");
				logerrpid();
				logerr(remoteip);
				logerr(" mail from invalid user <");
				logerr(mailfrom.s);
				logerrf(">\n");
				out("553 SMTP Access denied (#5.7.1)\r\n");
				sleep(5); /*- Prevent DOS */
				return;
			} else
			{
				logerr("qmail-smtpd: ");
				logerrpid();
				logerr(remoteip);
				logerrf(" Database error\n");
				out("451 Requested action aborted: database error (#4.3.2)\r\n");
				return;
			}
		} else
		if (is_inactive || pw->pw_gid & NO_SMTP)
		{
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" SMTP Access denied to <");
			logerr(mailfrom.s);
			logerr("> ");
			logerrf(is_inactive ? "user inactive" : "No SMTP Flag");
			out("553 SMTP Access denied (#5.7.1)\r\n");
			return;
		}
	}	  /*- if (env_get("CUGMAIL")) */
	/*-
	 * ANTISPOOFING 
	 * Delivery to local domains
	 * If the mailfrom is local and rcptto is local, do not allow
	 * receipt without authentication (unless MASQUERADE is set).
	 * (do not allow spoofing for local users)
	 */
	if (!relayclient && !authd && env_get("ANTISPOOFING") && addrallowed(mailfrom.s))
	{
		if (pop_bef_smtp(mailfrom.s)) /*- will set the variable authenticated */
			return; /*- temp error */
		if (authenticated != 1) /*- in case pop-bef-smtp also is negative */
		{
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" unauthenticated local SENDER address: MAIL from <");
			logerr(mailfrom.s);
			logerrf(">\n");
			out("530 authentication required for local users (#5.7.1)\r\n");
			return;
		}
	}
	x = env_get("MASQUERADE");
	if ((!x || (x && *x)) && authd) {
		int             at1, at2, iter_pass, flag;
		char           *dom1, *dom2, *allowed;

		if (mailfrom.s[at1 = str_chr(mailfrom.s, '@')]) {
			dom1 = mailfrom.s + at1 + 1;
			if (!addrallowed(mailfrom.s)) {
				err_nogateway(remoteip, mailfrom.s, 0, 1);
				return;
			}
			mailfrom.s[at1] = 0;
			for (flag = 0, iter_pass = 0;;iter_pass++) {
				if (x && *x) {
					allowed = iter_pass ? remoteinfo : x;
				}
				else {
					allowed = remoteinfo;
					iter_pass++;
				}
				if (allowed[at2 = str_chr(allowed, '@')]) {
					dom2 = allowed + at2 + 1;
					allowed[at2] = 0;
					if (str_diff(mailfrom.s, allowed)) {
						allowed[at2] = '@';
						flag = 1;
					}
					allowed[at2] = '@';
					/*
					 * now compare domains 
					 */
					if (domain_compare(dom1, dom2) == 1)
						flag = 1;
				} else {
					if (str_diff(mailfrom.s, allowed))
						flag = 1;
					/*
					 * now compare mailfrom domain with $DEFAULT_DOMAIN
					 */
					if ((dom2 = env_get("DEFAULT_DOMAIN"))) {
						if (domain_compare(dom1, dom2) == 1)
							flag = 1;
					}
				}
				if (!flag) {
					mailfrom.s[at1] = '@';
					break;
				}
				if (iter_pass == 1) {
					mailfrom.s[at1] = '@';
					err_nogateway(remoteip, mailfrom.s, 0, 1);
					break;
				}
			} /*- for (;;) */
		} else {
			if (x && *x) {
				if (str_diff(mailfrom.s, x) && str_diff(mailfrom.s, remoteinfo)) {
					err_nogateway(remoteip, mailfrom.s, 0, 1);
					return;
				}
			} else {
				if (str_diff(mailfrom.s, remoteinfo)) {
					err_nogateway(remoteip, mailfrom.s, 0, 1);
					return;
				}
			}
		}
	}
#endif
#ifdef USE_SPF
	flagbarfspf = 0;
	if (spfbehavior && !relayclient)
	{
#ifdef IPV6
		switch (r = spfcheck(spfipv6 ? remoteip : remoteip4))
#else
		switch (r = spfcheck(remoteip))
#endif
		{
		case SPF_OK:
			if (!env_put2("SPFRESULT", "pass"))
				die_nomem();
			break;
		case SPF_NONE:
			if (!env_put2("SPFRESULT", "none"))
				die_nomem();
			break;
		case SPF_UNKNOWN:
			if (!env_put2("SPFRESULT", "unknown"))
				die_nomem();
			break;
		case SPF_NEUTRAL:
			if (!env_put2("SPFRESULT", "neutral"))
				die_nomem();
			break;
		case SPF_SOFTFAIL:
			if (!env_put2("SPFRESULT", "softfail"))
				die_nomem();
			break;
		case SPF_FAIL:
			if (!env_put2("SPFRESULT", "fail"))
				die_nomem();
			break;
		case SPF_ERROR:
			if (!env_put2("SPFRESULT", "error"))
				die_nomem();
			break;
		}
		switch (r)
		{
		case SPF_NOMEM:
			die_nomem();
		case SPF_ERROR:
			if (spfbehavior < 2)
				break;
			out("451 SPF lookup failure (#4.3.0)\r\n");
			return;
		case SPF_NONE:
		case SPF_UNKNOWN:
			if (spfbehavior < 6)
				break;
		case SPF_NEUTRAL:
			if (spfbehavior < 5)
				break;
		case SPF_SOFTFAIL:
			if (spfbehavior < 4)
				break;
		case SPF_FAIL:
			if (spfbehavior < 3)
				break;
			if (!spfexplanation(&spfbarfmsg))
				die_nomem();
			if (!stralloc_0(&spfbarfmsg))
				die_nomem();
			flagbarfspf = 1;
		}
	} else
		env_unset("SPFRESULT");
	if (flagbarfspf)
	{
		err_spf();
		return;
	}
#endif /*- #ifdef USE_SPF */
	/*- authdomains */
	if (chkdomok)
	{
		switch (address_match("authdomains", &mailfrom, chkdomok ? &chkdom : 0, chkdomok ? &mapchkdom : 0, 0, &errStr))
		{
		case 0:
			chkdomok = 0;
		case 1:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr);
			return;
		}
	}
#ifdef SMTP_PLUGIN
	for (i = 0;i < plugin_count;i++)
	{
		if (!plug[i] || !plug[i]->mail_func)
			continue;
		if (plug[i]->mail_func(remoteip, addr.s, &mesg))
		{
			out(mesg);
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr("plugin(from)[");
			strnum[fmt_ulong(strnum, i)] = 0;
			logerr(strnum);
			logerr("]: ");
			logerr(mesg);
			logerrf("\n");
			return;
		}
	}
#endif
#if BATV
	if (batvok)
	{
		isbounce = 0;
		if (addr.len <= 1 /*- null term */
				|| stralloc_starts(&addr, "mailer-daemon@") 
				|| stralloc_starts(&addr, "Mailer-Daemon@") 
				|| stralloc_starts(&addr, "MAILER-DAEMON@"))
		{
			ret = str_rchr(addr.s,'@');
			if (!addr.s[ret] || !constmap(&mapnosign, addr.s + ret + 1, addr.len - ret - 2))
				isbounce = 1;
		}
	}
#endif /*- BATV*/
	seenmail = 1;
	out("250 ok\r\n");
}

void
smtp_rcpt(char *arg)
{
	int             allowed_rcpthosts = 0, isgoodrcpt = 0, result = 0;
	char           *tmp;
#if BATV
	int             ws = -1;
#endif
#ifdef SMTP_PLUGIN
	char           *mesg;
	int             i;
#endif

	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	if (!seenmail)
	{
		err_wantmail();
		return;
	}
	if (!addrparse(arg))
	{
		err_syntax();
		return;
	}
#if BATV
	if (batvok)
		ws = check_batv_sig(); /*- always strip sig, even if it's not a bounce */
#endif
	if (addrrelay())
	{
		err_relay();
		return;
	}
	/*- goodrcpt, goodrcptpatterns */
	switch (address_match(grcptFn, &addr, chkgrcptok ? &grcpt : 0, chkgrcptok ? &mapgrcpt : 0, chkgrcptokp ? &grcptp : 0, &errStr))
	{
	case 1:
		isgoodrcpt = 4;
	case 0:
		break;
	case -1:
		die_nomem();
	default:
		err_addressmatch(errStr);
		return;
	}
	/*- RECIPIENT BAD check */
	if (isgoodrcpt != 4)
	{
		/*- badrcptto, badrcptpatterns */
		switch (address_match(rcpFn, &addr, rcpok ? &rcp : 0, rcpok ? &maprcp : 0, brpok ? &brp : 0, &errStr))
		{
		case 1:
			err_rcp(remoteip, mailfrom.s, addr.s);
			return;
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr);
			return;
		}
	}
	if (acclistok)
	{
		switch (mail_acl(&acclist, qregex, mailfrom.s, addr.s, 0))
		{
		case 1:
			err_acl(remoteip, mailfrom.s, addr.s);
			return;
		case 0:
			break;
		default:
			out("451 Requested action aborted: local system failure (#4.3.0)\r\n");
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr(remoteip);
			logerr(" accesslist: ");
			logerr(error_str(errno));
			logerrf("\n");
			return;
		}
	}
	/*
	 * If AUTH_ALL is defined, allowed_rcpthosts = 0 
	 */
	if (!chkdomok && !env_get("AUTH_ALL"))
		allowed_rcpthosts = addrallowed(addr.s);
	if (relayclient)
	{
		--addr.len;
		if (!stralloc_cats(&addr, relayclient))
			die_nomem();
		if (!stralloc_0(&addr))
			die_nomem();
	} else	   /*- RELAYCLIENT not set */
	if (!allowed_rcpthosts)	/*- not in rcpthosts - delivery to remote domains */
	{
#ifdef INDIMAIL
		if (env_get("CHECKRELAY") && pop_bef_smtp(mailfrom.s))
			return;
		if (authenticated != 1)
		{
			err_nogateway(remoteip, mailfrom.s, addr.s, 0);
			return;
		}
#else
		err_nogateway(remoteip, mailfrom.s, addr.s, 0);
		return;
#endif /*- #ifdef INDIMAIL */
	}	  /*- if (!allowed_rcpthosts) */
	/*
	 * If rcptto is local, check status of recipients
	 * (do not allow mail to be sent to invalid users)
	 */
	if (allowed_rcpthosts == 1 && (tmp = env_get("CHECKRECIPIENT")))
	{
		/*- chkrcptdomains */
		switch (address_match("chkrcptdomains", &addr, chkrcptok ? &chkrcpt : 0, chkrcptok ? &mapchkrcpt : 0, 0, &errStr))
		{
		case 1:
			chkrcptok = 0;
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr);
			return;
		}
		if (!chkrcptok) 
		{
			if (isgoodrcpt != 4)
			{
#ifdef INDIMAIL
				if (*tmp)
					scan_int(tmp, &isgoodrcpt);
				else
#endif
					isgoodrcpt = 3; /*- default is cdb */
			}
			switch (isgoodrcpt)
			{
#ifdef INDIMAIL
				case 1: /* reject if user not in sql db */
					result = check_recipient_sql(addr.s);
					break;
				case 2: /* reject if user not in both recipient.cdb and sql db */
					if ((result = !check_recipient_cdb(addr.s)))
						result = check_recipient_sql(addr.s);
					break;
#endif
				case 4:
					result = 0;
					break;
				case 3: /* reject if user not in recipient */
				default:
					result = (!check_recipient_cdb(addr.s) ? 1 : 0);
					break;
			}
			if (result > 0)
			{
				rcpt_errcount++;
				sleep(5); /*- Prevent DOS */
			}
			switch (result)
			{
			case 1:
				err_mailbox(mailfrom.s, addr.s, "is absent on this domain (#5.1.1)");
				return;
			case 2:
				err_mailbox(mailfrom.s, addr.s, "is inactive on this domain (#5.2.1)");
				return;
			case 3:
				err_mailbox(mailfrom.s, addr.s, "is overquota on this domain (#5.2.2)");
				return;
			}
		} 
	}
	if (max_rcpt_errcount != -1 && rcpt_errcount >= max_rcpt_errcount)
	{
		err_rcpt_errcount(mailfrom.s, rcpt_errcount);
		return;
	}
	/*- RECIPIENT BLACKHOLE */
	if (isgoodrcpt != 4)
	{
		/*- blackholedrcpt, blackholedrcptpatterns */
		switch (address_match(bhrcpFn, &addr, bhrcpok ? &bhrcp : 0, bhrcpok ? &mapbhrcp : 0, bhbrpok ? &bhbrp : 0, &errStr))
		{
		case 1:
			err_bhrcp(remoteip, mailfrom.s, addr.s);
		case 0:
			break;
		case -1:
			die_nomem();
		default:
			err_addressmatch(errStr);
			return;
		}
	}
#if BATV
	/* if wasn't signed and was bounce, sideline it */
	if (batvok && !relayclient && !ws && isbounce) {
		logerr("qmail-smtpd: ");
		logerrpid();
		logerr("bad bounce: at ");
		logerr(remoteip);
		logerr(" to ");
		logerr(addr.s);
		logerrf("\n");
		out("553 Not our message (#5.7.1)\r\n");
		return;
	}
#endif
#ifdef SMTP_PLUGIN
	for (i = 0;i < plugin_count;i++)
	{
		if (!plug[i] || !plug[i]->rcpt_func)
			continue;
		if (plug[i]->rcpt_func(remoteip, mailfrom.s, addr.s, &mesg))
		{
			out(mesg);
			logerr("qmail-smtpd: ");
			logerrpid();
			logerr("plugin(rcpt)[");
			strnum[fmt_ulong(strnum, i)] = 0;
			logerr(strnum);
			logerr("]: ");
			logerr(mesg);
			logerrf("\n");
			return;
		}
	}
#endif
	/*- Check on max. number of RCPTS */
	if (maxrcptcount > 0 && (rcptcount + 1) > maxrcptcount)
	{
		err_mrc(remoteip, mailfrom.s, arg);
		return;
	}
	if (tarpitcount && (rcptcount + 1) >= tarpitcount)
		while (sleep(tarpitdelay));	/*- TARPIT Delay */
	if (mailfrom.len == 1 && (rcptcount + 1) > 1)
	{
		err_badbounce();
		return;
	}
	rcptcount++;
	if (!stralloc_cats(&rcptto, "T"))
		die_nomem();
	if (!stralloc_cats(&rcptto, addr.s))
		die_nomem();
	if (!stralloc_0(&rcptto))
		die_nomem();
	out("250 ok\r\n");
}

ssize_t
saferead(int fd, char *buf, int len)
{
	int             r;

	flush();
#ifdef TLS
	if (ssl && fd == ssl_rfd)
		r = ssl_timeoutread(timeout, ssl_rfd, ssl_wfd, ssl, buf, len);
	else
		r = timeoutread(timeout, fd, buf, len);
#else
	r = timeoutread(timeout, fd, buf, len);
#endif
	if (r == -1)
	{
		if (errno == error_timeout)
			die_alarm();
	}
	if (r < 0)
		die_read();
	return r;
}

/*-
 * =0 after boundary is found in body,
 * until blank line
 *
 */
int             linespastheader;
int             flagexecutable, flagbody, flagqsbmf, boundary_start;
char            linetype;

stralloc        pline = { 0 };
stralloc        line = { 0 };
stralloc        content = { 0 };
stralloc        boundary = { 0 };

/*
 * def put(ch):
 * line.append(ch)
 * if ch == '\n':
 * if linespastheader == 0:
 * if line.startswith('Content-Type:'):
 * content = 
 * 
 * put() puts characters into the queue.  We remember those characters
 * and form them into a line.  When we get a newline, we examine the
 * line.  If we're currently in a header (0 linespastheader), we look
 * for Content-Type.  If we're at the newline that ends a header, we
 * look to see if the content is multipart.  If it is, then we push
 * the current boundary, remember the boundary, otherwise we set the
 * boundary to the empty string.  Set the linespastheader to 1.  When
 * linespastheader is 1, and the boundary is empty, scan the line for
 * signatures.  If the boundary is non-empty, look for a match against
 * the boundary.  If it matches and is followed by two dashes, pop the
 * boundary, otherwise set linespastheader to 0.
 */

void
put(char *ch)
{
	char           *cp, *cpstart, *cpafter;
	unsigned int    len;

	if (!sigsok && !bodyok)
	{
		if (BytesToOverflow && !--BytesToOverflow)
			qmail_fail(&qqt);
		msg_size++;
		qmail_put(&qqt, ch, 1);
		return;
	}
	if (line.len < 1024 && !stralloc_catb(&line, ch, 1))
		die_nomem();
	if (*ch == '\n' && (sigsok || bodyok))
	{
		if (linespastheader == 0)
		{
			if (line.len > 1 && isspace(line.s[0]))
			{
				if (!stralloc_cat(&pline, &line))
					die_nomem();
			} else
			if (!stralloc_copy(&pline, &line))
				die_nomem();
		}
		if (bodycheck(&body, &pline, &content_desc, linespastheader == 0) == 1)
		{
			flagbody = 1;
			/*- turn off virus/content-filtering on first match */
			sigsok = bodyok = 0;
			qmail_fail(&qqt);
		}
		if (linespastheader == 0)
		{
			if (line.len == 1)
			{
				linespastheader = 1;
				if (flagqsbmf)
				{
					flagqsbmf = 0;
					linespastheader = 0;
				}
				if (content.len) /*- MIME header */
				{
					cp = content.s;
					len = content.len;
					while (len && (*cp == ' ' || *cp == '\t'))
					{
						++cp;
						--len;
					}
					cpstart = cp;
					if (len && *cp == '"') /*- might be commented */
					{
						++cp;
						--len;
						cpstart = cp;
						while (len && *cp != '"')
						{
							++cp;
							--len;
						}
					} else
					{
						while (len && *cp != ' ' && *cp != '\t' && *cp != ';')
						{
							++cp;
							--len;
						}
					}
					if (!case_diffb(cpstart, cp - cpstart, "message/rfc822"))
						linespastheader = 0;
					cpafter = content.s + content.len;
					while ((cp += byte_chr(cp, cpafter - cp, ';')) != cpafter)
					{
						++cp;
						while (cp < cpafter && (*cp == ' ' || *cp == '\t'))
							++cp;
						if (case_startb(cp, cpafter - cp, "boundary="))
						{
							cp += 9;	/*- after boundary= */
							if (cp < cpafter && *cp == '"')
							{
								++cp;
								cpstart = cp;
								while (cp < cpafter && *cp != '"')
									++cp;
							} else
							{
								cpstart = cp;
								while (cp < cpafter && *cp != ';' && *cp != ' ' && *cp != '\t')
									++cp;
							}
							/*
							 * push the current boundary. 
							 * Append a null and remember start. 
							 */
							if (!stralloc_0(&boundary))
								die_nomem();
							boundary_start = boundary.len;
							if (!stralloc_cats(&boundary, "--"))
								die_nomem();
							if (!stralloc_catb(&boundary, cpstart, cp - cpstart))
								die_nomem();
							break;
						}
					}
				}
			} else
			{	/*- non-blank header line */
				if ((*line.s == ' ' || *line.s == '\t'))
				{
					switch (linetype)
					{
					case 'C':
						if (!stralloc_catb(&content, line.s, line.len - 1))
							die_nomem();
						break;
					default:
						break;
					}
				} else
				{
					if (case_startb(line.s, line.len, "content-type:"))
					{
						if (!stralloc_copyb(&content, line.s + 13, line.len - 14))
							die_nomem();
						linetype = 'C';
					} else
						linetype = ' ';
				}
			}
		} else	   /*- non-header line */
		{
			if (boundary.len - boundary_start && *line.s == '-' && line.len > (boundary.len - boundary_start) &&
				!str_diffn(line.s, boundary.s + boundary_start, boundary.len - boundary_start))
			{	/*- matches a boundary */
				if (line.len > boundary.len - boundary_start + 2 && line.s[boundary.len - boundary_start + 0] == '-' &&
					line.s[boundary.len - boundary_start + 1] == '-')
				{
					/*
					 * XXXX - pop the boundary here 
					 */
					if (boundary_start)
						boundary.len = boundary_start - 1;
					boundary_start = boundary.len;
					while (boundary_start--)
						if (!boundary.s[boundary_start])
							break;
					boundary_start++;
					linespastheader = 2;
				} else
					linespastheader = 0;
			} else
			if (linespastheader == 1)
			{	/*- first line -- match a signature?  */
				if (/*- mailfrom.s[0] == '\0' && */
					   str_start(line.s, "Hi. This is the "))
					flagqsbmf = 1;
				else
				if (/*- mailfrom.s[0] == '\0' && */
					   str_start(line.s, "This message was created automatically by mail delivery software"))
					flagqsbmf = 1;
				else
				if (sigscheck(&line, &virus_desc, linespastheader == 0))
				{
					flagexecutable = 1;
					/*- turn off virus/content-filtering on first match */
					sigsok = bodyok = 0;
					qmail_fail(&qqt);
				}
				linespastheader = 2;
			}
			if (flagqsbmf && str_start(line.s, "---"))
				linespastheader = 0;
		}
		line.len = 0;
	}
	if (BytesToOverflow && !--BytesToOverflow)
		qmail_fail(&qqt);
	msg_size++;
	qmail_put(&qqt, ch, 1);
}

int
blast(int *hops)
{
	char            ch;
	int             state, err;
	int             flaginheader;
	unsigned int    pos;		/*- number of bytes since most recent \n, if fih */
	int             flagmaybew;	/*- 1 if this line might match RETURN-RECEIPT, if fih */
	int             flagmaybex;	/*- 1 if this line might match RECEIVED, if fih */
	int             flagmaybey;	/*- 1 if this line might match \r\n, if fih */
	int             flagmaybez;	/*- 1 if this line might match DELIVERED, if fih */
	int             seencr;

	state = 1;
	*hops = 0;
	flaginheader = 1;
	dsn = pos = 0;
	flagmaybew = flagmaybex = flagmaybey = flagmaybez = 1;
	msg_size = 0;
	seencr = 0;	/*- qmail-smtpd-newline patch */
	for (;;)
	{
		if ((err = substdio_get(&ssin, &ch, 1)) <= 0)
			return (1);
		if (ch == '\n')
		{
			if (seencr == 0)
			{
				substdio_seek(&ssin, -1);
				ch = '\r';
			}
		}
		if (ch == '\r')
			seencr = 1;
		else
			seencr = 0;
		if (flaginheader)
		{
			if (pos > 8 && pos < 14)
			{
				if (ch != "return-receipt"[pos] && ch != "RETURN-RECEIPT"[pos])
					flagmaybew = 0;
				if (flagmaybew && pos == 13)
					dsn = 1;
			}
			if (pos < 9)
			{
				if (ch != "delivered"[pos] && ch != "DELIVERED"[pos])
					flagmaybez = 0;
				if (flagmaybez && pos == 8)
					++*hops;
				if (pos < 8 && ch != "received"[pos] && ch != "RECEIVED"[pos])
					flagmaybex = 0;
				if (flagmaybex && pos == 7)
					++*hops;
				if (pos < 2 && ch != "\r\n"[pos])
					flagmaybey = 0;
				if (flagmaybey && pos == 1)
					flaginheader = 0;
			}
			/*
			 * We are interested only in return-receipt, delivered, received
			 * headers. So no point in checking beyond pos = 13
			 */
			if (pos < 14)
				++pos;
			if (ch == '\n')
			{
				pos = 0;
				flagmaybew = flagmaybex = flagmaybey = flagmaybez = 1;
			}
		}
		switch (state)
		{
		case 0:
			if (ch == '\n')
				straynewline();
			if (ch == '\r')
			{
				state = 4;
				continue;
			}
			break;
		case 1:/*- \r\n */
			if (ch == '\n')
				straynewline();
			if (ch == '.')
			{
				state = 2;
				continue;
			}
			if (ch == '\r')
			{
				state = 4;
				continue;
			}
			state = 0;
			break;
		case 2:/*- \r\n + . */
			if (ch == '\n')
				straynewline();
			if (ch == '\r')
			{
				state = 3;
				continue;
			}
			state = 0;
			break;
		case 3:/*- \r\n + .\r */
			if (ch == '\n')
				return (0);
			put(".");
			put("\r");
			if (ch == '\r')
			{
				state = 4;
				continue;
			}
			state = 0;
			break;
		case 4:/*- + \r */
			if (ch == '\n')
			{
				state = 1;
				break;
			}
			if (ch != '\r')
			{
				put("\r");
				state = 0;
			}
		}
		put(&ch);
	} /*- for (;;) */
	return (0);
}

#ifdef USE_SPF
void
spfreceived()
{
	stralloc        sa = { 0 };
	stralloc        rcvd_spf = { 0 };

	if (!spfbehavior || relayclient)
		return;

	if (!stralloc_copys(&rcvd_spf, "Received-SPF: "))
		die_nomem();
	if (!spfinfo(&sa))
		die_nomem();
	if (!stralloc_cat(&rcvd_spf, &sa))
		die_nomem();
	if (!stralloc_append(&rcvd_spf, "\n"))
		die_nomem();
	if (BytesToOverflow)
	{
		BytesToOverflow -= rcvd_spf.len;
		if (BytesToOverflow <= 0)
			qmail_fail(&qqt);
	}
	qmail_put(&qqt, rcvd_spf.s, rcvd_spf.len);
}
#endif

void
acceptmessage(unsigned long qp)
{
	datetime_sec    when;

	when = now();
	out("250 ok ");
	accept_buf[fmt_ulong(accept_buf, (unsigned long) when)] = 0;
	out(accept_buf);
	out(" qp ");
	accept_buf[fmt_ulong(accept_buf, qp)] = 0;
	out(accept_buf);
	out("\r\n");
}

static void
create_logfilter()
{
	int             fd;
	char           *tmpdir, *x;
	static stralloc tmpFile = { 0 };

	if (env_get("LOGFILTER"))
	{
		if (!(tmpdir = env_get("TMPDIR")))
			tmpdir = "/tmp";
		if (!stralloc_copys(&tmpFile, tmpdir))
			die_nomem();
		if (!stralloc_cats(&tmpFile, "/smtpFilterXXX"))
			die_nomem();
		if (!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
			die_nomem();
		if (!stralloc_0(&tmpFile))
			die_nomem();
		if ((x = env_get("SPAMFD")))
			scan_int(x, &spamfd);
		if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
			die_logfilter();
		if (unlink(tmpFile.s))
			die_logfilter();
		if (dup2(fd, spamfd) == -1)
			die_logfilter();
		if (fd != spamfd)
			close(fd);
	}
	return;
}

void
smtp_data(char *arg)
{
	int             hops;
	unsigned long   qp;
	char           *qqx;
#ifdef SMTP_PLUGIN
	int             i;
	char           *mesg;
#endif

#ifdef INDIMAIL
	sqlmatch_close_db();
#endif
	if (arg && *arg)
	{
		out("501 invalid parameter syntax (#5.3.2)\r\n");
		return;
	}
	if (!seenmail)
	{
		err_wantmail();
		return;
	}
	if (!rcptto.len || !rcptcount)
	{
		err_wantrcpt();
		return;
	}
	if (greyip && !relayclient)
	{
#ifdef IPV6
		switch (greylist(greyip, remoteip4, mailfrom.s, rcptto.s, rcptto.len, err_greytimeout, err_grey_tmpfail))
#else
		switch (greylist(greyip, remoteip, mailfrom.s, rcptto.s, rcptto.len, err_greytimeout, err_grey_tmpfail))
#endif
		{
		case 1: /*- success */
			break;
		case 0:
			err_grey();
			/*- flow through */
		case -1:
			return;
		case -2:
			die_nomem();
		}
	}
	seenmail = 0;
	/*- Return error if incoming SMTP msg exceeds DATABYTES */
	if (flagsize)
	{
		err_size(remoteip, mailfrom.s, rcptto.s, rcptto.len);
		return;
	}
	if (databytes)
		BytesToOverflow = databytes + 1;
	if (sigsok)
	{
		boundary.len = 0;
		boundary_start = 0;
		content.len = 0;
		linespastheader = 0;
		flagexecutable = 0;
		flagqsbmf = 0;
		linetype = ' ';
		virus_desc = "";
	}
	create_logfilter();
	if (qmail_open(&qqt) == -1)
	{
		err_qqt(remoteip);
		return;
	}
	qp = qmail_qp(&qqt); /*- pid of queue process */
	out("354 go ahead\r\n");
	if (proto.len)
	{
		if (!stralloc_0(&proto))
			die_nomem();
		protocol = proto.s;
	}
	received(&qqt, (char *) protocol, local, remoteip, remotehost, remoteinfo, fakehelo);
#ifdef USE_SPF
	spfreceived();
#endif
	/*- write the body */
	if (!blast(&hops)) {
		hops = (hops >= maxhops);
		if (hops)
			qmail_fail(&qqt);
#ifdef SMTP_PLUGIN
		for (i = 0;i < plugin_count;i++)
		{
			if (!plug[i] || !plug[i]->data_func)
				continue;
			if (plug[i]->data_func(local, remoteip, remotehost, remoteinfo, &mesg))
			{
				out(mesg);
				logerr("qmail-smtpd: ");
				logerrpid();
				logerr("plugin(data)[");
				strnum[fmt_ulong(strnum, i)] = 0;
				logerr(strnum);
				logerr("]: ");
				logerr(mesg);
				logerrf("\n");
				return;
			}
		}
#endif
	} else
		qmail_fail(&qqt);
	/*- write the envelope */
	qmail_from(&qqt, mailfrom.s);
	qmail_put(&qqt, rcptto.s, rcptto.len);
	/* 
	 * RFC 1047, In case we are accepting the mail
	 * we should immediately respond
	 * back to client as early as possible to
	 * avoid message duplication
	 */
	qqx = qmail_close(&qqt);
	if (!*qqx) /*- mail is now in queue */
	{
		acceptmessage(qp);
		log_trans(remoteip, mailfrom.s, rcptto.s, rcptto.len, authd ? remoteinfo : 0, 0);
		return;
	}
	/*- you will reach here if qmail_fail() was called or if qmail_close returns error */
	if (flagexecutable || flagbody)
	{
		sigsok = sigsok_orig;
		bodyok = bodyok_orig;
		if (flagexecutable)
		{
			log_virus(remoteip, virus_desc, mailfrom.s, rcptto.s, rcptto.len, flagblackhole);
			flagexecutable = flagblackhole = 0;
			return;
		}
		if (flagbody)
		{
			log_virus(remoteip, content_desc, mailfrom.s, rcptto.s, rcptto.len, flagblackhole);
			flagbody = flagblackhole = 0;
			return;
		}
	}
	if (hops)
	{
		err_hops();
		return;
	}
	if (databytes && !BytesToOverflow)
	{
		err_size(remoteip, mailfrom.s, rcptto.s, rcptto.len);
		return;
	}
	if (*qqx == 'D')
		out("554 ");
	else
		out("451 ");
	out(qqx + 1);
	out("\r\n");
	err_queue(remoteip, mailfrom.s, rcptto.s, rcptto.len,
		authd ? remoteinfo : 0, qqx + 1, *qqx == 'D', qp);
}

int
authgetl(void)
{
	int             i;

	if (!stralloc_copys(&authin, ""))
		die_nomem();
	for (;;)
	{
		if (!stralloc_readyplus(&authin, 1))
			die_nomem(); /*- XXX */
		i = substdio_get(&ssin, authin.s + authin.len, 1);
		if (i != 1)
			die_read();
		if (authin.s[authin.len] == '\n')
			break;
		++authin.len;
	}
	if (authin.len > 0 && authin.s[authin.len - 1] == '\r')
		--authin.len;
	authin.s[authin.len] = 0;
	if (*authin.s == '*' && *(authin.s + 1) == 0)
		return err_authabrt();
	if (authin.len == 0)
		return err_input();
	return (authin.len);
}

#ifndef INDIMAIL
#define AUTH_LOGIN       1
#define AUTH_PLAIN       2
#define AUTH_CRAM_MD5    3
#define AUTH_CRAM_SHA1   4
#define AUTH_CRAM_SHA256 5
#define AUTH_CRAM_RIPEMD 6
#define AUTH_DIGEST_MD5  7
#endif

int             po[2] = {-1, -1};
stralloc        authmethod = {0};

int
authenticate(int method)
{
	int             child, wstat, i, n;
	int             pi[2];
	char            respbuf[1024];

	if (!stralloc_0(&user))
		die_nomem();
	if (!stralloc_0(&pass))
		die_nomem();
	if (!stralloc_0(&resp))
		die_nomem();
	if (pipe(pi) == -1)
		return err_pipe();
	if (pipe(po) == -1)
		return err_pipe();
	switch (child = fork())
	{
	case -1:
		return err_fork();
	case 0:
		close(pi[1]);
		close(po[0]);
		if (pi[0] != 3)
		{
			if (dup2(pi[0], 3) == -1)
				return err_write();
			close(pi[0]);
		}
		if (po[1] != 6)
		{
			if (dup2(po[1], 6) == -1)
				return err_write();
			close(po[1]);
		}
		sig_pipedefault();
		execvp(*childargs, childargs);
		_exit(1);
	}
	close(pi[0]);
	close(po[1]);
	substdio_fdbuf(&ssup, safewrite, pi[1], upbuf, sizeof upbuf);
	if (substdio_put(&ssup, user.s, user.len) == -1)
		return err_write();
	if (substdio_put(&ssup, pass.s, pass.len) == -1)
		return err_write();
	if (substdio_put(&ssup, resp.s, resp.len) == -1)
		return err_write();
	strnum[0] = method;
	strnum[1] = 0;
	if (!stralloc_copyb(&authmethod, strnum, 2))
		die_nomem();
	if (substdio_put(&ssup, authmethod.s, authmethod.len) == -1)
		return err_write();
	if (substdio_flush(&ssup) == -1)
		return err_write();
	close(pi[1]);
	byte_zero(pass.s, pass.len);
	byte_zero(upbuf, sizeof upbuf);
	if (method == AUTH_DIGEST_MD5) {
		if ((n = saferead(po[0], respbuf, 33)) == -1)
			die_read();
		respbuf[n] = 0;
		close(po[0]);
	}
	if (wait_pid(&wstat, child) == -1)
		return err_child();
	if (wait_crashed(wstat))
		return err_child();
	if ((i = wait_exitcode(wstat)))
		return (i);
	if (method == AUTH_DIGEST_MD5) {
		if (!n)
			return (1);
		if (!stralloc_copys(&slop, "rspauth="))
			die_nomem();
		if (!stralloc_catb(&slop, respbuf, n))
			die_nomem();
		slop.s[slop.len] = 0;
		if (b64encode(&slop, &resp) < 0)
			die_nomem();
		resp.s[resp.len]=0;
		out("334 ");
		out(resp.s);
		out("\r\n");
		flush();
		/* digest-md5 requires a special okay response ... */
		if ((n = saferead(0, respbuf, 512)) == -1)
			die_read();
		if (n)
			respbuf[n] = 0;
		return (0);
	} else
		return (i);
}

int
auth_login(char *arg)
{
	int             r;

	if (*arg)
	{
		if ((r = b64decode((const unsigned char *) arg, str_len(arg), &user)) == 1)
			return err_input();
	} else
	{
		out("334 VXNlcm5hbWU6\r\n");
		flush(); /*- Username: */
		if (authgetl() < 0)
			return -1;
		if ((r = b64decode((const unsigned char *) authin.s, authin.len, &user)) == 1)
			return err_input();
	}
	if (r == -1)
		die_nomem();
	out("334 UGFzc3dvcmQ6\r\n");
	flush(); /*- Password: */
	if (authgetl() < 0)
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &pass)) == 1)
		return err_input();
	if (r == -1)
		die_nomem();
	if (!user.len || !pass.len)
		return err_input();
	r = authenticate(AUTH_LOGIN);
	if (!r || r == 3)
		authd = 1;
	return (r);
}

int
auth_plain(char *arg)
{
	int             r, id = 0;

	if (*arg)
	{
		if ((r = b64decode((const unsigned char *) arg, str_len(arg), &slop)) == 1)
			return err_input();
	} else
	{
		out("334 \r\n");
		flush();
		if (authgetl() < 0)
			return -1;
		if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
			return err_input();
	}
	if (r == -1 || !stralloc_0(&slop))
		die_nomem();
	while (slop.s[id])
		id++; /*- ignore authorize-id */
	if (slop.len > id + 1)
		if (!stralloc_copys(&user, slop.s + id + 1))
			die_nomem();
	if (slop.len > id + user.len + 2)
		if (!stralloc_copys(&pass, slop.s + id + user.len + 2))
			die_nomem();
	if (!user.len || !pass.len)
		return err_input();
	r = authenticate(AUTH_PLAIN);
	if (!r || r == 3)
		authd = 2;
	return (r);
}

int
auth_cram(int method)
{
	int             i, r;
	char           *s;
	char            unique[FMT_ULONG + FMT_ULONG + 3];

	s = unique;
	s += fmt_uint(s, getpid());
	*s++ = '.';
	s += fmt_ulong(s, (unsigned long) now());
	*s++ = '@';
	*s++ = 0;

	if (!stralloc_copys(&pass, "<")) /*- generate challenge */
		die_nomem();
	if (!stralloc_cats(&pass, unique))
		die_nomem();
	if (!stralloc_cats(&pass, hostname))
		die_nomem();
	if (!stralloc_cats(&pass, ">"))
		die_nomem();
	if (b64encode(&pass, &slop) < 0)
		die_nomem();
	if (!stralloc_0(&slop))
		die_nomem();

	out("334 ");	/*- "334 mychallenge \r\n" */
	out(slop.s);
	out("\r\n");
	flush();
	if (authgetl() < 0)	/*- got response */
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
		return err_input();
	if (r == -1 || !stralloc_0(&slop))
		die_nomem();
	i = str_chr(slop.s, ' ');
	s = slop.s + i;
	while (*s == ' ')
		++s;
	slop.s[i] = 0;
	if (!stralloc_copys(&user, slop.s))	/*- userid */
		die_nomem();
	if (!stralloc_copys(&resp, s)) /*- digest */
		die_nomem();
	if (!user.len || !resp.len)
		return err_input();
	r = authenticate(method);
	if (!r || r == 3)
		authd = method;
	return (r);
}

int
auth_cram_md5()
{
	return (auth_cram(AUTH_CRAM_MD5));
}

int
auth_cram_sha1()
{
	return (auth_cram(AUTH_CRAM_SHA1));
}

int
auth_cram_sha256()
{
	return (auth_cram(AUTH_CRAM_SHA256));
}

int
auth_cram_ripemd()
{
	return (auth_cram(AUTH_CRAM_RIPEMD));
}

/* parse digest response */
unsigned int
scan_response(stralloc *dst, stralloc *src, const char *search)
{
	char           *x = src->s;
	int             i, len;
	unsigned int    slen;

	slen = str_len((char *) search);
	if (!stralloc_copys(dst,""))
		die_nomem();
	for (i=0; src->len>i+slen; i+=str_chr(x+i, ',')+1) {
		char *s=x+i;
		if (case_diffb(s, slen, (char *) search) == 0) {
			s += slen; /* skip name */
			if (*s++ != '=')
				return 0; /* has to be here! */
			if (*s == '"') { /* var="value" */
				s++;
				len = str_chr(s, '"');
				if (!len)
					return 0;
				if (!stralloc_catb(dst, s, len))
					die_nomem();
			} else { /* var=value */
				len = str_chr(s, ',');
				if (!len)
					str_len(s); /* should be the end */
				if (!stralloc_catb(dst, s, len))
					die_nomem();
			}
			return dst->len;
		}
	}
	return 0;
}

/*
 * RFC 2831
 *
 * sets all 3 variables: user\0pass\0resp\0
 */
char            hextab[]="0123456789abcdef";

int
auth_digest_md5()
{
	unsigned char   unique[FMT_ULONG + FMT_ULONG + 3];
	unsigned char   digest[20], encrypted[41];
	unsigned char  *s, *x=encrypted;
	int             i, r, len = 0; /*- qop = 1; */
	stralloc        tmp = {0}, nonce = {0};

	s = unique;
	s += (i = fmt_uint((char *) s, getpid()));
	len += i;
	s += (i = fmt_str((char *) s, "."));
	len += i;
	s += (i = fmt_ulong((char *) s, (unsigned long) now()));
	len += i;
	s += (i = fmt_str((char *) s, "@"));
	len += i;
	*s++ = 0;
	hmac_sha1(unique, len, unique + 3, len - 3, digest); /* should be enough :) */
	for (i = 0; i < 20; i++) {
		*x = hextab[digest[i]/16]; ++x;
		*x = hextab[digest[i]%16]; ++x;
	}
	*x=0;
	if (!stralloc_copys(&tmp, (char *) encrypted))
		die_nomem();
	if (b64encode(&tmp, &nonce) != 0)
		die_nomem();
	if (!stralloc_cats(&slop, "realm=\""))
		die_nomem();
	if (!stralloc_cats(&slop, hostname))
		die_nomem();
	if (!stralloc_cats(&slop, "\",nonce=\""))
		die_nomem();
	if (!stralloc_cat(&slop, &nonce))
		die_nomem();
	if (!stralloc_cats(&slop, "\",qop=\"auth\""))
		die_nomem();
	if (!stralloc_cats(&slop, ",algorithm=md5-sess"))
		die_nomem();
	if (b64encode(&slop, &tmp) != 0)
		die_nomem();
	out("334 ");
	if (substdio_put(&ssout, tmp.s, tmp.len) == -1)
		return err_write();
	out("\r\n");
	flush();

	/* get digest-response */
	if (authgetl() < 0)
		return -1;
	if ((r = b64decode((const unsigned char *) authin.s, authin.len, &slop)) == 1)
		return err_input();
	if (!stralloc_0(&slop))
		die_nomem();

	/* scan slop for all required fields, fill resp for later auth.  */
	if (scan_response(&user, &slop, "username") == 0)
		return (err_input());
	if (scan_response(&tmp, &slop, "digest-uri") == 0)
		return (err_input());
	if (!stralloc_cats(&resp, "digest-uri="))
		die_nomem();
	if (!stralloc_cat(&resp, &tmp))
		die_nomem();

	/* check nc field */
	if (scan_response(&tmp, &slop, "nc") == 0)
		return (err_input());
	if (tmp.len != 8)
		return (err_input());
	if (case_diffb("00000001", 8, tmp.s) != 0)
		return (err_input());
	if (!stralloc_cats(&resp, "\nnc=")) die_nomem();
	if (!stralloc_cat(&resp, &tmp)) die_nomem();

	/* check nonce */
	if (scan_response(&tmp, &slop, "nonce") == 0)
		return (err_input());
	if (tmp.len != nonce.len)
		return (err_input());
	if (case_diffb(nonce.s, tmp.len, tmp.s) != 0)
		return (err_input());
	if (!stralloc_cats(&resp, "\nnonce=")) die_nomem();
	if (!stralloc_cat(&resp, &tmp)) die_nomem();

	/* check cnonce */
	if (scan_response(&tmp, &slop,  "cnonce") == 0)
		return (err_input());
	if (!stralloc_cats(&resp, "\ncnonce=")) die_nomem();
	if (!stralloc_cat(&resp, &tmp)) die_nomem();

	/* check qop */
	if (scan_response(&tmp, &slop, "qop") == 0)
		return (err_input());
	switch (tmp.len) {
		case 4: /*- qop=1; */
			if (case_diffb("auth", 4, tmp.s) != 0)
				return (err_input());
			break;
		case 8: /*- qop=2; */
			if (case_diffb("auth-int", 8, tmp.s) != 0)
				return (err_input());
			break;
		case 9: /*- qop=3; */
			if (case_diffb("auth-conf", 9, tmp.s) != 0)
				return (err_input());
			break;
		default:
			return (err_input());
	}
	if (!stralloc_cats(&resp, "\nqop="))
		die_nomem();
	if (!stralloc_cat(&resp, &tmp))
		die_nomem();

	/* xxx: todo / check realm against control/realms or so ?! */
	if (scan_response(&tmp, &slop, "realm") == 0)
		return (err_input());
	if (!stralloc_cats(&resp, "\nrealm="))
		die_nomem();
	if (!stralloc_cat(&resp, &tmp))
		die_nomem();

	/* check response */
	if (scan_response(&pass, &slop, "response") == 0)
		return (err_input());
	if (pass.len != 32)
		return (err_input());

	/* user=username
	 * pass=response (md5 hash = 32)
	 * resp=authfile (with all required vars for the checkpassword utility)
	 *      -> nc,qop,realm,nonce,cnonce,digesturi
	 *
	 *             a1 = md5(user:realm:pass) : nonce : cnonce
	 * qop=auth:   a2 = 'AUTHENTICATE' : digesturi
	 * qop=auth-*: a2 = 'AUTHENTICATE' : digesturi : '00000000000000000000000000000000'
	 * resp = md5(a1) + nonce + nc + conce + qop + md5(a2)
	 */

	if (!user.len || !pass.len || !resp.len)
		return (err_input());
	r = authenticate(AUTH_DIGEST_MD5);
	if (!r || r == 3)
		authd = 6;
	return (r);
}

void
smtp_auth(char *arg)
{
	int             i;
	char           *cmd = arg;

	switch (setup_state)
	{
	case 0:
		break;
	case 1:
		out("503 bad sequence of commands (#5.3.2)\r\n");
		return;
	case 2:
		smtp_relayreject(remoteip);
		return;
	case 3:
		smtp_paranoid(remoteip);
		return;
	case 4:
		smtp_ptr(remoteip);
		return;
	case 5:
		smtp_badhost(remoteip);
		return;
	case 6:
		smtp_badip(remoteip);
		return;
	}
	if (!hostname || !*hostname || !childargs || !*childargs)
	{
		out("503 auth not available (#5.3.3)\r\n");
		return;
	}
	if (authd)
	{
		err_authd();
		return;
	}

	/* forcetls patch */
#ifdef TLS
	if (env_get("FORCE_TLS"))
	{
		if (!ssl)
		{
			out("530 must issue STARTTLS first (#5.7.0)\r\n");
			return;
		}
	}
#endif
	if (seenmail)
	{
		err_transaction("auth");
		return;
	}
	if (!stralloc_copys(&user, ""))
		die_nomem();
	if (!stralloc_copys(&pass, ""))
		die_nomem();
	if (!stralloc_copys(&resp, ""))
		die_nomem();
	i = str_chr(cmd, ' ');
	arg = cmd + i;
	while (*arg == ' ')
		++arg;
	cmd[i] = 0;
	for (i = 0; authcmds[i].text; ++i)
	{
		if (case_equals(authcmds[i].text, cmd))
			break;
	}
	switch (authcmds[i].fun(arg))
	{
	case 0:
		relayclient = "";
	case 3: /*- relayclient is not set, relaying is denied */
		remoteinfo = user.s;
		if (!env_put2("TCPREMOTEINFO", remoteinfo))
			die_nomem();
		out("235 ok, go ahead (#2.0.0)\r\n");
		break;
	case 1: /*- auth fail */
	case 2: /*- misuse */
		sleep(5);
		out("535 authorization failed (#5.7.1)\r\n");
		break;
	case -1:
		out("454 temporary authentication failure (#4.3.0)\r\n");
		break;
	default:
		err_child();
	}
	return;
}

void
smtp_etrn(char *arg)
{
	int             status, i;
	char            tmpbuf[1024], err_buff[1024], status_buf[FMT_ULONG]; /*- needed for SIZE CMD */

	if (!*arg)
	{
		err_syntax();
		return;
	}
	if (!seenhelo)
	{
		out("503 Polite people say hello first (#5.5.4)\r\n");
		return;
	}
	if (seenmail)
	{
		err_transaction("ETRN");
		return;
	}
	if (!isalnum((int) *arg))
		arg++;
	if (!valid_hostname(arg))
	{
		out("501 invalid parameter syntax (#5.3.2)\r\n");
		return;
	}
	if (!nodnscheck)
	{
		i = fmt_str(tmpbuf, "@");
		i += fmt_strn(tmpbuf + i, arg, 1022);
		if (i > 1023)
			die_nomem();
		tmpbuf[i] = 0;
		switch (dnscheck(tmpbuf, i, 1))
		{
		case DNS_HARD:
			err_hmf(remoteip, tmpbuf, 1);
			return;
		case DNS_SOFT:
			err_smf();
			return;
		case DNS_MEM:
			die_nomem();
		}
	}
	/*
	 * XXX The implementation borrows heavily from the code that implements
	 * UCE restrictions. These typically return 450 or 550 when a request is
	 * rejected. RFC 1985 requires that 459 be sent when the server refuses
	 * to perform the request.
	 */
	switch ((status = etrn_queue(arg, remoteip)))
	{
	case 0:
		log_etrn(remoteip, arg, 0);
		out("250 OK, queueing for node <");
		out(arg);
		out("> started\r\n");
		return;
	case -1:
		log_etrn(remoteip, arg, "ETRN Error");
		out("451 Unable to queue messages (#4.3.0)\r\n");
		return;
	case -2:
		log_etrn(remoteip, arg, "ETRN Rejected");
		out("553 <");
		out(arg);
		out(">: etrn service unavailable (#5.7.1)\r\n");
		return;
	case -3:
		out("250 OK, No message waiting for node <");
		out(arg);
		out(">\r\n");
		return;
	case -4:
		out("252 OK, pending message for node <");
		out(arg);
		out("> started\r\n");
		return;
	default:
		status_buf[fmt_ulong(status_buf, (unsigned long) status)] = 0;
		if (status > 0)
		{
			out("253 OK, <");
			out(status_buf);
			out("> pending message for node <");
			out(arg);
			out("> started\r\n");
			return;
		}
		i = fmt_str(err_buff, "unable to talk to fast flush service status <");
		i += fmt_ulong(err_buff + i, (unsigned long) status);
		if (i > 1023)
			die_nomem();
		i += fmt_str(err_buff + i, ">");
		if (i > 1023)
			die_nomem();
		err_buff[i] = 0;
		log_etrn(remoteip, arg, err_buff);
		out("451 Unable to queue messages, status <");
		out(status_buf);
		out("> (#4.3.0)\r\n");
		return;
	}
	return;
}

#ifdef INDIMAIL
void
smtp_atrn(char *arg)
{
	char           *ptr, *cptr, *domain_ptr, *user_tmp, *domain_tmp;
	int             i, end_flag, status, Reject = 0, Accept = 0;
	char            err_buff[1024], status_buf[FMT_ULONG]; /*- needed for SIZE CMD */
	char            user[MAX_BUFF], domain[MAX_BUFF];
	stralloc        domBuf = { 0 };

	if (!authd)
	{
		err_authrequired();
		return;
	}
	if (!seenhelo)
	{
		out("503 Polite people say hello first (#5.5.4)\r\n");
		return;
	}
	if (seenmail)
	{
		err_transaction("ATRN");
		return;
	}
	for (; *arg && !isalnum((int) *arg); arg++);
	if (*arg)
		domain_ptr = arg;
	else
	{
		parse_email(remoteinfo, user, domain, MAX_BUFF);
		for (user_tmp = user, domain_tmp = domain, end_flag = 0;;)
		{
			if (!(ptr = vshow_atrn_map(&user_tmp, &domain_tmp)))
				break;
			if (end_flag)
			{
				if (!stralloc_cats(&domBuf, " "))
				{
					vclose();
					die_nomem();
				}
			}
			if (!stralloc_cats(&domBuf, ptr))
			{
				vclose();
				die_nomem();
			}
			end_flag = 1;
		}
		if (!stralloc_0(&domBuf))
		{
			vclose();
			die_nomem();
		}
		domain_ptr = domBuf.s;
	}
	for (cptr = domain_ptr;; cptr++)
	{
		if (*cptr == ' ' || *cptr == ',' || !*cptr)
		{
			if (*cptr)
			{
				end_flag = 0;
				*cptr = 0;
			} else
				end_flag = 1;
			if (!valid_hostname(arg))
			{
				out("501 invalid parameter syntax (#5.3.2)\r\n");
				return;
			}
			if (atrn_access(remoteinfo, domain_ptr))
			{
				Reject = 1;
				break;
			} else
				Accept = 1;
			if (end_flag)
				break;
			else
				*cptr = ' ';
			domain_ptr = cptr + 1;
		}
	}
	vclose();
	if (Reject)
	{
		log_atrn(remoteip, remoteinfo, domain_ptr, "ATRN Rejected");
		if (Accept)
			out("450 atrn service unavailable (#5.7.1)\r\n");
		else
			out("553 atrn service unavailable (#5.7.1)\r\n");
		return;
	}
	switch ((status = atrn_queue(arg, remoteip)))
	{
	case 0:
		log_atrn(remoteip, remoteinfo, arg, 0);
		out("QUIT\r\n");
		flush();
		_exit(0);
	case -1:
		log_atrn(remoteip, remoteinfo, arg, "ATRN Error");
		out("451 Unable to queue messages (#4.3.0)\r\n");
		return;
	case -2:
		log_atrn(remoteip, remoteinfo, arg, "ATRN Rejected");
		out("553 <");
		out(arg);
		out(">: atrn service unavailable (#5.7.1)\r\n");
		return;
	case -3:
		out("453 No message waiting for node(s) <");
		out(arg);
		out(">\r\n");
		return;
	case -4:
		out("451 Unable to queue messages (#4.3.0)\r\n");
		return;
	default:
		status_buf[fmt_ulong(status_buf, (unsigned long) status)] = 0;
		if (status > 0)
		{
			i = fmt_str(err_buff, "unable to talk to fast flush service status <");
			i += fmt_ulong(err_buff + i, (unsigned long) status);
			if (i > 1023)
				die_nomem();
			i += fmt_str(err_buff + i, ">");
			if (i > 1023)
				die_nomem();
			err_buff[i] = 0;
			log_atrn(remoteip, remoteinfo, arg, err_buff);
			out("451 Unable to queue messages, status <");
			out(status_buf);
			out("> (#4.3.0)\r\n");
		}
		return;
	}
	return;
}
#endif

#ifdef TLS
int             ssl_verified = 0;
const char     *ssl_verify_err = 0;

void
smtp_tls(char *arg)
{
	if (ssl)
		err_unimpl("unimplimented");
	else
	if (*arg)
		out("501 Syntax error (no parameters allowed) (#5.5.4)\r\n");
	else
		tls_init();
}

RSA            *
tmp_rsa_cb(SSL *ssl, int export, int keylen)
{
	stralloc        filename = {0};

	if (!export)
		keylen = 512;
	if (keylen == 512)
	{
		FILE           *in;

		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
		}
		if (!stralloc_copys(&filename, controldir))
			die_nomem();
		if (!stralloc_catb(&filename, "/rsa512.pem", 11))
			die_nomem();
		if (!stralloc_0(&filename))
			die_nomem();
		if ((in = fopen(filename.s, "r")))
		{
			RSA            *rsa = PEM_read_RSAPrivateKey(in, NULL, NULL, NULL);
			fclose(in);
			if (rsa)
			{
				alloc_free(filename.s);
				return rsa;
			}
		}
		alloc_free(filename.s);
	}
	return RSA_generate_key(keylen, RSA_F4, NULL, NULL);
}

DH             *
tmp_dh_cb(SSL *ssl, int export, int keylen)
{
	stralloc        filename = {0};

	if (!export)
		keylen = 1024;
	if (keylen == 512)
	{
		FILE           *in;
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
		}
		if (!stralloc_copys(&filename, controldir))
			die_nomem();
		if (!stralloc_catb(&filename, "/dh512.pem", 10))
			die_nomem();
		if (!stralloc_0(&filename))
			die_nomem();
		if ((in = fopen(filename.s, "r")))
		{
			DH             *dh = PEM_read_DHparams(in, NULL, NULL, NULL);
			fclose(in);
			if (dh)
			{
				alloc_free(filename.s);
				return dh;
			}
		}
	}
	if (keylen == 1024)
	{
		FILE           *in;
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = "control";
		}
		if (!stralloc_copys(&filename, controldir))
			die_nomem();
		if (!stralloc_catb(&filename, "/dh1024.pem", 11))
			die_nomem();
		if (!stralloc_0(&filename))
			die_nomem();
		if ((in = fopen(filename.s, "r")))
		{
			DH             *dh = PEM_read_DHparams(in, NULL, NULL, NULL);
			fclose(in);
			if (dh)
			{
				alloc_free(filename.s);
				return dh;
			}
		}
	}
	alloc_free(filename.s);
	return DH_generate_parameters(keylen, DH_GENERATOR_2, NULL, NULL);
}

/*
 * don't want to fail handshake if cert isn't verifiable 
 */
int
verify_cb(int preverify_ok, X509_STORE_CTX * ctx)
{
	return 1;
}

void
tls_nogateway()
{
	/*- there may be cases when relayclient is set */
	if (!ssl || relayclient)
		return;
	out("; no valid cert for gatewaying");
	if (ssl_verify_err)
	{
		out(": ");
		out((char *) ssl_verify_err);
	}
	out(" ");
}

void
tls_out(const char *s1, const char *s2)
{
	out("454 TLS ");
	out((char *) s1);
	if (s2)
	{
		out(": ");
		out((char *) s2);
	}
	out(" (#4.3.0)\r\n");
	flush();
}

void
tls_err(const char *s)
{
	tls_out(s, ssl_error());
	if (smtps)
		die_read();
}

int
tls_verify()
{
	stralloc        clients = { 0 }, filename = { 0 };
	struct constmap mapclients;

	if (!ssl || relayclient || ssl_verified)
		return 0;
	ssl_verified = 1;	/*- don't do this twice */
	/*
	 * request client cert to see if it can be verified by one of our CAs
	 * and the associated email address matches an entry in tlsclients 
	 */
	switch (control_readfile(&clients, "tlsclients", 0))
	{
	case 1:
		if (constmap_init(&mapclients, clients.s, clients.len, 0))
		{
			/*
			 * if clientca.pem contains all the standard root certificates, a
			 * 0.9.6b client might fail with SSL_R_EXCESSIVE_MESSAGE_SIZE;
			 * it is probably due to 0.9.6b supporting only 8k key exchange
			 * data while the 0.9.6c release increases that limit to 100k 
			 */
			if (!controldir)
			{
				if (!(controldir = env_get("CONTROLDIR")))
					controldir = "control";
			}
			if (!stralloc_copys(&filename, controldir))
				die_nomem();
			if (!stralloc_catb(&filename, "/", 1))
				die_nomem();
			clientca = ((clientca = env_get("CLIENTCA")) ? clientca : "clientca.pem");
			if (!stralloc_cats(&filename, clientca))
				die_nomem();
			if (!stralloc_0(&filename))
				die_nomem();
			STACK_OF(X509_NAME) * sk = SSL_load_client_CA_file(filename.s);
			alloc_free(filename.s);
			if (sk)
			{
				SSL_set_client_CA_list(ssl, sk);
				SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, NULL);
				break;
			}
			constmap_free(&mapclients);
		}
	case 0:
		alloc_free(clients.s);
		return 0;
	case -1:
		die_control();
	}

	if (ssl_timeoutrehandshake(timeout, ssl_rfd, ssl_wfd, ssl) <= 0)
	{
		const char     *err = ssl_error_str();
		tls_out("rehandshake failed", err);
		die_read();
	}

	do /*- one iteration */
	{
		X509           *peercert;
		X509_NAME      *subj;
		stralloc        email = { 0 };

		int             n = SSL_get_verify_result(ssl);
		if (n != X509_V_OK)
		{
			ssl_verify_err = X509_verify_cert_error_string(n);
			break;
		}
		if (!(peercert = SSL_get_peer_certificate(ssl)))
			break;
		subj = X509_get_subject_name(peercert);
		if ((n = X509_NAME_get_index_by_NID(subj, NID_pkcs9_emailAddress, -1)) >= 0)
		{
			const ASN1_STRING *s = X509_NAME_get_entry(subj, n)->value;
			if (s)
			{
				email.len = s->length;
				email.s = (char *) s->data;
			}
		}
		if (email.len <= 0)
			ssl_verify_err = "contains no email address";
		else
		if (!constmap(&mapclients, email.s, email.len))
			ssl_verify_err = "email address not in my list of tlsclients";
		else
		{
			/*
			 * add the cert email to the proto if it helped allow relaying 
			 */
			if (!stralloc_cats(&proto, "\n  (cert ")	/*- continuation line */
				|| !stralloc_catb(&proto, email.s, email.len)
				|| !stralloc_cats(&proto, ")"))
				die_nomem();
			authenticated = 1;
			relayclient = "";
		}
		X509_free(peercert);
	} while (0);
	constmap_free(&mapclients);
	alloc_free(clients.s);
	/*
	 * we are not going to need this anymore: free the memory 
	 */
	SSL_set_client_CA_list(ssl, NULL);
	SSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);
	return relayclient ? 1 : 0;
}

void
tls_init()
{
	SSL            *myssl;
	SSL_CTX        *ctx;
	const char     *ciphers;
	stralloc        saciphers = { 0 };
	X509_STORE     *store;
	X509_LOOKUP    *lookup;
	stralloc        filename = {0};

	SSL_library_init();
	/*
	 * a new SSL context with the bare minimum of options 
	 */
	if (!(ctx = SSL_CTX_new(SSLv23_server_method())))
	{
		tls_err("unable to initialize ctx");
		return;
	}
	if (!controldir)
	{
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = "control";
	}
	if (!stralloc_copys(&filename, controldir))
		die_nomem();
	if (!stralloc_catb(&filename, "/", 1))
		die_nomem();
	servercert = ((servercert = env_get("SERVERCERT")) ? servercert : "servercert.pem");
	if (!stralloc_cats(&filename, servercert))
		die_nomem();
	if (!stralloc_0(&filename))
		die_nomem();
	if (!SSL_CTX_use_certificate_chain_file(ctx, filename.s))
	{
		SSL_CTX_free(ctx);
		tls_err("missing certificate");
		return;
	}
	if (!stralloc_copys(&filename, controldir))
		die_nomem();
	if (!stralloc_catb(&filename, "/", 1))
		die_nomem();
	clientca = ((clientca = env_get("CLIENTCA")) ? clientca : "clientca.pem");
	if (!stralloc_cats(&filename, clientca))
		die_nomem();
	if (!stralloc_0(&filename))
		die_nomem();
	SSL_CTX_load_verify_locations(ctx, filename.s, NULL);
#if OPENSSL_VERSION_NUMBER >= 0x00907000L
	/*
	 * crl checking 
	 */
	store = SSL_CTX_get_cert_store(ctx);
	if (!stralloc_copys(&filename, controldir))
		die_nomem();
	if (!stralloc_catb(&filename, "/", 1))
		die_nomem();
	clientcrl = ((clientcrl = env_get("CLIENTCRL")) ? clientcrl : "clientcrl.pem");
	if (!stralloc_cats(&filename, clientcrl))
		die_nomem();
	if (!stralloc_0(&filename))
		die_nomem();
	if ((lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file())) &&
		(X509_load_crl_file(lookup, filename.s, X509_FILETYPE_PEM) == 1))
		X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#endif
	/*
	 * set the callback here; SSL_set_verify didn't work before 0.9.6c 
	 */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, verify_cb);
	/*
	 * a new SSL object, with the rest added to it directly to avoid copying 
	 */
	myssl = SSL_new(ctx);
	SSL_CTX_free(ctx);
	if (!myssl)
	{
		tls_err("unable to initialize ssl");
		return;
	}
	/*
	 * this will also check whether public and private keys match 
	 */
	if (!stralloc_copys(&filename, controldir))
		die_nomem();
	if (!stralloc_catb(&filename, "/", 1))
		die_nomem();
	servercert = ((servercert = env_get("SERVERCERT")) ? servercert : "servercert.pem");
	if (!stralloc_cats(&filename, servercert))
		die_nomem();
	if (!stralloc_0(&filename))
		die_nomem();
	if (!SSL_use_RSAPrivateKey_file(myssl, filename.s, SSL_FILETYPE_PEM))
	{
		SSL_free(myssl);
		alloc_free(filename.s);
		tls_err("no valid RSA private key");
		return;
	}
	alloc_free(filename.s);
	if (!(ciphers = env_get("TLSCIPHERS")))
	{
		if (control_readfile(&saciphers, "tlsserverciphers", 0) == -1)
		{
			SSL_free(myssl);
			die_control();
		}
		if (saciphers.len)
		{
			int             i;
			/*- convert all '\0's except the last one to ':' */
			for (i = 0; i < saciphers.len - 1; ++i)
				if (!saciphers.s[i])
					saciphers.s[i] = ':';
			ciphers = saciphers.s;
		}
	}
	if (!ciphers || !*ciphers)
		ciphers = "DEFAULT";
	SSL_set_cipher_list(myssl, ciphers);
	alloc_free(saciphers.s);
	SSL_set_tmp_rsa_callback(myssl, tmp_rsa_cb);
	SSL_set_tmp_dh_callback(myssl, tmp_dh_cb);
	SSL_set_rfd(myssl, ssl_rfd = substdio_fileno(&ssin));
	SSL_set_wfd(myssl, ssl_wfd = substdio_fileno(&ssout));
	if (!smtps)
	{
		out("220 ready for tls\r\n");
		flush();
	}
	if (ssl_timeoutaccept(timeout, ssl_rfd, ssl_wfd, myssl) <= 0)
	{
		/*
		 * neither cleartext nor any other response here is part of a standard 
		 */
		const char     *err = ssl_error_str();
		ssl_free(myssl);
		tls_out("connection failed", err);
		die_read();
	}
	ssl = myssl;
	/*
	 * populate the protocol string, used in Received 
	 */
	if (!stralloc_cats(&proto, "(") || !stralloc_cats(&proto, (char *) SSL_get_cipher(ssl)))
		die_nomem();
	if (!stralloc_cats(&proto, "encrypted) "))
		die_nomem();
	/*
	 * have to discard the pre-STARTTLS HELO/EHLO argument, if any 
	 */
	dohelo(remotehost);
}
#endif

struct commands smtpcommands[] = {
	{"rcpt", smtp_rcpt, 0},
	{"mail", smtp_mail, 0},
	{"data", smtp_data, flush},
	{"auth", smtp_auth, flush},
	{"quit", smtp_quit, flush},
	{"helo", smtp_helo, flush},
	{"ehlo", smtp_ehlo, flush},
	{"rset", smtp_rset, 0},
	{"help", smtp_help, flush},
	{"noop", smtp_noop, flush},
	{"vrfy", smtp_vrfy, flush},
	{"etrn", smtp_etrn, flush},
#ifdef INDIMAIL
	{"atrn", smtp_atrn, flush},
#endif
#ifdef TLS
	{"starttls", smtp_tls, flush},
#endif
	{0, err_unimpl, flush}
};

struct commands odmrcommands[] = {
	{"auth", smtp_auth, flush},
	{"quit", smtp_quit, flush},
	{"helo", smtp_helo, flush},
	{"ehlo", smtp_ehlo, flush},
	{"help", smtp_help, flush},
	{"etrn", smtp_etrn, flush},
#ifdef INDIMAIL
	{"atrn", smtp_atrn, flush},
#endif
	{0, err_unimpl, flush}
};

struct commands submcommands[] = {
	{"rcpt", smtp_rcpt, 0},
	{"mail", smtp_mail, 0},
	{"data", smtp_data, flush},
	{"auth", smtp_auth, flush},
	{"quit", smtp_quit, flush},
	{"helo", smtp_helo, flush},
	{"ehlo", smtp_ehlo, flush},
	{"rset", smtp_rset, 0},
	{"help", smtp_help, flush},
	{"noop", smtp_noop, flush},
	{"vrfy", smtp_vrfy, flush},
#ifdef TLS
	{"starttls", smtp_tls, flush},
#endif
	{0, err_unimpl, flush}
};

#ifdef SMTP_PLUGIN
void
load_plugin(char *library, char *plugin_symb, int j)
{
	PLUGIN         *(*func) (void);
	char           *error;

	if (!(handle[j] = dlopen(library, RTLD_NOW|RTLD_GLOBAL)))
		die_plugin("dlopen failed for ", library, ": ", dlerror());
	dlerror(); /*- man page told me to do this */
	*(void **) (&func) = dlsym(handle[j], plugin_symb);
	if ((error = dlerror()))
		die_plugin("dlsym ", plugin_symb, " failed: ", error);
	/*- execute the function */
	if (!(plug[j] = (*func) ())) /*- this function returns a pointer to PLUGIN */
		die_plugin("function ", plugin_symb, " failed", 0);
	return;
}
#endif

void
qmail_smtpd(int argc, char **argv, char **envp)
{
	char           *ptr;
	struct commands *cmdptr;
#ifdef SMTP_PLUGIN
	int             i, j, len;
	char           *start_plugin, *plugin_symb, *plugindir;
	stralloc        plugin = { 0 };
#endif

	if (argc > 2)
	{
		hostname = argv[1];
		childargs = argv + 2;
	}
	setup_state = 0;
	if ((ptr = env_get("TCPLOCALPORT")))
		scan_int(ptr, &smtp_port);
	else
		smtp_port = -1;
	if (smtp_port == ODMR_PORT && (!hostname || !*hostname || !childargs || !*childargs))
	{
		if (!env_put("SHUTDOWN=1"))
			die_nomem();
	}
	if (envp)
		environ = envp;
	sig_termcatch(sigterm);
	sig_pipeignore();
	if (chdir(auto_qmail) == -1)
		die_control();
	/*
	 * setup calls databytes_setup(), which sets
	 * databytes
	 */
	setup(); /*- remoteip is set */
	if (ipme_init() != 1)
		die_ipme();
	if (greetdelay)
		greetdelay_check(greetdelay);
	if (env_get("SHUTDOWN"))
	{
		smtp_greet("554 ");
		setup_state = 1;
	} else
	{
		if (dobadipcheck && badipcheck(remoteip))
		{
			smtp_greet("421 ");
			out(" sorry, your IP (");
			out(remoteip);
			out(") is temporarily denied (#4.7.1)");
			setup_state = 6;
		} else
		if (dobadhostcheck && badhostcheck())
		{
			smtp_greet("553 ");
			out(" sorry, your host (");
			out(remotehost);
			out(") has been denied (#5.7.1)");
			setup_state = 5;
		} else
		if (env_get("OPENRELAY"))
		{
			smtp_greet("553 ");
			out(" No mail accepted from an open relay (");
			out(remoteip);
			out("); check your server configs (#5.7.1)");
			setup_state = 2;
		} else
		if ((ptr = env_get("TCPPARANOID")))
		{
			smtp_greet("553 ");
			out(" sorry, your IP address (");
			out(remoteip);
			if (*ptr)
			{
				out(") PTR (reverse DNS) record points to wrong hostname ");
				out(ptr);
				out(" (#5.7.1)");
			} else
				out(") PTR (reverse DNS) record points to wrong hostname (#5.7.1)");
			setup_state = 3;
		} else
		if ((ptr = env_get("REQPTR")) && str_equal(remotehost, "unknown"))
		{
			smtp_greet("553 ");
			if (*ptr)
			{
				out(" ");
				out(ptr);
				out(": from ");
				out(remoteip);
				out(": (#5.7.1)");
			} else
			{
				out(" Sorry, no PTR (reverse DNS) record for (");
				out(remoteip);
				out(") (#5.7.1)");
			}
			setup_state = 4;
		} else
			smtp_greet("220 ");
	}
	out("\r\n");
	switch (smtp_port)
	{
	case ODMR_PORT: /*- RFC 2645 */
		cmdptr = odmrcommands;
		break;
	case SUBM_PORT: /*- RFC 2476 */
		cmdptr = submcommands;
		break;
	case SMTP_PORT:
	default:
		cmdptr = smtpcommands;
		break;
	}
#ifdef SMTP_PLUGIN
	if (env_get("DISABLE_PLUGIN"))
		goto command;
	if (!(plugindir = env_get("PLUGINDIR")))
		plugindir = "plugins";
	if (plugindir[i = str_chr(plugindir, '/')])
		die_plugin(plugindir, "plugindir cannot have an absolute path", 0, 0);
	if (!(plugin_symb = env_get("SMTP_PLUGIN_SYMB")))
		plugin_symb = "plugin_init";
	if (!stralloc_copys(&plugin, auto_qmail))
		die_nomem();
	if (!stralloc_append(&plugin, "/"))
		die_nomem();
	if (!stralloc_cats(&plugin, plugindir))
		die_nomem();
	if (!stralloc_append(&plugin, "/"))
		die_nomem();
	if (!(start_plugin = env_get("SMTP_PLUGIN")))
		start_plugin = "smtpd-plugin.so";
	if (!stralloc_cats(&plugin, start_plugin))
		die_nomem();
	if (!stralloc_0(&plugin))
		die_nomem();
	len = plugin.len;
	/*- figure out plugin count */
	for (i = plugin_count = 0;;plugin_count++) {
		if (!plugin_count) {
			if (access(plugin.s, R_OK)) {
				plugin.len -= 4;
				strnum[fmt_ulong(strnum, i++)] = 0;
				if (!stralloc_catb(&plugin, strnum, 1))
					die_nomem();
				if (!stralloc_cats(&plugin, ".so"))
					die_nomem();
				if (!stralloc_0(&plugin))
					die_nomem();
				if (access(plugin.s, R_OK))
					goto command;
			}
		} else {
			plugin.len = len - 4;
			strnum[fmt_ulong(strnum, i++)] = 0;
			if (!stralloc_catb(&plugin, strnum, 1))
				die_nomem();
			if (!stralloc_cats(&plugin, ".so"))
				die_nomem();
			if (!stralloc_0(&plugin))
				die_nomem();
			if (access(plugin.s, R_OK))
				break;
		}
	}
	if (!(handle = (void **) alloc(sizeof(void *) * plugin_count)))
		die_nomem();
	if (!(plug = (PLUGIN **) alloc(sizeof(PLUGIN *) * plugin_count)))
		die_nomem();
	plugin.len = len - 4;
	if (!stralloc_cats(&plugin, ".so"))
		die_nomem();
	if (!stralloc_0(&plugin))
		die_nomem();
	for (i = j = 0;i < plugin_count;) {
		if (!j) {
			if (access(plugin.s, R_OK)) { /*- smtpd-plugin.so */
				plugin.len -= 4;
				strnum[fmt_ulong(strnum, i)] = 0;
				if (!stralloc_catb(&plugin, strnum, 1))
					die_nomem();
				if (!stralloc_cats(&plugin, ".so"))
					die_nomem();
				if (!stralloc_0(&plugin))
					die_nomem();
				if (access(plugin.s, R_OK)) /*- smtpd-plugin0.so */
					goto command;
				load_plugin(plugin.s, plugin_symb, j++);
				i++;
			} else
				load_plugin(plugin.s, plugin_symb, j++);
		} else { /*- smtpd-plugin1.so, smtpd-plugin2.so, ... */
			plugin.len = len - 4;
			strnum[fmt_ulong(strnum, i)] = 0;
			if (!stralloc_catb(&plugin, strnum, 1))
				die_nomem();
			if (!stralloc_cats(&plugin, ".so"))
				die_nomem();
			if (!stralloc_0(&plugin))
				die_nomem();
			if (access(plugin.s, R_OK))
				break;
			load_plugin(plugin.s, plugin_symb, j++);
			i++;
		}
	}
command:
#endif
	if (commands(&ssin, cmdptr) == 0)
		die_read();
	die_nomem();
}

int
addrrelay() /*- Rejection of relay probes. */
{
	int             j;

	j = addr.len;
	while (--j >= 0)
		if (addr.s[j] == '@')
			break;
	if (j < 0)
		j = addr.len;
	while (--j >= 0)
	{
		if (addr.s[j] == '@')
			return 1;
		if (addr.s[j] == '%')
			return 1;
		if (addr.s[j] == '!')
			return 1;
	}
	return 0;
}

void
getversion_smtpd_c()
{
	static char    *x = "$Id: smtpd.c,v 1.173 2013-08-06 11:15:25+05:30 Cprogrammer Exp mbhangui $";

#ifdef INDIMAIL
	if (x)
		x = sccsidh;
#else
	if (x)
		x++;
	x=sccsidauthcramh;
#endif
}
