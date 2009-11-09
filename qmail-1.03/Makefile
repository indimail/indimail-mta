# Don't edit Makefile! Use conf-* for configuration.
COFLAGS=-z+05:30
DKIM_VERSION=1.3
INDIDIR := $(shell head -1 conf-qmail 2>/dev/null|| echo "/var/indimail")
SYSTEM :=  $(shell uname -s)
SHELL=/bin/sh
inc_deps := -I$(DESTDIR)$(INDIDIR)/include `cat ./inc_deps 2>/dev/null`
lib_deps := -L$(DESTDIR)$(INDIDIR)/lib     `cat ./lib_deps 2>/dev/null`

default: it man

all:it man

addresses.0: \
addresses.5
	nroff -man addresses.5 > addresses.0

libsyncdir.a: \
makelib syncdir.o
	./makelib libsyncdir.a syncdir.o

syncdir.o: \
compile syncdir.c env.h conf-sync
	./compile `grep -h -v "^#" conf-sync` syncdir.c

alloc.a: \
makelib alloc.o alloc_re.o
	./makelib alloc.a alloc.o alloc_re.o

alloc.o: \
compile alloc.c alloc.h error.h
	./compile alloc.c

alloc_re.o: \
compile alloc_re.c alloc.h byte.h
	./compile alloc_re.c

auto-ccld.sh: conf-cc conf-ld warn-auto.sh
	(cat warn-auto.sh; echo CC=\'`head -1 conf-cc`\'; \
	if test $(SYSTEM) = Darwin ; then \
		echo LD=\'`head -1 conf-ld | sed s{-s{{`\'; \
	else \
		echo LD=\'`head -1 conf-ld`\'; \
	fi) > auto-ccld.sh

auto-gid: \
load auto-gid.o substdio.a error.a str.a fs.a
	./load auto-gid substdio.a error.a str.a fs.a 

auto-gid.o: \
compile auto-gid.c subfd.h substdio.h exit.h \
scan.h fmt.h
	./compile auto-gid.c

auto-int: \
load auto-int.o substdio.a error.a str.a fs.a
	./load auto-int substdio.a error.a str.a fs.a 

auto-int.o: \
compile auto-int.c substdio.h exit.h scan.h fmt.h
	./compile auto-int.c

auto-int8: \
load auto-int8.o substdio.a error.a str.a fs.a
	./load auto-int8 substdio.a error.a str.a fs.a 

auto-int8.o: \
compile auto-int8.c substdio.h exit.h scan.h fmt.h
	./compile auto-int8.c

auto-str: \
load auto-str.o substdio.a error.a str.a
	./load auto-str substdio.a error.a str.a 

auto-str.o: \
compile auto-str.c substdio.h exit.h
	./compile auto-str.c

auto-uid: \
load auto-uid.o substdio.a error.a str.a fs.a
	./load auto-uid substdio.a error.a str.a fs.a 

auto-uid.o: \
compile auto-uid.c subfd.h substdio.h exit.h \
scan.h fmt.h
	./compile auto-uid.c

auto_break.c: \
auto-str conf-break
	./auto-str auto_break \
	"`head -1 conf-break`" > auto_break.c

auto_break.o: \
compile auto_break.c
	./compile auto_break.c

auto_patrn.c: \
auto-int8 conf-patrn
	./auto-int8 auto_patrn `head -1 conf-patrn` > auto_patrn.c

auto_patrn.o: \
compile auto_patrn.c
	./compile auto_patrn.c

auto_qmail.c: \
auto-str conf-qmail
	./auto-str auto_qmail `head -1 conf-qmail` > auto_qmail.c

auto_qmail.o: \
compile auto_qmail.c
	./compile auto_qmail.c

auto_spawn.c: \
auto-int conf-spawn
	./auto-int auto_spawn `head -1 conf-spawn` > auto_spawn.c

auto_spawn.o: \
compile auto_spawn.c
	./compile auto_spawn.c

auto_split.c: \
auto-int conf-split
	./auto-int auto_split `head -1 conf-split` > auto_split.c

auto_split.o: \
compile auto_split.c
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
	echo "#endif"; \
	) > auto_uids.h
auto_uids.c: \
auto-uid auto-gid conf-users conf-groups
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

auto_uids.o: \
compile auto_uids.c
	./compile auto_uids.c

auto_usera.c: \
auto-str conf-users
	./auto-str auto_usera `head -1 conf-users` > auto_usera.c

auto_usera.o: \
compile auto_usera.c
	./compile auto_usera.c

base64.o: \
compile base64.c base64.h stralloc.h substdio.h str.h
	./compile base64.c

binm1: \
binm1.sh conf-qmail
	cat binm1.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm1

binm1+df: \
binm1+df.sh conf-qmail
	cat binm1+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm1+df

binm2: \
binm2.sh conf-qmail
	cat binm2.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm2

binm2+df: \
binm2+df.sh conf-qmail
	cat binm2+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm2+df

binm3: \
binm3.sh conf-qmail
	cat binm3.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm3

binm3+df: \
binm3+df.sh conf-qmail
	cat binm3+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> binm3+df

bouncesaying: \
load bouncesaying.o strerr.a error.a substdio.a str.a wait.a
	./load bouncesaying strerr.a error.a substdio.a str.a \
	wait.a 

bouncesaying.0: \
bouncesaying.1
	nroff -man bouncesaying.1 > bouncesaying.0

bouncesaying.o: \
compile bouncesaying.c strerr.h error.h wait.h sig.h exit.h
	./compile bouncesaying.c

byte_chr.o: \
compile byte_chr.c byte.h
	./compile byte_chr.c

byte_copy.o: \
compile byte_copy.c byte.h
	./compile byte_copy.c

byte_cr.o: \
compile byte_cr.c byte.h
	./compile byte_cr.c

byte_rcspn.o: \
compile byte_rcspn.c byte.h
	./compile byte_rcspn.c


byte_cspn.o: \
compile byte_cspn.c byte.h
	./compile byte_cspn.c


byte_diff.o: \
compile byte_diff.c byte.h
	./compile byte_diff.c

byte_rchr.o: \
compile byte_rchr.c byte.h
	./compile byte_rchr.c

byte_zero.o: \
compile byte_zero.c byte.h
	./compile byte_zero.c

case.a: \
makelib case_diffb.o case_diffs.o case_lowerb.o case_lowers.o \
case_starts.o case_startb.o
	./makelib case.a case_diffb.o case_diffs.o case_lowerb.o \
	case_lowers.o case_starts.o case_startb.o

case_diffb.o: \
compile case_diffb.c case.h
	./compile case_diffb.c

case_diffs.o: \
compile case_diffs.c case.h
	./compile case_diffs.c

case_lowerb.o: \
compile case_lowerb.c case.h
	./compile case_lowerb.c

case_lowers.o: \
compile case_lowers.c case.h
	./compile case_lowers.c

case_starts.o: \
compile case_starts.c case.h
	./compile case_starts.c

case_startb.o: \
compile case_startb.c case.h
	./compile case_startb.c

cdb.a: \
makelib cdb_hash.o cdb_unpack.o cdb_seek.o
	./makelib cdb.a cdb_hash.o cdb_unpack.o cdb_seek.o

cdb_hash.o: \
compile cdb_hash.c cdb.h uint32.h
	./compile cdb_hash.c

cdb_seek.o: \
compile cdb_seek.c cdb.h uint32.h
	./compile cdb_seek.c

cdb_unpack.o: \
compile cdb_unpack.c cdb.h uint32.h
	./compile cdb_unpack.c

cdbmake.a: \
makelib cdbmake_pack.o cdbmake_hash.o cdbmake_add.o
	./makelib cdbmake.a cdbmake_pack.o cdbmake_hash.o \
	cdbmake_add.o

cdbmake_add.o: \
compile cdbmake_add.c cdbmake.h uint32.h alloc.h
	./compile cdbmake_add.c

cdbmake_hash.o: \
compile cdbmake_hash.c cdbmake.h uint32.h
	./compile cdbmake_hash.c

cdbmake_pack.o: \
compile cdbmake_pack.c cdbmake.h uint32.h
	./compile cdbmake_pack.c

cdbmss.o: \
compile cdbmss.c seek.h alloc.h cdbmss.h cdbmake.h \
uint32.h substdio.h
	./compile cdbmss.c

check: \
it man
	./instcheck

chkshsgr: \
load chkshsgr.o
	./load chkshsgr 

chkshsgr.o: \
compile chkshsgr.c exit.h
	./compile chkshsgr.c

chkspawn: \
load chkspawn.o substdio.a error.a str.a fs.a auto_spawn.o
	./load chkspawn substdio.a error.a str.a fs.a auto_spawn.o 

chkspawn.o: \
compile chkspawn.c subfd.h substdio.h fmt.h select.h \
exit.h auto_spawn.h
	./compile chkspawn.c

clean: \
TARGETS
	rm -f `cat TARGETS`

coe.o: \
compile coe.c coe.h
	./compile coe.c

commands.o: \
compile commands.c commands.h substdio.h stralloc.h gen_alloc.h str.h \
case.h
	./compile commands.c

compile: \
make-compile warn-auto.sh systype
	( cat warn-auto.sh; ./make-compile "`cat systype`" ) > \
	compile
	chmod 755 compile

choose: choose.sh warn-auto.sh
	rm -f choose
	cat warn-auto.sh choose.sh > choose
	chmod 555 choose

condredirect: \
load condredirect.o qmail.o strerr.a fd.a sig.a wait.a seek.a env.a \
substdio.a error.a str.a fs.a auto_qmail.o alloc.a
	./load condredirect qmail.o strerr.a fd.a sig.a wait.a \
	seek.a env.a substdio.a error.a str.a fs.a auto_qmail.o alloc.a

condredirect.0: \
condredirect.1
	nroff -man condredirect.1 > condredirect.0

condredirect.o: \
compile condredirect.c sig.h exit.h env.h error.h \
wait.h seek.h qmail.h substdio.h strerr.h fmt.h
	./compile condredirect.c

config: \
warn-auto.sh config.sh conf-qmail conf-break conf-split
	cat warn-auto.sh config.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> config
	chmod 755 config

config-fast: \
warn-auto.sh config-fast.sh conf-qmail conf-break conf-split
	cat warn-auto.sh config-fast.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> config-fast
	chmod 755 config-fast

constmap.o: \
compile constmap.c constmap.h alloc.h case.h
	./compile constmap.c

control.o: \
compile control.c open.h getln.h stralloc.h gen_alloc.h \
substdio.h error.h control.h alloc.h scan.h env.h variables.h
	./compile control.c

date822fmt.o: \
compile date822fmt.c datetime.h fmt.h date822fmt.h
	./compile date822fmt.c

datemail: \
warn-auto.sh datemail.sh conf-qmail conf-break conf-split
	cat warn-auto.sh datemail.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> datemail

datetime.a: \
makelib datetime.o datetime_un.o
	./makelib datetime.a datetime.o datetime_un.o

datetime.o: \
compile datetime.c datetime.h
	./compile datetime.c

datetime_un.o: \
compile datetime_un.c datetime.h
	./compile datetime_un.c

direntry.h: \
compile trydrent.c direntry.h1 direntry.h2
	( ./compile trydrent.c >/dev/null 2>&1 \
	&& cat direntry.h2 || cat direntry.h1 ) > direntry.h
	rm -f trydrent.o

dns.lib: \
tryrsolv.c compile load socket.lib dns.o scan_xlong.o socket_v6any.o socket_v4mappedprefix.o \
ipalloc.o ip.o strsalloc.o socket.lib \
stralloc.a alloc.a error.a fs.a str.a
	( ( ./compile tryrsolv.c && ./load tryrsolv dns.o \
	scan_xlong.o socket_v6any.o socket_v4mappedprefix.o \
	ipalloc.o ip.o strsalloc.o stralloc.a alloc.a error.a fs.a str.a \
	-lresolv `cat socket.lib` ) >/dev/null 2>&1 \
	&& echo -lresolv || exit 0 ) > dns.lib
	rm -f tryrsolv.o tryrsolv

dns.o: \
compile dns.c ip.h ipalloc.h gen_alloc.h fmt.h alloc.h str.h \
strsalloc.h stralloc.h gen_alloc.h dns.h case.h byte.h conf-tls \
conf-spf conf-domainkeys conf-ip
	./compile `grep -h -v "^#" conf-tls conf-spf conf-domainkeys conf-ip` dns.c

dnscname: \
load dnscname.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnscname dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib socket.lib`

dnscname.o: \
compile dnscname.c substdio.h subfd.h stralloc.h \
gen_alloc.h dns.h dnsdoe.h exit.h
	./compile dnscname.c

dnsdoe.o: \
compile dnsdoe.c substdio.h subfd.h exit.h dns.h dnsdoe.h
	./compile dnsdoe.c

dnsfq: \
load dnsfq.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsfq dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib socket.lib`

dnsfq.o: \
compile dnsfq.c substdio.h subfd.h stralloc.h gen_alloc.h \
dns.h dnsdoe.h ip.h ipalloc.h strsalloc.h gen_alloc.h exit.h \
conf-tls conf-spf conf-ip
	./compile `grep -h -v "^#" conf-tls conf-spf conf-ip` dnsfq.c

dnsip: \
load dnsip.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsip dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib socket.lib`

dnsip.o: \
compile dnsip.c substdio.h subfd.h strsalloc.h stralloc.h gen_alloc.h \
dns.h dnsdoe.h ip.h ipalloc.h gen_alloc.h exit.h fmt.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` dnsip.c

dnsmxip: \
load dnsmxip.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o now.o \
scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsmxip dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o now.o \
	scan_xlong.o socket_v4mappedprefix.o socket_v6any.o \
	stralloc.a alloc.a substdio.a error.a str.a fs.a  `cat dns.lib socket.lib`

dnsmxip.o: \
compile dnsmxip.c subfd.h substdio.h stralloc.h \
gen_alloc.h fmt.h dns.h dnsdoe.h ip.h ipalloc.h strsalloc.h gen_alloc.h \
now.h datetime.h exit.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` dnsmxip.c

dnsptr: \
load dnsptr.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnsptr dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	scan_xlong.o socket_v4mappedprefix.o socket_v6any.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a  `cat dns.lib socket.lib`

dnsptr.o: \
compile dnsptr.c substdio.h subfd.h strsalloc.h stralloc.h gen_alloc.h \
str.h scan.h dns.h dnsdoe.h ip.h exit.h conf-spf
	./compile `grep -h -v "^#" conf-spf` dnsptr.c

dnstxt: \
load dnstxt.o dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
case_diffb.o case_diffs.o scan_xlong.o socket_v4mappedprefix.o socket_v6any.o \
dns_text.o dktrace.o domainkeys.o stralloc.a alloc.a \
substdio.a error.a str.a fs.a dns.lib socket.lib
	./load dnstxt dns.o dnsdoe.o ip.o ipalloc.o strsalloc.o \
	case_diffb.o case_diffs.o scan_xlong.o socket_v4mappedprefix.o \
	socket_v6any.o dns_text.o dktrace.o domainkeys.o stralloc.a \
	alloc.a substdio.a error.a str.a fs.a `cat dns.lib socket.lib` -lcrypto

dnstxt.o: \
compile dnstxt.c substdio.h subfd.h strsalloc.h stralloc.h gen_alloc.h \
str.h scan.h dns.h dnsdoe.h ip.h exit.h fmt.h conf-spf
	./compile `grep -h -v "^#" conf-spf conf-domainkeys` dnstxt.c


dot-qmail.0: \
dot-qmail.5
	nroff -man dot-qmail.5 > dot-qmail.0

dot-qmail.5: \
dot-qmail.9 conf-break
	cat dot-qmail.9 \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	> dot-qmail.5

elq: \
warn-auto.sh elq.sh conf-qmail conf-break conf-split
	cat warn-auto.sh elq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> elq

etrn: \
warn-auto.sh etrn.sh conf-qmail
	cat warn-auto.sh etrn.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> etrn

atrn: \
warn-auto.sh atrn.sh conf-qmail
	cat warn-auto.sh atrn.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> atrn

multirotate: \
warn-auto.sh multirotate.sh conf-qmail
	cat warn-auto.sh multirotate.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> multirotate

qmail-sighup: \
warn-auto.sh qmail-sighup.sh conf-qmail
	cat warn-auto.sh qmail-sighup.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-sighup

qmail-sigterm: \
warn-auto.sh qmail-sigterm.sh conf-qmail
	cat warn-auto.sh qmail-sigterm.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-sigterm

qmail-sigalrm: \
warn-auto.sh qmail-sigalrm.sh conf-qmail
	cat warn-auto.sh qmail-sigalrm.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-sigalrm

qpq: \
warn-auto.sh qpq.sh conf-qmail
	cat warn-auto.sh qpq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qpq

qmailctl: \
warn-auto.sh qmailctl.sh conf-qmail
	(head -1 warn-auto.sh;cat qmailctl.sh) \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmailctl

indimail.plist: indimail.plist.in conf-qmail
	cat indimail.plist.in | sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> indimail.plist

svscanboot: \
warn-auto.sh svscanboot.sh conf-qmail
	cat warn-auto.sh svscanboot.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> svscanboot

upstart:upstart.sh conf-qmail
	( \
	echo "# svscan - runlevel compatibility"; \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	cat upstart.sh | sed s}QMAIL}"`head -1 conf-qmail`"}g \
	) > upstart

svscanboot.0: \
svscanboot.8
	nroff -man svscanboot.8 > svscanboot.0

svscanboot.8: \
svscanboot.9
	cat svscanboot.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> svscanboot.8

env.a: \
makelib env.o envread.o
	./makelib env.a env.o envread.o

env.o: \
compile env.c str.h alloc.h env.h
	./compile env.c

envelopes.0: \
envelopes.5
	nroff -man envelopes.5 > envelopes.0

envread.o: \
compile envread.c env.h str.h
	./compile envread.c

error.a: \
makelib error.o error_str.o error_temp.o
	./makelib error.a error.o error_str.o error_temp.o

error.o: \
compile error.c error.h
	./compile error.c

error_str.o: \
compile error_str.c error.h
	./compile error_str.c

error_temp.o: \
compile error_temp.c error.h
	./compile error_temp.c

except: \
load except.o strerr.a error.a substdio.a str.a wait.a
	./load except strerr.a error.a substdio.a str.a wait.a 

except.0: \
except.1
	nroff -man except.1 > except.0

except.o: \
compile except.c strerr.h wait.h error.h exit.h
	./compile except.c

fd.a: \
makelib fd_copy.o fd_move.o
	./makelib fd.a fd_copy.o fd_move.o

fd_copy.o: \
compile fd_copy.c fd.h
	./compile fd_copy.c

fd_move.o: \
compile fd_move.c fd.h
	./compile fd_move.c

fifo.o: \
compile fifo.c hasmkffo.h fifo.h
	./compile fifo.c

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

fmt_str.o: \
compile fmt_str.c fmt.h
	./compile fmt_str.c

fmt_strn.o: \
compile fmt_strn.c fmt.h
	./compile fmt_strn.c

fmt_uint.o: \
compile fmt_uint.c fmt.h
	./compile fmt_uint.c

fmt_uint0.o: \
compile fmt_uint0.c fmt.h
	./compile fmt_uint0.c

fmt_ulong.o: \
compile fmt_ulong.c fmt.h
	./compile fmt_ulong.c

fmtqfn.o: \
compile fmtqfn.c fmtqfn.h fmt.h auto_split.h
	./compile fmtqfn.c

forgeries.0: \
forgeries.7
	nroff -man forgeries.7 > forgeries.0

forward: \
load forward.o qmail.o strerr.a alloc.a fd.a wait.a sig.a env.a \
substdio.a error.a str.a fs.a auto_qmail.o alloc.a
	./load forward qmail.o strerr.a alloc.a fd.a wait.a sig.a \
	env.a substdio.a error.a str.a fs.a auto_qmail.o alloc.a

forward.0: \
forward.1
	nroff -man forward.1 > forward.0

forward.o: \
compile forward.c sig.h exit.h env.h qmail.h \
strerr.h substdio.h fmt.h
	./compile forward.c

fs.a: \
makelib fmt_str.o fmt_strn.o fmt_uint.o fmt_uint0.o fmt_ulong.o \
scan_ulong.o scan_8long.o scan_ushort.o scan_uint.o fmt_ushort.o
	./makelib fs.a fmt_str.o fmt_strn.o fmt_uint.o fmt_uint0.o \
	fmt_ulong.o scan_ulong.o scan_8long.o scan_ushort.o scan_uint.o \
	fmt_ushort.o

getln.a: \
makelib getln.o getln2.o
	./makelib getln.a getln.o getln2.o

getln.o: \
compile getln.c substdio.h byte.h stralloc.h gen_alloc.h getln.h
	./compile getln.c

getln2.o: \
compile getln2.c substdio.h stralloc.h gen_alloc.h byte.h getln.h
	./compile getln2.c

getopt.a: \
makelib subgetopt.o sgetopt.o
	./makelib getopt.a subgetopt.o sgetopt.o

gfrom.o: \
compile gfrom.c str.h gfrom.h
	./compile gfrom.c

hasflock.h: \
tryflock.c compile load
	( ( ./compile tryflock.c && ./load tryflock ) >/dev/null \
	2>&1 \
	&& echo \#define HASFLOCK 1 || exit 0 ) > hasflock.h
	rm -f tryflock.o tryflock

hasmkffo.h: \
trymkffo.c compile load
	( ( ./compile trymkffo.c && ./load trymkffo ) >/dev/null \
	2>&1 \
	&& echo \#define HASMKFIFO 1 || exit 0 ) > hasmkffo.h
	rm -f trymkffo.o trymkffo

hasnpbg1.h: \
trynpbg1.c compile load open.h open.a fifo.h fifo.o select.h
	( ( ./compile trynpbg1.c \
	&& ./load trynpbg1 fifo.o open.a && ./trynpbg1 ) \
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

tai.h: uint64.h

hastai.h: \
trytai.c compile load
	(( ./compile trytai.c && ./load trytai -ltai) >/dev/null \
	2>&1 \
	&& echo \#define HASTAI 1 || exit 0 ) > hastai.h
	rm -f trytai.o trytai

haswaitp.h: \
trywaitp.c compile load
	( ( ./compile trywaitp.c && ./load trywaitp ) >/dev/null \
	2>&1 \
	&& echo \#define HASWAITPID 1 || exit 0 ) > haswaitp.h
	rm -f trywaitp.o trywaitp

headerbody.o: \
compile headerbody.c stralloc.h gen_alloc.h substdio.h getln.h \
hfield.h headerbody.h
	./compile headerbody.c

hfield.o: \
compile hfield.c hfield.h
	./compile hfield.c

hier.o: \
compile hier.c auto_qmail.h auto_split.h auto_uids.h fmt.h fifo.h \
qmail-todo.h hasindimail.h tcpto.h ip.h ipalloc.h \
conf-domainkeys conf-spf conf-ip conf-dkim conf-batv
	./compile $(inc_deps) \
	`grep -h -v "^#" conf-tls conf-spf conf-ip conf-domainkeys conf-dkim conf-batv` hier.c

home: \
home.sh conf-qmail
	cat home.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> home

home+df: \
home+df.sh conf-qmail
	cat home+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> home+df

hostname: \
load hostname.o substdio.a error.a str.a
	./load hostname substdio.a error.a str.a

hostname.o: \
compile hostname.c subfd.h substdio.h exit.h
	./compile hostname.c

idedit: \
load idedit.o strerr.a substdio.a error.a str.a fs.a wait.a open.a \
seek.a
	./load idedit strerr.a substdio.a error.a str.a fs.a \
	wait.a open.a seek.a 

idedit.o: \
compile idedit.c exit.h scan.h fmt.h strerr.h open.h \
seek.h wait.h
	./compile idedit.c

setup: \
load setup.o fifo.o hier.o auto_qmail.o auto_split.o auto_uids.o get_uid.o \
strerr.a substdio.a open.a error.a str.a fs.a
	./load setup fifo.o hier.o auto_qmail.o auto_split.o auto_uids.o get_uid.o \
	strerr.a substdio.a open.a error.a str.a fs.a 

setup.o: \
compile setup.c substdio.h strerr.h error.h open.h \
exit.h fifo.h subfd.h str.h auto_uids.h
	./compile setup.c

instcheck: \
load instcheck.o fifo.o hier.o auto_qmail.o get_uid.o auto_split.o auto_uids.o \
strerr.a substdio.a error.a str.a fs.a alloc.a
	./load instcheck fifo.o hier.o auto_qmail.o get_uid.o auto_split.o \
	auto_uids.o strerr.a substdio.a error.a str.a fs.a alloc.a

instcheck.o: \
compile instcheck.c strerr.h error.h exit.h
	./compile instcheck.c

ip.o: \
compile ip.c fmt.h scan.h ip.h byte.h conf-ip
	./compile `grep -h -v "^#" conf-ip` ip.c

ipalloc.o: \
compile ipalloc.c alloc.h gen_allocdefs.h ip.h ipalloc.h \
gen_alloc.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` ipalloc.c

ipme.o: \
compile ipme.c hassalen.h byte.h ip.h ipalloc.h gen_alloc.h \
open.h getln.h alloc.h stralloc.h gen_alloc.h ipme.h \
ipalloc.h substdio.h variables.h conf-tls conf-ip conf-ipme
	./compile `grep -h -v "^#" conf-tls conf-ip conf-ipme` ipme.c

