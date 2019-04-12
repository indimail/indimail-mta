@SET_MAKE@
SUBDIRS=@subdir_list@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
release=$(shell grep Release: indimail.spec.in | cut -d: -f2|cut -c2-)

DEFS=@DEFS@ -fno-strict-aliasing -I. -Ieps-1.2 @auth_inc@
cdb_lib=$(top_builddir)/cdb-1.0/libcdb.la

INSTALL_PROGRAM = ${INSTALL}          -c
install_sh_PROGRAM = $(install_sh)    -c
INSTALL_SCRIPT = ${INSTALL}           -c
install_sh_SCRIPT = $(install_sh)     -c
INSTALL_STRIP_PROGRAM = $(install_sh) -c $(STRIP_OPT)

indimailbindir=@prefix@/bin
indimailsbindir=@prefix@/sbin
indimaillibexecdir=@libexecdir@
indimailauthlibdir=@libexecdir@/imapmodules

libindimail_LTLIBRARIES = libindimail.la
libindimail_la_SOURCES = $(COMMONSOURCES)
libindimail_la_DEPENDENCIES = $(top_builddir)/cdb-1.0/libcdb.la
libindimail_la_LIBADD = $(top_builddir)/cdb-1.0/libcdb.la -L@mysql_libdir@ -lmysqlclient @LIB_CRYPT@ @LIB_SSL@ -lm

libindimail_la_LDFLAGS = -version-info @LIBVER@:0:0
libindimaildir=@libdir@

iauth_LTLIBRARIES = iauth.la
iauth_la_SOURCES = iauth.c
iauth_la_LIBADD = $(top_builddir)/libindimail.la
iauth_la_DEPENDENCIES = $(top_builddir)/libindimail.la
iauth_la_LDFLAGS = -module -avoid-version -shared -no-undefined -rdynamic -nostartfiles 
iauthdir=@libdir@

SYSCONFDIR = @sysconfdir@
sysconf_DATA = indimail.settings controlfiles.i headerlist osh.table cronlist.i perm_list.i \
			   indimail.mrtg.cfg indimail-release
pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA = libindimail.pc

man_MANS=vadduser.1 adminclient.8 chowkidar.8 clearopensmtp.8 dbinfo.8 hostcntrl.8 hostsync.8 \
	initsvc.1 indisrvr.8 inlookup.8 InLookup.8 inquerytest.8 ipchange.8 \
	mgmtpass.8 postdel.8 printdir.1 proxyimap.8 proxypop3.8 resetquota.8 \
	vacation.8 vaddaliasdomain.1 vadddomain.1 valias.1 vatrn.1 vbulletin.1 vcalias.8 \
	vcaliasrev.8 vcfilter.1 vchkpass.8 vdeldomain.1 vdelivermail.8 vdeloldusers.8 \
	vdeluser.1 vdominfo.1 vfilter.8 vfstab.8 vgroup.1 vhostid.8 vipmap.1 vmoddomain.1 \
	vlimit.1 vmoduser.1 vmoveuser.1 vpasswd.1 vpriv.8 vproxy.8 vrenamedomain.1 \
	vrenameuser.1 vreorg.8 vsetuserquota.1 vsmtp.8 vuserinfo.1 indimail.7 IndiMail.7 \
	updaterules.8 osh.8 systpass.8 vserverinfo.1 sslerator.1 iecho.1 \
	ismaildup.1 login_tasks.3 vtable.1 install_tables.8 \
	crc.1 crcdiff.1 vsetpass.8 crypt.1 

setuid_list = ../sbin/systpass vdeldomain vadddomain vadduser vdeluser \
			  vaddaliasdomain vrenamedomain vrenameuser vbulletin vdominfo \
			  printdir vuserinfo ../sbin/vreorg vsetuserquota vmoduser \
			  vmoveuser vcfilter ../libexec/sq_vacation

doc_list= 37rules.pdf ChangeLog.indimail COPYING.indimail FAQ.pdf HOWTO.bogofilter \
	HOWTO.sharedmaildir INSTALL.indimail percenthack.shtml README \
	INSTALL-MYSQL RELEASE-Notes INSTALL-RPM README-CLUSTER \
	indimail_arch.png indimail_queue.png Quick-INSTALL \
	AUTHORS.altermime AUTHORS.bogofilter AUTHORS.courier-imap AUTHORS.fortune \
	AUTHORS.nssd AUTHORS.pam-multi \
	AUTHORS.ripmime COPYING.bogofilter COPYING.courier-imap \
	COPYING.flash COPYING.fortune COPYING.nssd COPYING.pam-multi \
	COPYRIGHT.flash COPYRIGHT.mpack LICENSE.altermime \
	LICENSE.ripmime README.altermime \
	README.courier-imap README.flash \
	README.ldap README.nssd \
	README.pam-multi README.ripmime \
	README.vlimits README-SSL-SERVER.fetchmail \
	README-SSL.fetchmail README-NTLM.fetchmail README.fetchmail \
	COPYING.fetchmail

