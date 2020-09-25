# Don't edit Makefile! Use conf-* for configuration.
COFLAGS=-z+05:30
SHELL=/bin/sh
servicedir=/service
include Makevars.$(SYSTEM)

edit = sed \
	-e 's,@indimaildir\@,$(qmaildir),g' \
	-e 's,@qmaildir\@,$(qmaildir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@qsysconfdir\@,$(qsysconfdir),g' \
	-e 's,@sysconfdir\@,$(qsysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@HOST\@,$(HOST),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@ucspitcp_version\@,$(ucspitcp_version),g' \
	-e 's,@DATE\@,$(DATE),g'

it = perm_list.mta 822date 822field 822header 822print 822received addcr \
	addrlist argv0 atrn autoresponder qbase64 batv binm1 binm1+df cronlist.q \
	binm2 binm2+df binm3 binm3+df bouncesaying cdbdump cdbget cdbgetm qnotify \
	create_services cdbmake cdbmake-12 cdbmake-sv cdbstats cdbtest cidr \
	condredirect config-fast datemail delcr dk-filter dknewkey dktest dnscname \
	dnsfq dnsip svctool qfrontend dnsmxip dnsptr dnstxt dot-forward drate elq \
	envdir envuidgid qdane dnstlsarr etrn except fastforward fghack fixcrio \
	forward generic.so greydaemon qf-smtp-ratelimit home home+df hostname \
	idedit iftocc indimail.plist instcheck ipmeprint ldap-checkpwd leapsecs \
	logselect maildir2mbox maildircmd maildirdeliver maildirqmtp maildirserial \
	maildirsmtp maildirwatch mailsubj mbox2maildir multilog multirotate \
	inewaliases newinclude new-inject nowutc ofmipd ofmipname parsedate \
	pgrphack pinq plugtest installer predate preline printforward \
	printmaillist proc proc+df qaes qail ctrlenv qarf qmail-cat qmail-cdb \
	qmail-clean qmailconfig qmailctl maildirsize qmail-daemon qmaildirmake \
	qmail-dk qmail-dkim qmail-getpw qmail-greyd qmail-inject qmail-local \
	qmail-lspawn qmail-multi qmail-newmrh qmail-newu qmail-nullqueue \
	qmail-popbull qmail-poppass qmail-pw2u qmail-qfilter qmail-qmqpc \
	qmail-qmqpd qmail-qmtpd qmail-qread qmail-queue qmail-remote qmail-rm \
	qmail-rspawn qmail-send qmail-showctl qmail-daned \
	qmail-smtpd qmail_smtpd.so qmail-start qmail-tcpok qmail-tcpto \
	qmail-todo qmail-upq qpq qquote qreceipt qsmhook queue-fix readproctitle \
	recipient-cdb recordio relaytest rewind irmail rpmattr rrforward rrt \
	sendmail serialcmd serialqmtp serialsmtp setforward setlock setmaillist \
	setuidgid smtpd-plugin0.so smtpd-plugin.so softlimit sortedtest \
	spawn-filter spfquery splogger srsfilter supervise surblfilter surblqueue \
	svc svok svscan svscanboot svstat swaks sys-checkpwd systemd tai2tai64n \
	tai64n tai64n2tai tai64nlocal tai64nunix tcp-env testzero tokenize uacl \
	udpclient udplogger update_tmprsadh upstart viruscanner whois yearcal \
	cdb-database analyzer qtools qlog leapsecs.dat

man = drate.1 qarf.1 drate.1 qnotify.1 rrt.1 plugin_init.3 \
	svscan.8 cdb-database.8 ctrlenv.8 plugtest.1 surblfilter.8 \
	svctool.8 predate.1 datemail.1 \
	mlmatchup.8 cdbdump.1 cdbmake.1 cdbget.1 cdbgetm.1 \
	logselect.8 qmail-daned.8 qmail-srs.5 qmail-limits.7 \
	forward.1 condredirect.1 qmail-control.5 rrforward.1 \
	qreceipt.1 maildirserial.1 qmail-rm.1 dot-qmail.5 \
	qmail-users.5 dot-forward.1 fastforward.1 new-inject.1 \
	filterto.1 replier.1 svscanboot.8 qmail-local.8 \
	qmail-getpw.8 qmail-remote.8 qmail-send.8 qmail-todo.8 \
	qmail-start.8 qmail-queue.8 qmail-dkim.8 dk-filter.8 \
	qmail-dk.8 dktest.8 qmail-multi.8 spawn-filter.8 \
	qmail-inject.8 qmail-newmrh.8 recipient-cdb.8 qmail-cdb.8 \
	qmail-newu.8 qmail-pw2u.8 qmail-qread.8 qmail-smtpd.8 \
	qmail-poppass.8 qscanq.8 run-cleanq.8

#default: $(it) $(man)

all: $(it) $(man)


include Makefile.$(SYSTEM)

compile: make-compile warn-auto.sh systype conf-cc conf-cc-$(SYSTEM)
	( cat warn-auto.sh; ./make-compile "`cat systype`" ) > \
	compile
	chmod 755 compile

make-compile: \
make-compile.sh auto-ccld.sh
	cat auto-ccld.sh make-compile.sh > make-compile
	chmod 755 make-compile

auto-ccld.sh: conf-cc conf-ld warn-auto.sh
	(cat warn-auto.sh; echo CC=\'`head -1 conf-cc`\'; \
	if test $(SYSTEM) = DARWIN ; then \
		echo LD=\'`head -1 conf-ld | sed s{-s{{`\'; \
	else \
		echo LD=\'`head -1 conf-ld`\'; \
	fi) > auto-ccld.sh

make-load: \
make-load.sh auto-ccld.sh
	cat auto-ccld.sh make-load.sh > make-load
	chmod 755 make-load

make-makelib: \
make-makelib.sh auto-ccld.sh
	cat auto-ccld.sh make-makelib.sh > make-makelib
	chmod 755 make-makelib

makelib: \
make-makelib warn-auto.sh systype
	( cat warn-auto.sh; ./make-makelib "`cat systype`" ) > \
	makelib
	chmod 755 makelib

libsyncdir.a: makelib syncdir.o
	./makelib libsyncdir.a syncdir.o

syncdir.o: compile syncdir.c conf-sync
	./compile `grep -h -v "^#" conf-sync` syncdir.c

auto-gid: load auto-gid.o
	./load auto-gid -lqmail

auto-gid.o: compile auto-gid.c
	./compile auto-gid.c

auto-int: load auto-int.o
	./load auto-int -lqmail

auto-int.o: compile auto-int.c
	./compile auto-int.c

auto-int8: load auto-int8.o
	./load auto-int8 -lqmail

auto-int8.o: compile auto-int8.c
	./compile auto-int8.c

auto-str: load auto-str.o
	./load auto-str -lqmail

auto-str.o: compile auto-str.c
	./compile auto-str.c

auto-uid: load auto-uid.o
	./load auto-uid -lqmail

auto-uid.o: compile auto-uid.c
	./compile auto-uid.c

auto_break.c: auto-str conf-break
	./auto-str auto_break \
	"`head -1 conf-break`" > auto_break.c

auto_break.o: compile auto_break.c
	./compile auto_break.c

auto_patrn.c: auto-int8 conf-patrn
	./auto-int8 auto_patrn `head -1 conf-patrn` > auto_patrn.c

auto_patrn.o: compile auto_patrn.c
	./compile auto_patrn.c

auto_qmail.c: auto-str conf-qmail
	./auto-str auto_qmail `head -1 conf-qmail` > auto_qmail.c

auto_qmail.o: compile auto_qmail.c
	./compile auto_qmail.c

auto_libexec.c: auto-str conf-libexec
	./auto-str auto_libexec `head -1 conf-libexec` > auto_libexec.c

auto_libexec.o: compile auto_libexec.c
	./compile auto_libexec.c

auto_control.c: auto-str conf-sysconfdir
	./auto-str auto_control "`head -1 conf-sysconfdir`/control" > auto_control.c

auto_control.o: compile auto_control.c
	./compile auto_control.c

auto_assign.c: auto-str conf-sysconfdir
	./auto-str auto_assign "`head -1 conf-sysconfdir`/users" > auto_assign.c

auto_assign.o: compile auto_assign.c
	./compile auto_assign.c

auto_shared.c: auto-str conf-shared
	./auto-str auto_shared `head -1 conf-shared` > auto_shared.c

auto_shared.o: compile auto_shared.c
	./compile auto_shared.c

auto_sysconfdir.c: auto-str conf-sysconfdir
	./auto-str auto_sysconfdir `head -1 conf-sysconfdir` > auto_sysconfdir.c

auto_sysconfdir.o: compile auto_sysconfdir.c
	./compile auto_sysconfdir.c

auto_prefix.c: auto-str conf-prefix
	./auto-str auto_prefix `head -1 conf-prefix` > auto_prefix.c

auto_prefix.o: compile auto_prefix.c
	./compile auto_prefix.c

auto_spawn.c: auto-int conf-spawn
	./auto-int auto_spawn `head -1 conf-spawn` > auto_spawn.c

auto_spawn.o: compile auto_spawn.c
	./compile auto_spawn.c

auto_split.c: auto-int conf-split
	./auto-int auto_split `head -1 conf-split` > auto_split.c

auto_split.o: compile auto_split.c
	./compile auto_split.c

auto_uids.h: conf-users conf-groups
	( \
	echo "/*"; \
	echo " * WARNING: This file was auto-generated. Do not edit!"; \
	echo " * edit conf-users and conf-groups instead"; \
	echo " * To add new user/groups, you will have to modify"; \
	echo " * get_uids.c, Makefile (rule for auto_uids.h), conf-users, conf-groups"; \
	echo " */"; \
	echo "#ifndef AUTO_UIDS_H"; \
	echo "#define AUTO_UIDS_H"; \
	echo ""; \
	echo "#define ALIASU    \"`head -1 conf-users  | tail -1`\""; \
	echo "#define QMAILD    \"`head -2 conf-users  | tail -1`\""; \
	echo "#define QMAILL    \"`head -3 conf-users  | tail -1`\""; \
	echo "#define ROOTUSER  \"`head -4 conf-users  | tail -1`\""; \
	echo "#define QMAILP    \"`head -5 conf-users  | tail -1`\""; \
	echo "#define QMAILQ    \"`head -6 conf-users  | tail -1`\""; \
	echo "#define QMAILR    \"`head -7 conf-users  | tail -1`\""; \
	echo "#define QMAILS    \"`head -8 conf-users  | tail -1`\""; \
	echo "#define INDIUSER  \"`head -9 conf-users  | tail -1`\""; \
	echo "#define QSCANDU   \"`head -10 conf-users | tail -1`\""; \
	echo "#define QMAILG    \"`head -1 conf-groups | tail -1`\""; \
	echo "#define NOFILESG  \"`head -2 conf-groups | tail -1`\""; \
	echo "#define INDIGROUP \"`head -3 conf-groups | tail -1`\""; \
	echo "#define QSCANDG   \"`head -4 conf-groups | tail -1`\""; \
	echo ""; \
	echo "extern int auto_uida;" ;\
	echo "extern int auto_uidd;" ;\
	echo "extern int auto_uidl;" ;\
	echo "extern int auto_uido;" ;\
	echo "extern int auto_uidp;" ;\
	echo "extern int auto_uidq;" ;\
	echo "extern int auto_uidr;" ;\
	echo "extern int auto_uids;" ;\
	echo "extern int auto_uidv;" ;\
	echo "extern int auto_uidc;" ;\
	echo ""; \
	echo "extern int auto_gidq;" ;\
	echo "extern int auto_gidn;" ;\
	echo "extern int auto_gidv;" ;\
	echo "extern int auto_gidc;" ;\
	echo ""; \
	echo "int             uidinit(int);"; \
	echo ""; \
	echo "#endif"; \
	) > auto_uids.h

auto_uids.c: auto-uid auto-gid conf-users conf-groups
	(./auto-uid auto_uidv `head -9  conf-users | tail -1` \
	&& ./auto-uid auto_uida `head -1  conf-users` \
	&&./auto-uid auto_uidd `head -2  conf-users | tail -1` \
	&&./auto-uid auto_uidl `head -3  conf-users | tail -1` \
	&&./auto-uid auto_uidp `head -5  conf-users | tail -1` \
	&&./auto-uid auto_uidq `head -6  conf-users | tail -1` \
	&&./auto-uid auto_uidr `head -7  conf-users | tail -1` \
	&&./auto-uid auto_uids `head -8  conf-users | tail -1` \
	&&./auto-uid auto_uidc `head -10 conf-users | tail -1` \
	&&./auto-uid auto_uido `head -4  conf-users | tail -1` \
	&&./auto-gid auto_gidv `head -3  conf-groups | tail -1` \
	&&./auto-gid auto_gidq `head -1  conf-groups` \
	&&./auto-gid auto_gidn `head -2  conf-groups | tail -1` \
	&&./auto-gid auto_gidc `head -4  conf-groups | tail -1` \
	) > auto_uids.c.tmp && mv auto_uids.c.tmp auto_uids.c

auto_uids.o: compile auto_uids.c
	./compile auto_uids.c

auto_usera.c: auto-str conf-users
	./auto-str auto_usera `head -1 conf-users` > auto_usera.c

auto_usera.o: compile auto_usera.c
	./compile auto_usera.c

md5c.o: compile md5c.c md5.h global.h
	./compile md5c.c

hmac_md5.o: compile hmac_md5.c auth_cram.h global.h
	./compile hmac_md5.c

hmac_sha1.o: compile hmac_sha1.c sha1.h typesx.h
	./compile hmac_sha1.c

hmac_sha256.o: compile hmac_sha256.c sha1.h typesx.h
	./compile hmac_sha256.c

hmac_sha512.o: compile hmac_sha512.c sha1.h typesx.h
	./compile hmac_sha512.c

sha1.o: compile sha1.c sha1.h
	./compile sha1.c

hmac_ripemd.o: compile hmac_ripemd.c ripemd.h typesx.h
	./compile hmac_ripemd.c

ripemd.o: compile ripemd.c ripemd.h
	./compile ripemd.c

digest_md5.o: compile digest_md5.c global.h md5.h
	./compile digest_md5.c

swaks: swaks.pl conf-qmail
	cat swaks.pl \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> swaks
	chmod 755 swaks

qlogselect: qlogselect.pl conf-qmail
	cat qlogselect.pl \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qlogselect
	chmod 755 qlogselect

binm1: \
binm1.sh conf-qmail
	cat binm1.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm1

binm1+df: \
binm1+df.sh conf-prefix
	cat binm1+df.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> binm1+df

binm2: \
binm2.sh conf-prefix
	cat binm2.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> binm2

binm2+df: \
binm2+df.sh conf-prefix
	cat binm2+df.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> binm2+df

binm3: \
binm3.sh conf-prefix
	cat binm3.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> binm3

binm3+df: \
binm3+df.sh conf-prefix
	cat binm3+df.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> binm3+df

bouncesaying: load bouncesaying.o
	./load bouncesaying -lqmail

bouncesaying.o: \
compile bouncesaying.c
	./compile bouncesaying.c

cdbmss.o: \
compile cdbmss.c cdbmss.h
	./compile cdbmss.c

check: \
it man
	./instcheck

chkshsgr: load chkshsgr.o
	./load chkshsgr

chkshsgr.o: compile chkshsgr.c
	./compile chkshsgr.c

chkspawn: load chkspawn.o auto_spawn.o
	./load chkspawn auto_spawn.o -lqmail

chkspawn.o: compile chkspawn.c auto_spawn.h
	./compile chkspawn.c

clean: TARGETS
	rm -f `cat TARGETS`

choose: choose.sh warn-auto.sh
	rm -f choose
	cat warn-auto.sh choose.sh > choose
	chmod 555 choose

hasmysql.h: choose compile load mysql_config hasmysql.h1 hasmysql.h2
	./choose R "./mysql_config --include" hasmysql.h1 hasmysql.h2 > hasmysql.h

mysql_config.o: compile mysql_config.c conf-mysqlrules
	./compile `grep -h -v "^#" conf-mysqlrules` mysql_config.c

mysql_config: load mysql_config.o
	./load mysql_config

condredirect: \
load condredirect.o qmail.o envdir_set.o pathexec_env.o \
openreadclose.o auto_sysconfdir.o variables.o auto_control.o \
auto_qmail.o pathexec_run.o
	./load condredirect qmail.o envdir_set.o pathexec_env.o \
	openreadclose.o auto_sysconfdir.o variables.o auto_control.o \
	auto_qmail.o pathexec_run.o -lqmail

condredirect.o: compile condredirect.c qmail.h \
auto_sysconfdir.h pathexec.h variables.h auto_control.h
	./compile `grep -h -v "^#" conf-spf` condredirect.c

condredirect.1: condredirect.9 conf-qmail conf-sysconfdir
	cat condredirect.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> condredirect.1

qmailconfig: \
warn-auto.sh qmailconfig.sh conf-prefix conf-prefix conf-break \
conf-split conf-sysconfdir conf-libexec
	cat warn-auto.sh qmailconfig.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qmailconfig
	chmod 755 qmailconfig

qfrontend: \
warn-auto.sh qfrontend.sh conf-libexec
	cat warn-auto.sh qfrontend.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> qfrontend
	chmod 755 qfrontend

qf-smtp-ratelimit: \
warn-auto.sh qf-smtp-ratelimit.in conf-libexec
	cat warn-auto.sh qf-smtp-ratelimit.in \
	| sed s}HOME}"`head -1 conf-qmail`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> qf-smtp-ratelimit
	chmod 755 qf-smtp-ratelimit

