# Don't edit Makefile! Use conf-* for configuration.
SHELL=/bin/sh
include Makevars.$(SYSTEM)
QMAILLIB="-lqmail"

edit = sed \
	-e 's,@indimaildir\@,$(qmaildir),g' \
	-e 's,@qmaildir\@,$(qmaildir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@qsysconfdir\@,$(qsysconfdir),g' \
	-e 's,@sysconfdir\@,$(qsysconfdir),g' \
	-e 's,@controldir\@,$(qsysconfdir)/control,g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@HOST\@,$(HOST),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@email\@,$(email),g' \
	-e 's,@DATE\@,$(DATE),g'

it = 822date 822field 822header 822print 822received addrlist analyzer \
	atrn autoresponder backup.conf batv binm1 binm1+df binm2 binm2+df \
	binm3 binm3+df bouncesaying cdb-database cdbdump cdbget cdbgetm \
	cdbmake cdbmake-12 cdbmake-sv cdbstats cdbtest cidr condredirect \
	config-fast create_services indimail-mta.cron ctrlenv datemail \
	dknewkey dnsblcheck dnscname dnsfq dnsip dnsmxip dnsptr \
	dnstlsarr dnstxt dot-forward drate etrn except fastforward \
	favicon.base64 filterit forward generic.so greydaemon home home+df \
	hostname iftocc indimail-mta.changes setqload qmonitor \
	indimail-mta.perms inewaliases installer instcheck ipmeprint irmail \
	ldap-checkpwd leapsecs leapsecs.dat maildir2mbox maildircmd \
	maildirdeliver maildirqmtp maildirserial maildirsize maildirsmtp \
	mailsubj make-recipients mbox2maildir mini-smtpd multirotate \
	newinclude new-inject nowutc ofmipd ofmipname parsedate plugtest \
	predate preline printass printforward printmaillist proc proc+df \
	qaes qarf qbase64 qdane qfrontend qf-log-subject qf-smtp-ratelimit \
	qmail-cat qmail-cdb qmail-clean qmailconfig qmail-daned qmail-direct \
	qtop qmaildirmake qmaildirwatch qmail-dkim qmail-getpw qmail-greyd \
	qmail-inject qmail-local qmail-lspawn qmail-multi qmail-newu \
	qmail-nullqueue qmail-popbull qmail-poppass qmail-pw2u qmail-qfilter \
	qmail-qmqpc qmail-qmqpd qmail-qmtpd qmail-qread qmail-queue \
	qmail-remote qmail-rm qmail-rspawn qmail-send qmail-showctl qmail-smtpd \
	qmail_smtpd.so qmail-spamfilter qmail-start qmail-tcpok qmail-tcpto \
	todo-proc qmail-upq qmta-send qnotify qpq qquote qreceipt qsmhook \
	qtools qscheduler queue-fix test-recipients rd-remote relaytest \
	rewind rpmattr rrforward rrt sendmail serialcmd serialqmtp serialsmtp \
	setforward setmaillist slowq-send slowq-start smtpd-plugin.so \
	sortedtest spawn-filter spfquery splogger srsfilter sslerator \
	surblfilter surblqueue svctool swaks sys-checkpwd tcp-env testzero \
	tokenize uacl udpclient udplogger update_tmprsadh viruscanner whois \
	yearcal

man = cdb-database.8 cdbdump.1 cdbget.1 cdbgetm.1 cdbmake.1 cleanq.8 \
	condredirect.1 ctrlenv.8 datemail.1 dknewkey.8 \
	dot-forward.1 dot-qmail.5 drate.1 drate.1 fastforward.1 filterto.1 \
	indimail-env.7 filterit.1 forward.1 maildirserial.1 mlmatchup.8 \
	multi-queue.7 new-inject.1 plugin_init.3 plugtest.1 predate.1 \
	printass.1 qarf.1 qfrontend.1 qmail-cdb.8 qmail-clean.8 \
	indimail-control.5 qmail-daned.8 qtop.1 qmail-direct.8 qmail-dkim.8 \
	qmail-getpw.8 qmail-inject.8 qmail-limits.7 qmail-local.8 qmail-multi.8 \
	qmonitor.8 indimail-mta.7 qmail-newu.8 qmail-poppass.8 qmail-pw2u.8 \
	qmail-qread.8 sslerator.8 qmail-queue-clients.7 qmail-queue.8 \
	qmail-remote.8 qmail-rm.1 qmail-send.8 qmail-showctl.8 qmail-smtpd.8 \
	qmail-spamfilter.8 indimail-srs.5 qmail-start.8 todo-proc.8 \
	qmail-users.5 qmta-send.8 qnotify.1 qreceipt.1 qscanq.8 \
	queue-fix.8 replier.1 make-recipients.1 test-recipients.1 \
	rrforward.1 rrt.1 run-cleanq.8 slowq-send.8 slowq-start.8 \
	spawn-filter.8 surblfilter.8 svctool.8 dnstlsarr.1 indimail-services.7

include Makefile.$(SYSTEM)

default: all

all: $(it) $(man) indimail-mta-release

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

compile: make-compile warn-auto.sh systype conf-cc conf-cc-$(SYSTEM)
	( cat warn-auto.sh; ./make-compile "`cat systype`" ) > \
	compile
	chmod 755 compile

make-load: \
make-load.sh auto-ccld.sh
	cat auto-ccld.sh make-load.sh > make-load
	chmod 755 make-load

load: \
make-load warn-auto.sh systype
	( cat warn-auto.sh; ./make-load "`cat systype`" ) > load
	chmod 755 load

make-makelib: \
make-makelib.sh auto-ccld.sh
	cat auto-ccld.sh make-makelib.sh > make-makelib
	chmod 755 make-makelib

makelib: \
make-makelib warn-auto.sh systype
	( cat warn-auto.sh; ./make-makelib "`cat systype`" ) > \
	makelib
	chmod 755 makelib

auto-gid: load auto-gid.o
	./load auto-gid $(QMAILLIB)

auto-gid.o: compile auto-gid.c
	./compile auto-gid.c

auto-int: load auto-int.o
	./load auto-int $(QMAILLIB)

auto-int.o: compile auto-int.c
	./compile auto-int.c

auto-int8: load auto-int8.o
	./load auto-int8 $(QMAILLIB)

auto-int8.o: compile auto-int8.c
	./compile auto-int8.c

auto-str: load auto-str.o
	./load auto-str $(QMAILLIB)

auto-str.o: compile auto-str.c
	./compile auto-str.c

auto-uid: load auto-uid.o
	./load auto-uid $(QMAILLIB)

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
	echo "#include <sys/types.h>"; \
	echo ""; \
	echo "#define ALIASU    \"`head -1  conf-users | tail -1`\""; \
	echo "#define QMAILD    \"`head -2  conf-users | tail -1`\""; \
	echo "#define QMAILL    \"`head -3  conf-users | tail -1`\""; \
	echo "#define ROOTUSER  \"`head -4  conf-users | tail -1`\""; \
	echo "#define QMAILP    \"`head -5  conf-users | tail -1`\""; \
	echo "#define QMAILQ    \"`head -6  conf-users | tail -1`\""; \
	echo "#define QMAILR    \"`head -7  conf-users | tail -1`\""; \
	echo "#define QMAILS    \"`head -8  conf-users | tail -1`\""; \
	echo "#define INDIUSER  \"`head -9  conf-users | tail -1`\""; \
	echo "#define QSCANDU   \"`head -10 conf-users | tail -1`\""; \
	echo "#define QMAILG    \"`head -1 conf-groups | tail -1`\""; \
	echo "#define NOFILESG  \"`head -2 conf-groups | tail -1`\""; \
	echo "#define INDIGROUP \"`head -3 conf-groups | tail -1`\""; \
	echo "#define QSCANDG   \"`head -4 conf-groups | tail -1`\""; \
	echo "#define QCERTSG   \"`head -5 conf-groups | tail -1`\""; \
	echo ""; \
	echo "extern int auto_uida; /* alias user */" ;\
	echo "extern int auto_uidd; /* qmail daemon user */" ;\
	echo "extern int auto_uidl; /* qmail log user */" ;\
	echo "extern int auto_uido; /* root user */" ;\
	echo "extern int auto_uidp; /* password access user */" ;\
	echo "extern int auto_uidq; /* qmail queue user */" ;\
	echo "extern int auto_uidr; /* qmail remote user */" ;\
	echo "extern int auto_uids; /* qmail send user */" ;\
	echo "extern int auto_uidi; /* indimail user */" ;\
	echo "extern int auto_uidv; /* virus scan user */" ;\
	echo ""; \
	echo "extern int auto_gidq; /* qmail group */" ;\
	echo "extern int auto_gidn; /* nofiles gorup */" ;\
	echo "extern int auto_gidi; /* indimail user */" ;\
	echo "extern int auto_gidv; /* virus scan group */" ;\
	echo "extern int auto_gidc; /* certs access user */" ;\
	echo ""; \
	echo "int             uidinit(int, int);"; \
	echo "char           *get_user(uid_t);"; \
	echo "char           *get_group(gid_t);"; \
	echo ""; \
	echo "#endif"; \
	) > auto_uids.h

auto_uids.c: auto-uid auto-gid conf-users conf-groups
	( ./auto-uid auto_uida `head -1  conf-users` \
	&&./auto-uid auto_uidd `head -2  conf-users | tail -1` \
	&&./auto-uid auto_uidl `head -3  conf-users | tail -1` \
	&&./auto-uid auto_uido `head -4  conf-users | tail -1` \
	&&./auto-uid auto_uidp `head -5  conf-users | tail -1` \
	&&./auto-uid auto_uidq `head -6  conf-users | tail -1` \
	&&./auto-uid auto_uidr `head -7  conf-users | tail -1` \
	&&./auto-uid auto_uids `head -8  conf-users | tail -1` \
	&&./auto-uid auto_uidi `head -9  conf-users | tail -1` \
	&&./auto-uid auto_uidv `head -10 conf-users | tail -1` \
	&&./auto-gid auto_gidq `head -1  conf-groups` \
	&&./auto-gid auto_gidn `head -2  conf-groups | tail -1` \
	&&./auto-gid auto_gidi `head -3  conf-groups | tail -1` \
	&&./auto-gid auto_gidv `head -4  conf-groups | tail -1` \
	&&./auto-gid auto_gidc `head -5  conf-groups | tail -1` \
	) > auto_uids.c.tmp && mv auto_uids.c.tmp auto_uids.c

auto_uids.o: compile auto_uids.c
	./compile auto_uids.c

auto_usera.c: auto-str conf-users
	./auto-str auto_usera `head -1 conf-users` > auto_usera.c

auto_usera.o: compile auto_usera.c
	./compile auto_usera.c

swaks: swaks.pl conf-qmail conf-prefix
	cat swaks.pl \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> swaks
	chmod 755 swaks

mailroute: mailroute.pl conf-prefix
	cat mailroute.pl \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> mailroute

do-spf: do-spf.pl conf-prefix
	cat do-spf.pl \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> do-spf

greydaemon: greydaemon.pl conf-prefix
	cat greydaemon.pl \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> greydaemon

binm1: \
binm1.sh conf-qmail
	cat binm1.sh \
	| sed s}PREFIX}"`head -1 conf-qmail`"}g \
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
	./load bouncesaying $(QMAILLIB)

bouncesaying.o: \
compile bouncesaying.c
	./compile bouncesaying.c

check: \
it man
	./instcheck

chkspawn: load chkspawn.o auto_spawn.o
	./load chkspawn auto_spawn.o $(QMAILLIB)

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
load condredirect.o qmail.o variables.o \
auto_control.o auto_prefix.o set_environment.o
	./load condredirect qmail.o variables.o \
	auto_control.o auto_prefix.o set_environment.o $(QMAILLIB)

condredirect.o: compile condredirect.c qmail.h \
set_environment.h srs.h
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
warn-auto.sh qfrontend.sh conf-libexec conf-sysconfdir
	cat warn-auto.sh qfrontend.sh | $(edit) > qfrontend
	chmod 755 qfrontend

qf-smtp-ratelimit: \
warn-auto.sh qf-smtp-ratelimit.in conf-libexec
	cat warn-auto.sh qf-smtp-ratelimit.in \
	| sed s}HOME}"`head -1 conf-qmail`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> qf-smtp-ratelimit
	chmod 755 qf-smtp-ratelimit

qf-log-subject: \
warn-auto.sh qf-log-subject.in conf-prefix
	cat warn-auto.sh qf-log-subject.in | \
	$(edit) > qf-log-subject
	chmod 755 qf-log-subject

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
compile control.c control.h variables.h auto_control.h
	./compile control.c

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

dns.h: \
compile dns.h1 trynameser.sh
	(sh ./trynameser.sh) > dns.h

dns.lib: \
trylib.c compile load socket.lib dns.o socket_v6any.o \
tlsarralloc.o socket_v4mappedprefix.o ipalloc.o ip.o strsalloc.o \
socket.lib
	( ( ./compile trylib.c && ./load trylib dns.o \
	socket_v6any.o socket_v4mappedprefix.o \
	tlsarralloc.o ipalloc.o ip.o strsalloc.o \
	-lresolv `cat socket.lib` $(QMAILLIB) ) >/dev/null 2>&1 \
	&& echo -lresolv || exit 0 ) > dns.lib
	rm -f trylib.o trylib

dns.o: \
compile dns.c ip.h ipalloc.h strsalloc.h dns.h tlsarralloc.h \
conf-tls conf-spf conf-ip conf-tlsa
	./compile `grep -h -v "^#" conf-tls conf-spf \
		conf-ip conf-tlsa` dns.c

dnscname: \
load dnscname.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o socket_v4mappedprefix.o socket_v6any.o dns.lib \
socket.lib
	./load dnscname dns.o dnsdoe.o ip.o ipalloc.o \
	strsalloc.o tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o `cat dns.lib socket.lib` $(QMAILLIB)

dnscname.o: compile dnscname.c dns.h dnsdoe.h
	./compile dnscname.c

dnsdoe.o: compile dnsdoe.c dns.h dnsdoe.h
	./compile dnsdoe.c

dnsfq: \
load dnsfq.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o socket_v4mappedprefix.o \
socket_v6any.o dns.lib socket.lib
	./load dnsfq dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o `cat dns.lib socket.lib` $(QMAILLIB)

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
	socket_v6any.o `cat dns.lib socket.lib` $(QMAILLIB)

dnsip.o: compile dnsip.c strsalloc.h dns.h dnsdoe.h \
ip.h ipalloc.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` dnsip.c

dnsmxip: \
load dnsmxip.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
tlsarralloc.o socket_v4mappedprefix.o \
socket_v6any.o dns.lib socket.lib
	./load dnsmxip dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o `cat dns.lib socket.lib` $(QMAILLIB)

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
	`cat dns.lib socket.lib` $(QMAILLIB)

dnsptr.o: \
compile dnsptr.c strsalloc.h dns.h dnsdoe.h ip.h conf-spf
	./compile `grep -h -v "^#" conf-spf` dnsptr.c

dnstxt: load dnstxt.o dns_text.o dns.lib socket.lib
	./load dnstxt dns_text.o `cat dns.lib socket.lib` \
	-lcrypto $(QMAILLIB)

dnstxt.o: \
compile dnstxt.c strsalloc.h dns.h dnsdoe.h ip.h
	./compile dnstxt.c

dnstlsarr: \
load dnstlsarr.o dns.o dnsdoe.o ip.o tlsarralloc.o ipalloc.o \
strsalloc.o socket_v4mappedprefix.o socket_v6any.o dossl.o \
auto_control.o variables.o control.o socket_tcp.o starttls.o\
auto_sysconfdir.o socket_tcp6.o timeoutconn.o socket_v6loopback.o \
socket_ip4loopback.o dns.lib socket.lib tls.lib
	./load dnstlsarr dns.o dnsdoe.o tlsarralloc.o ip.o ipalloc.o \
	strsalloc.o socket_v4mappedprefix.o socket_v6any.o starttls.o \
	auto_control.o socket_tcp.o socket_tcp6.o timeoutconn.o dossl.o \
	auto_sysconfdir.o socket_v6loopback.o socket_ip4loopback.o \
	variables.o control.o `cat dns.lib socket.lib tls.lib` $(QMAILLIB)

dnstlsarr.o: \
compile dnstlsarr.c dns.h dnsdoe.h tlsarralloc.h \
hastlsa.h control.h starttls.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-ip conf-tls` dnstlsarr.c

dnstlsarr.1: \
dnstlsarr.9
	cat dnstlsarr.9 \
	| sed s}SYSCONF}"`head -1 conf-sysconfdir`"}g \
	> dnstlsarr.1

dot-qmail.5: \
dot-qmail.9 conf-break
	cat dot-qmail.9 \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	> dot-qmail.5

irmail: \
warn-auto.sh rmail.sh conf-prefix
	cat warn-auto.sh rmail.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> irmail

etrn: \
warn-auto.sh etrn.sh conf-qmail conf-libexec conf-prefix
	cat warn-auto.sh etrn.sh \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> etrn