ipmeprint: \
load ipmeprint.o ipme.o ip.o ipalloc.o strsalloc.o open_read.o \
auto_qmail.o scan_xlong.o socket_v4mappedprefix.o socket_v6any.o variables.o envread.o getln.o getln2.o stralloc.a \
alloc.a substdio.a error.a str.a fs.a socket.lib
	./load ipmeprint ipme.o ip.o ipalloc.o strsalloc.o open_read.o \
	auto_qmail.o scan_xlong.o socket_v4mappedprefix.o socket_v6any.o variables.o envread.o getln.o getln2.o \
	stralloc.a alloc.a substdio.a error.a str.a fs.a  `cat socket.lib`

ipmeprint.o: \
compile ipmeprint.c subfd.h substdio.h ipme.h ip.h \
ipalloc.h gen_alloc.h fmt.h exit.h auto_qmail.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` ipmeprint.c

viruscanner: \
qscanq cleanq run-cleanq qscanq-stdin qhpsi

qhpsi: \
load qhpsi.o byte_copy.o error.o error_str.o auto_qmail.o get_uid.o auto_uids.o \
env.a str.a stralloc.a alloc.a strerr.a substdio.a
	./load qhpsi byte_copy.o error.o error_str.o auto_qmail.o get_uid.o auto_uids.o \
	env.a str.a stralloc.a alloc.a strerr.a substdio.a -ldl

qhpsi.o: compile qhpsi.c strerr.h env.h str.h auto_qmail.h \
stralloc.h auto_uids.h
	./compile $(inc_deps) qhpsi.c

qscanq: load qscanq.o substdio.a strerr.a env.a str.a wait.a error.a \
timestamp.o tai.a fd.a auto_spool.o mkfn.o fmt_xpid.o do_cleanq.o \
auto_cleaner.o auto_qstdin.o sig.a alloc.a
	./load qscanq strerr.a env.a str.a wait.a error.a timestamp.o \
	tai.a fd.a auto_spool.o mkfn.o fmt_xpid.o do_cleanq.o auto_cleaner.o \
	auto_qstdin.o byte_copy.o sig.a substdio.a alloc.a

qscanq.o: compile qscanq.c auto_spool.h auto_fnlen.h mkfn.h \
auto_retries.h wait.h sig.h substdio.h strerr.h env.h exitcodes.h
	./compile qscanq.c

timestamp.o: compile timestamp.c taia.h timestamp.h tai.h uint64.h
	./compile timestamp.c

mkfn.o: compile mkfn.c fmt.h fmt_xpid.h timestamp.h auto_pidt.h
	./compile mkfn.c

do_cleanq.o: compile do_cleanq.c wait.h auto_cleaner.h
	./compile do_cleanq.c

auto_pidt.h: auto-pidt
	./auto-pidt > auto_pidt.h.tmp && mv auto_pidt.h.tmp auto_pidt.h

auto-pidt: \
load auto-pidt.o substdio.a str.a error.a fmt_ulong.o
	./load auto-pidt substdio.a str.a error.a fmt_ulong.o 

auto-pidt.o: compile auto-pidt.c exit.h scan.h fmt.h
	./compile auto-pidt.c

auto_spool.c: auto-str conf-scandir
	./auto-str auto_spool "`head -1 conf-scandir`"/root/scanq > \
	auto_spool.c.tmp && mv auto_spool.c.tmp auto_spool.c

auto_spool.o: compile auto_spool.c
	./compile auto_spool.c

auto-fnlen: load auto-fnlen.o substdio.a str.a error.a fmt_ulong.o
	./load auto-fnlen substdio.a str.a error.a fmt_ulong.o 

auto-fnlen.o: compile auto-fnlen.c substdio.h exit.h scan.h \
fmt.h timestamp.h auto_pidt.h
	./compile auto-fnlen.c

auto_fnlen.h: auto-fnlen
	./auto-fnlen > auto_fnlen.h.tmp && mv auto_fnlen.h.tmp auto_fnlen.h

auto_retries.h: conf-retries
	echo "#define MAX_RETRIES" "`head -1 < conf-retries`" > \
	auto_retries.h.tmp && mv auto_retries.h.tmp auto_retries.h

tai.a: makelib taia_now.o taia_pack.o tai_pack.o
	./makelib tai.a taia_now.o taia_pack.o tai_pack.o

taia_now.o: compile taia_now.c taia.h tai.h uint64.h
	./compile taia_now.c

taia_pack.o: compile taia_pack.c taia.h tai.h uint64.h
	./compile taia_pack.c

auto_cleaner.c: auto-str conf-qmail
	./auto-str auto_runcleaner "`head -1 conf-qmail`"/bin/run-cleanq >> \
	auto_cleaner.c.tmp && mv auto_cleaner.c.tmp auto_cleaner.c

auto_cleaner.o: compile auto_cleaner.c
	./compile auto_cleaner.c

auto_qstdin.c: auto-strarr conf-qmail
	./auto-strarr auto_qstdin "`head -1 conf-qmail`"/bin/qscanq-stdin > \
	auto_qstdin.c.tmp && mv auto_qstdin.c.tmp auto_qstdin.c

auto_qstdin.o: compile auto_qstdin.c
	./compile auto_qstdin.c

auto-strarr: load auto-strarr.o substdio.a error.a str.a
	./load auto-strarr substdio.a error.a str.a 

auto-strarr.o: compile auto-strarr.c substdio.h exit.h
	./compile auto-strarr.c

cleanq: load cleanq.o strerr.a error.a substdio.a str.a wait.a getopt.a
	./load cleanq strerr.a error.a substdio.a str.a wait.a getopt.a 

cleanq.o: compile cleanq.c substdio.h error.h strerr.h direntry.h \
auto_ageout.h wait.h sgetopt.h str.h subgetopt.h
	./compile cleanq.c

auto_ageout.h: conf-ageout
	echo "#define MAX_AGE" "`head -1 < conf-ageout`" > auto_ageout.h.tmp && \
	mv auto_ageout.h.tmp auto_ageout.h

run-cleanq: load run-cleanq.o auto_qmail.o
	./load run-cleanq auto_qmail.o

run-cleanq.o: compile run-cleanq.c auto_qmail.h
	./compile run-cleanq.c

MakeArgs.o: compile MakeArgs.c alloc.h str.h alloc.h stralloc.h
	./compile MakeArgs.c

qscanq-stdin: load qscanq-stdin.o auto_qmail.o strerr.a substdio.a env.a str.a \
timestamp.o tai.a fd.a auto_spool.o mkfn.o fmt_xpid.o open_excl.o MakeArgs.o \
open_read.o do_ripmime.o auto_ripmime_cmd.o do_scan.o auto_scancmd.o \
seek.a do_cleanq.o auto_cleaner.o qregex.o wildmat.o matchregex.o \
cdb_seek.o cdb_unpack.o cdb_hash.o control.o str_diff.o variables.o \
constmap.o case_lowerb.o byte_chr.o byte_rchr.o scan_ulong.o case_diffb.o \
alloc.a getln.a sig.a wait.a error.a stralloc.a
	./load qscanq-stdin auto_qmail.o timestamp.o MakeArgs.o matchregex.o \
	auto_spool.o mkfn.o fmt_xpid.o open_excl.o open_read.o do_ripmime.o \
	auto_ripmime_cmd.o do_scan.o auto_scancmd.o seek.a do_cleanq.o \
	auto_cleaner.o qregex.o wildmat.o control.o str_diff.o variables.o \
	cdb_seek.o cdb_unpack.o cdb_hash.o \
	constmap.o case_lowerb.o byte_chr.o byte_rchr.o scan_ulong.o case_diffb.o \
	env.a strerr.a substdio.a str.a tai.a fd.a \
	stralloc.a alloc.a getln.a sig.a wait.a error.a

qscanq-stdin.o: compile qscanq-stdin.c substdio.h exit.h open.h fd.h wait.h \
seek.h sig.h auto_ageout.h exitcodes.h mkfn.h auto_fnlen.h env.h strerr.h auto_qmail.h
	./compile qscanq-stdin.c

do_ripmime.o: compile do_ripmime.c wait.h sig.h exitcodes.h \
strerr.h
	./compile do_ripmime.c

do_scan.o: compile do_scan.c wait.h sig.h exitcodes.h strerr.h env.h hasindimail.h \
qregex.h control.h str.h auto_qmail.h scan.h variables.h
	./compile $(inc_deps) do_scan.c

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
qscanq.9
	cat qscanq.9 \
	| sed s}INDIDIR}"$(INDIDIR)"}g \
	> qscanq.8

qscanq.0: \
qscanq.8
	nroff -man qscanq.8 > qscanq.0

qscanq-stdin.0: \
qscanq-stdin.8
	nroff -man qscanq-stdin.8 > qscanq-stdin.0

cleanq.0: \
cleanq.8
	nroff -man cleanq.8 > cleanq.0

run-cleanq.0: \
run-cleanq.8
	nroff -man run-cleanq.8 > run-cleanq.0

run-cleanq.8: \
run-cleanq.9
	cat run-cleanq.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> run-cleanq.8

it: \
qmail-local qmail-lspawn qmail-getpw qmail-remote qmail-rspawn \
qmail-clean qmail-send qmail-start qmail-daemon splogger qmail-multi qmail-queue qmail-inject \
qmail-dk qmail-dkim predate datemail mailsubj qmail-upq qmail-showctl qmail-newu spawn-filter \
qmail-pw2u qmail-qread qmail-qstat qmail-tcpto qmail-tcpok qmail-cat qmail-poppass \
qmail-pop3d qmail-popbull qmail-popup qmail-qmqpc qmail-qmqpd qmail-qmtpd \
qmail-smtpd sendmail tcp-env qmail-newmrh qmail-recipients qmail-cdb config \
config-fast dnscname dnsptr dnsip dnsmxip dnsfq dnstxt hostname ipmeprint qreceipt \
qsmhook forward rrforward preline condredirect bouncesaying except qmail-todo \
queue-fix idedit setup instcheck rpmattr maildir2mbox qmaildirmake maildirwatch b64encode \
plainencode b64decode maildirdeliver qmail-autoresponder qmail-rm qail elq pinq etrn \
atrn qmailctl indimail.plist svscanboot upstart qmail-sighup qmail-sigalrm qmail-sigterm \
home home+df proc proc+df binm1 binm1+df binm2 binm2+df \
binm3 binm3+df spfquery multirotate qpq 822header qmail-nullqueue \
maildirserial serialsmtp serialqmtp maildirsmtp maildirqmtp \
serialcmd maildircmd rewind relaytest qmail-qfilter \
iftocc ofmipd ofmipname 822field new-inject 822date 822received 822print \
tokenize addrlist qquote parsedate qmail-greyd greydaemon \
analyzer qtools qlog viruscanner update_tmprsadh dknewkey \
dktest dot-forward qmail-qmHandle batv \
fastforward printforward setforward newaliases printmaillist \
setmaillist newinclude sortedtest logselect mbox2maildir \
envdir envuidgid fghack multilog pgrphack readproctitle setlock \
setuidgid softlimit supervise svc svok svscan svstat tai64n tai64nlocal tai64nunix \
recordio argv0 addcr delcr fixcrio generic.so cdbmake cdbget \
cdbdump cdbstats cdbtest cdbmake-12 cdbmake-sv testzero dk-filter

analyzer: \
mlmatchup matchup columnt zoverall zsendmail xqp xsender xrecipient ddist \
deferrals failures successes rhosts recipients rxdelay senders suids \
zddist zdeferrals zfailures zsuccesses zrhosts zrecipients zrxdelay \
zsenders zsuids zsmtp rsmtp rsmtpfailures rsmtpsdomains rsmtprdomains \
rsmtpsenders rsmtprecipients multilog-matchup smtp-matchup rspamrdomain \
rspamsdomain zspam envmigrate rspamstat rspamhist

qtools: \
822body 822headerok 822bodyfilter 822headerfilter 822addr \
822fields checkaddr checkdomain filterto \
condtomaildir iftoccfrom ifaddr replier replier-config

qlog: spipe qfilelog multipipe teepipe multitail

load: \
make-load warn-auto.sh systype
	( cat warn-auto.sh; ./make-load "`cat systype`" ) > load
	chmod 755 load

lock.a: \
makelib lock_ex.o lock_exnb.o lock_un.o
	./makelib lock.a lock_ex.o lock_exnb.o lock_un.o

lock_ex.o: \
compile lock_ex.c hasflock.h lock.h
	./compile lock_ex.c

lock_exnb.o: \
compile lock_exnb.c hasflock.h lock.h
	./compile lock_exnb.c

lock_un.o: \
compile lock_un.c hasflock.h lock.h
	./compile lock_un.c

maildir.0: \
maildir.5
	nroff -man maildir.5 > maildir.0

maildir.o: \
compile maildir.c prioq.h datetime.h gen_alloc.h env.h stralloc.h \
gen_alloc.h direntry.h datetime.h now.h datetime.h str.h maildir.h \
strerr.h
	./compile maildir.c

maildir2mbox: \
load maildir2mbox.o maildir.o prioq.o now.o myctime.o gfrom.o lock.a \
getln.a env.a open.a strerr.a stralloc.a alloc.a substdio.a error.a \
str.a fs.a datetime.a
	./load maildir2mbox maildir.o prioq.o now.o myctime.o \
	gfrom.o lock.a getln.a env.a open.a strerr.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a datetime.a 

maildir2mbox.0: \
maildir2mbox.1
	nroff -man maildir2mbox.1 > maildir2mbox.0

maildir2mbox.o: \
compile maildir2mbox.c prioq.h datetime.h gen_alloc.h \
env.h stralloc.h gen_alloc.h subfd.h substdio.h getln.h \
error.h open.h lock.h gfrom.h str.h exit.h myctime.h maildir.h \
strerr.h
	./compile maildir2mbox.c

mbox2maildir: load mbox2maildir.o error.o byte_copy.o str_len.o lock_ex.o fmt_ulong.o substdio.a
	./load mbox2maildir error.o byte_copy.o str_len.o lock_ex.o fmt_ulong.o substdio.a

mbox2maildir.o: compile mbox2maildir.c hasflock.h substdio.h fmt.h lock.h
	./compile mbox2maildir.c

qmaildirmake: \
load qmaildirmake.o strerr.a substdio.a error.a str.a
	./load qmaildirmake strerr.a substdio.a error.a str.a 

qmaildirmake.0: \
qmaildirmake.1
	nroff -man qmaildirmake.1 > qmaildirmake.0

qmaildirmake.o: \
compile qmaildirmake.c strerr.h exit.h
	./compile qmaildirmake.c

maildirwatch: \
load maildirwatch.o hfield.o headerbody.o maildir.o prioq.o now.o \
getln.a env.a open.a strerr.a stralloc.a alloc.a substdio.a error.a \
str.a
	./load maildirwatch hfield.o headerbody.o maildir.o \
	prioq.o now.o getln.a env.a open.a strerr.a stralloc.a \
	alloc.a substdio.a error.a str.a 

maildirwatch.0: \
maildirwatch.1
	nroff -man maildirwatch.1 > maildirwatch.0

maildirwatch.o: \
compile maildirwatch.c getln.h subfd.h substdio.h prioq.h \
datetime.h gen_alloc.h stralloc.h gen_alloc.h str.h exit.h hfield.h \
open.h headerbody.h maildir.h strerr.h
	./compile maildirwatch.c

mailsubj: \
warn-auto.sh mailsubj.sh conf-qmail conf-break conf-split
	cat warn-auto.sh mailsubj.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> mailsubj

mailsubj.0: \
mailsubj.1
	nroff -man mailsubj.1 > mailsubj.0

make-compile: \
make-compile.sh auto-ccld.sh
	cat auto-ccld.sh make-compile.sh > make-compile
	chmod 755 make-compile

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

man: \
qmail-local.0 qmail-lspawn.0 qmail-getpw.0 qmail-remote.0 \
qmail-rspawn.0 qmail-clean.0 qmail-send.0 qmail-todo.0 qmail-start.0 splogger.0 \
qmail-queue.0 qmail-dk.0 qmail-dkim.8 dk-filter.8 qmail-inject.0 mailsubj.0 \
qmail-showctl.0 qmail-newu.0 \
qmail-pw2u.0 qmail-qread.0 qmail-qstat.0 qmail-tcpto.0 qmail-tcpok.0 \
qmail-pop3d.0 qmail-popbull.0 qmail-popup.0 qmail-qmqpc.0 \
qmail-qmqpd.0 qmail-qmtpd.0 qmail-smtpd.0 greydaemon.0 qmail-poppass.0 dktest.8 svscanboot.0 \
qmail-multi.0 spawn-filter.0 tcp-env.0 qmail-greyd.0 \
qmail-newmrh.0 qmail-recipients.0 qmail-cdb.0 qreceipt.0 forward.0 \
rrforward.0 preline.0 condredirect.0 bouncesaying.0 except.0 \
maildir2mbox.0 maildirwatch.0 qmail.0 qmail-limits.0 qmail-log.0 \
qmail-control.0 qmail-header.0 qmail-users.0 dot-qmail.0 \
qmail-command.0 tcp-environ.0 maildir.0 mbox.0 addresses.0 \
envelopes.0 forgeries.0 qmail-autoresponder.0 \
qmail-rm.0 maildirdeliver.0 822header.0 \
matchup.0 mlmatchup.0 columnt.0 xqp.0 xsender.0 xrecipient.0 \
serialcmd.0 maildircmd.0 rewind.0 \
maildirserial.0 serialsmtp.0 serialqmtp.0 maildirsmtp.0 \
maildirqmtp.0 qmail-qfilter.0 \
caldate.0 caldate_mjd.0 caltime.0 caltime_tai.0 \
config.0 leapsecs.0 mess822_date.0 tai.0 tai_now.0 tai_pack.0 \
iftocc.0 ofmipd.0 ofmipname.0 822field.0 new-inject.0 rewriting.0 \
rewritehost.0 822date.0 822received.0 822print.0 \
822body.0 822headerok.0 822bodyfilter.0 822headerfilter.0 822addr.0 \
822fields.0 checkaddr.0 checkdomain.0 filterto.0 \
condtomaildir.0 iftoccfrom.0 ifaddr.0 replier.0 replier-config.0 qscanq.0 \
qscanq-stdin.0 cleanq.0 run-cleanq.0 dot-forward.0 \
fastforward.0 printforward.0 setforward.0 newaliases.0 \
printmaillist.0 setmaillist.0 newinclude.0 qmaildirmake.0

mbox.0: \
mbox.5
	nroff -man mbox.5 > mbox.0

myctime.o: \
compile myctime.c datetime.h fmt.h myctime.h
	./compile myctime.c

ndelay.a: \
makelib ndelay.o ndelay_off.o
	./makelib ndelay.a ndelay.o ndelay_off.o

ndelay.o: \
compile ndelay.c ndelay.h
	./compile ndelay.c

ndelay_off.o: \
compile ndelay_off.c ndelay.h
	./compile ndelay_off.c

newfield.o: \
compile newfield.c fmt.h datetime.h stralloc.h gen_alloc.h \
date822fmt.h newfield.h stralloc.h
	./compile newfield.c

now.o: \
compile now.c datetime.h now.h datetime.h
	./compile now.c

open.a: \
makelib open_append.o open_excl.o open_read.o open_trunc.o \
open_write.o
	./makelib open.a open_append.o open_excl.o open_read.o \
	open_trunc.o open_write.o

open_append.o: \
compile open_append.c open.h
	./compile open_append.c

open_excl.o: \
compile open_excl.c open.h
	./compile open_excl.c

open_read.o: \
compile open_read.c open.h
	./compile open_read.c

open_trunc.o: \
compile open_trunc.c open.h
	./compile open_trunc.c

open_write.o: \
compile open_write.c open.h
	./compile open_write.c

pinq: \
warn-auto.sh pinq.sh conf-qmail conf-break conf-split
	cat warn-auto.sh pinq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> pinq

predate: \
load predate.o datetime.a strerr.a sig.a fd.a wait.a substdio.a \
error.a str.a fs.a
	./load predate datetime.a strerr.a sig.a fd.a wait.a \
	substdio.a error.a str.a fs.a 

predate.o: \
compile predate.c datetime.h wait.h fd.h fmt.h strerr.h \
subfd.h substdio.h exit.h sig.h
	./compile predate.c

preline: \
load preline.o strerr.a fd.a wait.a sig.a env.a getopt.a substdio.a \
error.a str.a alloc.a
	./load preline strerr.a fd.a wait.a sig.a env.a getopt.a \
	substdio.a error.a str.a alloc.a

preline.0: \
preline.1
	nroff -man preline.1 > preline.0

preline.o: \
compile preline.c fd.h sgetopt.h subgetopt.h strerr.h \
substdio.h exit.h wait.h env.h sig.h error.h
	./compile preline.c

prioq.o: \
compile prioq.c alloc.h gen_allocdefs.h prioq.h datetime.h \
gen_alloc.h
	./compile prioq.c

proc: \
proc.sh conf-qmail
	cat proc.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> proc

proc+df: \
proc+df.sh conf-qmail
	cat proc+df.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> proc+df

prot.o: \
compile prot.c hasshsgr.h prot.h
	./compile prot.c

qail: \
warn-auto.sh qail.sh conf-qmail conf-break conf-split
	cat warn-auto.sh qail.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qail

qmail-clean: \
load qmail-clean.o fmtqfn.o now.o getln.a sig.a stralloc.a alloc.a \
substdio.a error.a str.a fs.a auto_qmail.o auto_split.o envread.o str_diffn.o variables.o
	./load qmail-clean fmtqfn.o now.o getln.a sig.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a auto_qmail.o \
	auto_split.o envread.o str_diffn.o variables.o

qmail-clean.0: \
qmail-clean.8
	nroff -man qmail-clean.8 > qmail-clean.0

qmail-clean.o: \
compile qmail-clean.c sig.h now.h datetime.h str.h \
direntry.h getln.h stralloc.h gen_alloc.h subfd.h \
substdio.h byte.h scan.h fmt.h error.h exit.h fmtqfn.h auto_qmail.h env.h variables.h hasindimail.h

	./compile $(inc_deps) qmail-clean.c

qmail-command.0: \
qmail-command.8
	nroff -man qmail-command.8 > qmail-command.0

qmail-control.0: \
qmail-control.5
	nroff -man qmail-control.5 > qmail-control.0

qmail-control.5: \
qmail-control.9 conf-qmail
	cat qmail-control.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-control.5

qmail-getpw: \
load qmail-getpw.o case.a substdio.a error.a str.a fs.a auto_break.o \
auto_usera.o
	./load qmail-getpw case.a substdio.a error.a str.a fs.a \
	auto_break.o auto_usera.o 

qmail-getpw.0: \
qmail-getpw.8
	nroff -man qmail-getpw.8 > qmail-getpw.0

qmail-getpw.8: \
qmail-getpw.9 conf-break
	cat qmail-getpw.9 \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	> qmail-getpw.8

qmail-getpw.o: \
compile qmail-getpw.c subfd.h substdio.h \
error.h exit.h byte.h str.h case.h fmt.h auto_usera.h auto_break.h \
qlx.h
	./compile qmail-getpw.c

qmail-header.0: \
qmail-header.5
	nroff -man qmail-header.5 > qmail-header.0

qmail-inject: \
load qmail-inject.o headerbody.o hfield.o newfield.o quote.o now.o \
control.o date822fmt.o constmap.o qmail.o envrules.o wildmat.o \
matchregex.o cdb_seek.o cdb_unpack.o cdb_hash.o case.a \
fd.a wait.a open.a getln.a sig.a getopt.a datetime.a token822.o env.a \
stralloc.a alloc.a substdio.a error.a str.a fs.a auto_qmail.o variables.o \
dns.lib
	./load qmail-inject headerbody.o hfield.o newfield.o \
	quote.o now.o control.o date822fmt.o constmap.o qmail.o \
	matchregex.o cdb_seek.o cdb_unpack.o cdb_hash.o envrules.o \
	wildmat.o case.a fd.a wait.a open.a getln.a sig.a getopt.a datetime.a \
	token822.o env.a stralloc.a alloc.a substdio.a error.a \
	str.a fs.a auto_qmail.o variables.o `cat dns.lib`

qmail-inject.0: \
qmail-inject.8
	nroff -man qmail-inject.8 > qmail-inject.0

qmail-inject.8: qmail-inject.9
	cat qmail-inject.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-inject.8

qmail-inject.o: \
compile qmail-inject.c sig.h substdio.h stralloc.h gen_alloc.h \
subfd.h sgetopt.h subgetopt.h getln.h alloc.h str.h fmt.h \
hfield.h token822.h control.h env.h gen_allocdefs.h error.h \
qmail.h now.h datetime.h exit.h quote.h headerbody.h \
auto_qmail.h newfield.h constmap.h envrules.h scan.h
	./compile qmail-inject.c

envrules.o: \
compile envrules.c envrules.h env.h stralloc.h str.h \
matchregex.o control.h qregex.h error.h
	./compile envrules.c

tablematch.o: \
compile tablematch.c tablematch.h env.h stralloc.h str.h \
control.h
	./compile tablematch.c

bodycheck.o: \
compile bodycheck.c bodycheck.h stralloc.h str.h substdio.h alloc.h
	./compile bodycheck.c

qmail-limits.0: \
qmail-limits.7
	nroff -man qmail-limits.7 > qmail-limits.0

qmail-limits.7: \
qmail-limits.9 conf-spawn
	cat qmail-limits.9 \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-limits.7

qmail-local: \
load qmail-local.o qmail.o quote.o now.o gfrom.o myctime.o \
slurpclose.o case.a getln.a getopt.a sig.a open.a seek.a lock.a fd.a \
wait.a env.a stralloc.a alloc.a strerr.a substdio.a error.a str.a \
fs.a datetime.a auto_qmail.o auto_patrn.o case_diffb.o \
variables.o control.o constmap.o libsyncdir.a socket.lib
	./load qmail-local qmail.o quote.o now.o gfrom.o myctime.o \
	slurpclose.o case.a getln.a getopt.a sig.a open.a seek.a \
	lock.a fd.a wait.a env.a stralloc.a alloc.a strerr.a \
	substdio.a error.a str.a fs.a datetime.a auto_qmail.o \
	auto_patrn.o case_diffb.o variables.o \
	control.o constmap.o `cat socket.lib` -L. -lsyncdir

qmail-local.0: \
qmail-local.8
	nroff -man qmail-local.8 > qmail-local.0

qmail-local.o: \
compile qmail-local.c sig.h env.h byte.h exit.h \
open.h wait.h lock.h seek.h getln.h strerr.h subfd.h \
sgetopt.h subgetopt.h alloc.h error.h stralloc.h \
gen_alloc.h fmt.h str.h now.h datetime.h case.h quote.h qmail.h \
substdio.h slurpclose.h myctime.h gfrom.h auto_patrn.h constmap.h control.h auto_qmail.h variables.h \
conf-sync
	./compile `grep -h -v "^#" conf-sync` qmail-local.c

qmail-log.0: \
qmail-log.5
	nroff -man qmail-log.5 > qmail-log.0

qmail-lspawn.o: \
compile qmail-lspawn.c fd.h wait.h prot.h substdio.h stralloc.h \
gen_alloc.h scan.h exit.h error.h cdb.h uint32.h case.h open.h byte.h str.h \
slurpclose.h auto_qmail.h auto_uids.h qlx.h env.h hasindimail.h
	./compile $(inc_deps) qmail-lspawn.c

qmail-lspawn: \
load qmail-lspawn.o spawn.o prot.o slurpclose.o coe.o get_uid.o sig.a wait.a \
strerr.a case.a cdb.a fd.a open.a stralloc.a alloc.a substdio.a error.a str.a \
fs.a auto_qmail.o auto_uids.o auto_spawn.o env.o str_diffn.o str_cpy.o \
envread.o variables.o indimail.lib dns.lib
	./load qmail-lspawn spawn.o prot.o slurpclose.o coe.o auto_uids.o get_uid.o \
	auto_qmail.o auto_spawn.o env.o str_diffn.o str_cpy.o \
	envread.o variables.o \
	sig.a wait.a case.a cdb.a fd.a open.a stralloc.a alloc.a \
	strerr.a substdio.a error.a str.a fs.a \
	`cat indimail.lib` `cat dns.lib`

qmail-lspawn.0: \
qmail-lspawn.8
	nroff -man qmail-lspawn.8 > qmail-lspawn.0

qmail-newmrh: \
load qmail-newmrh.o cdbmss.o getln.a open.a cdbmake.a seek.a case.a \
stralloc.a alloc.a strerr.a substdio.a error.a str.a auto_qmail.o envread.o str_diffn.o variables.o
	./load qmail-newmrh cdbmss.o getln.a open.a cdbmake.a \
	seek.a case.a stralloc.a alloc.a strerr.a substdio.a \
	error.a str.a auto_qmail.o envread.o str_diffn.o variables.o

qmail-newmrh.0: \
qmail-newmrh.8
	nroff -man qmail-newmrh.8 > qmail-newmrh.0

qmail-newmrh.8: \
qmail-newmrh.9 conf-qmail
	cat qmail-newmrh.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-newmrh.8

qmail-newmrh.o: \
compile qmail-newmrh.c strerr.h stralloc.h gen_alloc.h \
getln.h exit.h open.h auto_qmail.h cdbmss.h cdbmake.h \
uint32.h substdio.h env.h variables.h case.h
	./compile qmail-newmrh.c

qmail-cdb: \
load qmail-cdb.o cdbmss.o getln.a open.a cdbmake.a seek.a case.a \
stralloc.a alloc.a strerr.a substdio.a error.a str.a auto_qmail.o envread.o str_diffn.o \
variables.o
	./load qmail-cdb cdbmss.o getln.a open.a cdbmake.a \
	seek.a case.a stralloc.a alloc.a strerr.a substdio.a \
	error.a str.a auto_qmail.o envread.o str_diffn.o variables.o

qmail-cdb.0: \
qmail-cdb.8
	nroff -man qmail-cdb.8 > qmail-cdb.0

qmail-cdb.8: \
qmail-cdb.9 conf-break conf-spawn
	cat qmail-cdb.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-cdb.8

qmail-cdb.o: \
compile qmail-cdb.c strerr.h stralloc.h gen_alloc.h substdio.h \
getln.h exit.h open.h auto_qmail.h cdbmss.h cdbmake.h \
uint32.h env.h variables.h case.h
	./compile qmail-cdb.c

qmail-recipients: \
load qmail-recipients.o cdbmss.o getln.a open.a cdbmake.a seek.a case.a \
stralloc.a alloc.a strerr.a substdio.a error.a str.a auto_qmail.o
	./load qmail-recipients cdbmss.o getln.a open.a cdbmake.a \
	seek.a case.a stralloc.a alloc.a strerr.a substdio.a \
	error.a str.a auto_qmail.o 

qmail-recipients.0: \
qmail-recipients.8
	nroff -man qmail-recipients.8 > qmail-recipients.0

qmail-recipients.8: \
qmail-recipients.9 conf-break conf-spawn
	cat qmail-recipients.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-recipients.8

qmail-recipients.o: \
compile qmail-recipients.c strerr.h stralloc.h gen_alloc.h substdio.h \
getln.h exit.h open.h auto_qmail.h cdbmss.h cdbmake.h \
uint32.h substdio.h case.h
	./compile qmail-recipients.c

qmail-newu: \
load qmail-newu.o cdbmss.o getln.a open.a seek.a cdbmake.a case.a \
stralloc.a alloc.a substdio.a error.a str.a auto_qmail.o
	./load qmail-newu cdbmss.o getln.a open.a seek.a cdbmake.a \
	case.a stralloc.a alloc.a substdio.a error.a str.a \
	auto_qmail.o 

qmail-newu.0: \
qmail-newu.8
	nroff -man qmail-newu.8 > qmail-newu.0

qmail-newu.8: \
qmail-newu.9 conf-break conf-spawn
	cat qmail-newu.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-newu.8

qmail-newu.o: \
compile qmail-newu.c stralloc.h gen_alloc.h subfd.h substdio.h \
getln.h cdbmss.h cdbmake.h uint32.h exit.h \
open.h error.h case.h auto_qmail.h byte.h
	./compile qmail-newu.c

qmail-pop3d: \
load qmail-pop3d.o commands.o case.a timeoutread.o timeoutwrite.o \
maildir.o prioq.o now.o env.a strerr.a sig.a open.a getln.a \
stralloc.a alloc.a substdio.a error.a str.a fs.a socket.lib
	./load qmail-pop3d commands.o case.a timeoutread.o \
	maildir.o timeoutwrite.o prioq.o now.o env.a strerr.a sig.a \
	open.a getln.a stralloc.a alloc.a substdio.a error.a str.a \
	fs.a  `cat socket.lib`

qmail-pop3d.0: \
qmail-pop3d.8
	nroff -man qmail-pop3d.8 > qmail-pop3d.0

qmail-pop3d.o: \
compile qmail-pop3d.c commands.h sig.h getln.h stralloc.h gen_alloc.h \
substdio.h alloc.h open.h prioq.h datetime.h gen_alloc.h scan.h fmt.h \
str.h exit.h maildir.h strerr.h timeoutread.h \
timeoutwrite.h
	./compile qmail-pop3d.c

qmail-popbull: \
load qmail-popbull.o now.o substdio.a stralloc.a alloc.a str.a error.a open.a
	./load qmail-popbull now.o substdio.a stralloc.a alloc.a fs.a str.a error.a open.a

qmail-popbull.0: \
qmail-popbull.8
	nroff -man qmail-popbull.8 > qmail-popbull.0

qmail-popbull.o: \
compile qmail-popbull.c direntry.h substdio.h stralloc.h subfd.h \
open.h fmt.h error.h datetime.h now.h str.h
	./compile qmail-popbull.c

qmail-popup: \
load qmail-popup.o commands.o timeoutread.o timeoutwrite.o now.o \
case.a fd.a sig.a wait.a stralloc.a alloc.a substdio.a error.a str.a \
fs.a socket.lib
	./load qmail-popup commands.o timeoutread.o timeoutwrite.o \
	now.o case.a fd.a sig.a wait.a stralloc.a alloc.a \
	substdio.a error.a str.a fs.a  `cat socket.lib`

qmail-popup.0: \
qmail-popup.8
	nroff -man qmail-popup.8 > qmail-popup.0

qmail-popup.o: \
compile qmail-popup.c commands.h fd.h sig.h stralloc.h gen_alloc.h \
substdio.h alloc.h wait.h str.h byte.h now.h datetime.h fmt.h exit.h \
timeoutread.h timeoutwrite.h
	./compile qmail-popup.c

qmail-pw2u: \
load qmail-pw2u.o constmap.o control.o open.a getln.a case.a getopt.a \
stralloc.a alloc.a substdio.a error.a str.a fs.a auto_usera.o \
auto_break.o auto_qmail.o envread.o str_diffn.o variables.o
	./load qmail-pw2u constmap.o control.o open.a getln.a \
	case.a getopt.a stralloc.a alloc.a substdio.a error.a str.a \
	fs.a auto_usera.o auto_break.o auto_qmail.o envread.o str_diffn.o variables.o

qmail-pw2u.0: \
qmail-pw2u.8
	nroff -man qmail-pw2u.8 > qmail-pw2u.0

qmail-pw2u.8: \
qmail-pw2u.9 conf-break conf-spawn
	cat qmail-pw2u.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-pw2u.8

qmail-pw2u.o: \
compile qmail-pw2u.c substdio.h subfd.h \
sgetopt.h subgetopt.h control.h constmap.h stralloc.h gen_alloc.h \
fmt.h str.h scan.h open.h error.h getln.h auto_break.h auto_qmail.h \
auto_usera.h byte.h
	./compile qmail-pw2u.c

qmail-qmqpc: \
load qmail-qmqpc.o slurpclose.o timeoutread.o timeoutwrite.o \
constmap.o case_diffb.o timeoutconn.o ip.o control.o auto_qmail.o \
socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
scan_xlong.o socket_v4mappedprefix.o socket_tcp.o socket_tcp6.o \
envread.o str_diffn.o variables.o sig.a ndelay.a open.a \
getln.a substdio.a stralloc.a alloc.a error.a str.a fs.a socket.lib
	./load qmail-qmqpc slurpclose.o timeoutread.o \
	constmap.o case_diffb.o socket_tcp.o socket_tcp6.o \
	socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
	scan_xlong.o socket_v4mappedprefix.o \
	timeoutwrite.o timeoutconn.o ip.o control.o \
	auto_qmail.o envread.o str_diffn.o variables.o \
	sig.a ndelay.a open.a getln.a substdio.a stralloc.a alloc.a \
	error.a str.a fs.a  `cat socket.lib`

qmail-qmqpc.0: \
qmail-qmqpc.8
	nroff -man qmail-qmqpc.8 > qmail-qmqpc.0

qmail-qmqpc.o: \
compile qmail-qmqpc.c substdio.h getln.h exit.h \
stralloc.h gen_alloc.h slurpclose.h error.h sig.h ip.h timeoutconn.h \
timeoutread.h timeoutwrite.h auto_qmail.h control.h fmt.h str.h \
haveip6.h socket.h conf-ip
	./compile `grep -h -v "^#" conf-ip` qmail-qmqpc.c

qmail-qmqpd: \
load qmail-qmqpd.o received.o now.o date822fmt.o qmail.o auto_qmail.o \
env.a substdio.a sig.a error.a wait.a fd.a str.a datetime.a fs.a alloc.a
	./load qmail-qmqpd received.o now.o date822fmt.o qmail.o \
	auto_qmail.o env.a substdio.a sig.a error.a wait.a fd.a \
	str.a datetime.a fs.a alloc.a

qmail-qmqpd.0: \
qmail-qmqpd.8
	nroff -man qmail-qmqpd.8 > qmail-qmqpd.0

qmail-qmqpd.o: \
compile qmail-qmqpd.c auto_qmail.h qmail.h substdio.h received.h \
sig.h exit.h now.h datetime.h fmt.h env.h byte.h str.h
	./compile qmail-qmqpd.c

qmail-qmtpd: \
load qmail-qmtpd.o rcpthosts.o control.o constmap.o received.o \
date822fmt.o now.o qmail.o cdb.a fd.a wait.a datetime.a open.a \
getln.a sig.a case.a env.a stralloc.a alloc.a substdio.a error.a \
str.a fs.a auto_qmail.o variables.o
	./load qmail-qmtpd rcpthosts.o control.o constmap.o \
	received.o date822fmt.o now.o qmail.o cdb.a fd.a wait.a \
	datetime.a open.a getln.a sig.a case.a env.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a auto_qmail.o variables.o

qmail-qmtpd.0: \
qmail-qmtpd.8
	nroff -man qmail-qmtpd.8 > qmail-qmtpd.0

qmail-qmtpd.o: \
compile qmail-qmtpd.c stralloc.h gen_alloc.h substdio.h qmail.h \
now.h datetime.h str.h fmt.h env.h sig.h rcpthosts.h \
auto_qmail.h control.h received.h scan.h recipients.h
	./compile qmail-qmtpd.c

qmail-qread: \
load qmail-qread.o fmtqfn.o readsubdir.o date822fmt.o datetime.a \
open.a getln.a stralloc.a alloc.a substdio.a error.a str.a fs.a \
auto_qmail.o auto_split.o envread.o str_diffn.o variables.o control.o \
stralloc_copy.o stralloc_cat.o
	./load qmail-qread fmtqfn.o readsubdir.o date822fmt.o \
	control.o stralloc_copy.o stralloc_cat.o str_diffn.o \
	auto_qmail.o auto_split.o envread.o variables.o \
	datetime.a open.a getln.a stralloc.a alloc.a substdio.a \
	error.a str.a fs.a 

qmail-qread.0: \
qmail-qread.8
	nroff -man qmail-qread.8 > qmail-qread.0

qmail-qread.8: \
qmail-qread.9
	cat qmail-qread.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-qread.8

qmail-qread.o: \
compile qmail-qread.c stralloc.h gen_alloc.h substdio.h subfd.h \
fmt.h str.h getln.h fmtqfn.h readsubdir.h direntry.h \
control.h auto_qmail.h open.h datetime.h date822fmt.h error.h \
scan.h exit.h env.h variables.h hasindimail.h conf-queue
	if test -f conf-queue ; then \
	./compile $(inc_deps) -DQUEUE_COUNT=`cat conf-queue` qmail-qread.c; \
	else \
	./compile $(inc_deps) -DQUEUE_COUNT=10 qmail-qread.c; \
	fi;

qmail-qstat: \
load qmail-qstat.o scan_ulong.o auto_qmail.o
	./load qmail-qstat scan_ulong.o auto_qmail.o

qmail-qstat.o: \
compile qmail-qstat.c auto_qmail.h scan.h conf-queue
	if test -f conf-queue ; then \
	./compile $(inc_deps) -DQUEUE_COUNT=`cat conf-queue` qmail-qstat.c; \
	else \
	./compile $(inc_deps) -DQUEUE_COUNT=10 qmail-qstat.c; \
	fi;

qmail-qstat.0: \
qmail-qstat.8
	nroff -man qmail-qstat.8 > qmail-qstat.0

qmail-multi: \
load qmail-multi.o envread.o env.o now.o fmt_ulong.o MakeArgs.o \
auto_qmail.o alloc.o alloc_re.o error.o wait_pid.o byte_cr.o variables.o control.o \
scan_ulong.o open_read.o getln.a stralloc.a str.a substdio.a sig.a dns.lib socket.lib
	./load qmail-multi envread.o env.o now.o fmt_ulong.o MakeArgs.o \
	auto_qmail.o alloc.o alloc_re.o error.o wait_pid.o byte_cr.o variables.o control.o \
	scan_ulong.o open_read.o getln.a stralloc.a str.a substdio.a sig.a \
	`cat dns.lib socket.lib`

qmail-multi.o: \
compile qmail-multi.c fmt.h now.h datetime.h env.h auto_qmail.h wait.h \
sig.h substdio.h control.h scan.h conf-queue
	if test -f conf-queue ; then \
	./compile -DQUEUE_COUNT=`cat conf-queue` qmail-multi.c; \
	else \
	./compile -DQUEUE_COUNT=10 qmail-multi.c; \
	fi;

spawn-filter: \
load spawn-filter.o variables.o \
fmt_ulong.o auto_qmail.o timeoutwrite.o byte_cr.o case_lowerb.o case_diffb.o \
scan_ulong.o constmap.o control.o open_read.o wildmat.o qregex.o MakeArgs.o \
matchregex.o cdb_seek.o cdb_unpack.o cdb_hash.o envrules.o error.a getln.a \
env.a stralloc.a wait.a strerr.a str.a substdio.a alloc.a socket.lib dns.lib
	./load spawn-filter variables.o \
	fmt_ulong.o auto_qmail.o timeoutwrite.o byte_cr.o case_lowerb.o case_diffb.o \
	scan_ulong.o constmap.o control.o open_read.o wildmat.o qregex.o MakeArgs.o \
	matchregex.o cdb_seek.o cdb_unpack.o cdb_hash.o envrules.o error.a \
	getln.a env.a stralloc.a wait.a strerr.a str.a substdio.a \
	alloc.a `cat dns.lib socket.lib`

spawn-filter.o: \
compile spawn-filter.c fmt.h str.h strerr.h env.h substdio.h stralloc.h error.h \
control.h wait.h auto_qmail.h variables.h qregex.h constmap.h scan.h
	./compile spawn-filter.c

spawn-filter.0: \
spawn-filter.8
	nroff -man spawn-filter.8 > spawn-filter.0

spawn-filter.8: \
spawn-filter.9
	cat spawn-filter.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> spawn-filter.8

qmail-queue: \
load qmail-queue.o triggerpull.o fmtqfn.o now.o date822fmt.o get_uid.o MakeArgs.o \
auto_qmail.o auto_split.o auto_uids.o envread.o str_diffn.o variables.o \
byte_chr.o case_diffb.o control.o alloc_re.o getln2.o getln.o open_read.o \
mess822_ok.o stralloc.a wait.a strerr.a str.a fs.a datetime.a seek.a ndelay.a open.a sig.a \
strsalloc.o alloc.a substdio.a error.a socket.lib libsyncdir.a
	./load qmail-queue triggerpull.o fmtqfn.o now.o date822fmt.o get_uid.o MakeArgs.o \
	auto_qmail.o auto_split.o auto_uids.o envread.o str_diffn.o variables.o \
	byte_chr.o case_diffb.o control.o alloc_re.o getln2.o getln.o open_read.o \
	mess822_ok.o strsalloc.o wait.a strerr.a stralloc.a datetime.a seek.a ndelay.a open.a \
	sig.a alloc.a substdio.a error.a str.a fs.a `cat socket.lib` -L. -lsyncdir

qmail-queue.0: \
qmail-queue.8
	nroff -man qmail-queue.8 > qmail-queue.0

qmail-queue.8: \
qmail-queue.9
	cat qmail-queue.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}INDIDIR}"$(INDIDIR)"}g \
	> qmail-queue.8

qmail-queue.o: \
compile qmail-queue.c sig.h exit.h open.h seek.h fmt.h tai.h \
alloc.h substdio.h datetime.h now.h datetime.h triggerpull.h stralloc.h \
control.h auto_qmail.h auto_uids.h date822fmt.h fmtqfn.h error.h env.h \
mess822.h scan.h str.h variables.h hasindimail.h wait.h getln.h \
conf-sync conf-qhpsi conf-ip
	./compile $(inc_deps) `grep -h -v "^#" conf-sync conf-qhpsi conf-ip` qmail-queue.c

domainkeys.o: compile domainkeys.c case.h alloc.h str.h byte.h conf-domainkeys
	./compile `grep -h -v "^#" conf-domainkeys` domainkeys.c
dktrace.o: compile dktrace.c dktrace.h str.h case.h conf-domainkeys
	./compile `grep -h -v "^#" conf-domainkeys` dktrace.c

dktest: load dktest.o scan_ulong.o domainkeys.o dktrace.o \
dns.o ip.o strsalloc.o error.o ipalloc.o fmt_ulong.o \
scan_xlong.o socket_v4mappedprefix.o socket_v6any.o \
dns_text.o case_diffs.o case_diffb.o fmt_str.o stralloc.a alloc.a str.a \
dns.lib
	./load dktest scan_ulong.o domainkeys.o dktrace.o \
	dns.o ip.o strsalloc.o error.o ipalloc.o fmt_ulong.o \
	scan_xlong.o socket_v4mappedprefix.o socket_v6any.o \
	dns_text.o case_diffs.o case_diffb.o fmt_str.o stralloc.a alloc.a str.a \
	-lcrypto `cat dns.lib`

dktest.o: compile dktest.c domainkeys.h conf-domainkeys
	./compile `grep -h -v "^#" conf-domainkeys` dktest.c

testtrace: load testtrace.o dns.o case_diffs.o case_diffb.o domainkeys.o dktrace.o \
scan_xlong.o scan_ulong.o ip.o ipalloc.o error.o strsalloc.o fmt_str.o fmt_ulong.o \
socket_v4mappedprefix.o socket_v6any.o str.a stralloc.a alloc.a dns.lib
	./load testtrace dns.o case_diffs.o case_diffb.o domainkeys.o dktrace.o fmt_ulong.o \
	scan_xlong.o scan_ulong.o ip.o ipalloc.o error.o strsalloc.o fmt_str.o \
	socket_v4mappedprefix.o socket_v6any.o str.a stralloc.a alloc.a `cat dns.lib` -lcrypto

dns_text.o: compile dns_text.c domainkeys.h byte.h
	./compile -DDOMAIN_KEYS dns_text.c
dns_text_dkim.o: compile dns_text.c byte.h domainkeys.h
	./compile dns_text.c -o dns_text_dkim.o

testtrace.o: compile testtrace.c dktrace.h
	./compile testtrace.c

dknewkey: dknewkey.sh warn-auto.sh
	rm -f dknewkey
	cat warn-auto.sh dknewkey.sh > dknewkey

qmail-dk: \
load qmail-dk.o triggerpull.o fmtqfn.o now.o date822fmt.o \
datetime.a seek.a ndelay.a open.a sig.a alloc.a substdio.a error.a \
str.a fs.a auto_qmail.o auto_split.o variables.o fd.a wait.a \
case_diffb.o case_diffs.o dns.o strsalloc.o scan_xlong.o ip.o ipalloc.o \
MakeArgs.o socket_v4mappedprefix.o socket_v6any.o mess822_ok.o \
domainkeys.o dktrace.o getopt.a env.a getln.a control.o dns_text.o stralloc.a dns.lib
	./load qmail-dk triggerpull.o fmtqfn.o now.o \
	date822fmt.o datetime.a seek.a ndelay.a open.a sig.a \
	fs.a auto_qmail.o auto_split.o variables.o mess822_ok.o \
	socket_v4mappedprefix.o socket_v6any.o scan_xlong.o \
	case_diffb.o case_diffs.o dns.o strsalloc.o ip.o ipalloc.o \
	fd.a wait.a domainkeys.o dktrace.o control.o dns_text.o \
	MakeArgs.o getopt.a env.a open.a getln.a substdio.a error.a \
	stralloc.a alloc.a str.a `cat dns.lib` -lcrypto

qmail-dk.o: \
compile qmail-dk.c substdio.h open.h sig.h fmt.h fd.h \
qmail.h alloc.h str.h stralloc.h datetime.h now.h wait.h auto_qmail.h \
sgetopt.h case.h getln.h env.h control.h domainkeys.h conf-domainkeys uint64.h
	./compile `grep -h -v "^#" conf-domainkeys` qmail-dk.c

qmail-dk.0: qmail-dk.8
	nroff -man qmail-dk.8 > qmail-dk.0
qmail-dk.8: qmail-dk.9
	cat qmail-dk.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-dk.8

dktest.8: dktest.9
	cat dktest.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> dktest.8

dkim.lib: trydkim.c compile load conf-dkim dns.lib
	( ( ./compile -I$(DESTDIR)$(INDIDIR)/include -I../libdkim-$(DKIM_VERSION) `grep -h -v "^#" conf-dkim` trydkim.c && \
	g++ -o trydkim trydkim.o  -L../libdkim-$(DKIM_VERSION) -L$(DESTDIR)$(INDIDIR)/lib `cat dns.lib` -ldkim -lcrypto ) >/dev/null 2>&1 \
	&& echo "-L../libdkim-$(DKIM_VERSION) -L$(DESTDIR)$(INDIDIR)/lib -ldkim" || exit 0 ) > dkim.lib
	rm -f trydkim.o trydkim

hasdkim.h:  trydkim.c compile load dns.lib dkim.lib conf-indi
	( ( ./compile -I$(DESTDIR)$(INDIDIR)/include -I../libdkim-$(DKIM_VERSION) `grep -h -v "^#" conf-dkim` trydkim.c && \
	g++ -o trydkim trydkim.o  -L../libdkim-$(DKIM_VERSION) `cat dkim.lib dns.lib` -lcrypto ) >/dev/null 2>&1 \
	&& (echo \#define HASDKIM 1) || exit 0 ) > hasdkim.h
	rm -f trydkim.o trydkim

qmail-dkim: qmail-dkim.o now.o error.o \
date822fmt.o datetime.o datetime_un.o stralloc_cats.o stralloc_catb.o \
stralloc_pend.o fmt_str.o byte_cr.o byte_copy.o \
fmt_uint.o fmt_uint0.o fmt_ulong.o alloc.o alloc_re.o \
auto_qmail.o variables.o case_diffb.o fd_copy.o control.o \
MakeArgs.o envread.o fd_move.o wait_pid.o open_excl.o open_read.o \
scan_ulong.o getln.o getln2.o case_diffs.o dns_text_dkim.o \
getopt.a sig.a stralloc.a substdio.a str.a dns.lib dkim.lib
	if test $(SYSTEM) = Darwin ; then \
	g++ -o qmail-dkim qmail-dkim.o now.o error.o \
	date822fmt.o datetime.o datetime_un.o stralloc_cats.o stralloc_catb.o \
	stralloc_pend.o fmt_str.o byte_cr.o byte_copy.o \
	fmt_uint.o fmt_uint0.o fmt_ulong.o alloc.o alloc_re.o \
	auto_qmail.o variables.o case_diffb.o fd_copy.o control.o \
	MakeArgs.o envread.o fd_move.o wait_pid.o open_excl.o open_read.o \
	scan_ulong.o getln.o getln2.o case_diffs.o dns_text_dkim.o \
	getopt.a sig.a stralloc.a substdio.a str.a \
	-L$(DESTDIR)$(INDIDIR)/lib `cat dns.lib dkim.lib` -lcrypto ; \
	else \
	g++ -s -o qmail-dkim qmail-dkim.o now.o error.o \
	date822fmt.o datetime.o datetime_un.o stralloc_cats.o stralloc_catb.o \
	stralloc_pend.o fmt_str.o byte_cr.o byte_copy.o \
	fmt_uint.o fmt_uint0.o fmt_ulong.o alloc.o alloc_re.o \
	auto_qmail.o variables.o case_diffb.o fd_copy.o control.o \
	MakeArgs.o envread.o fd_move.o wait_pid.o open_excl.o open_read.o \
	scan_ulong.o getln.o getln2.o case_diffs.o dns_text_dkim.o \
	getopt.a sig.a stralloc.a substdio.a str.a \
	-L$(DESTDIR)$(INDIDIR)/lib `cat dns.lib dkim.lib` -lcrypto ; \
	fi

qmail-dkim.o: \
compile qmail-dkim.c substdio.h open.h sig.h fmt.h fd.h \
qmail.h alloc.h str.h stralloc.h datetime.h now.h wait.h auto_qmail.h \
sgetopt.h case.h env.h control.h domainkeys.h hasdkim.h
	./compile -I$(DESTDIR)$(INDIDIR)/include -I../libdkim-$(DKIM_VERSION) $(inc_deps) qmail-dkim.c

qmail-dkim.8: qmail-dkim.9
	cat qmail-dkim.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-dkim.8

dk-filter.8: dk-filter.9
	cat dk-filter.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> dk-filter.8

qmail-remote: \
load qmail-remote.o control.o constmap.o timeoutread.o timeoutwrite.o base64.o \
timeoutconn.o tcpto.o now.o dns.o ip.o ipalloc.o strsalloc.o ipme.o quote.o variables.o \
socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
ndelay.a case.a sig.a open.a lock.a seek.a getln.a stralloc.a alloc.a \
socket_v4mappedprefix.o scan_xlong.o socket_tcp.o socket_tcp6.o \
substdio.a error.a str.a fs.a cdb.a auto_qmail.o envread.o dns.lib socket.lib error_str.o \
tls.lib
	./load qmail-remote control.o constmap.o timeoutread.o base64.o \
	timeoutwrite.o timeoutconn.o tcpto.o now.o dns.o ip.o scan_xlong.o \
	socket_ip4loopback.o socket_v6loopback.o socket_v6any.o ipalloc.o \
	socket_v4mappedprefix.o strsalloc.o ipme.o quote.o \
	variables.o socket_tcp.o socket_tcp6.o ndelay.a case.a sig.a open.a \
	lock.a seek.a getln.a stralloc.a alloc.a substdio.a error.a \
	str.a fs.a cdb.a auto_qmail.o envread.o error_str.o `cat dns.lib socket.lib \
	tls.lib`

qmail-remote.0: \
qmail-remote.8
	nroff -man qmail-remote.8 > qmail-remote.0
qmail-remote.8: \
qmail-remote.9
	cat qmail-remote.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-remote.8

qmail-remote.o: \
compile qmail-remote.c tls.h ssl_timeoutio.h sig.h stralloc.h \
gen_alloc.h substdio.h subfd.h scan.h case.h error.h \
auto_qmail.h control.h dns.h alloc.h quote.h ip.h ipalloc.h \
ipme.h cdb.h open.h base64.h gen_allocdefs.h str.h now.h \
datetime.h exit.h constmap.h tcpto.h timeoutconn.h timeoutread.h \
timeoutwrite.h env.h uint32.h haveip6.h socket.h variables.h \
byte.h fmt.h conf-tls conf-ip conf-batv hasindimail.h
	./compile `grep -h -v "^#" conf-tls conf-ip conf-batv` qmail-remote.c

qmail-rspawn: \
load qmail-rspawn.o spawn.o tcpto_clean.o now.o coe.o get_uid.o sig.a open.a \
strerr.a seek.a lock.a wait.a fd.a stralloc.a alloc.a substdio.a error.a str.a \
auto_qmail.o auto_uids.o auto_spawn.o constmap.o rcpthosts.o case_diffb.o \
control.o case_lowerb.o getln.o getln2.o byte_chr.o cdb_seek.o cdb_unpack.o \
cdb_hash.o stralloc_copy.o stralloc_cat.o scan_ulong.o substdi.c byte_cr.o \
env.o str_diffn.o str_cpy.o envread.o variables.o fmt_ulong.o fmt_str.o \
fmt_strn.o indimail.lib dns.lib
	./load qmail-rspawn spawn.o tcpto_clean.o now.o coe.o get_uid.o \
	auto_spawn.o constmap.o rcpthosts.o case_diffb.o control.o case_lowerb.o getln.o getln2.o \
	byte_chr.o cdb_seek.o cdb_unpack.o cdb_hash.o stralloc_copy.o stralloc_cat.o \
	scan_ulong.o substdi.c byte_cr.o env.o str_cpy.o envread.o variables.o \
	strerr.a sig.a open.a seek.a lock.a wait.a fd.a stralloc.a alloc.a \
	substdio.a error.a str.a auto_qmail.o auto_uids.o fmt_ulong.o fmt_str.o \
	fmt_strn.o `cat indimail.lib` `cat dns.lib`

qmail-rspawn.0: \
qmail-rspawn.8
	nroff -man qmail-rspawn.8 > qmail-rspawn.0

qmail-rspawn.o: \
compile qmail-rspawn.c fd.h wait.h substdio.h exit.h error.h str.h \
tcpto.h env.h rcpthosts.h fmt.h auto_qmail.h \
hasindimail.h ipalloc.h conf-tls conf-ip
	./compile $(inc_deps) `grep -h -v "^#" conf-tls conf-ip` qmail-rspawn.c

qmail-send: \
load qmail-send.o qsutil.o control.o constmap.o newfield.o prioq.o \
trigger.o fmtqfn.o quote.o now.o readsubdir.o qmail.o date822fmt.o matchregex.o\
datetime.a case.a ndelay.a getln.a wait.a seek.a fd.a sig.a open.a \
lock.a stralloc.a alloc.a substdio.a error.a fs.a auto_qmail.o cdb_seek.o \
cdb_unpack.o cdb_hash.o auto_split.o variables.o case_lowerb.o envrules.o qregex.o \
wildmat.o env.a str.a libsyncdir.a 
	./load qmail-send qsutil.o control.o constmap.o newfield.o \
	prioq.o trigger.o fmtqfn.o quote.o now.o readsubdir.o \
	wildmat.o cdb_unpack.o cdb_hash.o qmail.o date822fmt.o \
	auto_qmail.o auto_split.o cdb_seek.o variables.o \
	case_lowerb.o envrules.o matchregex.o \
	datetime.a case.a ndelay.a getln.a \
	wait.a seek.a fd.a sig.a open.a lock.a stralloc.a alloc.a \
	substdio.a error.a fs.a env.a str.a -L. -lsyncdir

qmail-send.0: \
qmail-send.8
	nroff -man qmail-send.8 > qmail-send.0

qmail-send.8: \
qmail-send.9 conf-break conf-spawn
	cat qmail-send.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-send.8

qmail-send.o: \
compile qmail-send.c sig.h direntry.h control.h select.h \
open.h seek.h exit.h lock.h ndelay.h now.h datetime.h getln.h \
substdio.h alloc.h error.h stralloc.h gen_alloc.h str.h byte.h fmt.h \
scan.h case.h auto_qmail.h trigger.h newfield.h stralloc.h quote.h \
qmail.h qsutil.h prioq.h datetime.h gen_alloc.h constmap.h envrules.h \
fmtqfn.h readsubdir.h direntry.h variables.h env.h qmail-todo.h \
hasindimail.h conf-mime conf-sync conf-batv
	./compile $(inc_deps) `grep -h -v "^#" conf-mime conf-sync conf-batv` qmail-send.c

qmail-daemon: \
load qmail-daemon.o scan_ulong.o envread.o env.o fmt_str.o fmt_strn.o fmt_ulong.o auto_qmail.o \
alloc.o alloc_re.o error.o open_write.o str.a lock_exnb.o sig_catch.o \
error_str.o sig_block.o wait_pid.o substdio.a stralloc.a
	./load qmail-daemon scan_ulong.o envread.o env.o fmt_str.o fmt_strn.o fmt_ulong.o auto_qmail.o \
	alloc.o alloc_re.o error.o str.a open_write.o lock_exnb.o sig_catch.o \
	error_str.o sig_block.o wait_pid.o substdio.a stralloc.a

qmail-daemon.o: \
compile qmail-daemon.c substdio.h error.h fmt.h env.h sig.h open.h lock.h alloc.h \
wait.h auto_qmail.h scan.h stralloc.h str.h conf-queue
	if test -f conf-queue ; then \
	./compile -DQUEUE_COUNT=`cat conf-queue` qmail-daemon.c; \
	else \
	./compile -DQUEUE_COUNT=10 qmail-daemon.c; \
	fi;

get_uid.o: \
compile get_uid.c auto_uids.h strerr.h str.h
	./compile get_uid.c

qmail-showctl: \
load qmail-showctl.o auto_uids.o control.o open.a getln.a stralloc.a dns.lib \
strerr.a alloc.a substdio.a error.a str.a fs.a auto_qmail.o auto_break.o indimail.lib \
auto_patrn.o auto_spawn.o auto_split.o envread.o str_diffn.o variables.o get_uid.o
	./load qmail-showctl auto_uids.o control.o auto_qmail.o \
	auto_break.o auto_patrn.o auto_spawn.o \
	auto_split.o envread.o str_diffn.o variables.o get_uid.o \
	strerr.a open.a getln.a stralloc.a alloc.a substdio.a error.a \
	str.a fs.a `cat indimail.lib` `cat dns.lib`

qmail-showctl.0: \
qmail-showctl.8
	nroff -man qmail-showctl.8 > qmail-showctl.0

qmail-showctl.o: \
compile qmail-showctl.c substdio.h subfd.h exit.h fmt.h \
str.h control.h constmap.h stralloc.h gen_alloc.h direntry.h \
auto_uids.h auto_qmail.h auto_break.h auto_patrn.h auto_spawn.h \
auto_split.h spf.h env.h variables.h hasindimail.h conf-spf conf-batv
	./compile $(inc_deps) `grep -h -v "^#" conf-spf conf-batv` qmail-showctl.c

qmail-smtpd: \
load qmail-smtpd.o smtpd.o rcpthosts.o recipients.o commands.o timeoutread.o \
timeoutwrite.o ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o constmap.o \
received.o date822fmt.o wildmat.o now.o qmail.o variables.o cdb.a fd.a \
wait.a datetime.a getln.a envrules.o bodycheck.o sqlmatch.o matchregex.o \
scan_xlong.o socket_v4mappedprefix.o socket_v6any.o \
open.a sig.a case.a spf.o dns.o auto_qmail.o auto_uids.o base64.o qregex_smtpd.o \
tablematch.o ndelay.o ndelay_off.o greylist.o auto_break.o \
env.a stralloc.a alloc.a substdio.a error.a str.a fs.a lock.a \
socket.lib tai.lib dns.lib indimail.lib tls.lib
	./load qmail-smtpd smtpd.o rcpthosts.o recipients.o commands.o timeoutread.o \
	timeoutwrite.o ip.o ipme.o ipalloc.o strsalloc.o control.o etrn.o atrn.o constmap.o \
	received.o date822fmt.o wildmat.o now.o qmail.o variables.o \
	envrules.o bodycheck.o spf.o dns.o auto_qmail.o sqlmatch.o matchregex.o \
	scan_xlong.o socket_v4mappedprefix.o socket_v6any.o \
	auto_uids.o base64.o qregex_smtpd.o tablematch.o ndelay.o ndelay_off.o \
	cdb.a fd.a wait.a datetime.a greylist.o auto_break.o \
	getln.a open.a sig.a case.a env.a stralloc.a alloc.a substdio.a error.a \
	str.a fs.a lock.a `cat socket.lib tai.lib dns.lib indimail.lib tls.lib`

qregex.o: \
compile qregex.c qregex.h case.h stralloc.h constmap.h matchregex.h byte.h env.h \
str.h cdb.h uint32.h open.h error.h control.h variables.h hasindimail.h
	./compile $(inc_deps) qregex.c

# qregex.c compiled with sqlmatch()
qregex_smtpd.o: \
compile qregex.c qregex.h case.h stralloc.h constmap.h matchregex.h byte.h env.h \
str.h cdb.h uint32.h open.h error.h control.h variables.h
	./compile -DUSE_SQL $(inc_deps) qregex.c -o qregex_smtpd.o

matchregex.o: \
compile matchregex.c stralloc.h error.h qregex.h
	./compile matchregex.c

qmail-nullqueue: \
load qmail-nullqueue.o
	./load qmail-nullqueue

qmail-nullqueue.o: \
compile qmail-nullqueue.c
	./compile qmail-nullqueue.c

qmail-todo.0: \
qmail-todo.8
	nroff -man qmail-todo.8 > qmail-todo.0
qmail-todo.8: \
qmail-todo.9
	cat qmail-todo.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> qmail-todo.8

qmail-smtpd.0: \
qmail-smtpd.8
	nroff -man qmail-smtpd.8 > qmail-smtpd.0

qmail-smtpd.8: \
qmail-smtpd.9
	cat qmail-smtpd.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-smtpd.8

greydaemon.0: \
greydaemon.8
	nroff -man greydaemon.8 > greydaemon.0

qmail-poppass.0: \
qmail-poppass.8
	nroff -man qmail-poppass.8 > qmail-poppass.0
qmail-poppass.8: \
qmail-poppass.9
	cat qmail-poppass.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-poppass.8

qmail-multi.0: \
qmail-multi.8
	nroff -man qmail-multi.8 > qmail-multi.0

qmail-multi.8: \
qmail-multi.9
	cat qmail-multi.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-multi.8

smtpd.o: \
compile smtpd.c tls.h ssl_timeoutio.h sig.h stralloc.h gen_alloc.h \
substdio.h alloc.h auto_qmail.h control.h received.h constmap.h \
error.h ipme.h ip.h ipalloc.h qmail.h recipients.h greylist.h \
matchregex.o str.h fmt.h scan.h byte.h case.h env.h now.h datetime.h \
variables.h exit.h rcpthosts.h timeoutread.h timeoutwrite.h commands.h \
wait.h etrn.h date822fmt.h fd.h base64.h envrules.h tablematch.h cdb.h uint32.h \
matchregex.h qregex.h hasindimail.h conf-tls conf-spf conf-ip conf-batv
	./compile $(inc_deps) `grep -h -v "^#" conf-tls conf-spf conf-ip conf-batv` smtpd.c

sqlmatch.o: \
compile sqlmatch.c stralloc.h byte.h error.h env.h open.h variables.h scan.h qregex.h \
hasindimail.h
	./compile $(inc_deps) sqlmatch.c

greylist.o: \
compile greylist.c stralloc.h ip.h scan.h byte.h greylist.h error.h \
timeoutread.h timeoutwrite.h
	./compile greylist.c

qmail-start: \
load qmail-start.o prot.o fd.a auto_uids.o get_uid.o \
strerr_die.o strerr_sys.o substdio.a str.a error.a

	./load qmail-start prot.o fd.a auto_uids.o get_uid.o \
	strerr_die.o strerr_sys.o substdio.a str.a error.a

qmail-start.0: \
qmail-start.8
	nroff -man qmail-start.8 > qmail-start.0

qmail-start.8: \
qmail-start.9 conf-break conf-spawn
	cat qmail-start.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-start.8

qmail-start.o: \
compile qmail-start.c fd.h prot.h exit.h auto_uids.h qmail-todo.h
	./compile qmail-start.c

qmail-tcpok: \
load qmail-tcpok.o scan_ulong.o auto_qmail.o envread.o str_diffn.o variables.o \
fmt_ulong.o control.o getln.o getln2.o \
alloc.a open.a lock.a strerr.a substdio.a error.a str.a stralloc.a
	./load qmail-tcpok scan_ulong.o auto_qmail.o envread.o \
	str_diffn.o variables.o fmt_ulong.o control.o getln.o getln2.o \
	open.a lock.a strerr.a substdio.a error.a str.a stralloc.a alloc.a

qmail-tcpok.0: \
qmail-tcpok.8
	nroff -man qmail-tcpok.8 > qmail-tcpok.0

qmail-tcpok.o: \
compile qmail-tcpok.c strerr.h substdio.h lock.h open.h \
auto_qmail.h exit.h env.h variables.h hasindimail.h scan.h stralloc.h fmt.h \
control.h subfd.h conf-queue
	if test -f conf-queue ; then \
	./compile $(inc_deps) -DQUEUE_COUNT=`cat conf-queue` qmail-tcpok.c; \
	else \
	./compile $(inc_deps) -DQUEUE_COUNT=10 qmail-tcpok.c; \
	fi;

qmail-tcpto: \
load qmail-tcpto.o ip.o now.o auto_qmail.o \
envread.o str_diffn.o variables.o scan_xlong.o \
control.o getln.o getln2.o \
socket_v4mappedprefix.o socket_v6any.o \
open.a lock.a substdio.a error.a str.a fs.a stralloc.a alloc.a
	./load qmail-tcpto ip.o now.o auto_qmail.o envread.o \
	str_diffn.o variables.o scan_xlong.o \
	control.o getln.o getln2.o \
	socket_v4mappedprefix.o socket_v6any.o \
	open.a lock.a substdio.a error.a str.a fs.a stralloc.a alloc.a

qmail-tcpto.0: \
qmail-tcpto.8
	nroff -man qmail-tcpto.8 > qmail-tcpto.0

qmail-tcpto.o: \
compile qmail-tcpto.c substdio.h subfd.h auto_qmail.h \
fmt.h ip.h lock.h error.h exit.h datetime.h now.h datetime.h env.h \
variables.h hasindimail.h open.h byte.h scan.h stralloc.h subfd.h \
control.h tcpto.h ipalloc.h conf-tls conf-ip conf-queue
	if test -f conf-queue ; then \
	./compile $(inc_deps) -DQUEUE_COUNT=`cat conf-queue` qmail-tcpto.c; \
	else \
	./compile $(inc_deps) -DQUEUE_COUNT=10 qmail-tcpto.c; \
	fi;

qmail-todo: \
load qmail-todo.o control.o constmap.o trigger.o fmtqfn.o now.o \
readsubdir.o case.a ndelay.a getln.a sig.a open.a stralloc.a alloc.a \
substdio.a error.a str.a fs.a auto_qmail.o auto_split.o envread.o \
str_diffn.o variables.o libsyncdir.a
	./load qmail-todo control.o constmap.o trigger.o fmtqfn.o now.o \
	readsubdir.o case.a ndelay.a getln.a sig.a open.a stralloc.a \
	alloc.a substdio.a error.a str.a fs.a auto_qmail.o auto_split.o \
	envread.o str_diffn.o variables.o -L. -lsyncdir

qmail-todo.o: \
compile qmail-todo.c alloc.h auto_qmail.h byte.h constmap.h control.h \
direntry.h error.h exit.h fmt.h fmtqfn.h getln.h open.h ndelay.h now.h \
readsubdir.h sig.h scan.h select.h str.h stralloc.h substdio.h \
trigger.h env.h variables.h qmail-todo.h hasindimail.h conf-sync
	./compile $(inc_deps) `grep -h -v "^#" conf-sync` qmail-todo.c
 

qmail-upq: \
warn-auto.sh qmail-upq.sh conf-qmail conf-break conf-split
	cat warn-auto.sh qmail-upq.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPLIT}"`head -1 conf-split`"}g \
	> qmail-upq

qmail-users.0: \
qmail-users.5
	nroff -man qmail-users.5 > qmail-users.0

qmail-users.5: \
qmail-users.9 conf-break conf-spawn
	cat qmail-users.9 \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	| sed s}BREAK}"`head -1 conf-break`"}g \
	| sed s}SPAWN}"`head -1 conf-spawn`"}g \
	> qmail-users.5

qmail.0: \
qmail.7
	nroff -man qmail.7 > qmail.0

qmail.o: \
compile qmail.c substdio.h wait.h exit.h fd.h \
qmail.h env.h auto_qmail.h scan.h
	./compile qmail.c

qreceipt: \
load qreceipt.o headerbody.o hfield.o quote.o token822.o qmail.o \
getln.a fd.a wait.a sig.a env.a stralloc.a alloc.a substdio.a error.a \
str.a auto_qmail.o scan_ulong.o
	./load qreceipt headerbody.o hfield.o quote.o token822.o \
	qmail.o getln.a fd.a wait.a sig.a env.a stralloc.a alloc.a \
	substdio.a error.a str.a auto_qmail.o scan_ulong.o

qreceipt.0: \
qreceipt.1
	nroff -man qreceipt.1 > qreceipt.0

qreceipt.o: \
compile qreceipt.c sig.h env.h substdio.h stralloc.h \
subfd.h getln.h alloc.h str.h hfield.h token822.h \
gen_alloc.h error.h gen_allocdefs.h headerbody.h exit.h \
open.h quote.h qmail.h
	./compile qreceipt.c

qsmhook: \
load qsmhook.o sig.a case.a fd.a wait.a getopt.a env.a stralloc.a \
alloc.a substdio.a error.a str.a
	./load qsmhook sig.a case.a fd.a wait.a getopt.a env.a \
	stralloc.a alloc.a substdio.a error.a str.a 

qsmhook.o: \
compile qsmhook.c fd.h stralloc.h gen_alloc.h sgetopt.h \
subgetopt.h wait.h env.h byte.h str.h alloc.h exit.h case.h \
subfd.h error.h substdio.h sig.h
	./compile qsmhook.c

qsutil.o: \
compile qsutil.c stralloc.h gen_alloc.h substdio.h \
qsutil.h
	./compile qsutil.c

quote.o: \
compile quote.c stralloc.h gen_alloc.h str.h quote.h
	./compile quote.c

rcpthosts.o: \
compile rcpthosts.c cdb.h uint32.h byte.h open.h error.h control.h \
constmap.h stralloc.h gen_alloc.h rcpthosts.h env.h variables.h case.h
	./compile rcpthosts.c

recipients.o: \
compile recipients.c cdb.h uint32.h byte.h open.h error.h control.h \
constmap.h stralloc.h gen_alloc.h recipients.h case.h str.h \
sig.h wait.h substdio.h fd.h auto_break.h
	./compile recipients.c

readsubdir.o: \
compile readsubdir.c readsubdir.h direntry.h fmt.h scan.h str.h \
auto_split.h
	./compile readsubdir.c

received.o: \
compile received.c fmt.h qmail.h substdio.h now.h datetime.h \
datetime.h date822fmt.h received.h
	./compile received.c

remoteinfo.o: \
compile remoteinfo.c byte.h substdio.h ip.h fmt.h timeoutconn.h \
haveip6.h error.h socket.h timeoutread.h timeoutwrite.h \
remoteinfo.h conf-ip
	./compile `grep -h -v "^#" conf-ip` remoteinfo.c

rrforward: \
load rrforward.o qmail.o strerr.a alloc.a fd.a wait.a sig.a env.a \
substdio.a error.a str.a fs.a lock.a seek.a auto_qmail.o alloc.a
	./load rrforward qmail.o strerr.a alloc.a fd.a wait.a sig.a \
	env.a substdio.a error.a str.a fs.a lock.a seek.a auto_qmail.o alloc.a

rrforward.0: \
rrforward.1
	nroff -man rrforward.1 > rrforward.0

rrforward.o: \
compile rrforward.c sig.h seek.h exit.h env.h qmail.h \
strerr.h substdio.h fmt.h lock.h scan.h str.h
	./compile rrforward.c

scan_8long.o: \
compile scan_8long.c scan.h
	./compile scan_8long.c

scan_ulong.o: \
compile scan_ulong.c scan.h
	./compile scan_ulong.c

seek.a: \
makelib seek_cur.o seek_end.o seek_set.o seek_trunc.o
	./makelib seek.a seek_cur.o seek_end.o seek_set.o \
	seek_trunc.o

seek_cur.o: \
compile seek_cur.c seek.h
	./compile seek_cur.c

seek_end.o: \
compile seek_end.c seek.h
	./compile seek_end.c

seek_set.o: \
compile seek_set.c seek.h
	./compile seek_set.c

seek_trunc.o: \
compile seek_trunc.c seek.h
	./compile seek_trunc.c

select.h: \
compile trysysel.c select.h1 select.h2
	( ./compile trysysel.c >/dev/null 2>&1 \
	&& cat select.h2 || cat select.h1 ) > select.h
	rm -f trysysel.o trysysel

sendmail: \
load sendmail.o env.a getopt.a alloc.a substdio.a error.a str.a \
auto_qmail.o alloc.a
	./load sendmail env.a getopt.a alloc.a substdio.a error.a \
	str.a auto_qmail.o alloc.a

sendmail.o: \
compile sendmail.c sgetopt.h subgetopt.h subfd.h \
substdio.h alloc.h auto_qmail.h exit.h env.h str.h
	./compile sendmail.c

install-strip: install
distclean: clean
	rm -f lib_deps inc_deps conf-qmail

install: \
it man
	if test ! -d $(DESTDIR)$(INDIDIR)/etc ; then \
		mkdir -p $(DESTDIR)$(INDIDIR)/etc ; \
	fi ;
	./setup $(DESTDIR)$(INDIDIR) > $(DESTDIR)$(INDIDIR)/etc/qmailprog.list

sgetopt.o: \
compile sgetopt.c subfd.h substdio.h sgetopt.h subgetopt.h \
subgetopt.h
	./compile sgetopt.c

shar: \
FILES BLURB BLURB2 BLURB3 BLURB4 README FAQ INSTALL INSTALL.alias \
INSTALL.ctl INSTALL.ids INSTALL.maildir INSTALL.mbox INSTALL.vsm \
REMOVE.sendmail REMOVE.binmail TEST.deliver TEST.receive UPGRADE \
THOUGHTS TODO THANKS CHANGES SECURITY INTERNALS SENDMAIL \
PIC.local2alias PIC.local2ext PIC.local2local PIC.local2rem \
PIC.local2virt PIC.nullclient PIC.relaybad PIC.relaygood \
PIC.rem2local FILES VERSION SYSDEPS TARGETS Makefile BIN.README \
BIN.Makefile idedit.c conf-break auto_break.h conf-spawn \
auto_spawn.h chkspawn.c conf-split auto_split.h conf-patrn \
auto_patrn.h conf-users conf-groups auto_uids.h auto_usera.h extra.h \
addresses.5 except.1 bouncesaying.1 condredirect.1 dot-qmail.9 \
envelopes.5 forgeries.7 forward.1 rrforward.1 maildir2mbox.1 qmaildirmake.1 \
maildirwatch.1 mailsubj.1 mbox.5 preline.1 qmail-clean.8 \
qmail-command.8 qmail-control.9 qmail-getpw.9 qmail-header.5 \
qmail-inject.8 qmail-limits.9 qmail-local.8 qmail-log.5 \
qmail-lspawn.8 qmail-newmrh.9 qmail-newu.9 qmail-pop3d.8 qmail-recipients.9 \
qmail-popbull.8 \
qmail-popup.8 qmail-pw2u.9 qmail-qmqpc.8 qmail-qmqpd.8 qmail-qmtpd.8 \
qmail-qread.8 qmail-qstat.8 qmail-queue.9 qmail-remote.9 \
qmail-rspawn.8 qmail-send.9 qmail-todo.9 qmail-showctl.8 qmail-smtpd.9 \
qmail-start.9 qmail-tcpok.8 qmail-tcpto.8 qmail-users.9 qmail.7 qmail-multi.9 \
qreceipt.1 splogger.8 tcp-env.1 config.sh config-fast.sh \
qmail-clean.c qmail-getpw.c qmail-inject.c qmail-local.c \
qmail-lspawn.c qmail-newmrh.c qmail-newu.c qmail-pop3d.c \
qmail-popup.c qmail-pw2u.c qmail-qmqpc.c qmail-qmqpd.c qmail-qmtpd.c \
qmail-qread.c qmail-multi.c qmail-queue.c qmail-remote.c \
qmail-rspawn.c qmail-send.c qmail-showctl.c qmail-smtpd.c smtpd.c \
qmail-start.c qmail-tcpok.c qmail-tcpto.c spawn.c dnscname.c dnsfq.c \
dnsip.c dnsmxip.c dnsptr.c dnstxt.c hostname.c ipmeprint.c tcp-env.c \
sendmail.c qreceipt.c qsmhook.c forward.c preline.c predate.c \
except.c bouncesaying.c condredirect.c qmaildirmake.c maildir2mbox.c \
maildirwatch.c splogger.c qail.sh elq.sh pinq.sh etrn.sh atrn.sh qmail-upq.sh \
datemail.sh mailsubj.sh qlx.h rcpthosts.h rcpthosts.c commands.h \
recipients.h recipients.c qmail-recipients.c \
commands.c dnsdoe.h dnsdoe.c fmtqfn.h fmtqfn.c gfrom.h gfrom.c \
myctime.h myctime.c newfield.h newfield.c qsutil.h qsutil.c \
readsubdir.h readsubdir.c received.h received.c tcpto.h tcpto.c \
tcpto_clean.c trigger.h trigger.c triggerpull.h triggerpull.c \
trynpbg1.c trysyslog.c conf-cc conf-ld home.sh home+df.sh proc.sh \
proc+df.sh binm1.sh binm2.sh binm3.sh binm1+df.sh binm2+df.sh \
binm3+df.sh find-systype.sh make-compile.sh make-load.sh \
make-makelib.sh trycpp.c warn-auto.sh auto-str.c auto-int.c \
auto-int8.c auto-gid.c auto-uid.c hier.c setup.c instcheck.c \
alloc.3 alloc.h alloc.c alloc_re.c case.3 case.h \
case_diffb.c case_diffs.c case_lowerb.c case_lowers.c case_starts.c \
cdb.3 cdb.h cdb_hash.c cdb_seek.c cdb_unpack.c cdbmake.h \
cdbmake_add.c cdbmake_hash.c cdbmake_pack.c cdbmss.h cdbmss.c coe.3 \
coe.h coe.c fd.h fd_copy.3 fd_copy.c fd_move.3 fd_move.c fifo_make.3 \
fifo.h fifo.c trymkffo.c tryvfork.c now.3 now.h now.c \
open.h open_append.c open_excl.c open_read.c open_trunc.c \
open_write.c seek.h seek_cur.c seek_end.c seek_set.c seek_trunc.c \
conf-qmail auto_qmail.h qmail.h qmail.c gen_alloc.h gen_allocdefs.h \
stralloc.3 stralloc.h stralloc_eady.c stralloc_pend.c stralloc_copy.c \
stralloc_opyb.c stralloc_opys.c stralloc_cat.c stralloc_catb.c \
stralloc_cats.c stralloc_arts.c strerr.h strerr_sys.c strerr_die.c \
substdio.h substdio.c substdi.c substdo.c substdio_copy.c subfd.h \
subfderr.c subfdouts.c subfdout.c subfdins.c subfdin.c \
exit.h timeoutconn.h timeoutconn.c timeoutread.h timeoutread.c \
timeoutwrite.h timeoutwrite.c remoteinfo.h remoteinfo.c uint32.h1 \
uint32.h2 tryulong32.c wait.3 wait.h wait_pid.c wait_nohang.c \
trywaitp.c sig.h sig_alarm.c sig_block.c sig_catch.c sig_pause.c \
sig_pipe.c sig_child.c sig_term.c sig_hup.c sig_misc.c sig_bug.c \
trysgact.c trysgprm.c env.3 env.h env.c envread.c byte.h byte_chr.c \
byte_copy.c byte_cr.c byte_diff.c byte_rchr.c byte_zero.c str.h \
str_chr.c str_cpy.c str_diff.c str_diffn.c str_len.c str_rchr.c \
str_start.c lock.h lock_ex.c lock_exnb.c lock_un.c tryflock.c getln.3 \
getln.h getln.c getln2.3 getln2.c sgetopt.3 sgetopt.h sgetopt.c \
subgetopt.3 subgetopt.h subgetopt.c error.3 error_str.3 error_temp.3 \
error.h error.c error_str.c error_temp.c fmt.h fmt_str.c fmt_strn.c \
fmt_uint.c fmt_uint0.c fmt_ulong.c scan.h scan_ulong.c scan_8long.c \
slurpclose.h slurpclose.c quote.h quote.c hfield.h hfield.c \
headerbody.h headerbody.c token822.h token822.c control.h control.c \
datetime.3 datetime.h datetime.c datetime_un.c prioq.h prioq.c \
date822fmt.h date822fmt.c dns.h dns.c trylsock.c tryrsolv.c ip.h ip.c \
ipalloc.h ipalloc.c select.h1 select.h2 trysysel.c ndelay.h ndelay.c \
ndelay_off.c direntry.3 direntry.h1 direntry.h2 trydrent.c prot.h \
prot.c chkshsgr.c warn-shsgr tryshsgr.c ipme.h ipme.c trysalen.c \
maildir.5 maildir.h maildir.c tcp-environ.5 constmap.h constmap.c \
tryltai.c trytai.c tryip6.c update_tmprsadh
	shar -m `cat FILES` > shar
	chmod 400 shar

sig.a: \
makelib sig_alarm.o sig_block.o sig_catch.o sig_pause.o sig_pipe.o \
sig_child.o sig_hup.o sig_term.o sig_bug.o sig_misc.o
	./makelib sig.a sig_alarm.o sig_block.o sig_catch.o \
	sig_pause.o sig_pipe.o sig_child.o sig_hup.o sig_term.o \
	sig_bug.o sig_misc.o

sig_alarm.o: \
compile sig_alarm.c sig.h
	./compile sig_alarm.c

sig_block.o: \
compile sig_block.c sig.h hassgprm.h
	./compile sig_block.c

sig_bug.o: \
compile sig_bug.c sig.h
	./compile sig_bug.c

sig_catch.o: \
compile sig_catch.c sig.h hassgact.h
	./compile sig_catch.c

sig_child.o: \
compile sig_child.c sig.h
	./compile sig_child.c

sig_hup.o: \
compile sig_hup.c sig.h
	./compile sig_hup.c

sig_misc.o: \
compile sig_misc.c sig.h
	./compile sig_misc.c

sig_pause.o: \
compile sig_pause.c sig.h hassgprm.h
	./compile sig_pause.c

sig_pipe.o: \
compile sig_pipe.c sig.h
	./compile sig_pipe.c

sig_term.o: \
compile sig_term.c sig.h
	./compile sig_term.c

slurpclose.o: \
compile slurpclose.c stralloc.h gen_alloc.h slurpclose.h \
error.h
	./compile slurpclose.c

socket.lib: \
trylsock.c compile load
	( ( ./compile trylsock.c && \
	./load trylsock -lsocket -lnsl ) >/dev/null 2>&1 \
	&& echo -lsocket -lnsl || exit 0 ) > socket.lib
	rm -f trylsock.o trylsock

tls.lib: \
tryltls.c compile conf-tls load tls.o ssl_timeoutio.o
	( ( ./compile `grep -v "^#" conf-tls` tryltls.c && \
	./load tryltls -lssl -lcrypto ) >/dev/null 2>&1 \
	&& echo tls.o ssl_timeoutio.o -lssl -lcrypto || exit 0 ) > tls.lib
	rm -f tryltls.o tryltls

tls.o: \
compile tls.c error.h conf-tls
	./compile `grep -h -v "^#" conf-tls` tls.c

ssl_timeoutio.o: \
compile ssl_timeoutio.c ssl_timeoutio.h select.h error.h ndelay.h conf-tls
	./compile `grep -h -v "^#" conf-tls` ssl_timeoutio.c

hasindimail.h: \
tryindimail.c compile load indimail.lib conf-indi
	( ( ./compile `grep -h -v "^#" conf-indi` tryindimail.c && \
	./load tryindimail `cat indimail.lib`) >/dev/null 2>&1 \
	&& (echo \#define INDIMAIL 1;echo \#ifdef INDIMAIL; \
	echo \#include \"indimail.h\"; \
	echo \#endif) || exit 0 ) > hasindimail.h
	rm -f tryindimail.o tryindimail;echo "----- indimail.h -----";/bin/cat hasindimail.h 1>&2

indimail.lib: tryindimail.c compile load conf-indi
	( ( ./compile `grep -h -v "^#" conf-indi` tryindimail.c && \
	./load tryindimail $(lib_deps) ) >/dev/null 2>&1 \
	&& echo "$(lib_deps) -L$(DESTDIR)/usr/lib" || exit 0 ) > indimail.lib
	rm -f tryindimail.o tryindimail;echo "----- indimail.lib -----";/bin/cat indimail.lib 1>&2

spawn.o: \
compile chkspawn spawn.c sig.h wait.h substdio.h byte.h str.h \
stralloc.h gen_alloc.h select.h exit.h coe.h open.h error.h alloc.h \
auto_qmail.h auto_uids.h auto_spawn.h env.h variables.h hasindimail.h
	./chkspawn
	./compile $(inc_deps) spawn.c

spf.o: \
compile spf.c stralloc.h gen_alloc.h alloc.h ipme.h ip.h ipalloc.h \
strsalloc.h str.h fmt.h scan.h byte.h now.h case.h conf-spf conf-tls conf-ip
	./compile `grep -h -v "^#" conf-spf conf-tls conf-ip` spf.c

spfquery: \
load spfquery.o spf.o ip.o ipme.o ipalloc.o strsalloc.o now.o dns.o variables.o \
scan_xlong.o socket_v4mappedprefix.o socket_v6any.o byte_cr.o open_read.o envread.o getln.a \
datetime.a stralloc.a alloc.a str.a substdio.a error.a fs.a case.a dns.lib
	./load spfquery spf.o ip.o ipme.o ipalloc.o strsalloc.o \
	scan_xlong.o socket_v4mappedprefix.o socket_v6any.o now.o dns.o variables.o byte_cr.o \
	open_read.o envread.o getln.a datetime.a stralloc.a alloc.a str.a substdio.a \
	case.a error.a fs.a `cat dns.lib`

spfquery.o: \
compile spfquery.c substdio.h subfd.h stralloc.h gen_alloc.h alloc.h \
spf.h exit.h dns.h conf-spf
	./compile `grep -h -v "^#" conf-spf` spfquery.c

splogger: \
load splogger.o substdio.a error.a str.a fs.a syslog.lib socket.lib
	./load splogger substdio.a error.a str.a fs.a  `cat \
	syslog.lib` `cat socket.lib`

splogger.0: \
splogger.8
	nroff -man splogger.8 > splogger.0

splogger.o: \
compile splogger.c error.h subfd.h substdio.h exit.h str.h \
scan.h fmt.h
	./compile splogger.c

str.a: \
makelib str_len.o str_diff.o str_diffn.o str_cpy.o str_cpyb.o str_chr.o \
str_rchr.o str_chrn.o str_start.o byte_chr.o byte_rchr.o byte_diff.o byte_copy.o \
byte_cspn.o byte_rcspn.o byte_cr.o byte_zero.o
	./makelib str.a str_len.o str_diff.o str_diffn.o str_cpy.o str_cpyb.o \
	str_chr.o str_chrn.o str_rchr.o str_start.o byte_chr.o byte_rchr.o \
	byte_cspn.o byte_rcspn.o byte_diff.o byte_copy.o byte_cr.o byte_zero.o

str_chr.o: \
compile str_chr.c str.h
	./compile str_chr.c

str_chrn.o: \
compile str_chrn.c str.h
	./compile str_chrn.c

str_cpy.o: \
compile str_cpy.c str.h
	./compile str_cpy.c

str_cpyb.o: \
compile str_cpyb.c str.h
	./compile str_cpyb.c

str_diff.o: \
compile str_diff.c str.h
	./compile str_diff.c

str_diffn.o: \
compile str_diffn.c str.h
	./compile str_diffn.c

str_len.o: \
compile str_len.c str.h
	./compile str_len.c

str_rchr.o: \
compile str_rchr.c str.h
	./compile str_rchr.c

str_start.o: \
compile str_start.c str.h
	./compile str_start.c

stralloc.a: \
makelib stralloc_eady.o stralloc_pend.o stralloc_copy.o \
stralloc_opys.o stralloc_opyb.o stralloc_cat.o stralloc_cats.o \
stralloc_catb.o stralloc_arts.o stralloc_num.o
	./makelib stralloc.a stralloc_eady.o stralloc_pend.o \
	stralloc_copy.o stralloc_opys.o stralloc_opyb.o \
	stralloc_cat.o stralloc_cats.o stralloc_catb.o \
	stralloc_arts.o stralloc_num.o

stralloc_num.o: \
compile stralloc_num.c stralloc.h
	./compile stralloc_num.c

stralloc_arts.o: \
compile stralloc_arts.c byte.h str.h stralloc.h gen_alloc.h
	./compile stralloc_arts.c

stralloc_cat.o: \
compile stralloc_cat.c byte.h stralloc.h gen_alloc.h
	./compile stralloc_cat.c

stralloc_catb.o: \
compile stralloc_catb.c stralloc.h gen_alloc.h byte.h
	./compile stralloc_catb.c

stralloc_cats.o: \
compile stralloc_cats.c byte.h str.h stralloc.h gen_alloc.h
	./compile stralloc_cats.c

stralloc_copy.o: \
compile stralloc_copy.c byte.h stralloc.h gen_alloc.h
	./compile stralloc_copy.c

stralloc_eady.o: \
compile stralloc_eady.c alloc.h stralloc.h gen_alloc.h \
gen_allocdefs.h
	./compile stralloc_eady.c

stralloc_opyb.o: \
compile stralloc_opyb.c stralloc.h gen_alloc.h byte.h
	./compile stralloc_opyb.c

stralloc_opys.o: \
compile stralloc_opys.c byte.h str.h stralloc.h gen_alloc.h
	./compile stralloc_opys.c

stralloc_pend.o: \
compile stralloc_pend.c alloc.h stralloc.h gen_alloc.h \
gen_allocdefs.h
	./compile stralloc_pend.c

strerr.a: \
makelib strerr_sys.o strerr_die.o
	./makelib strerr.a strerr_sys.o strerr_die.o

strerr_die.o: \
compile strerr_die.c subfd.h substdio.h exit.h strerr.h
	./compile strerr_die.c

strerr_sys.o: \
compile strerr_sys.c error.h strerr.h
	./compile strerr_sys.c

strsalloc.o: \
compile strsalloc.c alloc.h gen_allocdefs.h stralloc.h strsalloc.h \
gen_alloc.h
	./compile strsalloc.c

subfderr.o: \
compile subfderr.c subfd.h substdio.h
	./compile subfderr.c

subfdin.o: \
compile subfdin.c subfd.h substdio.h
	./compile subfdin.c

subfdins.o: \
compile subfdins.c subfd.h substdio.h
	./compile subfdins.c

subfdout.o: \
compile subfdout.c subfd.h substdio.h
	./compile subfdout.c

subfdouts.o: \
compile subfdouts.c subfd.h substdio.h
	./compile subfdouts.c

subgetopt.o: \
compile subgetopt.c subgetopt.h
	./compile subgetopt.c

substdi.o: \
compile substdi.c substdio.h byte.h error.h
	./compile substdi.c

substdio.a: \
makelib substdio.o substdi.o substdo.o subfderr.o subfdout.o \
subfdouts.o subfdin.o subfdins.o substdio_copy.o
	./makelib substdio.a substdio.o substdi.o substdo.o \
	subfderr.o subfdout.o subfdouts.o subfdin.o subfdins.o \
	substdio_copy.o

substdio.o: \
compile substdio.c substdio.h
	./compile substdio.c

substdio_copy.o: \
compile substdio_copy.c substdio.h
	./compile substdio_copy.c

substdo.o: \
compile substdo.c substdio.h str.h byte.h error.h
	./compile substdo.c

syslog.lib: \
trysyslog.c compile load
	( ( ./compile trysyslog.c && \
	./load trysyslog -lgen ) >/dev/null 2>&1 \
	&& echo -lgen || exit 0 ) > syslog.lib
	rm -f trysyslog.o trysyslog

systype: \
find-systype trycpp.c
	./find-systype > systype

tai.lib: \
tryltai.c compile load
	( ( ./compile tryltai.c && \
	./load tryltai -ltai ) >/dev/null 2>&1 \
	&& echo -ltai || exit 0 ) > tai.lib
	rm -f tryltai.o tryltai

tcp-env: \
load tcp-env.o dns.o remoteinfo.o timeoutread.o timeoutwrite.o \
control.o constmap.o open_read.o variables.o getln.o getln2.o \
socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
scan_xlong.o socket_v4mappedprefix.o socket_tcp.o socket_tcp6.o \
timeoutconn.o ip.o ipalloc.o strsalloc.o case.a ndelay.a sig.a \
env.a getopt.a stralloc.a alloc.a substdio.a \
error.a str.a fs.a dns.lib socket.lib
	./load tcp-env dns.o remoteinfo.o timeoutread.o \
	control.o constmap.o open_read.o variables.o getln.o getln2.o \
	socket_ip4loopback.o socket_v6loopback.o socket_v6any.o \
	scan_xlong.o socket_v4mappedprefix.o socket_tcp.o socket_tcp6.o \
	timeoutwrite.o timeoutconn.o ip.o ipalloc.o strsalloc.o \
	case.a ndelay.a sig.a env.a getopt.a stralloc.a alloc.a \
	substdio.a error.a str.a fs.a `cat dns.lib socket.lib`

tcp-env.0: \
tcp-env.1
	nroff -man tcp-env.1 > tcp-env.0

tcp-env.o: \
compile tcp-env.c sig.h strsalloc.h stralloc.h gen_alloc.h str.h env.h fmt.h \
scan.h subgetopt.h ip.h dns.h byte.h remoteinfo.h exit.h case.h \
conf-spf conf-ip
	./compile `grep -h -v "^#" conf-spf conf-ip` tcp-env.c

tcp-environ.0: \
tcp-environ.5
	nroff -man tcp-environ.5 > tcp-environ.0

tcpto.o: \
compile tcpto.c tcpto.h open.h lock.h seek.h now.h datetime.h ip.h fmt.h \
byte.h datetime.h env.h variables.h hasindimail.h ipalloc.h tcpto.h \
conf-tls conf-ip
	./compile $(inc_deps) `grep -h -v "^#" conf-tls conf-ip` tcpto.c

tcpto_clean.o: \
compile tcpto_clean.c tcpto.h open.h substdio.h ipalloc.h \
tcpto.h ip.h ipalloc.h conf-tls conf-ip
	./compile `grep -h -v "^#" conf-tls conf-ip` tcpto_clean.c

timeoutconn.o: \
compile timeoutconn.c ndelay.h select.h error.h ip.h \
byte.h timeoutconn.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` timeoutconn.c