config-fast: \
warn-auto.sh config-fast.sh conf-prefix conf-break \
conf-split conf-sysconfdir
	cat warn-auto.sh config-fast.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> config-fast
	chmod 755 config-fast

control.o: \
compile control.c control.h variables.h auto_control.h MakeArgs.h
	./compile control.c

date822fmt.o: \
compile date822fmt.c date822fmt.h
	./compile date822fmt.c

datemail: \
warn-auto.sh datemail.sh conf-prefix conf-break conf-split
	cat warn-auto.sh datemail.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> datemail

datemail.1: datemail.9 conf-prefix
	cat datemail.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> datemail.1

direntry.h: \
compile trydrent.c direntry.h1 direntry.h2
	( ./compile trydrent.c >/dev/null 2>&1 \
	&& cat direntry.h2 || cat direntry.h1 ) > direntry.h
	rm -f trydrent.o

dns.h: \
compile dns.h1 trynameser.sh
	(sh ./trynameser.sh) > dns.h

dns.lib: \
tryrsolv.c compile load socket.lib dns.o socket_v6any.o \
tlsarralloc.o socket_v4mappedprefix.o ipalloc.o ip.o strsalloc.o \
socket.lib
	( ( ./compile tryrsolv.c && ./load tryrsolv dns.o \
	socket_v6any.o socket_v4mappedprefix.o \
	tlsarralloc.o ipalloc.o ip.o strsalloc.o \
	-lresolv `cat socket.lib` -lqmail ) >/dev/null 2>&1 \
	&& echo -lresolv || exit 0 ) > dns.lib
	rm -f tryrsolv.o tryrsolv

dns.o: \
compile dns.c ip.h ipalloc.h strsalloc.h dns.h tlsarralloc.h \
conf-tls conf-spf conf-domainkeys conf-ip
	./compile `grep -h -v "^#" conf-tls conf-spf conf-domainkeys \
		conf-ip conf-tlsa` dns.c

dnscname: \
load dnscname.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o socket_v4mappedprefix.o socket_v6any.o dns.lib \
socket.lib
	./load dnscname dns.o dnsdoe.o ip.o ipalloc.o \
	strsalloc.o tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o `cat dns.lib socket.lib` -lqmail

dnscname.o: compile dnscname.c dns.h dnsdoe.h
	./compile dnscname.c

dnsdoe.o: compile dnsdoe.c dns.h dnsdoe.h
	./compile dnsdoe.c

dnsfq: \
load dnsfq.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o socket_v4mappedprefix.o \
socket_v6any.o dns.lib socket.lib
	./load dnsfq dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o  \
	tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o `cat dns.lib socket.lib` -lqmail

dnsfq.o: \
compile dnsfq.c dns.h dnsdoe.h ip.h ipalloc.h strsalloc.h \
conf-tls conf-spf conf-ip
	./compile `grep -h -v "^#" conf-tls conf-spf conf-ip` dnsfq.c

dnsip: \
load dnsip.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o socket_v4mappedprefix.o socket_v6any.o \
dns.lib socket.lib
	./load dnsip dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o `cat dns.lib socket.lib` -lqmail

dnsip.o: compile dnsip.c strsalloc.h dns.h dnsdoe.h \
ip.h ipalloc.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` dnsip.c

dnsmxip: \
load dnsmxip.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o socket_v4mappedprefix.o \
socket_v6any.o dns.lib socket.lib
	./load dnsmxip dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o `cat dns.lib socket.lib` -lqmail

dnsmxip.o: \
compile dnsmxip.c dns.h dnsdoe.h ip.h ipalloc.h strsalloc.h \
conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` dnsmxip.c

dnsptr: \
load dnsptr.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o socket_v4mappedprefix.o socket_v6any.o \
dns.lib socket.lib
	./load dnsptr dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	tlsarralloc.o socket_v4mappedprefix.o socket_v6any.o \
	`cat dns.lib socket.lib` -lqmail

dnsptr.o: \
compile dnsptr.c strsalloc.h dns.h dnsdoe.h ip.h conf-spf
	./compile `grep -h -v "^#" conf-spf` dnsptr.c

dnstxt: load dnstxt.o dns_text.o dns.lib socket.lib
	./load dnstxt dns_text.o `cat dns.lib socket.lib` \
		-lcrypto -lqmail

dnstxt.o: \
compile dnstxt.c strsalloc.h dns.h dnsdoe.h ip.h 
	./compile dnstxt.c

dnstlsarr: \
load dnstlsarr.o dns.o dnsdoe.o ip.o tlsarralloc.o ipalloc.o \
starttls.o strsalloc.o socket_v4mappedprefix.o socket_v6any.o \
auto_control.o variables.o control.o socket_tcp.o \
socket_tcp6.o timeoutconn.o socket_v6loopback.o \
socket_ip4loopback.o dns.lib socket.lib tls.lib
	./load dnstlsarr dns.o dnsdoe.o tlsarralloc.o ip.o ipalloc.o \
	starttls.o strsalloc.o socket_v4mappedprefix.o socket_v6any.o \
	auto_control.o socket_tcp.o socket_tcp6.o timeoutconn.o \
	socket_v6loopback.o socket_ip4loopback.o variables.o control.o \
	`cat dns.lib` `cat socket.lib tls.lib` -lqmail

dnstlsarr.o: \
compile dnstlsarr.c dns.h dnsdoe.h tlsarralloc.h \
hastlsa.h control.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-ip conf-tls` dnstlsarr.c

dot-qmail.5: \
dot-qmail.9 conf-break
	cat dot-qmail.9 \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	> dot-qmail.5

elq: \
warn-auto.sh elq.sh conf-prefix conf-break conf-split
	cat warn-auto.sh elq.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> elq

irmail: \
warn-auto.sh rmail.sh conf-prefix
	cat warn-auto.sh rmail.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> irmail

etrn: \
warn-auto.sh etrn.sh conf-qmail conf-libexec conf-prefix
	cat warn-auto.sh etrn.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> etrn

atrn: \
warn-auto.sh atrn.sh conf-qmail conf-sysconfdir conf-prefix
	cat warn-auto.sh atrn.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> atrn

multirotate: \
warn-auto.sh multirotate.sh conf-prefix
	cat warn-auto.sh multirotate.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> multirotate

qpq: \
warn-auto.sh qpq.sh conf-qmail conf-prefix
	cat warn-auto.sh qpq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> qpq

qmailctl: \
warn-auto.sh qmailctl.sh conf-prefix conf-sysconfdir
	(head -1 warn-auto.sh | sed  s}/bin/sh}$(bash)};cat qmailctl.sh) \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> qmailctl
	chmod +x $@

indimail.plist: indimail.plist.in conf-libexec
	cat indimail.plist.in | sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> indimail.plist

indimail-mta.fc: \
indimail-mta.fc.in conf-qmail conf-sysconfdir
	cat indimail-mta.fc.in \
	| sed s}@servicedir\@}"$(servicedir)"}g \
	| sed s}@qmaildir\@}"`head -1 conf-qmail`"}g \
	| sed s}@sysconfdir\@}"`head -1 conf-sysconfdir`"}g \
	> indimail-mta.fc

svscanboot: \
warn-auto.sh svscanboot.sh conf-prefix conf-sysconfdir conf-libexec
	cat warn-auto.sh svscanboot.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> svscanboot

upstart:upstart.in conf-prefix conf-sysconfdir
	( \
	echo "# svscan - runlevel compatibility"; \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	cat upstart.in \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	) > upstart

systemd:systemd.in conf-prefix conf-libexec
	( \
	echo "# svscan - runlevel compatibility"; \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	cat systemd.in \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	) > systemd

svscanboot.8: \
svscanboot.9 conf-prefix conf-sysconfdir conf-libexec
	cat svscanboot.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> svscanboot.8

svscan.8: \
svscan.9 conf-sysconfdir
	cat svscan.9 \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> svscan.8

cronlist.q:cronlist.in conf-prefix conf-libexec
	( \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	cat cronlist.in \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	) > cronlist.q

except: \
load except.o
	./load except -lqmail

except.o: \
compile except.c
	./compile except.c

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

fmtqfn.o: compile fmtqfn.c fmtqfn.h auto_split.h
	./compile fmtqfn.c

forward: \
load forward.o auto_qmail.o qmail.o variables.o \
openreadclose.o envdir_set.o pathexec_env.o pathexec_run.o \
auto_sysconfdir.o auto_control.o
	./load forward auto_qmail.o qmail.o variables.o \
	openreadclose.o envdir_set.o pathexec_env.o pathexec_run.o \
	auto_sysconfdir.o auto_control.o -lqmail

forward.1: forward.9 conf-sysconfdir
	cat forward.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> forward.1

forward.o: compile forward.c qmail.h variables.h \
envdir.h pathexec.h auto_sysconfdir.h auto_control.h
	./compile `grep -h -v "^#" conf-spf` forward.c

gfrom.o: \
compile gfrom.c gfrom.h
	./compile gfrom.c

hasnpbg1.h: \
trynpbg1.c compile load
	( ( ./compile trynpbg1.c \
	&& ./load trynpbg1 -lqmail && ./trynpbg1 ) \
	>/dev/null 2>&1 \
	&& echo \#define HASNAMEDPIPEBUG1 1 || exit 0 ) > \
	hasnpbg1.h
	rm -f trynpbg1.o trynpbg1

hassalen.h: \
trysalen.c compile
	( ./compile trysalen.c >/dev/null 2>&1 \
	&& echo \#define HASSALEN 1 || exit 0 ) > hassalen.h
	rm -f trysalen.o

hassgact.h: \
trysgact.c compile load
	( ( ./compile trysgact.c && ./load trysgact ) >/dev/null \
	2>&1 \
	&& echo \#define HASSIGACTION 1 || exit 0 ) > hassgact.h
	rm -f trysgact.o trysgact

hassgprm.h: \
trysgprm.c compile load
	( ( ./compile trysgprm.c && ./load trysgprm ) >/dev/null \
	2>&1 \
	&& echo \#define HASSIGPROCMASK 1 || exit 0 ) > hassgprm.h
	rm -f trysgprm.o trysgprm

hasshsgr.h: \
chkshsgr warn-shsgr tryshsgr.c compile load
	./chkshsgr || ( cat warn-shsgr; exit 1 )
	( ( ./compile tryshsgr.c \
	&& ./load tryshsgr && ./tryshsgr ) >/dev/null 2>&1 \
	&& echo \#define HASSHORTSETGROUPS 1 || exit 0 ) > \
	hasshsgr.h
	rm -f tryshsgr.o tryshsgr

headerbody.o: \
compile headerbody.c hfield.h headerbody.h
	./compile headerbody.c

hfield.o: \
compile hfield.c hfield.h
	./compile hfield.c

hier.o: \
compile hier.c auto_qmail.h auto_split.h auto_uids.h \
qmail-todo.h tcpto.h hassrs.h conf-domainkeys conf-srs \
conf-spf conf-dkim conf-batv auto_shared.h auto_control.h \
auto_assign.h auto_sysconfdir.h auto_prefix.h conf-plugin conf-dlopen
	./compile `grep -h -v "^#" conf-tls conf-spf conf-domainkeys conf-dkim \
	conf-batv conf-plugin conf-dlopen conf-tlsa` hier.c

home: \
home.sh conf-prefix
	cat home.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> home

home+df: \
home+df.sh conf-prefix
	cat home+df.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> home+df

hostname: load hostname.o
	./load hostname -lqmail

hostname.o: \
compile hostname.c
	./compile hostname.c

idedit: load idedit.o
	./load idedit -lqmail

idedit.o: \
compile idedit.c
	./compile idedit.c

instcheck: \
load instcheck.o hier.o auto_qmail.o get_uid.o auto_uids.o \
auto_sysconfdir.o auto_shared.o auto_control.o \
auto_assign.o auto_prefix.o auto_libexec.o
	./load instcheck hier.o auto_qmail.o get_uid.o \
	auto_sysconfdir.o auto_prefix.o auto_shared.o \
	auto_control.o auto_assign.o auto_uids.o auto_libexec.o -lqmail

instcheck.o: \
compile instcheck.c
	if test -f conf-logdir ; then \
		./compile instcheck.c -DLOGDIR="\"`cat conf-logdir`\""; \
	else \
		./compile instcheck.c -DLOGDIR=\"/var/log/svc\"; \
	fi

ip.o: \
compile ip.c ip.h conf-ip
	./compile `grep -h -v "^#" conf-ip` ip.c

ipalloc.o: \
compile ipalloc.c ip.h ipalloc.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` ipalloc.c

tlsarralloc.o: \
compile tlsarralloc.c tlsarralloc.h
	./compile tlsarralloc.c

ipme.o: \
compile ipme.c hassalen.h ip.h ipalloc.h \
ipme.h hasifaddr.h ipalloc.h variables.h \
conf-tls conf-ip conf-ipme auto_control.h
	./compile `grep -h -v "^#" conf-tls conf-ip conf-ipme` ipme.c

ipmeprint: \
load ipmeprint.o ipme.o ip.o ipalloc.o strsalloc.o \
auto_qmail.o socket_v4mappedprefix.o socket_v6any.o \
auto_control.o variables.o socket.lib
	./load ipmeprint ipme.o ip.o ipalloc.o strsalloc.o \
	auto_qmail.o auto_control.o socket_v4mappedprefix.o \
	socket_v6any.o variables.o `cat socket.lib` -lqmail

