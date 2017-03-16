/*
 * $Log: indiversion.c,v $
 * Revision 2.144  2017-03-13 13:45:13+05:30  Cprogrammer
 * replaced INDIMAILDIR, QMAILDIR with PREFIX
 *
 * Revision 2.143  2017-01-04 21:46:37+05:30  Cprogrammer
 * moved cputime, bogofilter-qfe to libexecdir
 *
 * Revision 2.142  2016-06-21 13:31:45+05:30  Cprogrammer
 * corrected executable paths
 *
 * Revision 2.141  2016-06-09 15:32:15+05:30  Cprogrammer
 * added check_group() function
 *
 * Revision 2.140  2016-06-08 15:28:08+05:30  Cprogrammer
 * fixed locating programs in aditional directories like sbin, libexec
 *
 * Revision 2.139  2016-05-25 09:02:02+05:30  Cprogrammer
 * use SYSCONFDIR for qmailprog.list, LIBEXECDIR for libexec programs
 *
 * Revision 2.138  2015-08-26 11:08:06+05:30  Cprogrammer
 * added isPrime() function
 *
 * Revision 2.137  2014-01-30 14:55:25+05:30  Cprogrammer
 * added getversion_setuserid_c()
 * workaround for git cloberring  field
 *
 * Revision 2.136  2011-12-10 14:56:44+05:30  Cprogrammer
 * added hmac_256()
 *
 * Revision 2.135  2011-12-05 13:46:41+05:30  Cprogrammer
 * added hmac_md5() and md5_crypt() functions
 *
 * Revision 2.134  2011-11-09 19:44:48+05:30  Cprogrammer
 * remvoed parseAddress(), storeHeader() from libindimail
 *
 * Revision 2.133  2011-10-28 14:19:21+05:30  Cprogrammer
 * added digest_md5()
 *
 * Revision 2.132  2011-10-27 14:31:46+05:30  Cprogrammer
 * added hmac_sha1(), hmac_ripemd() functions
 *
 * Revision 2.131  2011-08-05 18:12:47+05:30  Cprogrammer
 * ismaildup is a bin program
 *
 * Revision 2.130  2011-06-30 20:40:09+05:30  Cprogrammer
 * added ismaildup()
 *
 * Revision 2.129  2011-04-08 17:26:38+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.128  2011-04-02 14:00:56+05:30  Cprogrammer
 * fix for getversion_proxylogin() function which has been moved out of libindimail
 *
 * Revision 2.127  2010-07-04 14:39:39+05:30  Cprogrammer
 * removed open_smtp_relay
 *
 * Revision 2.126  2010-04-13 08:15:41+05:30  Cprogrammer
 * renamed vmoddomlimits to vlimit
 *
 * Revision 2.125  2010-04-11 22:55:01+05:30  Cprogrammer
 * added VlimitInLookup()
 *
 * Revision 2.124  2010-03-06 14:55:20+05:30  Cprogrammer
 * added program sslerator
 *
 * Revision 2.123  2010-02-16 13:07:25+05:30  Cprogrammer
 * added post_handle() function
 *
 * Revision 2.122  2009-11-23 11:41:09+05:30  Cprogrammer
 * added mode for libexec programs
 *
 * Revision 2.121  2009-11-23 11:33:36+05:30  Cprogrammer
 * added vserverinfo, programs in libexec directory
 *
 * Revision 2.120  2009-08-11 17:00:11+05:30  Cprogrammer
 * removed auth_admin() and adminCmmd()
 *
 * Revision 2.119  2009-08-05 14:36:31+05:30  Cprogrammer
 * added vsetpass
 *
 * Revision 2.118  2009-05-28 08:50:21+05:30  Cprogrammer
 * added tls-cert-check
 *
 * Revision 2.117  2009-01-25 12:16:18+05:30  Cprogrammer
 * added hashtable
 *
 * Revision 2.116  2009-01-12 10:37:12+05:30  Cprogrammer
 * added function backfill()
 *
 * Revision 2.115  2008-10-29 12:11:40+05:30  Cprogrammer
 * added getversion_unsetenv()
 *
 * Revision 2.114  2008-10-21 21:29:53+05:30  Cprogrammer
 * added getversion_mysql_query_c()
 *
 * Revision 2.113  2008-09-17 09:36:56+05:30  Cprogrammer
 * removed cdbmake programs as they are automatically picked up from qmailprog.list
 *
 * Revision 2.112  2008-09-16 10:02:46+05:30  Cprogrammer
 * added cdbmake programs
 *
 * Revision 2.111  2008-09-14 19:47:24+05:30  Cprogrammer
 * removed dc_filename
 *
 * Revision 2.110  2008-09-12 10:00:15+05:30  Cprogrammer
 * moved mgmtpass to sbin
 *
 * Revision 2.109  2008-09-12 09:57:15+05:30  Cprogrammer
 * added funcation in_crypt ()
 *
 * Revision 2.108  2008-09-11 22:49:27+05:30  Cprogrammer
 * added md5_crypt, sha256_crypt, sha512_crypt
 *
 * Revision 2.107  2008-09-08 09:44:43+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.106  2008-08-29 14:01:58+05:30  Cprogrammer
 * removed genpass()
 *
 * Revision 2.105  2008-08-02 09:07:23+05:30  Cprogrammer
 * remove function verror()
 * added function error_stack()
 *
 * Revision 2.104  2008-07-28 23:44:20+05:30  Cprogrammer
 * added copyemail, mail_report
 *
 * Revision 2.103  2008-07-28 23:22:48+05:30  Cprogrammer
 * removed licgen
 *
 * Revision 2.102  2008-07-18 19:09:06+05:30  Cprogrammer
 * renamed getversion_variables_c to getversion_indivariables_c
 *
 * Revision 2.101  2008-06-25 15:39:43+05:30  Cprogrammer
 * removed sstrcmp and sstrncmp
 *
 * Revision 2.100  2008-06-21 16:04:39+05:30  Cprogrammer
 * moved vfstab to sbin
 *
 * Revision 2.99  2008-06-05 20:13:08+05:30  Cprogrammer
 * moved myslsave, svctool, cputime to sbin
 * correct ident for shell scripts
 *
 * Revision 2.98  2008-06-05 16:20:17+05:30  Cprogrammer
 * moved programs to sbin
 *
 * Revision 2.97  2008-05-28 22:49:08+05:30  Cprogrammer
 * added execmysql program
 *
 * Revision 2.96  2008-05-28 16:36:02+05:30  Cprogrammer
 * removed vcdir
 *
 * Revision 2.95  2008-05-28 15:35:53+05:30  Cprogrammer
 * removed ldap, sqwebmail, cdb code
 *
 * Revision 2.94  2008-05-27 22:20:12+05:30  Cprogrammer
 * added install_tables
 *
 * Revision 2.93  2005-12-29 22:50:01+05:30  Cprogrammer
 * added set_mysql_options()
 *
 * Revision 2.92  2005-12-21 09:48:30+05:30  Cprogrammer
 * added getversion_getenvConfig_c
 *
 * Revision 2.91  2005-01-22 00:41:51+05:30  Cprogrammer
 * removed vfd_copy() and vfd_move()
 *
 * Revision 2.90  2004-10-22 20:51:17+05:30  Cprogrammer
 * added qmail programs
 *
 * Revision 2.89  2004-07-12 22:48:29+05:30  Cprogrammer
 * added runcmmd()
 *
 * Revision 2.88  2004-05-22 23:48:37+05:30  Cprogrammer
 * added ipchange
 *
 * Revision 2.87  2004-05-17 00:49:35+05:30  Cprogrammer
 * added updusercntrl() and hostcntrl_select()
 * removed pwdusercntrl()
 *
 * Revision 2.86  2004-03-23 21:35:28+05:30  Cprogrammer
 * added getversion_evaluate_c
 *
 * Revision 2.85  2003-12-30 00:29:41+05:30  Cprogrammer
 * added headerlist()
 *
 * Revision 2.84  2003-12-07 00:21:35+05:30  Cprogrammer
 * added loadbalance() and count_table()
 *
 * Revision 2.83  2003-10-16 00:05:03+05:30  Cprogrammer
 * added initsvc
 *
 * Revision 2.82  2003-09-14 01:58:14+05:30  Cprogrammer
 * added function checkPerm()
 *
 * Revision 2.81  2003-09-13 23:54:53+05:30  Cprogrammer
 * added vpriv functions
 *
 * Revision 2.80  2003-07-29 14:12:46+05:30  Cprogrammer
 * renamed indimail to cindimail
 *
 * Revision 2.79  2003-07-18 19:19:25+05:30  Cprogrammer
 * added licgen
 *
 * Revision 2.78  2003-07-06 22:31:34+05:30  Cprogrammer
 * added program updatefile
 *
 * Revision 2.77  2003-07-06 14:16:08+05:30  Cprogrammer
 * added atrn_access()
 *
 * Revision 2.76  2003-07-04 11:35:19+05:30  Cprogrammer
 * added atrn functions
 *
 * Revision 2.75  2003-06-23 01:34:36+05:30  Cprogrammer
 * added controlsync and ldap utils
 *
 * Revision 2.74  2003-06-08 19:13:01+05:30  Cprogrammer
 * added wildmat()
 *
 * Revision 2.73  2003-05-26 12:58:25+05:30  Cprogrammer
 * added vgroup and ldapaddr
 *
 * Revision 2.72  2003-03-24 19:24:20+05:30  Cprogrammer
 * removed matrix functions
 *
 * Revision 2.71  2003-03-05 00:18:48+05:30  Cprogrammer
 * added vupd_ip_map.c
 *
 * Revision 2.70  2003-02-19 22:24:36+05:30  Cprogrammer
 * removed spost
 *
 * Revision 2.69  2003-02-11 23:11:18+05:30  Cprogrammer
 * added vmatrix, pwhelper
 *
 * Revision 2.68  2003-02-03 21:55:15+05:30  Cprogrammer
 * added function getversion_layout_c()
 *
 * Revision 2.67  2003-02-02 23:49:43+05:30  Cprogrammer
 * added tableToMatrix(), lockTable()
 *
 * Revision 2.66  2003-02-01 14:12:33+05:30  Cprogrammer
 * added spost
 *
 * Revision 2.65  2003-01-03 20:06:37+05:30  Cprogrammer
 * removed unecessary printing of sccsid
 *
 * Revision 2.64  2003-01-03 19:01:03+05:30  Cprogrammer
 * renamed getversion_vrenameuser_c() to getversion_renameuser_c()
 *
 * Revision 2.63  2003-01-01 10:57:32+05:30  Cprogrammer
 * removed program editdbinfo
 *
 * Revision 2.62  2003-01-01 02:36:34+05:30  Cprogrammer
 * added dbinfoUpdate() and dbinfoSelect()
 *
 * Revision 2.61  2002-12-29 19:00:58+05:30  Cprogrammer
 * added GetSmtproute()
 *
 * Revision 2.60  2002-12-27 16:41:02+05:30  Cprogrammer
 * added valiasCount()
 *
 * Revision 2.59  2002-12-21 18:29:06+05:30  Cprogrammer
 * added function getversion_indimail_settings_c()
 * corrected few compiler warnings
 *
 * Revision 2.58  2002-12-09 20:32:45+05:30  Cprogrammer
 * added testmra, dbinfoAdd() dbinfoDel()
 *
 * Revision 2.57  2002-12-08 19:04:57+05:30  Cprogrammer
 * added chowkidar
 *
 * Revision 2.56  2002-12-02 01:48:58+05:30  Cprogrammer
 * added webauth, folderlist
 *
 * Revision 2.55  2002-11-28 00:46:29+05:30  Cprogrammer
 * added getversion_filewrt()
 *
 * Revision 2.54  2002-11-24 17:51:57+05:30  Cprogrammer
 * added program postdel
 *
 * Revision 2.53  2002-10-27 21:29:52+05:30  Cprogrammer
 * removed vcreate functions
 * added function getversion_create_table_c()
 *
 * Revision 2.52  2002-10-27 19:42:05+05:30  Cprogrammer
 * added getversion_limits_c()
 *
 * Revision 2.51  2002-10-20 22:15:54+05:30  Cprogrammer
 * added getversion_vcreate_bmf_table_c(), getversion_LoadBMF_c()
 *
 * Revision 2.50  2002-10-19 18:24:40+05:30  Cprogrammer
 * added getversion_spam_c()
 *
 * Revision 2.49  2002-10-16 23:44:56+05:30  Cprogrammer
 * added skip_system_files_c()
 *
 * Revision 2.48  2002-10-14 20:57:13+05:30  Cprogrammer
 * getversion_vfilter_mlistOpt_c(), getversion_vfilter_filterNo_c(), getversion_mlist_filterupdate_c()
 *
 * Revision 2.47  2002-10-12 21:17:02+05:30  Cprogrammer
 * added getversion_deliver_mail_c()
 *
 * Revision 2.46  2002-10-11 21:48:40+05:30  Cprogrammer
 * added getversion_getAddressBook_c()
 *
 * Revision 2.45  2002-10-11 20:02:41+05:30  Cprogrammer
 * added getversion_vfilter_display_c(), getversion_mlist_c()
 *
 * Revision 2.44  2002-10-11 01:03:48+05:30  Cprogrammer
 * added getversion_is_mailing_list_c()
 *
 * Revision 2.43  2002-10-10 23:37:08+05:30  Cprogrammer
 * added getversion_parseAddress_c()
 *
 * Revision 2.42  2002-10-10 03:33:49+05:30  Cprogrammer
 * added getversion_addressToken_c()
 *
 * Revision 2.41  2002-10-09 23:24:07+05:30  Cprogrammer
 * added mailing list functions
 *
 * Revision 2.40  2002-10-09 21:08:07+05:30  Cprogrammer
 * added getversion_getmailingList_c()
 *
 * Revision 2.39  2002-10-09 18:56:25+05:30  Cprogrammer
 * added getversion_storeHeader_c()
 *
 * Revision 2.38  2002-09-30 23:48:07+05:30  Cprogrammer
 * added filter routines
 *
 * Revision 2.37  2002-09-29 22:40:12+05:30  Cprogrammer
 * added vcreate_filter_table()
 *
 * Revision 2.36  2002-09-27 13:19:23+05:30  Cprogrammer
 * added programs crc, crcdiff
 *
 * Revision 2.35  2002-09-04 12:56:54+05:30  Cprogrammer
 * added maildir_to_email()
 *
 * Revision 2.34  2002-09-01 20:00:21+05:30  Cprogrammer
 * added pipe_exec()
 *
 * Revision 2.33  2002-09-01 19:55:09+05:30  Cprogrammer
 * renamed chkpass to vchkpass
 * added systpass
 *
 * Revision 2.32  2002-08-25 22:34:25+05:30  Cprogrammer
 * added add_control(), autoturn_dir()
 *
 * Revision 2.31  2002-08-11 18:59:38+05:30  Cprogrammer
 * removed memstore
 *
 * Revision 2.30  2002-08-11 18:54:39+05:30  Cprogrammer
 * removed cntrl_clearpwdflag_c()
 *
 * Revision 2.29  2002-08-11 14:13:07+05:30  Cprogrammer
 * added program vfstab
 *
 * Revision 2.28  2002-08-11 00:27:57+05:30  Cprogrammer
 * added getFreeFS(), getactualpath(), pathToFilesystem(), vfstabNew()
 *
 * Revision 2.27  2002-08-07 19:34:43+05:30  Cprogrammer
 * added fstab routines
 *
 * Revision 2.26  2002-08-05 01:32:49+05:30  Cprogrammer
 * added mysql_escape()
 *
 * Revision 2.25  2002-07-31 18:42:23+05:30  Cprogrammer
 * added getversion_udpopen_c()
 *
 * Revision 2.24  2002-07-28 23:15:05+05:30  Cprogrammer
 * added indimail_settings()
 *
 * Revision 2.23  2002-07-25 12:27:02+05:30  Cprogrammer
 * added adminclient
 *
 * Revision 2.22  2002-07-23 09:43:18+05:30  Cprogrammer
 * added mgmtpass
 *
 * Revision 2.21  2002-07-15 18:58:21+05:30  Cprogrammer
 * added adminCmmd()
 *
 * Revision 2.20  2002-07-15 18:41:37+05:30  Cprogrammer
 * added auth_admin()
 *
 * Revision 2.19  2002-07-15 02:06:07+05:30  Cprogrammer
 * added passwd_policy(), mgmtpassfuncs()
 *
 * Revision 2.18  2002-07-11 21:17:55+05:30  Cprogrammer
 * added makeseekable
 *
 * Revision 2.17  2002-07-05 03:55:26+05:30  Cprogrammer
 * added program inlookup
 *
 * Revision 2.16  2002-07-04 00:33:56+05:30  Cprogrammer
 * reorg of order of the program list
 *
 * Revision 2.15  2002-07-03 01:18:31+05:30  Cprogrammer
 * added copyPwdStruct()
 *
 * Revision 2.14  2002-06-26 03:17:58+05:30  Cprogrammer
 * added function LdapGetpw()
 *
 * Revision 2.13  2002-05-16 01:09:03+05:30  Cprogrammer
 * added SendWelcomeMail()
 *
 * Revision 2.12  2002-05-15 01:35:37+05:30  Cprogrammer
 * removed clearopensmtp.sh, added editdbinfo
 *
 * Revision 2.11  2002-05-13 02:27:45+05:30  Cprogrammer
 * added vcreate_dbinfo_table()
 *
 * Revision 2.10  2002-05-12 01:21:35+05:30  Cprogrammer
 * added vcreate_mgmtaccess_table() and add_vacation()
 *
 * Revision 2.9  2002-05-11 00:20:09+05:30  Cprogrammer
 * replaced md5.c with md5c.c
 *
 * Revision 2.8  2002-05-06 22:25:10+05:30  Cprogrammer
 * added vrenamedomain and vrenameuser
 *
 * Revision 2.7  2002-05-05 22:23:23+05:30  Cprogrammer
 * added getversion_vauth_renamedomain()
 *
 * Revision 2.6  2002-04-28 18:33:25+05:30  Cprogrammer
 * added vrename_user();
 *
 * Revision 2.5  2002-04-15 20:46:52+05:30  Cprogrammer
 * added separator to display main program list
 *
 * Revision 2.4  2002-04-15 20:45:01+05:30  Cprogrammer
 * corrected problem with display of id for indiversion
 *
 * Revision 2.3  2002-04-15 20:41:55+05:30  Cprogrammer
 * added function Ident to print version for indimail programs
 *
 * Revision 2.2  2002-04-12 15:51:47+05:30  Cprogrammer
 * removed insert_bulletin, added bulletin
 *
 * Revision 2.1  2002-04-12 02:08:48+05:30  Cprogrammer
 * added insert_bulletin(), mdaMysqlConnect()
 *
 * Revision 1.34  2002-04-10 01:19:07+05:30  Cprogrammer
 * added timeoutio()
 *
 * Revision 1.33  2002-04-09 20:28:19+05:30  Cprogrammer
 * added AliasInLookup()
 *
 * Revision 1.32  2002-04-09 12:07:54+05:30  Cprogrammer
 * added PwdInLookup()
 *
 * Revision 1.31  2002-04-08 21:29:55+05:30  Cprogrammer
 * added relay_select()
 *
 * Revision 1.30  2002-04-08 05:17:10+05:30  Cprogrammer
 * added inquery()
 *
 * Revision 1.29  2002-04-08 03:47:19+05:30  Cprogrammer
 * added findmdahost()
 *
 * Revision 1.28  2002-04-07 13:42:23+05:30  Cprogrammer
 * added vauthOpen_user(), ProcessInFifo(), strToPw()
 *
 * Revision 1.27  2002-04-06 23:47:22+05:30  Cprogrammer
 * added 21 functions
 *
 * Revision 1.26  2002-03-18 22:40:22+05:30  Cprogrammer
 * added vauth_init()
 *
 * Revision 1.25  2002-02-24 22:06:39+05:30  Cprogrammer
 * added remove_quotes()
 *
 * Revision 1.24  2002-02-24 03:25:50+05:30  Cprogrammer
 * added function parse_quota()
 *
 * Revision 1.23  2002-02-23 20:23:53+05:30  Cprogrammer
 * added function isvalid_domain()
 *
 * Revision 1.22  2001-12-27 01:27:33+05:30  Cprogrammer
 * added argument id to getversion()
 *
 * Revision 1.21  2001-12-24 15:00:28+05:30  Cprogrammer
 * added proxylogin()
 *
 * Revision 1.20  2001-12-21 01:11:20+05:30  Cprogrammer
 * added function vauth_get_realdomain()
 *
 * Revision 1.19  2001-12-21 00:35:53+05:30  Cprogrammer
 * added functions vcreate_aliasdomain_table(), vauth_delaliasdomain(), vauth_insertaliasdomain
 *
 * Revision 1.18  2001-12-19 20:55:58+05:30  Cprogrammer
 * added pwcomp.c
 *
 * Revision 1.17  2001-12-19 16:28:42+05:30  Cprogrammer
 * added function Dirname()
 *
 * Revision 1.16  2001-12-13 00:32:35+05:30  Cprogrammer
 * added is_already_running and MakeArgs
 *
 * Revision 1.15  2001-12-11 11:58:12+05:30  Cprogrammer
 * removed quotausercntrl()
 *
 * Revision 1.14  2001-12-11 11:32:40+05:30  Cprogrammer
 * added open_master and quotausercntrl
 *
 * Revision 1.13  2001-12-09 23:55:46+05:30  Cprogrammer
 * added vsmtp functions
 *
 * Revision 1.12  2001-12-08 17:44:54+05:30  Cprogrammer
 * added islocalif()
 *
 * Revision 1.11  2001-12-08 14:43:56+05:30  Cprogrammer
 * added tcpbind()
 *
 * Revision 1.10  2001-12-08 00:35:02+05:30  Cprogrammer
 * added new functions
 *
 * Revision 1.9  2001-12-03 04:17:28+05:30  Cprogrammer
 * added vlog(), trashpurge(), Delunreadmails()
 *
 * Revision 1.8  2001-12-02 20:22:30+05:30  Cprogrammer
 * added function getversion_replacestr_c();
 *
 * Revision 1.7  2001-12-02 18:47:45+05:30  Cprogrammer
 * added function valias_update() and init_dir_control()
 *
 * Revision 1.6  2001-12-01 02:15:55+05:30  Cprogrammer
 * added qmail_remote() and error_temp()
 *
 * Revision 1.5  2001-11-29 20:55:29+05:30  Cprogrammer
 * added open_relay_db()
 *
 * Revision 1.4  2001-11-29 13:28:05+05:30  Cprogrammer
 * added functions vauth_getflags, cntrl_clearaddflag, cntrl_clearpwdflag, cntrl_cleardelflag
 *
 * Revision 1.3  2001-11-28 23:01:07+05:30  Cprogrammer
 * added new functions for distributed architecture
 *
 * Revision 1.2  2001-11-24 20:37:05+05:30  Cprogrammer
 * added function getversion_no_of_days_c()
 *
 * Revision 1.1  2001-11-24 12:26:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#ifdef MAIN
#include "indimail.h"
#include <string.h>
#include <unistd.h>
#else
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#endif

#ifndef	lint
static char     sccsid[] = "$Id: indiversion.c,v 2.144 2017-03-13 13:45:13+05:30 Cprogrammer Exp mbhangui $";
#endif

void            getversion_indimail_settings_c();
void            getversion_addaliasdomain_c();
void            getversion_add_domain_assign_c();
void            getversion_adddomain_c();
void            getversion_add_user_assign_c();
void            getversion_adduser_c();
void            getversion_backfill_c();
void            getversion_bulk_mail_c();
void            getversion_Check_Login_c();
void            getversion_check_quota_c();
void            getversion_close_big_dir_c();
void            getversion_compile_morercpthosts_c();
void            getversion_CopyEmailFile_c();
void            getversion_count_dir_c();
void            getversion_count_rcpthosts_c();
void            getversion_CreateDomainDirs_c();
void            getversion_dec_dir_control_c();
void            getversion_del_control_c();
void            getversion_del_domain_assign_c();
void            getversion_check_group_c();
void            getversion_deldomain_c();
void            getversion_del_user_assign_c();
void            getversion_deluser_c();
void            getversion_fappend_c();
void            getversion_findhost_c();
void            getversion_getlastauth_c();
void            getversion_get_Mplexdir_c();
void            getversion_getpeer_c();
void            getversion_GetPrefix_c();
void            getversion_GetIndiId_c();
void            getversion_getindimail_c();
void            getversion_host_in_locals_c();
void            getversion_inc_dir_c();
void            getversion_inc_dir_control_c();
void            getversion_is_alias_domain_c();
void            getversion_is_distributed_domain_c();
void            getversion_is_user_present_c();
void            getversion_isvirtualdomain_c();
void            getversion_lockfile_c();
void            getversion_Login_Tasks_c();
void            getversion_logmysql_c();
void            getversion_lowerit_c();
void            getversion_MailQuotaWarn_c();
void            getversion_make_user_dir_c();
#ifndef HAVE_MD5_CRYPT
void            getversion_md5c_c();
#endif
#ifndef HAVE_SHA256_CRYPT
void            getversion_sha256_crypt_c();
#endif
#ifndef HAVE_SHA512_CRYPT
void            getversion_sha512_crypt_c();
#endif
void            getversion_mkpasswd3_c();
void            getversion_MoveFile_c();
void            getversion_mysql_perror_c();
void            getversion_next_big_dir_c();
void            getversion_open_big_dir_c();
void            getversion_parse_email_c();
void            getversion_passwd_c();
void            getversion_print_control_c();
void            getversion_purge_files_c();
void            getversion_makesalt_c();
void            getversion_r_chown_c();
void            getversion_recalc_quota_c();
void            getversion_remove_line_c();
void            getversion_r_mkdir_c();
void            getversion_ScanDir_c();
void            getversion_scat_c();
void            getversion_scopy_c();
void            getversion_setuserquota_c();
void            getversion_signal_process_c();
void            getversion_site_size_c();
void            getversion_skip_relay_c();
void            getversion_slen_c();
void            getversion_update_file_c();
void            getversion_update_newu_c();
void            getversion_update_quota_c();
void            getversion_update_rules_c();
void            getversion_userinfo_c();
void            getversion_user_over_quota_c();
void            getversion_vadd_ip_map_c();
void            getversion_valias_delete_c();
void            getversion_valias_delete_domain_c();
void            getversion_valiasinfo_c();
void            getversion_valias_insert_c();
void            getversion_valias_update_c();
void            getversion_valias_select_all_c();
void            getversion_valias_select_c();
void            getversion_indivariables_c();
void            getversion_vauth_active_c();
void            getversion_vauth_adddomain_c();
void            getversion_vauth_adduser_c();
void            getversion_vauth_deldomain_c();
void            getversion_vauth_deluser_c();
void            getversion_vauth_getall_c();
void            getversion_vauth_getpw_c();
void            getversion_vauth_munch_domain_c();
void            getversion_vauth_open_c();
void            getversion_vauth_setpw_c();
void            getversion_vauth_setquota_c();
void            getversion_vauth_vpasswd_c();
void            getversion_vclear_open_smtp_c();
void            getversion_vclose_c();
void            getversion_vcreate_dir_control_c();
void            getversion_vdel_dir_control_c();
void            getversion_vdelfiles_c();
void            getversion_vdel_ip_map_c();
void            getversion_vget_assign_c();
void            getversion_vgetent_c();
void            getversion_vget_ip_map_c();
void            getversion_vget_lastauth_c();
void            getversion_vgetpasswd_c();
void            getversion_vget_real_domain_c();
void            getversion_vmake_maildir_c();
void            getversion_vopen_smtp_relay_c();
void            getversion_vquota_select_c();
void            getversion_vread_dir_control_c();
void            getversion_vset_default_domain_c();
void            getversion_vset_lastauth_c();
void            getversion_vset_lastdeliver_c();
void            getversion_vshow_ip_map_c();
void            getversion_vupd_ip_map_c();
void            getversion_vupdate_rules_c();
void            getversion_vwrite_dir_control_c();
void            getversion_no_of_days_c();
void            getversion_addusercntrl_c();
void            getversion_delusercntrl_c();
void            getversion_updusercntrl_c();
void            getversion_get_smtp_service_port_c();
void            getversion_get_local_ip_c();
void            getversion_get_local_hostid_c();
void            getversion_vauth_updateflag_c();
void            getversion_vauth_getflags_c();
void            getversion_cntrl_clearaddflag_c();
void            getversion_cntrl_cleardelflag_c();
void            getversion_qmail_remote_c();
void            getversion_error_temp_c();
void            getversion_init_dir_control_c();
void            getversion_replacestr_c();
void            getversion_vlog_c();
void            getversion_trashpurge_c();
void            getversion_delunreadmails_c();
void            getversion_tcpopen_c();
void            getversion_isnum_c();
void            getversion_setsockbuf_c();
void            getversion_monkey_c();
void            getversion_sockwrite_c();
void            getversion_sockread_c();
void            getversion_tcpbind_c();
void            getversion_islocalif_c();
void            getversion_vsmtp_insert_c();
void            getversion_vsmtp_update_c();
void            getversion_vsmtp_delete_c();
void            getversion_vsmtp_delete_domain_c();
void            getversion_vsmtp_select_c();
void            getversion_open_master_c();
void            getversion_is_already_running_c();
void            getversion_MakeArgs_c();
void            getversion_Dirname_c();
void            getversion_pwcomp_c();
void            getversion_vauth_delaliasdomain_c();
void            getversion_vauth_insertaliasdomain_c();
void            getversion_vauth_get_realdomain_c();
void            getversion_proxylogin_c();
void            getversion_isvalid_domain_c();
void            getversion_parse_quota_c();
void            getversion_remove_quotes_c();
void            getversion_vauth_init_c();
void            getversion_UserInLookup_c();
void            getversion_VlimitInLookup_c();
void            getversion_RelayInLookup_c();
void            getversion_SqlServer_c();
void            getversion_LoadDbinfo_c();
void            getversion_pw_comp_c();
void            getversion_vauth_getipaddr_c();
void            getversion_vauth_gethostid_c();
void            getversion_vhostid_select_c();
void            getversion_vhostid_insert_c();
void            getversion_vhostid_update_c();
void            getversion_vhostid_delete_c();
void            getversion_dbload_c();
void            getversion_get_localtime_c();
void            getversion_RemoteBulkMail_c();
void            getversion_maildir_to_domain_c();
void            getversion_maildir_to_email_c();
void            getversion_FifoCreate_c();
void            getversion_getDbLock_c();
void            getversion_vauthOpen_user_c();
void            getversion_ProcessInFifo_c();
void            getversion_strToPw_c();
void            getversion_findmdahost_c();
void            getversion_inquery_c();
void            getversion_relay_select_c();
void            getversion_PwdInLookup_c();
void            getversion_AliasInLookup_c();
void            getversion_timeoutio_c();
void            getversion_bulletin_c();
void            getversion_mdaMysqlConnect_c();
void            getversion_renameuser_c();
void            getversion_vauth_renamedomain_c();
void            getversion_add_vacation_c();
void            getversion_SendWelcomeMail_c();
void            getversion_copyPwdStruct_c();
void            getversion_makeseekable_c();
void            getversion_passwd_policy_c();
void            getversion_mgmtpassfuncs_c();
void            getversion_udpopen_c();
void            getversion_vfstab_select_c();
void            getversion_vfstab_insert_c();
void            getversion_vfstab_delete_c();
void            getversion_vfstab_update_c();
void            getversion_vfstab_status_c();
void            getversion_fstabChangeCounters_c();
void            getversion_getFreeFS_c();
void            getversion_getactualpath_c();
void            getversion_pathToFilesystem_c();
void            getversion_vfstabNew_c();
void            getversion_add_control_c();
void            getversion_autoturn_dir_c();
void            getversion_pipe_exec_c();
void            getversion_vfilter_select_c();
void            getversion_vfilter_mlistOpt_c();
void            getversion_vfilter_insert_c();
void            getversion_vfilter_delete_c();
void            getversion_vfilter_update_c();
void            getversion_vfilter_display_c();
void            getversion_vfilter_filterNo_c();
void            getversion_parseAddress_c();
void            getversion_storeHeader_c();
void            getversion_getmailingList_c();
void            getversion_mlist_insert_c();
void            getversion_mlist_update_c();
void            getversion_mlist_delete_c();
void            getversion_mlist_filterupdate_c();
void            getversion_addressToken_c();
void            getversion_is_mailing_list_c();
void            getversion_mlist_filterno_c();
void            getversion_getAddressBook_c();
void            getversion_deliver_mail_c();
void            getversion_skip_system_files_c();
void            getversion_spam_c();
void            getversion_LoadBMF_c();
void            getversion_limits_c();
void            getversion_create_table_c();
void            getversion_filewrt_c();
void            getversion_dbinfoAdd_c();
void            getversion_dbinfoDel_c();
void            getversion_valiasCount_c();
void            getversion_GetSmtproute_c();
void            getversion_dbinfoUpdate_c();
void            getversion_dbinfoSelect_c();
void            getversion_lockTable_c();
void            getversion_layout_c();
void            getversion_wildmat_c();
void            getversion_vshow_atrn_map_c();
void            getversion_vadd_atrn_map_c();
void            getversion_vdel_atrn_map_c();
void            getversion_vupd_atrn_map_c();
void            getversion_atrn_access_c();
void            getversion_vpriv_select_c();
void            getversion_vpriv_insert_c();
void            getversion_vpriv_delete_c();
void            getversion_vpriv_update_c();
void            getversion_checkPerm_c();
void            getversion_loadbalance_c();
void            getversion_count_table_c();
void            getversion_vfilter_header_c();
void            getversion_evaluate_c();
void            getversion_hostcntrl_select_c();
void            getversion_runcmmd_c();
void            getversion_getenvConfig_c();
void            getversion_set_mysql_options_c();
void            getversion_error_stack_c();
void            getversion_in_crypt_c();
void            getversion_mysql_query_c();
void            getversion_unsetenv_c();
void            getversion_post_handle_c();
void            getversion_ismaildup_c();
void            getversion_md5_crypt_c();
void            getversion_hmac_md5_c();
void            getversion_hmac_sha1_c();
void            getversion_hmac_sha256_c();
void            getversion_hmac_ripemd_c();
void            getversion_sha1_c();
void            getversion_ripemd_c();
void            getversion_digest_md5_c();
void            getversion_setuserid_c();
void            getversion_isPrime_c();

void            getversion();

#ifdef MAIN
static void     Ident(char *, int);

char           *bin_program_list[] = 
{
	"vadddomain",
	"vdeldomain",
	"vpasswd",
	"vadduser",
	"vdeluser",
	"vaddaliasdomain",
	"vsetuserquota",
	"vbulletin",
	"vmoduser",
	"valias",
	"vuserinfo",
	"vipmap",
	"vacation",
	"vdominfo",
	"printdir",
	"vmoveuser",
	"vcalias",
	"vsmtp",
	"proxyimap",
	"proxypop3",
	"vproxy",
	"vhostid",
	"dbinfo",
	"vrenameuser",
	"vrenamedomain",
	"crc",
	"crcdiff",
	"vcfilter",
	"vgroup",
	"vatrn",
	"vpriv",
	"vlimit",
	"hostcntrl",
	"vcaliasrev",
	"versioninfo",
	"tcpserver",
	"tcprules",
	"tcprulescheck",
	"tcpclient",
	"who@",
	"date@",
	"finger@",
	"http@",
	"tcpcat",
	"mconnect",
	"mconnect-io",
	"rblsmtpd",
	"sslerator",
	"imapd",
	"pop3d",
	"logmonitor",
	"ismaildup",
	0
};

char           *sbin_program_list[] = 
{
	"inlookup",
	"indisrvr",
	"inquerytest",
	"osh",
	"initsvc",
	"install_tables",
	"clearopensmtp",
	"hostsync",
	"vdeloldusers",
	"vmoddomain",
	"vreorg",
	"vtable",
	"updaterules",
	"systpass",
	"vchkpass",
	"vsetpass",
	"chowkidar",
	"ipchange",
	"vdelivermail",
	"vfilter",
	"postdel",
	"adminclient",
	"vfstab",
	"svctool",
	"mgmtpass",
	"vserverinfo",
	0
};

char           *libexec_program_list[] = 
{
	"authlib/authindi",
	"qmailmrtg7",
	"sq_vacation",
	"execmysql",
	"tls-cert-check",
	"get-cert",
	"cindimail",
	"hashtable",
	"displaytop",
	"updatefile",
	"mail_report",
	"testmra",
	"copyemail",
	"controlsync",
	"myslave",
	"cputime",
	"bogofilter-qfe",
	0
};

int
main()
{
	void indimail_settings();
	char            buffer[MAX_BUFF];
	char           *listfile = SYSCONFDIR"/qmailprog.list";
	char          **ptr;
	FILE           *fp;

	getversion(0);
	printf("\nMain Program Version List (IndiMail)\n");
	for(ptr = bin_program_list;*ptr;ptr++)
		Ident(*ptr, 1);
	for(ptr = sbin_program_list;*ptr;ptr++)
		Ident(*ptr, 2);
	for(ptr = libexec_program_list;*ptr;ptr++)
		Ident(*ptr, 3);
	if ((fp = fopen(listfile, "r")))
	{
		printf("\nMain Program Version List (qmail)\n");
		for(;;)
		{
			if (fscanf(fp, "%s", buffer) == EOF)
				break;
			Ident(buffer, 0);
		}
		fclose(fp);
	}
	indimail_settings();
	return(0);
}
#endif

void
getversion(char *id)
{
	getversion_indimail_settings_c();
	getversion_addaliasdomain_c();
	getversion_add_domain_assign_c();
	getversion_adddomain_c();
	getversion_add_user_assign_c();
	getversion_adduser_c();
	getversion_backfill_c();
	getversion_bulk_mail_c();
	getversion_Check_Login_c();
	getversion_check_quota_c();
	getversion_close_big_dir_c();
	getversion_compile_morercpthosts_c();
	getversion_CopyEmailFile_c();
	getversion_count_dir_c();
	getversion_count_rcpthosts_c();
	getversion_CreateDomainDirs_c();
	getversion_dec_dir_control_c();
	getversion_del_control_c();
	getversion_del_domain_assign_c();
	getversion_deldomain_c();
	getversion_del_user_assign_c();
	getversion_deluser_c();
	getversion_fappend_c();
	getversion_findhost_c();
	getversion_getlastauth_c();
	getversion_get_Mplexdir_c();
	getversion_getpeer_c();
	getversion_GetPrefix_c();
	getversion_GetIndiId_c();
	getversion_getindimail_c();
	getversion_check_group_c();
	getversion_host_in_locals_c();
	getversion_inc_dir_c();
	getversion_inc_dir_control_c();
	getversion_is_alias_domain_c();
	getversion_is_distributed_domain_c();
	getversion_is_user_present_c();
	getversion_isvirtualdomain_c();
	getversion_lockfile_c();
	getversion_Login_Tasks_c();
	getversion_logmysql_c();
	getversion_lowerit_c();
	getversion_MailQuotaWarn_c();
	getversion_make_user_dir_c();
#ifndef HAVE_MD5_CRYPT
	getversion_md5c_c();
#endif
#ifndef HAVE_SHA256_CRYPT
	getversion_sha256_crypt_c();
#endif
#ifndef HAVE_SHA512_CRYPT
	getversion_sha512_crypt_c();
#endif
	getversion_mkpasswd3_c();
	getversion_MoveFile_c();
	getversion_mysql_perror_c();
	getversion_next_big_dir_c();
	getversion_open_big_dir_c();
	getversion_parse_email_c();
	getversion_passwd_c();
	getversion_print_control_c();
	getversion_purge_files_c();
	getversion_makesalt_c();
	getversion_r_chown_c();
	getversion_recalc_quota_c();
	getversion_remove_line_c();
	getversion_r_mkdir_c();
	getversion_ScanDir_c();
	getversion_scat_c();
	getversion_scopy_c();
	getversion_setuserquota_c();
	getversion_signal_process_c();
	getversion_site_size_c();
	getversion_skip_relay_c();
	getversion_slen_c();
	getversion_update_file_c();
	getversion_update_newu_c();
	getversion_update_quota_c();
	getversion_update_rules_c();
	getversion_userinfo_c();
	getversion_user_over_quota_c();
	getversion_vadd_ip_map_c();
	getversion_valias_delete_c();
	getversion_valias_delete_domain_c();
	getversion_valiasinfo_c();
	getversion_valias_insert_c();
	getversion_valias_update_c();
	getversion_valias_select_all_c();
	getversion_valias_select_c();
	getversion_indivariables_c();
	getversion_vauth_active_c();
	getversion_vauth_adddomain_c();
	getversion_vauth_adduser_c();
	getversion_vauth_deldomain_c();
	getversion_vauth_deluser_c();
	getversion_vauth_getall_c();
	getversion_vauth_getpw_c();
	getversion_vauth_munch_domain_c();
	getversion_vauth_open_c();
	getversion_vauth_setpw_c();
	getversion_vauth_setquota_c();
	getversion_vauth_vpasswd_c();
	getversion_vclear_open_smtp_c();
	getversion_vclose_c();
	getversion_vcreate_dir_control_c();
	getversion_vdel_dir_control_c();
	getversion_vdelfiles_c();
	getversion_vdel_ip_map_c();
	getversion_vget_assign_c();
	getversion_vgetent_c();
	getversion_vget_ip_map_c();
	getversion_vget_lastauth_c();
	getversion_vgetpasswd_c();
	getversion_vget_real_domain_c();
	getversion_vmake_maildir_c();
	getversion_vopen_smtp_relay_c();
	getversion_vquota_select_c();
	getversion_vread_dir_control_c();
	getversion_vset_default_domain_c();
	getversion_vset_lastauth_c();
	getversion_vset_lastdeliver_c();
	getversion_vshow_ip_map_c();
	getversion_vupd_ip_map_c();
	getversion_vupdate_rules_c();
	getversion_vwrite_dir_control_c();
	getversion_no_of_days_c();
	getversion_addusercntrl_c();
	getversion_delusercntrl_c();
	getversion_updusercntrl_c();
	getversion_get_smtp_service_port_c();
	getversion_get_local_ip_c();
	getversion_get_local_hostid_c();
	getversion_vauth_updateflag_c();
	getversion_vauth_getflags_c();
	getversion_cntrl_clearaddflag_c();
	getversion_cntrl_cleardelflag_c();
	getversion_qmail_remote_c();
	getversion_error_temp_c();
	getversion_init_dir_control_c();
	getversion_replacestr_c();
	getversion_vlog_c();
	getversion_trashpurge_c();
	getversion_delunreadmails_c();
	getversion_tcpopen_c();
	getversion_isnum_c();
	getversion_setsockbuf_c();
	getversion_monkey_c();
	getversion_sockwrite_c();
	getversion_sockread_c();
	getversion_tcpbind_c();
	getversion_islocalif_c();
	getversion_vsmtp_insert_c();
	getversion_vsmtp_update_c();
	getversion_vsmtp_delete_c();
	getversion_vsmtp_delete_domain_c();
	getversion_vsmtp_select_c();
	getversion_open_master_c();
	getversion_is_already_running_c();
	getversion_MakeArgs_c();
	getversion_Dirname_c();
	getversion_pwcomp_c();
	getversion_vauth_delaliasdomain_c();
	getversion_vauth_insertaliasdomain_c();
	getversion_vauth_get_realdomain_c();
#ifdef MAIN
	getversion_proxylogin_c();
#endif
	getversion_isvalid_domain_c();
	getversion_parse_quota_c();
	getversion_remove_quotes_c();
	getversion_vauth_init_c();
	getversion_UserInLookup_c();
	getversion_RelayInLookup_c();
	getversion_SqlServer_c();
	getversion_LoadDbinfo_c();
	getversion_pw_comp_c();
	getversion_vauth_getipaddr_c();
	getversion_vauth_gethostid_c();
	getversion_vhostid_select_c();
	getversion_vhostid_insert_c();
	getversion_vhostid_update_c();
	getversion_vhostid_delete_c();
	getversion_dbload_c();
	getversion_get_localtime_c();
	getversion_RemoteBulkMail_c();
	getversion_maildir_to_domain_c();
	getversion_maildir_to_email_c();
	getversion_FifoCreate_c();
	getversion_getDbLock_c();
	getversion_vauthOpen_user_c();
	getversion_ProcessInFifo_c();
	getversion_strToPw_c();
	getversion_findmdahost_c();
	getversion_inquery_c();
	getversion_relay_select_c();
	getversion_PwdInLookup_c();
	getversion_AliasInLookup_c();
	getversion_timeoutio_c();
	getversion_bulletin_c();
	getversion_mdaMysqlConnect_c();
	getversion_renameuser_c();
	getversion_vauth_renamedomain_c();
	getversion_add_vacation_c();
	getversion_SendWelcomeMail_c();
	getversion_copyPwdStruct_c();
	getversion_makeseekable_c();
	getversion_passwd_policy_c();
	getversion_mgmtpassfuncs_c();
	getversion_udpopen_c();
	getversion_vfstab_select_c();
	getversion_vfstab_insert_c();
	getversion_vfstab_delete_c();
	getversion_vfstab_update_c();
	getversion_vfstab_status_c();
	getversion_fstabChangeCounters_c();
	getversion_getFreeFS_c();
	getversion_getactualpath_c();
	getversion_pathToFilesystem_c();
	getversion_vfstabNew_c();
	getversion_add_control_c();
	getversion_autoturn_dir_c();
	getversion_pipe_exec_c();
	getversion_vfilter_mlistOpt_c();
	getversion_vfilter_select_c();
	getversion_vfilter_insert_c();
	getversion_vfilter_delete_c();
	getversion_vfilter_update_c();
	getversion_vfilter_display_c();
	getversion_vfilter_filterNo_c();
	getversion_getmailingList_c();
	getversion_mlist_insert_c();
	getversion_mlist_update_c();
	getversion_mlist_delete_c();
	getversion_mlist_filterupdate_c();
	getversion_addressToken_c();
	getversion_storeHeader_c();
	getversion_parseAddress_c();
	getversion_is_mailing_list_c();
	getversion_mlist_filterno_c();
	getversion_getAddressBook_c();
	getversion_deliver_mail_c();
	getversion_skip_system_files_c();
	getversion_spam_c();
	getversion_LoadBMF_c();
	getversion_limits_c();
	getversion_create_table_c();
	getversion_filewrt_c();
	getversion_dbinfoAdd_c();
	getversion_dbinfoDel_c();
	getversion_valiasCount_c();
	getversion_GetSmtproute_c();
	getversion_dbinfoUpdate_c();
	getversion_dbinfoSelect_c();
	getversion_lockTable_c();
	getversion_layout_c();
	getversion_wildmat_c();
	getversion_vshow_atrn_map_c();
	getversion_vadd_atrn_map_c();
	getversion_vdel_atrn_map_c();
	getversion_vupd_atrn_map_c();
	getversion_atrn_access_c();
	getversion_vpriv_select_c();
	getversion_vpriv_insert_c();
	getversion_vpriv_delete_c();
	getversion_vpriv_update_c();
	getversion_checkPerm_c();
	getversion_loadbalance_c();
	getversion_count_table_c();
	getversion_vfilter_header_c();
	getversion_evaluate_c();
	getversion_hostcntrl_select_c();
	getversion_runcmmd_c();
	getversion_getenvConfig_c();
	getversion_set_mysql_options_c();
	getversion_error_stack_c();
	getversion_post_handle_c();
	getversion_ismaildup_c();
	getversion_in_crypt_c();
	getversion_mysql_query_c();
	getversion_unsetenv_c();
	getversion_md5_crypt_c();
	getversion_hmac_md5_c();
	getversion_hmac_sha1_c();
	getversion_hmac_sha256_c();
	getversion_hmac_ripemd_c();
	getversion_sha1_c();
	getversion_ripemd_c();
	getversion_digest_md5_c();
	getversion_setuserid_c();
	if(id)
		printf("%s\nIndiMail Version %s\n", id, VERSION);
	else
		printf("IndiMail Version %s\n", VERSION);
	return;
}

void
getversion_indimail_settings_c()
{
	printf("%s\n", sccsid);
#ifdef MAIN
	printf("%s\n", sccsidh);
#endif
}

#ifdef MAIN
static void
Ident(char *pgname, int mode)
{
	FILE           *fp;
	char            buffer[MAX_BUFF];
	char            idbuf[] = "# %Id: "; /*- workaround for git bug */

	if (mode == 1)
		snprintf(buffer, MAX_BUFF, "strings %s/bin/%s", PREFIX, pgname);
	else
	if (mode == 2)
		snprintf(buffer, MAX_BUFF, "strings %s/sbin/%s", PREFIX, pgname);
	else
	if (mode == 3)
		snprintf(buffer, MAX_BUFF, "strings %s/%s", LIBEXECDIR, pgname);
	else {
		snprintf(buffer, MAX_BUFF, "%s/bin/%s", PREFIX, pgname);
		if (!access(buffer, F_OK))
			snprintf(buffer, MAX_BUFF, "strings %s/bin/%s", PREFIX, pgname);
		else {
			snprintf(buffer, MAX_BUFF, "%s/sbin/%s", PREFIX, pgname);
			if (!access(buffer, F_OK))
				snprintf(buffer, MAX_BUFF, "strings %s/sbin/%s", PREFIX, pgname);
			else {
				snprintf(buffer, MAX_BUFF, "%s/libexec/%s", PREFIX, pgname);
				if (!access(buffer, F_OK))
					snprintf(buffer, MAX_BUFF, "strings %s/libexec/%s", PREFIX, pgname);
				else
					return;
			}
		}
	}
	if(!(fp = popen(buffer, "r")))
	{
		perror(buffer);
		return;
	}
	for(idbuf[2] = '$';;)
	{
		if(!fgets(buffer, MAX_BUFF - 2, fp))
			break;
		if(!strncmp(buffer, idbuf + 2, 4))
			printf("%s", buffer);
		else
		if (!strncmp(buffer, idbuf, 7))
			printf("%s", buffer + 2);
	}
	pclose(fp);
	return;
}
#endif
