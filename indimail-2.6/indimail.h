/*
 * $Log: indimail.h,v $
 * Revision 2.230  2019-03-16 19:26:56+05:30  Cprogrammer
 * removed mailing list code
 *
 * Revision 2.229  2018-10-30 19:03:34+05:30  Cprogrammer
 * added proto for int_mysql_options()
 *
 * Revision 2.228  2018-03-30 09:31:40+05:30  Cprogrammer
 * added socket member to dbinfo structure
 *
 * Revision 2.227  2018-03-27 12:06:44+05:30  Cprogrammer
 * added use_ssl field to dbinfo table, structure
 *
 * Revision 2.226  2018-03-21 11:13:12+05:30  Cprogrammer
 * added error_mysql_options_str() function to display the exact mysql_option() error
 *
 * Revision 2.225  2018-02-05 12:26:05+05:30  Cprogrammer
 * changed datatype of timestamp column in hostcntrl, lastauth, userquota to TIMESTAMP from DATETIME
 *
 * Revision 2.224  2017-12-20 13:44:03+05:30  Cprogrammer
 * added timestamp column
 *
 * Revision 2.223  2017-10-21 15:20:37+05:30  Cprogrammer
 * reduced index length
 *
 * Revision 2.222  2017-08-09 23:23:04+05:30  Cprogrammer
 * increased SQL_BUF_SIZE
 *
 * Revision 2.221  2017-05-01 20:20:51+05:30  Cprogrammer
 * removed mailing list feature from vfilter
 *
 * Revision 2.220  2017-04-28 10:19:41+05:30  Cprogrammer
 * updated timestamp columns default to CURRENT_TIMESTAMP
 *
 * Revision 2.219  2016-06-09 15:26:55+05:30  Cprogrammer
 * added function check_group()
 *
 * Revision 2.218  2016-05-18 12:43:49+05:30  Cprogrammer
 * added dir argument to del_user_assign()
 *
 * Revision 2.217  2016-01-28 00:03:59+05:30  Cprogrammer
 * maildirquota specification for -q option to vadduser
 *
 * Revision 2.216  2015-08-26 11:07:24+05:30  Cprogrammer
 * added isPrime() function
 *
 * Revision 2.215  2015-08-21 10:32:21+05:30  Cprogrammer
 * added getEnvConfigLong()
 *
 * Revision 2.214  2014-07-27 12:30:03+05:30  Cprogrammer
 * added timestamp column to indimail tables
 *
 * Revision 2.213  2014-07-03 00:04:05+05:30  Cprogrammer
 * added function valias_track()
 *
 * Revision 2.212  2014-04-17 11:39:15+05:30  Cprogrammer
 * added prototype for grpscan(), setuserid(), setuser_privileges()
 *
 * Revision 2.211  2014-01-02 22:29:52+05:30  Cprogrammer
 * added variable delayed_insert for mysql
 *
 * Revision 2.210  2013-09-04 12:14:12+05:30  Cprogrammer
 * added definition for AUTH_CRAM_SHA512
 *
 * Revision 2.209  2013-06-10 15:44:07+05:30  Cprogrammer
 * changed defaultquota to signed BIGINT
 *
 * Revision 2.208  2012-09-24 19:15:14+05:30  Cprogrammer
 * made diskquota, maxmsgcount, defaultquota, defaultmaxmsgcount in vlimits unsigned
 *
 * Revision 2.207  2012-06-12 15:39:10+05:30  Cprogrammer
 * increased max password length definition to 128
 *
 * Revision 2.206  2012-04-22 13:57:07+05:30  Cprogrammer
 * use 64bit integer for quota
 *
 * Revision 2.205  2011-12-22 08:39:21+05:30  Cprogrammer
 * added definitions for AUTH methods
 *
 * Revision 2.204  2011-12-10 14:54:57+05:30  Cprogrammer
 * added hmac_sha256()
 *
 * Revision 2.203  2011-10-28 17:50:12+05:30  Cprogrammer
 * added digest_md5() function
 *
 * Revision 2.202  2011-10-28 14:15:27+05:30  Cprogrammer
 * added auth_method argument to pw_comp
 *
 * Revision 2.201  2011-10-27 14:31:28+05:30  Cprogrammer
 * added hmac_sha1(), hmac_ripemd()
 *
 * Revision 2.200  2011-10-25 20:17:02+05:30  Cprogrammer
 * added status argument to mgmtgetpass()
 *
 * Revision 2.199  2011-04-08 17:26:18+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.198  2011-04-03 16:22:26+05:30  Cprogrammer
 * added instance count arg to ProcessInFifo()
 *
 * Revision 2.197  2011-02-11 23:01:15+05:30  Cprogrammer
 * fix for specifying > 2GB values in quota and message counts
 *
 * Revision 2.196  2010-08-15 15:51:33+05:30  Cprogrammer
 * added users_per_level argument to print_control()
 *
 * Revision 2.195  2010-08-08 20:16:55+05:30  Cprogrammer
 * use configurable users per level
 *
 * Revision 2.194  2010-06-19 20:40:25+05:30  Cprogrammer
 * fix for syntax error during create_table on some MySQL versions
 *
 * Revision 2.193  2010-06-07 18:32:06+05:30  Cprogrammer
 * added connect_all argument to findmdahost()
 *
 * Revision 2.192  2010-05-02 08:28:51+05:30  Cprogrammer
 * added connect_all argument to vclear_open_smtp()
 *
 * Revision 2.191  2010-05-01 14:11:28+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.190  2010-04-24 14:58:51+05:30  Cprogrammer
 * define SMTP, QMTP, QMQP ports
 * new function get_smtp_qmtp_port to parse both smtproutes / qmtproutes
 *
 * Revision 2.189  2010-04-23 10:24:48+05:30  Cprogrammer
 * added spamcount arg to readLogFile()
 *
 * Revision 2.188  2010-04-15 13:43:04+05:30  Cprogrammer
 * added flags argument to set_mysql_options()
 *
 * Revision 2.187  2010-04-11 22:09:27+05:30  Cprogrammer
 * changed LPWD_QUERY to LIMIT_QUERY (domain limits)
 * added VlimitInlookup() function
 *
 * Revision 2.186  2010-02-24 15:03:28+05:30  Cprogrammer
 * removed SOCKET and PORT definitions for MySQL
 *
 * Revision 2.185  2010-02-19 10:35:32+05:30  Cprogrammer
 * changed default control host socket to /var/lib/mysql/mysql.sock
 *
 * Revision 2.184  2010-02-18 08:35:15+05:30  Cprogrammer
 * changed default socket to /var/lib/mysql/mysql.sock
 *
 * Revision 2.183  2010-02-16 13:07:14+05:30  Cprogrammer
 * added post_handle() function
 *
 * Revision 2.182  2009-12-01 10:34:22+05:30  Cprogrammer
 * changed order of passwd_expiry column
 *
 * Revision 2.181  2009-10-09 11:59:54+05:30  Cprogrammer
 * added definitions for record states in lastauth
 *
 * Revision 2.180  2009-09-28 13:45:47+05:30  Cprogrammer
 * added chk_rcpt argument to vadddomain
 *
 * Revision 2.179  2009-09-25 23:49:31+05:30  Cprogrammer
 * changed mdir_t to signed
 *
 * Revision 2.178  2009-09-23 15:00:01+05:30  Cprogrammer
 * change for new runcmmd
 *
 * Revision 2.177  2009-09-13 12:45:17+05:30  Cprogrammer
 * new argument to valias_insert() function
 *
 * Revision 2.176  2009-08-11 17:00:00+05:30  Cprogrammer
 * removed adminCmmd(), auth_admin()
 *
 * Revision 2.175  2009-03-13 20:13:13+05:30  Cprogrammer
 * added last_error field to dbinfo
 *
 * Revision 2.174  2009-02-10 09:26:09+05:30  Cprogrammer
 * added prototypes
 *
 * Revision 2.173  2009-01-27 14:15:54+05:30  Cprogrammer
 * removed BASE_PATH
 *
 * Revision 2.172  2009-01-15 08:59:59+05:30  Cprogrammer
 * added option to remove_line to continue after first matched line
 *
 * Revision 2.171  2009-01-13 14:40:13+05:30  Cprogrammer
 * added operation_mode argument (add = 1, delete = 2) to backfill()
 *
 * Revision 2.170  2009-01-12 10:36:46+05:30  Cprogrammer
 * added new function to backfill empty slots in dir_control
 *
 * Revision 2.169  2008-11-07 10:08:33+05:30  Cprogrammer
 * added findhost_cache()
 *
 * Revision 2.168  2008-11-06 18:12:32+05:30  Cprogrammer
 * removed flushpw()
 *
 * Revision 2.167  2008-11-06 15:39:12+05:30  Cprogrammer
 * added cache reset functions
 *
 * Revision 2.166  2008-10-29 11:17:26+05:30  Cprogrammer
 * data type for disable_mysql_escape changed to int
 *
 * Revision 2.165  2008-10-24 22:13:25+05:30  Cprogrammer
 * added disable_mysql_escape()
 *
 * Revision 2.164  2008-10-21 20:28:48+05:30  Cprogrammer
 * BUG - Change of quotes for mysql_escape_string bug
 *
 * Revision 2.163  2008-10-20 19:05:13+05:30  Cprogrammer
 * added passwd_expiry
 *
 * Revision 2.162  2008-09-12 09:57:06+05:30  Cprogrammer
 * added function in_crypt ()
 *
 * Revision 2.161  2008-09-08 09:44:14+05:30  Cprogrammer
 * added MYSQL_SOCKET, CNTRL_SOCKET definitions
 * added prototype for hmac_md5
 *
 * Revision 2.160  2008-08-29 14:00:53+05:30  Cprogrammer
 * add function makesalt()
 * changed passwd column lenght to 128 to accomodate SHA512 hash
 *
 * Revision 2.159  2008-08-28 21:53:09+05:30  Cprogrammer
 * increased pw_passwd length for SHA512 has
 *
 * Revision 2.158  2008-08-02 09:07:02+05:30  Cprogrammer
 * removed function verror.
 * added function error_stack()
 *
 * Revision 2.157  2008-07-14 19:48:54+05:30  Cprogrammer
 * removed arc4random (clash on Mac OS X)
 *
 * Revision 2.156  2008-07-13 19:43:42+05:30  Cprogrammer
 * 64bit port
 *
 * Revision 2.155  2008-06-25 14:09:57+05:30  Cprogrammer
 * added PRId64 and SCNd64
 *
 * Revision 2.154  2008-06-24 21:48:00+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.153  2008-06-24 10:48:36+05:30  Cprogrammer
 * added GNU GPL Version 3
 *
 * Revision 2.152  2008-06-13 10:13:15+05:30  Cprogrammer
 * moved vset_lastdeliver outside ENABLE_AUTH_LOGGING
 *
 * Revision 2.151  2008-06-13 09:29:15+05:30  Cprogrammer
 * define lastauth, userquota only if ENABLE_AUTH_LOGGING is defined
 *
 * Revision 2.150  2008-06-13 08:19:38+05:30  Cprogrammer
 * moved vpriv functions, checkperm to CLUSTERED_SITE
 * BUG - vauth_renamedomain was defined incorrectly inside CLUSTERED_SITE
 * moved adminCommand, IndiMailTable outside CLUSTERED_SITE
 *
 * Revision 2.149  2008-06-03 18:37:34+05:30  Cprogrammer
 * added mdahost argument to vadduser
 *
 * Revision 2.148  2008-05-28 21:55:41+05:30  Cprogrammer
 * removed LDAP_QUERY
 *
 * Revision 2.147  2008-05-28 15:09:31+05:30  Cprogrammer
 * removed ldap and cdb modules
 *
 * Revision 2.146  2008-05-27 22:35:11+05:30  Cprogrammer
 * new structure for all indimail tables
 *
 * Revision 2.145  2007-12-24 22:10:25+05:30  Cprogrammer
 * corrected timestamp columns
 *
 * Revision 2.144  2007-12-22 00:14:29+05:30  Cprogrammer
 * changed default mcd filename to mcdinfo
 *
 * Revision 2.143  2006-03-02 20:40:27+05:30  Cprogrammer
 * added last_attempted, failed_attempts to dbinfo structure
 *
 * Revision 2.142  2006-01-23 21:49:24+05:30  Cprogrammer
 * added mailboxpurge()
 *
 * Revision 2.141  2005-12-29 23:04:46+05:30  Cprogrammer
 * added domain_expiry column to vlimit table
 *
 * Revision 2.140  2005-12-29 22:49:46+05:30  Cprogrammer
 * added getEnvConfigStr(), set_mysql_options()
 *
 * Revision 2.139  2005-01-21 21:17:31+05:30  Cprogrammer
 * removed vfd_move(), vfd_copy()
 *
 * Revision 2.138  2004-09-21 17:01:34+05:30  Cprogrammer
 * added activate Flag to vadduser() and vauth_adduser()
 *
 * Revision 2.137  2004-09-20 19:54:15+05:30  Cprogrammer
 * added mails_flag argument to autoAddUser()
 *
 * Revision 2.136  2004-07-08 14:10:12+05:30  Cprogrammer
 * added function runcmmd()
 *
 * Revision 2.135  2004-06-30 09:06:54+05:30  Cprogrammer
 * removed '/' from ATCHARS
 *
 * Revision 2.134  2004-06-21 22:51:49+05:30  Cprogrammer
 * added MAILSIZE_LIMIT
 *
 * Revision 2.133  2004-06-20 00:48:16+05:30  Cprogrammer
 * added MAILCOUNT_LIMIT
 * changed name of QUOTA_MAILSIZE to OVERQUOTA_MAILSIZE
 *
 * Revision 2.132  2004-06-11 10:04:24+05:30  Cprogrammer
 * added function hostcntrl_select_all()
 *
 * Revision 2.131  2004-05-22 22:30:25+05:30  Cprogrammer
 * renamed ip_addr to ipaddr;
 *
 * Revision 2.130  2004-05-17 14:02:13+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.129  2004-05-17 01:29:09+05:30  Cprogrammer
 * changes for mysql 4.x
 * added timestamp argument to hostcntrl_select()
 *
 * Revision 2.128  2004-05-17 01:06:15+05:30  Cprogrammer
 * added force flag to addusercntrl()
 *
 * Revision 2.127  2004-05-17 00:46:04+05:30  Cprogrammer
 * added hostcntrl_select()
 * added force flag to delusercntrl()
 *
 * Revision 2.126  2004-05-16 23:13:28+05:30  Cprogrammer
 * added hostid argument to addusercntrl()
 * removed pwdusercntrl()
 * added updusercntrl()
 *
 * Revision 2.125  2004-05-10 18:07:48+05:30  Cprogrammer
 * increased size of name, domain and email id
 *
 * Revision 2.124  2004-05-03 22:02:29+05:30  Cprogrammer
 * use stdarg.h instead of varargs.h
 *
 * Revision 2.123  2004-02-18 14:24:29+05:30  Cprogrammer
 * added domain query
 *
 * Revision 2.122  2003-12-30 00:29:30+05:30  Cprogrammer
 * added headerlist()
 *
 * Revision 2.121  2003-12-23 00:08:22+05:30  Cprogrammer
 * removed spamfilter argument in spamReport()
 *
 * Revision 2.120  2003-12-22 21:12:46+05:30  Cprogrammer
 * added filename argument to LoadBMF()
 *
 * Revision 2.119  2003-12-21 14:21:59+05:30  Cprogrammer
 * added spamFilter argument to spamReport() and readLogFile()
 *
 * Revision 2.118  2003-12-06 17:29:05+05:30  Cprogrammer
 * increased size of valias_line to accomodate qmail-autoresponder
 * added function count_table()
 * changed return type of valiasCount to long
 *
 * Revision 2.117  2003-12-05 10:51:43+05:30  Cprogrammer
 * added loadbalance()
 * changed bounce_action to char(64) in vfilter table
 *
 * Revision 2.116  2003-10-27 23:35:41+05:30  Cprogrammer
 * added member isLocal to dbinfo structure
 *
 * Revision 2.115  2003-10-24 23:11:44+05:30  Cprogrammer
 * added vlimits_get_flag_mask()
 *
 * Revision 2.114  2003-10-24 00:28:45+05:30  Cprogrammer
 * added writemcdinfo()
 *
 * Revision 2.113  2003-10-23 14:14:43+05:30  Cprogrammer
 * added NO_SMTP and V_OVERRIDE flags
 *
 * Revision 2.112  2003-10-23 00:20:52+05:30  Cprogrammer
 * added atrn_access() prototype
 *
 * Revision 2.111  2003-10-22 01:51:14+05:30  Cprogrammer
 * added argument to getnpass()
 *
 * Revision 2.110  2003-10-05 23:59:36+05:30  Cprogrammer
 * removed primary key for atrn_map
 * arguments for vshow_atrn_map() changed to char **
 *
 * Revision 2.109  2003-09-16 12:31:53+05:30  Cprogrammer
 * vpriv_select() domain arg changed to char **
 * added mgmtlist()
 *
 * Revision 2.108  2003-09-14 01:57:44+05:30  Cprogrammer
 * added function checkPerm()
 *
 * Revision 2.107  2003-09-13 23:54:14+05:30  Cprogrammer
 * added vpriv functions for implementing privileges
 *
 * Revision 2.106  2003-09-10 19:19:25+05:30  Cprogrammer
 * vlimit structure change
 *
 * Revision 2.105  2003-08-24 16:03:21+05:30  Cprogrammer
 * additional domain argument added to LdapGetpw() and LdapSetpw()
 *
 * Revision 2.104  2003-07-04 11:35:04+05:30  Cprogrammer
 * added ATRN_MAP_LAYOUT and atrn function prototypes.
 *
 * Revision 2.103  2003-06-13 11:25:27+05:30  Cprogrammer
 * incresed DBINFO_BUFF to 128
 *
 * Revision 2.102  2003-06-08 19:12:39+05:30  Cprogrammer
 * added function wildmat()
 *
 * Revision 2.101  2003-04-12 00:20:41+05:30  Cprogrammer
 * replaced admin_commands with a structure
 *
 * Revision 2.100  2003-03-30 23:15:44+05:30  Cprogrammer
 * added argument skipGecos to getindimail
 *
 * Revision 2.99  2003-03-26 12:33:29+05:30  Cprogrammer
 * additional argument Subject added to SendWelcomeMail
 * changed definition of ACTIVATEMAIL
 *
 * Revision 2.98  2003-03-05 00:38:13+05:30  Cprogrammer
 * added domain filter to vshow_ip_map()
 *
 * Revision 2.97  2003-03-05 00:18:20+05:30  Cprogrammer
 * added vupd_ip_map()
 *
 * Revision 2.96  2003-03-04 21:07:17+05:30  Cprogrammer
 * added status argument to vfstab_update()
 *
 * Revision 2.95  2003-03-04 20:58:11+05:30  Cprogrammer
 * added row_format argument to dbinfoSelect()
 *
 * Revision 2.94  2003-02-03 21:54:49+05:30  Cprogrammer
 * added function layout()
 * added argument input_read to adminCmmd()
 *
 * Revision 2.93  2003-02-03 00:58:07+05:30  Cprogrammer
 * added function matrixToTable()
 *
 * Revision 2.92  2003-02-02 23:49:35+05:30  Cprogrammer
 * added lockTable()
 *
 * Revision 2.91  2003-02-02 00:24:22+05:30  Cprogrammer
 * added tableToMatrix()
 *
 * Revision 2.90  2003-02-01 22:50:25+05:30  Cprogrammer
 * added argument to LoadBMF()
 *
 * Revision 2.89  2003-02-01 15:34:13+05:30  Cprogrammer
 * removed update_flag argument
 *
 * Revision 2.88  2003-02-01 14:12:24+05:30  Cprogrammer
 * Change for CopyEmailFile() change
 *
 * Revision 2.87  2003-01-26 18:30:14+05:30  Cprogrammer
 * added variable use_vfilter
 *
 * Revision 2.86  2003-01-22 16:02:05+05:30  Cprogrammer
 * added option to select domain in dbinfoSelect()
 *
 * Revision 2.85  2003-01-17 01:07:25+05:30  Cprogrammer
 * added function MdaServer()
 *
 * Revision 2.84  2003-01-14 12:43:46+05:30  Cprogrammer
 * added silent option to print_control()
 *
 * Revision 2.83  2003-01-09 23:46:57+05:30  Cprogrammer
 * return type changed to int for error handling
 *
 * Revision 2.82  2003-01-04 00:17:22+05:30  Cprogrammer
 * removed definition of QMAILDIR as already present in config.h
 *
 * Revision 2.81  2003-01-03 02:45:44+05:30  Cprogrammer
 * removed redundant vcreate_... tables
 *
 * Revision 2.80  2003-01-03 02:27:48+05:30  Cprogrammer
 * removed function vcreate_cntrl_table
 *
 * Revision 2.79  2003-01-01 02:36:48+05:30  Cprogrammer
 * added dbinfoUpdate() and dbinfoSelect()
 *
 * Revision 2.78  2002-12-29 19:01:55+05:30  Cprogrammer
 * added GetSmtproute()
 *
 * Revision 2.77  2002-12-27 16:41:50+05:30  Cprogrammer
 * added function valiasCount()
 *
 * Revision 2.76  2002-12-26 03:27:14+05:30  Cprogrammer
 * added getremoteip()
 *
 * Revision 2.75  2002-12-21 18:22:13+05:30  Cprogrammer
 * added function FreeMakeArgs()
 *
 * Revision 2.74  2002-12-08 20:09:29+05:30  Cprogrammer
 * added functions dbinfoAdd(), dbinfoDel()
 *
 * Revision 2.73  2002-12-06 23:42:04+05:30  Cprogrammer
 * new function format_print_display()
 *
 * Revision 2.72  2002-12-05 18:33:42+05:30  Cprogrammer
 * changes to incorporate forwarding address
 *
 * Revision 2.71  2002-12-04 02:07:00+05:30  Cprogrammer
 * added additional argument to mgmtpassinfo() to supress printing
 *
 * Revision 2.70  2002-12-01 18:53:15+05:30  Cprogrammer
 * changed arguments to Scandir() to mdir_t
 *
 * Revision 2.69  2002-11-30 21:31:53+05:30  Cprogrammer
 * added service argument to autoAddUser()
 *
 * Revision 2.68  2002-11-28 00:49:20+05:30  Cprogrammer
 * reorganized few entries
 * added variable admin_command
 *
 * Revision 2.67  2002-11-22 01:17:37+05:30  Cprogrammer
 * positions of functions corrected
 *
 * Revision 2.66  2002-11-18 20:03:39+05:30  Cprogrammer
 * added definition MAX_LINK_COUNT for CopyEmailFile.c
 *
 * Revision 2.65  2002-11-18 12:43:42+05:30  Cprogrammer
 * change in functions vfilter_display() and vfilter_insert()
 *
 * Revision 2.64  2002-11-14 15:01:32+05:30  Cprogrammer
 * added Prototype Macro definition
 *
 * Revision 2.63  2002-11-13 13:37:48+05:30  Cprogrammer
 * added filter name
 *
 * Revision 2.62  2002-10-27 21:43:14+05:30  Cprogrammer
 * added definition ON_MASTER, ON_LOCAL for create_table()
 * added function create_table()
 *
 * Revision 2.61  2002-10-27 19:42:49+05:30  Cprogrammer
 * added definitions, functions for incorporating domain limits
 *
 * Revision 2.60  2002-10-21 01:33:31+05:30  Cprogrammer
 * changed vcreate_bmf_table()
 * added SPAM_TABLE_LAYOUT
 * changed BADMAILFROM_TABLE_LAYOUT
 * added UpdateSpamTable()
 *
 * Revision 2.59  2002-10-20 22:19:13+05:30  Cprogrammer
 * added functions LoadBMF(), vcreate_bmf_table()
 * changed table definition for badmailfrom
 *
 * Revision 2.58  2002-10-19 18:24:51+05:30  Cprogrammer
 * added spam functions
 *
 * Revision 2.57  2002-10-18 01:19:32+05:30  Cprogrammer
 * added array rfc_ids[] for list of RFC ids
 *
 * Revision 2.56  2002-10-16 23:45:11+05:30  Cprogrammer
 * added function skip_system_files()
 *
 * Revision 2.55  2002-10-15 11:44:55+05:30  Cprogrammer
 * change in vfilter_mlistOpt() to update based on comparision (5 or 6) rather than on filter_no
 *
 * Revision 2.54  2002-10-14 21:05:28+05:30  Cprogrammer
 * changed vfilter_select()
 * added vfilter_filterNo(), vfilter_mlistOpt(), mlist_filterupdate()
 *
 * Revision 2.53  2002-10-12 23:07:50+05:30  Cprogrammer
 * moved prototype for auth_admin() to non-clustered section
 *
 * Revision 2.52  2002-10-12 21:17:17+05:30  Cprogrammer
 * restructured function prototypes
 * added deliver_mail(), get_message_size()
 *
 * Revision 2.51  2002-10-11 21:49:02+05:30  Cprogrammer
 * added getAddressBook()
 *
 * Revision 2.50  2002-10-11 21:38:36+05:30  Cprogrammer
 * added rewindAddrToken()
 *
 * Revision 2.49  2002-10-11 20:08:26+05:30  Cprogrammer
 * added mlist_filterno(), vfilter_display()
 * change in vuserinfo() function
 *
 * Revision 2.48  2002-10-11 01:04:17+05:30  Cprogrammer
 * added is_mailing_list()
 *
 * Revision 2.47  2002-10-10 23:37:48+05:30  Cprogrammer
 * added function parseAddress()
 * added i_headers structure
 *
 * Revision 2.46  2002-10-10 23:21:18+05:30  Cprogrammer
 * changed storeHeader()
 *
 * Revision 2.45  2002-10-10 03:34:10+05:30  Cprogrammer
 * added addressToken()
 *
 * Revision 2.44  2002-10-09 23:26:23+05:30  Cprogrammer
 * added mlist_insert(), mlist_update(), mlist_delete()
 * changed index for table mailing_list
 *
 * Revision 2.43  2002-10-09 21:09:24+05:30  Cprogrammer
 * added getmailingList()
 *
 * Revision 2.42  2002-10-09 18:56:56+05:30  Cprogrammer
 * added structure "header"
 * added storeHeader()
 *
 * Revision 2.41  2002-09-30 23:48:26+05:30  Cprogrammer
 * added filter definitions
 *
 * Revision 2.40  2002-09-04 12:57:24+05:30  Cprogrammer
 * added maildir_to_email()
 *
 * Revision 2.39  2002-09-01 19:45:13+05:30  Cprogrammer
 * added pipe_exec() function
 *
 * Revision 2.38  2002-08-30 23:31:05+05:30  Cprogrammer
 * added variable isopen_vauthinit
 *
 * Revision 2.37  2002-08-25 22:36:33+05:30  Cprogrammer
 * added add_control(), autoturn_dir()
 * vadddomain addtional argument for etrn support
 *
 * Revision 2.36  2002-08-11 14:18:08+05:30  Cprogrammer
 * changed AVG_USER_QUOTA to string
 *
 * Revision 2.35  2002-08-11 00:32:44+05:30  Cprogrammer
 * changed datatype for fstab columns to bigint
 *
 * Revision 2.34  2002-08-07 19:39:27+05:30  Cprogrammer
 * added fstab routines
 *
 * Revision 2.33  2002-08-05 01:16:35+05:30  Cprogrammer
 * added mysql_escape()
 *
 * Revision 2.32  2002-08-03 00:38:57+05:30  Cprogrammer
 * added domain argument to SqlServer()
 *
 * Revision 2.31  2002-08-02 18:18:57+05:30  Cprogrammer
 * added function mgmtadduser()
 *
 * Revision 2.30  2002-08-02 00:12:21+05:30  Cprogrammer
 * added columns zztimestamp to bulkmail and mgmtaccess
 *
 * Revision 2.29  2002-08-01 16:04:15+05:30  Cprogrammer
 * added argument inactFlag to SendWelcomeMail()
 *
 * Revision 2.28  2002-07-31 18:42:41+05:30  Cprogrammer
 * added udpopen()
 *
 * Revision 2.27  2002-07-23 00:06:54+05:30  Cprogrammer
 * added function mgmtpassinfo()
 *
 * Revision 2.26  2002-07-22 21:02:58+05:30  Cprogrammer
 * define autoAdduser only if USE_LDAP_PASSWD defined
 *
 * Revision 2.25  2002-07-22 19:39:49+05:30  Cprogrammer
 * added autoAddUser()
 * change in LdapGetpw()
 *
 * Revision 2.24  2002-07-15 19:46:59+05:30  Cprogrammer
 * added LDAP_QUERY
 *
 * Revision 2.23  2002-07-15 18:58:28+05:30  Cprogrammer
 * added adminCmmd()
 *
 * Revision 2.22  2002-07-15 18:41:50+05:30  Cprogrammer
 * added auth_admin()
 *
 * Revision 2.21  2002-07-15 02:07:27+05:30  Cprogrammer
 * update of indimail changed to update low_priority
 * added mgmtaccess passwd functions
 *
 * Revision 2.20  2002-07-09 14:47:21+05:30  Cprogrammer
 * added variable ldapversion
 *
 * Revision 2.19  2002-07-05 00:38:04+05:30  Cprogrammer
 * definition of welcomemail
 *
 * Revision 2.18  2002-07-03 01:22:39+05:30  Cprogrammer
 * added copyPwdStruct()
 *
 * Revision 2.17  2002-07-01 19:06:22+05:30  Cprogrammer
 * addition argument allow_pop3 added to LdapGetpw()
 *
 * Revision 2.16  2002-06-26 03:23:27+05:30  Cprogrammer
 * changes for implementing USE_LDAP_PASSWD
 *
 * Revision 2.15  2002-05-16 01:09:19+05:30  Cprogrammer
 * added definition for WELCOMEMAIL
 * added SendWelcomeMail()
 *
 * Revision 2.14  2002-05-13 12:37:00+05:30  Cprogrammer
 * removed 2nd argument to vclear_open_smtp()
 *
 * Revision 2.13  2002-05-13 02:28:17+05:30  Cprogrammer
 * added vcreate_dbinfo_table()
 * added DBINFO table layout for dbinfo syncing
 * added member distributed to DBINFO
 *
 * Revision 2.12  2002-05-12 01:24:06+05:30  Cprogrammer
 * added add_vacation()
 *
 * Revision 2.11  2002-05-11 16:33:46+05:30  Cprogrammer
 * added vcreate_mgmtaccess_table()
 *
 * Revision 2.10  2002-05-11 00:20:24+05:30  Cprogrammer
 * added pw_comp()
 *
 * Revision 2.9  2002-05-10 15:29:15+05:30  Cprogrammer
 * pw_gid codes standardized
 *
 * Revision 2.8  2002-05-09 00:38:09+05:30  Cprogrammer
 * added argument domdir to vauth_renamedomain()
 *
 * Revision 2.7  2002-05-04 00:57:13+05:30  Cprogrammer
 * added function vauth_renamedomain()
 *
 * Revision 2.6  2002-04-26 15:27:01+05:30  Cprogrammer
 * added vrenameuser()
 *
 * Revision 2.5  2002-04-24 15:11:11+05:30  Cprogrammer
 * added pop3d_capability() and imapd_capability()
 *
 * Revision 2.4  2002-04-12 15:52:18+05:30  Cprogrammer
 * replaced insert_bulletin() with bulletin()
 *
 * Revision 2.3  2002-04-12 12:25:56+05:30  Cprogrammer
 * added domain argument to mdaMysqlConnect()
 *
 * Revision 2.2  2002-04-12 02:09:12+05:30  Cprogrammer
 * added MdaMysqlConnect()
 *
 * Revision 2.1  2002-04-11 23:39:29+05:30  Cprogrammer
 * added insert_bulletin()
 *
 * Revision 1.78  2002-04-10 15:26:48+05:30  Cprogrammer
 * removed redundant variables timeout and table
 *
 * Revision 1.77  2002-04-10 03:00:53+05:30  Cprogrammer
 * added timeoutread() and timeoutwrite()
 *
 * Revision 1.76  2002-04-09 20:29:32+05:30  Cprogrammer
 * added member 'fd' to indicate if mysql connection has succeeded or not
 *
 * Revision 1.75  2002-04-09 13:48:07+05:30  Cprogrammer
 * added PwdInLookup() and AliasInLookup()
 *
 * Revision 1.74  2002-04-08 23:38:35+05:30  Cprogrammer
 * added function relay_select()
 *
 * Revision 1.73  2002-04-08 21:30:09+05:30  Cprogrammer
 * removed variable relay_table
 *
 * Revision 1.72  2002-04-08 19:22:12+05:30  Cprogrammer
 * added definition for INFIFO
 *
 * Revision 1.71  2002-04-08 16:49:04+05:30  Cprogrammer
 * added definitions for Fifo Server.
 *
 * Revision 1.70  2002-04-08 03:48:30+05:30  Cprogrammer
 * added findmdahost() and extern variable is_overquota
 *
 * Revision 1.69  2002-04-07 13:43:33+05:30  Cprogrammer
 * added inquery(), vauthOpen_user(), ProcessInFifo(), strToPw()
 *
 * Revision 1.68  2002-04-06 22:38:49+05:30  Cprogrammer
 * removed mysqlreconnect, PingDb, LoadDbInfo_BIN, DumpDbInfo
 * added FifoCreate
 *
 * Revision 1.67  2002-04-03 01:45:37+05:30  Cprogrammer
 * Added ServiceType (imap or pop3) argument to Login_Tasks
 *
 * Revision 1.66  2002-04-01 04:05:20+05:30  Cprogrammer
 * added function readPidLock()
 *
 * Revision 1.65  2002-04-01 01:39:05+05:30  Cprogrammer
 * added delDbLock()
 *
 * Revision 1.64  2002-03-31 21:52:19+05:30  Cprogrammer
 * added RemoveLock() and getDbLock()
 *
 * Revision 1.63  2002-03-29 23:32:01+05:30  Cprogrammer
 * added vauth_gethostid()
 *
 * Revision 1.62  2002-03-29 20:48:27+05:30  Cprogrammer
 * added host_table
 * added get_local_hostid()
 * added AuthModuser()
 * added connect_db(), PingDb(), LoadDbInfo_TXT(), LoadDbInfo_BIN()
 * added DumpDbinfo()
 * added vcreate_host_table(), vauth_getipaddr(), vhostid_select(), vhostid_insert(),
 * vhostid_update(), vhostid_delete()
 *
 * Revision 1.61  2002-03-28 23:59:36+05:30  Cprogrammer
 * added comments for dbinfo structure
 * added SqlServer
 *
 * Revision 1.60  2002-03-28 01:13:45+05:30  Cprogrammer
 * change in no of arguments to proxylogin() and monkey()
 *
 * Revision 1.59  2002-03-27 18:52:37+05:30  Cprogrammer
 * added memstore()
 *
 * Revision 1.58  2002-03-27 11:16:10+05:30  Cprogrammer
 * changed name of check_user() to UserInLookup(), check_relay to RelayInLookup()
 *
 * Revision 1.57  2002-03-27 01:54:24+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.56  2002-03-27 00:57:12+05:30  Cprogrammer
 * added DBINFO and tcpserver/smtpd functions
 *
 * Revision 1.55  2002-03-25 00:38:28+05:30  Cprogrammer
 * add vauth_init()
 * added semaphore locking code
 *
 * Revision 1.54  2002-03-24 19:16:06+05:30  Cprogrammer
 * get_write_lock() modification
 *
 * Revision 1.53  2002-03-19 20:42:59+05:30  Cprogrammer
 * added function mysqlquery()
 *
 * Revision 1.52  2002-03-13 09:56:43+05:30  Cprogrammer
 * added function mysyqlreconnect()
 *
 * Revision 1.51  2002-03-03 16:15:13+05:30  Cprogrammer
 * added BULKMAIL_TABLE_LAYOUT
 * added function vcreat_bulkmail_table()
 *
 * Revision 1.50  2002-03-03 15:42:26+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.49  2002-03-02 01:27:53+05:30  Cprogrammer
 * added function RemoteBulkMail()
 *
 * Revision 1.48  2002-02-25 13:55:52+05:30  Cprogrammer
 * corrected wrong placements of few definitions
 *
 * Revision 1.47  2002-02-24 22:09:06+05:30  Cprogrammer
 * added remove_quotes()
 *
 * Revision 1.46  2002-02-24 03:27:53+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.45  2002-02-23 23:59:00+05:30  Cprogrammer
 * added additional argument to get the ip address
 *
 * Revision 1.44  2002-02-23 20:34:20+05:30  Cprogrammer
 * added isvalid_domain()
 * corrected problem with #define inside mysql column definitions
 *
 * Revision 1.43  2002-02-23 00:24:32+05:30  Cprogrammer
 * Major Revamp of Vpopmail.h
 * a) Changed varchar to char for performance
 * b) Introduced constants for column lenght definitions
 *
 * Revision 1.42  2002-02-22 17:35:47+05:30  Cprogrammer
 * reduced size of column definitions
 *
 * Revision 1.41  2001-12-30 09:53:47+05:30  Cprogrammer
 * #define for MAX_DOMAINNAME
 *
 * Revision 1.40  2001-12-29 11:05:35+05:30  Cprogrammer
 * added new argument type to Delunreadmails()
 * ;
 *
 * Revision 1.39  2001-12-27 20:46:21+05:30  Cprogrammer
 * new index timestamp
 *
 * Revision 1.38  2001-12-27 01:31:18+05:30  Cprogrammer
 * prototype change for getversion()
 *
 * Revision 1.37  2001-12-24 15:01:07+05:30  Cprogrammer
 * added proxylogin()
 *
 * Revision 1.36  2001-12-24 00:58:21+05:30  Cprogrammer
 * code revamp for trashpurge() and Delunreadmails()
 *
 * Revision 1.35  2001-12-22 21:03:23+05:30  Cprogrammer
 * change in ScanDir and vuserinfo to display mail summary
 *
 * Revision 1.34  2001-12-21 01:11:42+05:30  Cprogrammer
 * added vauth_get_realdomain()
 *
 * Revision 1.33  2001-12-21 00:36:46+05:30  Cprogrammer
 * removed unecessary definitions, added ALIASDOMAIN_TABLE_LAYOUT
 *
 * Revision 1.32  2001-12-19 21:23:43+05:30  Cprogrammer
 * added prototype for pwcomp()
 *
 * Revision 1.31  2001-12-19 16:30:06+05:30  Cprogrammer
 * added function Dirname()
 *
 * Revision 1.30  2001-12-13 13:36:18+05:30  Cprogrammer
 * define MASTER_HOST
 *
 * Revision 1.29  2001-12-13 00:33:35+05:30  Cprogrammer
 * added is_already_running(), MakeArgs
 * change in tcpbind() function
 *
 * Revision 1.28  2001-12-12 13:44:18+05:30  Cprogrammer
 * removed relay table related variables
 * change in get_smtp_service_port to accomodate mda ip address
 * change in vsmtp functions to accomodate mda ip address
 *
 * Revision 1.27  2001-12-10 00:04:37+05:30  Cprogrammer
 * prototype change for vsmtp_update()
 *
 * Revision 1.26  2001-12-09 23:57:07+05:30  Cprogrammer
 * added vsmtp functions
 * prototype change for valias_update()
 *
 * Revision 1.25  2001-12-09 00:59:38+05:30  Cprogrammer
 * added column port to smtp_port
 *
 * Revision 1.24  2001-12-08 23:54:10+05:30  Cprogrammer
 * added table definition for SMTP table
 *
 * Revision 1.23  2001-12-08 17:46:19+05:30  Cprogrammer
 * added islocalif()
 *
 * Revision 1.22  2001-12-08 14:44:38+05:30  Cprogrammer
 * added function tcpbind()
 *
 * Revision 1.21  2001-12-08 00:37:24+05:30  Cprogrammer
 * added tcp/ip functions
 *
 * Revision 1.20  2001-12-03 04:18:48+05:30  Cprogrammer
 * added vlog(), trashpurge(), Delunreadmails()
 *
 * Revision 1.19  2001-12-02 20:24:13+05:30  Cprogrammer
 * prototype change for GetPrefix()
 *
 * Revision 1.18  2001-12-02 18:51:42+05:30  Cprogrammer
 * dir_control function call changes
 *
 * Revision 1.17  2001-12-01 23:11:57+05:30  Cprogrammer
 * added qmail_tmda(), replacestr() and valias_update()
 *
 * Revision 1.16  2001-12-01 02:16:35+05:30  Cprogrammer
 * added functions qmail_remote() and error_temp()
 *
 * Revision 1.15  2001-11-30 00:15:40+05:30  Cprogrammer
 * added variables cntrl_table and relay_table
 *
 * Revision 1.14  2001-11-29 21:00:01+05:30  Cprogrammer
 * code change for saving mysql connections
 * code change for having a different relay table
 *
 * Revision 1.13  2001-11-29 13:31:00+05:30  Cprogrammer
 * added functions vauth_getflags(), cntrl_clearaddflags(), cntrl_clearpwdflags(), cntrl_cleardelflags()
 *
 * Revision 1.12  2001-11-29 13:21:21+05:30  Cprogrammer
 * added verbose switch
 *
 * Revision 1.11  2001-11-28 23:11:40+05:30  Cprogrammer
 * added addusercntrl(), delusercntrl(), pwdusercntrl()
 * added index on pw_uid on indimail
 * added field timestamp on hostcntrl
 * added definitions for ADD_FLAG, PWD_FLAG and DEL_FLAG
 *
 * Revision 1.10  2001-11-24 20:37:59+05:30  Cprogrammer
 * added function no_of_days();
 *
 * Revision 1.9  2001-11-24 20:27:44+05:30  Cprogrammer
 * added flushpw()
 * prototype changes for getindimail() and getlastauth()
 *
 * Revision 1.8  2001-11-24 12:23:02+05:30  Cprogrammer
 * version information added
 * added index on quota to lastauth
 *
 * Revision 1.7  2001-11-23 20:57:19+05:30  Cprogrammer
 * Prototype change for vget_lastauth();
 *
 * Revision 1.6  2001-11-23 00:16:09+05:30  Cprogrammer
 * remove function pw_name_getall and add getindimail()
 *
 * Revision 1.5  2001-11-22 22:51:40+05:30  Cprogrammer
 * prototypes for functions
 * open_central_db()
 * vclose_cntrl()
 * vcreate_cntrl_table()
 * is_user_present()
 *
 * Revision 1.4  2001-11-20 21:49:18+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.3  2001-11-20 11:01:40+05:30  Cprogrammer
 * changes for distributed architecture and inactive user movement
 *
 * Revision 1.2  2001-11-14 19:29:19+05:30  Cprogrammer
 * distributed arc change
 *
 * Revision 1.1  2001-10-24 19:26:02+05:30  Cprogrammer
 * Initial revision
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef INDIMAILH_H
#define INDIMAILH_H

#ifndef	lint
static char     sccsidh[] = "$Id: indimail.h,v 2.230 2019-03-16 19:26:56+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#ifdef VFILTER
#include "eps.h"
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#include "typesx.h"

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
typedef int64_t mdir_t;
typedef uint64_t umdir_t;
#else
typedef long long mdir_t;
typedef unsigned long long umdir_t;
#define PRId64 "lld"
#define PRIu64 "llu"
#define SCNd64 "lld"
#define SCNu64 "llu"
#endif
#define SALTSIZE  32
#define PORT_SMTP 25
#define PORT_QMTP 209
#define PORT_QMQP 628

#ifdef USE_MYSQL
#include <mysql.h>
/* Edit to match your set up */
#define MYSQL_HOST              "localhost"
#define MYSQL_USER              "indimail"
#define MYSQL_PASSWD            "ssh-1.5-" /*- control file overrides this */
#define MYSQL_DATABASE          "indimail"
#define MYSQL_DEFAULT_TABLE     "indimail"
#define MYSQL_INACTIVE_TABLE    "indibak"
#define RELAY_DEFAULT_TABLE     "relay"