COMMONSOURCES = addaliasdomain.c add_control.c add_domain_assign.c adddomain.c addressToken.c \
add_user_assign.c adduser.c addusercntrl.c add_vacation.c \
atrn_access.c autoturn_dir.c backfill.c bulk_mail.c bulletin.c \
Check_Login.c checkPerm.c check_quota.c close_big_dir.c cntrl_clearaddflag.c \
cntrl_cleardelflag.c compile_morercpthosts.c CopyEmailFile.c copyPwdStruct.c \
count_dir.c count_rcpthosts.c count_table.c CreateDomainDirs.c create_table.c \
check_group.c dbinfoAdd.c dbinfoDel.c dbinfoSelect.c dbinfoUpdate.c dbload.c \
dec_dir_control.c del_control.c del_domain_assign.c deldomain.c digest_md5.c \
deliver_mail.c delunreadmails.c del_user_assign.c deluser.c delusercntrl.c \
Dirname.c error_stack.c error_temp.c evaluate.c fappend.c FifoCreate.c \
filewrt.c findhost.c findmdahost.c fstabChangeCounters.c getactualpath.c \
getAddressBook.c getDbLock.c getEnvConfig.c getFreeFS.c GetIndiId.c getindimail.c \
getlastauth.c get_local_hostid.c get_local_ip.c get_localtime.c getmailingList.c \
get_Mplexdir.c getpeer.c GetPrefix.c GetSmtproute.c get_smtp_service_port.c \
hmac_sha256.c hostcntrl_select.c host_in_locals.c in_crypt.c inc_dir.c inc_dir_control.c \
init_dir_control.c inquery.c is_alias_domain.c is_already_running.c ismaildup.c \
is_distributed_domain.c islocalif.c isnum.c is_user_present.c \
isvalid_domain.c isvirtualdomain.c layout.c limits.c LoadBMF.c \
LoadDbinfo.c lockfile.c Login_Tasks.c logmysql.c lowerit.c \
maildir_to_domain.c maildir_to_email.c MailQuotaWarn.c MakeArgs.c makeseekable.c \
make_user_dir.c makesalt.c md5c.c mdaMysqlConnect.c mgmtpassfuncs.c mkpasswd3.c \
monkey.c MoveFile.c mysql_perror.c mysql_query.c next_big_dir.c no_of_days.c \
open_big_dir.c open_master.c parse_email.c hmac_sha512.c setuserid.c \
parse_quota.c passwd.c passwd_policy.c pathToFilesystem.c pipe_exec.c post_handle.c \
print_control.c purge_files.c pw_comp.c pwcomp.c \
qmail_remote.c r_chown.c recalc_quota.c relay_select.c \
RemoteBulkMail.c remove_line.c remove_quotes.c renameuser.c replacestr.c r_mkdir.c \
runcmmd.c ScanDir.c scat.c scopy.c SendWelcomeMail.c set_mysql_options.c setsockbuf.c \
setuserquota.c sha256_crypt.c sha512_crypt.c \
skip_relay.c skip_system_files.c slen.c hmac_sha1.c hmac_ripemd.c ripemd.c \
md5_crypt.c hmac_md5.c sockread.c sockwrite.c spam.c SqlServer.c strToPw.c tcpbind.c \
tcpopen.c timeoutio.c trashpurge.c udpopen.c unsetenv.c update_file.c update_newu.c \
update_quota.c update_rules.c updusercntrl.c userinfo.c user_over_quota.c \
vadd_atrn_map.c vadd_ip_map.c valiasCount.c valias_delete.c valias_delete_domain.c \
valiasinfo.c valias_insert.c valias_select_all.c valias_select.c valias_update.c \
variables.c vauth_active.c vauth_adddomain.c vauth_adduser.c vauth_delaliasdomain.c \
vauth_deldomain.c vauth_deluser.c vauth_getall.c vauth_getflags.c vauth_gethostid.c \
vauth_getipaddr.c vauth_getpw.c vauth_get_realdomain.c vauth_init.c \
vauth_insertaliasdomain.c vauth_munch_domain.c vauth_open.c vauthOpen_user.c \
vauth_renamedomain.c vauth_setpw.c vauth_setquota.c vauth_updateflag.c vauth_vpasswd.c \
vclear_open_smtp.c vclose.c vcreate_dir_control.c vdel_atrn_map.c vdel_dir_control.c \
vdelfiles.c vdel_ip_map.c vfilter_delete.c vfilter_display.c vfilter_filterNo.c \
vfilter_header.c vfilter_insert.c vfilter_select.c vfilter_update.c \
vfstab_delete.c vfstab_insert.c vfstabNew.c vfstab_select.c vfstab_status.c \
vfstab_update.c vget_assign.c vget_ip_map.c vget_lastauth.c vgetpasswd.c \
vget_real_domain.c vhostid_delete.c vhostid_insert.c vhostid_select.c vhostid_update.c \
vlog.c vmake_maildir.c vopen_smtp_relay.c vpriv_delete.c vpriv_insert.c \
vpriv_select.c vpriv_update.c vread_dir_control.c vset_default_domain.c \
vset_lastauth.c vset_lastdeliver.c vshow_atrn_map.c vshow_ip_map.c vsmtp_delete.c \
vsmtp_delete_domain.c vsmtp_insert.c vsmtp_select.c vsmtp_update.c vupdate_rules.c \
vupd_atrn_map.c vupd_ip_map.c vwrite_dir_control.c wildmat.c

libindimail.la: $(libindimail_la_OBJECTS) $(libindimail_la_DEPENDENCIES)
	$(LIBTOOL) --mode=link --tag=CC gcc $(libindimail_la_LDFLAGS) \
	-O -o libindimail.la $(libindimail_la_OBJECTS) $(libindimail_la_LIBADD) \
	-rpath @libdir@

iauth.la: $(iauth_la_OBJECTS) $(iauth_la_DEPENDENCIES)
	$(LIBTOOL) --mode=link $(COMPILE) $(MODFLAGS) -o $@ iauth.lo $(iauth_la_LDFLAGS) \
	$(iauth_la_LIBADD) -L.libs -rpath @libdir@

iauth.lo: iauth.c
	$(LIBTOOL) --mode=compile gcc $(DEFS) @auth_inc@ -c $^

all-local: libindimail.la iauth.la

indimailbin_PROGRAMS = crc crcdiff dbinfo hostcntrl \
	printdir proxyimap proxypop3 sslerator vacation vaddaliasdomain vadddomain \
	vadduser valias vatrn vbulletin vcalias vcfilter vdeldomain vdeluser \
	vdominfo vgroup vhostid vipmap vlimit vmoduser vmoveuser vpasswd vpriv ismaildup \
	vproxy vrenamedomain vrenameuser vsetuserquota vsmtp vuserinfo crypt iecho
indimailsbin_PROGRAMS = adminclient chowkidar clearopensmtp \
	hostsync indisrvr initsvc inlookup inquerytest install_tables \
	ipchange mgmtpass osh postdel systpass updaterules \
	vserverinfo vchkpass vsetpass vdelivermail vdeloldusers vfilter vfstab \
	vmoddomain vreorg vtable resetquota

indimaillibexec_PROGRAMS = sq_vacation qmailmrtg7 cindimail updatefile
indimaillibexec_SCRIPTS = mailzipper overquota.sh controlsync cputime \
						  myslave bogofilter-qfe iupgrade.sh ilocal_upgrade.sh

indimailauthlib_PROGRAMS = authindi authpgsql

indimailauthlib_SCRIPTS = authgeneric

indimailbin_SCRIPTS = vcaliasrev

noinst_SCRIPTS = indimail.spec indimail-rpmlintrc \
	INSTALL INSTALL-RPM INSTALL-MYSQL README-CLUSTER RELEASE-Notes Quick-INSTALL \
	README vadddomain_handle vadduser_handle vdeldomain_handle \
	vaddaliasdomain_handle vrenamedomain_handle vpasswd_handle vmoduser_handle \
	vdeluser_handle vrenameuser_handle

post_handle_list = vadddomain vadduser vdeldomain vaddaliasdomain vrenamedomain vmoduser vpasswd \
				   vdeluser_handle vrenameuser_handle