timeoutread.o: \
compile timeoutread.c timeoutread.h select.h error.h
	./compile timeoutread.c

timeoutwrite.o: \
compile timeoutwrite.c timeoutwrite.h select.h error.h
	./compile timeoutwrite.c

token822.o: \
compile token822.c stralloc.h gen_alloc.h alloc.h str.h token822.h \
gen_alloc.h gen_allocdefs.h
	./compile token822.c

trigger.o: \
compile trigger.c select.h open.h trigger.h hasnpbg1.h
	./compile trigger.c

triggerpull.o: \
compile triggerpull.c ndelay.h open.h triggerpull.h
	./compile triggerpull.c

uint32.h: \
tryulong32.c compile load uint32.h1 uint32.h2
	( ( ./compile tryulong32.c && ./load tryulong32 && \
	./tryulong32 ) >/dev/null 2>&1 \
	&& cat uint32.h2 || cat uint32.h1 ) > uint32.h
	rm -f tryulong32.o tryulong32

uint64.h: \
tryulong64.c compile load uint64.h1 uint64.h2
	( ( ./compile tryulong64.c && ./load tryulong64 && \
	./tryulong64 ) >/dev/null 2>&1 \
	&& cat uint64.h1 || cat uint64.h2 ) > uint64.h
	rm -f tryulong64.o tryulong64

