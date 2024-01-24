# Don't edit Makefile! Use conf-* for configuration.
COFLAGS=-z+05:30
SHELL=/bin/sh
include Makevars.$(SYSTEM)

edit = sed \
	-e 's,@indimaildir\@,$(qmaildir),g' \
	-e 's,@qmaildir\@,$(qmaildir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@qsysconfdir\@,$(qsysconfdir),g' \
	-e 's,@sysconfdir\@,$(qsysconfdir),g' \
	-e 's,@controldir\@,"$(qsysconfdir)/control",g' \
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

it = envdir envuidgid setlock setuidgid softlimit svc svok \
	svstat tai64n tai64nlocal tai64nunix tai2tai64n tai64n2tai \
	spipe qfilelog multipipe teepipe logselect svscan qlogselect \
	supervise readproctitle multilog docker-entrypoint fghack \
	pgrphack svps svscanboot upstart systemd qmailctl openrc \
	minisvc instcheck installer daemontools.perms daemontools.changes

man = spipe.1 qfilelog.1 multipipe.1 teepipe.1 tai2tai64n.1 \
	tai64n2tai.1 envdir.8 envuidgid.8 fghack.8 multilog.8 pgrphack.8 \
	readproctitle.8 setlock.8 setuidgid.8 softlimit.8 supervise.8 \
	svc.8 svok.8 svscan.8 svstat.8 tai64n.8 tai64nlocal.8 \
	tai64nunix.8 logselect.8 svscanboot.8 qmailctl.8  qlogselect.1 \
	minisvc.8 docker-entrypoint.8 runpod.1

include Makefile.$(SYSTEM)

default: all

all: $(it) $(man)

distclean: clean
	/bin/rm -f Makefile. Makevars. 

clean:
	rm -f `cat TARGETS`

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

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

systype: \
find-systype trycpp.c
	./find-systype > systype

auto-str: load auto-str.o
	./load auto-str -lqmail

auto-str.o: compile auto-str.c
	./compile auto-str.c

choose: choose.sh warn-auto.sh
	rm -f choose
	cat warn-auto.sh choose.sh > choose
	chmod 555 choose

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

auto_sysconfdir.c: auto-str conf-sysconfdir
	./auto-str auto_sysconfdir `head -1 conf-sysconfdir` > auto_sysconfdir.c

auto_sysconfdir.o: compile auto_sysconfdir.c
	./compile auto_sysconfdir.c

envdir: load envdir.o
	./load envdir -lqmail

envdir.o: compile envdir.c
	./compile envdir.c

envuidgid: envuidgid.o load 
	./load envuidgid -lqmail

envuidgid.o: compile envuidgid.c
	./compile envuidgid.c

fghack: fghack.o load
	./load fghack -lqmail

fghack.o: compile fghack.c
	./compile fghack.c
	
multilog: load multilog.o match.o deepsleep.o
	./load multilog match.o deepsleep.o \
	-lqmail

multilog.o: compile deepsleep.h match.h multilog.c
	./compile multilog.c

deepsleep.o: compile deepsleep.c deepsleep.h
	./compile deepsleep.c

match.o: compile match.c match.h
	./compile match.c

pgrphack: load pgrphack.o
	./load pgrphack -lqmail

pgrphack.o: compile pgrphack.c
	./compile pgrphack.c

readproctitle: load readproctitle.o
	./load readproctitle -lqmail

readproctitle.o: compile readproctitle.c
	./compile readproctitle.c

setlock: load setlock.o
	./load setlock -lqmail

setlock.o: compile setlock.c
	./compile setlock.c

setuidgid: load setuidgid.o
	./load setuidgid -lqmail

setuidgid.o: compile setuidgid.c
	./compile setuidgid.c

softlimit: load softlimit.o
	./load softlimit -lqmail

softlimit.o: compile softlimit.c
	./compile softlimit.c

supervise: deepsleep.o load supervise.o subreaper.o
	./load supervise deepsleep.o subreaper.o -lqmail

supervise.o: compile supervise.c deepsleep.h \
subreaper.h conf-runfs
	./compile `grep -v "^#" conf-runfs` supervise.c

subreaper.o: compile subreaper.c subreaper.h
	./compile subreaper.c

run_init.o: compile run_init.c
	./compile `grep -v "^#" conf-runfs` run_init.c

