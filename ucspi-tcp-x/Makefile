# Don't edit Makefile! Use conf-* for configuration.
COFLAGS=-z+05:30
SHELL=/bin/sh
OPENSSLBIN=openssl
SSLINCS=-I/usr/kerberos/include
SSLLIBS := -lssl -lcrypto
prefix    != head -1 conf-prefix 2>/dev/null
mandir    != [ -d $(prefix)/man ] && echo $(prefix)/man || echo $(prefix)/share/man
shareddir != head -1 conf-shared 2>/dev/null
DEFS      != uname -s | tr "[:lower:]" "[:upper:]"
SYSTEM    != uname -s | tr "[:lower:]" "[:upper:]"
edit = sed \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@DATE\@,$(DATE),g'

default: it

compile: make-compile warn-auto.sh systype conf-cc-$(SYSTEM)
	( cat warn-auto.sh; ./make-compile "`cat systype`" ) > \
	compile
	chmod 755 compile

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

auto-ccld.sh: conf-cc conf-ld warn-auto.sh
	(cat warn-auto.sh; echo CC=\'`head -1 conf-cc`\'; \
	if test $(SYSTEM) = DARWIN ; then \
		echo LD=\'`head -1 conf-ld | sed s{-s{{`\'; \
	else \
		echo LD=\'`head -1 conf-ld`\'; \
	fi) > auto-ccld.sh

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

auto-str: \
load auto-str.o
	./load auto-str -lqmail

auto-str.o: \
compile auto-str.c
	./compile auto-str.c

auto_home.c: \
auto-str conf-prefix
	./auto-str auto_home `head -1 conf-prefix` > auto_home.c

auto_home.o: \
compile auto_home.c
	./compile auto_home.c

auto_shared.c: \
auto-str conf-shared
	./auto-str auto_shared `head -1 conf-shared` > auto_shared.c

auto_shared.o: \
compile auto_shared.c
	./compile auto_shared.c

check: \
it instcheck
	./instcheck

chkshsgr: \
load chkshsgr.o
	./load chkshsgr 

chkshsgr.o: \
compile chkshsgr.c
	./compile chkshsgr.c

sig.o: \
compile sig.c
	./compile sig.c

choose: \
warn-auto.sh choose.sh conf-prefix
	cat warn-auto.sh choose.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> choose
	chmod 755 choose

tcpserver.1: tcpserver.9
	cat tcpserver.9 \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> tcpserver.1

commands.o: \
compile commands.c commands.h
	./compile commands.c

date@: \
warn-auto.sh date@.sh conf-prefix
	cat warn-auto.sh date@.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> date@
	chmod 755 date@

dns.a: \
makelib dns_dfd.o dns_domain.o dns_dtda.o dns_ip.o dns_ipq.o \
dns_name.o dns_nd.o dns_packet.o dns_random.o dns_rcip.o dns_rcrw.o \
dns_resolve.o dns_sortip.o dns_transmit.o dns_txt.o 
	./makelib dns.a dns_dfd.o dns_domain.o dns_dtda.o dns_ip.o \
	dns_ipq.o dns_name.o dns_nd.o dns_packet.o dns_random.o \
	dns_rcip.o dns_rcrw.o dns_resolve.o dns_sortip.o \
	dns_transmit.o dns_txt.o

dns_dfd.o: \
compile dns_dfd.c dns.h haveip6.h
	./compile dns_dfd.c

dns_domain.o: \
compile dns_domain.c dns.h haveip6.h
	./compile dns_domain.c

dns_dtda.o: \
compile dns_dtda.c dns.h haveip6.h
	./compile dns_dtda.c

dns_ip.o: \
compile dns_ip.c dns.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` dns_ip.c

dns_ipq.o: \
compile dns_ipq.c dns.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` dns_ipq.c

dns_name.o: \
compile dns_name.c ip6.h dns.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` dns_name.c

dns_nd.o: \
compile dns_nd.c dns.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` dns_nd.c

dns_packet.o: \
compile dns_packet.c dns.h haveip6.h
	./compile dns_packet.c

dns_random.o: \
compile dns_random.c dns.h haveip6.h
	./compile dns_random.c

dns_rcip.o: \
compile dns_rcip.c openreadclose.h \
ip4.h ip6.h dns.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` dns_rcip.c

dns_rcrw.o: \
compile dns_rcrw.c openreadclose.h dns.h haveip6.h
	./compile dns_rcrw.c

dns_resolve.o: \
compile dns_resolve.c dns.h ip6.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` dns_resolve.c

dns_sortip.o: \
compile dns_sortip.c dns.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` dns_sortip.c

dns_transmit.o: \
compile dns_transmit.c socket.h dns.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` dns_transmit.c

dns_txt.o: \
compile dns_txt.c dns.h haveip6.h
	./compile dns_txt.c

finger@: \
warn-auto.sh finger@.sh conf-prefix
	cat warn-auto.sh finger@.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> finger@
	chmod 755 finger@

fixcrio: \
load fixcrio.o
	./load fixcrio -lqmail

fixcrio.o: \
compile fixcrio.c pathexec.h
	./compile fixcrio.c

hasmysql.h: \
choose compile load mysql_config.c hasmysql.h1 hasmysql.h2 mysql_config
	./choose R "./mysql_config --include" hasmysql.h1 hasmysql.h2 > hasmysql.h

hassgact.h: \
choose compile load trysgact.c hassgact.h1 hassgact.h2
	./choose cl trysgact hassgact.h1 hassgact.h2 > hassgact.h

hassgprm.h: \
choose compile load trysgprm.c hassgprm.h1 hassgprm.h2
	./choose cl trysgprm hassgprm.h1 hassgprm.h2 > hassgprm.h

hasshsgr.h: \
choose compile load tryshsgr.c hasshsgr.h1 hasshsgr.h2 chkshsgr \
warn-shsgr
	./chkshsgr || ( cat warn-shsgr; exit 1 )
	./choose clr tryshsgr hasshsgr.h1 hasshsgr.h2 > hasshsgr.h

haswaitp.h: \
choose compile load trywaitp.c haswaitp.h1 haswaitp.h2
	./choose cl trywaitp haswaitp.h1 haswaitp.h2 > haswaitp.h

hier.o: \
compile hier.c auto_home.h auto_shared.h conf-dlopen
	./compile `grep -h -v "^#" conf-dlopen` hier.c

http@: \
warn-auto.sh http@.sh conf-prefix
	cat warn-auto.sh http@.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> http@
	chmod 755 http@

instcheck: \
load instcheck.o hier.o auto_home.o auto_shared.o
	./load instcheck hier.o auto_home.o auto_shared.o -lqmail

instcheck.o: \
compile instcheck.c
	./compile instcheck.c

ip4_bit.o: \
compile ip4_bit.c ip4_bit.h ip4.h
	./compile ip4_bit.c

ip4_fmt.o: \
compile ip4_fmt.c ip4.h
	./compile ip4_fmt.c

ip4_scan.o: \
compile ip4_scan.c ip4.h
	./compile ip4_scan.c

it: \
prog instcheck tcpserver.1 shared installer

load: warn-auto.sh conf-ld systype make-load
	( cat warn-auto.sh; ./make-load "`cat systype`" ) > load
	chmod 755 load

mconnect: \
warn-auto.sh mconnect.sh conf-prefix
	cat warn-auto.sh mconnect.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> mconnect
	chmod 755 mconnect

mconnect-io: \
load mconnect-io.o unix.a
	./load mconnect-io unix.a -lqmail

mconnect-io.o: \
compile mconnect-io.c
	./compile mconnect-io.c

openreadclose.o: \
compile openreadclose.c readclose.h openreadclose.h 
	./compile openreadclose.c

pathexec_env.o: \
compile pathexec_env.c pathexec.h conf-dlopen
	./compile `grep -h -v "^#" conf-dlopen` pathexec_env.c

pathexec_rundl.o: \
compile pathexec_run.c pathexec.h conf-dlopen
	./compile `grep -h -v "^#" conf-dlopen` pathexec_run.c -o pathexec_rundl.o

pathexec_run.o: \
compile pathexec_run.c pathexec.h conf-dlopen
	./compile pathexec_run.c

prog: \
tcpserver tcprules tcprulescheck tcpclient who@ date@ \
finger@ http@ tcpcat mconnect mconnect-io \
rblsmtpd rts

shared: \
rblsmtpd.so

prot.o: \
compile prot.c hasshsgr.h prot.h
	./compile prot.c

rblsmtpd.so: \
load rbl.o commands.o pathexec_rundl.o load_shared.o \
dlnamespace.o dns.a unix.a socket.lib
	if test $(SYSTEM) = DARWIN ; then \
	$(LD) -bundle -undefined dynamic_lookup -o rblsmtpd.so \
	rbl.o commands.o pathexec_rundl.o load_shared.o \
	tcdlmopen.o dlnamespace.o dns.a unix.a `cat socket.lib` -lqmail; \
	else \
	$(CC) -shared -rdynamic -nostartfiles -fPIC -s -O4 -o rblsmtpd.so \
	rbl.o commands.o pathexec_rundl.o load_shared.o \
	dlnamespace.o tcdlmopen.o dns.a unix.a `cat socket.lib` -lqmail -ldl; \
	fi

rblsmtpd: \
load rblsmtpd.o commands.o pathexec_rundl.o load_shared.o \
dlnamespace.o tcdlmopen.o dns.a unix.a socket.lib
	./load rblsmtpd commands.o pathexec_rundl.o load_shared.o \
	dlnamespace.o tcdlmopen.o dns.a unix.a \
	`cat socket.lib` -lqmail -ldl

rblsmtpd.o: \
compile rblsmtpd.c commands.h pathexec.h dns.h \
haveip6.h ip6.h conf-ip
	./compile -DMAIN `grep -h -v "^#" conf-ip` rblsmtpd.c

rbl.o: \
compile rblsmtpd.c commands.h pathexec.h dns.h \
haveip6.h ip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` rblsmtpd.c -o rbl.o

readclose.o: \
compile readclose.c readclose.h 
	./compile readclose.c

tcpremoteinfo.o: \
compile tcpremoteinfo.c socket.h timeoutconn.h \
tcpremoteinfo.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` tcpremoteinfo.c

rts: \
warn-auto.sh rts.sh conf-prefix
	cat warn-auto.sh rts.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> rts
	chmod 755 rts

rules.o: \
compile rules.c rules.h ip4_bit.h ip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` rules.c

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

install: it installer DIRS
	mkdir -p $(DESTDIR)$(prefix)
	mkdir -p $(DESTDIR)$(prefix)/lib
	mkdir -p $(DESTDIR)$(shareddir)
	mkdir -p $(DESTDIR)$(mandir)
	./installer $(DESTDIR)                         < DIRS
	./installer $(DESTDIR)$(prefix)                < BIN
	./installer $(DESTDIR)$(shareddir)             < SHARED
	./installer $(DESTDIR)$(prefix)"/lib/indimail" < PLUGINS
	./installer $(DESTDIR)$(mandir)                < MAN

socket.lib: \
trylsock.c compile load
	( ( ./compile trylsock.c && \
	./load trylsock -lsocket -lnsl ) >/dev/null 2>&1 \
	&& echo -lsocket -lnsl || exit 0 ) > socket.lib
	rm -f trylsock.o trylsock

socket_accept.o: \
compile socket_accept.c socket.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_accept.c

socket_bind.o: \
compile socket_bind.c socket.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_bind.c

socket_conn.o: \
compile socket_conn.c socket.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_conn.c

socket_delay.o: \
compile socket_delay.c socket.h haveip6.h
	./compile socket_delay.c

socket_listen.o: \
compile socket_listen.c socket.h haveip6.h
	./compile socket_listen.c

socket_local.o: \
compile socket_local.c socket.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_local.c

socket_opts.o: \
compile socket_opts.c socket.h haveip6.h
	./compile socket_opts.c

socket_opts6.o: \
compile socket_opts6.c socket.h haveip6.h
	./compile socket_opts6.c

socket_remote.o: \
compile socket_remote.c socket.h ip6.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_remote.c

socket_tcp.o: \
compile socket_tcp.c socket.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_tcp.c

socket_udp.o: \
compile socket_udp.c socket.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_udp.c

systype: find-systype.sh conf-cc conf-ld trycpp.c x86cpuid.c
	( cat warn-auto.sh; \
	echo CC=\'`head -1 conf-cc`\'; \
	if test $(SYSTEM) = DARWIN ; then \
		echo LD=\'`head -1 conf-ld | sed s{-s{{`\'; \
	else \
		echo LD=\'`head -1 conf-ld`\'; fi ; \
	cat find-systype.sh; \
	) | sh > systype

tcpcat: \
warn-auto.sh tcpcat.sh conf-prefix
	cat warn-auto.sh tcpcat.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> tcpcat
	chmod 755 tcpcat

tcpclient: \
load tcpclient.o tcpremoteinfo.o timeoutconn.o \
pathexec_run.o dns.a unix.a socket.lib
	./load tcpclient tcpremoteinfo.o timeoutconn.o \
	pathexec_run.o dns.a unix.a `cat socket.lib` -lqmail

tcpclient.o: \
compile tcpclient.c ip4.h socket.h pathexec.h \
timeoutconn.h tcpremoteinfo.h dns.h haveip6.h \
ip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` tcpclient.c

tcprules: \
load tcprules.o unix.a
	./load tcprules unix.a -lqmail

tcprules.o: \
compile tcprules.c conf-ip
	./compile `grep -h -v "^#" conf-ip` tcprules.c

tcprulescheck: \
load tcprulescheck.o rules.o socket_v4mappedprefix.o \
socket_v6any.o unix.a
	./load tcprulescheck rules.o socket_v4mappedprefix.o \
	socket_v6any.o unix.a -lqmail

tcprulescheck.o: \
compile tcprulescheck.c rules.h
	./compile tcprulescheck.c

hasdlmopen.h: \
choose compile load trydlmopen.c hasdlmopen.h1 hasdlmopen.h2 conf-dlmopen
	./choose CL trydlmopen hasdlmopen.h1 hasdlmopen.h2 > hasdlmopen.h

load_shared.o: \
compile load_shared.c dlnamespace.h hasdlmopen.h pathexec.h
	./compile `grep -h -v "^#" conf-dlopen` load_shared.c

tcdlmopen.o: \
compile tcdlmopen.c env.h dlnamespace.h \
hasdlmopen.h conf-dlopen
	./compile `grep -h -v "^#" conf-dlopen` tcdlmopen.c

tcpserver_plugin.o: \
compile tcpserver_plugin.c hasdlmopen.h dlnamespace.h conf-dlopen
	./compile `grep -h -v "^#" conf-dlopen` tcpserver_plugin.c

dlnamespace.o: \
compile dlnamespace.c hasdlmopen.h pathexec.h
	./compile `grep -h -v "^#" conf-dlopen` dlnamespace.c

tcpserver: \
load tcpserver.o rules.o tcpremoteinfo.o timeoutconn.o pathexec_rundl.o \
tcpserver_plugin.o load_shared.o control.o auto_home.o dlnamespace.o \
tcdlmopen.o load_mysql.o dns.a unix.a socket.lib mysql_config
	./load tcpserver rules.o tcpremoteinfo.o timeoutconn.o load_mysql.o \
	control.o auto_home.o tcpserver_plugin.o load_shared.o \
	dlnamespace.o tcdlmopen.o pathexec_rundl.o dns.a unix.a \
	`cat socket.lib` $(SSLLIBS) -lqmail -ldl

mysql_inc: mysql_config
	./mysql_config --include > mysql_inc || true

tcpserver.o: \
compile tcpserver.c ip4.h prot.h pathexec.h \
socket.h tcpremoteinfo.h rules.h dns.h haveip6.h ip6.h \
conf-tls conf-ip haveip6.h conf-mysqlrules conf-dlopen \
hasmysql.h mysql_inc
	./compile $(SSLINCS) \
	`grep -h -v "^#" conf-tls conf-ip conf-mysqlrules conf-dlopen mysql_inc` \
	tcpserver.c

mysql_config.o: compile mysql_config.c conf-mysqlrules
	./compile `grep -h -v "^#" conf-mysqlrules` mysql_config.c
mysql_config: load mysql_config.o
	./load mysql_config

load_mysql.o: compile load_mysql.c hasmysql.h mysql_inc
	./compile `grep -h -v "^#" conf-mysqlrules mysql_inc` load_mysql.c

control.o: \
compile control.c control.h
	./compile control.c

timeoutconn.o: \
compile timeoutconn.c socket.h haveip6.h timeoutconn.h conf-ip
	./compile `grep -h -v "^#" conf-ip` timeoutconn.c

unix.a: \
makelib openreadclose.o \
pathexec_env.o prot.o readclose.o socket_accept.o socket_bind.o \
socket_conn.o socket_delay.o socket_listen.o socket_local.o \
socket_opts.o socket_opts6.o socket_remote.o socket_tcp.o socket_udp.o \
socket_conn6.o socket_bind6.o socket_accept6.o socket_recv6.o \
socket_send6.o socket_local6.o socket_tcp6.o ip4_fmt.o ip6_fmt.o \
ip4_bit.o ip6_bit.o ip4_scan.o ip6_scan.o socket_getifname.o \
socket_getifidx.o socket_v4mappedprefix.o socket_ip4loopback.o \
socket_v6any.o socket_v6loopback.o ip6_compactaddr.o ip6_expandaddr.o
	./makelib unix.a openreadclose.o \
	pathexec_env.o prot.o readclose.o socket_accept.o \
	socket_bind.o socket_conn.o socket_delay.o socket_listen.o \
	socket_local.o socket_opts.o socket_opts6.o socket_remote.o \
	socket_tcp.o socket_udp.o socket_conn6.o socket_bind6.o \
	socket_accept6.o socket_recv6.o socket_send6.o socket_local6.o \
	socket_tcp6.o ip4_fmt.o ip6_fmt.o ip4_bit.o ip6_bit.o \
	ip4_scan.o ip6_scan.o socket_getifname.o socket_getifidx.o \
	socket_v4mappedprefix.o socket_ip4loopback.o socket_v6any.o \
	socket_v6loopback.o ip6_compactaddr.o ip6_expandaddr.o

socket_conn6.o: \
compile socket_conn6.c socket.h ip4.h ip6.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_conn6.c

socket_bind6.o: \
compile socket_bind6.c socket.h ip6.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_bind6.c

socket_accept6.o: \
compile socket_accept6.c socket.h ip6.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_accept6.c

socket_recv6.o: \
compile socket_recv6.c socket.h ip6.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_recv6.c

socket_send6.o: \
compile socket_send6.c socket.h ip4.h ip6.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_send6.c

socket_local6.o: \
compile socket_local6.c socket.h haveip6.h ip6.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_local6.c

ip6_compactaddr.o: \
compile ip6_compactaddr.c ip6.h 
	./compile ip6_compactaddr.c

ip6_expandaddr.o: \
compile ip6_expandaddr.c ip6.h
	./compile ip6_expandaddr.c

ip6_fmt.o: \
compile ip6_fmt.c ip4.h ip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` ip6_fmt.c

ip6_scan.o: \
compile ip6_scan.c ip4.h ip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` ip6_scan.c

ip6_bit.o: \
compile ip6_bit.c ip6.h ip6.h
	./compile ip6_bit.c

socket_tcp6.o: \
compile socket_tcp6.c socket.h haveip6.h conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_tcp6.c

haveip6.h: \
choose tryip6.c compile haveip6.h1 haveip6.h2 conf-ip
	./compile `grep -h -v "^#" conf-ip` tryip6.c; \
	cc tryip6.o -o tryip6; \
	./choose r tryip6 haveip6.h1 haveip6.h2 > haveip6.h

socket_getifname.o: \
compile socket_getifname.c socket.h haveip6.h
	./compile socket_getifname.c

socket_getifidx.o: \
compile socket_getifidx.c socket.h haveip6.h
	./compile socket_getifidx.c

socket_ip4loopback.o: \
compile socket_ip4loopback.c
	./compile socket_ip4loopback.c

socket_v4mappedprefix.o: \
compile socket_v4mappedprefix.c conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_v4mappedprefix.c

socket_v6any.o: \
compile socket_v6any.c conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_v6any.c

socket_v6loopback.o: \
compile socket_v6loopback.c conf-ip
	./compile `grep -h -v "^#" conf-ip` socket_v6loopback.c

who@: \
warn-auto.sh who@.sh conf-prefix
	cat warn-auto.sh who@.sh \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> who@
	chmod 755 who@

clean:
	rm -f `cat TARGETS`