wait.a: \
makelib wait_pid.o wait_nohang.o
	./makelib wait.a wait_pid.o wait_nohang.o

wait_nohang.o: \
compile wait_nohang.c haswaitp.h
	./compile wait_nohang.c

wait_pid.o: \
compile wait_pid.c error.h haswaitp.h
	./compile wait_pid.c

wildmat.o: \
compile wildmat.c hasindimail.h
	./compile $(inc_deps) wildmat.c

variables.o: \
compile variables.c variables.h
	./compile variables.c

etrn.o: \
compile etrn.c etrn.h rcpthosts.h case.h sig.h stralloc.h constmap.h wait.h \
auto_qmail.h hasindimail.h control.h fmt.h str.h
	./compile $(inc_deps) etrn.c

atrn.o: \
compile atrn.c etrn.h rcpthosts.h case.h sig.h stralloc.h constmap.h wait.h \
hasindimail.h control.h str.h
	./compile $(inc_deps) atrn.c

############### Begin queue-fix

queue-fix: \
load queue-fix.o fifo.o get_uid.o fs.a stralloc.a getln.a open.a error.a \
strerr.a substdio.a alloc.a auto_split.o auto_uids.o str.a
	./load queue-fix fifo.o get_uid.o strerr.a fs.a stralloc.a getln.a open.a error.a \
	substdio.a alloc.a auto_split.o auto_uids.o str.a