MOSTLYCLEANFILES=vcaliasrev myslave controlsync bogofilter-qfe \
cronlist.i crypt vdeluser_handle vrenameuser_handle \
cscope.* *.dSYM socket.lib indimail-rpmlintrc mailzipper \
perm_list.i dbinfo.8 inlookup.8 vadddomain.1 vdeldomain.1 vadduser.1 \
vdeluser.1 vdelivermail.8 vrenamedomain.1 vrenameuser.1 vpasswd.1 vmoveuser.1 \
vcfilter.1 indimail.7 indimail.mrtg.cfg overquota.sh \
vadddomain_handle vadduser_handle vpasswd_handle vdeldomain_handle vaddaliasdomain_handle \
vrenamedomain_handle login_tasks.3 authgeneric usr.sbin.mysqld usr.sbin.clamd \
vmoduser_handle vmoduser.1 usr.bin.freshclam indimail.mod indimail.pp

#DISTCLEANFILES=.gitignore

vuserinfo_SOURCES = vuserinfo.c
vuserinfo_LDADD = @auth_libs@
vuserinfo_DEPENDENCIES = $(top_builddir)/libindimail.la

vdominfo_SOURCES = vdominfo.c
vdominfo_LDADD = @auth_libs@
vdominfo_DEPENDENCIES = $(top_builddir)/libindimail.la

vdelivermail_SOURCES = vdelivermail.c 
vdelivermail_LDADD = @auth_libs@
vdelivermail_DEPENDENCIES = $(top_builddir)/libindimail.la

ismaildup_SOURCES = ismaildup.c 
ismaildup_LDADD = @auth_libs@
ismaildup_DEPENDENCIES = $(top_builddir)/libindimail.la
ismaildup_CFLAGS = -DMAIN

printdir_SOURCES = printdir.c 
printdir_LDADD = @auth_libs@
printdir_DEPENDENCIES = $(top_builddir)/libindimail.la

hostsync_SOURCES = hostsync.c 
hostsync_LDADD = @auth_libs@
hostsync_DEPENDENCIES = $(top_builddir)/libindimail.la

clearopensmtp_SOURCES = clearopensmtp.c
clearopensmtp_LDADD = @auth_libs@
clearopensmtp_DEPENDENCIES = $(top_builddir)/libindimail.la

vadddomain_SOURCES = vadddomain.c 
vadddomain_LDADD = @auth_libs@
vadddomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vdeldomain_SOURCES = vdeldomain.c 
vdeldomain_LDADD = @auth_libs@
vdeldomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vpasswd_SOURCES = vpasswd.c 
vpasswd_LDADD = @auth_libs@
vpasswd_DEPENDENCIES = $(top_builddir)/libindimail.la

vadduser_SOURCES = vadduser.c
vadduser_LDADD = @auth_libs@ $(shell cat socket.lib 2>/dev/null)
vadduser_DEPENDENCIES = $(top_builddir)/libindimail.la

vgroup_SOURCES = vgroup.c
vgroup_LDADD = @auth_libs@
vgroup_DEPENDENCIES = $(top_builddir)/libindimail.la

cindimail_SOURCES = cindimail.c 
cindimail_LDADD = @auth_libs@ -lreadline @LIB_TERMCAP@
cindimail_DEPENDENCIES = $(top_builddir)/libindimail.la

vatrn_SOURCES = vatrn.c 
vatrn_LDADD = @auth_libs@
vatrn_DEPENDENCIES = $(top_builddir)/libindimail.la

vdeluser_SOURCES = vdeluser.c 
vdeluser_LDADD = @auth_libs@
vdeluser_DEPENDENCIES = $(top_builddir)/libindimail.la

vaddaliasdomain_SOURCES = vaddaliasdomain.c 
vaddaliasdomain_LDADD = @auth_libs@
vaddaliasdomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vsetuserquota_SOURCES = vsetuserquota.c 
vsetuserquota_LDADD = @auth_libs@
vsetuserquota_DEPENDENCIES = $(top_builddir)/libindimail.la

vbulletin_SOURCES = vbulletin.c 
vbulletin_LDADD = @auth_libs@
vbulletin_DEPENDENCIES = $(top_builddir)/libindimail.la

vdeloldusers_SOURCES = vdeloldusers.c 
vdeloldusers_LDADD = @auth_libs@
vdeloldusers_DEPENDENCIES = $(top_builddir)/libindimail.la

vmoduser_SOURCES = vmoduser.c 
vmoduser_LDADD = @auth_libs@
vmoduser_DEPENDENCIES = $(top_builddir)/libindimail.la

vlimit_SOURCES = vlimit.c
vlimit_LDADD = @auth_libs@
vlimit_DEPENDENCIES = $(top_builddir)/libindimail.la

valias_SOURCES = valias.c 
valias_LDADD = @auth_libs@
valias_DEPENDENCIES = $(top_builddir)/libindimail.la

vipmap_SOURCES = vipmap.c 
vipmap_LDADD = @auth_libs@
vipmap_DEPENDENCIES = $(top_builddir)/libindimail.la

vacation_SOURCES = vacation.c 
vacation_LDADD = @auth_libs@
vacation_DEPENDENCIES = $(top_builddir)/libindimail.la

updaterules_SOURCES = updaterules.c 
updaterules_LDADD = @auth_libs@
updaterules_DEPENDENCIES = $(top_builddir)/libindimail.la

vreorg_SOURCES = vreorg.c
vreorg_LDADD = @auth_libs@
vreorg_DEPENDENCIES = $(top_builddir)/libindimail.la

vmoveuser_SOURCES = vmoveuser.c
vmoveuser_LDADD = @auth_libs@
vmoveuser_DEPENDENCIES = $(top_builddir)/libindimail.la

vrenameuser_SOURCES = vrenameuser.c
vrenameuser_LDADD = @auth_libs@
vrenameuser_DEPENDENCIES = $(top_builddir)/libindimail.la

vrenamedomain_SOURCES = vrenamedomain.c
vrenamedomain_LDADD = @auth_libs@
vrenamedomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vcalias_SOURCES = vcalias.c
vcalias_LDADD = @auth_libs@
vcalias_DEPENDENCIES = $(top_builddir)/libindimail.la

vsmtp_SOURCES = vsmtp.c 
vsmtp_LDADD = @auth_libs@
vsmtp_DEPENDENCIES = $(top_builddir)/libindimail.la

vfstab_SOURCES = vfstab.c
vfstab_LDADD = @auth_libs@
vfstab_DEPENDENCIES = $(top_builddir)/libindimail.la

vhostid_SOURCES = vhostid.c 
vhostid_LDADD = @auth_libs@
vhostid_DEPENDENCIES = $(top_builddir)/libindimail.la

indisrvr_SOURCES = indisrvr.c 
indisrvr_LDADD = @auth_libs@ $(shell cat socket.lib 2>/dev/null)
indisrvr_DEPENDENCIES = $(top_builddir)/libindimail.la

sslerator_SOURCES = sslerator.c 
sslerator_LDADD = @auth_libs@ $(shell cat socket.lib 2>/dev/null)
sslerator_DEPENDENCIES = $(top_builddir)/libindimail.la