/* max field sizes */
#define MAX_PW_NAME             40
#define MAX_PW_DOMAIN           67
#define MAX_PW_HOST             32
#define MAX_PW_PASS             128
#define MAX_PW_GECOS            48
#define MAX_PW_DIR              156
#define MAX_PW_QUOTA            30
#define MAX_ALIAS_LINE          156
#define DBINFO_BUFF             128
#define INFIFO                  "infifo"
#define MCDFILE                 "mcdinfo"
#define FS_OFFLINE              0
#define FS_ONLINE               1
#define AVG_USER_QUOTA          "5000000"
#define ON_MASTER               0
#define ON_LOCAL                1

#define DBINFO_TABLE_LAYOUT "\
filename char(128) not null, \
domain   char(64) not null, \
distributed int not null, \
server   char(28) not null, \
mdahost  char(28) not null, \
port     int not null, \
use_ssl  int not null, \
dbname   char(28) not null, \
user     char(28) not null, \
passwd   char(28) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
unique index (filename, domain, server, mdahost, port, dbname, user, passwd), \
index (domain)"

#define MAX_FAIL_ATTEMPTS       5
#define MYSQL_RETRY_INTERVAL    5
struct dbinfo
{
	char            domain[DBINFO_BUFF];   /*- domain name */
	int             distributed;           /*- 1 for distributed, 0 for non-distributed */
	char            mdahost[DBINFO_BUFF];  /*- server for imap, pop3, delivery */
	char            server[DBINFO_BUFF];   /*- mysql server */
	int             port;                  /*- mysql port */
	char           *socket;                /*- mysql_socket */
	char            use_ssl;               /*- set for ssl connection */
	char            database[DBINFO_BUFF]; /*- mysql database */
	char            user[DBINFO_BUFF];     /*- mysql user */
	char            password[DBINFO_BUFF]; /*- mysql passwd */
	int             fd;
	time_t          last_attempted;
	int             failed_attempts;
	char            isLocal;
	char           *last_error;
};
typedef struct dbinfo DBINFO;
extern MYSQL    mysql[2];
extern MYSQL  **MdaMysql;
extern DBINFO **RelayHosts;