atrn: \
warn-auto.sh atrn.sh conf-qmail conf-sysconfdir conf-prefix
	cat warn-auto.sh atrn.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> atrn

multirotate: \
warn-auto.sh multirotate.sh conf-prefix conf-servicedir
	cat warn-auto.sh multirotate.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@servicedir\@}`head -1 conf-servicedir`}g \
	> multirotate

qpq: \
warn-auto.sh qpq.sh conf-qmail conf-prefix
	cat warn-auto.sh qpq.sh \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> qpq

indimail-mta.fc: \
indimail-mta.fc.in conf-qmail conf-sysconfdir conf-servicedir
	cat indimail-mta.fc.in \
	| sed s}@servicedir\@}"$(servicedir)"}g \
	| sed s}@qmaildir\@}"`head -1 conf-qmail`"}g \
	| sed s}@sysconfdir\@}"`head -1 conf-sysconfdir`"}g \
	> indimail-mta.fc

indimail-mta.cron: indimail-mta.cron.in conf-prefix \
conf-sysconfdir conf-libexec conf-servicedir
	( \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	$(edit) indimail-mta.cron.in \
	) > $@

except: \
load except.o
	./load except $(QMAILLIB)

except.o: \
compile except.c
	./compile except.c

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

systype: \
find-systype trycpp.c
	./find-systype > systype

fmtqfn.o: compile fmtqfn.c fmtqfn.h
	./compile fmtqfn.c

forward: \
load forward.o qmail.o variables.o auto_prefix.o auto_control.o \
control.o rcpthosts.o set_environment.o srs.o srs.lib
	./load forward qmail.o variables.o auto_prefix.o auto_control.o \
	control.o rcpthosts.o set_environment.o srs.o \
	-L../libsrs2-x/libsrs2/.libs `cat srs.lib` $(QMAILLIB)

forward.1: forward.9 conf-sysconfdir
	cat forward.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> forward.1

forward.o: compile forward.c qmail.h set_environment.h hassrs.h
	./compile forward.c

gfrom.o: \
compile gfrom.c gfrom.h
	./compile gfrom.c

hasnpbg1.h: \
trynpbg1.c compile load
	( ( ./compile trynpbg1.c \
	&& ./load trynpbg1 $(QMAILLIB) && ./trynpbg1 ) \
	>/dev/null 2>&1 \
	&& echo "#define HASNAMEDPIPEBUG1 1" || exit 0 ) > \
	hasnpbg1.h
	rm -f trynpbg1.o trynpbg1

hassalen.h: \
trysalen.c compile
	( ./compile trysalen.c >/dev/null 2>&1 \
	&& echo "#define HASSALEN 1" || exit 0 ) > hassalen.h
	rm -f trysalen.o

hassgact.h: \
trysgact.c compile load
	( ( ./compile trysgact.c && ./load trysgact ) >/dev/null \
	2>&1 \
	&& echo "#define HASSIGACTION 1" || exit 0 ) > hassgact.h
	rm -f trysgact.o trysgact

hassgprm.h: \
trysgprm.c compile load
	( ( ./compile trysgprm.c && ./load trysgprm ) >/dev/null \
	2>&1 \
	&& echo "#define HASSIGPROCMASK 1" || exit 0 ) > hassgprm.h
	rm -f trysgprm.o trysgprm

hasrresvport.h: \
tryrresvport.c compile load
	( ( ./compile tryrresvport.c && ./load tryrresvport ) >/dev/null \
	2>&1 \
	&& echo "#define HASRRESVPORT 1" || exit 0 ) > hasrresvport.h
	rm -f tryrresvport.o tryrresvport

varargs.h: \
choose compile load hasvarargs.h hasstdarg.h
	./choose c trystdarg hasvarargs.h hasstdarg.h > varargs.h

headerbody.o: \
compile headerbody.c hfield.h headerbody.h
	./compile headerbody.c

hfield.o: \
compile hfield.c hfield.h
	./compile hfield.c

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
	./load hostname $(QMAILLIB)

hostname.o: \
compile hostname.c
	./compile hostname.c

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
conf-tls conf-ip conf-ipme
	./compile `grep -h -v "^#" conf-tls conf-ip conf-ipme` ipme.c

ipmeprint: \
load ipmeprint.o ipme.o ip.o ipalloc.o strsalloc.o \
socket_v4mappedprefix.o socket_v6any.o \
auto_control.o variables.o socket.lib
	./load ipmeprint ipme.o ip.o ipalloc.o strsalloc.o \
	auto_control.o socket_v4mappedprefix.o \
	socket_v6any.o variables.o `cat socket.lib` $(QMAILLIB)