proxyimap_SOURCES = proxyimap.c 
proxyimap_LDADD = @auth_libs@ proxylogin.lo tls.lo auth_admin.lo adminCmmd.lo
proxyimap_DEPENDENCIES = proxylogin.lo tls.lo auth_admin.lo adminCmmd.lo $(top_builddir)/libindimail.la

proxypop3_SOURCES = proxypop3.c 
proxypop3_LDADD = @auth_libs@ proxylogin.lo tls.lo auth_admin.lo adminCmmd.lo
proxypop3_DEPENDENCIES = proxylogin.lo tls.lo auth_admin.lo adminCmmd.lo $(top_builddir)/libindimail.la

vchkpass_SOURCES = vchkpass.c 
vchkpass_LDADD = @auth_libs@
vchkpass_DEPENDENCIES = $(top_builddir)/libindimail.la

vsetpass_SOURCES = vsetpass.c 
vsetpass_LDADD = @auth_libs@
vsetpass_DEPENDENCIES = $(top_builddir)/libindimail.la

authindi_SOURCES = authindi.c 
authindi_LDADD = @auth_libs@
authindi_DEPENDENCIES = $(top_builddir)/libindimail.la

systpass_SOURCES = systpass.c 
systpass_LDADD = @auth_libs@
systpass_DEPENDENCIES = $(top_builddir)/libindimail.la

vproxy_SOURCES = vproxy.c 
vproxy_LDADD = @auth_libs@
vproxy_DEPENDENCIES = $(top_builddir)/libindimail.la

dbinfo_SOURCES = dbinfo.c 
dbinfo_LDADD = @auth_libs@
dbinfo_DEPENDENCIES = $(top_builddir)/libindimail.la

inquerytest_SOURCES = inquerytest.c 
inquerytest_LDADD = @auth_libs@ ProcessInFifo.lo  AliasInLookup.lo PwdInLookup.lo \
	RelayInLookup.lo UserInLookup.lo VlimitInLookup.lo -ldl
inquerytest_DEPENDENCIES = ProcessInFifo.lo  AliasInLookup.lo PwdInLookup.lo RelayInLookup.lo \
	UserInLookup.lo VlimitInLookup.lo $(top_builddir)/libindimail.la

inlookup_SOURCES = inlookup.c 
inlookup_LDADD = @auth_libs@ ProcessInFifo.lo AliasInLookup.lo PwdInLookup.lo \
	RelayInLookup.lo UserInLookup.lo VlimitInLookup.lo -ldl
inlookup_DEPENDENCIES = ProcessInFifo.lo AliasInLookup.lo PwdInLookup.lo RelayInLookup.lo \
	UserInLookup.lo VlimitInLookup.lo $(top_builddir)/libindimail.la

mgmtpass_SOURCES = mgmtpass.c 
mgmtpass_LDADD = @auth_libs@
mgmtpass_DEPENDENCIES = $(top_builddir)/libindimail.la

adminclient_SOURCES = adminclient.c
adminclient_LDADD = @auth_libs@ tls.lo auth_admin.lo adminCmmd.lo
adminclient_DEPENDENCIES = tls.lo auth_admin.lo adminCmmd.lo $(top_builddir)/libindimail.la

sq_vacation_SOURCES = sq_vacation.c
sq_vacation_LDADD = @auth_libs@
sq_vacation_DEPENDENCIES = $(top_builddir)/libindimail.la

qmailmrtg7_SOURCES = qmailmrtg7.c
qmailmrtg7_LDADD = @auth_libs@
qmailmrtg7_DEPENDENCIES = $(top_builddir)/libindimail.la

vpriv_SOURCES = vpriv.c 
vpriv_LDADD = @auth_libs@
vpriv_DEPENDENCIES = $(top_builddir)/libindimail.la

hostcntrl_SOURCES = hostcntrl.c 
hostcntrl_LDADD = @auth_libs@
hostcntrl_DEPENDENCIES = $(top_builddir)/libindimail.la

ipchange_SOURCES = ipchange.c 
ipchange_LDADD = @auth_libs@
ipchange_DEPENDENCIES = $(top_builddir)/libindimail.la

resetquota_SOURCES = resetquota.c 
resetquota_LDADD = @auth_libs@
resetquota_DEPENDENCIES = $(top_builddir)/libindimail.la

crc_SOURCES=crc.c
crc_LDADD=

crcdiff_SOURCES=crcdiff.c
crcdiff_LDADD=

authpgsql_SOURCES=authpgsql.c
authpgsql_CFLAGS = -I/usr/include/postgresql
authpgsql_LDADD=@auth_pg_libs@ @auth_libs@
authpgsql_DEPENDENCIES = $(top_builddir)/libindimail.la

crypt_SOURCES = crypt.c
crypt_LDADD = @LIB_CRYPT@

vfilter_SOURCES=vfilter.c
vfilter_LDADD= storeHeader.lo parseAddress.lo $(top_builddir)/eps-1.2/libeps.la @auth_libs@
vfilter_DEPENDENCIES=storeHeader.lo parseAddress.lo $(top_builddir)/libindimail.la

vmoddomain_SOURCES=vmoddomain.c
vmoddomain_LDADD= @auth_libs@
vmoddomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vcfilter_SOURCES=vcfilter.c
vcfilter_LDADD= @auth_libs@
vcfilter_DEPENDENCIES = $(top_builddir)/libindimail.la

chowkidar_SOURCES = chowkidar.c
chowkidar_LDADD = @auth_libs@
chowkidar_DEPENDENCIES = $(top_builddir)/libindimail.la

postdel_SOURCES = postdel.c
postdel_LDADD = @auth_libs@
postdel_DEPENDENCIES = $(top_builddir)/libindimail.la

vserverinfo_SOURCES = vserverinfo.c
vserverinfo_LDADD = @auth_libs@
vserverinfo_DEPENDENCIES = $(top_builddir)/libindimail.la

osh_SOURCES = osh.c
osh_LDADD = @LIB_CURSES@ @LIB_TERMCAP@
osh_DEPENDENCIES = exp.lo handlers.lo getEnvConfig.lo

updatefile_SOURCES=updatefile.c
updatefile_LDADD= @auth_libs@
updatefile_DEPENDENCIES = $(top_builddir)/libindimail.la

initsvc_SOURCES = initsvc.c
initsvc_LDADD =
initsvc_DEPENDENCIES = getEnvConfig.lo fappend.lo

install_tables_SOURCES = install_tables.c
install_tables_LDADD = @auth_libs@
install_tables_DEPENDENCIES = $(top_builddir)/libindimail.la

vtable_SOURCES = vtable.c
vtable_LDADD = @auth_libs@
vtable_DEPENDENCIES = $(top_builddir)/libindimail.la