#ifdef CLUSTERED_SITE
#define CNTRL_HOST              "localhost"
#define MASTER_HOST             "localhost"
#define CNTRL_USER              "indimail"
#define CNTRL_PASSWD            "ssh-1.5-" /*- control file overrides this */
#define CNTRL_DATABASE          "indimail"
#define CNTRL_DEFAULT_TABLE     "hostcntrl"
#endif /*- #ifdef CLUSTERED_SITE */

/* defaults - no need to change */
#define SMALL_SITE              0
#define LARGE_SITE              1
#define MYSQL_DOT_CHAR          '_'
#define MYSQL_LARGE_USERS_TABLE "users"

#define VLOG_ERROR_INTERNAL     0  /* logs an internal error these messages only go to syslog if option is on */
#define VLOG_ERROR_LOGON        1  /* bad logon, user does not exist */
#define VLOG_AUTH               2  /* logs a successful authentication */
#define VLOG_ERROR_PASSWD       3  /* password is incorrect or empty*/
#define VLOG_ERROR_ACCESS       4  /* access is denied by 2 in gid */

#ifdef IP_ALIAS_DOMAINS
#define IP_ALIAS_MAP_FILE       "etc/ip_alias_map"
#define IP_ALIAS_TOKENS         " \t\n"
#endif