ipmeprint.o: compile ipmeprint.c ipme.h ip.h ipalloc.h \
conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` ipmeprint.c

viruscanner: \
qscanq cleanq run-cleanq qscanq-stdin qhpsi

qhpsi: \
load qhpsi.o auto_qmail.o get_uid.o auto_uids.o
	./load qhpsi auto_qmail.o get_uid.o auto_uids.o \
	-ldl $(QMAILLIB)

qhpsi.o: compile qhpsi.c auto_qmail.h auto_uids.h qmail.h
	./compile qhpsi.c

qscanq: load qscanq.o auto_uids.o get_uid.o \
auto_spool.o mkfn.o do_cleanq.o \
auto_cleaner.o auto_qstdin.o fmt_xpid.o
	./load qscanq auto_uids.o get_uid.o \
	fmt_xpid.o mkfn.o auto_spool.o do_cleanq.o \
	auto_cleaner.o auto_qstdin.o $(QMAILLIB)

qscanq.o: compile qscanq.c auto_spool.h auto_fnlen.h mkfn.h \
auto_uids.h auto_retries.h exitcodes.h
	./compile qscanq.c

fmt_xpid.o: compile fmt_xpid.c
	./compile fmt_xpid.c

mkfn.o: compile mkfn.c auto_pidt.h
	./compile mkfn.c

do_cleanq.o: compile do_cleanq.c auto_cleaner.h
	./compile do_cleanq.c

auto_pidt.h: auto-pidt
	./auto-pidt > auto_pidt.h.tmp && mv auto_pidt.h.tmp auto_pidt.h

auto-pidt: \
load auto-pidt.o
	./load auto-pidt $(QMAILLIB)

auto-pidt.o: compile auto-pidt.c
	./compile auto-pidt.c

auto_spool.c: auto-str conf-scandir
	./auto-str auto_spool "`head -1 conf-scandir`"/root/scanq > \
	auto_spool.c.tmp && mv auto_spool.c.tmp auto_spool.c

auto_spool.o: compile auto_spool.c
	./compile auto_spool.c

auto-fnlen: load auto-fnlen.o
	./load auto-fnlen $(QMAILLIB)

auto-fnlen.o: compile auto-fnlen.c auto_pidt.h
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
	./load auto-strarr $(QMAILLIB)

auto-strarr.o: compile auto-strarr.c
	./compile auto-strarr.c

cleanq: load cleanq.o get_uid.o auto_uids.o
	./load cleanq get_uid.o auto_uids.o $(QMAILLIB)

cleanq.o: compile cleanq.c auto_ageout.h auto_uids.h \
hasunlinkat.h
	./compile cleanq.c

hasunlinkat.h: \
tryunlinkat.c compile load
	( ( ./compile tryunlinkat.c \
	&& ./load tryunlinkat ) >/dev/null 2>&1 \
	&& echo "#define HASUNLINKAT 1" || exit 0 ) > hasunlinkat.h
	rm -f tryunlinkat.o tryunlinkat

auto_ageout.h: conf-ageout
	echo "#define MAX_AGE" "`head -1 < conf-ageout`" > auto_ageout.h.tmp && \
	mv auto_ageout.h.tmp auto_ageout.h

run-cleanq: load run-cleanq.o auto_libexec.o
	./load run-cleanq auto_libexec.o $(QMAILLIB)

run-cleanq.o: compile run-cleanq.c auto_libexec.h \
conf-servicedir
	./compile -DSERVICEDIR=\"`head -1 conf-servicedir`\" run-cleanq.c

custom_error.o: compile custom_error.c qmail.h
	./compile custom_error.c

getqueue.o: \
compile getqueue.c getqueue.h haslibrt.h custom_error.h \
qmail.h conf-queue
	./compile `grep -h -v "^#" conf-queue` getqueue.c

qscanq-stdin: load qscanq-stdin.o auto_qmail.o \
auto_prefix.o auto_spool.o mkfn.o do_ripmime.o auto_ripmime_cmd.o \
do_scan.o auto_scancmd.o do_cleanq.o auto_cleaner.o qregex.o \
wildmat.o variables.o control.o auto_control.o \
fmt_xpid.o qmulti.o custom_error.o getqueue.o rt.lib
	./load qscanq-stdin auto_qmail.o \
	auto_prefix.o auto_spool.o mkfn.o do_ripmime.o \
	auto_ripmime_cmd.o do_scan.o auto_scancmd.o do_cleanq.o \
	auto_cleaner.o qregex.o wildmat.o control.o variables.o \
	auto_control.o fmt_xpid.o qmulti.o custom_error.o \
	getqueue.o `cat rt.lib` $(QMAILLIB)

qscanq-stdin.o: compile qscanq-stdin.c auto_ageout.h \
exitcodes.h mkfn.h auto_fnlen.h qmulti.h
	./compile qscanq-stdin.c

do_ripmime.o: compile do_ripmime.c exitcodes.h
	./compile do_ripmime.c

do_scan.o: compile do_scan.c exitcodes.h qregex.h control.h \
variables.h auto_control.h qmail.h
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
	$(edit) qscanq.9 > qscanq.8

cleanq.8: \
cleanq.9 conf-servicedir
	$(edit) cleanq.9 > $@

run-cleanq.8: run-cleanq.9 conf-sysconfdir \
conf-prefix conf-servicedir
	$(edit) run-cleanq.9 > $@

analyzer: \
mlmatchup matchup columnt zoverall zsendmail xqp xsender xrecipient ddist \
deferrals failures successes rhosts recipients rxdelay senders suids \
zddist zdeferrals zfailures zsuccesses zrhosts zrecipients zrxdelay \
zsenders zsuids zsmtp rsmtp rsmtpfailures rsmtpsdomains rsmtprdomains \
rsmtpsenders rsmtprecipients smtp-matchup rspamrdomain \
rspamsdomain zspam rspamstat rspamhist

qtools: \
822body 822headerok 822bodyfilter 822headerfilter 822addr \
822fields checkaddr checkdomain filterto condtomaildir \
iftoccfrom ifaddr replier replier-config

indimail-mta.7: \
indimail-mta.9
	$(edit) indimail-mta.9 > indimail-mta.7

maildir.o: \
compile maildir.c prioq.h maildir.h
	./compile maildir.c

maildir2mbox: \
load maildir2mbox.o maildir.o prioq.o gfrom.o
	./load maildir2mbox maildir.o prioq.o \
	gfrom.o $(QMAILLIB)

maildir2mbox.o: \
compile maildir2mbox.c prioq.h gfrom.h maildir.h
	./compile maildir2mbox.c

mbox2maildir: load mbox2maildir.o
	./load mbox2maildir $(QMAILLIB)

mbox2maildir.o: compile mbox2maildir.c
	./compile mbox2maildir.c

qmaildirmake: \
load qmaildirmake.o
	./load qmaildirmake $(QMAILLIB)

qmaildirmake.o: \
compile qmaildirmake.c
	./compile qmaildirmake.c

qmaildirwatch: \
load qmaildirwatch.o hfield.o headerbody.o maildir.o prioq.o
	./load qmaildirwatch hfield.o headerbody.o maildir.o \
	prioq.o $(QMAILLIB)

qmaildirwatch.o: \
compile qmaildirwatch.c prioq.h hfield.h headerbody.h maildir.h
	./compile qmaildirwatch.c

mailsubj: \
warn-auto.sh mailsubj.sh conf-prefix conf-break conf-split
	cat warn-auto.sh mailsubj.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> mailsubj

doc/INSTALL-MINI: doc/INSTALL-MINI.in \
	conf-prefix conf-libexec conf-qmail conf-sysconfdir conf-shared
	$(edit) $@.in > $@

newfield.o: \
compile newfield.c newfield.h
	./compile newfield.c

predate: load predate.o
	./load predate $(QMAILLIB)

predate.o: compile predate.c
	./compile predate.c

preline: load preline.o
	./load preline $(QMAILLIB)

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

qmail-clean: load qmail-clean.o fmtqfn.o auto_split.o \
variables.o
	./load qmail-clean fmtqfn.o \
	auto_split.o variables.o $(QMAILLIB)

qmail-clean.o: compile qmail-clean.c fmtqfn.h \
auto_split.h variables.h
	./compile qmail-clean.c

qmail-clean.8: qmail-clean.9 conf-split
	cat qmail-clean.9 \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	> qmail-clean.8

indimail-control.5: \
indimail-control.9 conf-sysconfdir
	cat indimail-control.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> indimail-control.5

indimail-srs.5: \
indimail-srs.9 conf-sysconfdir
	cat indimail-srs.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> indimail-srs.5

qmail-getpw: \
load qmail-getpw.o auto_break.o auto_usera.o
	./load qmail-getpw auto_break.o auto_usera.o \
	$(static_option) $(QMAILLIB) $(dynamic_option)

qmail-getpw.8: \
qmail-getpw.9 conf-break
	cat qmail-getpw.9 \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	> qmail-getpw.8

qmail-getpw.o: compile qmail-getpw.c auto_usera.h auto_break.h \
qlx.h
	./compile qmail-getpw.c

qmail-inject: load qmail-inject.o headerbody.o hfield.o \
newfield.o quote.o control.o qmail.o envrules.o wildmat.o \
srs.o rcpthosts.o auto_control.o parse_env.o do_match.o \
variables.o set_environment.o auto_qmail.o auto_prefix.o srs.lib
	./load qmail-inject headerbody.o hfield.o newfield.o \
	quote.o control.o qmail.o envrules.o do_match.o \
	wildmat.o srs.o rcpthosts.o auto_control.o variables.o \
	auto_qmail.o auto_prefix.o parse_env.o \
	set_environment.o $(static_option) $(QMAILLIB) \
	-L../libsrs2-x/libsrs2/.libs `cat srs.lib` $(dynamic_option)

qmail-inject.8: qmail-inject.9 conf-prefix conf-qmail conf-sysconfdir
	cat qmail-inject.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}HOME}"`head -1 conf-qmail`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-inject.8

qmail-inject.o: \
compile qmail-inject.c hfield.h control.h \
qmail.h quote.h headerbody.h newfield.h envrules.h \
hassrs.h set_environment.h conf-srs
	./compile qmail-inject.c

envrules.o: \
compile envrules.c envrules.h control.h \
wildmat.h parse_env.h
	./compile envrules.c

do_match.o: \
compile do_match.c do_match.h wildmat.h
	./compile do_match.c

parse_env.o: \
compile parse_env.c parse_env.h
	./compile parse_env.c

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

syncdir.o: compile syncdir.c conf-sync
	./compile `grep -h -v "^#" conf-sync` syncdir.c

qmail-local: \
load qmail-local.o qmail.o quote.o gfrom.o slurpclose.o srs.o \
auto_patrn.o rcpthosts.o auto_control.o variables.o control.o \
maildir_deliver.o filterit_sub.o auto_prefix.o syncdir.o \
set_environment.o socket.lib srs.lib
	./load qmail-local qmail.o quote.o gfrom.o slurpclose.o srs.o \
	rcpthosts.o auto_patrn.o variables.o auto_control.o control.o \
	maildir_deliver.o filterit_sub.o auto_prefix.o syncdir.o \
	set_environment.o \
	$(static_option) -L../libsrs2-x/libsrs2/.libs `cat srs.lib` \
	$(QMAILLIB) $(dynamic_option) `cat socket.lib` -lm

qmail-local.o: \
compile qmail-local.c quote.h qmail.h slurpclose.h \
gfrom.h auto_patrn.h control.h variables.h hassrs.h \
syncdir.h maildir_deliver.h conf-sync conf-mailarch
	./compile `grep -h -v "^#" conf-sync conf-mailarch` qmail-local.c

qmail-local.8:\
qmail-local.9 conf-sysconfdir
	cat qmail-local.9 \
	| sed s}@qmaildir\@}"`head -1 conf-qmail`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-local.8

maildir_deliver.o: \
compile maildir_deliver.c syncdir.h conf-sync
	./compile `grep -h -v "^#" conf-sync` maildir_deliver.c

qmail-lspawn: \
load qmail-lspawn.o lspawn.o slurpclose.o get_uid.o \
auto_control.o auto_prefix.o \
auto_uids.o auto_spawn.o auto_assign.o control.o \
indimail_stub.o variables.o dns.lib
	./load qmail-lspawn lspawn.o slurpclose.o auto_uids.o \
	get_uid.o auto_control.o auto_prefix.o \
	auto_spawn.o indimail_stub.o control.o variables.o auto_assign.o \
	$(static_option) $(QMAILLIB) $(dynamic_option) `cat dns.lib` -ldl

qmail-lspawn.o: compile qmail-lspawn.c slurpclose.h \
auto_uids.h qlx.h auto_assign.h indimail_stub.h \
auto_control.h auto_prefix.h variables.h conf-virtual
	./compile `grep -h -v "^#" conf-virtual` \
	-DSPAWN=lspawn -DQSPAWN=qmail_lspawn \
	-Dreport_SPAWN=report_lspawn \
	-Dinitialize_SPAWN=initialize_lspawn \
	-Dtruncreport_SPAWN=truncreport_lspawn \
	qmail-lspawn.c

qmail-rspawn: \
load qmail-rspawn.o rspawn.o tcpto_clean.o get_uid.o auto_uids.o \
auto_spawn.o rcpthosts.o control.o auto_prefix.o variables.o \
dns.lib indimail_stub.o
	./load qmail-rspawn rspawn.o tcpto_clean.o get_uid.o auto_spawn.o \
	rcpthosts.o control.o variables.o auto_uids.o \
	indimail_stub.o auto_control.o auto_prefix.o \
	$(static_option) $(QMAILLIB) $(dynamic_option) `cat dns.lib` -ldl

qmail-rspawn.o: compile qmail-rspawn.c tcpto.h rcpthosts.h \
indimail_stub.h ipalloc.h auto_control.h auto_prefix.h \
variables.h conf-tls conf-ip conf-virtual
	./compile `grep -h -v "^#" conf-tls conf-ip conf-virtual` \
	-DSPAWN=rspawn -DQSPAWN=qmail_rspawn \
	-Dreport_SPAWN=report_rspawn \
	-Dinitialize_SPAWN=initialize_rspawn \
	-Dtruncreport_SPAWN=truncreport_rspawn \
	qmail-rspawn.c

qmail-cdb: \
load qmail-cdb.o auto_control.o auto_sysconfdir.o
	./load qmail-cdb auto_control.o auto_sysconfdir.o $(QMAILLIB)

qmail-cdb.o: \
compile qmail-cdb.c variables.h auto_control.h
	./compile qmail-cdb.c

qmail-cdb.8: \
qmail-cdb.9 conf-break conf-spawn conf-sysconfdir
	$(edit) qmail-cdb.9 > qmail-cdb.8

qmail-newu: \
load qmail-newu.o auto_assign.o
	./load qmail-newu auto_assign.o $(QMAILLIB)

qmail-newu.8: \
qmail-newu.9 conf-sysconfdir
	cat qmail-newu.9 \
	| sed s}ASSIGN}"`head -1 conf-sysconfdir`/users"}g \
	> qmail-newu.8

qmail-newu.o: \
compile qmail-newu.c auto_assign.h
	./compile qmail-newu.c

cdb-database: \
load cdb-database.o auto_control.o variables.o
	./load cdb-database auto_control.o variables.o $(QMAILLIB)

cdb-database.o: \
compile cdb-database.c auto_control.h
	./compile cdb-database.c

cdb-database.8: \
cdb-database.9 conf-prefix conf-sysconfdir
	cat cdb-database.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}CONTROL}"`head -1 conf-sysconfdir`/control"}g \
	> cdb-database.8

qmail-popbull: \
load qmail-popbull.o
	./load qmail-popbull $(QMAILLIB)

qmail-popbull.o: \
compile qmail-popbull.c
	./compile qmail-popbull.c

qmail-pw2u: load qmail-pw2u.o control.o auto_usera.o auto_control.o \
auto_break.o auto_assign.o variables.o
	./load qmail-pw2u control.o auto_control.o auto_usera.o \
	auto_break.o auto_assign.o variables.o $(QMAILLIB)

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
timeoutconn.o ip.o control.o \
socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
socket_v4mappedprefix.o socket_tcp.o socket_tcp6.o \
auto_control.o socket.lib
	./load qmail-qmqpc slurpclose.o socket_tcp.o socket_tcp6.o \
	socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
	auto_control.o socket_v4mappedprefix.o timeoutconn.o ip.o \
	control.o variables.o $(static_option) $(QMAILLIB) \
	$(dynamic_option) `cat socket.lib`

qmail-qmqpc.o: \
compile qmail-qmqpc.c auto_control.h slurpclose.h ip.h \
timeoutconn.h control.h variables.h qmail.h socket.h \
haveip6.h buffer_defs.h conf-ip
	./compile `grep -h -v "^#" conf-ip` qmail-qmqpc.c

qmail-qmqpd: \
load qmail-qmqpd.o received.o qmail.o auto_prefix.o
	./load qmail-qmqpd received.o qmail.o auto_prefix.o \
	$(static_option) $(QMAILLIB) $(dynamic_option)

qmail-qmqpd.o: \
compile qmail-qmqpd.c buffer_defs.h qmail.h received.h
	./compile qmail-qmqpd.c

qmail-qmtpd: \
load qmail-qmtpd.o rcpthosts.o control.o received.o \
qmail.o auto_control.o auto_prefix.o variables.o
	./load qmail-qmtpd rcpthosts.o control.o auto_control.o \
	received.o qmail.o variables.o auto_prefix.o \
	$(static_option) $(QMAILLIB) $(dynamic_option)

qmail-qmtpd.o: \
compile qmail-qmtpd.c qmail.h rcpthosts.h control.h received.h \
buffer_defs.h recipients.h
	./compile qmail-qmtpd.c

process_queue.o: \
compile process_queue.c control.h auto_qmail.h set_environment.h \
process_queue.h conf-queue
	./compile `grep -h -v "^#" conf-queue` process_queue.c

qmail-qread: \
load qmail-qread.o fmtqfn.o readsubdir.o auto_qmail.o \
auto_split.o variables.o control.o auto_control.o \
set_environment.o queue_load.o process_queue.o rt.lib
	./load qmail-qread fmtqfn.o readsubdir.o control.o \
	auto_qmail.o auto_split.o variables.o auto_control.o \
	set_environment.o process_queue.o queue_load.o \
	$(QMAILLIB) `cat rt.lib`

qmail-qread.o: \
compile qmail-qread.c auto_qmail.h auto_split.h readsubdir.h \
fmtqfn.h haslibrt.h control.h variables.h process_queue.h \
queue_load.h set_environment.h conf-queue
	./compile `grep -h -v "^#" conf-queue` qmail-qread.c

qmail-qread.8: qmail-qread.9 conf-sysconfdir conf-split
	cat qmail-qread.9 \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-qread.8

queue_load.o: \
compile queue_load.c queue_load.h haslibrt.h
	./compile queue_load.c

qmail-multi: load qmail-multi.o qmulti.o auto_control.o \
variables.o control.o mailfilter.o auto_prefix.o \
auto_qmail.o custom_error.o getqueue.o dns.lib socket.lib rt.lib
	./load qmail-multi qmulti.o auto_control.o auto_qmail.o \
	variables.o control.o mailfilter.o auto_prefix.o \
	custom_error.o getqueue.o $(static_option) $(QMAILLIB) \
	$(dynamic_option) `cat dns.lib socket.lib rt.lib`

qmail-multi.o: \
compile qmail-multi.c qmulti.h qmail.h mailfilter.h \
conf-queue
	./compile `grep -h -v "^#" conf-queue` qmail-multi.c

qmail-multi.8: \
qmail-multi.9 conf-prefix conf-qmail
	$(edit) qmail-multi.9 > qmail-multi.8

qmail-spamfilter: load qmail-spamfilter.o qmulti.o \
auto_prefix.o auto_qmail.o auto_control.o control.o \
custom_error.o getqueue.o rt.lib
	./load qmail-spamfilter qmulti.o auto_control.o \
	auto_prefix.o auto_qmail.o variables.o control.o \
	custom_error.o getqueue.o $(static_option) $(QMAILLIB) \
	$(dynamic_option) `cat rt.lib`

qmail-spamfilter.o: \
compile qmail-spamfilter.c qmulti.h qmail.h
	./compile qmail-spamfilter.c

qmail-spamfilter.8: \
qmail-spamfilter.9 conf-prefix conf-qmail
	cat qmail-spamfilter.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> qmail-spamfilter.8

qmulti.o: \
compile qmulti.c auto_qmail.h control.h qmulti.h qmail.h \
auto_prefix.h qscheduler.h custom_error.h haslibrt.h \
conf-queue
	./compile `grep -h -v "^#" conf-queue` qmulti.c

mailfilter.o: \
compile mailfilter.c mailfilter.h qmulti.h qmail.h
	./compile mailfilter.c

spawn-filter: \
load spawn-filter.o variables.o report.o auto_qmail.o \
auto_prefix.o control.o wildmat.o qregex.o parse_env.o \
auto_control.o envrules.o getDomainToken.o do_match.o \
socket.lib dns.lib
	./load spawn-filter variables.o report.o auto_qmail.o \
	auto_prefix.o control.o wildmat.o qregex.o auto_control.o \
	getDomainToken.o envrules.o parse_env.o do_match.o \
	$(static_option) $(QMAILLIB) $(dynamic_option) \
	`cat dns.lib socket.lib` -lm

spawn-filter.o: \
compile spawn-filter.c control.h auto_qmail.h variables.h qregex.h \
report.h getDomainToken.h auto_prefix.h buffer_defs.h
	./compile spawn-filter.c

spawn-filter.8: \
spawn-filter.9 conf-prefix conf-sysconfdir
	cat spawn-filter.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> spawn-filter.8

do_rate.o: \
compile do_rate.c do_rate.h
	./compile do_rate.c

delivery_rate.o: \
compile delivery_rate.c delivery_rate.h qsutil.h \
variables.h do_rate.h getDomainToken.h control.h \
varargs.h
	./compile delivery_rate.c

report.o: \
compile report.c report.h
	./compile report.c

getDomainToken.o: \
compile getDomainToken.c getDomainToken.h wildmat.h \
parse_env.h
	./compile getDomainToken.c

drate.1: \
drate.9 conf-prefix conf-qmail
	cat drate.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}CONTROL}"`head -1 conf-sysconfdir`/control"}g \
	| sed s}QUEUEDIR}"`head -1 conf-qmail`/queue"}g \
	> drate.1

qmail-direct: load qmail-direct.o pidopen.o custom_error.o \
syncdir.o
	./load qmail-direct pidopen.o custom_error.o syncdir.o $(QMAILLIB)

qmail-direct.o: \
compile qmail-direct.c pidopen.h custom_error.h syncdir.h conf-sync
	./compile `grep -h -v "^#" conf-sync` qmail-direct.c

qmail-direct.8: \
qmail-direct.9 conf-sysconfdir conf-prefix
	cat qmail-direct.9 \
	| sed s}@prefix\@}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-direct.8

qmail-queue: \
load qmail-queue.o triggerpull.o fmtqfn.o get_uid.o auto_split.o \
auto_uids.o variables.o auto_prefix.o auto_qmail.o do_match.o \
control.o auto_control.o strsalloc.o custom_error.o getqueue.o \
syncdir.o wildmat.o socket.lib rt.lib
	./load qmail-queue triggerpull.o fmtqfn.o get_uid.o auto_split.o \
	auto_uids.o auto_prefix.o variables.o auto_qmail.o do_match.o \
	control.o auto_control.o strsalloc.o custom_error.o getqueue.o \
	syncdir.o wildmat.o $(static_option) $(QMAILLIB) $(dynamic_option) \
	`cat socket.lib rt.lib`

qmail-queue.8: \
qmail-queue.9 conf-qmail conf-prefix conf-servicedir conf-split
	$(edit) qmail-queue.9 \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	> qmail-queue.8

qmail-queue-clients.7: \
qmail-queue-clients.9 conf-prefix
	$(edit) qmail-queue-clients.9 \
	> qmail-queue-clients.7

qmail-queue.o: \
compile qmail-queue.c triggerpull.h do_match.h \
control.h auto_uids.h fmtqfn.h variables.h qmail.h \
qscheduler.h syncdir.h haslibrt.h auto_prefix.h \
auto_split.h buffer_defs.h conf-sync conf-qhpsi \
conf-ip conf-mailarch
	./compile `grep -h -v "^#" conf-sync conf-qhpsi conf-ip conf-mailarch` \
	qmail-queue.c

dns_text.o: compile dns_text.c dns.h
	./compile dns_text.c

dknewkey: dknewkey.sh warn-auto.sh conf-sysconfdir
	rm -f dknewkey
	(cat warn-auto.sh; $(edit) dknewkey.sh) > $@
	chmod +x $@

dknewkey.8: dknewkey.9
	$(edit) dknewkey.9 > $@

dkim.lib: \
compile trydkim.c dns_text.o conf-dkim dns.lib
	( ( ./compile -I/usr/local/include/dkim -I/opt/local/include/dkim \
	-I/usr/include/dkim -I../libdkim2-x \
	`grep -h -v "^#" conf-dkim` trydkim.c -o trydkim1.o && \
	c++ -o trydkim1 trydkim1.o dns_text.o \
	`cat dns.lib` $(LDFLAGS) -L/usr/local/lib -L/opt/local/lib \
	-L../libdkim2-x/.libs -ldkim2 -lcrypto -ldkim2 $(QMAILLIB) ) >/dev/null 2>&1 \
	&& echo "-ldkim2" || echo "WARNING!!! Not linked with libdkim2" 1>&2 ) > dkim.lib
	rm -f trydkim1.o trydkim1

hasdkim.h: dns_text.o \
trydkim.c compile dns.lib dkim.lib conf-indi
	( ( ./compile -I/usr/local/include/dkim -I/opt/local/include/dkim \
	-I/usr/include/dkim -I../libdkim2-x \
	`grep -h -v "^#" conf-dkim` trydkim.c -o trydkim2.o && \
	c++ -o trydkim2 trydkim2.o dns_text.o \
	`cat dkim.lib dns.lib` $(LDFLAGS) -L/usr/local/lib -L/opt/local/lib \
	-L../libdkim2-x/.libs -lcrypto $(QMAILLIB) ) >/dev/null 2>&1 \
	&& (echo "#define HASDKIM 1") || echo "WARNING!!! Not compiled with -DHASDKIM" 1>&2 ) > hasdkim.h; \
	rm -f trydkim2.o trydkim2

qmail-dkim: qmail-dkim.o auto_control.o pidopen.o auto_prefix.o \
variables.o control.o dns_text.o wildmat.o getDomainToken.o \
qmulti.o auto_qmail.o custom_error.o getqueue.o parse_env.o \
dns.lib dkim.lib rt.lib conf-ld-$(SYSTEM) load
	./load qmail-dkim auto_control.o qmulti.o auto_prefix.o \
	variables.o control.o pidopen.o dns_text.o custom_error.o \
	-L../libdkim2-x/.libs `cat dkim.lib` wildmat.o parse_env.o \
	getDomainToken.o getqueue.o `cat conf-ld-$(SYSTEM) 2>/dev/null` \
	auto_qmail.o $(static_option) $(QMAILLIB) \
	$(dynamic_option) -lcrypto `cat dns.lib rt.lib`

qmail-dkim.o: \
compile qmail-dkim.c qmail.h control.h hasdkim.h variables.h \
auto_control.h qmulti.h pidopen.h custom_error.h getDomainToken.h \
buffer_defs.h
	./compile -I/usr/include/dkim -I/usr/local/include/dkim \
		-I../libdkim2-x -I/opt/local/include/dkim qmail-dkim.c

qmail-dkim.8: qmail-dkim.9 conf-prefix conf-sysconfdir
	cat qmail-dkim.9 \
	| sed s}@prefix\@}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-dkim.8

pidopen.o: \
compile pidopen.h pidopen.c
	./compile pidopen.c

hastlsa.h: trytlsa.c compile load dns.lib \
dns.o tlsarralloc.o ip.o socket_v4mappedprefix.o \
socket_v6any.o ipalloc.o strsalloc.o conf-tlsa
	( ( ./compile `grep -h -v "^#" conf-tlsa` \
	trytlsa.c && ./load trytlsa dns.o tlsarralloc.o \
	ip.o socket_v4mappedprefix.o socket_v6any.o \
	ipalloc.o strsalloc.o `cat dns.lib` $(QMAILLIB)) >/dev/null 2>&1 \
	&& echo "#define HASTLSA 1" || echo "WARNING||| Not compiled with -DHASTLSA" 1>&2 ) > hastlsa.h
	rm -f trytlsa.o trytlsa

qnotify.1: qnotify.9 conf-qmail conf-sysconfdir
	cat qnotify.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qnotify.1

rrt.1: rrt.9 conf-sysconfdir
	cat rrt.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> rrt.1

qr_digest_md5.o: compile qr_digest_md5.c
	./compile qr_digest_md5.c

hassmtputf8.h: \
tryidn2.c compile load conf-smtputf8
	( ( ./compile `grep -h -v "^#" conf-smtputf8` tryidn2.c \
		&& ./load tryidn2 -lidn2 ) >/dev/null 2>&1 \
	&& echo "#define SMTPUTF8 1" || echo "WARNING!!! Not compiled with -DSMTPUTF8" 1>&2 ) > hassmtputf8.h
	rm -f tryidn2.o tryidn2

idn2.lib: \
tryidn2.c compile load conf-smtputf8
	( ( ./compile `grep -h -v "^#" conf-smtputf8` tryidn2.c -o tryidn21.o \
		&& ./load tryidn21 -lidn2 ) >/dev/null 2>&1 \
	&& echo "-lidn2" || echo "WARNING!!! Not linked with libidn2" 1>&2 ) > idn2.lib
	rm -f tryidn21.o tryidn21

qmail-remote: \
load qmail-remote.o control.o timeoutconn.o tcpto.o dns.o ip.o \
ipalloc.o strsalloc.o ipme.o quote.o variables.o dossl.o \
auto_sysconfdir.o auto_control.o socket_ip4loopback.o \
socket_v6loopback.o socket_v6any.o tlsarralloc.o parse_env.o \
qr_digest_md5.o fn_handler.o query_skt.o tlsacheck.o \
dns.lib socket.lib socket_v4mappedprefix.o cdb_match.o \
socket_tcp.o socket_tcp6.o idn2.lib gsasl.lib
	./load qmail-remote control.o tlsacheck.o auto_control.o \
	timeoutconn.o tcpto.o dns.o ip.o socket_ip4loopback.o dossl.o \
	auto_sysconfdir.o socket_v6loopback.o socket_v6any.o ipalloc.o \
	socket_v4mappedprefix.o strsalloc.o ipme.o quote.o fn_handler.o \
	qr_digest_md5.o query_skt.o tlsarralloc.o variables.o cdb_match.o \
	socket_tcp.o socket_tcp6.o parse_env.o $(static_option) $(QMAILLIB) \
	$(dynamic_option) `cat dns.lib socket.lib idn2.lib gsasl.lib` \
	-lssl -lcrypto

qmail-remote.8: \
qmail-remote.9 conf-qmail conf-sysconfdir conf-libexec
	cat qmail-remote.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONF}"`head -1 conf-sysconfdir`"}g \
	> qmail-remote.8

qmail-remote.o: \
compile qmail-remote.c control.h dns.h quote.h ip.h auto_sysconfdir.h \
ipalloc.h query_skt.h ipme.h tcpto.h timeoutconn.h haveip6.h socket.h \
variables.h auto_control.h cdb_match.h haslibgsasl.h dossl.h parse_env.h \
hastlsa.h tlsacheck.h fn_handler.h tlsarralloc.h hassmtputf8.h batv.h \
buffer_defs.h varargs.h conf-tls conf-ip conf-batv conf-mxps conf-smtputf8
	./compile `grep -h -v "^#" conf-tls conf-ip conf-mxps \
		conf-batv` qmail-remote.c

dossl.o: \
compile dossl.c variables.h control.h auto_sysconfdir.h \
varargs.h dossl.h hastlsa.h auto_control.h conf-tls
	./compile `grep -h -v "^#" conf-tls` dossl.c

qmail-send: \
load qmail-send.o qsutil.o control.o newfield.o prioq.o parse_env.o \
trigger.o fmtqfn.o quote.o readsubdir.o qmail.o do_match.o \
auto_qmail.o srs.o rcpthosts.o auto_control.o auto_split.o \
variables.o getDomainToken.o do_rate.o envrules.o qregex.o wildmat.o \
auto_prefix.o delivery_rate.o syncdir.o srs.lib rt.lib
	./load qmail-send qsutil.o control.o newfield.o \
	prioq.o trigger.o fmtqfn.o quote.o readsubdir.o \
	getDomainToken.o do_rate.o wildmat.o qmail.o auto_qmail.o \
	auto_split.o variables.o envrules.o srs.o do_match.o \
	delivery_rate.o rcpthosts.o auto_control.o auto_prefix.o \
	syncdir.o parse_env.o $(static_option) \
	-L../libsrs2-x/libsrs2/.libs `cat srs.lib` $(QMAILLIB) \
	$(dynamic_option) -lm `cat rt.lib` -ldl

qmail-send.o: \
compile qmail-send.c control.h auto_qmail.h trigger.h newfield.h \
quote.h qmail.h qsutil.h prioq.h envrules.h syncdir.h delivery_rate.h \
fmtqfn.h readsubdir.h variables.h auto_control.h auto_split.h hassrs.h \
varargs.h haslibrt.h conf-mime conf-sync conf-batv conf-srs \
conf-bounce conf-loglock
	./compile `grep -h -v "^#" conf-mime conf-sync conf-batv \
	conf-bounce conf-loglock` qmail-send.c

qmail-send.8: \
qmail-send.9 conf-break conf-spawn conf-qmail conf-sysconfdir conf-prefix
	cat qmail-send.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-send.8

todo-proc: \
load todo-proc.o control.o trigger.o fmtqfn.o readsubdir.o auto_qmail.o \
auto_split.o auto_control.o variables.o syncdir.o rt.lib
	./load todo-proc control.o trigger.o fmtqfn.o auto_control.o \
	readsubdir.o auto_qmail.o auto_split.o variables.o \
	syncdir.o $(static_option) $(QMAILLIB) \
	$(dynamic_option) `cat rt.lib`

todo-proc.o: \
compile todo-proc.c auto_qmail.h control.h syncdir.h varargs.h \
fmtqfn.h readsubdir.h trigger.h variables.h qscheduler.h \
haslibrt.h auto_control.h auto_split.h auto_uids.h conf-sync
	./compile `grep -h -v "^#" conf-sync` todo-proc.c

todo-proc.8: \
todo-proc.9 conf-qmail conf-sysconfdir
	cat todo-proc.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> todo-proc.8

set_queuedir.o: \
compile set_queuedir.c set_queuedir.h auto_qmail.h control.h
	./compile set_queuedir.c

slowq-send: \
load slowq-send.o qsutil.o control.o newfield.o prioq.o \
trigger.o fmtqfn.o quote.o readsubdir.o qmail.o do_match.o \
auto_qmail.o srs.o rcpthosts.o auto_control.o auto_split.o \
variables.o envrules.o parse_env.o qregex.o wildmat.o \
getDomainToken.o do_rate.o delivery_rate.o auto_prefix.o \
syncdir.o srs.lib
	./load slowq-send qsutil.o control.o newfield.o \
	prioq.o trigger.o fmtqfn.o quote.o readsubdir.o \
	do_rate.o wildmat.o qmail.o auto_qmail.o auto_split.o \
	variables.o envrules.o parse_env.o srs.o do_match.o \
	rcpthosts.o delivery_rate.o auto_control.o getDomainToken.o \
	auto_prefix.o syncdir.o \
	$(static_option) -L../libsrs2-x/libsrs2/.libs \
	`cat srs.lib` $(QMAILLIB) $(dynamic_option) -lm -ldl

slowq-send.o: \
compile slowq-send.c control.h auto_qmail.h trigger.h newfield.h \
quote.h qmail.h qsutil.h prioq.h envrules.h syncdir.h delivery_rate.h \
fmtqfn.h readsubdir.h variables.h auto_control.h auto_split.h hassrs.h \
varargs.h conf-mime conf-sync conf-batv conf-loglock \
conf-bounce conf-srs
	./compile `grep -h -v "^#" conf-mime conf-sync conf-batv \
	conf-loglock conf-bounce` slowq-send.c

slowq-send.8: \
slowq-send.9 conf-break conf-spawn conf-qmail conf-sysconfdir conf-prefix
	cat slowq-send.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> slowq-send.8

qmta-send: \
load qmta-send.o trigger.o fmtqfn.o readsubdir.o \
variables.o auto_control.o control.o qsutil.o auto_qmail.o \
auto_split.o auto_uids.o prioq.o newfield.o quote.o \
qmail.o syncdir.o srs.o rcpthosts.o auto_spawn.o \
slurpclose.o get_uid.o qmta_lspawn.o qmta_rspawn.o \
qmta-lspawn.o qmta-rspawn.o auto_assign.o tcpto_clean.o \
set_environment.o set_queuedir.o auto_prefix.o
	./load qmta-send trigger.o fmtqfn.o readsubdir.o \
	variables.o auto_control.o control.o qsutil.o \
	rcpthosts.o auto_split.o auto_uids.o prioq.o auto_qmail.o \
	newfield.o quote.o qmail.o syncdir.o srs.o qmta_lspawn.o \
	qmta_rspawn.o qmta-lspawn.o qmta-rspawn.o slurpclose.o \
	get_uid.o auto_spawn.o auto_assign.o tcpto_clean.o \
	set_environment.o set_queuedir.o auto_prefix.o \
	$(static_option) -L../libsrs2-x/libsrs2/.libs `cat srs.lib` \
	$(QMAILLIB) $(dynamic_option)

qmta-lspawn.o: compile qmail-lspawn.c slurpclose.h \
auto_uids.h qlx.h auto_assign.h indimail_stub.h \
auto_control.h variables.h
	./compile \
	-DSPAWN=lspawn -DQSPAWN=qmail_lspawn \
	-Dreport_SPAWN=report_lspawn \
	-Dinitialize_SPAWN=initialize_lspawn \
	-Dtruncreport_SPAWN=truncreport_lspawn \
	qmail-lspawn.c -o qmta-lspawn.o

qmta-rspawn.o: compile qmail-rspawn.c \
tcpto.h rcpthosts.h indimail_stub.h \
ipalloc.h auto_control.h variables.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` \
	-DSPAWN=rspawn -DQSPAWN=qmail_rspawn \
	-Dreport_SPAWN=report_rspawn \
	-Dinitialize_SPAWN=initialize_rspawn \
	-Dtruncreport_SPAWN=truncreport_rspawn \
	qmail-rspawn.c -o qmta-rspawn.o