ipmeprint.o: compile ipmeprint.c ipme.h ip.h ipalloc.h \
auto_qmail.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` ipmeprint.c

viruscanner: \
qscanq cleanq run-cleanq qscanq-stdin qhpsi

qhpsi: \
load qhpsi.o auto_qmail.o get_uid.o auto_uids.o
	./load qhpsi auto_qmail.o get_uid.o auto_uids.o \
	-ldl -lqmail

qhpsi.o: compile qhpsi.c auto_qmail.h auto_uids.h
	./compile qhpsi.c

qscanq: load qscanq.o auto_uids.o get_uid.o \
timestamp.o auto_spool.o mkfn.o do_cleanq.o \
auto_cleaner.o auto_qstdin.o fmt_xpid.o
	./load qscanq auto_uids.o get_uid.o timestamp.o \
	fmt_xpid.o mkfn.o auto_spool.o do_cleanq.o \
	auto_cleaner.o auto_qstdin.o -lqmail

qscanq.o: compile qscanq.c auto_spool.h auto_fnlen.h mkfn.h \
auto_uids.h auto_retries.h
	./compile qscanq.c

fmt_xpid.o: compile fmt_xpid.c
	./compile fmt_xpid.c

timestamp.o: compile timestamp.c
	./compile timestamp.c

mkfn.o: compile mkfn.c timestamp.h auto_pidt.h
	./compile mkfn.c

do_cleanq.o: compile do_cleanq.c auto_cleaner.h
	./compile do_cleanq.c

auto_pidt.h: auto-pidt
	./auto-pidt > auto_pidt.h.tmp && mv auto_pidt.h.tmp auto_pidt.h

auto-pidt: \
load auto-pidt.o
	./load auto-pidt -lqmail

auto-pidt.o: compile auto-pidt.c
	./compile auto-pidt.c

auto_spool.c: auto-str conf-scandir
	./auto-str auto_spool "`head -1 conf-scandir`"/root/scanq > \
	auto_spool.c.tmp && mv auto_spool.c.tmp auto_spool.c

auto_spool.o: compile auto_spool.c
	./compile auto_spool.c

auto-fnlen: load auto-fnlen.o
	./load auto-fnlen -lqmail

auto-fnlen.o: compile auto-fnlen.c timestamp.h auto_pidt.h
	./compile auto-fnlen.c

auto_fnlen.h: auto-fnlen
	./auto-fnlen > auto_fnlen.h.tmp && mv auto_fnlen.h.tmp auto_fnlen.h

auto_retries.h: conf-retries
	echo "#define MAX_RETRIES" "`head -1 < conf-retries`" > \
	auto_retries.h.tmp && mv auto_retries.h.tmp auto_retries.h

auto_cleaner.c: auto-str conf-qmail
	./auto-str auto_runcleaner "`head -1 conf-prefix`"/sbin/run-cleanq >> \
	auto_cleaner.c.tmp && mv auto_cleaner.c.tmp auto_cleaner.c

auto_cleaner.o: compile auto_cleaner.c
	./compile auto_cleaner.c

auto_qstdin.c: auto-strarr conf-qmail
	./auto-strarr auto_qstdin "`head -1 conf-prefix`"/sbin/qscanq-stdin > \
	auto_qstdin.c.tmp && mv auto_qstdin.c.tmp auto_qstdin.c

auto_qstdin.o: compile auto_qstdin.c
	./compile auto_qstdin.c

auto-strarr: load auto-strarr.o
	./load auto-strarr -lqmail

auto-strarr.o: compile auto-strarr.c
	./compile auto-strarr.c

cleanq: load cleanq.o get_uid.o auto_uids.o 
	./load cleanq get_uid.o auto_uids.o -lqmail

cleanq.o: compile cleanq.c direntry.h auto_ageout.h auto_uids.h
	./compile cleanq.c

auto_ageout.h: conf-ageout
	echo "#define MAX_AGE" "`head -1 < conf-ageout`" > auto_ageout.h.tmp && \
	mv auto_ageout.h.tmp auto_ageout.h

run-cleanq: load run-cleanq.o auto_qmail.o
	./load run-cleanq auto_qmail.o

run-cleanq.o: compile run-cleanq.c auto_qmail.h
	./compile run-cleanq.c

MakeArgs.o: compile MakeArgs.c
	./compile MakeArgs.c

qscanq-stdin: load qscanq-stdin.o auto_qmail.o timestamp.o \
auto_spool.o mkfn.o MakeArgs.o do_ripmime.o auto_ripmime_cmd.o \
do_scan.o auto_scancmd.o do_cleanq.o auto_cleaner.o qregex.o \
wildmat.o matchregex.o variables.o auto_control.o  fmt_xpid.o
	./load qscanq-stdin auto_qmail.o timestamp.o MakeArgs.o \
	matchregex.o auto_spool.o mkfn.o do_ripmime.o \
	auto_ripmime_cmd.o do_scan.o auto_scancmd.o do_cleanq.o \
	auto_cleaner.o qregex.o wildmat.o control.o variables.o \
	auto_control.o fmt_xpid.o -lqmail

qscanq-stdin.o: compile qscanq-stdin.c auto_ageout.h \
exitcodes.h mkfn.h auto_fnlen.h auto_qmail.h
	./compile qscanq-stdin.c

do_ripmime.o: compile do_ripmime.c exitcodes.h
	./compile do_ripmime.c

do_scan.o: compile do_scan.c exitcodes.h qregex.h control.h \
auto_qmail.h variables.h auto_control.h MakeArgs.h
	./compile do_scan.c

auto_ripmime_cmd.c: auto-strarr conf-ripmime-cmd
	./auto-strarr auto_ripmime_cmd `head -1 conf-ripmime-cmd` > \
	auto_ripmime_cmd.c.tmp && mv auto_ripmime_cmd.c.tmp auto_ripmime_cmd.c

auto_ripmime_cmd.o: compile auto_ripmime_cmd.c
	./compile auto_ripmime_cmd.c

auto_scancmd.c: auto-strarr conf-scancmd
	./auto-strarr auto_scancmd `head -1 conf-scancmd` > auto_scancmd.c.tmp \
	&& mv auto_scancmd.c.tmp auto_scancmd.c

auto_scancmd.o: compile auto_scancmd.c
	./compile auto_scancmd.c

qscanq.8: \
qscanq.9 conf-logdir conf-sysconfdir
	cat qscanq.9 \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}@LOGDIR\@}"`head -1 conf-logdir`"}g \
	> qscanq.8

run-cleanq.8: \
run-cleanq.9 conf-prefix
	cat run-cleanq.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> run-cleanq.8

analyzer: \
mlmatchup matchup columnt zoverall zsendmail xqp xsender xrecipient ddist \
deferrals failures successes rhosts recipients rxdelay senders suids \
zddist zdeferrals zfailures zsuccesses zrhosts zrecipients zrxdelay \
zsenders zsuids zsmtp rsmtp rsmtpfailures rsmtpsdomains rsmtprdomains \
rsmtpsenders rsmtprecipients multilog-matchup smtp-matchup rspamrdomain \
rspamsdomain zspam envmigrate rspamstat rspamhist

qtools: \
822body 822headerok 822bodyfilter 822headerfilter 822addr \
822fields checkaddr checkdomain filterto condtomaildir \
iftoccfrom ifaddr replier replier-config

qlog: spipe qfilelog multipipe teepipe qlogselect

load: \
make-load warn-auto.sh systype
	( cat warn-auto.sh; ./make-load "`cat systype`" ) > load
	chmod 755 load

maildir.o: \
compile maildir.c prioq.h direntry.h maildir.h
	./compile maildir.c

maildir2mbox: \
load maildir2mbox.o maildir.o prioq.o myctime.o gfrom.o
	./load maildir2mbox maildir.o prioq.o myctime.o \
	gfrom.o -lqmail

maildir2mbox.o: \
compile maildir2mbox.c prioq.h gfrom.h myctime.h maildir.h
	./compile maildir2mbox.c

mbox2maildir: load mbox2maildir.o
	./load mbox2maildir -lqmail

mbox2maildir.o: compile mbox2maildir.c
	./compile mbox2maildir.c

qmaildirmake: \
load qmaildirmake.o
	./load qmaildirmake -lqmail

qmaildirmake.o: \
compile qmaildirmake.c
	./compile qmaildirmake.c

maildirwatch: \
load maildirwatch.o hfield.o headerbody.o maildir.o prioq.o
	./load maildirwatch hfield.o headerbody.o maildir.o \
	prioq.o -lqmail

maildirwatch.o: \
compile maildirwatch.c prioq.h hfield.h headerbody.h maildir.h
	./compile maildirwatch.c

mailsubj: \
warn-auto.sh mailsubj.sh conf-prefix conf-break conf-split
	cat warn-auto.sh mailsubj.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> mailsubj

doc/INSTALL-MINI: doc/INSTALL-MINI.in
	$(edit) $@.in > $@

myctime.o: \
compile myctime.c myctime.h
	./compile myctime.c

newfield.o: \
compile newfield.c newfield.h
	./compile newfield.c

pinq: \
warn-auto.sh pinq.sh conf-prefix conf-break conf-split
	cat warn-auto.sh pinq.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> pinq

predate: load predate.o
	./load predate -lqmail

predate.o: compile predate.c
	./compile predate.c

preline: load preline.o
	./load preline -lqmail

preline.o: \
compile preline.c
	./compile preline.c

prioq.o: \
compile prioq.c prioq.h
	./compile prioq.c

proc: \
proc.sh conf-prefix
	cat proc.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> proc

proc+df: \
proc+df.sh conf-prefix
	cat proc+df.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> proc+df

prot.o: \
compile prot.c hasshsgr.h prot.h
	./compile prot.c

qail: \
warn-auto.sh qail.sh conf-prefix conf-break conf-split
	cat warn-auto.sh qail.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qail

qmail-clean: load qmail-clean.o fmtqfn.o auto_qmail.o auto_split.o \
variables.o
	./load qmail-clean fmtqfn.o auto_qmail.o \
	auto_split.o variables.o -lqmail

qmail-clean.o: compile qmail-clean.c direntry.h fmtqfn.h auto_qmail.h \
variables.h
	./compile qmail-clean.c

qmail-control.5: \
qmail-control.9 conf-sysconfdir
	cat qmail-control.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-control.5

qmail-srs.5: \
qmail-srs.9 conf-sysconfdir
	cat qmail-srs.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-srs.5

qmail-getpw: \
load qmail-getpw.o auto_break.o auto_usera.o
	./load qmail-getpw auto_break.o auto_usera.o -lqmail

qmail-getpw.8: \
qmail-getpw.9 conf-break
	cat qmail-getpw.9 \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	> qmail-getpw.8

qmail-getpw.o: compile qmail-getpw.c auto_usera.h auto_break.h \
qlx.h
	./compile qmail-getpw.c

qmail-inject: load qmail-inject.o headerbody.o hfield.o \
newfield.o quote.o control.o date822fmt.o qmail.o envrules.o wildmat.o \
matchregex.o pathexec_run.o openreadclose.o pathexec_env.o envdir_set.o \
srs.o rcpthosts.o auto_control.o token822.o auto_qmail.o variables.o \
auto_sysconfdir.o dns.lib srs.lib
	./load qmail-inject headerbody.o hfield.o newfield.o \
	quote.o control.o date822fmt.o qmail.o pathexec_run.o \
	openreadclose.o pathexec_env.o envdir_set.o matchregex.o envrules.o \
	wildmat.o srs.o rcpthosts.o auto_control.o variables.o auto_qmail.o \
	auto_sysconfdir.o token822.o `cat dns.lib srs.lib` -lqmail

qmail-inject.8: qmail-inject.9 conf-prefix conf-sysconfdir
	cat qmail-inject.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-inject.8

qmail-inject.o: \
compile qmail-inject.c hfield.h token822.h control.h \
qmail.h quote.h headerbody.h envdir.h \
auto_sysconfdir.h newfield.h envrules.h pathexec.h \
variables.h conf-srs auto_control.h hassrs.h
	./compile qmail-inject.c

envrules.o: \
compile envrules.c envrules.h matchregex.o control.h qregex.h
	./compile envrules.c

tablematch.o: \
compile tablematch.c tablematch.h control.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` tablematch.c

bodycheck.o: \
compile bodycheck.c bodycheck.h
	./compile bodycheck.c

qmail-limits.7: \
qmail-limits.9 conf-spawn
	cat qmail-limits.9 \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-limits.7

qmail-local: \
load qmail-local.o qmail.o quote.o gfrom.o myctime.o \
slurpclose.o srs.o auto_qmail.o auto_patrn.o rcpthosts.o \
auto_control.o variables.o control.o libsyncdir.a socket.lib srs.lib
	./load qmail-local qmail.o quote.o gfrom.o myctime.o \
	slurpclose.o srs.o auto_qmail.o rcpthosts.o \
	auto_patrn.o variables.o auto_control.o control.o \
	`cat socket.lib srs.lib` -L. -lsyncdir -lqmail

qmail-local.8:\
qmail-local.9 conf-sysconfdir
	cat qmail-local.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-local.8

qmail-local.o: \
compile qmail-local.c quote.h qmail.h slurpclose.h myctime.h \
gfrom.h auto_patrn.h control.h auto_qmail.h variables.h hassrs.h \
conf-sync conf-mailarch
	./compile `grep -h -v "^#" conf-sync conf-mailarch` qmail-local.c

qmail-lspawn.o: compile qmail-lspawn.c prot.h slurpclose.h auto_qmail.h \
auto_uids.h qlx.h auto_assign.h indimail_stub.h auto_control.h variables.h
	./compile qmail-lspawn.c

qmail-lspawn: \
load qmail-lspawn.o spawn.o prot.o slurpclose.o \
get_uid.o auto_sysconfdir.o auto_control.o auto_qmail.o \
auto_uids.o auto_spawn.o auto_assign.o control.o \
indimail_stub.o variables.o dns.lib
	./load qmail-lspawn spawn.o prot.o slurpclose.o \
	auto_uids.o get_uid.o auto_sysconfdir.o auto_control.o \
	auto_qmail.o auto_spawn.o indimail_stub.o control.o \
	variables.o auto_assign.o `cat dns.lib` -ldl -lqmail

qmail-newmrh: \
load qmail-newmrh.o cdbmss.o auto_control.o \
auto_qmail.o variables.o
	./load qmail-newmrh cdbmss.o auto_qmail.o \
	auto_control.o variables.o -lqmail

qmail-newmrh.8: \
qmail-newmrh.9 conf-sysconfdir conf-prefix
	cat qmail-newmrh.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	| sed s}@prefix\@}"`head -1 conf-prefix`"}g \
	> qmail-newmrh.8

qmail-newmrh.o: \
compile qmail-newmrh.c cdbmss.h auto_control.h
	./compile qmail-newmrh.c

qmail-cdb: \
load qmail-cdb.o cdbmss.o auto_qmail.o \
variables.o auto_control.o
	./load qmail-cdb cdbmss.o auto_control.o variables.o \
	auto_qmail.o -lqmail

qmail-cdb.8: \
qmail-cdb.9 conf-break conf-spawn conf-sysconfdir
	cat qmail-cdb.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-cdb.8

qmail-cdb.o: \
compile qmail-cdb.c auto_qmail.h cdbmss.h \
variables.h auto_control.h
	./compile qmail-cdb.c

recipient-cdb: \
load recipient-cdb.o cdbmss.o auto_assign.o
	./load recipient-cdb cdbmss.o auto_assign.o -lqmail

recipient-cdb.8: \
recipient-cdb.9 conf-sysconfdir
	cat recipient-cdb.9 \
	| sed s}ASSIGN}"`head -1 conf-sysconfdir`/users"}g \
	> recipient-cdb.8

recipient-cdb.o: \
compile recipient-cdb.c auto_assign.h cdbmss.h
	./compile recipient-cdb.c

qmail-newu: \
load qmail-newu.o cdbmss.o auto_assign.o
	./load qmail-newu cdbmss.o auto_assign.o -lqmail

qmail-newu.8: \
qmail-newu.9 conf-sysconfdir
	cat qmail-newu.9 \
	| sed s}ASSIGN}"`head -1 conf-sysconfdir`/users"}g \
	> qmail-newu.8

qmail-newu.o: \
compile qmail-newu.c cdbmss.h auto_assign.h
	./compile qmail-newu.c

cdb-database: \
load cdb-database.o cdbmss.o auto_control.o variables.o
	./load cdb-database cdbmss.o auto_control.o variables.o -lqmail

cdb-database.o: \
compile cdb-database.c cdbmss.h auto_control.h
	./compile cdb-database.c

cdb-database.8: \
cdb-database.9 conf-prefix conf-sysconfdir
	cat cdb-database.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}CONTROL}"`head -1 conf-sysconfdir`"}g \
	> cdb-database.8

qmail-popbull: \
load qmail-popbull.o
	./load qmail-popbull -lqmail

qmail-popbull.o: \
compile qmail-popbull.c direntry.h
	./compile qmail-popbull.c

qmail-pw2u: load qmail-pw2u.o control.o auto_usera.o auto_control.o \
auto_break.o auto_assign.o variables.o
	./load qmail-pw2u control.o auto_control.o auto_usera.o \
	auto_break.o auto_assign.o variables.o -lqmail

qmail-pw2u.o: compile qmail-pw2u.c control.h auto_break.h auto_assign.h \
auto_usera.h
	./compile qmail-pw2u.c

qmail-pw2u.8: \
qmail-pw2u.9 conf-break conf-sysconfdir
	cat qmail-pw2u.9 \
	| sed s}ASSIGN}"`head -1 conf-sysconfdir`/users"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	> qmail-pw2u.8

qmail-qmqpc: load qmail-qmqpc.o slurpclose.o \
timeoutconn.o ip.o control.o auto_qmail.o \
socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
socket_v4mappedprefix.o socket_tcp.o socket_tcp6.o \
auto_control.o socket.lib
	./load qmail-qmqpc slurpclose.o socket_tcp.o socket_tcp6.o \
	socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
	auto_control.o socket_v4mappedprefix.o timeoutconn.o ip.o \
	control.o auto_qmail.o variables.o \
	`cat socket.lib` -lqmail

qmail-qmqpc.o: \
compile qmail-qmqpc.c auto_control.h slurpclose.h ip.h \
timeoutconn.h auto_qmail.h control.h variables.h \
haveip6.h socket.h conf-ip
	./compile `grep -h -v "^#" conf-ip` qmail-qmqpc.c

qmail-qmqpd: \
load qmail-qmqpd.o received.o date822fmt.o qmail.o auto_qmail.o
	./load qmail-qmqpd received.o date822fmt.o qmail.o \
	auto_qmail.o -lqmail

qmail-qmqpd.o: \
compile qmail-qmqpd.c auto_qmail.h qmail.h received.h
	./compile qmail-qmqpd.c

qmail-qmtpd: \
load qmail-qmtpd.o rcpthosts.o control.o received.o \
date822fmt.o qmail.o auto_qmail.o variables.o auto_control.o
	./load qmail-qmtpd rcpthosts.o control.o auto_control.o \
	auto_qmail.o received.o date822fmt.o qmail.o variables.o \
	-lqmail

qmail-qmtpd.o: \
compile qmail-qmtpd.c qmail.h rcpthosts.h auto_qmail.h \
control.h received.h recipients.h
	./compile qmail-qmtpd.c

qmail-qread: \
load qmail-qread.o fmtqfn.o readsubdir.o date822fmt.o \
auto_qmail.o auto_split.o variables.o control.o envdir_set.o \
pathexec_env.o auto_sysconfdir.o auto_control.o \
pathexec_run.o openreadclose.o
	./load qmail-qread fmtqfn.o readsubdir.o date822fmt.o \
	control.o auto_qmail.o auto_split.o variables.o \
	envdir_set.o pathexec_env.o pathexec_run.o \
	auto_sysconfdir.o auto_control.o openreadclose.o -lqmail

qmail-qread.8: qmail-qread.9 conf-sysconfdir
	cat qmail-qread.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-qread.8

qmail-qread.o: \
compile qmail-qread.c fmtqfn.h readsubdir.h direntry.h \
control.h auto_qmail.h date822fmt.h envdir.h pathexec.h \
variables.h conf-queue auto_control.h auto_sysconfdir.h
	if test -f conf-queue ; then \
		./compile -DMULTI_QUEUE -DQUEUE_COUNT=`cat conf-queue` qmail-qread.c; \
	else \
		./compile -DQUEUE_COUNT=10 qmail-qread.c; \
	fi

qmail-multi: load qmail-multi.o MakeArgs.o auto_control.o \
auto_qmail.o variables.o control.o dns.lib socket.lib
	./load qmail-multi MakeArgs.o auto_control.o \
	auto_qmail.o variables.o control.o \
	`cat dns.lib socket.lib` -lqmail

qmail-multi.o: \
compile qmail-multi.c auto_qmail.h control.h conf-queue MakeArgs.h
	if test -f conf-queue ; then \
		./compile -DQUEUE_COUNT=`cat conf-queue` qmail-multi.c; \
	else \
		./compile -DQUEUE_COUNT=10 qmail-multi.c; \
	fi

spawn-filter: \
load spawn-filter.o variables.o evaluate.o \
auto_qmail.o control.o wildmat.o qregex.o MakeArgs.o \
auto_control.o matchregex.o envrules.o socket.lib dns.lib
	./load spawn-filter variables.o evaluate.o \
	auto_qmail.o control.o wildmat.o qregex.o \
	MakeArgs.o auto_control.o matchregex.o \
	envrules.o `cat dns.lib socket.lib` -lm -lqmail

spawn-filter.o: \
compile spawn-filter.c control.h auto_qmail.h variables.h qregex.h \
evaluate.h MakeArgs.h
	./compile spawn-filter.c