#define TRUE                 1
#define FALSE                0
#define ABORT               -1

#define AUTH_LOGIN       1
#define AUTH_PLAIN       2
#define AUTH_CRAM_MD5    3
#define AUTH_CRAM_SHA1   4
#define AUTH_CRAM_SHA256 5
#define AUTH_CRAM_SHA512 6
#define AUTH_CRAM_RIPEMD 7
#define AUTH_DIGEST_MD5  8

/*
 * What character marks an inverted character class? 
 */
#define NEGATE_CLASS		'^'
#define MAXADDR             "5000"
enum
{
	MULTIPLIER = 31,
	IGNOREHASHTAB = 1,
	SPAMMERHASHTAB = 2
};

/*
 * There are two hash tables:
 * 1. Hash table with addresses from log file
 * 2. The addresses that are to be ignored 
 */
typedef struct maddr maddr;
struct maddr
{
	char           *mail;
	int             cnt;
	maddr          *next;
};

#ifdef VFILTER
extern char    *vfilter_comparision[];
extern char    *vfilter_header[];
extern char    *i_headers[];
extern char    *h_mailinglist[];

#define FILTER_EQUAL                  0
#define FILTER_CONTAIN                1
#define FILTER_DOES_NOT_CONTAIN       2
#define FILTER_STARTS_WITH            3
#define FILTER_ENDS_WITH              4
#define FILTER_NOT_IN_ADDRESS_BOOK    5
#define FILTER_NOT_IN_TO_CC_BCC       6