qmta-send.o: \
compile qmta-send.c qsutil.h fmtqfn.h readsubdir.h trigger.h \
variables.h prioq.h qmail.h newfield.h quote.h set_environment.h \
auto_split.h auto_uids.h auto_prefix.h set_queuedir.h varargs.h \
hassrs.h conf-mime conf-sync conf-batv conf-srs conf-loglock \
conf-bounce
	./compile `grep -h -v "^#" conf-mime conf-sync conf-batv \
	conf-loglock conf-bounce` qmta-send.c

qmta-send.8: \
qmta-send.9 conf-break conf-spawn conf-qmail conf-sysconfdir \
conf-prefix conf-split
	cat qmta-send.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmta-send.8

qscheduler: \
load qscheduler.o control.o auto_qmail.o auto_control.o variables.o \
readsubdir.o get_uid.o auto_uids.o auto_split.o rt.lib
	./load qscheduler control.o auto_qmail.o auto_split.o get_uid.o \
	readsubdir.o variables.o auto_uids.o auto_control.o $(QMAILLIB) `cat rt.lib`

qscheduler.o: \
compile qscheduler.c qscheduler.h control.h auto_qmail.h readsubdir.h \
auto_split.h auto_uids.h haslibrt.h conf-queue
	./compile `grep -h -v "^#" conf-queue` qscheduler.c

setqload: \
load setqload.o send_qload.o rt.lib
	./load setqload send_qload.o $(QMAILLIB) `cat rt.lib`

setqload.o: \
compile setqload.c qscheduler.h haslibrt.h
	./compile setqload.c

send_qload.o: \
compile send_qload.c haslibrt.h qscheduler.h send_qload.h
	./compile send_qload.c

qmonitor: \
load qmonitor.o send_qload.o queue_load.o set_environment.o \
variables.o auto_control.o rt.lib
	./load qmonitor send_qload.o queue_load.o set_environment.o \
	variables.o auto_control.o $(QMAILLIB) `cat rt.lib`

qmonitor.o: \
compile qmonitor.c haslibrt.h queue_load.h send_qload.h \
set_environment.h conf-queue
	./compile `grep -h -v "^#" conf-queue` qmonitor.c

qmonitor.8: qmonitor.9 conf-qmail
	cat qmonitor.9 \
	| sed s}@tload\@}"`head -1 conf-queue|sed 's{.*QUEUE_LOAD{QUEUE_LOAD{g'|cut -d= -f2|awk '{print $$1}'|awk '{print $$1 * 2}'`"}g \
	| sed s}@qload\@}"`head -1 conf-queue|sed 's{.*QUEUE_LOAD{QUEUE_LOAD{g'|cut -d= -f2|awk '{print $$1}'`"}g \
	| sed s}@qmax\@}"`head -1 conf-queue|sed 's{.*QUEUE_MAX{QUEUE_MAX{g'|cut -d= -f2|awk '{print $$1}'`"}g \
	> qmonitor.8

multi-queue.7: \
multi-queue.9 conf-sysconfdir conf-qmail
	cat multi-queue.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> multi-queue.7

get_uid.o: \
compile get_uid.c auto_uids.h
	./compile get_uid.c

qmail-showctl: \
load qmail-showctl.o auto_uids.o control.o auto_qmail.o auto_break.o \
indimail_stub.o auto_patrn.o auto_spawn.o auto_split.o variables.o \
get_uid.o auto_control.o auto_sysconfdir.o auto_assign.o set_environment.o \
auto_prefix.o
	./load qmail-showctl auto_uids.o control.o auto_qmail.o \
	auto_break.o auto_patrn.o auto_spawn.o auto_control.o \
	auto_split.o variables.o get_uid.o indimail_stub.o \
	auto_sysconfdir.o auto_assign.o set_environment.o \
	auto_prefix.o -ldl $(QMAILLIB)

qmail-showctl.o: compile qmail-showctl.c control.h auto_sysconfdir.h \
auto_uids.h auto_qmail.h auto_break.h auto_patrn.h auto_spawn.h \
auto_split.h spf.h variables.h auto_control.h indimail_stub.h \
hassrs.h auto_prefix.h conf-srs conf-spf conf-batv conf-dkim conf-tls
	./compile `grep -h -v "^#" conf-spf conf-batv conf-dkim conf-tls` \
	qmail-showctl.c

qmail-showctl.8: qmail-showctl.9 conf-split conf-sysconfdir conf-qmail
	cat qmail-showctl.9 \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> qmail-showctl.8

plugtest: \
load plugtest.o auto_qmail.o auto_prefix.o
	./load plugtest auto_qmail.o auto_prefix.o \
	-ldl $(QMAILLIB)

plugtest.o: \
compile plugtest.c auto_qmail.h auto_prefix.h smtp_plugin.h
	./compile plugtest.c

# $(LD) -bundle -undefined dynamic_lookup -o qmail_smtpd.so \

valid_hname.o: \
compile valid_hname.c
	./compile valid_hname.c

qmail_smtpd.so: \
smtpd.o rcpthosts.o recipients.o indimail_stub.o ip.o ipme.o \
ipalloc.o strsalloc.o control.o etrn.o atrn.o received.o \
wildmat.o qmail.o variables.o envrules.o auto_qmail.o \
bodycheck.o sqlmatch.o qcount_dir.o mail_acl.o do_match.o \
socket_ip4loopback.o socket_v6loopback.o socket_v4mappedprefix.o \
socket_v6any.o spf.o dns.o auto_uids.o qregex_sql.o srs.o \
tablematch.o greylist.o query_skt.o fn_handler.o load_mysql.o \
auto_break.o auto_control.o auto_assign.o auto_prefix.o \
auto_libexec.o parse_env.o auto_sysconfdir.o valid_hname.o \
read_assign.o socket.lib dns.lib tls.lib gsasl.lib mysql_config
	if test $(SYSTEM) = DARWIN ; then \
		$(CC) -dynamiclib -o qmail_smtpd.so smtpd.o rcpthosts.o \
		valid_hname.o recipients.o indimail_stub.o load_mysql.o \
		qcount_dir.o ip.o ipme.o ipalloc.o strsalloc.o control.o \
		etrn.o atrn.o received.o wildmat.o qmail.o auto_qmail.o \
		variables.o envrules.o bodycheck.o spf.o dns.o srs.o \
		sqlmatch.o socket_v4mappedprefix.o parse_env.o do_match.o \
		socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
		mail_acl.o auto_uids.o qregex_sql.o tablematch.o greylist.o \
		auto_break.o auto_control.o auto_assign.o query_skt.o fn_handler.o \
		tlsarralloc.o auto_prefix.o auto_libexec.o auto_sysconfdir.o \
		read_assign.o $(static_option) $(QMAILLIB) $(dynamic_option) \
		-L../libsrs2-x/libsrs2/.libs \
		`cat gsasl.lib socket.lib dns.lib tls.lib srs.lib`; \
	else \
		$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 -o qmail_smtpd.so \
		smtpd.o rcpthosts.o recipients.o auto_control.o auto_assign.o \
		qcount_dir.o ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o \
		received.o wildmat.o qmail.o variables.o envrules.o parse_env.o srs.o \
		bodycheck.o spf.o dns.o sqlmatch.o auto_qmail.o do_match.o valid_hname.o \
		socket_v4mappedprefix.o socket_ip4loopback.o socket_v6loopback.o \
		socket_v6any.o mail_acl.o auto_uids.o qregex_sql.o tablematch.o \
		greylist.o auto_break.o indimail_stub.o load_mysql.o query_skt.o \
		fn_handler.o tlsarralloc.o auto_prefix.o auto_libexec.o auto_sysconfdir.o \
		read_assign.o -Wl,-Bstatic $(QMAILLIB) -Wl,-Bdynamic \
		-L../libsrs2-x/libsrs2/.libs \
		`cat gsasl.lib socket.lib dns.lib tls.lib srs.lib` -ldl; \
	fi