svc: load svc.o run_init.o
	./load svc run_init.o -lqmail

svc.o: compile svc.c conf-runfs
	./compile `grep -v "^#" conf-runfs` svc.c

svok: load svok.o run_init.o
	./load svok run_init.o -lqmail

svok.o: compile svok.c conf-runfs
	./compile `grep -v "^#" conf-runfs` svok.c

svscan: load svscan.o
	./load svscan -lqmail

svscan.o: compile svscan.c auto_sysconfdir.h conf-runfs
	./compile `grep -v "^#" conf-runfs` svscan.c

svscan.8: svscan.9 conf-sysconfdir conf-servicedir
	$(edit) svscan.9 > svscan.8

svstat: load svstat.o run_init.o
	./load svstat run_init.o -lqmail

svstat.o: compile svstat.c conf-runfs
	./compile `grep -v "^#" conf-runfs` svstat.c

tai64n: load tai64n.o
	./load tai64n -lqmail

tai64n.o: compile tai64n.c
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
compile recordio.c
	./compile recordio.c

recordio: \
load recordio.o
	./load recordio -lqmail

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

multipipe.o: compile multipipe.c bool.h svcfns.h
	./compile multipipe.c

teepipe: load teepipe.o
	./load teepipe -lqmail

teepipe.o: compile teepipe.c
	./compile teepipe.c

logselect.o: compile sorted.h logselect.c
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

qlogselect: qlogselect.pl conf-prefix
	cat qlogselect.pl \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	> qlogselect
	chmod 755 qlogselect

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

svps: svps.in conf-prefix conf-servicedir
	cat warn-auto.sh $@.in  | $(edit) > $@
	chmod +x $@

upstart: upstart.in conf-prefix conf-servicedir
	( \
	echo "# svscan - runlevel compatibility"; \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	echo "#"; \
	$(edit) upstart.in \
	) > upstart

systemd: systemd.in conf-libexec conf-servicedir
	( \
	echo "# svscan - runlevel compatibility"; \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	echo "#"; \
	$(edit) systemd.in \
	) > systemd
set-network-wait: set-network-wait.in conf-shared
	( \
	cat warn-auto.sh; \
	echo "# svscan service override"; \
	echo "#"; \
	$(edit) $@.in; \
	) > $@
	chmod +x $@
openrc: openrc.in conf-libexec conf-servicedir
	( \
	echo "#!/sbin/openrc-run"; \
	echo "# svscan - openrc script"; \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	echo "#"; \
	$(edit) openrc.in \
	) > openrc

svscanboot: \
warn-auto.sh svscanboot.sh conf-prefix conf-sysconfdir \
conf-libexec conf-servicedir
	( \
	cat warn-auto.sh; \
	$(edit) svscanboot.sh \
	) > svscanboot

svscanboot.8: svscanboot.9 conf-libexec conf-servicedir
	$(edit) svscanboot.9 > svscanboot.8

svscan_rc: \
warn-auto.sh conf-prefix svscan_rc.in \
conf-prefix conf-libexec conf-sysconfdir conf-shared
	(cat warn-auto.sh; $(edit) $@.in) > $@

svscan.plist: \
warn-auto.sh conf-sysconfdir conf-libexec conf-servicedir
	( \
	echo "# WARNING: This file was auto-generated. Do not edit!"; \
	echo "#"; \
	$(edit) $@.in \
	) > $@

qmailctl: warn-auto.sh qmailctl.sh conf-prefix conf-libexec \
conf-servicedir conf-sysconfdir
	( \
	cat warn-auto.sh; \
	$(edit) qmailctl.sh \
	) > qmailctl
	chmod +x qmailctl

qmailctl.8: qmailctl.9 conf-servicedir
	$(edit) qmailctl.9 > qmailctl.8

docker-entrypoint: \
warn-auto.sh docker-entrypoint.sh conf-prefix conf-servicedir \
conf-sysconfdir
	( \
	cat warn-auto.sh; \
	$(edit) $@.sh \
	) > $@
	chmod 755 docker-entrypoint

docker-entrypoint.8: docker-entrypoint.9 conf-servicedir
	$(edit) docker-entrypoint.9 > docker-entrypoint.8

runpod: warn-auto.sh runpod.sh
	(cat warn-auto.sh runpod.sh) > runpod
	chmod +x runpod