evaluate.o: \
compile evaluate.h
	./compile evaluate.c

spawn-filter.8: \
spawn-filter.9 conf-prefix conf-sysconfdir
	cat spawn-filter.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}CONTROL}"`head -1 conf-sysconfdir`"}g \
	> spawn-filter.8

drate.1: \
drate.9 conf-prefix
	cat drate.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> drate.1

qmail-queue: \
load qmail-queue.o triggerpull.o fmtqfn.o date822fmt.o get_uid.o MakeArgs.o \
auto_qmail.o auto_split.o auto_uids.o variables.o matchregex.o \
control.o auto_control.o strsalloc.o socket.lib libsyncdir.a
	./load qmail-queue triggerpull.o fmtqfn.o date822fmt.o get_uid.o MakeArgs.o \
	auto_qmail.o auto_split.o auto_uids.o variables.o matchregex.o \
	control.o auto_control.o strsalloc.o \
	`cat socket.lib` -L. -lsyncdir -lqmail

qmail-queue.8: \
qmail-queue.9 conf-qmail conf-prefix
	cat qmail-queue.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> qmail-queue.8

qmail-queue.o: \
compile qmail-queue.c matchregex.h triggerpull.h MakeArgs.h \
control.h auto_qmail.h auto_uids.h date822fmt.h fmtqfn.h \
variables.h conf-sync conf-qhpsi conf-ip conf-mailarch
	./compile `grep -h -v "^#" conf-sync conf-qhpsi conf-ip conf-mailarch` \
	qmail-queue.c

domainkeys.o: compile domainkeys.c conf-domainkeys
	./compile `grep -h -v "^#" conf-domainkeys` domainkeys.c

dktrace.o: compile dktrace.c dktrace.h conf-domainkeys
	./compile `grep -h -v "^#" conf-domainkeys` dktrace.c

dktest: load dktest.o domainkeys.o dktrace.o dns.o ip.o strsalloc.o ipalloc.o \
socket_v4mappedprefix.o socket_v6any.o tlsarralloc.o dns_text.o dns.lib
	./load dktest domainkeys.o dktrace.o dns.o ip.o strsalloc.o ipalloc.o \
	socket_v4mappedprefix.o socket_v6any.o tlsarralloc.o \
	dns_text.o -lcrypto `cat dns.lib` -lqmail

dktest.o: compile dktest.c domainkeys.h conf-domainkeys
	./compile `grep -h -v "^#" conf-domainkeys` dktest.c

testtrace: load testtrace.o dns.o case_diffs.o case_diffb.o domainkeys.o dktrace.o \
scan_xlong.o ip.o iperror.o strsfmt_str.o socket_v4mappedprefix.o socket_v6any.o \
dns.lib
	./load testtrace dns.o domainkeys.o dktrace.o ip.o ipalloc.o strsalloc.o \
	socket_v4mappedprefix.o socket_v6any.o `cat dns.lib` -lcrypto -lqmail

dns_text.o: compile dns_text.c dns.h
	./compile -DDOMAIN_KEYS dns_text.c

testtrace.o: compile testtrace.c dktrace.h
	./compile testtrace.c

dknewkey: dknewkey.sh warn-auto.sh
	rm -f dknewkey
	cat warn-auto.sh dknewkey.sh > dknewkey

qmail-dk: \
load qmail-dk.o triggerpull.o fmtqfn.o date822fmt.o \
tlsarralloc.o auto_control.o auto_qmail.o auto_split.o variables.o \
dns.o strsalloc.o ip.o ipalloc.o MakeArgs.o socket_v4mappedprefix.o \
socket_v6any.o domainkeys.o dktrace.o control.o dns_text.o dns.lib
	./load qmail-dk triggerpull.o fmtqfn.o auto_control.o auto_qmail.o \
	auto_split.o variables.o socket_v4mappedprefix.o \
	socket_v6any.o tlsarralloc.o dns.o strsalloc.o ip.o ipalloc.o \
	domainkeys.o dktrace.o control.o dns_text.o MakeArgs.o \
	`cat dns.lib` -lcrypto -lqmail

qmail-dk.o: \
compile qmail-dk.c qmail.h auto_qmail.h control.h domainkeys.h \
variables.h conf-domainkeys auto_control.h MakeArgs.h
	./compile `grep -h -v "^#" conf-domainkeys` qmail-dk.c

qmail-dk.8: qmail-dk.9 conf-sysconfdir
	cat qmail-dk.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-dk.8

dktest.8: dktest.9 conf-sysconfdir
	cat dktest.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> dktest.8

dkim.lib: \
compile trydkim.c dns_text.o conf-dkim dns.lib
	( ( ./compile -I/usr/include/dkim \
		`grep -h -v "^#" conf-dkim` trydkim.c && \
	g++ -o trydkim trydkim.o dns_text.o \
	`cat dns.lib` -ldkim -lcrypto -ldkim -lqmail ) >/dev/null 2>&1 \
	&& echo "-ldkim" || exit 0 ) > dkim.lib
	rm -f trydkim.o trydkim

hasdkim.h: dns_text.o \
trydkim.c compile load dns.lib dkim.lib conf-indi
	( ( ./compile -I/usr/include/dkim \
		`grep -h -v "^#" conf-dkim` trydkim.c && \
	g++ -o trydkim trydkim.o dns_text.o \
	`cat dkim.lib dns.lib` -lcrypto -lqmail ) >/dev/null 2>&1 \
	&& (echo \#define HASDKIM 1) || exit 0 ) > hasdkim.h
	rm -f trydkim.o trydkim

qmail-dkim: qmail-dkim.o date822fmt.o auto_control.o \
auto_qmail.o variables.o control.o MakeArgs.o dns_text.o \
dns.lib dkim.lib
	g++ -s -o qmail-dkim qmail-dkim.o date822fmt.o auto_control.o \
	auto_qmail.o variables.o control.o MakeArgs.o \
	dns_text.o `cat dns.lib dkim.lib` -lcrypto -lqmail

qmail-dkim.o: \
compile qmail-dkim.c qmail.h auto_qmail.h control.h domainkeys.h \
hasdkim.h variables.h auto_control.h MakeArgs.h
	./compile -I/usr/include/dkim qmail-dkim.c

qmail-dkim.8: qmail-dkim.9 conf-sysconfdir
	cat qmail-dkim.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-dkim.8

dk-filter.8: dk-filter.9 conf-prefix conf-sysconfdir
	cat dk-filter.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> dk-filter.8

hastlsa.h: trytlsa.c compile load dns.lib \
dns.o tlsarralloc.o ip.o socket_v4mappedprefix.o \
socket_v6any.o ipalloc.o strsalloc.o
	( ( ./compile `grep -h -v "^#" conf-tlsa` \
	trytlsa.c && ./load trytlsa dns.o tlsarralloc.o \
	ip.o socket_v4mappedprefix.o socket_v6any.o \
	ipalloc.o strsalloc.o `cat dns.lib` -lqmail) >/dev/null 2>&1 \
	&& (echo \#define HASTLSA 1) || exit 0 ) > hastlsa.h
	rm -f trytlsa.o trytlsa

qnotify.1: qnotify.9 conf-qmail conf-sysconfdir
	cat qnotify.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qnotify.1

rrt.1: rrt.9 conf-sysconfdir
	cat rrt.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> rrt.1

qmail-remote: \
load qmail-remote.o control.o timeoutconn.o tcpto.o dns.o ip.o \
ipalloc.o strsalloc.o ipme.o quote.o variables.o \
auto_control.o socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
md5c.o hmac_md5.o hmac_sha1.o hmac_sha256.o hmac_sha512.o hmac_ripemd.o sha1.o ripemd.o \
tlsarralloc.o digest_md5.o socket_v4mappedprefix.o socket_tcp.o socket_tcp6.o \
fn_handler.o query_skt.o tlsacheck.o auto_qmail.o dns.lib socket.lib tls.lib 
	./load qmail-remote control.o tlsacheck.o auto_control.o timeoutconn.o \
	tcpto.o dns.o ip.o socket_ip4loopback.o socket_v6loopback.o \
	socket_v6any.o ipalloc.o socket_v4mappedprefix.o strsalloc.o \
	ipme.o quote.o md5c.o hmac_md5.o hmac_sha1.o hmac_ripemd.o \
	sha1.o ripemd.o digest_md5.o fn_handler.o query_skt.o \
	tlsarralloc.o variables.o socket_tcp.o socket_tcp6.o \
	hmac_sha256.o hmac_sha512.o auto_qmail.o \
	`cat dns.lib socket.lib tls.lib` -lqmail

qmail-remote.8: \
qmail-remote.9 conf-qmail conf-sysconfdir conf-libexec
	cat qmail-remote.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	| sed s}@sysconfdir\@}"`head -1 conf-sysconfdir`"}g \
	> qmail-remote.8

qmail-remote.o: \
compile qmail-remote.c tls.h ssl_timeoutio.h auth_cram.h \
auto_qmail.h control.h dns.h quote.h ip.h ipalloc.h \
query_skt.h ipme.h tcpto.h timeoutconn.h haveip6.h socket.h \
variables.h hastlsv1_1_client.h hastlsv1_2_client.h \
auth_digest_md5.h auto_control.h hastlsa.h tlsacheck.h fn_handler.h \
tlsarralloc.h conf-tls conf-ip conf-batv conf-mxps
	./compile `grep -h -v "^#" conf-tls conf-ip conf-mxps conf-batv` qmail-remote.c

qmail-rspawn: \
load qmail-rspawn.o spawn.o tcpto_clean.o get_uid.o \
auto_qmail.o auto_uids.o auto_spawn.o rcpthosts.o \
control.o variables.o auto_sysconfdir.o auto_control.o \
dns.lib indimail_stub.o
	./load qmail-rspawn spawn.o tcpto_clean.o get_uid.o \
	auto_spawn.o rcpthosts.o control.o variables.o auto_sysconfdir.o \
	auto_qmail.o auto_uids.o indimail_stub.o auto_control.o \
	`cat dns.lib` -ldl -lqmail

qmail-rspawn.o: compile qmail-rspawn.c \
tcpto.h rcpthosts.h auto_qmail.h indimail_stub.h \
ipalloc.h conf-tls conf-ip auto_control.h variables.h
	./compile `grep -h -v "^#" conf-tls conf-ip` qmail-rspawn.c

qmail-send: \
load qmail-send.o qsutil.o control.o newfield.o prioq.o \
trigger.o fmtqfn.o quote.o readsubdir.o qmail.o date822fmt.o matchregex.o\
auto_qmail.o srs.o rcpthosts.o auto_control.o auto_split.o variables.o \
envrules.o qregex.o wildmat.o libsyncdir.a srs.lib
	./load qmail-send qsutil.o control.o newfield.o \
	prioq.o trigger.o fmtqfn.o quote.o readsubdir.o \
	wildmat.o qmail.o date822fmt.o auto_qmail.o auto_split.o \
	variables.o envrules.o matchregex.o srs.o rcpthosts.o \
	auto_control.o -L. -lsyncdir `cat srs.lib` -ldl -lqmail

qmail-send.8: \
qmail-send.9 conf-break conf-spawn conf-qmail conf-sysconfdir conf-prefix
	cat qmail-send.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-send.8

qmail-send.o: \
compile qmail-send.c direntry.h control.h \
auto_qmail.h trigger.h newfield.h quote.h \
qmail.h qsutil.h prioq.h envrules.h \
fmtqfn.h readsubdir.h direntry.h variables.h qmail-todo.h \
auto_control.h conf-mime conf-sync conf-batv \
hassrs.h conf-srs conf-locklogs
	./compile `grep -h -v "^#" conf-mime \
		conf-sync conf-batv conf-locklogs` qmail-send.c

qmail-daemon: \
load qmail-daemon.o control.o auto_qmail.o auto_control.o variables.o
	./load qmail-daemon control.o auto_qmail.o \
	variables.o auto_control.o -lqmail

qmail-daemon.o: compile qmail-daemon.c control.h auto_qmail.h conf-queue
	if test -f conf-queue ; then \
		./compile -DQUEUE_COUNT=`cat conf-queue` qmail-daemon.c; \
	else \
		./compile -DQUEUE_COUNT=10 qmail-daemon.c; \
	fi

get_uid.o: \
compile get_uid.c auto_uids.h
	./compile get_uid.c

qmail-showctl: \
load qmail-showctl.o auto_uids.o control.o dns.lib \
auto_qmail.o auto_break.o indimail_stub.o \
auto_patrn.o auto_spawn.o auto_split.o variables.o \
get_uid.o auto_control.o auto_sysconfdir.o
	./load qmail-showctl auto_uids.o control.o auto_qmail.o \
	auto_break.o auto_patrn.o auto_spawn.o auto_control.o \
	auto_split.o variables.o get_uid.o indimail_stub.o \
	auto_sysconfdir.o `cat dns.lib` -ldl -lqmail

qmail-showctl.o: compile qmail-showctl.c control.h direntry.h \
auto_uids.h auto_qmail.h auto_break.h auto_patrn.h auto_spawn.h \
auto_split.h spf.h variables.h auto_control.h indimail_stub.h \
hassrs.h conf-srs conf-spf conf-batv
	./compile `grep -h -v "^#" conf-spf conf-batv` \
	qmail-showctl.c

plugtest: \
load plugtest.o auto_qmail.o auto_prefix.o
	./load plugtest auto_qmail.o auto_prefix.o \
	-ldl -lqmail

plugtest.o: \
compile plugtest.c auto_qmail.h auto_prefix.h smtp_plugin.h
	./compile plugtest.c

qmail_smtpd.so: \
smtpd.o rcpthosts.o recipients.o indimail_stub.o \
ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o \
received.o date822fmt.o wildmat.o qmail.o variables.o \
envrules.o bodycheck.o sqlmatch.o matchregex.o qcount_dir.o \
socket_ip4loopback.o socket_v6loopback.o socket_v4mappedprefix.o \
socket_v6any.o mail_acl.o spf.o dns.o auto_qmail.o auto_uids.o \
qregex_sql.o tablematch.o greylist.o query_skt.o fn_handler.o load_mysql.o \
auto_break.o auto_control.o auto_assign.o auto_prefix.o auto_sysconfdir.o \
socket.lib dns.lib tls.lib hmac_sha1.o mysql_config conf-sql
	if test $(SYSTEM) = DARWIN ; then \
		$(LD) -bundle -undefined dynamic_lookup -o qmail_smtpd.so \
		smtpd.o rcpthosts.o recipients.o indimail_stub.o load_mysql.o \
		qcount_dir.o ip.o ipme.o ipalloc.o strsalloc.o control.o \
		etrn.o atrn.o received.o date822fmt.o wildmat.o qmail.o \
		variables.o envrules.o bodycheck.o spf.o dns.o auto_qmail.o \
		sqlmatch.o matchregex.o socket_v4mappedprefix.o \
		socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
		mail_acl.o auto_uids.o qregex_sql.o tablematch.o greylist.o \
		auto_break.o auto_control.o auto_assign.o query_skt.o fn_handler.o \
		tlsarralloc.o auto_prefix.o auto_sysconfdir.o \
		hmac_sha1.o `cat socket.lib dns.lib tls.lib` -lqmail ; \
	elif test $(SYSTEM) = FREEBSD ; then \
		$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 -o qmail_smtpd.so \
		smtpd.o rcpthosts.o recipients.o auto_control.o auto_assign.o \
		qcount_dir.o ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o \
		received.o date822fmt.o wildmat.o qmail.o variables.o envrules.o \
		bodycheck.o spf.o dns.o auto_qmail.o sqlmatch.o matchregex.o \
		socket_v4mappedprefix.o socket_ip4loopback.o socket_v6loopback.o \
		socket_v6any.o mail_acl.o auto_uids.o qregex_sql.o tablematch.o \
		greylist.o auto_break.o indimail_stub.o load_mysql.o query_skt.o \
		fn_handler.o tlsarralloc.o auto_prefix.o auto_sysconfdir.o hmac_sha1.o \
		`cat socket.lib dns.lib tls.lib` -L/usr/local/lib -lqmail; \
	else \
		$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 -o qmail_smtpd.so \
		smtpd.o rcpthosts.o recipients.o auto_control.o auto_assign.o \
		qcount_dir.o ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o \
		received.o date822fmt.o wildmat.o qmail.o variables.o envrules.o \
		bodycheck.o spf.o dns.o auto_qmail.o sqlmatch.o matchregex.o \
		socket_v4mappedprefix.o socket_ip4loopback.o socket_v6loopback.o \
		socket_v6any.o mail_acl.o auto_uids.o qregex_sql.o tablematch.o \
		greylist.o auto_break.o indimail_stub.o load_mysql.o query_skt.o \
		fn_handler.o tlsarralloc.o auto_prefix.o auto_sysconfdir.o hmac_sha1.o \
		`cat socket.lib dns.lib tls.lib` -lqmail ; \
	fi

qmail-smtpd: \
load qmail-smtpd.o smtpd.o rcpthosts.o recipients.o \
ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o \
received.o date822fmt.o wildmat.o qmail.o variables.o auto_control.o \
envrules.o bodycheck.o sqlmatch.o matchregex.o indimail_stub.o \
socket_ip4loopback.o socket_v6loopback.o socket_v4mappedprefix.o \
socket_v6any.o mail_acl.o spf.o dns.o auto_qmail.o auto_uids.o \
qregex_sql.o tablematch.o greylist.o auto_assign.o qcount_dir.o \
query_skt.o fn_handler.o tlsarralloc.o load_mysql.o \
auto_sysconfdir.o auto_break.o auto_prefix.o socket.lib dns.lib tls.lib \
hmac_sha1.o mysql_config conf-sql
	./load qmail-smtpd smtpd.o rcpthosts.o recipients.o \
	ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o \
	qcount_dir.o received.o date822fmt.o wildmat.o qmail.o variables.o \
	envrules.o bodycheck.o spf.o dns.o auto_qmail.o sqlmatch.o \
	matchregex.o socket_v4mappedprefix.o socket_ip4loopback.o \
	socket_v6loopback.o socket_v6any.o mail_acl.o auto_uids.o \
	indimail_stub.o qregex_sql.o tablematch.o greylist.o auto_break.o \
	auto_control.o auto_assign.o hmac_sha1.o query_skt.o fn_handler.o \
	tlsarralloc.o load_mysql.o auto_sysconfdir.o auto_prefix.o \
	`cat socket.lib dns.lib tls.lib` -ldl -lqmail

mail_acl.o: compile mail_acl.c matchregex.h
	./compile mail_acl.c

indimail_stub.o: \
compile indimail_stub.c auto_qmail.h auto_sysconfdir.h auto_uids.h \
auto_control.h variables.h
	./compile indimail_stub.c

qregex.o: \
compile qregex.c qregex.h matchregex.h \
control.h variables.h auto_control.h
	./compile qregex.c

mysql_inc: mysql_config
	./mysql_config --include > mysql_inc || true

# qregex.c compiled with sqlmatch()
qregex_sql.o: \
compile qregex.c qregex.h sqlmatch.h matchregex.h \
control.h variables.h hasmysql.h mysql_config \
mysql_inc conf-sql
	./compile `grep -h -v "^#" conf-sql mysql_inc` \
	qregex.c -o qregex_sql.o

load_mysql.o: compile load_mysql.c indimail_stub.h \
mysql_config conf-sql auto_control.h variables.h \
hasmysql.h mysql_inc
	./compile `grep -h -v "^#" conf-sql mysql_inc` load_mysql.c

matchregex.o: \
compile matchregex.c qregex.h
	./compile matchregex.c

qmail-nullqueue: load qmail-nullqueue.o
	./load qmail-nullqueue -lqmail

qmail-nullqueue.o: \
compile qmail-nullqueue.c
	./compile qmail-nullqueue.c

qmail-todo.8: \
qmail-todo.9 conf-qmail conf-sysconfdir
	cat qmail-todo.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-todo.8

qmail-smtpd.8: \
qmail-smtpd.9 conf-qmail conf-sysconfdir conf-prefix
	cat qmail-smtpd.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}ASSIGN}"`head -1 conf-sysconfdir`/users"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-smtpd.8