qmail-smtpd: \
load qmail-smtpd.o smtpd.o rcpthosts.o recipients.o parse_env.o \
ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o \
received.o wildmat.o qmail.o variables.o auto_control.o \
envrules.o bodycheck.o sqlmatch.o indimail_stub.o do_match.o \
socket_ip4loopback.o socket_v6loopback.o socket_v4mappedprefix.o \
socket_v6any.o mail_acl.o spf.o dns.o auto_uids.o auto_qmail.o \
qregex_sql.o tablematch.o greylist.o auto_assign.o qcount_dir.o \
query_skt.o fn_handler.o tlsarralloc.o load_mysql.o srs.o \
auto_sysconfdir.o auto_break.o auto_prefix.o auto_libexec.o \
read_assign.o valid_hname.o socket.lib dns.lib tls.lib gsasl.lib srs.lib \
mysql_config conf-sql
	./load qmail-smtpd smtpd.o rcpthosts.o recipients.o srs.o \
	ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o \
	qcount_dir.o received.o wildmat.o qmail.o variables.o \
	envrules.o bodycheck.o spf.o dns.o sqlmatch.o auto_qmail.o \
	socket_v4mappedprefix.o socket_ip4loopback.o do_match.o \
	socket_v6loopback.o socket_v6any.o mail_acl.o auto_uids.o \
	indimail_stub.o qregex_sql.o tablematch.o greylist.o auto_break.o \
	auto_control.o auto_assign.o query_skt.o fn_handler.o valid_hname.o \
	tlsarralloc.o load_mysql.o auto_sysconfdir.o auto_prefix.o \
	read_assign.o auto_libexec.o parse_env.o \
	$(static_option) $(QMAILLIB) -L../libsrs2-x/libsrs2/.libs \
	`cat srs.lib` \
	$(dynamic_option) `cat socket.lib dns.lib gsasl.lib tls.lib` -ldl

qmail-smtpd.o: \
compile qmail-smtpd.c variables.h hassmtputf8.h hasmysql.h \
haslibgsasl.h auto_control.h auto_sysconfdir.h conf-tls conf-spf \
conf-ip conf-batv conf-srs conf-plugin mysql_inc
	./compile `grep -h -v "^#" conf-tls conf-spf \
	conf-ip conf-batv conf-plugin mysql_inc` qmail-smtpd.c

smtpd.o: \
compile smtpd.c control.h received.h varargs.h \
ipme.h ip.h ipalloc.h qmail.h recipients.h greylist.h \
variables.h rcpthosts.h etrn.h batv.h do_match.h \
envrules.h tablematch.h qregex.h sqlmatch.h buffer_defs.h \
smtp_plugin.h auto_prefix.h hasmysql.h indimail_stub.h \
auto_control.h dns.h hassmtputf8.h haslibgsasl.h \
hassrs.h valid_hname.h conf-tls conf-spf conf-ip conf-plugin \
conf-batv conf-smtputf8 mysql_config mysql_inc
	./compile `grep -h -v "^#" conf-tls conf-spf \
	conf-ip conf-batv conf-plugin mysql_inc` smtpd.c

qmail-smtpd.8: \
qmail-smtpd.9 conf-qmail conf-sysconfdir conf-prefix
	$(edit) qmail-smtpd.9 > qmail-smtpd.8

mini-smtpd: \
load mini-smtpd.o ipme.o socket_v6any.o socket_v4mappedprefix.o \
ipalloc.o ip.o variables.o rcpthosts.o control.o qmail.o \
received.o auto_control.o auto_qmail.o auto_prefix.o
	./load mini-smtpd ipme.o socket_v6any.o socket_v4mappedprefix.o \
	ipalloc.o ip.o variables.o rcpthosts.o control.o qmail.o \
	received.o auto_qmail.o auto_control.o auto_prefix.o \
	$(static_option) $(QMAILLIB) $(dynamic_option)

mini-smtpd.o: \
compile mini-smtpd.c auto_qmail.h control.h received.h ipme.h ip.h \
qmail.h rcpthosts.h buffer_defs.h
	./compile mini-smtpd.c

mail_acl.o: compile mail_acl.c wildmat.h varargs.h
	./compile mail_acl.c

indimail_stub.o: \
compile indimail_stub.c control.h \
variables.h indimail_stub.h
	./compile indimail_stub.c

qregex.o: \
compile qregex.c qregex.h control.h \
variables.h auto_control.h wildmat.h
	./compile qregex.c

mysql_inc: mysql_config
	./mysql_config --include > mysql_inc || true

# qregex.c compiled with sqlmatch()
qregex_sql.o: \
compile qregex.c qregex.h sqlmatch.h \
control.h variables.h hasmysql.h mysql_config \
mysql_inc conf-sql
	./compile `grep -h -v "^#" conf-sql mysql_inc` \
	qregex.c -o qregex_sql.o

load_mysql.o: compile load_mysql.c indimail_stub.h \
mysql_config conf-sql auto_control.h variables.h \
hasmysql.h mysql_inc
	./compile `grep -h -v "^#" conf-sql mysql_inc` load_mysql.c

qmail-nullqueue: load qmail-nullqueue.o
	./load qmail-nullqueue $(QMAILLIB)

qmail-nullqueue.o: \
compile qmail-nullqueue.c
	./compile qmail-nullqueue.c

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
load qmail-start.o auto_uids.o get_uid.o
	./load qmail-start auto_uids.o get_uid.o $(QMAILLIB)

qmail-start.8: \
qmail-start.9 conf-qmail
	cat qmail-start.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	| sed s}@tload\@}"`head -1 conf-queue|sed 's{.*QUEUE_LOAD{QUEUE_LOAD{g'|cut -d= -f2|awk '{print $$1}'|awk '{print $$1 * 2}'`"}g \
	| sed s}@qload\@}"`head -1 conf-queue|sed 's{.*QUEUE_LOAD{QUEUE_LOAD{g'|cut -d= -f2|awk '{print $$1}'`"}g \
	| sed s}@qmax\@}"`head -1 conf-queue|sed 's{.*QUEUE_MAX{QUEUE_MAX{g'|cut -d= -f2|awk '{print $$1}'`"}g \
	> qmail-start.8

qmail-start.o: \
compile qmail-start.c auto_uids.h haslibrt.h
	./compile qmail-start.c

slowq-start: \
load slowq-start.o auto_uids.o get_uid.o set_queuedir.o \
auto_qmail.o control.o auto_control.o variables.o
	./load slowq-start auto_uids.o get_uid.o set_queuedir.o \
	auto_qmail.o control.o auto_control.o variables.o $(QMAILLIB)

slowq-start.8: \
slowq-start.9 conf-qmail conf-servicedir
	$(edit) slowq-start.9 > $@

slowq-start.o: \
compile slowq-start.c auto_uids.h set_queuedir.h
	./compile slowq-start.c

qmail-tcpok: \
load qmail-tcpok.o auto_qmail.o variables.o control.o auto_control.o \
set_environment.o process_queue.o
	./load qmail-tcpok auto_qmail.o variables.o control.o \
	set_environment.o auto_control.o process_queue.o $(QMAILLIB)

qmail-tcpok.o: \
compile qmail-tcpok.c auto_qmail.h variables.h control.h tcpto.h \
process_queue.h conf-queue
	./compile `grep -h -v "^#" conf-queue` qmail-tcpok.c

qmail-tcpto: \
load qmail-tcpto.o ip.o auto_qmail.o variables.o control.o auto_control.o \
socket_v4mappedprefix.o socket_v6any.o set_environment.o process_queue.o
	./load qmail-tcpto ip.o auto_qmail.o variables.o control.o \
	auto_control.o socket_v4mappedprefix.o socket_v6any.o \
	set_environment.o process_queue.o $(QMAILLIB)

qmail-tcpto.o: \
compile qmail-tcpto.c auto_qmail.h ip.h variables.h control.h tcpto.h \
process_queue.h conf-tls conf-ip conf-queue
	./compile `grep -h -v "^#" conf-queue` qmail-tcpto.c

qmail-upq: \
warn-auto.sh qmail-upq.sh conf-qmail conf-split
	cat warn-auto.sh qmail-upq.sh \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qmail-upq

qmail-users.5: \
qmail-users.9 conf-qmail conf-sysconfdir
	cat qmail-users.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}@assign@}"`head -1 conf-sysconfdir`/users"}g \
	> qmail-users.5

qmail.o: compile qmail.c qmail.h auto_prefix.h buffer_defs.h
	./compile qmail.c

qreceipt: \
load qreceipt.o headerbody.o hfield.o quote.o qmail.o \
variables.o auto_prefix.o auto_control.o \
set_environment.o
	./load qreceipt headerbody.o hfield.o quote.o \
	variables.o auto_control.o \
	auto_prefix.o qmail.o set_environment.o $(QMAILLIB)

qreceipt.1: qreceipt.9 conf-sysconfdir
	cat qreceipt.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qreceipt.1

qreceipt.o: \
compile qreceipt.c hfield.h \
headerbody.h quote.h qmail.h set_environment.h
	./compile qreceipt.c

qsmhook: \
load qsmhook.o
	./load qsmhook $(QMAILLIB)

qsmhook.o: \
compile qsmhook.c
	./compile qsmhook.c

qsutil.o: \
compile qsutil.c qsutil.h variables.h conf-loglock \
varargs.h auto_control.h
	./compile `grep -h -v "^#" conf-loglock` qsutil.c

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
compile readsubdir.c readsubdir.h auto_split.h
	./compile readsubdir.c

received.o: \
compile received.c qmail.h received.h
	./compile received.c

remoteinfo.o: \
compile remoteinfo.c ip.h timeoutconn.h \
haveip6.h socket.h remoteinfo.h conf-ip
	./compile `grep -h -v "^#" conf-ip` remoteinfo.c

rrforward: \
load rrforward.o qmail.o auto_prefix.o \
auto_control.o variables.o set_environment.o
	./load rrforward qmail.o auto_prefix.o \
	auto_control.o variables.o set_environment.o $(QMAILLIB)

rrforward.1: rrforward.9 conf-sysconfdir
	cat rrforward.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> rrforward.1

rrforward.o: \
compile rrforward.c qmail.h set_environment.h
	./compile rrforward.c

sendmail: \
load sendmail.o auto_prefix.o
	./load sendmail auto_prefix.o $(QMAILLIB)

sendmail.o: \
compile sendmail.c auto_prefix.h
	./compile sendmail.c

install-strip: install

distclean: clean
	/bin/rm -f Makefile. Makevars.

instcheck: \
warn-auto.sh instcheck.in conf-libexec conf-sysconfdir
	cat warn-auto.sh instcheck.in | $(edit) > $@
	chmod +x $@

installer: \
load installer.o
	./load installer $(QMAILLIB)

installer.o: \
compile installer.c
	./compile installer.c

INPUT: INPUT.in \
conf-prefix conf-libexec conf-qmail \
conf-sysconfdir conf-shared
	$(edit) INPUT.in > $@

DIRS: DIRS.in \
conf-prefix conf-libexec conf-qmail \
conf-sysconfdir conf-shared
	([ -f DIRS.$(SYSTEM) ] && \
	$(edit) DIRS.in DIRS.$(SYSTEM) || $(edit) DIRS.in) > DIRS

BIN: BIN.in
	([ -f BIN.$(SYSTEM) ] && \
	$(edit) BIN.in BIN.$(SYSTEM) || $(edit) BIN.in) > BIN

SBIN: SBIN.in SBIN.sql
	([ -f SBIN.$(SYSTEM) ] && \
	$(edit) SBIN.in SBIN.$(SYSTEM) || $(edit) SBIN.in; \
	[ -n "$(USE_SQL)" ] && $(edit) SBIN.sql) > SBIN

SHARED: SHARED.in doc/INSTALL-MINI
	([ -f SHARED.$(SYSTEM) ] && \
	$(edit) SHARED.in SHARED.$(SYSTEM) || $(edit) SHARED.in) > SHARED

MAN: MAN.in MAN.sql
	([ -f MAN.$(SYSTEM) ] && \
	$(edit) MAN.in MAN.$(SYSTEM) || $(edit) MAN.in; \
	[ -n "$(USE_SQL)" ] && cat MAN.sql || true) > MAN


install: \
$(it) $(man) installer DIRS BIN SBIN SHARED MAN INPUT
	./installer $(DESTDIR)                         < DIRS
	./installer $(DESTDIR)$(prefix)                < BIN
	./installer $(DESTDIR)$(prefix)                < SBIN
	./installer $(DESTDIR)$(shareddir)             < SHARED
	./installer $(DESTDIR)$(qsysconfdir)           < ETC
	./installer $(DESTDIR)$(prefix)"/lib/indimail" < PLUGINS
	./installer $(DESTDIR)$(libexecdir)            < LIBEXEC
	./installer $(DESTDIR)$(mandir)                < MAN
	[ -z $(DESTDIR) ] && ./instcheck indimail-mta || true

uninstall: installer DIRS BIN SBIN SHARED MAN
	# reverse the order with which we did install (LIFO). i.e.
	# MAN, LIBEXEC, PLUGINS, ETC, SHARED, SBIN, BIN, DIRS
	# reverse order of input files using sed so that
	# we don't leave non-empty directories
	sed -n '1!G;h;$$p' MAN     | ./installer -u $(DESTDIR)$(mandir)
	sed -n '1!G;h;$$p' LIBEXEC | ./installer -u $(DESTDIR)$(libexecdir)
	sed -n '1!G;h;$$p' PLUGINS | ./installer -u $(DESTDIR)$(prefix)"/lib/indimail"
	sed -n '1!G;h;$$p' ETC     | ./installer -u $(DESTDIR)$(qsysconfdir)
	sed -n '1!G;h;$$p' SHARED  | ./installer -u $(DESTDIR)$(shareddir)
	sed -n '1!G;h;$$p' SBIN    | ./installer -u $(DESTDIR)$(prefix)
	sed -n '1!G;h;$$p' BIN     | ./installer -u $(DESTDIR)$(prefix)
	sed -n '1!G;h;$$p' DIRS    | ./installer -u $(DESTDIR)

slurpclose.o: compile slurpclose.c slurpclose.h
	./compile slurpclose.c

socket.lib: \
trylib.c compile load
	( ( ./compile trylib.c -o trysocket.o && ./load trysocket -lsocket -lnsl ) >/dev/null 2>&1 \
	&& echo -lsocket -lnsl || echo "WARNING!!! Not linked with libsocket" 1>&2 ) > socket.lib
	rm -f trysocket.o trysocket

fts.lib: \
trylib.c compile load
	( ( ./compile trylib.c -o tryfts.o && ./load tryfts -lfts ) >/dev/null 2>&1 \
	&& echo -lfts || echo "WARNING!!! Not linked with libfts" 1>&2 ) > fts.lib
	rm -f trytfs.o trytfs

rt.lib: \
trylib.c compile load
	( ( ./compile trylib.c -o tryrt1.o && ./load tryrt1 -lrt ) >/dev/null 2>&1 \
	&& echo -lrt || echo "WARNING!!! Not linked with librt" 1>&2 ) > rt.lib
	rm -f tryrt1.o tryrt1

haslibrt.h: \
trylib.c compile load
	( ( ./compile trylib.c -o tryrt2.o && ./load tryrt2 -lrt ) >/dev/null 2>&1 \
	&& echo "#define HASLIBRT 1" || echo "WARNING!!! Not compiled -with -DHASLIBRT" 1>&2 ) > haslibrt.h
	rm -f tryrt2.o tryrt2

crypt.lib: \
trylib.c compile load
	( ( ./compile trylib.c -o trycrypt.o && ./load trycrypt -lcrypt ) >/dev/null 2>&1 \
	&& echo -lcrypt || echo "WARNING!!! Not linked with libcrypt" 1>&2 ) > crypt.lib
	rm -f trycrypt.o trycrypt

gsasl.lib: \
trylib.c compile load
	( ( ./compile trylib.c -o trygsasl1.o && ./load trygsasl1 -lgsasl ) >/dev/null 2>&1 \
	&& echo -lgsasl || echo "WARNING!!! Not linked with libgsasl" 1>&2 ) > gsasl.lib
	rm -f trygsasl1.o trygsasl1

haslibgsasl.h: \
trylib.c compile load
	( ( ./compile trylib.c -o trygsasl2.o && ./load trygsasl2 -lgsasl ) >/dev/null 2>&1 \
	&& echo "#define HASLIBGSASL 1" || echo "WARNING!!! Not compiled with -DHASLIBGSASL" 1<&2 ) > haslibgsasl.h
	rm -f trygsasl2.o trygsasl2