queue-fix.o: \
compile queue-fix.c direntry.h stralloc.h fmt.h error.h subfd.h getln.h str.h open.h fifo.h \
scan.h auto_split.h auto_uids.h tcpto.h
	./compile queue-fix.c

plainencode: \
load plainencode.o base64.o stralloc.a alloc.a error.o str.a
	./load plainencode base64.o stralloc.a alloc.a error.o str.a

plainencode.o: \
compile plainencode.c stralloc.h base64.h
	./compile plainencode.c

b64encode: \
load b64encode.o base64.o stralloc.a alloc.a error.o str.a
	./load b64encode base64.o stralloc.a alloc.a error.o str.a

b64encode.o: \
compile b64encode.c stralloc.h base64.h
	./compile b64encode.c

b64decode: \
load b64decode.o base64.o stralloc.a alloc.a error.o str.a
	./load b64decode base64.o stralloc.a alloc.a error.o str.a

b64decode.o: \
compile b64decode.c stralloc.h base64.h
	./compile b64decode.c

maildirdeliver: \
load maildirdeliver.o now.o envread.o substdio.a strerr.a error.a fs.a \
open.a sig.a str.a
	./load maildirdeliver now.o envread.o strerr.a error.a fs.a \
	open.a sig.a substdio.a str.a