minisvc: warn-auto.sh minisvc.in conf-prefix conf-sysconfdir \
conf-servicedir conf-shared conf-logdir
	( \
	cat warn-auto.sh; \
	$(edit) $@.in \
	) > $@
	chmod +x $@

minisvc.8: minisvc.9 conf-servicedir
	$(edit) minisvc.9 > $@

daemontools.perms: perm_list.in \
conf-prefix conf-libexec conf-sysconfdir conf-shared
	$(edit) perm_list.in > $@

daemontools.spec: daemontools.spec.in doc/ChangeLog \
conf-release conf-version conf-prefix conf-libexec conf-qmail \
conf-sysconfdir conf-shared conf-servicedir conf-email
	(cat $@.in; sh ./catChangeLog) | $(edit) > daemontools.spec

daemontools.changes: catChangeLog doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@

PKGBUILD: PKGBUILD.in doc/ChangeLog conf-release \
conf-version conf-prefix conf-libexec conf-qmail \
conf-sysconfdir conf-shared conf-servicedir conf-email \
daemontools.changes
	cat $@.in | $(edit) > PKGBUILD

instcheck: \
warn-auto.sh instcheck.in conf-libexec conf-sysconfdir
	cat warn-auto.sh instcheck.in | $(edit) > $@
	chmod +x $@

installer: \
load installer.o
	./load installer -lqmail

installer.o: \
compile installer.c
	./compile installer.c

INPUT: INPUT.in \
conf-prefix conf-libexec conf-qmail \
conf-sysconfdir conf-shared
	$(edit) INPUT.in > $@

DIRS: DIRS.in
	$(edit) $@.in > $@

BIN: BIN.in
	([ -f BIN.$(SYSTEM) ] && \
	cat BIN.in BIN.$(SYSTEM) || cat BIN.in) > BIN

LIBEXEC: LIBEXEC.in
	([ -f LIBEXEC.$(SYSTEM) ] && \
	cat LIBEXEC.in LIBEXEC.$(SYSTEM) || cat LIBEXEC.in) > LIBEXEC

SBIN: SBIN.in
	([ -f SBIN.$(SYSTEM) ] && \
	cat SBIN.in SBIN.$(SYSTEM) || cat SBIN.in) > SBIN

SHARED: SHARED.in
	([ -f SHARED.$(SYSTEM) ] && \
	cat SHARED.in SHARED.$(SYSTEM) || cat SHARED.in) > SHARED

MAN: MAN.in
	([ -f MAN.$(SYSTEM) ] && \
	cat MAN.in MAN.$(SYSTEM) || cat MAN.in) > MAN

install-strip: install

install: $(it) $(man) installer DIRS BIN SBIN MAN SHARED LIBEXEC INPUT
	./installer $(DESTDIR)                         < DIRS
	./installer $(DESTDIR)$(prefix)                < BIN
	./installer $(DESTDIR)$(prefix)                < SBIN
	./installer $(DESTDIR)$(shareddir)             < SHARED
	./installer $(DESTDIR)$(qsysconfdir)           < ETC
	./installer $(DESTDIR)$(libexecdir)            < LIBEXEC
	./installer $(DESTDIR)$(mandir)                < MAN

uninstall: installer DIRS BIN SBIN SHARED MAN
	sed -n '1!G;h;$$p' MAN     | ./installer -ui $(DESTDIR)$(mandir)
	sed -n '1!G;h;$$p' LIBEXEC | ./installer -ui $(DESTDIR)$(libexecdir)
	sed -n '1!G;h;$$p' ETC     | ./installer -ui $(DESTDIR)$(qsysconfdir)
	sed -n '1!G;h;$$p' SHARED  | ./installer -ui $(DESTDIR)$(shareddir)
	sed -n '1!G;h;$$p' SBIN    | ./installer -ui $(DESTDIR)$(prefix)
	sed -n '1!G;h;$$p' BIN     | ./installer -ui $(DESTDIR)$(prefix)
	sed -n '1!G;h;$$p' DIRS    | ./installer -u  $(DESTDIR)

debian/Makefile: debian/Makefile.in conf-release conf-version \
conf-prefix conf-libexec conf-qmail conf-sysconfdir conf-shared
	$(edit) $@.in > $@