plugin_init.3: \
plugin_init.9 conf-prefix
	cat plugin_init.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> plugin_init.3

plugtest.1: \
plugtest.9 conf-prefix
	cat plugtest.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> plugtest.1

qmail-poppass.8: \
qmail-poppass.9 conf-prefix
	cat qmail-poppass.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> qmail-poppass.8

qmail-multi.8: \
qmail-multi.9 conf-prefix conf-qmail
	cat qmail-multi.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> qmail-multi.8

smtpd.o: \
compile smtpd.c tls.h ssl_timeoutio.h auto_qmail.h control.h received.h \
ipme.h ip.h ipalloc.h qmail.h recipients.h greylist.h \
matchregex.o variables.h rcpthosts.h etrn.h date822fmt.h \
envrules.h tablematch.h matchregex.h qregex.h sqlmatch.h \
smtp_plugin.h auto_prefix.h hastlsv1_1_server.h \
hastlsv1_2_server.h hasmysql.h indimail_stub.h \
auto_control.h dns.h conf-tls conf-spf conf-ip \
conf-plugin conf-batv mysql_config mysql_inc
	./compile `grep -h -v "^#" conf-tls conf-spf \
	conf-ip conf-batv conf-plugin mysql_inc` smtpd.c

sqlmatch.o: compile sqlmatch.c variables.h sqlmatch.h \
qregex.h auto_control.h load_mysql.h mysql_config \
conf-sql hasmysql.h mysql_inc
	./compile `grep -h -v "^#" conf-sql mysql_inc` sqlmatch.c

greylist.o: \
compile greylist.c ip.h greylist.h query_skt.h fn_handler.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` greylist.c

query_skt.o: compile query_skt.c fn_handler.h
	./compile query_skt.c

fn_handler.o: \
compile fn_handler.c
	./compile fn_handler.c

qmail-start: \
load qmail-start.o prot.o auto_uids.o get_uid.o
	./load qmail-start prot.o auto_uids.o get_uid.o -lqmail

qmail-start.8: \
qmail-start.9 conf-qmail
	cat qmail-start.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-start.8

qmail-start.o: \
compile qmail-start.c prot.h auto_uids.h qmail-todo.h
	./compile qmail-start.c

qmail-tcpok: \
load qmail-tcpok.o auto_qmail.o variables.o \
control.o auto_control.o 
	./load qmail-tcpok auto_qmail.o variables.o control.o \
	auto_control.o -lqmail

qmail-tcpok.o: \
compile qmail-tcpok.c auto_qmail.h variables.h control.h conf-queue
	if test -f conf-queue ; then \
		./compile -DMULTI_QUEUE -DQUEUE_COUNT=`cat conf-queue` qmail-tcpok.c; \
	else \
		./compile -DQUEUE_COUNT=10 qmail-tcpok.c; \
	fi

qmail-tcpto: \
load qmail-tcpto.o ip.o auto_qmail.o \
variables.o control.o auto_control.o \
socket_v4mappedprefix.o socket_v6any.o 
	./load qmail-tcpto ip.o auto_qmail.o variables.o \
	control.o auto_control.o socket_v4mappedprefix.o \
	socket_v6any.o -lqmail

qmail-tcpto.o: \
compile qmail-tcpto.c auto_qmail.h ip.h variables.h \
control.h tcpto.h ipalloc.h conf-tls conf-ip conf-queue
	if test -f conf-queue ; then \
		./compile -DMULTI_QUEUE -DQUEUE_COUNT=`cat conf-queue` qmail-tcpto.c; \
	else \
		./compile -DQUEUE_COUNT=10 qmail-tcpto.c; \
	fi

qmail-todo: \
load qmail-todo.o control.o trigger.o fmtqfn.o \
readsubdir.o auto_qmail.o auto_split.o \
auto_control.o variables.o libsyncdir.a
	./load qmail-todo control.o trigger.o fmtqfn.o \
	auto_control.o readsubdir.o auto_qmail.o auto_split.o \
	variables.o -L. -lsyncdir -lqmail

qmail-todo.o: \
compile qmail-todo.c auto_qmail.h control.h \
direntry.h fmtqfn.h readsubdir.h trigger.h variables.h \
qmail-todo.h auto_control.h conf-sync
	./compile `grep -h -v "^#" conf-sync` qmail-todo.c

qmail-upq: \
warn-auto.sh qmail-upq.sh conf-qmail conf-split
	cat warn-auto.sh qmail-upq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qmail-upq

qmail-users.5: \
qmail-users.9 conf-qmail conf-sysconfdir
	cat qmail-users.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}@assign@}"`head -1 conf-sysconfdir`/users"}g \
	> qmail-users.5

qmail.o: compile qmail.c qmail.h auto_qmail.h
	./compile qmail.c

qreceipt: \
load qreceipt.o headerbody.o hfield.o quote.o token822.o qmail.o \
envdir_set.o pathexec_env.o pathexec_run.o openreadclose.o \
auto_sysconfdir.o variables.o auto_qmail.o auto_control.o
	./load qreceipt headerbody.o hfield.o quote.o token822.o \
	envdir_set.o pathexec_env.o pathexec_run.o openreadclose.o \
	auto_sysconfdir.o variables.o auto_control.o \
	auto_qmail.o qmail.o -lqmail

qreceipt.1: qreceipt.9 conf-sysconfdir
	cat qreceipt.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qreceipt.1

qreceipt.o: \
compile qreceipt.c hfield.h token822.h variables.h \
auto_sysconfdir.h headerbody.h \
quote.h qmail.h auto_qmail.h envdir.h pathexec.h \
auto_control.h auto_sysconfdir.h
	./compile qreceipt.c

qsmhook: \
load qsmhook.o
	./load qsmhook -lqmail

qsmhook.o: \
compile qsmhook.c
	./compile qsmhook.c

qsutil.o: \
compile qsutil.c qsutil.h conf-locklogs
	./compile `grep -h -v "^#" conf-locklogs` qsutil.c

quote.o: \
compile quote.c quote.h
	./compile quote.c

rcpthosts.o: \
compile rcpthosts.c control.h rcpthosts.h variables.h \
auto_control.h
	./compile rcpthosts.c

recipients.o: \
compile recipients.c control.h recipients.h \
auto_break.h auto_assign.h
	./compile recipients.c

readsubdir.o: \
compile readsubdir.c readsubdir.h direntry.h \
auto_split.h
	./compile readsubdir.c

received.o: \
compile received.c qmail.h date822fmt.h received.h
	./compile received.c

remoteinfo.o: \
compile remoteinfo.c ip.h timeoutconn.h \
haveip6.h socket.h remoteinfo.h conf-ip
	./compile `grep -h -v "^#" conf-ip` remoteinfo.c

rrforward: \
load rrforward.o qmail.o openreadclose.o auto_qmail.o \
envdir_set.o pathexec_env.o pathexec_run.o \
auto_sysconfdir.o auto_control.o variables.o
	./load rrforward qmail.o openreadclose.o auto_qmail.o \
	envdir_set.o pathexec_env.o pathexec_run.o \
	auto_sysconfdir.o auto_control.o variables.o -lqmail

rrforward.1: rrforward.9 conf-sysconfdir
	cat rrforward.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> rrforward.1

rrforward.o: \
compile rrforward.c qmail.h auto_sysconfdir.h \
envdir.h pathexec.h variables.h auto_control.h
	./compile rrforward.c

sendmail: \
load sendmail.o auto_qmail.o
	./load sendmail auto_qmail.o -lqmail

sendmail.o: \
compile sendmail.c auto_qmail.h
	./compile sendmail.c

install-strip: install

distclean: clean

installer: \
load installer.o
	./load installer -lqmail

installer.o: \
compile installer.c
	./compile installer.c

DIRS: DIRS.in
	$(edit) $@.in > $@

BIN: BIN.in
	([ -f BIN.$(SYSTEM) ] && \
	cat BIN.in BIN.$(SYSTEM) || cat BIN.in) > BIN

SBIN: SBIN.in SBIN.sql
	([ -f SBIN.$(SYSTEM) ] && \
	cat SBIN.in SBIN.$(SYSTEM) || cat SBIN.in; \
	[ -n "$(USE_SQL)" ] && cat SBIN.sql) > SBIN

SHARED: SHARED.in
	([ -f SHARED.$(SYSTEM) ] && \
	cat SHARED.in SHARED.$(SYSTEM) || cat SHARED.in) > SHARED

MAN: MAN.in MAN.sql
	([ -f MAN.$(SYSTEM) ] && \
	cat MAN.in MAN.$(SYSTEM) || cat MAN.in; \
	[ -n "$(USE_SQL)" ] && cat MAN.sql) > MAN


install: \
$(it) $(man) conf-qmail conf-prefix conf-libexec \
conf-shared conf-sysconfdir conf-logdir perm_list.mta \
installer DIRS BIN SBIN SHARED MAN
	mkdir -p $(DESTDIR)$(prefix)
	mkdir -p $(DESTDIR)$(prefix)/lib
	mkdir -p $(DESTDIR)$(qmaildir)
	mkdir -p $(DESTDIR)$(shareddir)
	mkdir -p $(DESTDIR)$(qsysconfdir)
	mkdir -p $(DESTDIR)$(libexecdir)
	mkdir -p $(DESTDIR)$(mandir)
	mkdir -p $(DESTDIR)$(logdir)
	./installer $(DESTDIR)                         < DIRS
	./installer $(DESTDIR)$(prefix)                < BIN
	./installer $(DESTDIR)$(prefix)                < SBIN
	./installer $(DESTDIR)$(shareddir)             < SHARED
	./installer $(DESTDIR)$(qsysconfdir)           < ETC
	./installer $(DESTDIR)$(prefix)"/lib/indimail" < PLUGINS
	./installer $(DESTDIR)$(libexecdir)            < LIBEXEC
	./installer $(DESTDIR)$(mandir)                < MAN

slurpclose.o: compile slurpclose.c slurpclose.h
	./compile slurpclose.c

socket.lib: \
trylsock.c compile load
	( ( ./compile trylsock.c && \
	./load trylsock -lsocket -lnsl ) >/dev/null 2>&1 \
	&& echo -lsocket -lnsl || exit 0 ) > socket.lib
	rm -f trylsock.o trylsock

tls.lib: \
trytls.c compile conf-tls load tls.o ssl_timeoutio.o
	( ( ./compile `grep -v "^#" conf-tls` trytls.c && \
	./load trytls -lssl -lcrypto ) >/dev/null 2>&1 \
	&& echo tls.o ssl_timeoutio.o -lssl -lcrypto || exit 0 ) > tls.lib
	rm -f trytls.o trytls

tls.o: \
compile tls.c conf-tls
	./compile `grep -h -v "^#" conf-tls` tls.c

ssl_timeoutio.o: \
compile ssl_timeoutio.c ssl_timeoutio.h conf-tls
	./compile `grep -h -v "^#" conf-tls` ssl_timeoutio.c

spawn.o: \
compile chkspawn spawn.c auto_qmail.h auto_uids.h auto_spawn.h \
variables.h indimail_stub.h auto_control.h 
	./chkspawn
	./compile spawn.c

spf.o: \
compile spf.c ipme.h ip.h ipalloc.h strsalloc.h \
conf-spf conf-tls conf-ip
	./compile `grep -h -v "^#" conf-spf conf-tls conf-ip` spf.c

spfquery: \
load spfquery.o spf.o ip.o ipme.o ipalloc.o strsalloc.o \
dns.o variables.o socket_ip4loopback.o socket_v6loopback.o \
socket_v4mappedprefix.o tlsarralloc.o auto_control.o \
socket_v6any.o dns.lib
	./load spfquery spf.o ip.o ipme.o ipalloc.o \
	strsalloc.o auto_control.o socket_ip4loopback.o \
	socket_v6loopback.o tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o dns.o variables.o `cat dns.lib` -lqmail

spfquery.o: \
compile spfquery.c spf.h dns.h conf-spf
	./compile `grep -h -v "^#" conf-spf` spfquery.c

splogger: \
load splogger.o syslog.lib socket.lib
	./load splogger `cat syslog.lib` `cat socket.lib` -lqmail

splogger.o: \
compile splogger.c
	./compile splogger.c

srs.lib: trysrs.c compile load conf-srs dns.lib
	((./compile -I/usr/include/srs2 \
	`grep -h -v "^#" conf-srs` trysrs.c && \
	./load trysrs -lsrs2 ) >/dev/null 2>&1 \
	&& echo "-lsrs2" || exit 0 ) > srs.lib
	rm -f trysrs.o trysrs

hassrs.h:  trysrs.c compile load srs.lib conf-indi
	((./compile -I/usr/include/srs2 `grep -h -v "^#" conf-srs` \
	trysrs.c && ./load trysrs `cat srs.lib`) >/dev/null 2>&1 \
	&& (echo \#define HAVESRS 1) || exit 0 ) > hassrs.h
	rm -f trysrs.o trysrs

srs.o: compile srs.c hassrs.h srs.h auto_qmail.h control.h rcpthosts.h
	./compile -I/usr/include/srs2 srs.c

srsfilter: \
load srsfilter.o srs.o qmail.o control.o rcpthosts.o \
auto_control.o variables.o auto_qmail.o srs.lib
	./load srsfilter srs.o qmail.o control.o rcpthosts.o \
	auto_control.o variables.o auto_qmail.o \
	`cat srs.lib` -lqmail

srsfilter.o: \
compile srsfilter.c qmail.h srs.h hassrs.h
	./compile srsfilter.c

strsalloc.o: \
compile strsalloc.c strsalloc.h
	./compile strsalloc.c

syslog.lib: \
trysyslog.c compile load
	( ( ./compile trysyslog.c && \
	./load trysyslog -lgen ) >/dev/null 2>&1 \
	&& echo -lgen || exit 0 ) > syslog.lib
	rm -f trysyslog.o trysyslog

systype: \
find-systype trycpp.c
	./find-systype > systype

tcp-env: \
load tcp-env.o dns.o remoteinfo.o control.o variables.o \
socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
socket_v4mappedprefix.o socket_tcp.o socket_tcp6.o \
auto_control.o timeoutconn.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o dns.lib socket.lib
	./load tcp-env dns.o remoteinfo.o control.o variables.o \
	auto_control.o socket_ip4loopback.o socket_v6loopback.o \
	tlsarralloc.o socket_v6any.o socket_v4mappedprefix.o \
	socket_tcp.o socket_tcp6.o timeoutconn.o \
	ip.o ipalloc.o strsalloc.o `cat dns.lib socket.lib` -lqmail

tcp-env.o: \
compile tcp-env.c strsalloc.h ip.h dns.h remoteinfo.h \
socket.h haveip6.h conf-spf conf-ip
	./compile `grep -h -v "^#" conf-spf conf-ip` tcp-env.c

tcpto.o: \
compile tcpto.c tcpto.h ip.h \
variables.h ipalloc.h tcpto.h \
conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` tcpto.c