tls.lib: \
trytls.c compile conf-tls load conf-ld-$(SYSTEM)
	( ( ./compile `grep -v "^#" conf-tls` trytls.c && \
	./load trytls `cat conf-ld-$(SYSTEM)` \
	-lssl -lcrypto ) >tls.err 2>&1 \
	&& echo $(LDFLAGS) `cat conf-ld-$(SYSTEM)` -lssl -lcrypto || echo "WARNING!!! Not linked with libcrypto, libssl" 1>&2 ) > tls.lib
	echo -tls.err----; cat tls.err;echo ---tls.lib---; cat tls.lib
	rm -f trytls.o trytls tls.err

lspawn.o: \
compile chkspawn spawn.c auto_uids.h auto_spawn.h \
variables.h indimail_stub.h auto_control.h conf-virtual
	./chkspawn
	./compile `grep -h -v "^#" conf-virtual` \
	-DMAIN -DSPAWN=lspawn -DQSPAWN=qmail_lspawn \
	-Dreport_SPAWN=report_lspawn \
	-Dinitialize_SPAWN=initialize_lspawn \
	-Dtruncreport_SPAWN=truncreport_lspawn \
	spawn.c -o lspawn.o

rspawn.o: \
compile chkspawn spawn.c auto_uids.h auto_spawn.h \
variables.h indimail_stub.h auto_control.h conf-virtual
	./chkspawn
	./compile `grep -h -v "^#" conf-virtual` \
	-DMAIN -DSPAWN=rspawn -DQSPAWN=qmail_rspawn \
	-Dreport_SPAWN=report_rspawn \
	-Dinitialize_SPAWN=initialize_rspawn \
	-Dtruncreport_SPAWN=truncreport_rspawn \
	spawn.c -o rspawn.o

qmta_lspawn.o: \
compile chkspawn spawn.c auto_uids.h auto_spawn.h \
variables.h indimail_stub.h auto_control.h
	./chkspawn
	./compile -DSPAWN=lspawn -DQSPAWN=qmail_lspawn \
	-Dreport_SPAWN=report_lspawn \
	-Dinitialize_SPAWN=initialize_lspawn \
	-Dtruncreport_SPAWN=truncreport_lspawn \
	spawn.c -o qmta_lspawn.o

qmta_rspawn.o: \
compile chkspawn spawn.c auto_uids.h auto_spawn.h \
variables.h indimail_stub.h auto_control.h
	./chkspawn
	./compile -DSPAWN=rspawn -DQSPAWN=qmail_rspawn \
	-Dreport_SPAWN=report_rspawn \
	-Dinitialize_SPAWN=initialize_rspawn \
	-Dtruncreport_SPAWN=truncreport_rspawn \
	spawn.c -o qmta_rspawn.o

spf.o: \
compile spf.c ipme.h ip.h ipalloc.h strsalloc.h \
dns.h spf.h conf-spf conf-tls conf-ip
	./compile `grep -h -v "^#" conf-spf conf-tls conf-ip` spf.c

spfquery: \
load spfquery.o spf.o ip.o ipme.o ipalloc.o strsalloc.o \
dns.o variables.o socket_ip4loopback.o socket_v6loopback.o \
socket_v4mappedprefix.o tlsarralloc.o auto_control.o \
socket_v6any.o dns.lib
	./load spfquery spf.o ip.o ipme.o ipalloc.o \
	strsalloc.o auto_control.o socket_ip4loopback.o \
	socket_v6loopback.o tlsarralloc.o socket_v4mappedprefix.o \
	socket_v6any.o dns.o variables.o `cat dns.lib` $(QMAILLIB)

spfquery.o: \
compile spfquery.c spf.h dns.h conf-spf
	./compile `grep -h -v "^#" conf-spf` spfquery.c

splogger: \
load splogger.o syslog.lib socket.lib
	./load splogger `cat syslog.lib` `cat socket.lib` $(QMAILLIB)

splogger.o: \
compile splogger.c
	./compile splogger.c

srs.lib: trysrs.c compile load conf-srs dns.lib
	( ( ./compile -I/usr/include/srs2 -I/usr/local/include/srs2 \
	-I/opt/local/include/srs2 -I../libsrs2-x/libsrs2 \
	`grep -h -v "^#" conf-srs` trysrs.c -o trysrs1.o && \
	./load trysrs1 -L/usr/local/lib -L/opt/local/lib \
	-L../libsrs2-x/libsrs2/.libs -lsrs2 ) \
	>/dev/null 2>&1 && echo "-lsrs2" || exit 0 ) > srs.lib
	rm -f trysrs1.o trysrs1

hassrs.h: trysrs.c compile load srs.lib conf-indi
	( ( ./compile -I/usr/include/srs2 -I/usr/local/include/srs2 \
	-I/opt/local/include/srs2 -I../libsrs2-x/libsrs2 \
	`grep -h -v "^#" conf-srs` trysrs.c -o trysrs2.o && \
	./load trysrs2 -L/usr/local/lib -L/usr/local/lib \
	-L/opt/local/lib -L../libsrs2-x/libsrs2/.libs `cat srs.lib` ) \
	 >/dev/null 2>&1 && echo "#define HAVESRS 1" || exit 0 ) > hassrs.h
	rm -f trysrs2.o trysrs2

srs.o: compile srs.c hassrs.h srs.h control.h rcpthosts.h
	./compile -I/usr/include/srs2 -I/usr/local/include/srs2 \
		-I../libsrs2-x/libsrs2 -I/opt/local/include/srs2 srs.c

srsfilter: \
load srsfilter.o srs.o qmail.o control.o rcpthosts.o \
auto_prefix.o auto_control.o variables.o srs.lib
	./load srsfilter srs.o qmail.o control.o rcpthosts.o \
	auto_prefix.o auto_control.o variables.o \
	-L../libsrs2-x/libsrs2/.libs `cat srs.lib` $(QMAILLIB)

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
	ip.o ipalloc.o strsalloc.o `cat dns.lib socket.lib` $(QMAILLIB)

tcp-env.o: \
compile tcp-env.c strsalloc.h ip.h dns.h remoteinfo.h \
hassalen.h socket.h haveip6.h conf-spf conf-ip
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

etrn.o: compile etrn.c etrn.h rcpthosts.h control.h \
indimail_stub.h variables.h auto_libexec.h read_assign.h
	./compile etrn.c

atrn.o: \
compile atrn.c rcpthosts.h control.h auto_qmail.h \
auto_libexec.h read_assign.h
	./compile atrn.c

read_assign.o: \
compile read_assign.c auto_assign.c read_assign.h
	./compile read_assign.c

############### Begin queue-fix

queue-fix: \
load queue-fix.o get_uid.o auto_split.o auto_uids.o \
set_environment.o auto_control.o \
variables.o
	./load queue-fix get_uid.o auto_split.o auto_uids.o \
	auto_control.o variables.o \
	set_environment.o $(QMAILLIB)

queue-fix.o: \
compile queue-fix.c auto_split.h auto_uids.h tcpto.h
	./compile queue-fix.c

queue-fix.8: queue-fix.9 conf-split
	cat queue-fix.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	> queue-fix.8

qbase64: \
load qbase64.o
	./load qbase64 $(QMAILLIB)

qbase64.o: \
compile qbase64.c
	./compile qbase64.c

maildirdeliver: load maildirdeliver.o maildir_deliver.o \
syncdir.o
	./load maildirdeliver maildir_deliver.o syncdir.o \
	$(QMAILLIB)

maildirdeliver.o: \
compile maildirdeliver.c maildir_deliver.h conf-sync
	./compile `grep -h -v "^#" conf-sync` maildirdeliver.c

qmail-rm: load qmail-rm.o auto_split.o auto_qmail.o \
control.o auto_control.o variables.o fts.lib
	./load qmail-rm auto_split.o auto_qmail.o \
	control.o auto_control.o variables.o \
	`cat fts.lib` $(QMAILLIB)

qmail-rm.o: \
compile qmail-rm.c auto_split.h auto_qmail.h control.h \
hasrenameat.h hasunlinkat.h conf-queue
	./compile `grep -h -v "^#" conf-queue` qmail-rm.c

hasrenameat.h: \
tryrenameat.c compile load
	( ( ./compile tryrenameat.c \
	&& ./load tryrenameat ) >/dev/null 2>&1 \
	&& echo "#define HASRENAMEAT 1" || exit 0 ) > hasrenameat.h
	rm -f tryrenameat.o tryrenameat

qmail-rm.1: qmail-rm.9 conf-qmail conf-split
	cat qmail-rm.9 \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-rm.1

autoresponder: load autoresponder.o auto_sysconfdir.o \
variables.o control.o quote.o auto_control.o srs.o rcpthosts.o \
ip.o ipme.o ipalloc.o socket_v4mappedprefix.o socket_v6any.o \
qregex.o wildmat.o auto_prefix.o conf-srs
	./load autoresponder auto_control.o auto_sysconfdir.o \
	variables.o control.o quote.o ip.o ipme.o ipalloc.o \
	socket_v4mappedprefix.o socket_v6any.o qregex.o \
	wildmat.o auto_prefix.o srs.o rcpthosts.o $(QMAILLIB) \
	-L../libsrs2-x/libsrs2/.libs `cat srs.lib`

autoresponder.o: \
compile autoresponder.c auto_sysconfdir.h control.h ip.h ipme.h \
auto_prefix.h quote.h qregex.h conf-mime hassrs.h buffer_defs.h
	./compile -DMAKE_SEEKABLE -DQUOTE `grep -h -v "^#" conf-mime` autoresponder.c

822field: \
load 822field.o
	./load 822field $(QMAILLIB)

822field.o: \
compile 822field.c
	./compile 822field.c

822header: \
load 822header.o
	./load 822header $(QMAILLIB)

822header.o: \
compile 822header.c
	./compile 822header.c

# qtools
822body: load 822body.o
	./load 822body $(QMAILLIB)

822body.o: compile 822body.c buffer_defs.h
	./compile 822body.c

822headerok: load 822headerok.o
	./load 822headerok $(QMAILLIB)

822headerok.o: compile 822headerok.c buffer_defs.h
	./compile 822headerok.c

822bodyfilter: load 822bodyfilter.o
	./load 822bodyfilter $(QMAILLIB)

822bodyfilter.o: compile 822bodyfilter.c buffer_defs.h
	./compile 822bodyfilter.c

822headerfilter: load 822headerfilter.o
	./load 822headerfilter $(QMAILLIB)

822headerfilter.o: compile 822headerfilter.c buffer_defs.h
	./compile 822headerfilter.c

822addr: load 822addr.o
	./load 822addr $(QMAILLIB)

822addr.o: compile 822addr.c buffer_defs.h
	./compile 822addr.c

822fields: load 822fields.o
	./load 822fields $(QMAILLIB)

822fields.o: compile 822fields.c buffer_defs.h
	./compile 822fields.c

checkaddr: load checkaddr.o
	./load checkaddr $(QMAILLIB)

checkaddr.o: compile checkaddr.c buffer_defs.h
	./compile checkaddr.c

checkdomain: load checkdomain.o
	./load checkdomain $(QMAILLIB)

checkdomain.o: compile checkdomain.c buffer_defs.h
	./compile checkdomain.c

filterto.o: compile filterto.c qmail.h set_environment.h
	./compile filterto.c

filterto: load filterto.o qmail.o auto_prefix.o \
variables.o auto_control.o \
set_environment.o
	./load filterto qmail.o variables.o auto_prefix.o \
	auto_control.o set_environment.o \
	$(QMAILLIB)

filterto.1: filterto.9 conf-sysconfdir
	cat filterto.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> filterto.1

condtomaildir: load condtomaildir.o syncdir.o maildir_deliver.o
	./load condtomaildir syncdir.o maildir_deliver.o $(QMAILLIB)

condtomaildir.o: \
compile condtomaildir.c maildir_deliver.h
	./compile condtomaildir.c

iftoccfrom: load iftoccfrom.o
	./load iftoccfrom $(QMAILLIB)

iftoccfrom.o: compile iftoccfrom.c buffer_defs.h
	./compile iftoccfrom.c

ifaddr: load ifaddr.o
	./load ifaddr $(QMAILLIB)

ifaddr.o: compile ifaddr.c buffer_defs.h
	./compile ifaddr.c

replier: load replier.o qmail.o variables.o getconf.o \
auto_prefix.o auto_control.o set_environment.o
	./load replier qmail.o variables.o auto_control.o \
	auto_prefix.o getconf.o \
	set_environment.o $(QMAILLIB)

replier.o: compile replier.c getconf.h set_environment.h
	./compile replier.c

replier.1: replier.9 conf-sysconfdir
	cat replier.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> replier.1

replier-config: load replier-config.o auto_ezmlm.o auto_prefix.o
	./load replier-config auto_ezmlm.o auto_prefix.o $(QMAILLIB)

replier-config.o: compile replier-config.c \
auto_ezmlm.h auto_prefix.h help.t
	./compile replier-config.c

############### serialsmtp ###################################

serialsmtp: \
load serialsmtp.o quote.o
	./load serialsmtp quote.o $(QMAILLIB)

serialsmtp.o: \
compile serialsmtp.c quote.h conf-ip
	./compile `grep -h -v "^#" conf-ip` serialsmtp.c

maildirserial: \
load maildirserial.o qmail.o maildir.o quote.o \
variables.o auto_control.o prioq.o \
auto_prefix.o set_environment.o
	./load maildirserial qmail.o maildir.o variables.o \
	auto_control.o quote.o prioq.o \
	auto_prefix.o set_environment.o $(QMAILLIB)

maildirserial.1: maildirserial.9 conf-sysconfdir
	cat maildirserial.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> maildirserial.1

maildirserial.o: \
compile maildirserial.c prioq.h variables.h qmail.h \
auto_control.h set_environment.h conf-mime
	./compile `grep -h -v "^#" conf-mime` maildirserial.c

serialcmd: load serialcmd.o quote.o
	./load serialcmd quote.o $(QMAILLIB)

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
	./load rewind $(QMAILLIB)

rewind.o: \
compile rewind.c
	./compile rewind.c

serialqmtp: \
load serialqmtp.o
	./load serialqmtp $(QMAILLIB)

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
	./load leapsecs $(QMAILLIB)

leapsecs.o: compile leapsecs.c
	./compile leapsecs.c

leapsecs.dat: leapsecs.txt leapsecs
	./leapsecs < leapsecs.txt > leapsecs.dat

###### qmailanalog

matchup: load matchup.o
	./load matchup $(QMAILLIB)

matchup.o: compile matchup.c
	./compile matchup.c

mlmatchup: load mlmatchup.o
	./load mlmatchup $(QMAILLIB)

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
	chmod +x $@

suids: \
warn-auto.sh suids.sh
	cat warn-auto.sh suids.sh \
	> suids
	chmod +x $@

xqp: \
warn-auto.sh xqp.sh
	cat warn-auto.sh xqp.sh \
	> xqp
	chmod +x $@

xrecipient: \
warn-auto.sh xrecipient.sh
	cat warn-auto.sh xrecipient.sh \
	> xrecipient
	chmod +x $@

xsender: \
warn-auto.sh xsender.sh
	cat warn-auto.sh xsender.sh \
	> xsender
	chmod +x $@

zddist: \
warn-auto.sh zddist.sh conf-libexec
	cat warn-auto.sh zddist.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zddist
	chmod +x $@

ddist: \
warn-auto.sh ddist.sh
	cat warn-auto.sh ddist.sh \
	> ddist
	chmod +x $@

zdeferrals: \
warn-auto.sh zdeferrals.sh conf-libexec
	cat warn-auto.sh zdeferrals.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zdeferrals
	chmod +x $@

deferrals: warn-auto.sh deferrals.sh
	cat warn-auto.sh deferrals.sh \
	> deferrals
	chmod +x $@

zfailures: \
warn-auto.sh zfailures.sh conf-libexec
	cat warn-auto.sh zfailures.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zfailures
	chmod +x $@

failures: \
warn-auto.sh failures.sh
	cat warn-auto.sh failures.sh \
	> failures
	chmod +x $@

zoverall: \
warn-auto.sh zoverall.sh
	cat warn-auto.sh zoverall.sh \
	> zoverall
	chmod +x $@

zrecipients: \
warn-auto.sh zrecipients.sh conf-libexec
	cat warn-auto.sh zrecipients.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zrecipients
	chmod +x $@

zrhosts: \
warn-auto.sh zrhosts.sh conf-libexec
	cat warn-auto.sh zrhosts.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zrhosts
	chmod +x $@

zsmtp: \
warn-auto.sh zsmtp.sh conf-libexec
	cat warn-auto.sh zsmtp.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zsmtp
	chmod +x $@

zrxdelay: \
warn-auto.sh zrxdelay.sh conf-libexec
	cat warn-auto.sh zrxdelay.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zrxdelay
	chmod +x $@

zsenders: \
warn-auto.sh zsenders.sh conf-libexec
	cat warn-auto.sh zsenders.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zsenders
	chmod +x $@