struct header
{
	char           *name;
	char          **data;
	int             data_items;
};

#endif /*- #ifdef VFILTER */

#ifdef CLUSTERED_SITE
#define CNTRL_TABLE_LAYOUT "\
pw_name char(40) not null, \
pw_domain char(67) not null, \
pw_passwd char(128) not null, \
host char(64) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL, \
primary key (pw_name, pw_domain)"

#define HOST_TABLE_LAYOUT "\
host char(64) not null, \
ipaddr char(16) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key (host), index ipaddr (ipaddr)"

#define SMTP_TABLE_LAYOUT "\
host char(64) not null, \
src_host char(64) not null, \
domain char(64) not null, port int, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key (domain, host, src_host)"

#define ALIASDOMAIN_TABLE_LAYOUT "\
alias char(64) not null, \
domain char(67), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(alias)"

#define SPAM_TABLE_LAYOUT "\
email char(64) not null, \
spam_count int not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
index email (email), index spam_count (spam_count), index timestamp (timestamp)"

#define BADMAILFROM_TABLE_LAYOUT "\
email char(64) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key (email)"

extern char     cntrl_host[];
extern char    *cntrl_port;
extern int      isopen_cntrl;
extern int      isopen_vauthinit[];
#endif /*- #ifdef CLUSTERED_SITE */

typedef struct 
{
	char           *name; /*- User printable name of the function.  */
	char           *doc;  /*- Documentation for this function.  */
} ADMINCOMMAND;
extern ADMINCOMMAND adminCommands[];

typedef struct
{
	int             which;
	char           *table_name;
	char           *template;
} IndiMAILTable;
extern IndiMAILTable IndiMailTable[];

/* small site table layout */
#define SMALL_TABLE_LAYOUT "\
pw_name char(40) not null, \
pw_domain char(67) not null, \
pw_passwd char(128) not null, \
pw_uid int, \
pw_gid int, \
pw_gecos char(48) not null, \
pw_dir char(156), \
pw_shell char(30), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
primary key (pw_name, pw_domain), index pw_gecos (pw_gecos(25)), index pw_uid (pw_uid) "

/* large site table layout */
#define LARGE_TABLE_LAYOUT "\
pw_name char(40) not null, \
pw_passwd char(128) not null, \
pw_uid int, \
pw_gid int, \
pw_gecos char(48), \
pw_dir char(156), \
pw_shell char(30), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
primary key(pw_name)"

#define RELAY_TABLE_LAYOUT "\
email char(96) not null, \
ipaddr char(18) not null, \
timestamp int, \
unique index (email, ipaddr), index(ipaddr), index(timestamp)"

#ifdef IP_ALIAS_DOMAINS
#define IP_ALIAS_TABLE_LAYOUT "\
ipaddr char(18) not null, \
domain char(67), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(ipaddr)"
#endif

#ifdef ENABLE_AUTH_LOGGING

/* last auth definitions */
#define AUTH_TIME  1
#define CREAT_TIME 2
#define PASS_TIME  3
#define ACTIV_TIME 4
#define INACT_TIME 5
#define POP3_TIME  6
#define IMAP_TIME  7
#define WEBM_TIME  8