tcpto_clean.o: \
compile tcpto_clean.c tcpto.h ipalloc.h \
tcpto.h ip.h ipalloc.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` tcpto_clean.c

timeoutconn.o: \
compile timeoutconn.c ip.h timeoutconn.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` timeoutconn.c

token822.o: \
compile token822.c token822.h
	./compile token822.c

trigger.o: \
compile trigger.c trigger.h hasnpbg1.h
	./compile trigger.c

triggerpull.o: \
compile triggerpull.c triggerpull.h
	./compile triggerpull.c

wildmat.o: \
compile wildmat.c
	./compile wildmat.c

variables.o: \
compile variables.c variables.h haveip6.h conf-ip
	./compile variables.c

etrn.o: compile etrn.c etrn.h rcpthosts.h auto_qmail.h \
control.h indimail_stub.h auto_control.h variables.h
	./compile etrn.c

atrn.o: \
compile atrn.c etrn.h rcpthosts.h control.h
	./compile atrn.c

############### Begin queue-fix

queue-fix: \
load queue-fix.o get_uid.o auto_split.o auto_uids.o
	./load queue-fix get_uid.o auto_split.o auto_uids.o -lqmail

queue-fix.o: \
compile queue-fix.c direntry.h auto_split.h auto_uids.h tcpto.h
	./compile queue-fix.c

qbase64: \
load qbase64.o
	./load qbase64 -lqmail

qbase64.o: \
compile qbase64.c
	./compile qbase64.c

maildirdeliver: load maildirdeliver.o
	./load maildirdeliver -lqmail

maildirdeliver.o: compile maildirdeliver.c
	./compile maildirdeliver.c

qmail-rm: load qmail-rm.o auto_split.o auto_qmail.o \
control.o auto_control.o variables.o 
	./load qmail-rm auto_split.o auto_qmail.o \
	control.o auto_control.o variables.o -lqmail

qmail-rm.o: \
compile qmail-rm.c auto_split.h auto_qmail.h \
control.h
	./compile qmail-rm.c

qmail-rm.1: qmail-rm.9 conf-qmail
	cat qmail-rm.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-rm.1

autoresponder: load autoresponder.o auto_qmail.o \
variables.o control.o quote.o auto_control.o \
ip.o ipme.o ipalloc.o socket_v4mappedprefix.o socket_v6any.o \
date822fmt.o qregex.o matchregex.o wildmat.o \
myctime.o
	./load autoresponder auto_control.o auto_qmail.o \
	variables.o control.o quote.o myctime.o \
	date822fmt.o ip.o ipme.o ipalloc.o \
	socket_v4mappedprefix.o socket_v6any.o qregex.o \
	matchregex.o wildmat.o -lqmail

autoresponder.o: \
compile autoresponder.c date822fmt.h \
auto_qmail.h control.h ip.h ipme.h quote.h \
qregex.h conf-mime 
	./compile -DMAKE_SEEKABLE -DQUOTE `grep -h -v "^#" conf-mime` autoresponder.c

822field: \
load 822field.o
	./load 822field -lqmail

822field.o: \
compile 822field.c
	./compile 822field.c

822header: \
load 822header.o
	./load 822header -lqmail

822header.o: \
compile 822header.c
	./compile 822header.c

# qtools
822body: load 822body.o
	./load 822body -lqmail

822body.o: compile 822body.c
	./compile 822body.c

822headerok: load 822headerok.o
	./load 822headerok -lqmail

822headerok.o: compile 822headerok.c
	./compile 822headerok.c

822bodyfilter: load 822bodyfilter.o pathexec_run.o
	./load 822bodyfilter pathexec_run.o -lqmail

822bodyfilter.o: compile 822bodyfilter.c pathexec.h
	./compile 822bodyfilter.c

822headerfilter: load 822headerfilter.o pathexec_run.o
	./load 822headerfilter pathexec_run.o -lqmail

822headerfilter.o: compile 822headerfilter.c pathexec.h
	./compile 822headerfilter.c

822addr: load 822addr.o
	./load 822addr -lqmail

822addr.o: compile 822addr.c
	./compile 822addr.c

822fields: load 822fields.o
	./load 822fields -lqmail

822fields.o: compile 822fields.c
	./compile 822fields.c

checkaddr: load checkaddr.o
	./load checkaddr -lqmail

checkaddr.o: compile checkaddr.c
	./compile checkaddr.c

checkdomain: load checkdomain.o
	./load checkdomain -lqmail

checkdomain.o: compile checkdomain.c
	./compile checkdomain.c

filterto.o: compile filterto.c variables.h auto_control.h \
qmail.h pathexec.h envdir.h auto_sysconfdir.h
	./compile filterto.c

filterto: load filterto.o auto_qmail.o qmail.o \
openreadclose.o envdir_set.o pathexec_env.o variables.o \
auto_sysconfdir.o auto_control.o
	./load filterto auto_qmail.o qmail.o variables.o \
	openreadclose.o envdir_set.o pathexec_env.o pathexec_run.o \
	auto_sysconfdir.o auto_control.o -lqmail

filterto.1: filterto.9 conf-sysconfdir
	cat filterto.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> filterto.1

condtomaildir: load condtomaildir.o pathexec_run.o
	./load condtomaildir pathexec_run.o -lqmail

condtomaildir.o: compile condtomaildir.c pathexec.h
	./compile condtomaildir.c

iftoccfrom: load iftoccfrom.o
	./load iftoccfrom -lqmail

iftoccfrom.o: compile iftoccfrom.c
	./compile iftoccfrom.c

ifaddr: load ifaddr.o
	./load ifaddr -lqmail

ifaddr.o: compile ifaddr.c
	./compile ifaddr.c

replier: load replier.o qmail.o variables.o pathexec_env.o \
pathexec_run.o openreadclose.o getconf.o auto_qmail.o \
auto_sysconfdir.o auto_control.o envdir_set.o
	./load replier qmail.o variables.o auto_control.o \
	pathexec_env.o pathexec_run.o openreadclose.o \
	auto_sysconfdir.o getconf.o auto_qmail.o \
	envdir_set.o -lqmail

replier.o: compile replier.c auto_sysconfdir.h variables.h \
getconf.h pathexec.h auto_control.h
	./compile replier.c

replier.1: replier.9 conf-sysconfdir
	cat replier.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> replier.1

replier-config: load replier-config.o auto_ezmlm.o auto_qmail.o
	./load replier-config auto_ezmlm.o auto_qmail.o -lqmail

replier-config.o: compile replier-config.c \
auto_ezmlm.h auto_qmail.h help.t
	./compile replier-config.c

############### serialsmtp ###################################

serialsmtp: \
load serialsmtp.o quote.o
	./load serialsmtp quote.o -lqmail

serialsmtp.o: \
compile serialsmtp.c quote.h conf-ip
	./compile `grep -h -v "^#" conf-ip` serialsmtp.c

maildirserial: \
load maildirserial.o auto_qmail.o qmail.o maildir.o quote.o \
openreadclose.o envdir_set.o pathexec_env.o pathexec_run.o variables.o \
auto_sysconfdir.o auto_control.o prioq.o
	./load maildirserial auto_qmail.o qmail.o maildir.o variables.o \
	openreadclose.o envdir_set.o pathexec_env.o pathexec_run.o \
	auto_control.o auto_sysconfdir.o quote.o prioq.o \
	-lqmail

maildirserial.1: maildirserial.9 conf-sysconfdir
	cat maildirserial.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> maildirserial.1

maildirserial.o: \
compile maildirserial.c prioq.h \
variables.h qmail.h auto_sysconfdir.h auto_control.h \
auto_sysconfdir.h conf-mime
	./compile `grep -h -v "^#" conf-mime` maildirserial.c

serialcmd: load serialcmd.o quote.o
	./load serialcmd quote.o -lqmail

serialcmd.o: \
compile serialcmd.c quote.h
	./compile serialcmd.c

maildircmd: \
warn-auto.sh maildircmd.sh conf-prefix conf-sysconfdir
	cat warn-auto.sh maildircmd.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> maildircmd

rewind: \
load rewind.o
	./load rewind -lqmail

rewind.o: \
compile rewind.c
	./compile rewind.c

serialqmtp: \
load serialqmtp.o
	./load serialqmtp -lqmail

serialqmtp.o: \
compile serialqmtp.c
	./compile serialqmtp.c

maildirsmtp: \
warn-auto.sh maildirsmtp.sh conf-prefix conf-sysconfdir
	cat warn-auto.sh maildirsmtp.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> maildirsmtp

maildirqmtp: \
warn-auto.sh maildirqmtp.sh conf-prefix
	cat warn-auto.sh maildirqmtp.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> maildirqmtp

leapsecs: load leapsecs.o
	./load leapsecs -lqmail

leapsecs.o: compile leapsecs.c auto_qmail.h
	./compile leapsecs.c

leapsecs.dat: leapsecs.txt leapsecs
	./leapsecs < leapsecs.txt > leapsecs.dat

###### qmailanalog

matchup: load matchup.o 
	./load matchup -lqmail

matchup.o: compile matchup.c
	./compile matchup.c

mlmatchup: load mlmatchup.o
	./load mlmatchup -lqmail

mlmatchup.8: mlmatchup.9 conf-prefix
	cat mlmatchup.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> mlmatchup.8

mlmatchup.o: compile mlmatchup.c
	./compile mlmatchup.c

successes: \
warn-auto.sh successes.sh
	cat warn-auto.sh successes.sh \
	> successes

suids: \
warn-auto.sh suids.sh
	cat warn-auto.sh suids.sh \
	> suids

xqp: \
warn-auto.sh xqp.sh
	cat warn-auto.sh xqp.sh \
	> xqp

xrecipient: \
warn-auto.sh xrecipient.sh
	cat warn-auto.sh xrecipient.sh \
	> xrecipient

xsender: \
warn-auto.sh xsender.sh
	cat warn-auto.sh xsender.sh \
	> xsender

zddist: \
warn-auto.sh zddist.sh conf-libexec
	cat warn-auto.sh zddist.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zddist

ddist: \
warn-auto.sh ddist.sh
	cat warn-auto.sh ddist.sh \
	> ddist

zdeferrals: \
warn-auto.sh zdeferrals.sh conf-libexec
	cat warn-auto.sh zdeferrals.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zdeferrals

deferrals: warn-auto.sh deferrals.sh
	cat warn-auto.sh deferrals.sh \
	> deferrals

zfailures: \
warn-auto.sh zfailures.sh conf-libexec
	cat warn-auto.sh zfailures.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zfailures

failures: \
warn-auto.sh failures.sh
	cat warn-auto.sh failures.sh \
	> failures

zoverall: \
warn-auto.sh zoverall.sh
	cat warn-auto.sh zoverall.sh \
	> zoverall

zrecipients: \
warn-auto.sh zrecipients.sh conf-libexec
	cat warn-auto.sh zrecipients.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zrecipients

zrhosts: \
warn-auto.sh zrhosts.sh conf-libexec
	cat warn-auto.sh zrhosts.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zrhosts

zsmtp: \
warn-auto.sh zsmtp.sh conf-libexec
	cat warn-auto.sh zsmtp.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zsmtp

zrxdelay: \
warn-auto.sh zrxdelay.sh conf-libexec
	cat warn-auto.sh zrxdelay.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zrxdelay

zsenders: \
warn-auto.sh zsenders.sh conf-libexec
	cat warn-auto.sh zsenders.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zsenders

zsendmail: \
warn-auto.sh zsendmail.sh
	cat warn-auto.sh zsendmail.sh \
	> zsendmail

zsuccesses: \
warn-auto.sh zsuccesses.sh conf-libexec
	cat warn-auto.sh zsuccesses.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zsuccesses

zsuids: \
warn-auto.sh zsuids.sh conf-libexec
	cat warn-auto.sh zsuids.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zsuids

multilog-matchup: \
warn-auto.sh multilog-matchup.sh conf-prefix
	cat warn-auto.sh multilog-matchup.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> multilog-matchup

smtp-matchup: \
warn-auto.sh smtp-matchup.sh
	cat warn-auto.sh smtp-matchup.sh \
	> smtp-matchup

recipients: \
warn-auto.sh recipients.sh
	cat warn-auto.sh recipients.sh \
	> recipients

rhosts: \
warn-auto.sh rhosts.sh
	cat warn-auto.sh rhosts.sh \
	> rhosts

envmigrate: \
warn-auto.sh envmigrate.sh
	cat warn-auto.sh envmigrate.sh \
	> envmigrate

rsmtp: \
warn-auto.sh rsmtp.sh conf-sysconfdir
	cat warn-auto.sh rsmtp.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtp

rsmtpfailures: \
warn-auto.sh rsmtpfailures.sh
	cat warn-auto.sh rsmtpfailures.sh \
	> rsmtpfailures

rsmtpsdomains: \
warn-auto.sh rsmtpsdomains.sh conf-sysconfdir
	cat warn-auto.sh rsmtpsdomains.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtpsdomains

rsmtprdomains: \
warn-auto.sh rsmtprdomains.sh conf-sysconfdir
	cat warn-auto.sh rsmtprdomains.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtprdomains

rsmtprecipients: \
warn-auto.sh rsmtprecipients.sh conf-sysconfdir
	cat warn-auto.sh rsmtprecipients.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtprecipients

rsmtpsenders: \
warn-auto.sh rsmtpsenders.sh conf-sysconfdir
	cat warn-auto.sh rsmtpsenders.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtpsenders

rxdelay: \
warn-auto.sh rxdelay.sh
	cat warn-auto.sh rxdelay.sh \
	> rxdelay

rspamrdomain: \
warn-auto.sh rspamrdomain.sh conf-sysconfdir
	cat warn-auto.sh rspamrdomain.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rspamrdomain

rspamsdomain: \
warn-auto.sh rspamsdomain.sh conf-sysconfdir
	cat warn-auto.sh rspamsdomain.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rspamsdomain

zspam: \
warn-auto.sh zspam.sh conf-libexec
	cat warn-auto.sh zspam.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zspam

dk-filter: \
warn-auto.sh dk-filter.sh conf-prefix conf-sysconfdir
	cat warn-auto.sh dk-filter.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> dk-filter

senders: \
warn-auto.sh senders.sh conf-qmail
	cat warn-auto.sh senders.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> senders

fmt_ushort.o: \
compile fmt_ushort.c fmt.h
	./compile fmt_ushort.c

columnt: \
load columnt.o slurpclose.o
	./load columnt slurpclose.o -lqmail

columnt.o: compile columnt.c slurpclose.h
	./compile columnt.c

columnt.o: compile columnt.c slurpclose.h \
	./compile columnt.c

qmail-cat: load  qmail-cat.o
	./load qmail-cat -lqmail

qmail-cat.o: compile qmail-cat.c
	./compile qmail-cat.c

qaes: load qaes.o
	./load qaes -lcrypto -lqmail

qaes.o: compile qaes.c
	./compile qaes.c

qarf: \
load  qarf.o date822fmt.o
	./load qarf date822fmt.o -lqmail

qarf.o: compile qarf.c date822fmt.h
	./compile qarf.c

qnotify: \
load  qnotify.o date822fmt.o qmail.o auto_qmail.o \
pathexec_env.o pathexec_run.o envdir_set.o variables.o \
openreadclose.o auto_sysconfdir.o \
auto_control.o
	./load qnotify date822fmt.o qmail.o \
	auto_qmail.o pathexec_env.o pathexec_run.o envdir_set.o \
	variables.o openreadclose.o auto_sysconfdir.o \
	auto_control.o -lqmail

qnotify.o: \
compile qnotify.c date822fmt.h qmail.h variables.h \
envdir.h pathexec.h auto_sysconfdir.h \
auto_control.h
	./compile qnotify.c

rrt.o: \
compile rrt.c auto_sysconfdir.h control.h qmail.h \
date822fmt.h variables.h envdir.h \
pathexec.h auto_control.h
	./compile rrt.c

rrt: \
load rrt.o auto_qmail.o qmail.o control.o variables.o \
auto_control.o envdir_set.o pathexec_env.o \
auto_sysconfdir.o pathexec_run.o openreadclose.o  date822fmt.o
	./load rrt auto_qmail.o qmail.o control.o variables.o \
	envdir_set.o pathexec_env.o pathexec_run.o \
	auto_sysconfdir.o auto_control.o openreadclose.o \
	date822fmt.o -lqmail


qmail-poppass: \
load  qmail-poppass.o auto_uids.o MakeArgs.o
	./load qmail-poppass auto_uids.o MakeArgs.o -lqmail

qmail-poppass.o: \
compile qmail-poppass.c auto_uids.h MakeArgs.h
	./compile qmail-poppass.c

rpmattr: \
load  rpmattr.o auto_uids.o get_uid.o
	./load rpmattr auto_uids.o get_uid.o -lqmail
	

rpmattr.o: compile rpmattr.c auto_uids.h
	./compile rpmattr.c

rspamstat: \
warn-auto.sh rspamstat.sh conf-sysconfdir
	cat warn-auto.sh rspamstat.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rspamstat

rspamhist: \
warn-auto.sh rspamhist.sh
	cat warn-auto.sh rspamhist.sh \
	> rspamhist

iftocc: load iftocc.o
	./load iftocc -lqmail

iftocc.o: compile iftocc.c
	./compile iftocc.c

ofmipd: load ofmipd.o rwhconfig.o \
qmail.o auto_sysconfdir.o control.o auto_control.o variables.o \
auto_qmail.o
	./load ofmipd rwhconfig.o qmail.o \
	auto_sysconfdir.o variables.o auto_qmail.o control.o \
	auto_control.o -lqmail