zsendmail: \
warn-auto.sh zsendmail.sh
	cat warn-auto.sh zsendmail.sh \
	> zsendmail
	chmod +x $@

zsuccesses: \
warn-auto.sh zsuccesses.sh conf-libexec
	cat warn-auto.sh zsuccesses.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zsuccesses
	chmod +x $@

zsuids: \
warn-auto.sh zsuids.sh conf-libexec
	cat warn-auto.sh zsuids.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zsuids
	chmod +x $@

smtp-matchup: \
warn-auto.sh smtp-matchup.sh
	cat warn-auto.sh smtp-matchup.sh \
	> smtp-matchup
	chmod +x $@

recipients: \
warn-auto.sh recipients.sh
	cat warn-auto.sh recipients.sh \
	> recipients
	chmod +x $@

rhosts: \
warn-auto.sh rhosts.sh
	cat warn-auto.sh rhosts.sh \
	> rhosts
	chmod +x $@

rsmtp: \
warn-auto.sh rsmtp.sh conf-sysconfdir
	cat warn-auto.sh rsmtp.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtp
	chmod +x $@

rsmtpfailures: \
warn-auto.sh rsmtpfailures.sh
	cat warn-auto.sh rsmtpfailures.sh \
	> rsmtpfailures
	chmod +x $@

rsmtpsdomains: \
warn-auto.sh rsmtpsdomains.sh conf-sysconfdir
	cat warn-auto.sh rsmtpsdomains.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtpsdomains
	chmod +x $@

rsmtprdomains: \
warn-auto.sh rsmtprdomains.sh conf-sysconfdir
	cat warn-auto.sh rsmtprdomains.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtprdomains
	chmod +x $@

rsmtprecipients: \
warn-auto.sh rsmtprecipients.sh conf-sysconfdir
	cat warn-auto.sh rsmtprecipients.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtprecipients
	chmod +x $@

rsmtpsenders: \
warn-auto.sh rsmtpsenders.sh conf-sysconfdir
	cat warn-auto.sh rsmtpsenders.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rsmtpsenders
	chmod +x $@

rxdelay: \
warn-auto.sh rxdelay.sh
	cat warn-auto.sh rxdelay.sh \
	> rxdelay
	chmod +x $@

rspamrdomain: \
warn-auto.sh rspamrdomain.sh conf-sysconfdir
	cat warn-auto.sh rspamrdomain.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rspamrdomain
	chmod +x $@

rspamsdomain: \
warn-auto.sh rspamsdomain.sh conf-sysconfdir
	cat warn-auto.sh rspamsdomain.sh \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> rspamsdomain
	chmod +x $@

zspam: \
warn-auto.sh zspam.sh conf-libexec
	cat warn-auto.sh zspam.sh \
	| sed s}LIBEXEC}"`head -1 conf-libexec`"}g \
	> zspam
	chmod +x $@

senders: \
warn-auto.sh senders.sh conf-qmail
	cat warn-auto.sh senders.sh \
	> senders
	chmod +x $@

fmt_ushort.o: \
compile fmt_ushort.c fmt.h
	./compile fmt_ushort.c

columnt: \
load columnt.o slurpclose.o
	./load columnt slurpclose.o $(QMAILLIB)

columnt.o: compile columnt.c slurpclose.h
	./compile columnt.c

columnt.o: compile columnt.c slurpclose.h \
	./compile columnt.c

qmail-cat: load qmail-cat.o
	./load qmail-cat $(QMAILLIB)

qmail-cat.o: compile qmail-cat.c buffer_defs.h
	./compile qmail-cat.c

qaes: load qaes.o
	./load qaes -lcrypto $(QMAILLIB)

qaes.o: compile qaes.c buffer_defs.h
	./compile qaes.c

qarf: \
load qarf.o
	./load qarf $(QMAILLIB)

qarf.o: compile qarf.c buffer_defs.h
	./compile qarf.c

set_environment.o: compile set_environment.c \
auto_control.h variables.h
	./compile set_environment.c

qnotify: \
load qnotify.o qmail.o variables.o auto_prefix.o \
auto_control.o set_environment.o
	./load qnotify qmail.o variables.o auto_prefix.o \
	auto_control.o set_environment.o \
	$(QMAILLIB)

qnotify.o: \
compile qnotify.c qmail.h set_environment.h buffer_defs.h
	./compile qnotify.c

rrt: \
load rrt.o qmail.o control.o variables.o \
auto_prefix.o auto_control.o set_environment.o
	./load rrt qmail.o control.o variables.o auto_prefix.o \
	auto_control.o set_environment.o $(QMAILLIB)

rrt.o: \
compile rrt.c control.h qmail.h buffer_defs.h \
variables.h set_environment.h
	./compile rrt.c

qmail-poppass: \
load qmail-poppass.o auto_uids.o
	./load qmail-poppass auto_uids.o $(QMAILLIB)

qmail-poppass.o: \
compile qmail-poppass.c auto_uids.h buffer_defs.h
	./compile qmail-poppass.c

rpmattr: \
load rpmattr.o auto_uids.o get_uid.o
	./load rpmattr auto_uids.o get_uid.o $(QMAILLIB)
	

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
	./load iftocc $(QMAILLIB)

iftocc.o: compile iftocc.c
	./compile iftocc.c

ofmipd: load ofmipd.o rwhconfig.o qmail.o \
auto_prefix.o control.o auto_control.o variables.o 
	./load ofmipd rwhconfig.o qmail.o \
	auto_prefix.o auto_control.o variables.o control.o \
	$(static_option) $(QMAILLIB) $(dynamic_option)

ofmipd.o: compile ofmipd.c qmail.h buffer_defs.h \
variables.h rwhconfig.h
	./compile ofmipd.c

rwhconfig.o: compile rwhconfig.c rwhconfig.h variables.h \
auto_control.h
	./compile rwhconfig.c

ofmipname: load ofmipname.o
	./load ofmipname $(QMAILLIB)

ofmipname.o: \
compile ofmipname.c
	./compile ofmipname.c

new-inject: \
load new-inject.o qmail.o rwhconfig.o \
variables.o auto_control.o auto_prefix.o set_environment.o
	./load new-inject qmail.o variables.o \
	auto_control.o rwhconfig.o auto_prefix.o set_environment.o \
	$(static_option) $(QMAILLIB) $(dynamic_option)

new-inject.1: new-inject.9 conf-sysconfdir
	cat new-inject.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> new-inject.1

new-inject.o: \
compile new-inject.c qmail.h rwhconfig.h set_environment.h
	./compile new-inject.c

822date: load 822date.o
	./load 822date $(QMAILLIB)

822date.o: compile 822date.c
	./compile 822date.c

822received: load 822received.o
	./load 822received $(QMAILLIB)

822received.o: compile 822received.c
	./compile 822received.c

822print: load 822print.o
	./load 822print $(QMAILLIB)

822print.o: compile 822print.c
	./compile 822print.c

parsedate: load parsedate.o
	./load parsedate $(QMAILLIB)

parsedate.o: compile parsedate.c
	./compile parsedate.c

tokenize: load tokenize.o
	./load tokenize $(QMAILLIB)

tokenize.o: compile tokenize.c
	./compile tokenize.c

addrlist: load addrlist.o
	./load addrlist $(QMAILLIB)

addrlist.o: compile addrlist.c
	./compile addrlist.c

qquote: load qquote.o
	./load qquote $(QMAILLIB)

qquote.o: compile qquote.c
	./compile qquote.c

getconf.o: compile getconf.c getconf.h
	./compile getconf.c

auto_ezmlm.o: compile \
auto_ezmlm.c
	./compile auto_ezmlm.c

auto_ezmlm.c: auto-str conf-ezmlm
	./auto-str auto_ezmlm `head -1 conf-ezmlm` > auto_ezmlm.c

make-text: load make-text.o
	./load make-text $(QMAILLIB)

make-text.o: compile make-text.c
	./compile make-text.c

help.t: make-text help.txt
	./make-text <help.txt >help.t

svcfns.o: compile svcfns.c bool.h svcfns.h
	./compile svcfns.c

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
	chmod +x $@

tmprsadh: update_tmprsadh
	echo "Creating new temporary RSA and DH parameters"
	./update_tmprsadh

dot-forward: \
load dot-forward.o control.o qmail.o variables.o \
auto_control.o auto_prefix.o \
set_environment.o
	./load dot-forward control.o qmail.o variables.o \
	auto_control.o auto_prefix.o \
	set_environment.o $(QMAILLIB)

dot-forward.1: dot-forward.9 conf-sysconfdir
	cat dot-forward.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> dot-forward.1

dot-forward.o: compile dot-forward.c control.h qmail.h \
buffer_defs.h set_environment.h
	./compile dot-forward.c

fastforward: \
load fastforward.o slurpclose.o strset.o qmail.o variables.o \
auto_control.o auto_prefix.o set_environment.o
	./load fastforward slurpclose.o strset.o qmail.o variables.o \
	auto_control.o auto_prefix.o \
	set_environment.o $(QMAILLIB)

fastforward.o: \
compile fastforward.c strset.h qmail.h slurpclose.h \
buffer_defs.h set_environment.h
	./compile fastforward.c

fastforward.1: fastforward.9 conf-sysconfdir
	cat fastforward.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> fastforward.1

strset.o: \
compile strset.c strset.h
	./compile strset.c

printforward: \
load printforward.o
	./load printforward $(QMAILLIB)

printforward.o: \
compile printforward.c
	./compile printforward.c

printmaillist: \
load printmaillist.o
	./load printmaillist $(QMAILLIB)

printmaillist.o: \
compile printmaillist.c
	./compile printmaillist.c

setforward: \
load setforward.o
	./load setforward $(QMAILLIB)

setforward.o: \
compile setforward.c
	./compile setforward.c

setmaillist: \
load setmaillist.o
	./load setmaillist $(QMAILLIB)

setmaillist.o: \
compile setmaillist.c
	./compile setmaillist.c

inewaliases: \
load inewaliases.o control.o variables.o auto_control.o
	./load inewaliases control.o variables.o \
	auto_control.o $(QMAILLIB)

inewaliases.o: \
compile inewaliases.c control.h variables.h
	./compile inewaliases.c

newinclude: \
load newinclude.o control.o variables.o auto_control.o
	./load newinclude control.o variables.o \
	auto_control.o $(QMAILLIB)

newinclude.o: \
compile newinclude.c control.h variables.h
	./compile newinclude.c

sorted.o: compile sorted.c sorted.h
	./compile sorted.c

sortedtest: load sortedtest.o sorted.o
	./load sortedtest sorted.o $(QMAILLIB)

sortedtest.o: compile sortedtest.c sorted.h
	./compile sortedtest.c

generic.o: compile generic.c qmail.h
	./compile generic.c

relaytest.o: compile relaytest.c
	./compile relaytest.c

relaytest: load relaytest.o
	./load relaytest $(QMAILLIB)

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
	./load cdbmake $(QMAILLIB)

cdbmake.o: \
compile cdbmake.c
	./compile cdbmake.c

cdbget: load cdbget.o
	./load cdbget $(QMAILLIB)

cdbget.o: \
compile cdbget.c
	./compile cdbget.c

cdbdump: load cdbdump.o
	./load cdbdump $(QMAILLIB)

cdbdump.o: compile cdbdump.c
	./compile cdbdump.c

cdbstats: load cdbstats.o
	./load cdbstats $(QMAILLIB)

cdbstats.o: compile cdbstats.c
	./compile cdbstats.c

cdbtest: load cdbtest.o
	./load cdbtest $(QMAILLIB)

cdbtest.o: compile cdbtest.c
	./compile cdbtest.c

testzero: load testzero.o
	./load testzero $(QMAILLIB)

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
load qmail-qfilter.o qmulti.o control.o auto_control.o \
custom_error.o auto_prefix.o auto_qmail.o getqueue.o \
variables.o rt.lib
	./load qmail-qfilter qmulti.o control.o auto_qmail.o \
	auto_prefix.o auto_control.o variables.o custom_error.o \
	getqueue.o `cat rt.lib` $(QMAILLIB)

qmail-qfilter.o: \
compile qmail-qfilter.c buffer_defs.h qmail.h qmulti.h custom_error.h
	./compile qmail-qfilter.c

qfrontend.1: qfrontend.9 conf-libexec
	$(edit) qfrontend.9 > $@

cdb_match.o: \
compile cdb_match.c variables.h auto_control.h cdb_match.h
	./compile cdb_match.c

qmail-greyd: load qmail-greyd.o ip.o socket_v6any.o \
socket_v4mappedprefix.o variables.o auto_control.o \
control.o tablematch.o cdb_match.o
	./load qmail-greyd auto_control.o ip.o socket_v6any.o \
	socket_v4mappedprefix.o variables.o control.o tablematch.o \
	cdb_match.o $(QMAILLIB)

qmail-greyd.o: \
compile qmail-greyd.c ip.h control.h variables.h greylist.h \
tablematch.h socket.h cdb_match.h auto_control.h haveip6.h \
varargs.h conf-ip
	./compile -DDYNAMIC_BUF `grep -h -v "^#" conf-ip` qmail-greyd.c

qmail-daned: \
load qmail-daned.o ip.o socket_v6any.o socket_v4mappedprefix.o \
variables.o auto_control.o control.o ipalloc.o dns.o dnsdoe.o \
strsalloc.o socket_tcp.o socket_tcp6.o timeoutconn.o tlsarralloc.o \
socket_ip4loopback.o socket_v6loopback.o auto_sysconfdir.o \
dossl.o cdb_match.o starttls.o dns.lib tls.lib
	./load qmail-daned auto_control.o auto_sysconfdir.o \
	cdb_match.o ip.o socket_v6any.o socket_v4mappedprefix.o \
	dossl.o tlsarralloc.o variables.o control.o ipalloc.o dns.o \
	dnsdoe.o strsalloc.o socket_tcp.o socket_tcp6.o timeoutconn.o \
	socket_ip4loopback.o socket_v6loopback.o starttls.o \
	`cat dns.lib tls.lib` $(QMAILLIB)

qmail-daned.o: \
compile qmail-daned.c control.h variables.h \
tlsacheck.h haveip6.h ip.h auto_control.h \
cdb_match.h hastlsa.h starttls.h conf-ip conf-tls
	./compile -DDYNAMIC_BUF `grep -h -v "^#" conf-ip conf-tls` \
	qmail-daned.c

qmail-daned.8: \
qmail-daned.9 conf-qmail conf-sysconfdir
	cat qmail-daned.9 \
	| sed s}SYSCONF}"`head -1 conf-sysconfdir`"}g \
	> qmail-daned.8

starttls.o: compile starttls.c conf-tls hastlsa.h conf-ip \
socket.h ip.h dns.h control.h variables.h tlsacheck.h \
ipalloc.h tlsarralloc.h auto_control.h auto_sysconfdir.h \
buffer_defs.h dossl.h haveip6.h varargs.h
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
	ip.o socket_ip4loopback.o socket_v6any.o $(QMAILLIB)

qdane.o: \
compile qdane.c tlsacheck.h
	./compile qdane.c

batv: load batv.o
	./load batv $(QMAILLIB) -lcrypto

batv.o: compile batv.c conf-batv batv.h
	./compile `grep -h -v "^#" conf-batv` batv.c

uacl: \
load uacl.o mail_acl.o wildmat.o control.o \
variables.o qregex.o do_match.o auto_control.o
	./load uacl mail_acl.o wildmat.o control.o \
	variables.o qregex.o do_match.o auto_control.o $(QMAILLIB)

uacl.o: \
compile uacl.c qregex.h mail_acl.h control.h \
wildmat.h varargs.h
	./compile uacl.c

qmail-sql: \
load qmail-sql.o variables.o auto_control.o load_mysql.o \
sqlmatch.o auto_uids.o indimail_stub.o control.o
	./load qmail-sql indimail_stub.o variables.o auto_control.o \
	control.o sqlmatch.o auto_uids.o load_mysql.o \
	-ldl $(QMAILLIB)

qmail-sql.o: \
compile qmail-sql.c sqlmatch.h variables.h \
auto_uids.h auto_control.h hasmysql.h load_mysql.h \
mysql_config conf-sql mysql_inc
	./compile `grep -h -v "^#" conf-sql mysql_inc` qmail-sql.c

qmail-sql.8: \
qmail-sql.9 conf-sysconfdir
	cat qmail-sql.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> qmail-sql.8

sql-database: \
load sql-database.o variables.o auto_control.o load_mysql.o \
sqlmatch.o auto_uids.o indimail_stub.o control.o
	./load sql-database indimail_stub.o variables.o \
	auto_control.o control.o sqlmatch.o auto_uids.o \
	load_mysql.o -ldl $(QMAILLIB)

sql-database.o: \
compile sql-database.c sqlmatch.h variables.h \
auto_uids.h auto_control.h hasmysql.h load_mysql.h \
mysql_config conf-sql mysql_inc
	./compile `grep -h -v "^#" conf-sql mysql_inc` sql-database.c

sql-database.8: \
sql-database.9 conf-sysconfdir
	cat sql-database.9 \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> sql-database.8
	