iecho_SOURCES=iecho.c
iecho_LDADD=

addaliasdomain.lo: indimail.h config.h
add_control.lo: indimail.h config.h
add_domain_assign.lo: indimail.h config.h
add_vacation.lo: indimail.h config.h
adddomain.lo: indimail.h config.h
add_user_assign.lo: indimail.h config.h
adminCmmd.lo: indimail.h config.h
adminclient.lo: indimail.h config.h
addressToken.lo: indimail.h config.h
addusercntrl.lo: indimail.h config.h
updusercntrl.lo: indimail.h config.h
adduser.lo: indimail.h config.h
atrn_access.lo: indimail.h config.h
auth_admin.lo: indimail.h config.h
autoturn_dir.lo: indimail.h config.h
bulletin.lo: indimail.h config.h
backfill.lo: indimail.h config.h
bulk_mail.lo: indimail.h config.h
vchkpass.lo: indimail.h config.h
vsetpass.lo: indimail.h config.h
systpass.lo: indimail.h config.h
vproxy.lo: indimail.h config.h
checkPerm.lo: indimail.h config.h
Check_Login.lo: indimail.h config.h
check_quota.lo: indimail.h config.h
clearopensmtp.lo: indimail.h config.h
close_big_dir.lo: indimail.h config.h
cntrl_clearaddflag.lo: indimail.h config.h
cntrl_cleardelflag.lo: indimail.h config.h
compile_morercpthosts.lo: indimail.h config.h
CopyEmailFile.lo: indimail.h config.h
copyPwdStruct.lo: indimail.h config.h
count_dir.lo: indimail.h config.h
count_rcpthosts.lo: indimail.h config.h
CreateDomainDirs.lo: indimail.h config.h
inquerytest.lo: indimail.h config.h
inlookup.lo: indimail.h config.h
mgmtpass.lo: indimail.h config.h
dec_dir_control.lo: indimail.h config.h
del_control.lo: indimail.h config.h
del_domain_assign.lo: indimail.h config.h
deldomain.lo: indimail.h config.h
deliver_mail.lo: indimail.h config.h ismaildup.h
delunreadmails.lo: indimail.h config.h
del_user_assign.lo: indimail.h config.h
delusercntrl.lo: indimail.h config.h
deluser.lo: indimail.h config.h
evaluate.lo: evaluate.h
findhost.lo: indimail.h config.h
getFreeFS.lo: indimail.h config.h
getlastauth.lo: indimail.h config.h
getmailingList.lo: indimail.h config.h
get_Mplexdir.lo: indimail.h config.h
GetPrefix.lo: indimail.h config.h
GetSmtproute.lo: indimail.h config.h
get_smtp_service_port.lo: indimail.h config.h
GetIndiId.lo: indimail.h config.h
getindimail.lo: indimail.h config.h
host_in_locals.lo: indimail.h config.h
inc_dir_control.lo: indimail.h config.h
inc_dir.lo: indimail.h config.h
init_dir_control.lo: indimail.h config.h
is_alias_domain.lo: indimail.h config.h
is_distributed_domain.lo: indimail.h config.h
isvalid_domain.lo: indimail.h config.h
islocalif.lo: indimail.h config.h
is_user_present.lo: indimail.h config.h
isvirtualdomain.lo: indimail.h config.h
lockfile.lo: indimail.h config.h
Login_Tasks.lo: indimail.h config.h
logmysql.lo: indimail.h config.h
MailQuotaWarn.lo: indimail.h config.h
make_user_dir.lo: indimail.h config.h
md5c.lo: md5.h config.h
mdaMysqlConnect.lo: indimail.h config.h
mgmtpassfuncs.lo: indimail.h config.h
mkpasswd3.lo: indimail.h config.h
monkey.lo: indimail.h config.h
MoveFile.lo: indimail.h config.h
mysql_perror.lo: indimail.h config.h
next_big_dir.lo: indimail.h config.h
no_of_days.lo: indimail.h config.h
open_big_dir.lo: indimail.h config.h
open_master.lo: indimail.h config.h
open_relay_db.lo: indimail.h config.h
parse_email.lo: indimail.h config.h
parse_quota.lo: indimail.h config.h
passwd.lo: indimail.h config.h
pathToFilesystem.lo: indimail.h config.h
print_control.lo: indimail.h config.h
printdir.lo: indimail.h config.h
purge_files.lo: indimail.h config.h
qmail_remote.lo: indimail.h config.h
r_chown.lo: indimail.h config.h
recalc_quota.lo: indimail.h config.h
remove_line.lo: indimail.h config.h
RemoteBulkMail.lo: indimail.h config.h
remove_quotes.lo: indimail.h config.h
renameuser.lo: indimail.h config.h
r_mkdir.lo: indimail.h config.h
ScanDir.lo: indimail.h config.h
SendWelcomeMail.lo: indimail.h config.h
setsockbuf.lo: indimail.h config.h
setuserquota.lo: indimail.h config.h
indisrvr.lo: indimail.h config.h
sslerator.lo: indimail.h config.h
hostsync.lo: indimail.h config.h
signal_process.lo: indimail.h config.h
sha256_crypt.c: config.h
sha512_crypt.c: config.h
skip_relay.lo: indimail.h config.h
sockread.lo: indimail.h config.h
tcpbind.lo: indimail.h config.h
tcpopen.lo: indimail.h config.h
trashpurge.lo: indimail.h config.h
udpopen.lo: indimail.h config.h
update_file.lo: indimail.h config.h
update_newu.lo: indimail.h config.h
update_quota.lo: indimail.h config.h
update_rules.lo: indimail.h config.h
updaterules.lo: indimail.h config.h
updatefile.lo: indimail.h config.h
userinfo.lo: indimail.h config.h
user_over_quota.lo: indimail.h config.h
vacation.lo: indimail.h config.h
vaddaliasdomain.lo: indimail.h config.h
vadddomain.lo: indimail.h config.h
vadd_atrn_map.lo: indimail.h config.h
vadd_ip_map.lo: indimail.h config.h
vadduser.lo: indimail.h config.h
valias.lo: indimail.h config.h
valias_delete.lo: indimail.h config.h
valias_delete_domain.lo: indimail.h config.h
valiasinfo.lo: indimail.h config.h
valias_insert.lo: indimail.h config.h
valias_select_all.lo: indimail.h config.h
valias_select.lo: indimail.h config.h
valias_update.lo: indimail.h config.h
valiasCount.lo: indimail.h config.h
variables.lo: indimail.h config.h
vauth_active.lo: indimail.h config.h
vauth_adddomain.lo: indimail.h config.h
vauth_adduser.lo: indimail.h config.h
vauth_deldomain.lo: indimail.h config.h
vauth_deluser.lo: indimail.h config.h
vauth_getall.lo: indimail.h config.h
vauth_getflags.lo: indimail.h config.h
vauth_getpw.lo: indimail.h config.h
vauth_get_realdomain.lo: indimail.h config.h
vauth_init.lo: indimail.h config.h
create_table.lo: indimail.h config.h
dbinfo.lo: indimail.h config.h
vrenameuser.lo: indimail.h config.h
initsvc.lo: indimail.h config.h
vrenamedomain.lo: indimail.h config.h
check_group.lo: error_stack.h config.h
dbinfoAdd.lo: indimail.h config.h
dbinfoDel.lo: indimail.h config.h
count_table.lo: indimail.h config.h
dbinfoSelect.lo: indimail.h config.h
dbinfoUpdate.lo: indimail.h config.h
dbload.lo: indimail.h config.h
RelayInLookup.lo: indimail.h config.h
UserInLookup.lo: indimail.h config.h
VlimitInLookup.lo: indimail.h config.h
AliasInLookup.lo: indimail.h config.h
spam.lo: indimail.h config.h
SqlServer.lo: indimail.h config.h
vhostid_select.lo: indimail.h config.h
vhostid_insert.lo: indimail.h config.h
vhostid_update.lo: indimail.h config.h
vhostid_delete.lo: indimail.h config.h
getactualpath.lo: indimail.h config.h
getAddressBook.lo: indimail.h config.h
getDbLock.lo: indimail.h config.h
vauthOpen_user.lo: indimail.h config.h
vauth_renamedomain.lo: indimail.h config.h
layout.lo: indimail.h config.h
limits.lo: indimail.h config.h
LoadBMF.lo: indimail.h config.h
LoadDbinfo.lo: indimail.h config.h
makeseekable.lo: indimail.h config.h
pw_comp.lo: indimail.h config.h md5.h
get_local_hostid.lo: indimail.h config.h
get_local_ip.lo: indimail.h config.h
vauth_getipaddr.lo: indimail.h config.h
vauth_gethostid.lo: indimail.h config.h
maildir_to_domain.lo: indimail.h config.h
maildir_to_email.lo: indimail.h config.h
inquery.lo: indimail.h config.h
postdel.lo: indimail.h config.h
ProcessInFifo.lo: indimail.h config.h
PwdInLookup.lo: indimail.h config.h
parseAddress.lo: indimail.h config.h
storeHeader.lo: indimail.h config.h
ssl_timeout.lo: config.h
strToPw.lo: indimail.h config.h
chowkidar.lo: indimail.h config.h
findmdahost.lo: indimail.h config.h
fstabChangeCounters.lo: indimail.h config.h
FifoCreate.lo: indimail.h config.h
relay_select.lo: indimail.h config.h
vatrn.lo: indimail.h config.h
vauth_munch_domain.lo: indimail.h config.h
vauth_open.lo: indimail.h config.h
vauth_setpw.lo: indimail.h config.h
vauth_setquota.lo: indimail.h config.h
vauth_updateflag.lo: indimail.h config.h
vauth_vpasswd.lo: indimail.h config.h
vcalias.lo: indimail.h config.h
vclear_open_smtp.lo: indimail.h config.h
vclose.lo: indimail.h config.h
vcreate_dir_control.lo: indimail.h config.h
vcfilter.lo: indimail.h config.h
vdel_dir_control.lo: indimail.h config.h
vdeldomain.lo: indimail.h config.h
vdelfiles.lo: indimail.h config.h
vdel_atrn_map.lo: indimail.h config.h
vdel_ip_map.lo: indimail.h config.h
vdelivermail.lo: indimail.h config.h
vdeloldusers.lo: indimail.h config.h
vdeluser.lo: indimail.h config.h
vdominfo.lo: indimail.h config.h
vmoddomain.lo: indimail.h config.h evaluate.h
vfilter.lo: indimail.h config.h evaluate.h
vfilter_delete.lo: indimail.h config.h
vfilter_display.lo: indimail.h config.h
vfilter_header.lo: indimail.h config.h
vfilter_filterNo.lo: indimail.h config.h
vfilter_insert.lo: indimail.h config.h
vfilter_select.lo: indimail.h config.h
vfilter_update.lo: indimail.h config.h
vfstabNew.lo: indimail.h config.h
vfstab.lo: indimail.h config.h
vfstab_delete.lo: indimail.h config.h
vfstab_insert.lo: indimail.h config.h
vfstab_select.lo: indimail.h config.h
vfstab_status.lo: indimail.h config.h
vfstab_update.lo: indimail.h config.h
vget_assign.lo: indimail.h config.h
vget_ip_map.lo: indimail.h config.h
vget_lastauth.lo: indimail.h config.h
vgetpasswd.lo: indimail.h config.h
vget_real_domain.lo: indimail.h config.h
vgroup.lo: indimail.h config.h
vipmap.lo: indimail.h config.h
vlog.lo: indimail.h config.h
vmake_maildir.lo: indimail.h config.h
vmoduser.lo: indimail.h config.h
vmoveuser.lo: indimail.h config.h
vopen_smtp_relay.lo: indimail.h config.h
vpasswd.lo: indimail.h config.h
vbulletin.lo: indimail.h config.h
vpriv_select.lo: indimail.h config.h
vpriv_insert.lo: indimail.h config.h
vpriv_delete.lo: indimail.h config.h
vpriv_update.lo: indimail.h config.h
vread_dir_control.lo: indimail.h config.h
vreorg.lo: indimail.h config.h
vset_default_domain.lo: indimail.h config.h
vset_lastauth.lo: indimail.h config.h
vset_lastdeliver.lo: indimail.h config.h
vsetuserquota.lo: indimail.h config.h
vshow_atrn_map.lo: indimail.h config.h
vshow_ip_map.lo: indimail.h config.h
vsmtp.lo: indimail.h config.h
vhostid.lo: indimail.h config.h
vsmtp_delete.lo: indimail.h config.h
vsmtp_delete_domain.lo: indimail.h config.h
vsmtp_insert.lo: indimail.h config.h
vsmtp_select.lo: indimail.h config.h
vsmtp_update.lo: indimail.h config.h
vupdate_rules.lo: indimail.h config.h
vuserinfo.lo: indimail.h config.h
vlimit.lo: indimail.h config.h
cindimail.lo: indimail.h config.h
vserverinfo.lo: indimail.h config.h
filewrt.lo: indimail.h config.h
vwrite_dir_control.lo: indimail.h config.h
wildmat.lo: indimail.h config.h
Dirname.lo: indimail.h config.h
vauth_delaliasdomain.lo: indimail.h config.h
vauth_insertaliasdomain.lo: indimail.h config.h
vcreate_get_realdomain.lo: indimail.h config.h
vupd_atrn_map.lo: indimail.h config.h
vupd_ip_map.lo: indimail.h config.h
proxylogin.lo: indimail.h config.h
proxypop3.lo: indimail.h config.h
proxyimap.lo: indimail.h config.h
MakeArgs.lo: indimail.h config.h
vpriv.lo: indimail.h config.h
hostcntrl.lo: indimail.h config.h
hostcntrl_select.lo: indimail.h config.h
runcmmd.lo: indimail.h config.h
getEnvConfig.lo: indimail.h config.h
set_mysql_options.lo: indimail.h config.h
unsetenv.lo: config.h
ipchange.lo: indimail.h config.h
resetquota.lo: indimail.h config.h
osh.lo: config.h osh.h
exp.lo: config.h osh.h
handlers.lo: config.h osh.h
install_tables.lo: indimail.h config.h
vtable.lo: config.h
error_stack.lo: error_stack.c error_stack.h config.h
post_handle.lo: post_handle.c config.h
randltr.lo: indimail.h config.h
in_crypt.lo: indimail.h config.h
hmac_md5.lo: md5.h global.h
md5c.lo: md5.h global.h
md5_crypt.lo: md5.h global.h