maildirdeliver.0: \
maildirdeliver.1
	nroff -man maildirdeliver.1 > maildirdeliver.0

maildirdeliver.o: \
compile maildirdeliver.c byte.h datetime.h error.h exit.h fmt.h now.h open.h \
sig.h str.h strerr.h substdio.h env.h
	./compile maildirdeliver.c

qmail-rm: load qmail-rm.o auto_split.o auto_qmail.o error_str.o error.o fmt_str.o \
	open_write.o lock_exnb.o fmt_ulong.o stralloc.a substdio.a str.a alloc.a
	./load qmail-rm auto_split.o auto_qmail.o error_str.o error.o fmt_str.o \
	open_write.o lock_exnb.o fmt_ulong.o stralloc.a substdio.a str.a alloc.a

qmail-rm.o: \
compile qmail-rm.c auto_split.h stralloc.h open.h lock.h auto_qmail.h substdio.h fmt.h error.h
	./compile qmail-rm.c

qmail-rm.0: qmail-rm.8
	nroff -man qmail-rm.8 > qmail-rm.0

qmail-rm.8: qmail-rm.9
	cat qmail-rm.9 \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> qmail-rm.8

qmail-autoresponder: load qmail-autoresponder.o alloc.o error.o error_str.o \
alloc_re.o case_diffs.o case_diffb.o case_lowers.o env.o envread.o auto_qmail.o subgetopt.o \
open_read.o scan_ulong.o variables.o control.o quote.o tai_now.o fmt_ulong.o \
ip.o ipme.o ipalloc.o scan_xlong.o socket_v4mappedprefix.o socket_v6any.o \
datetime.o date822fmt.o datetime_un.o fmt_uint.o fmt_uint0.o now.o \
fmt_str.o myctime.o sgetopt.o getln.a mess822.a strerr.a substdio.a stralloc.a \
str.a
	./load qmail-autoresponder alloc.o error.o error_str.o alloc_re.o \
	case_diffs.o case_diffb.o case_lowers.o env.o envread.o auto_qmail.o now.o \
	open_read.o scan_ulong.o variables.o control.o subgetopt.o \
	quote.o tai_now.o fmt_ulong.o fmt_str.o myctime.o sgetopt.o \
	datetime.o date822fmt.o datetime_un.o fmt_uint.o fmt_uint0.o \
	ip.o ipme.o ipalloc.o scan_xlong.o socket_v4mappedprefix.o socket_v6any.o \
	getln.a mess822.a strerr.a substdio.a stralloc.a \
	str.a

qmail-autoresponder.o: \
compile qmail-autoresponder.c substdio.h subfd.h mess822.h date822fmt.h \
getln.h stralloc.h env.h case.h auto_qmail.h str.h strerr.h control.h \
error.h alloc.h ip.h byte.h ipme.h fmt.h quote.h tai.h sgetopt.h conf-mime
	./compile -DMAKE_SEEKABLE -DQUOTE `grep -h -v "^#" conf-mime` qmail-autoresponder.c

qmail-autoresponder.0: \
qmail-autoresponder.1
	nroff -man qmail-autoresponder.1 > qmail-autoresponder.0

822field: \
load 822field.o mess822.a getln.a strerr.a substdio.a stralloc.a \
alloc.a error.a str.a case.a fs.a
	./load 822field mess822.a getln.a strerr.a substdio.a \
	stralloc.a alloc.a error.a str.a case.a fs.a 

822field.0: \
822field.1
	nroff -man 822field.1 > 822field.0

822field.o: \
compile 822field.c strerr.h subfd.h substdio.h getln.h \
mess822.h stralloc.h gen_alloc.h caltime.h caldate.h exit.h
	./compile 822field.c

822header: \
load 822header.o mess822.a getln.a strerr.a substdio.a stralloc.a \
alloc.a error.a str.a getopt.a case.a
	./load 822header mess822.a getln.a strerr.a substdio.a \
	getopt.a stralloc.a alloc.a error.a str.a case.a

822header.0: \
822header.1
	nroff -man 822header.1 > 822header.0

822header.o: \
compile 822header.c strerr.h subfd.h substdio.h getln.h \
mess822.h stralloc.h gen_alloc.h caltime.h caldate.h exit.h uint64.h \
sgetopt.h str.h case.h
	./compile 822header.c

822body: load 822body.o byte_cr.o str_len.o strerr.a error.a getln.a stralloc.a \
alloc.a str.a substdio.a mess822.a
	./load 822body byte_cr.o str_len.o strerr.a error.a getln.a stralloc.a alloc.a \
	str.a substdio.a mess822.a 

822body.o: compile \
822body.c \
substdio.h \
strerr.h error.h fd.h getln.h stralloc.h gen_alloc.h mess822.h \
caltime.h caldate.h exit.h
	./compile 822body.c

mess822.a: \
makelib mess822_date.o mess822_quote.o mess822_fold.o mess822_token.o \
mess822_addr.o mess822_when.o mess822_line.o mess822_ok.o
	./makelib mess822.a mess822_date.o mess822_quote.o \
	mess822_fold.o mess822_token.o mess822_addr.o \
	mess822_when.o mess822_line.o mess822_ok.o

############### serialsmtp ###################################

serialsmtp: \
load serialsmtp.o timeoutread.o timeoutwrite.o quote.o case.a env.a \
sig.a getln.a stralloc.a strerr.a substdio.a alloc.a error.a str.a \
wait.a open.a fd.a fs.a
	./load serialsmtp timeoutread.o timeoutwrite.o quote.o \
	case.a env.a sig.a getln.a stralloc.a strerr.a substdio.a \
	alloc.a error.a str.a wait.a open.a fd.a fs.a 

serialsmtp.0: \
serialsmtp.1
	nroff -man serialsmtp.1 > serialsmtp.0

serialsmtp.o: \
compile serialsmtp.c strerr.h getln.h subfd.h substdio.h \
open.h timeoutread.h timeoutwrite.h exit.h stralloc.h \
gen_alloc.h sig.h str.h case.h quote.h scan.h env.h byte.h conf-ip
	./compile `grep -h -v "^#" conf-ip` serialsmtp.c

maildirserial: \
load maildirserial.o auto_qmail.o qmail.o maildir.o sconfig.o quote.o \
prioq.o now.o env.a getopt.a mess822.a libtai.a getln.a strerr.a \
substdio.a stralloc.a alloc.a error.a str.a open.a wait.a sig.a fd.a \
fs.a
	./load maildirserial auto_qmail.o qmail.o maildir.o \
	sconfig.o quote.o prioq.o now.o env.a getopt.a mess822.a \
	libtai.a getln.a strerr.a substdio.a stralloc.a alloc.a \
	error.a str.a open.a wait.a sig.a fd.a fs.a 

maildirserial.0: \
maildirserial.1
	nroff -man maildirserial.1 > maildirserial.0

maildirserial.o: \
compile maildirserial.c sgetopt.h subgetopt.h scan.h stralloc.h \
gen_alloc.h fd.h open.h getln.h subfd.h strerr.h \
maildir.h prioq.h datetime.h \
wait.h sig.h str.h fmt.h tai.h uint64.h mess822.h \
caltime.h caldate.h now.h datetime.h sconfig.h \
qmail.h substdio.h auto_qmail.h conf-mime
	./compile `grep -h -v "^#" conf-mime` maildirserial.c

serialcmd: \
now.o stralloc_num.o \
load serialcmd.o timeoutread.o timeoutwrite.o quote.o env.o str_cpy.o \
case.a env.a sig.a getln.a stralloc.a strerr.a substdio.a alloc.a error.a \
wait.a open.a fd.a fs.a str.a
	./load serialcmd timeoutread.o timeoutwrite.o quote.o env.o str_cpy.o \
	now.o stralloc_num.o \
	case.a env.a sig.a getln.a stralloc.a strerr.a substdio.a \
	alloc.a error.a wait.a open.a fd.a fs.a str.a

serialcmd.o: \
compile serialcmd.c strerr.h getln.h subfd.h substdio.h now.h open.h \
stralloc.h sig.h str.h byte.h env.h quote.h
	./compile serialcmd.c

serialcmd.0: serialcmd.1
	nroff -man serialcmd.1 > serialcmd.0

maildircmd: \
warn-auto.sh maildircmd.sh conf-qmail
	cat warn-auto.sh maildircmd.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> maildircmd

maildircmd.0: \
maildircmd.1
	nroff -man maildircmd.1 > maildircmd.0

rewind: \
load seek_set.o rewind.o
	./load rewind seek_set.o

rewind.o: \
compile rewind.c seek.h
	./compile rewind.c

rewind.0: rewind.1
	nroff -man rewind.1 > rewind.0

serialqmtp: \
load serialqmtp.o timeoutread.o timeoutwrite.o env.a sig.a getln.a \
stralloc.a strerr.a substdio.a alloc.a error.a str.a wait.a open.a \
fd.a fs.a
	./load serialqmtp timeoutread.o timeoutwrite.o env.a sig.a \
	getln.a stralloc.a strerr.a substdio.a alloc.a error.a \
	str.a wait.a open.a fd.a fs.a 

serialqmtp.0: \
serialqmtp.1
	nroff -man serialqmtp.1 > serialqmtp.0

serialqmtp.o: \
compile serialqmtp.c strerr.h getln.h substdio.h stralloc.h \
gen_alloc.h subfd.h sig.h timeoutread.h \
timeoutwrite.h fd.h exit.h open.h wait.h str.h fmt.h env.h
	./compile serialqmtp.c

maildirsmtp: \
warn-auto.sh maildirsmtp.sh conf-qmail
	cat warn-auto.sh maildirsmtp.sh \
	| sed s}HOME}"`head -1 conf-qmail`"}g \
	> maildirsmtp

maildirsmtp.0: \
maildirsmtp.1
	nroff -man maildirsmtp.1 > maildirsmtp.0

maildirqmtp: \
warn-auto.sh maildirqmtp.sh conf-qmail
	cat warn-auto.sh maildirqmtp.sh \
	| sed s}HOME}"`head -1 conf-qmail`"}g \
	> maildirqmtp

maildirqmtp.0: \
maildirqmtp.1
	nroff -man maildirqmtp.1 > maildirqmtp.0

config.0: \
config.3
	nroff -man config.3 > config.0

sconfig.o: \
compile sconfig.c open.h substdio.h error.h getln.h \
stralloc.h gen_alloc.h sconfig.h stralloc.h env.h
	./compile sconfig.c

caldate.0: \
caldate.3
	nroff -man caldate.3 > caldate.0

caldate_fmjd.o: \
compile caldate_fmjd.c caldate.h
	./compile caldate_fmjd.c

caldate_mjd.0: \
caldate_mjd.3
	nroff -man caldate_mjd.3 > caldate_mjd.0

caltime.0: \
caltime.3
	nroff -man caltime.3 > caltime.0

caltime_tai.0: \
caltime_tai.3
	nroff -man caltime_tai.3 > caltime_tai.0

caltime_utc.o: \
compile caltime_utc.c tai.h uint64.h leapsecs.h caldate.h caltime.h \
caldate.h
	./compile caltime_utc.c

leapsecs.0: \
leapsecs.3
	nroff -man leapsecs.3 > leapsecs.0

leapsecs_init.o: \
compile leapsecs_init.c leapsecs.h
	./compile leapsecs_init.c

leapsecs_read.o: \
compile leapsecs_read.c tai.h uint64.h leapsecs.h
	./compile leapsecs_read.c

leapsecs_sub.o: \
compile leapsecs_sub.c leapsecs.h tai.h uint64.h
	./compile leapsecs_sub.c

libtai.a: \
makelib tai_now.o tai_unpack.o caldate_fmjd.o leapsecs_read.o \
leapsecs_init.o leapsecs_sub.o caltime_utc.o tai_pack.o caltime_fmt.o \
caltime_tai.o caldate_fmt.o caldate_mjd.o leapsecs_add.o
	./makelib libtai.a tai_now.o tai_unpack.o caldate_fmjd.o \
	leapsecs_read.o leapsecs_init.o leapsecs_sub.o caltime_utc.o tai_pack.o \
	caltime_fmt.o caltime_tai.o caldate_fmt.o caldate_mjd.o leapsecs_add.o

tai.0: \
tai.3
	nroff -man tai.3 > tai.0

tai_now.0: \
tai_now.3
	nroff -man tai_now.3 > tai_now.0

tai_pack.0: \
tai_pack.3
	nroff -man tai_pack.3 > tai_pack.0

mess822_date.0: \
mess822_date.3
	nroff -man mess822_date.3 > mess822_date.0

###### qmailanalog

matchup: \
load matchup.o strerr.a getln.a substdio.a stralloc.a alloc.a error.a \
str.a fs.a case.a
	./load matchup strerr.a getln.a substdio.a stralloc.a \
	alloc.a error.a str.a fs.a case.a 

matchup.0: \
matchup.1
	nroff -man matchup.1 > matchup.0

matchup.o: \
compile matchup.c stralloc.h gen_alloc.h gen_alloc.h gen_allocdefs.h \
strerr.h getln.h subfd.h substdio.h exit.h alloc.h \
str.h fmt.h scan.h case.h
	./compile matchup.c

mlmatchup: \
load mlmatchup.o strerr.a getln.a substdio.a stralloc.a alloc.a error.a \
str.a fs.a case.a
	./load mlmatchup strerr.a getln.a substdio.a stralloc.a \
	alloc.a error.a str.a fs.a case.a

mlmatchup.0: \
mlmatchup.1
	nroff -man mlmatchup.1 > mlmatchup.0

mlmatchup.o: \
compile mlmatchup.c stralloc.h gen_alloc.h gen_alloc.h gen_allocdefs.h \
strerr.h getln.h substdio.h subfd.h substdio.h exit.h \
str.h fmt.h scan.h case.h
	./compile mlmatchup.c
	

successes: \
warn-auto.sh successes.sh conf-qmail
	cat warn-auto.sh successes.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> successes

suids: \
warn-auto.sh suids.sh conf-qmail
	cat warn-auto.sh suids.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> suids

xqp: \
warn-auto.sh xqp.sh conf-qmail
	cat warn-auto.sh xqp.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> xqp

xqp.0: \
xqp.1
	nroff -man xqp.1 > xqp.0

xrecipient: \
warn-auto.sh xrecipient.sh conf-qmail
	cat warn-auto.sh xrecipient.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> xrecipient

xrecipient.0: \
xrecipient.1
	nroff -man xrecipient.1 > xrecipient.0

xsender: \
warn-auto.sh xsender.sh conf-qmail
	cat warn-auto.sh xsender.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> xsender

xsender.0: \
xsender.1
	nroff -man xsender.1 > xsender.0

zddist: \
warn-auto.sh zddist.sh conf-qmail
	cat warn-auto.sh zddist.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zddist

ddist: \
warn-auto.sh ddist.sh conf-qmail
	cat warn-auto.sh ddist.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> ddist

zdeferrals: \
warn-auto.sh zdeferrals.sh conf-qmail
	cat warn-auto.sh zdeferrals.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zdeferrals

deferrals: warn-auto.sh deferrals.sh conf-qmail
	cat warn-auto.sh deferrals.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> deferrals

zfailures: \
warn-auto.sh zfailures.sh conf-qmail
	cat warn-auto.sh zfailures.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zfailures

failures: \
warn-auto.sh failures.sh conf-qmail
	cat warn-auto.sh failures.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> failures

zoverall: \
warn-auto.sh zoverall.sh conf-qmail
	cat warn-auto.sh zoverall.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zoverall

zrecipients: \
warn-auto.sh zrecipients.sh conf-qmail
	cat warn-auto.sh zrecipients.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zrecipients

zrhosts: \
warn-auto.sh zrhosts.sh conf-qmail
	cat warn-auto.sh zrhosts.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zrhosts

zsmtp: \
warn-auto.sh zsmtp.sh conf-qmail
	cat warn-auto.sh zsmtp.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zsmtp

zrxdelay: \
warn-auto.sh zrxdelay.sh conf-qmail
	cat warn-auto.sh zrxdelay.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zrxdelay

zsenders: \
warn-auto.sh zsenders.sh conf-qmail
	cat warn-auto.sh zsenders.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zsenders

zsendmail: \
warn-auto.sh zsendmail.sh conf-qmail
	cat warn-auto.sh zsendmail.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zsendmail

zsuccesses: \
warn-auto.sh zsuccesses.sh conf-qmail
	cat warn-auto.sh zsuccesses.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zsuccesses

zsuids: \
warn-auto.sh zsuids.sh conf-qmail
	cat warn-auto.sh zsuids.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zsuids

multilog-matchup: \
warn-auto.sh multilog-matchup.sh conf-qmail
	cat warn-auto.sh multilog-matchup.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> multilog-matchup

smtp-matchup: \
warn-auto.sh smtp-matchup.sh conf-qmail
	cat warn-auto.sh smtp-matchup.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> smtp-matchup

recipients: \
warn-auto.sh recipients.sh conf-qmail
	cat warn-auto.sh recipients.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> recipients

rhosts: \
warn-auto.sh rhosts.sh conf-qmail
	cat warn-auto.sh rhosts.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rhosts

envmigrate: \
warn-auto.sh envmigrate.sh
	cat warn-auto.sh envmigrate.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> envmigrate

rsmtp: \
warn-auto.sh rsmtp.sh conf-qmail
	cat warn-auto.sh rsmtp.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rsmtp

rsmtpfailures: \
warn-auto.sh rsmtpfailures.sh conf-qmail
	cat warn-auto.sh rsmtpfailures.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rsmtpfailures

rsmtpsdomains: \
warn-auto.sh rsmtpsdomains.sh conf-qmail
	cat warn-auto.sh rsmtpsdomains.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rsmtpsdomains

rsmtprdomains: \
warn-auto.sh rsmtprdomains.sh conf-qmail
	cat warn-auto.sh rsmtprdomains.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rsmtprdomains

rsmtprecipients: \
warn-auto.sh rsmtprecipients.sh conf-qmail
	cat warn-auto.sh rsmtprecipients.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rsmtprecipients

rsmtpsenders: \
warn-auto.sh rsmtpsenders.sh conf-qmail
	cat warn-auto.sh rsmtpsenders.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rsmtpsenders

rxdelay: \
warn-auto.sh rxdelay.sh conf-qmail
	cat warn-auto.sh rxdelay.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rxdelay

rspamrdomain: \
warn-auto.sh rspamrdomain.sh conf-qmail
	cat warn-auto.sh rspamrdomain.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rspamrdomain

rspamsdomain: \
warn-auto.sh rspamsdomain.sh conf-qmail
	cat warn-auto.sh rspamsdomain.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rspamsdomain

zspam: \
warn-auto.sh zspam.sh conf-qmail
	cat warn-auto.sh zspam.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> zspam

dk-filter: \
warn-auto.sh dk-filter.sh conf-qmail
	cat warn-auto.sh dk-filter.sh \
	| sed s}QMAILHOME}"`head -1 conf-qmail`"}g \
	> dk-filter

scan_uint.o: \
compile scan_uint.c scan.h
	./compile scan_uint.c

scan_ushort.o: \
compile scan_ushort.c scan.h
	./compile scan_ushort.c

senders: \
warn-auto.sh senders.sh conf-qmail
	cat warn-auto.sh senders.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> senders

fmt_ushort.o: \
compile fmt_ushort.c fmt.h
	./compile fmt_ushort.c

columnt: \
load columnt.o slurpclose.o strerr.a substdio.a stralloc.a alloc.a \
error.a str.a
	./load columnt slurpclose.o strerr.a substdio.a stralloc.a \
	alloc.a error.a str.a 

columnt.0: \
columnt.1
	nroff -man columnt.1 > columnt.0

columnt.o: \
compile columnt.c stralloc.h gen_alloc.h alloc.h slurpclose.h \
strerr.h substdio.h exit.h
	./compile columnt.c

qmail-cat: \
load  qmail-cat.o error.o error_str.o byte_copy.o str_len.o byte_cr.o \
byte_chr.o getln.o getln2.o alloc.o alloc_re.o stralloc.a substdio.a
	./load qmail-cat error.o error_str.o byte_copy.o str_len.o byte_cr.o \
	byte_chr.o getln.o getln2.o alloc.o alloc_re.o stralloc.a substdio.a

qmail-cat.o: \
compile qmail-cat.c substdio.h error.h stralloc.h getln.h
	./compile qmail-cat.c

qmail-poppass: \
load  qmail-poppass.o error.o error_str.o byte_copy.o str_len.o byte_cr.o \
sig_catch.o byte_chr.o getln.o getln2.o str_diffn.o alloc.o alloc_re.o \
fmt_ulong.o auto_uids.o wait_pid.o byte_zero.o sig_pipe.o \
MakeArgs.o envread.o stralloc.a substdio.a
	./load qmail-poppass error.o error_str.o byte_copy.o str_len.o byte_cr.o \
	byte_chr.o getln.o getln2.o str_diffn.o alloc.o alloc_re.o sig_pipe.o \
	fmt_ulong.o auto_uids.o wait_pid.o byte_zero.o sig_catch.o \
	MakeArgs.o envread.o stralloc.a substdio.a

qmail-poppass.o: \
compile qmail-poppass.c substdio.h error.h stralloc.h getln.h str.h sig.h wait.h byte.h \
auto_uids.h fmt.h env.h
	./compile qmail-poppass.c

rpmattr: \
load  rpmattr.o auto_uids.o str_diffn.o error.o error_str.o strerr.a \
substdio.a str_len.o get_uid.o byte_copy.o fmt_ulong.o str_diff.o
	./load rpmattr auto_uids.o str_diffn.o error.o error_str.o strerr.a \
		substdio.a str_len.o get_uid.o byte_copy.o fmt_ulong.o str_diff.o

rpmattr.o: compile rpmattr.c substdio.h strerr.h auto_uids.h substdio.h str.h
	./compile rpmattr.c

rspamstat: \
warn-auto.sh rspamstat.sh conf-qmail
	cat warn-auto.sh rspamstat.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rspamstat

rspamhist: \
warn-auto.sh rspamhist.sh conf-qmail
	cat warn-auto.sh rspamhist.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> rspamhist

iftocc: load iftocc.o mess822.a getln.a strerr.a substdio.a stralloc.a \
alloc.a error.a env.a str.a case.a fs.a open.a
	./load iftocc mess822.a getln.a strerr.a substdio.a \
	stralloc.a alloc.a error.a env.a str.a case.a fs.a open.a 

iftocc.0: iftocc.1
	nroff -man iftocc.1 > iftocc.0

iftocc.o: compile iftocc.c strerr.h subfd.h substdio.h getln.h \
mess822.h stralloc.h gen_alloc.h caltime.h caldate.h case.h env.h \
exit.h
	./compile iftocc.c

ofmipd: load ofmipd.o rewritehost.o rwhconfig.o sconfig.o qmail.o auto_qmail.o \
timeoutread.o timeoutwrite.o commands.o base64.o now.o env.a cdb.a mess822.a \
libtai.a getln.a strerr.a substdio.a stralloc.a alloc.a error.a \
case.a str.a fs.a open.a wait.a sig.a fd.a
	./load ofmipd rewritehost.o rwhconfig.o sconfig.o qmail.o \
	auto_qmail.o timeoutread.o timeoutwrite.o commands.o base64.o now.o env.a \
	cdb.a mess822.a libtai.a getln.a strerr.a substdio.a \
	stralloc.a alloc.a error.a case.a str.a fs.a open.a wait.a \
	sig.a fd.a 