ofmipd.o: compile ofmipd.c qmail.h \
auto_sysconfdir.h variables.h \
rwhconfig.h
	./compile ofmipd.c

rwhconfig.o: compile rwhconfig.c \
rwhconfig.h auto_qmail.h variables.h auto_control.h
	./compile rwhconfig.c

ofmipname: load ofmipname.o cdbmss.o
	./load ofmipname cdbmss.o -lqmail

ofmipname.o: \
compile ofmipname.c cdbmss.h
	./compile ofmipname.c

new-inject: \
load new-inject.o qmail.o auto_sysconfdir.o rwhconfig.o \
envdir_set.o pathexec_env.o pathexec_run.o openreadclose.o \
variables.o auto_qmail.o auto_control.o
	./load new-inject qmail.o auto_sysconfdir.o variables.o \
	auto_qmail.o envdir_set.o pathexec_env.o pathexec_run.o \
	openreadclose.o auto_control.o rwhconfig.o -lqmail

new-inject.1: new-inject.9 conf-sysconfdir
	cat new-inject.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> new-inject.1

new-inject.o: \
compile new-inject.c variables.h \
auto_sysconfdir.h qmail.h \
rwhconfig.h envdir.h pathexec.h auto_control.h
	./compile new-inject.c

822date: load 822date.o auto_sysconfdir.o
	./load 822date auto_sysconfdir.o -lqmail

822date.o: compile 822date.c auto_sysconfdir.h
	./compile 822date.c

822received: load 822received.o auto_sysconfdir.o
	./load 822received auto_sysconfdir.o -lqmail

822received.o: compile 822received.c \
auto_sysconfdir.h
	./compile 822received.c

822print: load 822print.o auto_sysconfdir.o
	./load 822print auto_sysconfdir.o -lqmail

822print.o: compile 822print.c auto_sysconfdir.h
	./compile 822print.c

parsedate: load parsedate.o auto_sysconfdir.o
	./load parsedate auto_sysconfdir.o -lqmail

parsedate.o: compile parsedate.c auto_sysconfdir.h
	./compile parsedate.c

tokenize: load tokenize.o
	./load tokenize -lqmail

tokenize.o: compile tokenize.c
	./compile tokenize.c

addrlist: load addrlist.o
	./load addrlist -lqmail

addrlist.o: compile addrlist.c
	./compile addrlist.c

qquote: load qquote.o
	./load qquote -lqmail

qquote.o: compile qquote.c
	./compile qquote.c

pathexec_run.o: compile pathexec_run.c pathexec.h
	./compile pathexec_run.c

pathexec_env.o: compile pathexec_env.c pathexec.h
	./compile pathexec_env.c

tomaildir: load tomaildir.o sig.a open.a  env.a \
fs.a now.o
	./load tomaildir sig.a open.a  env.a fs.a \
	now.o

tomaildir.o: compile tomaildir.c \
sig.h byte.h exit.h substdio.h strerr.h error.h fmt.h \
str.h now.h datetime.h env.h
	./compile tomaildir.c

getconf.o: compile getconf.c openreadclose.h getconf.h
	./compile getconf.c

openreadclose.o: compile openreadclose.c openreadclose.h
	./compile openreadclose.c

auto_ezmlm.o: compile \
auto_ezmlm.c
	./compile auto_ezmlm.c

auto_ezmlm.c: auto-str conf-ezmlm
	./auto-str auto_ezmlm `head -1 conf-ezmlm` > auto_ezmlm.c

make-text: load make-text.o
	./load make-text -lqmail

make-text.o: compile make-text.c
	./compile make-text.c

help.t: make-text help.txt
	./make-text <help.txt >help.t

spipe: load spipe.o svcfns.o
	./load spipe svcfns.o

spipe.o: compile spipe.c bool.h svcfns.h
	./compile spipe.c

svcfns.o: compile svcfns.c bool.h svcfns.h
	./compile svcfns.c

qfilelog: load qfilelog.o
	./load qfilelog

qfilelog.o: compile qfilelog.c
	./compile qfilelog.c

multipipe: load multipipe.o svcfns.o
	./load multipipe svcfns.o

multipipe.o: compile multipipe.c direntry.h bool.h svcfns.h
	./compile multipipe.c

teepipe: load teepipe.o
	./load teepipe

teepipe.o: compile teepipe.c
	./compile teepipe.c

cert cert-req: Makefile-cert
	@$(MAKE) -sf $< $@

Makefile-cert: conf-sysconfdir Makefile-cert.mk
	@cat Makefile-cert.mk \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> $@

update_tmprsadh: conf-qmail update_tmprsadh.sh conf-sysconfdir
	@cat update_tmprsadh.sh\
	| sed s}@sysconfdir\@}"`head -1 conf-sysconfdir`"}g \
	> $@

tmprsadh: update_tmprsadh
	echo "Creating new temporary RSA and DH parameters"
	./update_tmprsadh

#
# Domain KEYS
#
domainkeys.h: domainkeys.c makeheader  dktrace.h
	./makeheader <domainkeys.c >domainkeys.h

makeheader: load makeheader.o
	./load makeheader

makehader.o: makeheader.c compile
	./compile makeheader.c

dot-forward: \
load dot-forward.o control.o qmail.o auto_qmail.o variables.o \
openreadclose.o envdir_set.o pathexec_env.o token822.o \
auto_sysconfdir.o auto_control.o pathexec_run.o
	./load dot-forward control.o qmail.o auto_qmail.o variables.o \
	openreadclose.o envdir_set.o pathexec_env.o pathexec_run.o \
	auto_sysconfdir.o auto_control.o token822.o -lqmail

dot-forward.1: dot-forward.9 conf-sysconfdir
	cat dot-forward.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> dot-forward.1

dot-forward.o: compile dot-forward.c token822.h control.h qmail.h \
auto_sysconfdir.h variables.h envdir.h pathexec.h auto_control.h
	./compile dot-forward.c

fastforward: \
load fastforward.o slurpclose.o strset.o qmail.o auto_qmail.o \
envdir_set.o pathexec_env.o pathexec_run.o openreadclose.o variables.o \
auto_sysconfdir.o auto_control.o
	./load fastforward slurpclose.o strset.o qmail.o variables.o \
	envdir_set.o pathexec_env.o pathexec_run.o openreadclose.o \
	auto_sysconfdir.o auto_qmail.o auto_control.o -lqmail

fastforward.1: fastforward.9 conf-sysconfdir
	cat fastforward.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> fastforward.1

fastforward.o: \
compile fastforward.c strset.h qmail.h slurpclose.h auto_sysconfdir.h \
envdir.h pathexec.h variables.h auto_control.h
	./compile fastforward.c

strset.o: \
compile strset.c strset.h
	./compile strset.c

printforward: \
load printforward.o
	./load printforward -lqmail

printforward.o: \
compile printforward.c
	./compile printforward.c

printmaillist: \
load printmaillist.o
	./load printmaillist -lqmail

printmaillist.o: \
compile printmaillist.c
	./compile printmaillist.c

setforward: \
load setforward.o cdbmss.o
	./load setforward cdbmss.o -lqmail

setforward.o: \
compile setforward.c cdbmss.h
	./compile setforward.c

setmaillist: \
load setmaillist.o
	./load setmaillist -lqmail

setmaillist.o: \
compile setmaillist.c
	./compile setmaillist.c

inewaliases: \
load inewaliases.o auto_qmail.o token822.o control.o \
cdbmss.o variables.o auto_control.o
	./load inewaliases auto_qmail.o token822.o control.o \
	variables.o cdbmss.o auto_control.o -lqmail

inewaliases.o: \
compile inewaliases.c token822.h control.h \
auto_qmail.h cdbmss.h variables.h
	./compile inewaliases.c

newinclude: \
load newinclude.o auto_qmail.o token822.o control.o \
variables.o auto_control.o
	./load newinclude auto_qmail.o token822.o control.o \
	variables.o auto_control.o -lqmail

newinclude.o: \
compile newinclude.c token822.h control.h \
auto_qmail.h variables.h
	./compile newinclude.c

envdir: load envdir.o envdir_set.o openreadclose.o pathexec_env.o \
pathexec_run.o
	./load envdir envdir_set.o openreadclose.o pathexec_env.o \
	pathexec_run.o -lqmail

envdir.o: compile envdir.c pathexec.h envdir.h
	./compile envdir.c

envdir_set.o: compile envdir_set.c openreadclose.h direntry.h \
pathexec.h
	./compile envdir_set.c

envuidgid: envuidgid.o load pathexec_env.o pathexec_run.o
	./load envuidgid pathexec_env.o pathexec_run.o -lqmail

envuidgid.o: compile envuidgid.c pathexec.h
	./compile envuidgid.c

fghack: fghack.o load pathexec_run.o
	./load fghack pathexec_run.o -lqmail

fghack.o: compile fghack.c pathexec.h
	./compile fghack.c
	
multilog: load multilog.o timestamp.o match.o deepsleep.o \
iopause.o
	./load multilog timestamp.o match.o deepsleep.o \
	iopause.o -lqmail

multilog.o: compile deepsleep.h direntry.h match.h multilog.c \
timestamp.h
	./compile multilog.c

deepsleep.o: compile deepsleep.c deepsleep.h iopause.h
	./compile deepsleep.c

match.o: compile match.c match.h
	./compile match.c

iopause.h: choose compile iopause.h1 iopause.h2 load trypoll.c
	./choose clr trypoll iopause.h1 iopause.h2 > iopause.h

iopause.o: compile iopause.c iopause.h
	./compile iopause.c

pgrphack: load pgrphack.o pathexec_run.o
	./load pgrphack pathexec_run.o -lqmail

pgrphack.o: compile pathexec.h pgrphack.c
	./compile pgrphack.c

readproctitle: load readproctitle.o
	./load readproctitle -lqmail

readproctitle.o: compile readproctitle.c
	./compile readproctitle.c

setlock: load setlock.o pathexec_run.o
	./load setlock pathexec_run.o -lqmail

setlock.o: compile pathexec.h setlock.c
	./compile setlock.c

setuidgid: load setuidgid.o prot.o pathexec_run.o
	./load setuidgid prot.o pathexec_run.o -lqmail

setuidgid.o: compile pathexec.h prot.h setuidgid.c
	./compile setuidgid.c

softlimit: load softlimit.o pathexec_run.o
	./load softlimit pathexec_run.o -lqmail

softlimit.o: compile pathexec.h softlimit.c
	./compile softlimit.c

supervise: deepsleep.o load supervise.o iopause.o
	./load supervise deepsleep.o iopause.o -lqmail

supervise.o: compile supervise.c deepsleep.h iopause.h
	./compile supervise.c

sig.o: compile sig.c sig.h
	./compile sig.c

svc: load svc.o
	./load svc -lqmail

svc.o: compile svc.c
	./compile svc.c

svok: load svok.o
	./load svok -lqmail

svok.o: compile svok.c
	./compile svok.c

svscan: load svscan.o pathexec_run.o auto_sysconfdir.o
	./load svscan pathexec_run.o auto_sysconfdir.o -lqmail

svscan.o: compile direntry.h pathexec.h svscan.c auto_sysconfdir.h
	./compile svscan.c

svstat: load svstat.o
	./load svstat -lqmail

svstat.o: compile svstat.c
	./compile svstat.c

tai64n: load tai64n.o timestamp.o
	./load tai64n timestamp.o -lqmail

tai64n.o: compile tai64n.c timestamp.h
	./compile tai64n.c

tai64nlocal: load tai64nlocal.o
	./load tai64nlocal -lqmail

tai64nlocal.o: compile tai64nlocal.c
	./compile tai64nlocal.c

tai64nunix: \
load tai64nunix.o
	./load tai64nunix -lqmail

tai64nunix.o: \
compile tai64nunix.c
	./compile tai64nunix.c

tai2tai64n: load tai2tai64n.o
	./load tai2tai64n -lqmail

tai2tai64n.o: compile tai2tai64n.c bool.h
	./compile tai2tai64n.c

tai64n2tai: load tai64n2tai.o
	./load tai64n2tai -lqmail

tai64n2tai.o: compile tai64n2tai.c bool.h
	./compile tai64n2tai.c

tai_decode.o: compile tai_decode.c bool.h
	./compile tai_decode.c

tai_encode.o: compile tai_encode.c bool.h
	./compile tai_encode.c

tai64n_decode.o: compile tai64n_decode.c tai.h bool.h
	./compile tai64n_decode.c

tai64n_encode.o: compile tai64n_encode.c tai.h bool.h
	./compile tai64n_encode.c

recordio.o: \
compile recordio.c iopause.h pathexec.h
	./compile recordio.c

recordio: \
load recordio.o pathexec_run.o iopause.o
	./load recordio pathexec_run.o iopause.o -lqmail

argv0.o: \
compile argv0.c pathexec.h
	./compile argv0.c

argv0: \
load argv0.o pathexec_run.o
	./load argv0 pathexec_run.o -lqmail

addcr: load addcr.o 
	./load addcr -lqmail

addcr.o: \
compile addcr.c
	./compile addcr.c

delcr: load delcr.o
	./load delcr -lqmail

delcr.o: \
compile delcr.c
	./compile delcr.c

fixcrio: load fixcrio.o pathexec_run.o iopause.o
	./load fixcrio pathexec_run.o iopause.o -lqmail

fixcrio.o: \
compile fixcrio.c iopause.h pathexec.h
	./compile fixcrio.c

logselect.o: compile direntry.h sorted.h pathexec.h \
timestamp.h logselect.c
	./compile logselect.c

logselect: load logselect.o sorted.o
	./load logselect sorted.o -lqmail

logselect.8: logselect.9 conf-prefix
	cat logselect.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> logselect.8

sorted.o: compile sorted.c sorted.h
	./compile sorted.c

sortedtest: load sortedtest.o sorted.o 
	./load sortedtest sorted.o -lqmail

sortedtest.o: compile sortedtest.c sorted.h
	./compile sortedtest.c

generic.o: compile generic.c MakeArgs.h
	./compile generic.c

relaytest.o: compile relaytest.c
	./compile relaytest.c

relaytest: load relaytest.o
	./load relaytest -lqmail

################### IPV6 ########################################

haveip6.h: \
choose tryip6.c compile haveip6.h1 haveip6.h2 conf-ip
	./compile `grep -h -v "^#" conf-ip` tryip6.c; \
	cc tryip6.o -o tryip6; \
	./choose r tryip6 haveip6.h1 haveip6.h2 > haveip6.h

socket_v4mappedprefix.o: \
compile socket_v4mappedprefix.c conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_v4mappedprefix.c

socket_tcp.o: \
compile socket_tcp.c
	./compile socket_tcp.c

socket_tcp6.o: \
compile socket_tcp6.c socket.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_tcp6.c

socket_ip4loopback.o: \
compile socket_ip4loopback.c
	./compile socket_ip4loopback.c

socket_v6loopback.o: \
compile socket_v6loopback.c conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_v6loopback.c

socket_v6any.o: \
compile socket_v6any.c conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_v6any.c

cdbmake: load cdbmake.o
	./load cdbmake -lqmail

cdbmake.o: \
compile cdbmake.c
	./compile cdbmake.c

cdbget: load cdbget.o
	./load cdbget -lqmail

cdbget.o: \
compile cdbget.c
	./compile cdbget.c

cdbdump: load cdbdump.o
	./load cdbdump -lqmail

cdbdump.o: compile cdbdump.c
	./compile cdbdump.c

cdbstats: load cdbstats.o
	./load cdbstats -lqmail

cdbstats.o: compile cdbstats.c
	./compile cdbstats.c

cdbtest: load cdbtest.o
	./load cdbtest -lqmail

cdbtest.o: compile cdbtest.c
	./compile cdbtest.c

testzero: load testzero.o
	./load testzero -lqmail

testzero.o: compile testzero.c
	./compile testzero.c

cdbmake-12: warn-auto.sh cdbmake-12.sh conf-prefix
	cat warn-auto.sh cdbmake-12.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> cdbmake-12
	chmod 755 cdbmake-12

cdbmake-sv: \
warn-auto.sh cdbmake-sv.sh conf-prefix
	cat warn-auto.sh cdbmake-sv.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> cdbmake-sv
	chmod 755 cdbmake-sv

cdbgetm: \
warn-auto.sh cdbgetm.sh conf-prefix
	cat warn-auto.sh cdbgetm.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> cdbgetm
	chmod 755 cdbgetm

qmail-qfilter: \
load qmail-qfilter.o auto_qmail.o
	./load qmail-qfilter auto_qmail.o -lqmail

qmail-qfilter.o: \
compile qmail-qfilter.c auto_qmail.h qmail.h
	./compile qmail-qfilter.c

qmail-greyd: load qmail-greyd.o ip.o socket_v6any.o \
socket_v4mappedprefix.o variables.o auto_control.o \
control.o auto_qmail.o tablematch.o
	./load qmail-greyd auto_control.o ip.o socket_v6any.o \
	socket_v4mappedprefix.o variables.o control.o \
	auto_qmail.o tablematch.o -lm -lqmail

qmail-greyd.o: \
compile qmail-greyd.c auto_qmail.h ip.h control.h \
variables.h greylist.h tablematch.h socket.h haveip6.h \
conf-ip auto_control.h
	./compile -DDYNAMIC_BUF `grep -h -v "^#" conf-ip` qmail-greyd.c