edit = sed \
	-e 's,@indimaildir\@,$(indimaildir),g' \
	-e 's,@qmaildir\@,$(qmaildir),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@controldir\@,$(controldir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@domaindir\@,$(domaindir),g' \
	-e 's,@mysqlbindir\@,$(mysqlbindir),g' \
	-e 's,@moduledir\@,$(moduledir),g' \
	-e 's,@auth_inc\@,$(auth_inc),g' \
	-e 's,@auth_libs\@,$(auth_libs),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@basepath\@,$(basepath),g' \
	-e 's,@mysql_prefix\@,$(mysql_prefix),g' \
	-e 's,@mysql_incdir\@,$(mysql_incdir),g' \
	-e 's,@mysql_libdir\@,$(mysql_libdir),g' \
	-e 's,@defaultdomain\@,$(defaultdomain),g' \
	-e 's,@PACKAGE\@,$(PACKAGE),g' \
	-e 's,@PACKAGE_VERSION\@,$(PACKAGE_VERSION),g' \
	-e 's,@version\@,$(VERSION),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@pkgconfigdir\@,$(pkgconfigdir),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@DESTDIR\@,$(DESTDIR),g' \
	-e 's,@HOST\@,$(host),g' \
	-e 's,@DATE\@,$(DATE),g' \
	-e 's,@prefix\@,$(prefix),g'