ofmipd.0: ofmipd.8
	nroff -man ofmipd.8 > ofmipd.0

ofmipd.o: compile ofmipd.c commands.h sig.h auto_qmail.h qmail.h substdio.h \
timeoutread.h timeoutwrite.h stralloc.h gen_alloc.h \
sconfig.h env.h exit.h error.h str.h mess822.h \
caltime.h caldate.h tai.h uint64.h caltime.h cdb.h \
fmt.h now.h case.h base64.h wait.h \
uint32.h byte.h getln.h open.h leapsecs.h rewritehost.h rwhconfig.h
	./compile ofmipd.c

rewritehost.0: rewritehost.3
	nroff -man rewritehost.3 > rewritehost.0

rewritehost.o: compile rewritehost.c stralloc.h gen_alloc.h str.h case.h \
rewritehost.h byte.h
	./compile rewritehost.c

rewriting.0: rewriting.5
	nroff -man rewriting.5 > rewriting.0

rwhconfig.o: compile rwhconfig.c rewritehost.h stralloc.h gen_alloc.h sconfig.h \
stralloc.h strerr.h rwhconfig.h strerr.h auto_qmail.h
	./compile rwhconfig.c

ofmipname: load ofmipname.o cdbmss.o cdbmake.a strerr.a getln.a substdio.a \
stralloc.a alloc.a error.a seek.a open.a str.a
	./load ofmipname cdbmss.o cdbmake.a strerr.a getln.a \
	substdio.a stralloc.a alloc.a error.a seek.a open.a str.a 

ofmipname.0: \
ofmipname.8
	nroff -man ofmipname.8 > ofmipname.0

ofmipname.o: \
compile ofmipname.c cdbmss.h cdbmake.h uint32.h substdio.h strerr.h \
open.h subfd.h stralloc.h gen_alloc.h getln.h \
exit.h byte.h
	./compile ofmipname.c

new-inject: \
load new-inject.o qmail.o auto_qmail.o rewritehost.o rwhconfig.o \
constmap.o sconfig.o env.a getopt.a mess822.a getln.a strerr.a \
substdio.a stralloc.a alloc.a error.a sig.a fd.a wait.a open.a case.a \
str.a fs.a libtai.a
	./load new-inject qmail.o auto_qmail.o rewritehost.o \
	rwhconfig.o constmap.o sconfig.o env.a getopt.a mess822.a \
	getln.a strerr.a substdio.a stralloc.a alloc.a error.a \
	sig.a fd.a wait.a open.a case.a str.a fs.a libtai.a 

new-inject.0: \
new-inject.1
	nroff -man new-inject.1 > new-inject.0

new-inject.o: \
compile new-inject.c subfd.h substdio.h getln.h mess822.h \
stralloc.h gen_alloc.h caltime.h caldate.h strerr.h exit.h caltime.h \
leapsecs.h tai.h uint64.h sgetopt.h subgetopt.h stralloc.h sconfig.h \
stralloc.h auto_qmail.h case.h constmap.h qmail.h sig.h \
rewritehost.h rwhconfig.h strerr.h str.h
	./compile new-inject.c

tai_now.o: compile tai.h tai_now.c uint64.h
	./compile tai_now.c

tai_pack.o: compile tai_pack.c tai.h uint64.h
	./compile tai_pack.c

822date: load 822date.o libtai.a mess822.a getln.a strerr.a substdio.a \
stralloc.a alloc.a error.a str.a case.a fs.a
	./load 822date libtai.a mess822.a getln.a strerr.a \
	substdio.a stralloc.a alloc.a error.a str.a case.a fs.a 

822date.0: 822date.1
	nroff -man 822date.1 > 822date.0

822date.o: compile 822date.c strerr.h subfd.h substdio.h getln.h \
mess822.h stralloc.h gen_alloc.h caltime.h caldate.h exit.h \
leapsecs.h caltime.h tai.h uint64.h
	./compile 822date.c

caltime_fmt.o: compile caltime_fmt.c caldate.h caltime.h caldate.h
	./compile caltime_fmt.c

caltime_tai.o: compile caltime_tai.c tai.h uint64.h leapsecs.h caldate.h caltime.h \
caldate.h
	./compile caltime_tai.c

caldate_fmt.o: compile caldate_fmt.c caldate.h
	./compile caldate_fmt.c

caldate_mjd.o: compile caldate_mjd.c caldate.h
	./compile caldate_mjd.c

leapsecs_add.o: compile leapsecs_add.c leapsecs.h tai.h uint64.h
	./compile leapsecs_add.c

822received: load 822received.o libtai.a mess822.a getln.a strerr.a substdio.a \
stralloc.a alloc.a error.a str.a case.a fs.a
	./load 822received libtai.a mess822.a getln.a strerr.a \
	substdio.a stralloc.a alloc.a error.a str.a case.a fs.a 

822received.0: 822received.1
	nroff -man 822received.1 > 822received.0

822received.o: compile 822received.c strerr.h subfd.h substdio.h getln.h \
mess822.h stralloc.h gen_alloc.h caltime.h caldate.h exit.h \
leapsecs.h caltime.h tai.h uint64.h case.h
	./compile 822received.c

822print: load 822print.o libtai.a mess822.a getln.a strerr.a substdio.a \
stralloc.a alloc.a error.a str.a case.a fs.a
	./load 822print libtai.a mess822.a getln.a strerr.a \
	substdio.a stralloc.a alloc.a error.a str.a case.a fs.a 

822print.0: 822print.1
	nroff -man 822print.1 > 822print.0

822print.o: compile 822print.c subfd.h substdio.h getln.h mess822.h \
stralloc.h gen_alloc.h caltime.h caldate.h strerr.h exit.h leapsecs.h \
caltime.h tai.h uint64.h byte.h str.h
	./compile 822print.c

parsedate: load parsedate.o libtai.a mess822.a getln.a strerr.a substdio.a \
stralloc.a alloc.a error.a str.a case.a fs.a
	./load parsedate libtai.a mess822.a getln.a strerr.a \
	substdio.a stralloc.a alloc.a error.a str.a case.a fs.a 

parsedate.o: compile parsedate.c strerr.h subfd.h substdio.h getln.h \
mess822.h stralloc.h gen_alloc.h caltime.h caldate.h leapsecs.h \
caltime.h tai.h uint64.h
	./compile parsedate.c

tokenize: load tokenize.o mess822.a getln.a strerr.a substdio.a stralloc.a \
alloc.a error.a str.a
	./load tokenize mess822.a getln.a strerr.a substdio.a \
	stralloc.a alloc.a error.a str.a 

tokenize.o: compile tokenize.c strerr.h subfd.h substdio.h getln.h \
mess822.h stralloc.h gen_alloc.h caltime.h caldate.h
	./compile tokenize.c

addrlist: load addrlist.o mess822.a getln.a strerr.a substdio.a stralloc.a \
alloc.a error.a str.a
	./load addrlist mess822.a getln.a strerr.a substdio.a \
	stralloc.a alloc.a error.a str.a 

addrlist.o: compile addrlist.c strerr.h subfd.h substdio.h getln.h \
mess822.h stralloc.h gen_alloc.h caltime.h caldate.h
	./compile addrlist.c

qquote: load qquote.o mess822.a strerr.a substdio.a stralloc.a alloc.a error.a \
str.a
	./load qquote mess822.a strerr.a substdio.a stralloc.a \
	alloc.a error.a str.a 

qquote.o: compile qquote.c mess822.h stralloc.h gen_alloc.h caltime.h caldate.h \
subfd.h strerr.h
	./compile qquote.c

822body.0: 822body.1
	nroff -man 822body.1 > 822body.0

822headerok.0: 822headerok.1
	nroff -man 822headerok.1 > 822headerok.0

822bodyfilter.0: 822bodyfilter.1
	nroff -man 822bodyfilter.1 > 822bodyfilter.0

822headerfilter.0: 822headerfilter.1
	nroff -man 822headerfilter.1 > 822headerfilter.0

822addr.0: 822addr.1
	nroff -man 822addr.1 > 822addr.0

822fields.0: 822fields.1
	nroff -man 822fields.1 > 822fields.0

checkaddr.0: checkaddr.1
	nroff -man checkaddr.1 > checkaddr.0

checkdomain.0: checkdomain.1
	nroff -man checkdomain.1 > checkdomain.0

filterto.0: filterto.1
	nroff -man filterto.1 > filterto.0

condtomaildir.0: condtomaildir.1
	nroff -man condtomaildir.1 > condtomaildir.0

iftoccfrom.0: iftoccfrom.1
	nroff -man iftoccfrom.1 > iftoccfrom.0

ifaddr.0: ifaddr.1
	nroff -man ifaddr.1 > ifaddr.0

replier.0: replier.1
	nroff -man replier.1 > replier.0

replier-config.0: replier-config.1
	nroff -man replier-config.1 > replier-config.0


822headerok: load 822headerok.o getln.a substdio.a str.a stralloc.a alloc.a \
error.a mess822.a
	./load 822headerok getln.a substdio.a str.a stralloc.a alloc.a error.a \
	mess822.a 

822headerok.o: compile 822headerok.c \
error.h getln.h substdio.h stralloc.h gen_alloc.h mess822.h caltime.h \
caldate.h exit.h
	./compile 822headerok.c

822bodyfilter: load 822bodyfilter.o strerr.a getln.a substdio.a pathexec_run.o \
env.a stralloc.a alloc.a str.a mess822.a fd.a error.a seek.a
	./load 822bodyfilter pathexec_run.o strerr.a getln.a substdio.a env.a \
	stralloc.a alloc.a str.a mess822.a fd.a error.a seek.a 

822bodyfilter.o: compile 822bodyfilter.c \
exit.h error.h strerr.h substdio.h getln.h stralloc.h \
gen_alloc.h mess822.h caltime.h caldate.h fd.h seek.h pathexec.h
	./compile 822bodyfilter.c

pathexec_run.o: compile pathexec_run.c \
error.h stralloc.h gen_alloc.h str.h env.h pathexec.h
	./compile pathexec_run.c

822headerfilter: load 822headerfilter.o wait.a strerr.a substdio.a \
pathexec_run.o env.a getln.a stralloc.a alloc.a str.a mess822.a fd.a error.a
	./load 822headerfilter wait.a strerr.a substdio.a pathexec_run.o env.a getln.a \
	stralloc.a alloc.a str.a mess822.a fd.a error.a 

822headerfilter.o: compile 822headerfilter.c \
exit.h error.h wait.h strerr.h substdio.h getln.h \
stralloc.h gen_alloc.h mess822.h caltime.h caldate.h fd.h pathexec.h
	./compile 822headerfilter.c

822addr: load 822addr.o strerr.a error.a mess822.a fs.a getln.a \
stralloc.a substdio.a alloc.a str.a stralloc.a case.a
	./load 822addr strerr.a error.a mess822.a fs.a getln.a stralloc.a \
	substdio.a alloc.a str.a stralloc.a case.a 

822addr.o: compile 822addr.c \
substdio.h strerr.h getln.h stralloc.h gen_alloc.h mess822.h caltime.h \
caldate.h case.h exit.h alloc.h
	./compile 822addr.c

822fields: load 822fields.o strerr.a error.a mess822.a fs.a getln.a \
stralloc.a substdio.a alloc.a str.a stralloc.a case.a
	./load 822fields strerr.a error.a mess822.a fs.a getln.a stralloc.a \
	substdio.a alloc.a str.a stralloc.a case.a 

822fields.o: compile 822fields.c \
substdio.h strerr.h getln.h stralloc.h gen_alloc.h mess822.h caltime.h \
caldate.h case.h exit.h alloc.h
	./compile 822fields.c

checkaddr: load checkaddr.o mess822.a getln.a strerr.a substdio.a \
stralloc.a alloc.a error.a env.a str.a case.a fs.a
	./load checkaddr mess822.a getln.a strerr.a substdio.a stralloc.a alloc.a \
	error.a env.a str.a case.a fs.a 

checkaddr.o: compile checkaddr.c \
substdio.h strerr.h getln.h stralloc.h gen_alloc.h mess822.h caltime.h \
caldate.h case.h env.h exit.h
	./compile checkaddr.c

checkdomain: load checkdomain.o mess822.a getln.a strerr.a substdio.a \
stralloc.a alloc.a error.a env.a str.a case.a fs.a
	./load checkdomain mess822.a getln.a strerr.a substdio.a stralloc.a \
	alloc.a error.a env.a str.a case.a fs.a 

checkdomain.o: compile checkdomain.c \
substdio.h strerr.h getln.h stralloc.h gen_alloc.h mess822.h caltime.h \
caldate.h case.h env.h exit.h str.h
	./compile checkdomain.c

filterto.o: compile filterto.c \
sig.h exit.h env.h error.h wait.h qmail.h substdio.h \
strerr.h fmt.h getln.h stralloc.h gen_alloc.h mess822.h caltime.h \
caldate.h fd.h pathexec.h
	./compile filterto.c

filterto: load filterto.o auto_qmail.o qmail.o strerr.a fd.a sig.a wait.a \
env.a pathexec_run.o getln.a mess822.a substdio.a error.a str.a fs.a stralloc.a \
alloc.a
	./load filterto auto_qmail.o qmail.o strerr.a fd.a sig.a wait.a env.a \
	pathexec_run.o getln.a mess822.a substdio.a error.a str.a fs.a stralloc.a \
	alloc.a 

tomaildir: load tomaildir.o sig.a open.a strerr.a substdio.a env.a str.a \
error.a fs.a now.o
	./load tomaildir sig.a open.a strerr.a substdio.a env.a str.a error.a fs.a \
	now.o 

tomaildir.o: compile tomaildir.c \
sig.h byte.h exit.h open.h substdio.h strerr.h error.h fmt.h \
str.h now.h datetime.h env.h
	./compile tomaildir.c

condtomaildir: load condtomaildir.o sig.a open.a strerr.a substdio.a env.a \
pathexec_run.o stralloc.a alloc.a str.a error.a fs.a seek.a wait.a now.o
	./load condtomaildir sig.a open.a strerr.a substdio.a env.a pathexec_run.o \
	stralloc.a alloc.a str.a error.a fs.a seek.a wait.a now.o 

condtomaildir.o: compile condtomaildir.c \
sig.h byte.h exit.h open.h substdio.h strerr.h error.h fmt.h \
env.h str.h wait.h seek.h now.h datetime.h pathexec.h
	./compile condtomaildir.c

iftoccfrom: load iftoccfrom.o mess822.a getln.a strerr.a substdio.a \
stralloc.a alloc.a error.a env.a str.a case.a fs.a open.a
	./load iftoccfrom mess822.a getln.a strerr.a substdio.a stralloc.a alloc.a \
	error.a env.a str.a case.a fs.a open.a 

iftoccfrom.o: compile iftoccfrom.c \
substdio.h strerr.h getln.h stralloc.h gen_alloc.h mess822.h caltime.h \
caldate.h case.h env.h exit.h
	./compile iftoccfrom.c

ifaddr: load ifaddr.o mess822.a getln.a strerr.a substdio.a stralloc.a \
alloc.a error.a env.a str.a fs.a case.a
	./load ifaddr mess822.a getln.a strerr.a substdio.a stralloc.a alloc.a \
	error.a env.a str.a fs.a case.a 

getconf.o: compile getconf.c \
stralloc.h gen_alloc.h openreadclose.h strerr.h getconf.h byte.h
	./compile getconf.c

ifaddr.o: compile ifaddr.c \
substdio.h strerr.h getln.h stralloc.h gen_alloc.h mess822.h caltime.h \
caldate.h case.h env.h exit.h alloc.h str.h
	./compile ifaddr.c

replier.o: compile replier.c \
stralloc.h gen_alloc.h strerr.h error.h qmail.h substdio.h env.h sig.h \
open.h getln.h case.h scan.h str.h fmt.h exit.h getconf.h \
constmap.h fd.h wait.h pathexec.h byte.h
	./compile replier.c

replier: load replier.o qmail.o stralloc.a alloc.a strerr.a substdio.a \
env.a pathexec_env.o pathexec_run.o str.a error.a sig.a openreadclose.o open.a \
getln.a case.a fs.a fd.a wait.a getconf.o constmap.o auto_qmail.o
	./load replier qmail.o stralloc.a alloc.a strerr.a substdio.a env.a \
	pathexec_env.o pathexec_run.o str.a error.a sig.a openreadclose.o open.a \
	getln.a case.a fs.a fd.a wait.a getconf.o constmap.o auto_qmail.o 

replier-config: load replier-config.o strerr.a substdio.a stralloc.a str.a \
error.a alloc.a open.a auto_ezmlm.o auto_qmail.o
	./load replier-config strerr.a substdio.a stralloc.a str.a error.a alloc.a \
	open.a auto_ezmlm.o auto_qmail.o 

replier-config.o: compile replier-config.c \
strerr.h substdio.h stralloc.h gen_alloc.h open.h exit.h \
auto_ezmlm.h auto_qmail.h str.h help.t
	./compile replier-config.c

openreadclose.o: compile openreadclose.c \
error.h open.h stralloc.h gen_alloc.h openreadclose.h
	./compile openreadclose.c


auto_ezmlm.o: compile \
auto_ezmlm.c
	./compile auto_ezmlm.c

auto_ezmlm.c: auto-str conf-ezmlm
	./auto-str auto_ezmlm `head -1 conf-ezmlm` > auto_ezmlm.c

make-text: load make-text.o getln.a substdio.a stralloc.a alloc.a error.a \
str.a
	./load make-text getln.a substdio.a stralloc.a alloc.a error.a str.a 

make-text.o: compile \
make-text.c \
substdio.h stralloc.h gen_alloc.h getln.h str.h exit.h
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

multipipe.o: compile multipipe.c select.h direntry.h bool.h svcfns.h
	./compile multipipe.c

multitail: load multitail.o
	./load multitail 

multitail.o: compile multitail.c systime.h bool.h
	./compile multitail.c

teepipe: load teepipe.o
	./load teepipe 

teepipe.o: compile teepipe.c
	./compile teepipe.c

systime.h: compile trysystime.c trytime.c
	( ( ./compile trysystime.c >/dev/null 2>&1 && \
	    echo '#include <sys/time.h>' ); \
	  ( ./compile trytime.c >/dev/null 2>&1 && \
	    echo '#include <time.h>' ) ) >systime.h
	rm -f trysystime.o trytime.o

cert cert-req: Makefile-cert
	@$(MAKE) -sf $< $@

Makefile-cert: conf-qmail Makefile-cert.mk
	@cat Makefile-cert.mk \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> $@

update_tmprsadh: conf-qmail update_tmprsadh.sh
	@cat update_tmprsadh.sh\
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> $@

tmprsadh: update_tmprsadh
	echo "Creating new temporary RSA and DH parameters"
	./update_tmprsadh

qmail-qmHandle: conf-qmail qmHandle.in
	@cat qmHandle.in\
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> $@

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
load dot-forward.o control.o qmail.o auto_qmail.o variables.o token822.o env.a \
getln.a getopt.a strerr.a error.a substdio.a stralloc.a alloc.a \
case.a str.a sig.a seek.a open.a wait.a fd.a fs.a
	./load dot-forward control.o qmail.o auto_qmail.o variables.o \
	token822.o env.a getln.a getopt.a strerr.a error.a \
	substdio.a stralloc.a alloc.a case.a str.a sig.a seek.a \
	open.a wait.a fd.a fs.a 

dot-forward.0: \
dot-forward.1
	nroff -man dot-forward.1 > dot-forward.0

dot-forward.o: \
compile dot-forward.c sgetopt.h subgetopt.h substdio.h \
stralloc.h gen_alloc.h getln.h strerr.h error.h exit.h open.h wait.h \
seek.h env.h str.h fmt.h token822.h gen_alloc.h control.h qmail.h \
substdio.h auto_qmail.h variables.h fd.h case.h sig.h
	./compile dot-forward.c

fastforward: \
load fastforward.o slurpclose.o coe.o strset.o qmail.o auto_qmail.o \
getopt.a cdb.a env.a strerr.a substdio.a stralloc.a alloc.a error.a \
case.a str.a fs.a sig.a wait.a seek.a open.a fd.a
	./load fastforward slurpclose.o coe.o strset.o qmail.o \
	auto_qmail.o getopt.a cdb.a env.a strerr.a substdio.a \
	stralloc.a alloc.a error.a case.a str.a fs.a sig.a wait.a \
	seek.a open.a fd.a 

fastforward.0: \
fastforward.1
	nroff -man fastforward.1 > fastforward.0

fastforward.o: \
compile fastforward.c stralloc.h gen_alloc.h substdio.h subfd.h \
substdio.h strset.h uint32.h sgetopt.h subgetopt.h exit.h \
strerr.h env.h sig.h qmail.h substdio.h fmt.h case.h alloc.h coe.h \
seek.h wait.h open.h slurpclose.h str.h cdb.h byte.h
	./compile fastforward.c

strset.o: \
compile strset.c strset.h uint32.h str.h byte.h alloc.h
	./compile strset.c

printforward: \
load printforward.o cdb.a strerr.a substdio.a stralloc.a alloc.a \
error.a str.a
	./load printforward cdb.a strerr.a substdio.a stralloc.a \
	alloc.a error.a str.a 

printforward.0: \
printforward.1
	nroff -man printforward.1 > printforward.0

printforward.o: \
compile printforward.c substdio.h subfd.h substdio.h strerr.h \
stralloc.h gen_alloc.h cdb.h uint32.h
	./compile printforward.c

printmaillist: \
load printmaillist.o getln.a strerr.a substdio.a stralloc.a alloc.a \
error.a str.a
	./load printmaillist getln.a strerr.a substdio.a \
	stralloc.a alloc.a error.a str.a 

printmaillist.0: \
printmaillist.1
	nroff -man printmaillist.1 > printmaillist.0

printmaillist.o: \
compile printmaillist.c substdio.h subfd.h substdio.h strerr.h \
stralloc.h gen_alloc.h getln.h str.h
	./compile printmaillist.c

setforward: \
load setforward.o cdbmss.o cdbmake.a strerr.a substdio.a stralloc.a \
alloc.a error.a str.a seek.a open.a case.a
	./load setforward cdbmss.o cdbmake.a strerr.a substdio.a \
	stralloc.a alloc.a error.a str.a seek.a open.a case.a 

setforward.0: \
setforward.1
	nroff -man setforward.1 > setforward.0

setforward.o: \
compile setforward.c substdio.h subfd.h substdio.h strerr.h \
stralloc.h gen_alloc.h open.h case.h cdbmss.h cdbmake.h \
uint32.h substdio.h
	./compile setforward.c

setmaillist: \
load setmaillist.o getln.a strerr.a substdio.a stralloc.a alloc.a \
error.a str.a open.a
	./load setmaillist getln.a strerr.a substdio.a stralloc.a \
	alloc.a error.a str.a open.a 

setmaillist.0: \
setmaillist.1
	nroff -man setmaillist.1 > setmaillist.0

setmaillist.o: \
compile setmaillist.c substdio.h subfd.h substdio.h strerr.h \
stralloc.h gen_alloc.h getln.h open.h byte.h
	./compile setmaillist.c

newaliases: \
load newaliases.o auto_qmail.o token822.o control.o cdbmss.o variables.o \
env.a cdbmake.a strerr.a getln.a substdio.a stralloc.a alloc.a error.a \
str.a fs.a seek.a open.a case.a
	./load newaliases auto_qmail.o token822.o control.o variables.o \
	cdbmss.o env.a cdbmake.a strerr.a getln.a substdio.a stralloc.a \
	alloc.a error.a str.a fs.a seek.a open.a case.a 

newaliases.0: \
newaliases.1
	nroff -man newaliases.1 > newaliases.0

newaliases.o: \
compile newaliases.c substdio.h strerr.h stralloc.h gen_alloc.h \
getln.h open.h token822.h gen_alloc.h control.h byte.h \
auto_qmail.h case.h cdbmss.h cdbmake.h uint32.h substdio.h variables.h
	./compile newaliases.c

newinclude: \
load newinclude.o auto_qmail.o token822.o control.o variables.o getln.a strerr.a \
stralloc.a env.a alloc.a substdio.a error.a str.a fs.a open.a wait.a \
fd.a
	./load newinclude auto_qmail.o token822.o control.o variables.o \
	getln.a strerr.a stralloc.a env.a alloc.a substdio.a \
	error.a str.a fs.a open.a wait.a fd.a 

newinclude.0: \
newinclude.1
	nroff -man newinclude.1 > newinclude.0

newinclude.o: \
compile newinclude.c substdio.h strerr.h stralloc.h gen_alloc.h \
getln.h open.h token822.h gen_alloc.h control.h \
auto_qmail.h env.h variables.h byte.h
	./compile newinclude.c

envdir: load envdir.o openreadclose.o pathexec_env.o pathexec_run.o \
strerr.a substdio.a env.a str.a open.a error.a stralloc.a alloc.a
	./load envdir openreadclose.o pathexec_env.o pathexec_run.o \
	strerr.a substdio.a env.a str.a open.a error.a stralloc.a alloc.a