qmail-daned: \
load qmail-daned.o ip.o socket_v6any.o socket_v4mappedprefix.o \
variables.o auto_control.o control.o auto_qmail.o \
ipalloc.o dns.o dnsdoe.o strsalloc.o socket_tcp.o \
socket_tcp6.o timeoutconn.o tlsarralloc.o \
socket_ip4loopback.o socket_v6loopback.o MakeArgs.o \
dns.lib tls.lib starttls.o
	./load qmail-daned auto_control.o \
	ip.o socket_v6any.o socket_v4mappedprefix.o \
	tlsarralloc.o variables.o control.o auto_qmail.o \
	ipalloc.o dns.o dnsdoe.o strsalloc.o socket_tcp.o \
	socket_tcp6.o timeoutconn.o socket_ip4loopback.o \
	socket_v6loopback.o starttls.o MakeArgs.o \
	`cat dns.lib tls.lib` -lm -lqmail

qmail-daned.o: \
compile qmail-daned.c control.h variables.h \
tlsacheck.h haveip6.h ip.h auto_qmail.h auto_control.h \
hastlsa.h conf-ip conf-tls MakeArgs.h
	./compile -DDYNAMIC_BUF `grep -h -v "^#" conf-ip conf-tls` \
	qmail-daned.c

qmail-daned.8: \
qmail-daned.9 conf-qmail conf-sysconfdir
	cat qmail-daned.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-daned.8

starttls.o: compile starttls.c conf-tls hastlsa.h conf-ip \
socket.h tls.h ip.h dns.h control.h variables.h tlsacheck.h \
ipalloc.h tlsarralloc.h auto_control.h haveip6.h
	./compile `grep -h -v "^#" conf-ip conf-tls` starttls.c

tlsacheck.o: \
compile tlsacheck.c haveip6.h ip.h \
tlsacheck.h fn_handler.h query_skt.h
	./compile `grep -h -v "^#" conf-ip` tlsacheck.c

qdane: \
load qdane.o tlsacheck.o ip.o query_skt.o fn_handler.o \
socket_v4mappedprefix.o socket_v6loopback.o \
ip.o socket_ip4loopback.o socket_v6any.o
	./load qdane tlsacheck.o query_skt.o fn_handler.o \
	socket_v4mappedprefix.o socket_v6loopback.o \
	ip.o socket_ip4loopback.o socket_v6any.o -lqmail

qdane.o: \
compile qdane.c tlsacheck.h
	./compile qdane.c

batv: load batv.o
	./load batv -lssl -lcrypto -lqmail

batv.o: compile batv.c conf-batv
	./compile `grep -h -v "^#" conf-batv` batv.c

uacl: \
load uacl.o mail_acl.o matchregex.o wildmat.o control.o \
variables.o qregex.o auto_qmail.o auto_control.o 
	./load uacl mail_acl.o matchregex.o wildmat.o control.o \
	variables.o qregex.o auto_qmail.o auto_control.o -lqmail

uacl.o: \
compile uacl.c qregex.h auto_qmail.h mail_acl.h \
matchregex.h control.h
	./compile uacl.c

qmail-sql: \
load qmail-sql.o variables.o auto_control.o \
load_mysql.o sqlmatch.o auto_uids.o auto_sysconfdir.o indimail_stub.o \
control.o auto_qmail.o tls.lib
	./load qmail-sql indimail_stub.o \
	variables.o auto_control.o \
	control.o sqlmatch.o auto_uids.o auto_sysconfdir.o \
	auto_qmail.o load_mysql.o `cat tls.lib` -ldl -lqmail

qmail-sql.o: \
compile qmail-sql.c sqlmatch.h auto_qmail.h variables.h \
auto_uids.h auto_control.h hasmysql.h load_mysql.h \
mysql_config conf-sql mysql_inc
	./compile `grep -h -v "^#" conf-sql mysql_inc` qmail-sql.c

qmail-sql.8: \
qmail-sql.9 conf-sysconfdir
	cat qmail-sql.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-sql.8

sql-database: \
load sql-database.o variables.o auto_control.o \
load_mysql.o sqlmatch.o auto_uids.o auto_sysconfdir.o indimail_stub.o \
control.o auto_qmail.o tls.lib
	./load sql-database indimail_stub.o \
	variables.o auto_control.o \
	control.o sqlmatch.o auto_uids.o auto_sysconfdir.o \
	auto_qmail.o load_mysql.o `cat tls.lib` -ldl -lqmail

sql-database.o: \
compile sql-database.c sqlmatch.h auto_qmail.h variables.h \
auto_uids.h auto_control.h hasmysql.h load_mysql.h \
mysql_config conf-sql mysql_inc
	./compile `grep -h -v "^#" conf-sql mysql_inc` sql-database.c

sql-database.8: \
sql-database.9 conf-sysconfdir
	cat sql-database.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> sql-database.8
	
sys-checkpwd: load sys-checkpwd.o MakeArgs.o \
shadow.lib crypt.lib
	./load sys-checkpwd MakeArgs.o \
	`cat shadow.lib crypt.lib` -lqmail

sys-checkpwd.o: \
compile sys-checkpwd.c pathexec.h hasspnam.h \
hasuserpw.h MakeArgs.h
	./compile sys-checkpwd.c

hasifaddr.h: \
tryifaddr.c compile load
	((./compile tryifaddr.c \
	&& ./load tryifaddr) >/dev/null 2>&1 \
	&& echo \#define HASIFADDR 1 || exit 0 ) > hasifaddr.h
	rm -f tryifaddr.o tryifaddr

hasspnam.h: \
tryspnam.c compile load
	((./compile tryspnam.c \
	&& ./load tryspnam) >/dev/null 2>&1 \
	&& echo \#define HASGETSPNAM 1 || exit 0 ) > hasspnam.h
	rm -f tryspnam.o tryspnam

hasuserpw.h: \
tryuserpw.c compile load
	((./compile tryuserpw.c \
	  && ./load tryuserpw) >/dev/null 2>&1 \
	&& echo \#define HASGETUSERPW 1 || exit 0 ) > hasuserpw.h
	rm -f tryuserpw.o tryuserpw

crypt.lib: \
trycrypt.c compile load
	((./compile trycrypt.c && \
	./load trycrypt -lcrypt) >/dev/null 2>&1 \
	&& echo -lcrypt || exit 0) > crypt.lib
	rm -f trycrypt.o trycrypt

ldap.lib: \
tryldap.c compile load hasldap.h
	((./compile tryldap.c && \
	./load tryldap -lldap ) >/dev/null 2>&1 \
	&& echo -lldap || exit 0) > ldap.lib
	rm -f tryldap.o tryldap

hasldap.h: \
tryldap.c compile load
	((./compile tryldap.c && \
	./load tryldap -lldap) >/dev/null 2>&1 \
	&& echo "#define HASLDAP 1" || exit 0) > hasldap.h
	rm -f tryldap.o tryldap

shadow.lib: \
tryshadow.c compile load
	((./compile tryshadow.c && \
	./load tryshadow -lshadow) >/dev/null 2>&1 \
	&& echo -lshadow || exit 0) > shadow.lib
	rm -f tryshadow.o tryshadow

ldap-checkpwd: \
load ldap-checkpwd.o ldap.lib
	./load ldap-checkpwd `cat ldap.lib` -lqmail

ldap-checkpwd.o: \
compile ldap-checkpwd.c pathexec.h hasldap.h
	./compile -DEXTENDED_ATTRIBUTES -DLDAP_DEPRECATED ldap-checkpwd.c

surblfilter.8: surblfilter.9 conf-qmail conf-sysconfdir
	cat surblfilter.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> surblfilter.8

surblfilter: \
load surblfilter.o control.o tlsarralloc.o \
auto_control.o variables.o auto_qmail.o \
strsalloc.o ip.o dns.o ipalloc.o \
socket_v6any.o socket_v4mappedprefix.o \
dns.lib
	./load surblfilter auto_control.o control.o variables.o \
	auto_qmail.o tlsarralloc.o strsalloc.o \
	ip.o dns.o ipalloc.o socket_v6any.o socket_v4mappedprefix.o \
	`cat dns.lib` -lqmail

surblfilter.o: \
compile surblfilter.c auto_qmail.h control.h dns.h ip.h \
ipalloc.h variables.h qmail.h auto_control.h
	./compile surblfilter.c

surblqueue: \
warn-auto.sh surblqueue.sh conf-prefix
	cat warn-auto.sh surblqueue.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> surblqueue
	chmod 755 surblqueue

drate: \
load drate.o evaluate.o variables.o myctime.o auto_uids.o auto_control.o
	./load drate evaluate.o variables.o myctime.o \
	auto_uids.o auto_control.o -lm -lqmail

drate.o: \
compile drate.c evaluate.h variables.h myctime.h \
auto_uids.h auto_control.h
	./compile drate.c

cidr: load cidr.o
	./load cidr -lm

cidr.o: compile cidr.c
	./compile cidr.c

nowutc: load nowutc.o auto_qmail.o auto_sysconfdir.o
	./load nowutc auto_sysconfdir.o -lqmail

nowutc.o: compile nowutc.c auto_sysconfdir.h
	./compile nowutc.c

yearcal: load yearcal.o 
	./load yearcal -lqmail

yearcal.o: compile yearcal.c
	./compile yearcal.c

udplogger: load udplogger.o 
	./load udplogger -lqmail

udplogger.o: compile udplogger.c conf-ip haveip6.h
	./compile -DDYNAMIC_BUF `grep -h -v "^#" conf-ip` udplogger.c

udpclient: load udpclient.o udpopen.o
	./load udpclient udpopen.o -lqmail

udpopen.o: compile udpopen.c udpopen.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` udpopen.c

tcpopen.o: compile tcpopen.c tcpopen.h haveip6.h conf-ip
	./compile -DQMAIL_INTERNAL `grep -h -v "^#" conf-ip` tcpopen.c

udpclient.o: compile udpclient.c
	./compile udpclient.c

whois: \
load whois.o tcpopen.o
	./load whois tcpopen.o -lqmail

whois.o:\
compile whois.c tcpopen.h
	./compile `grep -h -v "^#" conf-ip` whois.c

conf-release:
	if test ! -f conf-release ; then \
		echo "1.1" > conf-release; \
	fi

indimail-mta.spec: indimail-mta.spec.in doc/ChangeLog mysql_config \
conf-release conf-version ../ucspi-tcp-x/conf-version
	(cat $@.in; sh ./catChangeLog) | $(edit) > indimail-mta.spec

debian/Makefile: debian/Makefile.in conf-release conf-version \
../ucspi-tcp-x/conf-version
	$(edit) $@.in > $@

hastlsv1_1_server.h: \
trytlsv1_1_server.c compile load
	((./compile trytlsv1_1_server.c && ./load trytlsv1_1_server \
	-lssl -lcrypto) >/dev/null 2>&1 \
	&& echo \#define TLSV1_1_SERVER_METHOD 1 || exit 0 ) > hastlsv1_1_server.h
	rm -f trytlsv1_1_server.o trytlsv1_1_server

hastlsv1_2_server.h: \
trytlsv1_2_server.c compile load
	((./compile trytlsv1_2_server.c && ./load trytlsv1_2_server \
	-lssl -lcrypto) >/dev/null 2>&1 \
	&& echo \#define TLSV1_2_SERVER_METHOD 1 || exit 0 ) > hastlsv1_2_server.h
	rm -f trytlsv1_2_server.o trytlsv1_2_server

hastlsv1_1_client.h: \
trytlsv1_1_client.c compile load
	(( ./compile trytlsv1_1_client.c && ./load trytlsv1_1_client \
	-lssl -lcrypto) >/dev/null 2>&1 \
	&& echo \#define TLSV1_1_CLIENT_METHOD 1 || exit 0 ) > hastlsv1_1_client.h
	rm -f trytlsv1_1_client.o trytlsv1_1_client

hastlsv1_2_client.h: \
trytlsv1_2_client.c compile load
	(( ./compile trytlsv1_2_client.c && ./load trytlsv1_2_client \
	-lssl -lcrypto) >/dev/null 2>&1 \
	&& echo \#define TLSV1_2_CLIENT_METHOD 1 || exit 0 ) > hastlsv1_2_client.h
	rm -f trytlsv1_2_client.o trytlsv1_2_client

hasinotify.h: \
tryinotify.c compile load
	( ( ./compile tryinotify.c && ./load tryinotify ) >/dev/null \
	2>&1 \
	&& echo \#define HASINOTIFY 1 || exit 0 ) > hasinotify.h
	rm -f tryinotify.o tryinotify

inotify: load inotify.o
	./load inotify -lqmail

inotify.o: compile inotify.c hasinotify.h
	./compile inotify.c

maildirsize: \
load maildirsize.o qcount_dir.o
	./load maildirsize qcount_dir.o -lqmail

maildirsize.o: \
compile maildirsize.c indimail_stub.h auto_control.h variables.h
	./compile maildirsize.c

qcount_dir.o: \
compile qcount_dir.c
	./compile qcount_dir.c

ctrlenv: \
load ctrlenv.o control.o auto_control.o \
pathexec_env.o pathexec_run.o variables.o \
load_mysql.o indimail_stub.o sqlmatch.o \
auto_qmail.o auto_uids.o auto_sysconfdir.o
	./load ctrlenv auto_control.o control.o \
	pathexec_env.o pathexec_run.o variables.o \
	load_mysql.o indimail_stub.o sqlmatch.o \
	auto_qmail.o auto_uids.o auto_sysconfdir.o \
	-lqmail -ldl

ctrlenv.o: compile ctrlenv.c variables.h \
hasmysql.h load_mysql.h indimail_stub.h \
auto_control.h sqlmatch.h mysql_config \
mysql_inc conf-sql
	./compile `grep -h -v "^#" conf-sql mysql_inc` ctrlenv.c

ctrlenv.8: \
ctrlenv.9 conf-prefix conf-sysconfdir
	cat ctrlenv.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}CONTROL}"`head -1 conf-sysconfdir`"}g \
	> ctrlenv.8

################### PLUGINS ########################################
# dnsblcheck plugin
smtpd-plugin0.so: dnsbl.o control.o auto_control.o variables.o \
socket_v6any.o socket_v4mappedprefix.o ip.o ipalloc.o \
strsalloc.o tlsarralloc.o dns.o dns.lib
	if test $(SYSTEM) = DARWIN ; then \
		$(LD) -bundle -undefined dynamic_lookup \
		-o smtpd-plugin0.so dnsbl.o control.o auto_control.o variables.o \
		socket_v6any.o socket_v4mappedprefix.o ip.o ipalloc.o \
		strsalloc.o tlsarralloc.o dns.o `cat dns.lib` -lqmail; \
	elif test $(SYSTEM) = FREEBSD ; then \
		$(LD) -shared -o smtpd-plugin0.so dnsbl.o control.o \
		auto_control.o variables.o socket_v6any.o \
		socket_v4mappedprefix.o ip.o ipalloc.o \
		strsalloc.o tlsarralloc.o dns.o `cat dns.lib` -L/usr/local/lib -lqmail; \
	else \
		$(CC) -Wl,-symbolic -shared -rdynamic -nostartfiles -fPIC -s -O4 \
		-o smtpd-plugin0.so dnsbl.o control.o auto_control.o variables.o \
		socket_v6any.o socket_v4mappedprefix.o ip.o ipalloc.o \
		strsalloc.o tlsarralloc.o dns.o `cat dns.lib` -lqmail; \
	fi

dnsbl.o: compile dnsbl.c smtp_plugin.h dns.h control.h
	./compile dnsbl.c

smtpd-plugin.so:smtp_plugin.o
	if test $(SYSTEM) = DARWIN ; then \
		$(LD) -bundle -undefined dynamic_lookup -o smtpd-plugin.so \
		smtp_plugin.o; \
	elif test $(SYSTEM) = FREEBSD ; then \
		$(LD) -shared -o smtpd-plugin.so smtp_plugin.o; \
	else \
		$(CC) -Wl,-symbolic -shared -rdynamic -nostartfiles -fPIC -s -O4 \
		-o smtpd-plugin.so smtp_plugin.o; \
	fi

smtp_plugin.o: compile smtp_plugin.c smtp_plugin.h
	./compile smtp_plugin.c

generic.so: generic.o auto_scancmd.o MakeArgs.o
	if test $(SYSTEM) = DARWIN ; then \
		$(LD) -bundle -undefined dynamic_lookup -o generic.so generic.o \
		auto_scancmd.o MakeArgs.o -lqmail; \
	elif test $(SYSTEM) = FREEBSD ; then \
		$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 -o \
		generic.so generic.o auto_scancmd.o MakeArgs.o \
		-L/usr/local/lib -lqmail; \
	else \
		$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 -o \
		generic.so generic.o auto_scancmd.o MakeArgs.o -lqmail; \
	fi

perm_list.mta: perm_list.in mysql_config
	$(edit) perm_list.in > $@

svctool: svctool.in mysql_config
	$(edit) $@.in > $@
	chmod +x $@

svctool.8: svctool.9 conf-qmail conf-sysconfdir mysql_config
	$(edit) svctool.9 > $@

create_services: create_services.in
	$(edit) $@.in > $@; chmod 755 $@
	chmod +x $@

docker-entrypoint: \
warn-auto.sh docker-entrypoint.sh conf-prefix
	cat warn-auto.sh docker-entrypoint.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> docker-entrypoint
	chmod 755 docker-entrypoint

svscan_rc: \
warn-auto.sh conf-prefix svscan_rc.in
	(cat warn-auto.sh; $(edit) $@.in) > $@

#################################################################
indimail-mta-permissions.easy: indimail-mta-permissions.easy.in
	$(edit) $@.in > $@
indimail-mta-permissions.secure: indimail-mta-permissions.secure.in
	$(edit) $@.in > $@
indimail-mta-permissions.paranoid: indimail-mta-permissions.paranoid.in
	$(edit) $@.in > $@