editman = sed \
	-e 's,INDIMAILDIR,$(indimaildir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@qmaildir\@,$(qmaildir),g' \
	-e 's,CONTROLDIR,$(controldir),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@prefix\@,$(prefix),g'

indimail.fc: indimail.fc.in
	$(edit) $@.in > $@
controlsync: controlsync.in
	$(edit) $@.in > $@
mailzipper: mailzipper.in
	$(edit) $@.in > $@
vcaliasrev: vcaliasrev.in
	$(edit) $@.in > $@
myslave: myslave.in
	$(edit) $@.in > $@
indimail.mrtg.cfg: indimail.mrtg.cfg.in
	$(edit) $@.in > $@
overquota.sh: overquota.sh.in
	$(edit) $@.in > $@
authgeneric: authgeneric.in
	$(edit) $@.in > $@
bogofilter-qfe: bogofilter-qfe.in
	$(edit) $@.in > $@
cronlist.i: cronlist.in
	(echo "# DO NOT EDIT THIS FILE - edit the master and reinstall."; $(edit) cronlist.in) > $@
indimail-permissions.easy: indimail-permissions.easy.in
	$(edit) $@.in > $@
indimail-permissions.secure: indimail-permissions.secure.in
	$(edit) $@.in > $@
indimail-permissions.paranoid: indimail-permissions.paranoid.in
	$(edit) $@.in > $@
perm_list.i: perm_list.in
	$(edit) perm_list.in > $@
usr.sbin.mysqld: usr.sbin.mysqld.in
	$(edit) $@.in > $@
usr.sbin.clamd: usr.sbin.clamd.in
	$(edit) $@.in > $@
usr.bin.freshclam: usr.bin.freshclam.in
	$(edit) $@.in > $@

debian/Makefile: debian/Makefile.in
	$(edit) $@.in > $@;

INSTALL: INSTALL.in configure.ac
	$(edit) $@.in > doc/$@.indimail; rm -f $@;ln -sf doc/$@.indimail $@
Quick-INSTALL: Quick-INSTALL.in configure.ac
	$(edit) $@.in > doc/$@; rm -f $@;ln -sf doc/$@ $@
INSTALL-RPM: INSTALL-RPM.in configure.ac
	$(edit) $@.in > doc/$@; rm -f $@;ln -sf doc/$@ $@
INSTALL-MYSQL: INSTALL-MYSQL.in configure.ac
	$(edit) $@.in > doc/$@; rm -f $@;ln -sf doc/$@ $@
RELEASE-Notes: RELEASE-Notes.in configure.ac
	$(edit) $@.in > doc/$@; rm -f $@;ln -sf doc/$@ $@
README-CLUSTER: README-CLUSTER.in configure.ac
	$(edit) $@.in > doc/$@; rm -f $@;ln -sf doc/$@ $@
README: README.in configure.ac
	$(edit) $@.in > doc/$@; rm -f $@;ln -sf doc/$@ $@

post_handle:
	mkdir -p post_handle
vadddomain_handle: vadddomain_handle.in post_handle
	$(edit) $@.in > post_handle/vadddomain; ln -sf post_handle/vadddomain $@
vaddaliasdomain_handle: vaddaliasdomain_handle.in post_handle
	$(edit) $@.in > post_handle/vaddaliasdomain; ln -sf post_handle/vaddaliasdomain $@
vrenamedomain_handle: vrenamedomain_handle.in post_handle
	$(edit) $@.in > post_handle/vrenamedomain; ln -sf post_handle/vrenamedomain $@
vdeldomain_handle: vdeldomain_handle.in post_handle
	$(edit) $@.in > post_handle/vdeldomain; ln -sf post_handle/vdeldomain $@
vadduser_handle: vadduser_handle.in post_handle
	$(edit) $@.in > post_handle/vadduser; ln -sf post_handle/vadduser $@
vpasswd_handle: vpasswd_handle.in post_handle
	$(edit) $@.in > post_handle/vpasswd; ln -sf post_handle/vpasswd $@
vmoduser_handle: vmoduser_handle.in post_handle
	$(edit) $@.in > post_handle/vmoduser; ln -sf post_handle/vmoduser $@