#define LASTAUTH_TABLE_LAYOUT "\
user char(40) not null, \
domain char(67) not null,\
service char(10) not null, \
remote_ip char(16) not null,  \
quota int not null, \
gecos char(48) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL, \
primary key (user, domain, service), index gecos (gecos), index quota (quota), \
index timestamp (timestamp)"

#define USERQUOTA_TABLE_LAYOUT "\
user char(40) not null, \
domain char(67) not null,\
quota bigint unsigned not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL, \
primary key(user, domain), index quota (quota)"
#endif

#define DIR_CONTROL_TABLE_LAYOUT "\
domain char(67) not null,\
cur_users int, \
level_cur int, level_max int, \
level_start0 int, level_start1 int, level_start2 int, \
level_end0 int, level_end1 int, level_end2 int, \
level_mod0 int, level_mod1 int, level_mod2 int, \
level_index0 int , level_index1 int, level_index2 int, the_dir char(156), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
unique index (domain)"

#define DIR_CONTROL_SELECT "\
cur_users, \
level_cur, level_max, \
level_start0, level_start1, level_start2, \
level_end0, level_end1, level_end2, \
level_mod0, level_mod1, level_mod2, \
level_index0, level_index1, level_index2, the_dir"

#ifdef VALIAS
#define VALIAS_TABLE_LAYOUT "\
alias  char(40) not null, \
domain char(67) not null, \
valias_line char(190) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
unique index(alias, domain, valias_line), index (alias, domain)"
#endif

#define MGMT_TABLE_LAYOUT "\
user  char(32) not null, \
pass char(128) not null, \
pw_uid int not null, \
pw_gid int not null, \
lastaccess int not null, \
lastupdate int not null, \
day char(2) not null, \
attempts int not null, \
status char(2) not null, \
zztimestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
unique index(user)"

#ifdef ENABLE_MYSQL_LOGGING
#define VLOG_TABLE_LAYOUT "\
id bigint primary key auto_increment, \
user char(40), \
passwd char(28), \
domain char(67), \
logon char(32), \
remoteip char(18), \
message varchar(254), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP, \
error int, \
index user_idx (user), \
index domain_idx (domain), \
index remoteip_idx (remoteip), \
index error_idx (error), \
index message_idx (message)"
#endif

#define BULKMAIL_TABLE_LAYOUT "\
emailid char(107) not null, \
filename char(64) not null, \
zztimestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
primary key(emailid, filename)"