envdir.o: byte.h compile direntry.h envdir.c error.h gen_alloc.h \
open.h openreadclose.h pathexec.h stralloc.h strerr.h
	./compile envdir.c

envuidgid: envuidgid.o load strerr_die.o byte_copy.o str_len.o \
str_chr.o str_diffn.o byte_diff.o pathexec_env.o pathexec_run.o \
fmt_ulong.o error.o error_str.o strerr_sys.o \
envread.o alloc.o alloc_re.o substdio.a stralloc.a
	./load envuidgid strerr_die.o byte_copy.o str_len.o \
	str_chr.o str_diffn.o byte_diff.o pathexec_env.o pathexec_run.o \
	fmt_ulong.o error.o error_str.o strerr_sys.o \
	envread.o alloc.o alloc_re.o substdio.a stralloc.a

envuidgid.o: compile envuidgid.c fmt.h pathexec.h strerr.h
	./compile envuidgid.c

fghack: fghack.o load strerr_die.o strerr_sys.o byte_copy.o str_len.o str_diffn.o \
alloc.o alloc_re.o str_chr.o wait_pid.o envread.o pathexec_run.o error.o \
error_str.o substdio.a stralloc.a
	./load fghack strerr_die.o strerr_sys.o byte_copy.o str_len.o str_diffn.o \
	alloc.o alloc_re.o str_chr.o wait_pid.o envread.o pathexec_run.o error.o \
	error_str.o substdio.a stralloc.a

fghack.o: compile error.h fghack.c pathexec.h strerr.h \
wait.h
	./compile fghack.c
	
multilog: multilog.o load timestamp.o match.o deepsleep.o taia_frac.o \
scan_ulong.o taia_uint.o taia_add.o sig.o \
wait_pid.o fd_copy.o fd_move.o coe.o error.o error_str.o \
strerr_die.o strerr_sys.o seek_set.o taia_now.o taia_pack.o \
tai_pack.o byte_rchr.o iopause.o taia_less.o taia_sub.o taia_approx.o \
lock.a str.a alloc.o alloc_re.o sig.a substdio.a stralloc.a open.a
	./load multilog timestamp.o match.o deepsleep.o taia_frac.o \
	scan_ulong.o taia_uint.o taia_add.o sig.o \
	wait_pid.o fd_copy.o fd_move.o coe.o error.o error_str.o \
	strerr_die.o strerr_sys.o seek_set.o taia_now.o taia_pack.o \
	tai_pack.o byte_rchr.o iopause.o taia_less.o taia_sub.o taia_approx.o \
	lock.a alloc.o alloc_re.o sig.a substdio.a stralloc.a open.a str.a

multilog.o: alloc.h byte.h coe.h compile deepsleep.h \
direntry.h env.h error.h fd.h lock.h match.h multilog.c open.h scan.h \
seek.h sig.h str.h strerr.h timestamp.h wait.h
	./compile multilog.c

deepsleep.o: compile deepsleep.c deepsleep.h iopause.h tai.h taia.h \
uint64.h
	./compile deepsleep.c

match.o: compile match.c match.h
	./compile match.c

iopause.h: choose compile iopause.h1 iopause.h2 load trypoll.c
	./choose clr trypoll iopause.h1 iopause.h2 > iopause.h

iopause.o: compile iopause.c iopause.h select.h tai.h taia.h uint64.h
	./compile iopause.c

taia_less.o: compile tai.h taia.h taia_less.c uint64.h
	./compile taia_less.c

taia_uint.o: compile tai.h taia.h taia_uint.c uint64.h
	./compile taia_uint.c

taia_add.o: compile tai.h taia.h taia_add.c uint64.h
	./compile taia_add.c

taia_sub.o: compile tai.h taia.h taia_sub.c uint64.h
	./compile taia_sub.c

tai_sub.o: compile tai.h tai_sub.c uint64.h
	./compile tai_sub.c

tai_unpack.o: compile tai.h tai_unpack.c uint64.h
	./compile tai_unpack.c

taia_approx.o: compile tai.h taia.h taia_approx.c uint64.h
	./compile taia_approx.c

taia_frac.o: compile tai.h taia.h taia_frac.c uint64.h
	./compile taia_frac.c

pgrphack: load pgrphack.o pathexec_run.o strerr_sys.o strerr_die.o \
envread.o alloc.o alloc_re.o error.o error_str.o \
str.a stralloc.a substdio.a
	./load pgrphack pathexec_run.o strerr_sys.o strerr_die.o \
	envread.o alloc.o alloc_re.o error.o error_str.o \
	stralloc.a substdio.a str.a

pgrphack.o: compile pathexec.h pgrphack.c strerr.h
	./compile pgrphack.c

readproctitle: load readproctitle.o error.o
	./load readproctitle error.o

readproctitle.o: compile error.h readproctitle.c
	./compile readproctitle.c

setlock: load setlock.o strerr_die.o strerr_sys.o sgetopt.o subgetopt.o \
envread.o pathexec_run.o error_str.o \
open_append.o error.o alloc.o alloc_re.o \
lock.a substdio.a stralloc.a str.a
	./load setlock strerr_die.o strerr_sys.o sgetopt.o subgetopt.o \
	envread.o pathexec_run.o error_str.o \
	open_append.o error.o alloc.o alloc_re.o \
	lock.a substdio.a stralloc.a str.a

setlock.o: compile lock.h open.h pathexec.h setlock.c sgetopt.h \
strerr.h subgetopt.h
	./compile setlock.c

setuidgid: load setuidgid.o strerr_die.o strerr_sys.o \
prot.o envread.o pathexec_run.o alloc.o alloc_re.o error.o error_str.o \
substdio.a stralloc.a str.a
	./load setuidgid strerr_die.o strerr_sys.o \
	prot.o envread.o pathexec_run.o alloc.o alloc_re.o error.o error_str.o \
	substdio.a stralloc.a str.a

setuidgid.o: compile pathexec.h prot.h setuidgid.c strerr.h
	./compile setuidgid.c

softlimit: load softlimit.o sgetopt.o subgetopt.o pathexec_run.o error.o error_str.o \
scan_ulong.o envread.o strerr_sys.o strerr_die.o alloc.o alloc_re.o \
stralloc.a substdio.a str.a
	./load softlimit sgetopt.o subgetopt.o pathexec_run.o error.o error_str.o \
	scan_ulong.o envread.o strerr_sys.o strerr_die.o alloc.o alloc_re.o \
	stralloc.a substdio.a str.a

softlimit.o: compile pathexec.h scan.h sgetopt.h softlimit.c str.h \
strerr.h subgetopt.h
	./compile softlimit.c

supervise: deepsleep.o load supervise.o sig.o lock_exnb.o wait_nohang.o wait_pid.o fifo.o \
	tai_pack.o taia_pack.o taia_less.o taia_add.o taia_sub.o \
	taia_now.o taia_uint.o taia_approx.o taia_frac.o ndelay.o \
	iopause.o coe.o strerr_die.o strerr_sys.o error.o error_str.o \
	sig.a substdio.a str.a open.a
	./load supervise deepsleep.o sig.o \
	lock_exnb.o wait_nohang.o wait_pid.o fifo.o \
	tai_pack.o taia_pack.o taia_less.o taia_add.o taia_sub.o \
	taia_now.o taia_uint.o taia_approx.o taia_frac.o ndelay.o \
	iopause.o coe.o strerr_die.o strerr_sys.o error.o error_str.o \
	sig.a substdio.a str.a open.a

supervise.o: coe.h compile deepsleep.h env.h error.h fifo.h iopause.h \
lock.h ndelay.h open.h sig.h strerr.h supervise.c tai.h taia.h \
uint64.h wait.h
	./compile supervise.c

sig.o: compile sig.c sig.h
	./compile sig.c

svc: load svc.o error.o error_str.o strerr_die.o strerr_sys.o \
ndelay_off.o sig_catch.o sig.o sgetopt.o subgetopt.o \
substdio.a str.a open.a
	./load svc error.o error_str.o strerr_die.o strerr_sys.o \
	ndelay_off.o sig_catch.o sig.o sgetopt.o subgetopt.o \
	substdio.a str.a open.a

svc.o: substdio.h byte.h compile error.h ndelay.h open.h sgetopt.h \
sig.h strerr.h subgetopt.h svc.c
	./compile svc.c

svok: load svok.o strerr_die.o strerr_sys.o error.o error_str.o \
open_write.o substdio.a str.a
	./load svok strerr_die.o strerr_sys.o error.o error_str.o \
	open_write.o substdio.a str.a

svok.o: compile error.h open.h strerr.h svok.c
	./compile svok.c

svscan: load svscan.o scan_ulong.o coe.o strerr_die.o strerr_sys.o error.o \
envread.o fd_move.o fd_copy.o wait_nohang.o pathexec_run.o error_str.o \
alloc.o alloc_re.o substdio.a stralloc.a str.a
	./load svscan scan_ulong.o coe.o strerr_die.o strerr_sys.o error.o \
	envread.o fd_move.o fd_copy.o wait_nohang.o pathexec_run.o error_str.o \
	alloc.o alloc_re.o substdio.a stralloc.a str.a

svscan.o: byte.h coe.h compile direntry.h env.h error.h fd.h \
pathexec.h str.h strerr.h svscan.c wait.h scan.h
	./compile svscan.c

svstat: load svstat.o strerr_die.o strerr_sys.o \
fmt_ulong.o open_read.o open_write.o error.o error_str.o \
tai_now.o tai_sub.o tai_unpack.o \
substdio.a str.a
	./load svstat strerr_die.o strerr_sys.o \
	fmt_ulong.o open_read.o open_write.o error.o error_str.o \
	tai_now.o tai_sub.o tai_unpack.o \
	substdio.a str.a

svstat.o: substdio.h compile error.h fmt.h open.h strerr.h svstat.c \
tai.h uint64.h
	./compile svstat.c

tai64n: load tai64n.o timestamp.o error.o \
tai_pack.o taia_now.o taia_pack.o substdio.a str.a
	./load tai64n timestamp.o error.o \
	tai_pack.o taia_now.o taia_pack.o substdio.a str.a

tai64n.o: substdio.h compile tai64n.c timestamp.h
	./compile tai64n.c

tai64nlocal: load tai64nlocal.o error.o fmt_ulong.o fmt_uint0.o fmt_uint.o \
substdio.a str.a
	./load tai64nlocal error.o fmt_ulong.o fmt_uint0.o fmt_uint.o \
	substdio.a str.a

tai64nlocal.o: substdio.h subfd.h compile fmt.h tai64nlocal.c
	./compile tai64nlocal.c

tai64nunix: \
load tai64nunix.o error.o fmt_ulong.o fmt_uint0.o fmt_uint.o \
substdio.a str.a
	./load tai64nunix error.o fmt_ulong.o fmt_uint0.o fmt_uint.o \
	substdio.a str.a

tai64nunix.o: \
compile tai64nunix.c substdio.h subfd.h fmt.h
	./compile tai64nunix.c

recordio.o: \
compile recordio.c sig.h substdio.h strerr.h str.h byte.h \
exit.h fmt.h iopause.h fd.h pathexec.h
	./compile recordio.c

recordio: \
load recordio.o pathexec_run.o alloc.o alloc_re.o envread.o fmt_ulong.o \
sig_catch.o fd_copy.o fd_move.o taia_now.o taia_less.o taia_sub.o \
taia_approx.o taia_uint.o taia_frac.o taia_add.o iopause.o error.o \
error_str.o strerr.a stralloc.a substdio.a byte_copy.o str.a
	./load recordio pathexec_run.o alloc.o alloc_re.o envread.o fmt_ulong.o \
	sig_catch.o fd_copy.o fd_move.o taia_now.o taia_less.o taia_sub.o \
	taia_approx.o taia_uint.o taia_frac.o taia_add.o iopause.o error.o \
	error_str.o strerr.a stralloc.a substdio.a byte_copy.o str.a

argv0.o: \
compile argv0.c pathexec.h strerr.h
	./compile argv0.c

argv0: \
load argv0.o pathexec_run.o error_str.o alloc.o alloc_re.o envread.o error.o \
strerr.a substdio.a stralloc.a str.a
	./load argv0 pathexec_run.o error_str.o alloc.o alloc_re.o envread.o error.o \
	strerr.a substdio.a stralloc.a str.a

addcr: \
load addcr.o error.o substdio.a str.a
	./load addcr error.o substdio.a str.a

addcr.o: \
compile addcr.c substdio.h subfd.h exit.h
	./compile addcr.c

delcr: \
load delcr.o error.o byte_copy.o byte_cr.o str_len.o substdio.a
	./load delcr error.o byte_copy.o byte_cr.o str_len.o substdio.a

delcr.o: \
compile delcr.c subfd.h substdio.h exit.h
	./compile delcr.c

fixcrio: \
load fixcrio.o pathexec_run.o sig_catch.o alloc.o alloc_re.o \
envread.o byte_copy.o error.o error_str.o strerr_die.o \
strerr_sys.o fd_copy.o fd_move.o taia_add.o taia_less.o \
taia_sub.o taia_approx.o taia_now.o taia_uint.o taia_frac.o \
iopause.o str_chr.o str_len.o str_diffn.o substdio.a stralloc.a
	./load fixcrio pathexec_run.o sig_catch.o alloc.o alloc_re.o \
	envread.o byte_copy.o error.o error_str.o strerr_die.o \
	strerr_sys.o fd_copy.o fd_move.o taia_add.o taia_less.o \
	taia_sub.o taia_approx.o taia_now.o taia_uint.o taia_frac.o \
	iopause.o str_chr.o str_len.o str_diffn.o substdio.a stralloc.a

fixcrio.o: \
compile fixcrio.c sig.h strerr.h byte.h exit.h \
iopause.h taia.h tai.h uint64.h pathexec.h fd.h
	./compile fixcrio.c

logselect.o: compile byte.h open.h error.h direntry.h stralloc.h substdio.h str.h subfd.h \
sorted.h strerr.h pathexec.h taia.h timestamp.h logselect.c
	./compile logselect.c

logselect: load logselect.o sorted.o error.o error_str.o open_read.o alloc.o \
tai_pack.o taia_pack.o alloc_re.o byte_diff.o byte_cr.o byte_copy.o \
str_len.o str_diff.o strerr.a substdio.a stralloc.a
	./load logselect sorted.o error.o error_str.o alloc.o open_read.o \
	tai_pack.o taia_pack.o alloc_re.o byte_diff.o byte_cr.o byte_copy.o \
	str_len.o str_diff.o strerr.a substdio.a stralloc.a

sorted.o: compile sorted.c str.h gen_allocdefs.h sorted.h alloc.h stralloc.h
	./compile sorted.c

sortedtest: load sortedtest.o str_len.o sorted.o error.o byte_diff.o byte_copy.o alloc.o alloc_re.o substdio.a stralloc.a
	./load sortedtest str_len.o sorted.o error.o byte_diff.o byte_copy.o alloc.o alloc_re.o substdio.a stralloc.a

sortedtest.o: compile sortedtest.c sorted.h stralloc.h
	./compile sortedtest.c


generic.o: \
compile generic.c str.h env.h wait.h
	./compile generic.c

generic.so:generic.o wait_pid.o envread.o auto_scancmd.o MakeArgs.o \
byte_copy.o alloc.o alloc_re.o stralloc.a str_diffn.o error.o str_len.o
	if test $(SYSTEM) = Darwin ; then \
	$(LD) -bundle -undefined dynamic_lookup -o generic.so generic.o wait_pid.o \
	envread.o auto_scancmd.o MakeArgs.o byte_copy.o alloc.o \
	alloc_re.o stralloc.a str_diffn.o error.o str_len.o; \
	else \
	$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 -o generic.so generic.o wait_pid.o \
	envread.o auto_scancmd.o MakeArgs.o byte_copy.o alloc.o \
	alloc_re.o stralloc.a str_diffn.o error.o str_len.o; fi
#	$(LD) -G -Bdynamic -o generic.so generic.o wait_pid.o \

relaytest.o: \
compile relaytest.c strerr.h getln.h substdio.h \
timeoutread.h timeoutwrite.h stralloc.h scan.h byte.h
	./compile relaytest.c

relaytest: \
load relaytest.o timeoutread.o \
timeoutwrite.o getln.a strerr.a substdio.a stralloc.a alloc.a \
error.a str.a
	./load relaytest timeoutread.o scan_ulong.o strerr.a error.a \
	getln.a substdio.a stralloc.a alloc.a str.a timeoutwrite.o

################### IPV6 ########################################

haveip6.h: \
choose tryip6.c compile haveip6.h1 haveip6.h2 conf-ip
	./compile `grep -h -v "^#" conf-ip` tryip6.c; \
	cc tryip6.o -o tryip6; \
	./choose r tryip6 haveip6.h1 haveip6.h2 > haveip6.h

socket_v4mappedprefix.o: \
compile socket_v4mappedprefix.c conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_v4mappedprefix.c

scan_xlong.o: \
compile scan_xlong.c scan.h
	./compile scan_xlong.c

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

cdbmake: \
load cdbmake.o byte_copy.o byte_cr.o error.o error_str.o str_len.o subfderr.o open_trunc.o \
cdb_hash.o uint32_pack.o seek_set.o cdb_make.o cdb.a alloc.a substdio.a strerr.a
	./load cdbmake byte_copy.o byte_cr.o error.o error_str.o str_len.o subfderr.o open_trunc.o \
	cdb_hash.o uint32_pack.o seek_set.o cdb_make.o cdb.a alloc.a substdio.a strerr.a

cdbmake.o: \
compile cdbmake.c error.h open.h strerr.h cdb_make.h subfd.h substdio.h \
uint32.h cdb.h uint32.h
	./compile cdbmake.c

cdb_make.o: \
compile cdb_make.c seek.h error.h alloc.h cdb.h uint32.h \
cdb_make.h substdio.h uint32.h
	./compile cdb_make.c

cdbget: load cdbget.o cdb.o scan_ulong.o error.o error_str.o str_len.o subfderr.o seek_set.o \
byte_diff.o uint32_unpack.o byte_copy.o cdb.a substdio.a strerr.a
	./load cdbget cdb.o scan_ulong.o error.o error_str.o str_len.o subfderr.o seek_set.o \
	byte_diff.o uint32_unpack.o byte_copy.o cdb.a substdio.a strerr.a

cdbget.o: \
compile cdbget.c exit.h scan.h str.h subfd.h substdio.h strerr.h cdb.h uint32.h
	./compile cdbget.c

cdb.o: \
compile cdb.c readwrite.h error.h seek.h byte.h cdb.h uint32.h
	./compile cdb.c

cdbdump: load cdbdump.o byte_cr.o byte_copy.o fmt_ulong.o subfderr.o \
uint32_unpack.o error.o error_str.o str_len.o substdio.a strerr.a
	./load cdbdump byte_cr.o byte_copy.o fmt_ulong.o subfderr.o \
	uint32_unpack.o error.o error_str.o str_len.o substdio.a strerr.a

cdbdump.o: compile cdbdump.c uint32.h fmt.h subfd.h substdio.h strerr.h exit.h
	./compile cdbdump.c

cdbstats: load cdbstats.o byte_copy.o byte_cr.o error.o error_str.o str_len.o \
subfderr.o byte_diff.o uint32_unpack.o cdb.o fmt_ulong.o seek_set.o \
seek_cur.o cdb.a substdio.a strerr.a
	./load cdbstats byte_copy.o byte_cr.o error.o error_str.o str_len.o \
	subfderr.o byte_diff.o uint32_unpack.o cdb.o fmt_ulong.o seek_set.o \
	seek_cur.o cdb.a substdio.a strerr.a

cdbstats.o: compile cdbstats.c uint32.h fmt.h subfd.h substdio.h strerr.h seek.h cdb.h \
uint32.h exit.h str.h
	./compile cdbstats.c

cdbtest: load cdbtest.o str_len.o error.o error_str.o byte_copy.o byte_cr.o \
byte_diff.o cdb.o fmt_ulong.o uint32_unpack.o seek_set.o seek_cur.o subfderr.o \
cdb.a substdio.a strerr.a 
	./load cdbtest str_len.o error.o error_str.o byte_copy.o byte_cr.o \
	byte_diff.o cdb.o fmt_ulong.o uint32_unpack.o seek_set.o seek_cur.o subfderr.o \
	cdb.a substdio.a strerr.a 

cdbtest.o: compile cdbtest.c uint32.h fmt.h subfd.h substdio.h strerr.h seek.h cdb.h \
uint32.h str.h exit.h
	./compile cdbtest.c

testzero: load testzero.o scan_ulong.o cdb_make.o error.o error_str.o \
uint32_pack.o str_len.o seek_set.o byte_copy.o subfderr.o cdb.a alloc.a substdio.a strerr.a
	./load testzero scan_ulong.o cdb_make.o error.o error_str.o \
	uint32_pack.o str_len.o seek_set.o byte_copy.o subfderr.o cdb.a alloc.a substdio.a strerr.a

testzero.o: compile testzero.c uint32.h scan.h strerr.h cdb_make.h subfd.h substdio.h \
uint32.h exit.h
	./compile testzero.c

cdbmake-12: warn-auto.sh cdbmake-12.sh conf-qmail
	cat warn-auto.sh cdbmake-12.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> cdbmake-12
	chmod 755 cdbmake-12

cdbmake-sv: \
warn-auto.sh cdbmake-sv.sh conf-qmail
	cat warn-auto.sh cdbmake-sv.sh \
	| sed s}QMAIL}"`head -1 conf-qmail`"}g \
	> cdbmake-sv
	chmod 755 cdbmake-sv

uint32_pack.o: compile uint32_pack.c uint32.h
	./compile uint32_pack.c
uint32_unpack.o: compile uint32_unpack.c uint32.h
	./compile uint32_unpack.c

qmail-qfilter: \
load qmail-qfilter.o auto_qmail.o error.o scan_ulong.o envread.o \
fmt_ulong.o alloc.a env.a substdio.a str.a
	./load qmail-qfilter auto_qmail.o error.o scan_ulong.o envread.o \
	fmt_ulong.o env.a alloc.a substdio.a str.a

qmail-qfilter.o: \
compile qmail-qfilter.c auto_qmail.h scan.h qmail.h substdio.h env.h \
alloc.h str.h fmt.h byte.h
	./compile qmail-qfilter.c

qmail-qfilter.0: \
qmail-qfilter.1
	nroff -man qmail-qfilter.1 > qmail-qfilter.0

qmail-greyd: \
load qmail-greyd.o sgetopt.o subgetopt.o \
error.o error_str.o strerr_die.o strerr_sys.o scan_ulong.o \
str_len.o str_cpy.o str_chr.o str_diff.o str_diffn.o \
ip.o socket_v6any.o socket_v4mappedprefix.o \
envread.o open_read.o constmap.o variables.o \
case_diffb.o getln.o getln2.o fmt_ulong.o \
byte_rchr.o byte_copy.o byte_cr.o byte_chr.o byte_diff.o \
alloc_re.o alloc.o control.o scan_xlong.o fmt_str.o \
cdb_seek.o cdb_hash.o cdb_unpack.o auto_qmail.o \
sig_block.o sig_catch.o \
tablematch.o substdio.a stralloc.a
	./load qmail-greyd sgetopt.o subgetopt.o \
	error.o error_str.o strerr_die.o strerr_sys.o scan_ulong.o \
	str_len.o str_cpy.o str_chr.o str_diff.o str_diffn.o \
	ip.o socket_v6any.o socket_v4mappedprefix.o \
	envread.o open_read.o constmap.o variables.o \
	case_diffb.o getln.o getln2.o fmt_ulong.o \
	byte_rchr.o byte_copy.o byte_cr.o byte_chr.o byte_diff.o \
	alloc_re.o alloc.o control.o scan_xlong.o fmt_str.o \
	cdb_seek.o cdb_hash.o cdb_unpack.o auto_qmail.o \
	sig_block.o sig_catch.o \
	tablematch.o substdio.a stralloc.a -lm

qmail-greyd.o: \
compile qmail-greyd.c sgetopt.h auto_qmail.h ip.h \
fmt.h str.h subfd.h byte.h strerr.h scan.h getln.h \
env.h stralloc.h constmap.h control.h error.h \
uint32.h open.h cdb.h variables.h sig.h greylist.h \
tablematch.h
	./compile -DUSE_HASH -DDYNAMIC_BUF `grep -h -v "^#" conf-ip` qmail-greyd.c

qmail-greyd.0: \
qmail-greyd.8
	nroff -man qmail-greyd.8 > qmail-greyd.0

batv: \
load batv.o sgetopt.o subgetopt.o str_len.o \
error_str.o error.o strerr_die.o strerr_sys.o now.o \
scan_ulong.o str_diffn.o str_rchr.o byte_copy.o \
alloc_re.o alloc.o stralloc.a substdio.a
	./load batv sgetopt.o subgetopt.o str_len.o \
	error_str.o error.o strerr_die.o strerr_sys.o now.o \
	scan_ulong.o str_diffn.o str_rchr.o byte_copy.o \
	alloc_re.o alloc.o stralloc.a substdio.a -lssl -lcrypto

batv.o: \
compile batv.c sgetopt.h str.h constmap.h strerr.h \
stralloc.h subfd.h now.h env.h scan.h conf-batv
	./compile `grep -h -v "^#" conf-batv` batv.c

#################################################################