vdeluser_handle: vdeluser_handle.in post_handle
	$(edit) $@.in > post_handle/vdeluser; ln -sf post_handle/vdeluser $@
vrenameuser_handle: vrenameuser_handle.in post_handle
	$(edit) $@.in > post_handle/vrenameuser; ln -sf post_handle/vrenameuser $@

indimail.spec: indimail.spec.in catChangeLog doc/ChangeLog.indimail
	($(edit) $@.in;./catChangeLog) | $(edit) > $@
indimail-rpmlintrc: indimail-rpmlintrc.in
	$(edit) indimail-rpmlintrc.in > $@
login_tasks.3: login_tasks.9
	$(editman) login_tasks.9 > $@
dbinfo.8: dbinfo.9
	cat $^ | sed s}INDIMAIL}$(indimaildir)}g > $@
inlookup.8: inlookup.9
	$(editman) inlookup.9 > $@
vcfilter.1: vcfilter.9
	cat $^ | sed s}SYSCONFDIR}$(sysconfdir)}g > $@
vadddomain.1:vadddomain.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g \
		| sed s}INDIMAILDIR}$(indimaildir)}g > $@
vdeldomain.1:vdeldomain.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vadduser.1:vadduser.9
	cat $^ | sed s}INDIMAIL}$(indimaildir)}g \
		| sed s}LIBEXECDIR}$(libexecdir)}g > $@
vdeluser.1:vdeluser.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vrenamedomain.1:vrenamedomain.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vrenameuser.1:vrenameuser.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vpasswd.1:vpasswd.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vmoduser.1:vmoduser.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vmoveuser.1:vmoveuser.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vdelivermail.8:vdelivermail.9
	cat $^ | sed s}INDIMAIL}$(indimaildir)}g \
		| sed s}LIBEXECDIR}$(libexecdir)}g > $@
indimail.7:indimail.9
	$(editman) indimail.9 > $@

crc.$(OBJEXT): crc.c
	$(COMPILE) $(DEFS) -DMAIN -c crc.c
crc$(EXEEXT): $(crc_DEPENDENCIES) $(crc_OBJECTS)
	$(LINK) $@.$(OBJEXT) $(crc_LDADD)

iecho.$(OBJEXT): iecho.c
	$(COMPILE) $(DEFS) -c iecho.c
iecho$(EXEEXT): $(iecho_DEPENDENCIES) $(iecho_OBJECTS)
	$(LINK) $@.$(OBJEXT) $(iecho_LDADD)

osh$(EXEEXT): $(osh_DEPENDENCIES) $(osh_OBJECTS)
	$(LINK) $@.$(OBJEXT) exp.$(OBJEXT) handlers.$(OBJEXT) getEnvConfig.$(OBJEXT) $(osh_LDADD)

initsvc$(EXEEXT): $(initsvc_OBJECTS) $(initsvc_DEPENDENCIES)
	$(LINK) $(initsvc_LDFLAGS) $(initsvc_OBJECTS) fappend.$(OBJEXT) getEnvConfig.$(OBJEXT) $(initsvc_LDADD)

install-data-local: initsvc libindimail.la indimail-rpmlintrc \
indimail.spec iauth.la indimail.te indimail.fc usr.sbin.mysqld usr.sbin.clamd usr.bin.freshclam
	if [ " $(DESTDIR)" = " " ] ; then \
		if test `@idcommand@` != "0" ; then \
			echo "you are not root"; \
			exit 1; \
		fi ; \
		echo "Shutting down IndiMail"; \
		./initsvc -off || exit 0; \
	fi;

# This is the last step in indimail installation. Post this installation will start in qmail
# Hence we still cannot start svscan.
install-data-hook: 
	$(INSTALL_DATA) config.h $(DESTDIR)@prefix@/include/indimail/indimail_config.h || exit 1
	$(INSTALL_DATA) indimail.h $(DESTDIR)@prefix@/include/indimail/indimail.h || exit 1
	$(INSTALL_DATA) typesx.h $(DESTDIR)@prefix@/include/indimail/typesx.h || exit 1
	$(INSTALL_DATA) perm_list.i $(DESTDIR)@sysconfdir@/perm_list.i  || exit 1
	$(mkinstalldirs) $(DESTDIR)@shareddir@/modules || exit 1
	$(INSTALL_PROGRAM) $(DESTDIR)@libdir@/iauth.so  $(DESTDIR)@prefix@/lib/indimail/modules || exit 1
	/bin/rm -f $(DESTDIR)@libdir@/iauth.* || exit 1
	$(mkinstalldirs) $(DESTDIR)@libexecdir@ || exit 1
	$(INSTALL_DATA) post_handle/* $(DESTDIR)@libexecdir@ || exit 1
	for f in $(post_handle_list); do \
		if [ -f $(DESTDIR)@libexecdir@/$$f ] ; then \
			chmod 755 $(DESTDIR)@libexecdir@/$$f; \
			echo "chmod 755 $(DESTDIR)@libexecdir@/$$f"; \
		fi;\
	done
	$(mkinstalldirs) $(DESTDIR)@shareddir@/doc || exit 1
	for f in $(doc_list); do \
		$(INSTALL_DATA) doc/$$f $(DESTDIR)@shareddir@/doc/$$f || exit 1 ;\
	done
	if test `@idcommand@` = "0" ; then \
		for f in $(setuid_list); do \
			if [ -f $(DESTDIR)@prefix@/bin/$$f ] ; then \
				echo "chown root $(DESTDIR)@prefix@/bin/$$f"; \
				chown root $(DESTDIR)@prefix@/bin/$$f; \
				echo "chmod 4555 $(DESTDIR)@prefix@/bin/$$f"; \
				chmod 4555 $(DESTDIR)@prefix@/bin/$$f; \
			fi;  \
		done; \
	fi
	if test " $(DISTRIB_ID)" = " Ubuntu" -o " $(DISTRIB_ID)" = " Debian" ; then \
		$(mkinstalldirs) $(DESTDIR)@sysconfdir@/apparmor.d/local || exit 1; \
		$(INSTALL_DATA) usr.sbin.mysqld $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.sbin.mysqld  || exit 1; \
		$(INSTALL_DATA) usr.sbin.clamd $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.sbin.clamd  || exit 1; \
		$(INSTALL_DATA) usr.bin.freshclam $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.bin.freshclam  || exit 1; \
	fi
	if test " $(DISTRIB_ID)" != " Ubuntu" -a " $(DISTRIB_ID)" != " Debian" ; then \
		$(INSTALL_DATA) indimail.te $(DESTDIR)@sysconfdir@/indimail.te  || exit 1; \
		$(INSTALL_DATA) indimail.fc $(DESTDIR)@sysconfdir@/indimail.fc  || exit 1; \
	fi

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

AUTOMAKE_OPTIONS = foreign no-dependencies