#define LARGE_INSERT "insert low_priority into  %s \
( pw_name, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell ) \
values \
( \"%s\", \"%s\", %d, 0, \"%s\", \"%s\", \"%s\" )"

#define SMALL_INSERT "insert low_priority into  %s \
( pw_name, pw_domain, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell ) \
values \
( \"%s\", \"%s\", \"%s\", %d, 0, \"%s\", \"%s\", \"%s\" )"

#define LARGE_SELECT "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, \
pw_gecos, pw_dir, pw_shell from %s where pw_name = \"%s\""

#define SMALL_SELECT "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, \
pw_gecos, pw_dir, pw_shell from %s where pw_name = \"%s\" \
and pw_domain = \"%s\""

#define LARGE_GETALL "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, pw_gecos, \
pw_dir, pw_shell from %s"

#define SMALL_GETALL "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, \
pw_gecos, pw_dir, pw_shell from %s where pw_domain = \"%s\""

#define LARGE_SETPW "update low_priority %s set pw_passwd = \"%s\", \
pw_uid = %d, pw_gid = %d, pw_gecos = \"%s\", pw_dir = \"%s\", \
pw_shell = \"%s\" where pw_name = \"%s\""

#define SMALL_SETPW "update low_priority %s set pw_passwd = \"%s\", \
pw_uid = %d, pw_gid = %d, pw_gecos = \"%s\", pw_dir = \"%s\", \
pw_shell = \"%s\" where pw_name = \"%s\" and pw_domain = \"%s\""

#define FSTAB_TABLE_LAYOUT "\
filesystem char(64) not null, \
host char(64) not null, \
status int not null, \
max_users bigint not null, \
cur_users bigint not null, \
max_size bigint not null, \
cur_size bigint not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key (filesystem, host), index status (status)"

#ifdef VFILTER
#define FILTER_TABLE_LAYOUT "\
emailid char(107) not null, \
filter_no smallint not null, \
filter_name char(32) not null, \
header_name smallint not null, \
comparision tinyint not null, \
keyword char(64) not null, \
destination char(156) not null, \
bounce_action char(64) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(emailid, filter_no), unique index (emailid, header_name, comparision, keyword, destination)"
#endif

#define MAILING_LIST_TABLE_LAYOUT "\
emailid char(107) not null, \
filter_no smallint not null, \
mailing_list char(64) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(emailid, mailing_list), index emailid (emailid, filter_no)"

#ifdef ENABLE_DOMAIN_LIMITS
#define LIMITS_TABLE_LAYOUT " \
domain                   CHAR(67) NOT NULL, \
domain_expiry            INT(10) NOT NULL DEFAULT -1, \
passwd_expiry            INT(10) NOT NULL DEFAULT -1, \
maxpopaccounts           INT(10) NOT NULL DEFAULT -1, \
maxaliases               INT(10) NOT NULL DEFAULT -1, \
maxforwards              INT(10) NOT NULL DEFAULT -1, \
maxautoresponders        INT(10) NOT NULL DEFAULT -1, \
maxmailinglists          INT(10) NOT NULL DEFAULT -1, \
diskquota                BIGINT UNSIGNED NOT NULL DEFAULT 0, \
maxmsgcount              BIGINT UNSIGNED NOT NULL DEFAULT 0, \
defaultquota             BIGINT NOT NULL DEFAULT 0, \
defaultmaxmsgcount       BIGINT UNSIGNED NOT NULL DEFAULT 0, \
disable_pop              TINYINT(1) NOT NULL DEFAULT 0, \
disable_imap             TINYINT(1) NOT NULL DEFAULT 0, \
disable_dialup           TINYINT(1) NOT NULL DEFAULT 0, \
disable_passwordchanging TINYINT(1) NOT NULL DEFAULT 0, \
disable_webmail          TINYINT(1) NOT NULL DEFAULT 0, \
disable_relay            TINYINT(1) NOT NULL DEFAULT 0, \
disable_smtp             TINYINT(1) NOT NULL DEFAULT 0, \
perm_account             TINYINT(2) NOT NULL DEFAULT 0, \
perm_alias               TINYINT(2) NOT NULL DEFAULT 0, \
perm_forward             TINYINT(2) NOT NULL DEFAULT 0, \
perm_autoresponder       TINYINT(2) NOT NULL DEFAULT 0, \
perm_maillist            TINYINT(4) NOT NULL DEFAULT 0, \
perm_quota               TINYINT(2) NOT NULL DEFAULT 0, \
perm_defaultquota        TINYINT(2) NOT NULL DEFAULT 0, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
primary key(domain)"
#endif

#define ATRN_MAP_LAYOUT "\
pw_name char(40) not null, \
pw_domain char(67) not null, \
domain_list char(67), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
unique index atrnmap (pw_name, pw_domain, domain_list)"

#define PRIV_CMD_LAYOUT "\
user        char(32) not null, \
program     char(64) not null, \
cmdswitches char(128), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(user, program)"

#endif /*- #ifdef USE_MYSQL */

#define ATCHARS                 "@%:"
#define BUFF_SIZE               300
#define AUTH_SIZE               300
#define FILE_SIZE               156
#define MSG_BUF_SIZE            8192
#define MAX_BUFF                300
#define QUOTA_BUFLEN            20
#define SQL_BUF_SIZE            1024
#define USE_POP                 0x00
#define USE_APOP                0x01
#define ADD_FLAG                2
#define PWD_FLAG                3
#define DEL_FLAG                4
#define MAX_DOMAINNAME          100

#define BOUNCE_ALL              "bounce-no-mailbox"
#define DELETE_ALL              "delete"

/* gid flags */
#define NO_PASSWD_CHNG          0x01
#define NO_POP                  0x02
#define NO_WEBMAIL              0x04
#define NO_IMAP                 0x08
#define BOUNCE_MAIL             0x10
#define NO_RELAY                0x20
#define NO_DIALUP               0x40
#define QA_ADMIN                0x80
#define V_OVERRIDE              0x100
#define NO_SMTP                 0x200
#define V_USER0                 0x400
#define V_USER1                 0x800
#define V_USER2                 0x1000
#define V_USER3                 0x2000

/* modes for indimail dirs, files and qmail files */
#define OVERQUOTA_MAILSIZE      1000
#define MAILCOUNT_LIMIT         1500 /*- Allow 1500 incoming mails */
#define MAILSIZE_LIMIT          10485760 /*- Allow 10MB incoming mails */
#define INDIMAIL_UMASK          0077
#define INDIMAIL_TCPRULES_UMASK 0022
#define INDIMAIL_DIR_MODE       0750
#define INDIMAIL_QMAIL_MODE     0644
#define BULK_MAILDIR            "bulk_mail"
#define WELCOMEMAIL             "1welcome.txt,all"
#define ACTIVATEMAIL            "activate.txt"
#define MIGRATEUSER             "/usr/local/bin/migrateuser"
#define MIGRATEFLAG             "Qmail.txt"
#define TOKENS                  " \t"

/* error return codes */
#define VA_SUCCESS              0
#define SOCKBUF                 32768 /*- Buffer size used in Monkey service -*/
#define MAXSLEEP                0
#define MAXNOBUFRETRY           60 /*- Defines maximum number of ENOBUF retries -*/
#define SELECTTIMEOUT           30 /*- secs after which select will timeout -*/
#if !defined(INADDR_NONE) && defined(sun)
#define INADDR_NONE             0xffffffff /*- should be in <netinet/in.h> -*/
#endif
#define NULL_REMOTE_IP          "0.0.0.0"

/*- Definitions for Bigdir */
#define MAX_DIR_LEVELS          3
#define MAX_USERS_PER_LEVEL     100
#define MAX_DIR_NAME            300
#define MAX_DIR_LIST            62

typedef struct
{
	int             level_cur;
	int             level_max;
	int             level_start[MAX_DIR_LEVELS];
	int             level_end[MAX_DIR_LEVELS];
	int             level_mod[MAX_DIR_LEVELS];
	/*- current spot in dir list */
	int             level_index[MAX_DIR_LEVELS];
	unsigned long   cur_users;
	char            the_dir[MAX_DIR_NAME];
} vdir_type;
extern vdir_type vdir;
extern char    *rfc_ids[];

#ifdef ENABLE_DOMAIN_LIMITS
/* permissions for non-postmaster admins */
#define VLIMIT_DISABLE_CREATE 0x01
#define VLIMIT_DISABLE_MODIFY 0x02
#define VLIMIT_DISABLE_DELETE 0x04

#define VLIMIT_DISABLE_ALL (VLIMIT_DISABLE_CREATE|VLIMIT_DISABLE_MODIFY|VLIMIT_DISABLE_DELETE)
#define VLIMIT_DISABLE_BITS 3

struct vlimits {
      /* max service limits */
      long      domain_expiry;
      long      passwd_expiry;
      int       maxpopaccounts;
      int       maxaliases;
      int       maxforwards;
      int       maxautoresponders;
      int       maxmailinglists;

      /* quota & message count limits */
      umdir_t   diskquota;
      umdir_t   maxmsgcount;
      mdir_t    defaultquota;
      umdir_t   defaultmaxmsgcount;

      /* the following are 0 (false) or 1 (true) */
      short     disable_pop;
      short     disable_imap;
      short     disable_dialup;
      short     disable_passwordchanging;
      short     disable_webmail;
      short     disable_relay;
      short     disable_smtp;

      /* the following permissions are for non-postmaster admins */
      short     perm_account;
      short     perm_alias;
      short     perm_forward;
      short     perm_autoresponder;
      short     perm_maillist;
      short     perm_maillist_users;
      short     perm_maillist_moderators;
      short     perm_quota;
      short     perm_defaultquota;
};
#endif

/*- Fifo Server Definitions */
#define USER_QUERY   1
#define RELAY_QUERY  2
#define PWD_QUERY    3
#ifdef CLUSTERED_SITE
#define HOST_QUERY   4
#endif
#define ALIAS_QUERY  5
#define LIMIT_QUERY  6
#define DOMAIN_QUERY 7

#define MAX_LINK_COUNT 32000

int             timeoutread(int, int, char *, int);
int             timeoutwrite(int, int, char *, int);
int             pipe_exec(char **, char *, int);
int             vadddomain(char *, char *, char *, uid_t, gid_t, int);
int             vaddaliasdomain(char *, char *);
int             is_alias_domain(char *);
int             vdeldomain(char *);
int             vadduser(char *, char *, char *, char *, char *, char *, int, int, int);
int             vdeluser(char *, char *, int);
int             vrenameuser(char *, char *, char *, char *);
int             parse_email(char *, char *, char *, int);
int             vpasswd(char *, char *, char *, int);
int             vsetuserquota(char *, char *, char *);
char           *make_user_dir(char *, char *, uid_t, gid_t, int);
char           *maildir_to_email(char *, char *);
void            vset_default_domain(char *);
int             vadddotqmail(char *, char *, ...);
int             vdeldotqmail(char *, char *);
char           *vget_real_domain(char *);
int             vmake_maildir(char *, uid_t, gid_t, char *);
int             vsqwebmail_pass(char *, char *, uid_t, gid_t);
int             open_smtp_relay(char *, char *);
char           *get_Mplexdir(char *, char *, int, uid_t, gid_t);
int             Login_Tasks(struct passwd *, const char *, char *);
int             Check_Login(const char *, const char *, const char *);
int             bulk_mail(const char *, const char *, const char *);
int             RemoteBulkMail(const char *, const char *, const char *);
int             CopyEmailFile(const char *, char *, const char *, char *, char *, char *, int, int, long);
int             GetIndiId(uid_t *, gid_t *);
int             valiasinfo(char *, char *);
int             vuserinfo(char *, char *, char *, int, int, int, int, int, int, int, int, int, int, int);
int             isvirtualdomain(char *);
char            randltr(void);
void            makesalt(char *, int);
int             mkpasswd3(char *, char *, int);
char           *vgetpasswd(char *);
int             vdelfiles(char *, char *, char *);
int             add_domain_assign(char *, char *, uid_t, gid_t);
int             del_control(char *);
int             add_control(char *, char *);
int             del_domain_assign(char *, char *, gid_t, gid_t);
int             remove_line(char *, char *, int, mode_t);
int             signal_process(char *, int);
int             update_newu(void);
int             add_user_assign(char *, char *);
int             del_user_assign(char *, char *);
void            lowerit(char *);
int             update_file(char *, char *, mode_t);
int             count_rcpthosts(void);
int             compile_morercpthosts(void);
struct passwd  *vgetent(FILE *);
int             r_mkdir(char *, mode_t, uid_t, gid_t);
int             r_chown(char *, uid_t, gid_t);
char           *vget_assign(char *, char *, int, uid_t *, gid_t *);
char           *autoturn_dir(char *);
int             fappend(char *, char *, char *, mode_t, uid_t, gid_t);
int             purge_files(char *, int);
char           *genpass(int);
int             user_over_quota(char *, char *, int);
mdir_t          count_dir(char *, mdir_t *);
char           *maildir_to_domain(char *);
int             FifoCreate(char *);
int             pw_comp(unsigned char *, unsigned char *, unsigned char *, unsigned char *, int);
void            hmac_md5(u8 *, int, u8 *, int, u8 *);
void            hmac_sha1(u8 *, size_t, u8 *, size_t, u8 *);
void            hmac_sha256(const unsigned char *, size_t, const unsigned char *, size_t, void *);
void            hmac_ripemd(u8 *, size_t, u8 *, size_t, u8 *);
int             digest_md5(char *, unsigned char *, unsigned char *, unsigned char *);
char           *in_crypt(const char *, const char *);
struct passwd  *copyPwdStruct(struct passwd *);
int             passwd_policy(char *);
char           *addressToken(char *);
mdir_t          get_message_size();
int             deliver_mail(char *, mdir_t, char *, uid_t, gid_t, char *, mdir_t *, mdir_t *);
int             makeseekable(FILE *);
int             update_quota(char *, mdir_t);
int             MailQuotaWarn(char *, char *, char *, char *);
int             ScanDir(FILE *, char *, int, mdir_t *, mdir_t *, mdir_t *, mdir_t *, mdir_t *, mdir_t *);
int             MoveFile(const char *, const char *);
char           *open_big_dir(char *, char *, char *);
int             close_big_dir(char *, char *, uid_t, gid_t);
char           *next_big_dir(uid_t uid, gid_t gid, int);
char           *inc_dir(vdir_type *, int);
int             read_dir_control(vdir_type *);
int             write_dir_control(vdir_type *, uid_t, gid_t);
int             vread_dir_control(char *, vdir_type *, char *);
int             vwrite_dir_control(char *, vdir_type *, char *, uid_t, gid_t);
int             vcreate_dir_control(char *, char *);
int             vdel_dir_control(char *);
int             inc_dir_control(vdir_type *, int);
int             dec_dir_control(char *, char *, char *, uid_t, gid_t);
void            init_dir_control(vdir_type *);
unsigned long   print_control(char *, char *, int, int);
int             vread_dir_control_cdb(char *, vdir_type *, char *);
int             vwrite_dir_control_cdb(char *, vdir_type *, char *, uid_t, gid_t);
int             vdel_dir_control_cdb(char *, char *);
char           *backfill(char *, char *, char *, int);
char           *dc_filename(char *, char *, char *);
char           *dc_filename_cdb(char *, char *);
void            SetSiteSize(int);
int             vauth_adddomain(char *);
char           *vauth_adduser(char *, char *, char *, char *, char *, char *, int, int);
struct passwd  *vauth_getpw(char *, char *);
int             vauth_deldomain(char *);
int             vauth_deluser(char *, char *);
int             vauth_setquota(char *, char *, char *);
int             vauth_vpasswd(char *, char *, char *, int);
struct passwd  *vauth_getall(char *, int, int);
int             vauth_setpw(struct passwd *, char *);
int             vmkpasswd(char *domain);
void            vclose();
char           *GetPrefix(char *, char *);
char           *Dirname(char *);
char           *pathToFilesystem(char *);
struct passwd  *vauth_multi(char *, char *);
int             scopy(char *, const char *, const int);
int             scat(char *, const char *, const int);
int             slen(const char *);
int             sstrchar(char *, char);
int             sstrcmp(char *, char *);
int             sstrncmp(char *, char *, int);
int             CreateDomainDirs(char *, uid_t, gid_t);
char           *findhost(char *, int);
char           *findmdahost(char *, int *);
int             is_distributed_domain(char *);
void            getversion(char *);
char           *no_of_days(time_t);
char           *get_local_ip();
char           *get_local_hostid();
int             get_smtp_service_port(char *, char *, char *);
int             GetSmtproute(char *);
int             get_smtp_qmtp_port(char *, char *, int);
int             qmail_remote(char *, char *);
int             qmail_tmda(char *, int *);
int             error_temp(int);
void            vlog(int , char *, char *, char *, char *, char *, char *);
long            trashpurge(char *);
long            mailboxpurge(char *, char *, long, int);
long            Delunreadmails(char *, int, int);
int             is_already_running(char *);
char          **MakeArgs(char *);
void            FreeMakeArgs(char **);
int             runcmmd(char *, int);
int             pwcomp(struct passwd *, struct passwd *);
int             proxylogin(char **, char *, char *, char *, char *, char *, int);
int             AuthModuser(int, char **, unsigned, unsigned);
void            imapd_capability();
void            pop3d_capability();
int             isvalid_domain(char *);
mdir_t          parse_quota(char *, mdir_t *);
int             remove_quotes(char *);
char           *get_localtime();
char           *memstore(char *, int, int *);
int             add_vacation(char *, char *);
void            SendWelcomeMail(char *, char *, char *, int, char *);
int             isnum(char *);
int             IsPrime(unsigned int);
int             setsockbuf(int, int, int);
int             tcpopen(char *, char *, int);
int             udpopen(char *, char *);
int             tcpbind(char *, char *, int);
int             monkey(char *, char *, char *, int);
int             sockwrite(int, char *, int);
int             sockread(int, char *, int);
int             islocalif(char *);
char           *GetIpaddr();
char           *getremoteip();
char           *replacestr(char *, char *, char *);
char           *getactualpath(char *);
int             skip_system_files(char *);
int             wildmat(char *, char *);
int             atrn_access(char *, char *);
int             loadbalance(int);
void            getEnvConfigStr(char **, char *, char *);
void            getEnvConfigInt(int *, char *, long);
void            getEnvConfigLong(long *, char *, long);
#ifdef HAVE_STDARG_H
int             filewrt     __P((int, char *, ...));
int             post_handle __P((const char *, ...));
char           *error_stack __P((FILE *, const char *, ...));
#else
int             filewrt     ();
int             post_handle ();
char           *error_stack ();
#endif

#ifdef USE_MAILDIRQUOTA
mdir_t          check_quota(char *, mdir_t *);
mdir_t          recalc_quota(char *, mdir_t *, mdir_t, mdir_t, int);
#else
mdir_t          check_quota(char *);
mdir_t          recalc_quota(char *, int);
#endif /*- #ifdef USE_MAILDIRQUOTA */

#if defined(POP_AUTH_OPEN_RELAY)
int             update_rules(int);
int             skip_relay(char *);
int             vopen_smtp_relay(char *, char *);
int             vupdate_rules(int);
int             vclear_open_smtp(time_t, int);
#endif

#ifdef IP_ALIAS_DOMAINS
int             vget_ip_map(char *, char *, int);
int             vadd_ip_map(char *, char *);
int             vdel_ip_map(char *, char *);
int             vupd_ip_map(char *, char *);
int             vshow_ip_map(int, char *, char *, char *);
int             host_in_locals(char *);
#endif

#ifdef FILE_LOCKING
int             getDbLock(char *, char);
int             delDbLock(int, char *, char);
int             readPidLock(char *, char);
int             lockcreate(char *, char);
int             get_write_lock(int);
int             ReleaseLock(int);
int             RemoveLock(char *, char);
#ifdef USE_SEMAPHORES
int             lockremove(int);
#endif
#endif

int             vauth_open(char *);

#ifdef ENABLE_DOMAIN_LIMITS
int             vget_limits(char *, struct vlimits *);
int             vset_limits(char *, struct vlimits *);
int             vdel_limits(char *);
int             vlimits_get_flag_mask(struct vlimits *);
#endif

#ifdef USE_MYSQL
char           *layout(char *);
int             lockTable(int, char *, int);
int             tableToMatrix(int, char *, int);
int             matrixToTable(int, char *, char *, char *, char *);
int             create_table(int, char *, char *);
long            count_table(char *);
char           *vauth_munch_domain(char *);
void            vauth_init(int, MYSQL *);
#ifdef HAVE_STDARG_H
char           *mysql_perror __P((char *, ...));
#else
char           *mysql_perror ();
#endif
int             OpenDatabases();
int             connect_db(DBINFO **, MYSQL **);
void            close_db();
int             UserInLookup(char *);
#ifdef ENABLE_DOMAIN_LIMITS
int             VlimitInLookup(char *, struct vlimits *);
#endif
int             RelayInLookup(char *, char *);
struct passwd  *PwdInLookup(char *);
char           *AliasInLookup(char *);
int             relay_select(char *, char *);
char           *SqlServer(char *, char *);
char           *MdaServer(char *, char *);
MYSQL         **mdaMysqlConnect(char *, char *);
DBINFO        **LoadDbInfo_TXT(int *);
int             writemcdinfo(DBINFO **, time_t);
char          **LoadBMF(int *, char *);
int             UpdateSpamTable(char *);
void           *inquery(char, char *, char *);
int             vauthOpen_user(char *, int);
int             ProcessInFifo(int);
struct passwd  *strToPw(char *, int);
long            bulletin(char *, char *);
int             vfstabNew(char *, long, long);
char           *vfstab_select(char *, int *, long *, long *, long *, long *);
int             vfstab_insert(char *, char *, int, long, long);
int             vfstab_delete(char *, char *);
int             vfstab_update(char *, char *, long, long, int);
int             vfstab_status(char *, char *, int);
int             fstabChangeCounter(char *, char *, long, long);
char           *getFreeFS();
char          **getindimail(char *, int, char **, unsigned long *);
char           *mysql_escape(char *);
char          **getAddressBook(char *);
char           *vshow_atrn_map(char **, char **);
int             vadd_atrn_map(char *, char *, char *);
int             vdel_atrn_map(char *, char *, char *);
int             vupd_atrn_map(char *, char *, char *, char *);
int             set_mysql_options(MYSQL *, char *, char *, unsigned int *);
int             int_mysql_options(MYSQL *, enum mysql_option, const void *);
char           *error_mysql_options_str(unsigned int);
int             vauth_renamedomain(char *, char *, char *);
int             vset_lastdeliver(char *, char *, int);
int             disable_mysql_escape(int);
int             loadIgnoreList(char *);
int             spamReport(int, char *);
int             readLogFile(char *, int, int);
int             isIgnored(char *);
maddr          *insertAddr(int, char *);
unsigned int    hash(char *);
void            print_list(int);
#ifdef QUERY_CACHE
void            vauth_getpw_cache(char);
void            vauth_get_realdomain_cache(char);
void            vget_assign_cache(char);
void            vget_real_domain_cache(char);
#endif /*- #ifdef QUERY_CACHE */
#ifdef CLUSTERED_SITE
int             open_central_db(char *);
int             open_master();
int             open_relay_db();
void            vclose_cntrl();
int             is_user_present(char *, char *);
int             addusercntrl(char *, char *, char *, char *, int);
int             delusercntrl(char *, char *, int);
int             updusercntrl(char *, char *, char *, int);
int             hostcntrl_select(char *, char *, time_t *, char *, int);
char          **hostcntrl_select_all();
int             vauth_updateflag(char *, char *, int);
struct passwd  *vauth_getflags(char *, int);
int             cntrl_clearaddflag(char *, char *, char *);
int             cntrl_cleardelflag(char *, char *);
int             cntrl_clearpwdflag(char *, char *, char *);
int             vsmtp_insert(char *, char *, char *, int);
int             vsmtp_delete(char *, char *, char *, int);
int             vsmtp_update(char *, char *, char *, int, int);
int             vsmtp_delete_domain(char *);
char           *vsmtp_select(char *, int *);
int             vauth_delaliasdomain(char *);
int             vauth_insertaliasdomain(char *, char *);
char           *vauth_get_realdomain(char *);
int             mysqlquery(MYSQL *, char *);
char           *vauth_getipaddr(char *);
char           *vauth_gethostid(char *);
char           *vhostid_select();
int             vhostid_insert(char *, char *);
int             vhostid_update(char *, char *);
int             vhostid_delete(char *);
char           *mgmtgetpass(char *, int *);
int             mgmtpassinfo(char *, int);
int             mgmtsetpass(char *, char *, uid_t, gid_t, time_t, time_t);
int             mgmtadduser(char *, char *, uid_t, gid_t, time_t, time_t);
int             mgmtlist();
int             getpassword(char *);
int             setpassword(char *);
int             updateLoginFailed(char *);
int             ChangeLoginStatus(char *, int);
int             isDisabled(char *);
int             dbinfoAdd(char *, int, char *, char *, int, char *, char *, char *);
int             dbinfoDel(char *, char *);
int             dbinfoUpdate(char *, int, char *, char *, int, char *, char *, char *);
int             dbinfoSelect(char *, char *, char *, int);
char           *vpriv_select(char **, char **);
int             vpriv_insert(char *, char *, char *);
int             vpriv_delete(char *, char *);
int             vpriv_update(char *, char *, char *);
int             checkPerm(char *, char *, char **);
gid_t          *grpscan(char *, int *);
int             setuserid(char *);
int             setuser_privileges(uid_t, gid_t, char *);
#ifdef QUERY_CACHE
void            findhost_cache(char);
void            is_user_present_cache(char);
void            cntrl_clearaddflag_cache(char);
void            cntrl_cleardelflag_cache(char);
void            is_distributed_domain_cache(char);
#endif /*- #ifdef QUERY_CACHE */
#endif /*- #ifdef CLUSTERED_SITE */
#endif /*- #ifdef USE_MYSQL */

#ifdef ENABLE_AUTH_LOGGING
#define FROM_ACTIVE_TO_INACTIVE 0
#define FROM_INACTIVE_TO_ACTIVE 1
int             vset_lastauth(char *, char *, char *, char *, char *, int);
int             vlogauth(struct passwd *, char *, char *, char *, char *, int);
time_t          vget_lastauth(struct passwd *, char *, int, char *);
int             vauth_active(struct passwd *, char *, int);
char          **getlastauth(char *, int, int, int, unsigned long *);
int             vquota_select(char *, char *, mdir_t *, time_t *, int);
#endif

#ifdef ENABLE_MYSQL_LOGGING
int             logmysql(int, char *, char *, char *, char *, char *, char *);
#endif

#ifdef VALIAS
long            valiasCount(char *, char *);
char           *valias_select(char *, char *);
char           *valias_track(char *, char *, char *, int);
char           *valias_select_all(char *, char *, int);
int             valias_insert(char *, char *, char *, int);
int             valias_update(char *, char *, char *, char *);
int             valias_delete(char *, char *, char *);
int             valias_delete_domain(char *);
#endif /*- #ifdef VALIAS */

#ifdef VFILTER
char          **headerList();
int             vfilter_delete(char *, int);
int             vfilter_display(char *, int, int *, char *, int *, int *, char *, char *, int *, char *);
void            format_filter_display(int, int, char *, char *, int, int, char *, char *, char *, int);
int             vfilter_filterNo(char *);
int             vfilter_insert(char *, char *, int, int, char *, char *, int, char *);
int             vfilter_mlistOpt(char *, int);
int             vfilter_select(char *, int *, char *, int *, int *, char *, char *, int *, char *);
int             vfilter_update(char *, int, int, int, char *, char *, int, char *);
int             storeHeader(struct header ***, struct header_t *);
char          **getmailingList(char *, int);
int             mlist_filterupdate(char *, int);
int             mlist_insert(char *, int, char **);
int             mlist_update(char *, char *, char *);
int             mlist_delete(char *, char *);
int             mlist_filterno(char *);
void            parseAddress(struct header_t *, char *);
int             is_mailing_list(char *, char *);
void            rewindAddrToken();
int             check_group(gid_t);
#endif

extern int      create_flag;
extern int      site_size;
extern int      encrypt_flag;
extern int      fdm;
extern int      is_open;
extern int      delayed_insert;
extern char     mysql_host[];
extern char    *indi_port;
extern int      OptimizeAddDomain;
extern int      userNotFound;
extern mdir_t   CurBytes, CurCount;
extern char    *default_table, *inactive_table, *cntrl_table;
extern char     dirlist[];
extern uid_t    indimailuid;
extern gid_t    indimailgid;
extern int      is_inactive;
extern int      is_overquota;
extern int      use_etrn;
extern int      use_vfilter;
extern int      verbose;

#endif