sys-checkpwd: load sys-checkpwd.o \
shadow.lib crypt.lib
	./load sys-checkpwd \
	`cat shadow.lib crypt.lib` $(QMAILLIB)

sys-checkpwd.o: \
compile sys-checkpwd.c hasspnam.h \
hasuserpw.h
	./compile sys-checkpwd.c

hasifaddr.h: \
tryifaddr.c compile load
	( ( ./compile tryifaddr.c \
	&& ./load tryifaddr ) >/dev/null 2>&1 \
	&& echo "#define HASIFADDR 1" || exit 0 ) > hasifaddr.h
	rm -f tryifaddr.o tryifaddr

hasspnam.h: \
tryspnam.c compile load
	( ( ./compile tryspnam.c \
	&& ./load tryspnam ) >/dev/null 2>&1 \
	&& echo "#define HASGETSPNAM 1" || exit 0 ) > hasspnam.h
	rm -f tryspnam.o tryspnam

hasuserpw.h: \
tryuserpw.c compile load
	( ( ./compile tryuserpw.c \
	&& ./load tryuserpw ) >/dev/null 2>&1 \
	&& echo "#define HASGETUSERPW 1" || exit 0 ) > hasuserpw.h
	rm -f tryuserpw.o tryuserpw

ldap.lib: \
tryldap.c compile load hasldap.h
	( ( ./compile -DLDAP_DEPRECATED=1 tryldap.c && \
	./load tryldap -lldap ) >/dev/null 2>&1 \
	&& echo -lldap || exit 0 ) > ldap.lib
	rm -f tryldap.o tryldap

hasldap.h: \
tryldap.c compile load
	( ( ./compile -DLDAP_DEPRECATED=1 tryldap.c && \
	./load tryldap -lldap ) >/dev/null 2>&1 \
	&& echo "#define HASLDAP 1" || exit 0 ) > hasldap.h
	rm -f tryldap.o tryldap

shadow.lib: \
trylib.c compile load
	( ( ./compile trylib.c && \
	./load trylib -lshadow ) >/dev/null 2>&1 \
	&& echo -lshadow || exit 0 ) > shadow.lib
	rm -f trylib.o trylib

ldap-checkpwd: \
load ldap-checkpwd.o ldap.lib crypt.lib
	./load ldap-checkpwd `cat crypt.lib ldap.lib` $(QMAILLIB)

ldap-checkpwd.o: \
compile ldap-checkpwd.c hasldap.h
	./compile -DEXTENDED_ATTRIBUTES -DLDAP_DEPRECATED ldap-checkpwd.c

surblfilter.8: surblfilter.9 conf-qmail conf-sysconfdir
	cat surblfilter.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> surblfilter.8

surblfilter: \
load surblfilter.o control.o tlsarralloc.o auto_control.o variables.o \
strsalloc.o ip.o dns.o ipalloc.o socket_v6any.o socket_v4mappedprefix.o \
dns.lib rt.lib
	./load surblfilter auto_control.o control.o variables.o tlsarralloc.o \
	strsalloc.o ip.o dns.o ipalloc.o socket_v6any.o \
	socket_v4mappedprefix.o `cat dns.lib rt.lib` $(QMAILLIB)

surblfilter.o: \
compile surblfilter.c control.h dns.h ip.h \
ipalloc.h variables.h buffer_defs.h qmail.h auto_control.h
	./compile surblfilter.c

surblqueue: \
warn-auto.sh surblqueue.sh conf-prefix
	cat warn-auto.sh surblqueue.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> surblqueue
	chmod 755 surblqueue

drate: \
load drate.o variables.o auto_uids.o auto_qmail.o \
auto_control.o control.o do_rate.o getDomainToken.o \
parse_env.o get_uid.o
	./load drate variables.o auto_uids.o parse_env.o \
	auto_qmail.o do_rate.o getDomainToken.o wildmat.o \
	auto_control.o control.o get_uid.o -lm $(QMAILLIB)

drate.o: \
compile drate.c variables.h auto_uids.h auto_qmail.h \
do_rate.h control.h getDomainToken.h
	./compile drate.c

cidr: load cidr.o
	./load cidr -lm

cidr.o: compile cidr.c
	./compile cidr.c

nowutc: load nowutc.o
	./load nowutc $(QMAILLIB)

nowutc.o: compile nowutc.c
	./compile nowutc.c

yearcal: load yearcal.o
	./load yearcal $(QMAILLIB)

yearcal.o: compile yearcal.c
	./compile yearcal.c

udplogger: load udplogger.o
	./load udplogger $(QMAILLIB)

udplogger.o: compile udplogger.c conf-ip haveip6.h
	./compile -DDYNAMIC_BUF `grep -h -v "^#" conf-ip` udplogger.c

udpclient: load udpclient.o
	./load udpclient $(QMAILLIB)

udpclient.o: compile udpclient.c
	./compile udpclient.c

whois: \
load whois.o
	./load whois $(QMAILLIB)

whois.o:\
compile whois.c
	./compile `grep -h -v "^#" conf-ip` whois.c

conf-release:
	if test ! -f conf-release ; then \
		echo "1.1" > conf-release; \
	fi

indimail-mta.spec: indimail-mta.spec.in doc/ChangeLog mysql_config \
conf-release conf-version conf-prefix conf-libexec conf-qmail \
conf-sysconfdir conf-shared conf-servicedir conf-email
	(cat $@.in; sh ./catChangeLog) | $(edit) > indimail-mta.spec

PKGBUILD: PKGBUILD.in doc/ChangeLog mysql_config conf-release \
conf-version conf-prefix conf-libexec conf-qmail conf-sysconfdir \
conf-shared conf-servicedir conf-email indimail-mta.changes
	cat $@.in | $(edit) > PKGBUILD

indimail-mta.changes: catChangeLog doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@

debian/Makefile: debian/Makefile.in conf-release conf-version \
conf-prefix conf-libexec conf-qmail conf-sysconfdir conf-shared
	$(edit) $@.in > $@

maildirsize: \
load maildirsize.o qcount_dir.o
	./load maildirsize qcount_dir.o $(QMAILLIB)

maildirsize.o: \
compile maildirsize.c indimail_stub.h auto_control.h variables.h
	./compile maildirsize.c

qcount_dir.o: \
compile qcount_dir.c
	./compile qcount_dir.c

ctrlenv: \
load ctrlenv.o control.o auto_control.o variables.o \
load_mysql.o indimail_stub.o sqlmatch.o cdb_match.o \
auto_uids.o
	./load ctrlenv auto_control.o control.o variables.o \
	load_mysql.o indimail_stub.o sqlmatch.o \
	auto_uids.o cdb_match.o \
	$(QMAILLIB) -ldl

ctrlenv.o: compile ctrlenv.c variables.h \
hasmysql.h load_mysql.h indimail_stub.h \
auto_control.h sqlmatch.h cdb_match.h mysql_config \
mysql_inc conf-sql
	./compile `grep -h -v "^#" conf-sql mysql_inc` ctrlenv.c

ctrlenv.8: \
ctrlenv.9 conf-prefix conf-sysconfdir
	cat ctrlenv.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}CONTROL}"`head -1 conf-sysconfdir`"}g \
	> ctrlenv.8

sslerator: load sslerator.o variables.o auto_control.o \
control.o auto_sysconfdir.o tls.lib
	./load sslerator variables.o auto_control.o \
	auto_sysconfdir.o control.o `cat tls.lib` $(QMAILLIB)

sslerator.o: compile sslerator.c control.h auto_sysconfdir.h \
conf-tls
	./compile `grep -h -v "^#" conf-tls` sslerator.c

sslerator.8: sslerator.9 conf-sysconfdir
	cat sslerator.9 \
	| sed s}@sysconfdir\@}"`head -1 conf-sysconfdir`"}g \
	> sslerator.8

################### PLUGINS ########################################
# dnsblcheck plugin
smtpd-plugin.so: dnsbl.o control.o auto_control.o variables.o \
socket_v6any.o socket_v4mappedprefix.o ip.o ipalloc.o \
strsalloc.o tlsarralloc.o dns.o dns.lib conf-ld-$(SYSTEM)
	if test $(SYSTEM) = DARWIN ; then \
		$(CC) -dynamiclib -o smtpd-plugin.so dnsbl.o \
		control.o auto_control.o variables.o \
		socket_v6any.o socket_v4mappedprefix.o ip.o ipalloc.o \
		strsalloc.o tlsarralloc.o dns.o \
		$(static_option) `cat conf-ld-$(SYSTEM) 2>/dev/null` $(QMAILLIB) \
		$(dynamic_option) `cat dns.lib`; \
	elif test $(SYSTEM) = FREEBSD ; then \
		$(LD) -shared -o smtpd-plugin.so dnsbl.o control.o \
		auto_control.o variables.o socket_v6any.o \
		socket_v4mappedprefix.o ip.o ipalloc.o \
		strsalloc.o tlsarralloc.o dns.o `cat dns.lib` \
		`cat conf-ld-$(SYSTEM) 2>/dev/null` $(QMAILLIB); \
	else \
		$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 \
		-o smtpd-plugin.so dnsbl.o control.o auto_control.o variables.o \
		socket_v6any.o socket_v4mappedprefix.o ip.o ipalloc.o \
		strsalloc.o tlsarralloc.o dns.o `cat dns.lib` \
		$(static_option) `cat conf-ld-$(SYSTEM) 2>/dev/null` $(QMAILLIB) \
		$(dynamic_option); \
	fi

dnsbl.o: compile dnsbl.c smtp_plugin.h dns.h control.h
	./compile dnsbl.c

smtpd-plugin0.so: smtp_plugin.o
	if test $(SYSTEM) = DARWIN ; then \
		$(CC) -dynamiclib -o smtpd-plugin0.so smtp_plugin.o; \
	elif test $(SYSTEM) = FREEBSD ; then \
		$(LD) -shared -o smtpd-plugin0.so smtp_plugin.o; \
	else \
		$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 \
		-o smtpd-plugin0.so smtp_plugin.o; \
	fi

smtp_plugin.o: compile smtp_plugin.c smtp_plugin.h
	./compile smtp_plugin.c

generic.so: generic.o auto_scancmd.o conf-ld-$(SYSTEM)
	if test $(SYSTEM) = DARWIN ; then \
		$(LD) -bundle -undefined dynamic_lookup -o generic.so generic.o \
		auto_scancmd.o `cat conf-ld-$(SYSTEM) 2>/dev/null` \
		$(QMAILLIB); \
	else \
		$(CC) -shared -rdynamic -nostartfiles -fPIC -s \
		-O4 -o generic.so generic.o auto_scancmd.o \
		`cat conf-ld-$(SYSTEM) 2>/dev/null` $(QMAILLIB); \
	fi

dnsblcheck: load dnsblcheck.o variables.o auto_control.o ip.o \
socket_v4mappedprefix.o socket_v6any.o dns.o control.o strsalloc.o \
ipalloc.o tlsarralloc.o dns.lib
	./load dnsblcheck variables.o auto_control.o ip.o \
	socket_v4mappedprefix.o socket_v6any.o dns.o control.o strsalloc.o \
	ipalloc.o tlsarralloc.o $(QMAILLIB) `cat dns.lib`

dnsblcheck.o: compile dnsblcheck.c control.h dns.h
	./compile dnsblcheck.c

test-recipients: \
load test-recipients.o variables.o recipients.o auto_control.o \
indimail_stub.o control.o auto_break.o auto_sysconfdir.o
	./load test-recipients variables.o recipients.o auto_control.o \
	indimail_stub.o control.o auto_break.o auto_sysconfdir.o \
	$(QMAILLIB) -ldl

test-recipients.o: \
compile test-recipients.c recipients.h variables.h auto_control.h \
indimail_stub.h
	./compile test-recipients.c

test-recipients.1: \
test-recipients.9 conf-sysconfdir conf-libexec
	$(edit) test-recipients.9 > $@

make-recipients: make-recipients.sh conf-qmail conf-sysconfdir
	cat warn-auto.sh make-recipients.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}SYSCONF}"`head -1 conf-sysconfdir`"}g \
	> $@
	chmod +x $@

make-recipients.1: \
make-recipients.9 conf-sysconfdir
	$(edit) make-recipients.9 > $@

filterit: \
load filterit.o filterit_sub.o variables.o quote.o maildir_deliver.o \
syncdir.o qmail.o srs.o set_environment.o auto_control.o auto_prefix.o \
control.o rcpthosts.o srs.lib
	./load filterit filterit_sub.o variables.o auto_control.o \
	auto_prefix.o quote.o qmail.o maildir_deliver.o syncdir.o \
	set_environment.o control.o rcpthosts.o srs.o \
	-L../libsrs2-x/libsrs2/.libs `cat srs.lib` $(QMAILLIB)

filterit.o: \
compile filterit.c filterit.h
	./compile filterit.c

filterit_sub.o: \
compile filterit_sub.c cdb_match.o quote.h maildir_deliver.h \
buffer_defs.h qmail.h set_environment.h filterit.h hassrs.h
	./compile filterit_sub.c

filterit.1: filterit.9 conf-sysconfdir conf-prefix
	cat filterit.9 \
	| sed s}@prefix\@}"`head -1 conf-prefix`"}g \
	| sed s}@controldir\@}"`head -1 conf-sysconfdir`/control"}g \
	> filterit.1

indimail-mta.perms: perm_list.in mysql_config \
conf-prefix conf-libexec conf-qmail conf-sysconfdir conf-shared
	$(edit) perm_list.in > $@

svctool: svctool.in mysql_config conf-prefix conf-libexec conf-qmail \
conf-sysconfdir conf-shared conf-servicedir warn-auto.sh
	( cat warn-auto.sh; \
	$(edit) $@.in \
	| sed s}QUEUESPLIT}"`head -1 conf-split`"}g \
	) > $@
	chmod +x $@

svctool.8: svctool.9 mysql_config \
conf-prefix conf-libexec conf-qmail conf-sysconfdir conf-shared \
conf-servicedir
	$(edit) svctool.9 > $@

create_services: create_services.in \
conf-prefix conf-libexec conf-qmail \
conf-sysconfdir conf-shared conf-servicedir
	$(edit) $@.in > $@; chmod 755 $@

favicon.base64: favicon.png
	base64 < favicon.png > favicon.base64

qlocal_upgrade: \
warn-auto.sh qlocal_upgrade.in conf-prefix conf-libexec \
conf-sysconfdir conf-servicedir
	cat warn-auto.sh qlocal_upgrade.in \
	| $(edit) > $@

qupgrade: \
warn-auto.sh qupgrade.in conf-prefix conf-libexec \
conf-sysconfdir conf-servicedir
	cat warn-auto.sh qupgrade.in \
	| $(edit) > $@

qtop: \
warn-auto.sh qtop.in
	cat warn-auto.sh $@.in | $(edit) > $@
	chmod 755 $@

qtop.1: qtop.9 conf-logdir
	$(edit) qtop.9 > qtop.1

qmta-send.service: qmta-send.service.in warn-auto.sh
	(sed -n '2,$$p' warn-auto.sh;cat $@.in) | $(edit) > $@

indimail-env.7: indimail-env.9 conf-split conf-sysconfdir
	$(edit) indimail-env.9 \
	| sed s}DIRSPLIT}"`head -1 conf-split`"}g \
	>$@

indimail-services.7: indimail-services.9 conf-split conf-sysconfdir \
conf-logdir
	$(edit) indimail-services.9 >$@

backup.conf: backup.conf.in
	(sed -n '2,$$p' warn-auto.sh;cat $@.in) | $(edit) > $@

rd-remote.o: \
compile rd-remote.c control.h auto_control.h variables.h \
do_match.h quote.h auto_prefix.h
	./compile rd-remote.c

rd-remote: \
load rd-remote.o control.o variables.o auto_control.o auto_prefix.o \
do_match.o quote.o
	./load rd-remote control.o variables.o auto_control.o auto_prefix.o \
	wildmat.o do_match.o quote.o $(QMAILLIB)

printass.o: \
compile printass.c read_assign.h auto_assign.h
	./compile printass.c

printass: \
load printass.o read_assign.o auto_assign.o
	./load printass read_assign.o auto_assign.o $(QMAILLIB)

printass.1: \
printass.9 conf-sysconfdir
	$(edit) printass.9 >$@

#################################################################
indimail-mta-permissions.easy: indimail-mta-permissions.easy.in \
conf-prefix conf-libexec conf-qmail conf-sysconfdir conf-shared
	$(edit) $@.in > $@
indimail-mta-permissions.secure: indimail-mta-permissions.secure.in \
conf-prefix conf-libexec conf-qmail conf-sysconfdir conf-shared
	$(edit) $@.in > $@
indimail-mta-permissions.paranoid: indimail-mta-permissions.paranoid.in \
conf-prefix conf-libexec conf-qmail conf-sysconfdir conf-shared
	$(edit) $@.in > $@
indimail-mta-release: indimail-mta-release.in conf-version conf-email
	$(edit) $@.in > $@
