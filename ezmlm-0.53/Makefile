# Don't edit Makefile! Use conf-* for configuration.
COFLAGS=-z+05:30
SHELL=/bin/sh

default: it
all: it

alloc.0: \
alloc.3
	nroff -man alloc.3 > alloc.0

alloc.a: \
makelib alloc.o alloc_re.o
	./makelib alloc.a alloc.o alloc_re.o

alloc.o: \
compile alloc.c alloc.h alloc.c error.h alloc.c
	./compile alloc.c

alloc_re.o: \
compile alloc_re.c alloc.h alloc_re.c byte.h alloc_re.c
	./compile alloc_re.c

auto-ccld.sh: \
conf-cc conf-ld warn-auto.sh
	( cat warn-auto.sh; \
	echo CC=\'`head -1 conf-cc`\'; \
	echo LD=\'`head -1 conf-ld`\' \
	) > auto-ccld.sh

auto-str: \
load auto-str.o substdio.a error.a str.a
	./load auto-str substdio.a error.a str.a 

auto-str.o: \
compile auto-str.c substdio.h auto-str.c readwrite.h auto-str.c \
exit.h auto-str.c
	./compile auto-str.c

auto_bin.c: \
auto-str conf-bin
	./auto-str auto_bin `head -1 conf-bin` > auto_bin.c

auto_bin.o: \
compile auto_bin.c
	./compile auto_bin.c

auto_qmail.c: \
auto-str conf-qmail
	./auto-str auto_qmail `head -1 conf-qmail` > auto_qmail.c

auto_qmail.o: \
compile auto_qmail.c
	./compile auto_qmail.c

byte_chr.o: \
compile byte_chr.c byte.h byte_chr.c
	./compile byte_chr.c

byte_copy.o: \
compile byte_copy.c byte.h byte_copy.c
	./compile byte_copy.c

byte_cr.o: \
compile byte_cr.c byte.h byte_cr.c
	./compile byte_cr.c

byte_diff.o: \
compile byte_diff.c byte.h byte_diff.c
	./compile byte_diff.c

byte_rchr.o: \
compile byte_rchr.c byte.h byte_rchr.c
	./compile byte_rchr.c

byte_zero.o: \
compile byte_zero.c byte.h byte_zero.c
	./compile byte_zero.c

case.0: \
case.3
	nroff -man case.3 > case.0

case.a: \
makelib case_diffb.o case_lowerb.o case_startb.o
	./makelib case.a case_diffb.o case_lowerb.o case_startb.o

case_diffb.o: \
compile case_diffb.c case.h case_diffb.c
	./compile case_diffb.c

case_lowerb.o: \
compile case_lowerb.c case.h case_lowerb.c
	./compile case_lowerb.c

case_startb.o: \
compile case_startb.c case.h case_startb.c
	./compile case_startb.c

compile: \
make-compile warn-auto.sh systype
	( cat warn-auto.sh; ./make-compile "`cat systype`" ) > \
	compile
	chmod 755 compile

constmap.o: \
compile constmap.c constmap.h constmap.c alloc.h constmap.c case.h \
constmap.c
	./compile constmap.c

cookie.o: \
compile cookie.c cookie.h cookie.c str.h cookie.c uint32.h cookie.c \
surfpcs.h uint32.h surfpcs.h cookie.c
	./compile cookie.c

date822fmt.o: \
compile date822fmt.c datetime.h date822fmt.c fmt.h date822fmt.c \
date822fmt.h date822fmt.c
	./compile date822fmt.c

datetime.0: \
datetime.3
	nroff -man datetime.3 > datetime.0

datetime.o: \
compile datetime.c datetime.h datetime.c
	./compile datetime.c

direntry.0: \
direntry.3
	nroff -man direntry.3 > direntry.0

direntry.h: \
compile trydrent.c direntry.h1 direntry.h2
	( ./compile trydrent.c >/dev/null 2>&1 \
	&& cat direntry.h2 || cat direntry.h1 ) > direntry.h
	rm -f trydrent.o

env.0: \
env.3
	nroff -man env.3 > env.0

env.a: \
makelib envread.o
	./makelib env.a envread.o

envread.o: \
compile envread.c env.h envread.c str.h envread.c
	./compile envread.c

error.0: \
error.3
	nroff -man error.3 > error.0

error.a: \
makelib error.o error_str.o
	./makelib error.a error.o error_str.o

error.o: \
compile error.c error.c error.h error.c
	./compile error.c

error_str.0: \
error_str.3
	nroff -man error_str.3 > error_str.0

error_str.o: \
compile error_str.c error_str.c error.h error_str.c
	./compile error_str.c

error_temp.0: \
error_temp.3
	nroff -man error_temp.3 > error_temp.0

ezmlm-list: \
load ezmlm-list.o strerr.a getln.a substdio.a stralloc.a alloc.a \
error.a open.a str.a
	./load ezmlm-list strerr.a getln.a substdio.a stralloc.a \
	alloc.a error.a open.a str.a 

ezmlm-list.0: \
ezmlm-list.1
	nroff -man ezmlm-list.1 > ezmlm-list.0

ezmlm-list.o: \
compile ezmlm-list.c stralloc.h gen_alloc.h stralloc.h ezmlm-list.c \
substdio.h ezmlm-list.c getln.h ezmlm-list.c strerr.h ezmlm-list.c \
error.h ezmlm-list.c readwrite.h ezmlm-list.c exit.h ezmlm-list.c \
str.h open.h ezmlm-list.c
	./compile ezmlm-list.c

ezmlm-make: \
load ezmlm-make.o auto_bin.o open.a getopt.a substdio.a strerr.a \
stralloc.a alloc.a error.a str.a
	./load ezmlm-make auto_bin.o open.a getopt.a substdio.a \
	strerr.a stralloc.a alloc.a error.a str.a 

ezmlm-make.0: \
ezmlm-make.1
	nroff -man ezmlm-make.1 > ezmlm-make.0

ezmlm-make.o: \
compile ezmlm-make.c ezmlm-make.c ezmlm-make.c sgetopt.h subgetopt.h \
sgetopt.h ezmlm-make.c stralloc.h gen_alloc.h stralloc.h ezmlm-make.c \
strerr.h ezmlm-make.c exit.h ezmlm-make.c readwrite.h ezmlm-make.c \
open.h ezmlm-make.c substdio.h ezmlm-make.c str.h ezmlm-make.c \
auto_bin.h ezmlm-make.c ezmlm-make.c ezmlm-make.c ezmlm-make.c \
ezmlm-make.c
	./compile ezmlm-make.c

ezmlm-manage: \
load ezmlm-manage.o auto_qmail.o getconf.o subscribe.o log.o cookie.o \
now.o datetime.o date822fmt.o slurpclose.o slurp.o qmail.o quote.o \
surf.a getln.a env.a sig.a strerr.a substdio.a stralloc.a alloc.a \
error.a str.a fs.a case.a open.a seek.a wait.a lock.a fd.a
	./load ezmlm-manage auto_qmail.o getconf.o subscribe.o \
	log.o cookie.o now.o datetime.o date822fmt.o slurpclose.o \
	slurp.o qmail.o quote.o surf.a getln.a env.a sig.a strerr.a \
	substdio.a stralloc.a alloc.a error.a str.a fs.a case.a \
	open.a seek.a wait.a lock.a fd.a 

ezmlm-manage.0: \
ezmlm-manage.1
	nroff -man ezmlm-manage.1 > ezmlm-manage.0

ezmlm-manage.o: \
compile ezmlm-manage.c ezmlm-manage.c ezmlm-manage.c error.h \
ezmlm-manage.c stralloc.h gen_alloc.h stralloc.h ezmlm-manage.c str.h \
ezmlm-manage.c env.h ezmlm-manage.c sig.h ezmlm-manage.c slurp.h \
ezmlm-manage.c getconf.h ezmlm-manage.c strerr.h ezmlm-manage.c \
byte.h ezmlm-manage.c getln.h ezmlm-manage.c case.h ezmlm-manage.c \
qmail.h substdio.h qmail.h ezmlm-manage.c substdio.h substdio.h \
ezmlm-manage.c readwrite.h ezmlm-manage.c seek.h ezmlm-manage.c \
quote.h ezmlm-manage.c datetime.h ezmlm-manage.c now.h datetime.h \
datetime.h now.h ezmlm-manage.c date822fmt.h ezmlm-manage.c fmt.h \
ezmlm-manage.c subscribe.h strerr.h strerr.h subscribe.h \
log.h open.h scan.h ezmlm-manage.c cookie.h ezmlm-manage.c
	./compile ezmlm-manage.c

ezmlm-reject: \
load ezmlm-reject.o getln.a strerr.a substdio.a error.a stralloc.a \
alloc.a str.a getopt.a case.a
	./load ezmlm-reject getln.a strerr.a substdio.a error.a \
	stralloc.a alloc.a str.a getopt.a case.a 

ezmlm-reject.0: \
ezmlm-reject.1
	nroff -man ezmlm-reject.1 > ezmlm-reject.0

ezmlm-reject.o: \
compile ezmlm-reject.c strerr.h ezmlm-reject.c substdio.h \
ezmlm-reject.c readwrite.h ezmlm-reject.c stralloc.h gen_alloc.h \
stralloc.h ezmlm-reject.c getln.h ezmlm-reject.c sgetopt.h \
case.h subgetopt.h sgetopt.h ezmlm-reject.c
	./compile ezmlm-reject.c

ezmlm-return: \
load ezmlm-return.o quote.o getconf.o issub.o subscribe.o log.o \
slurpclose.o slurp.o now.o cookie.o surf.a lock.a env.a sig.a \
strerr.a getln.a substdio.a stralloc.a alloc.a error.a str.a fs.a \
case.a open.a
	./load ezmlm-return quote.o getconf.o issub.o subscribe.o \
	log.o slurpclose.o slurp.o now.o cookie.o surf.a lock.a \
	env.a sig.a strerr.a getln.a substdio.a stralloc.a alloc.a \
	error.a str.a fs.a case.a open.a 

ezmlm-return.0: \
ezmlm-return.1
	nroff -man ezmlm-return.1 > ezmlm-return.0

ezmlm-return.o: \
compile ezmlm-return.c stralloc.h gen_alloc.h stralloc.h \
ezmlm-return.c str.h ezmlm-return.c env.h ezmlm-return.c sig.h \
ezmlm-return.c slurp.h ezmlm-return.c getconf.h ezmlm-return.c \
strerr.h ezmlm-return.c byte.h ezmlm-return.c case.h ezmlm-return.c \
getln.h ezmlm-return.c substdio.h ezmlm-return.c error.h \
ezmlm-return.c quote.h ezmlm-return.c readwrite.h ezmlm-return.c \
fmt.h ezmlm-return.c now.h datetime.h now.h ezmlm-return.c cookie.h \
scan.h open.h ezmlm-return.c subscribe.h strerr.h strerr.h subscribe.h \
lock.h log.h slurpclose.h \
ezmlm-return.c issub.h strerr.h strerr.h issub.h ezmlm-return.c
	./compile ezmlm-return.c

ezmlm-send: \
load ezmlm-send.o auto_qmail.o getconf.o qmail.o constmap.o slurp.o \
slurpclose.o wait.a getln.a strerr.a sig.a env.a open.a lock.a \
substdio.a stralloc.a alloc.a error.a str.a fd.a case.a fs.a
	./load ezmlm-send auto_qmail.o getconf.o qmail.o \
	constmap.o slurp.o slurpclose.o wait.a getln.a strerr.a \
	sig.a env.a open.a lock.a substdio.a stralloc.a alloc.a \
	error.a str.a fd.a case.a fs.a 

ezmlm-send.0: \
ezmlm-send.1
	nroff -man ezmlm-send.1 > ezmlm-send.0

ezmlm-send.o: \
compile ezmlm-send.c stralloc.h gen_alloc.h stralloc.h ezmlm-send.c \
subfd.h substdio.h subfd.h ezmlm-send.c strerr.h ezmlm-send.c error.h \
ezmlm-send.c qmail.h substdio.h substdio.h qmail.h ezmlm-send.c env.h \
ezmlm-send.c lock.h ezmlm-send.c sig.h ezmlm-send.c open.h \
ezmlm-send.c getln.h ezmlm-send.c case.h ezmlm-send.c scan.h \
ezmlm-send.c str.h ezmlm-send.c fmt.h ezmlm-send.c readwrite.h \
ezmlm-send.c exit.h ezmlm-send.c substdio.h substdio.h ezmlm-send.c \
byte.h getconf.h ezmlm-send.c constmap.h ezmlm-send.c
	./compile ezmlm-send.c

ezmlm-sub: \
load ezmlm-sub.o subscribe.o log.o now.o fs.a strerr.a getln.a \
substdio.a stralloc.a alloc.a error.a str.a case.a open.a lock.a
	./load ezmlm-sub subscribe.o log.o now.o fs.a strerr.a \
	getln.a substdio.a stralloc.a alloc.a error.a str.a case.a \
	open.a lock.a 

ezmlm-sub.0: \
ezmlm-sub.1
	nroff -man ezmlm-sub.1 > ezmlm-sub.0

ezmlm-sub.o: \
compile ezmlm-sub.c strerr.h ezmlm-sub.c subscribe.h strerr.h \
strerr.h subscribe.h ezmlm-sub.c log.h ezmlm-sub.c
	./compile ezmlm-sub.c

ezmlm-unsub: \
load ezmlm-unsub.o subscribe.o log.o now.o fs.a strerr.a getln.a \
substdio.a stralloc.a alloc.a error.a str.a case.a open.a lock.a
	./load ezmlm-unsub subscribe.o log.o now.o fs.a strerr.a \
	getln.a substdio.a stralloc.a alloc.a error.a str.a case.a \
	open.a lock.a 

ezmlm-unsub.0: \
ezmlm-unsub.1
	nroff -man ezmlm-unsub.1 > ezmlm-unsub.0

ezmlm-unsub.o: \
compile ezmlm-unsub.c strerr.h ezmlm-unsub.c subscribe.h strerr.h \
strerr.h subscribe.h ezmlm-unsub.c log.h ezmlm-unsub.c
	./compile ezmlm-unsub.c

ezmlm-warn: \
load ezmlm-warn.o auto_qmail.o getconf.o cookie.o issub.o now.o \
slurpclose.o slurp.o quote.o datetime.o date822fmt.o qmail.o surf.a \
case.a strerr.a sig.a getln.a substdio.a stralloc.a alloc.a error.a \
open.a lock.a str.a fs.a fd.a wait.a
	./load ezmlm-warn auto_qmail.o getconf.o cookie.o issub.o \
	now.o slurpclose.o slurp.o quote.o datetime.o date822fmt.o \
	qmail.o surf.a case.a strerr.a sig.a getln.a substdio.a \
	stralloc.a alloc.a error.a open.a lock.a str.a fs.a fd.a \
	wait.a 

ezmlm-warn.0: \
ezmlm-warn.1
	nroff -man ezmlm-warn.1 > ezmlm-warn.0

ezmlm-warn.o: \
compile ezmlm-warn.c ezmlm-warn.c ezmlm-warn.c direntry.h direntry.h \
direntry.h ezmlm-warn.c readwrite.h ezmlm-warn.c getln.h ezmlm-warn.c \
substdio.h ezmlm-warn.c stralloc.h gen_alloc.h stralloc.h \
ezmlm-warn.c slurp.h ezmlm-warn.c getconf.h ezmlm-warn.c byte.h \
ezmlm-warn.c error.h ezmlm-warn.c str.h ezmlm-warn.c strerr.h \
ezmlm-warn.c sig.h ezmlm-warn.c now.h datetime.h now.h ezmlm-warn.c \
datetime.h datetime.h ezmlm-warn.c date822fmt.h ezmlm-warn.c fmt.h \
ezmlm-warn.c cookie.h ezmlm-warn.c qmail.h substdio.h substdio.h \
issub.h quote.h open.h lock.h scan.h qmail.h ezmlm-warn.c
	./compile ezmlm-warn.c

ezmlm-weed: \
load ezmlm-weed.o getln.a strerr.a substdio.a error.a stralloc.a \
alloc.a str.a
	./load ezmlm-weed getln.a strerr.a substdio.a error.a \
	stralloc.a alloc.a str.a 

ezmlm-weed.0: \
ezmlm-weed.1
	nroff -man ezmlm-weed.1 > ezmlm-weed.0

ezmlm-weed.o: \
compile ezmlm-weed.c stralloc.h gen_alloc.h stralloc.h ezmlm-weed.c \
str.h ezmlm-weed.c byte.h ezmlm-weed.c readwrite.h ezmlm-weed.c \
substdio.h ezmlm-weed.c getln.h ezmlm-weed.c strerr.h ezmlm-weed.c
	./compile ezmlm-weed.c

ezmlm.0: \
ezmlm.5
	nroff -man ezmlm.5 > ezmlm.0

fd.a: \
makelib fd_copy.o fd_move.o
	./makelib fd.a fd_copy.o fd_move.o

fd_copy.0: \
fd_copy.3
	nroff -man fd_copy.3 > fd_copy.0

fd_copy.o: \
compile fd_copy.c fd_copy.c fd.h fd_copy.c
	./compile fd_copy.c

fd_move.0: \
fd_move.3
	nroff -man fd_move.3 > fd_move.0

fd_move.o: \
compile fd_move.c fd.h fd_move.c
	./compile fd_move.c

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

fmt_str.o: \
compile fmt_str.c fmt.h fmt_str.c
	./compile fmt_str.c

fmt_uint.o: \
compile fmt_uint.c fmt.h fmt_uint.c
	./compile fmt_uint.c

fmt_uint0.o: \
compile fmt_uint0.c fmt.h fmt_uint0.c
	./compile fmt_uint0.c

fmt_ulong.o: \
compile fmt_ulong.c fmt.h fmt_ulong.c
	./compile fmt_ulong.c

fork.h: \
compile load tryvfork.c fork.h1 fork.h2
	( ( ./compile tryvfork.c && ./load tryvfork ) >/dev/null \
	2>&1 \
	&& cat fork.h2 || cat fork.h1 ) > fork.h
	rm -f tryvfork.o tryvfork

fs.a: \
makelib fmt_str.o fmt_uint.o fmt_uint0.o fmt_ulong.o scan_ulong.o \
scan_8long.o
	./makelib fs.a fmt_str.o fmt_uint.o fmt_uint0.o \
	fmt_ulong.o scan_ulong.o scan_8long.o

getconf.o: \
compile getconf.c stralloc.h gen_alloc.h stralloc.h getconf.c slurp.h \
byte.h getconf.c strerr.h getconf.c getconf.h getconf.c
	./compile getconf.c

getln.0: \
getln.3
	nroff -man getln.3 > getln.0

getln.a: \
makelib getln.o getln2.o
	./makelib getln.a getln.o getln2.o

getln.o: \
compile getln.c substdio.h getln.c byte.h getln.c stralloc.h \
gen_alloc.h stralloc.h getln.c getln.h getln.c
	./compile getln.c

getln2.0: \
getln2.3
	nroff -man getln2.3 > getln2.0

getln2.o: \
compile getln2.c substdio.h getln2.c stralloc.h gen_alloc.h \
stralloc.h getln2.c byte.h getln2.c getln.h getln2.c
	./compile getln2.c

getopt.0: \
getopt.3
	nroff -man getopt.3 > getopt.0

getopt.a: \
makelib subgetopt.o sgetopt.o
	./makelib getopt.a subgetopt.o sgetopt.o

hasflock.h: \
tryflock.c compile load
	( ( ./compile tryflock.c && ./load tryflock ) >/dev/null \
	2>&1 \
	&& echo \#define HASFLOCK 1 || exit 0 ) > hasflock.h
	rm -f tryflock.o tryflock

hassgact.h: \
trysgact.c compile load
	( ( ./compile trysgact.c && ./load trysgact ) >/dev/null \
	2>&1 \
	&& echo \#define HASSIGACTION 1 || exit 0 ) > hassgact.h
	rm -f trysgact.o trysgact

install: \
load install.o getln.a strerr.a substdio.a stralloc.a alloc.a open.a \
error.a str.a fs.a
	./load install getln.a strerr.a substdio.a stralloc.a \
	alloc.a open.a error.a str.a fs.a 

install.o: \
compile install.c substdio.h install.c stralloc.h gen_alloc.h \
stralloc.h install.c getln.h install.c readwrite.h install.c exit.h \
install.c open.h install.c error.h install.c strerr.h install.c \
scan.h byte.h install.c
	./compile install.c

issub.o: \
compile issub.c stralloc.h gen_alloc.h stralloc.h issub.c getln.h \
issub.c readwrite.h issub.c substdio.h issub.c open.h issub.c byte.h \
issub.c case.h issub.c lock.h issub.c error.h issub.c issub.h \
strerr.h issub.h issub.c uint32.h issub.c
	./compile issub.c

it: \
ezmlm-make ezmlm-manage ezmlm-send ezmlm-reject ezmlm-return \
ezmlm-warn ezmlm-weed ezmlm-list ezmlm-sub ezmlm-unsub ezmlm-queue

load: \
make-load warn-auto.sh systype
	( cat warn-auto.sh; ./make-load "`cat systype`" ) > load
	chmod 755 load

lock.a: \
makelib lock_ex.o
	./makelib lock.a lock_ex.o

lock_ex.o: \
compile lock_ex.c lock_ex.c lock_ex.c lock_ex.c hasflock.h lock_ex.c \
lock.h lock_ex.c
	./compile lock_ex.c

log.o: \
compile log.c substdio.h log.c readwrite.h log.c stralloc.h \
gen_alloc.h stralloc.h log.c log.h log.c now.h datetime.h now.h log.c \
fmt.h log.c open.h log.c
	./compile log.c

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
ezmlm.0 ezmlm-make.0 ezmlm-manage.0 ezmlm-send.0 ezmlm-reject.0 \
ezmlm-return.0 ezmlm-warn.0 ezmlm-weed.0 ezmlm-list.0 ezmlm-sub.0 \
ezmlm-unsub.0 alloc.0 case.0 datetime.0 direntry.0 env.0 error.0 \
error_str.0 error_temp.0 ezmlm.0 fd_copy.0 fd_move.0 getln.0 getln2.0 \
getopt.0 now.0 sgetopt.0 stralloc.0 subfd.0 subgetopt.0 substdio.0 \
substdio_copy.0 substdio_in.0 substdio_out.0 surf.0 surfpcs.0 wait.0

now.0: \
now.3
	nroff -man now.3 > now.0

now.o: \
compile now.c now.c datetime.h now.c now.h datetime.h datetime.h \
now.h now.c
	./compile now.c

open.a: \
makelib open_append.o open_read.o open_trunc.o
	./makelib open.a open_append.o open_read.o open_trunc.o

open_append.o: \
compile open_append.c open_append.c open_append.c open.h \
open_append.c
	./compile open_append.c

open_read.o: \
compile open_read.c open_read.c open_read.c open.h open_read.c
	./compile open_read.c

open_trunc.o: \
compile open_trunc.c open_trunc.c open_trunc.c open.h open_trunc.c
	./compile open_trunc.c

qmail.o: \
compile qmail.c substdio.h qmail.c readwrite.h qmail.c wait.h qmail.c \
exit.h qmail.c fork.h qmail.c fd.h qmail.c qmail.h substdio.h \
substdio.h qmail.h qmail.c auto_qmail.h qmail.c
	./compile qmail.c

quote.o: \
compile quote.c stralloc.h gen_alloc.h stralloc.h quote.c str.h \
quote.c quote.h quote.c
	./compile quote.c

scan_8long.o: \
compile scan_8long.c scan.h scan_8long.c
	./compile scan_8long.c

scan_ulong.o: \
compile scan_ulong.c scan.h scan_ulong.c
	./compile scan_ulong.c

seek.a: \
makelib seek_set.o
	./makelib seek.a seek_set.o

seek_set.o: \
compile seek_set.c seek_set.c seek.h seek_set.c
	./compile seek_set.c

setup: \
it man install conf-bin conf-man
	mkdir -p $(DESTDIR)"`head -n 1 conf-bin`"
	./install $(DESTDIR)"`head -1 conf-bin`" < BIN
	mkdir -p $(DESTDIR)"`head -n 1 conf-man`"
	./install $(DESTDIR)"`head -1 conf-man`" < MAN

sgetopt.0: \
sgetopt.3
	nroff -man sgetopt.3 > sgetopt.0

sgetopt.o: \
compile sgetopt.c substdio.h sgetopt.c subfd.h substdio.h substdio.h \
subfd.h sgetopt.c sgetopt.h sgetopt.h subgetopt.h sgetopt.h sgetopt.c \
subgetopt.h subgetopt.h sgetopt.c
	./compile sgetopt.c

shar: \
FILES BLURB README INSTALL TODO THANKS CHANGES FILES BIN MAN VERSION \
SYSDEPS Makefile ezmlm.5 ezmlm-make.1 ezmlm-make.c ezmlm-send.1 \
ezmlm-send.c ezmlm-reject.1 ezmlm-reject.c ezmlm-list.1 ezmlm-list.c \
ezmlm-sub.1 ezmlm-sub.c ezmlm-unsub.1 ezmlm-unsub.c ezmlm-manage.1 \
ezmlm-manage.c ezmlm-return.1 ezmlm-return.c ezmlm-warn.1 \
ezmlm-warn.c ezmlm-weed.1 ezmlm-weed.c getconf.h getconf.c log.h \
log.c issub.h issub.c subscribe.h subscribe.c cookie.h cookie.c \
auto-str.c conf-bin auto_bin.h conf-man install.c conf-cc conf-ld \
find-systype.sh make-compile.sh make-load.sh make-makelib.sh trycpp.c \
warn-auto.sh fork.h1 fork.h2 tryvfork.c wait.3 wait.h wait_pid.c \
trywaitp.c error.3 error_str.3 error_temp.3 error.h error.c \
error_str.c substdio.3 substdio_copy.3 substdio_in.3 substdio_out.3 \
substdio.h substdio.c substdi.c substdo.c substdio_copy.c subfd.3 \
subfd.h subfderr.c readwrite.h exit.h byte.h byte_chr.c byte_copy.c \
byte_cr.c byte_diff.c byte_rchr.c byte_zero.c str.h str_chr.c \
str_cpy.c str_diff.c str_diffn.c str_len.c str_rchr.c str_start.c \
getopt.3 sgetopt.3 subgetopt.3 sgetopt.h sgetopt.c subgetopt.h \
subgetopt.c strerr.h strerr.c strerr_sys.c strerr_die.c gen_alloc.h \
gen_allocdefs.h stralloc.3 stralloc.h stralloc_eady.c stralloc_pend.c \
stralloc_copy.c stralloc_opyb.c stralloc_opys.c stralloc_cat.c \
stralloc_catb.c stralloc_cats.c stralloc_arts.c alloc.3 alloc.h \
alloc.c alloc_re.c open.h open_append.c open_read.c open_trunc.c \
uint32.h1 uint32.h2 tryulong32.c case.3 case.h case_diffb.c \
case_lowerb.c case_startb.c fmt.h fmt_str.c fmt_uint.c fmt_uint0.c \
fmt_ulong.c scan.h scan_ulong.c scan_8long.c lock.h lock_ex.c \
tryflock.c env.3 env.h envread.c slurpclose.h slurpclose.c sig.h \
sig_catch.c sig_pipe.c trysgact.c datetime.3 datetime.h datetime.c \
date822fmt.h date822fmt.c now.3 now.h now.c quote.h quote.c seek.h \
seek_set.c conf-qmail auto_qmail.h qmail.h qmail.c direntry.3 \
direntry.h1 direntry.h2 trydrent.c getln.3 getln.h getln.c getln2.3 \
getln2.c fd.h fd_copy.3 fd_copy.c fd_move.3 fd_move.c surf.3 surf.h \
surf.c surfpcs.3 surfpcs.h surfpcs.c slurp.h slurp.c constmap.h \
constmap.c
	shar -m `cat FILES` > shar
	chmod 400 shar

sig.a: \
makelib sig_catch.o sig_pipe.o
	./makelib sig.a sig_catch.o sig_pipe.o

sig_catch.o: \
compile sig_catch.c sig_catch.c sig.h sig_catch.c hassgact.h \
sig_catch.c
	./compile sig_catch.c

sig_pipe.o: \
compile sig_pipe.c sig_pipe.c sig.h sig_pipe.c
	./compile sig_pipe.c

slurp.o: \
compile slurp.c stralloc.h gen_alloc.h stralloc.h slurp.c slurp.h \
slurpclose.h slurp.c error.h slurp.c open.h slurp.c
	./compile slurp.c

slurpclose.o: \
compile slurpclose.c stralloc.h gen_alloc.h stralloc.h slurpclose.c \
readwrite.h slurpclose.c slurpclose.h slurpclose.c error.h \
slurpclose.c
	./compile slurpclose.c

str.a: \
makelib str_len.o str_diff.o str_diffn.o str_cpy.o str_chr.o \
str_rchr.o str_start.o byte_chr.o byte_rchr.o byte_diff.o byte_copy.o \
byte_cr.o byte_zero.o
	./makelib str.a str_len.o str_diff.o str_diffn.o str_cpy.o \
	str_chr.o str_rchr.o str_start.o byte_chr.o byte_rchr.o \
	byte_diff.o byte_copy.o byte_cr.o byte_zero.o

str_chr.o: \
compile str_chr.c str.h str_chr.c
	./compile str_chr.c

str_cpy.o: \
compile str_cpy.c str.h str_cpy.c
	./compile str_cpy.c

str_diff.o: \
compile str_diff.c str.h str_diff.c
	./compile str_diff.c

str_diffn.o: \
compile str_diffn.c str.h str_diffn.c
	./compile str_diffn.c

str_len.o: \
compile str_len.c str.h str_len.c
	./compile str_len.c

str_rchr.o: \
compile str_rchr.c str.h str_rchr.c
	./compile str_rchr.c

str_start.o: \
compile str_start.c str.h str_start.c
	./compile str_start.c

stralloc.0: \
stralloc.3
	nroff -man stralloc.3 > stralloc.0

stralloc.a: \
makelib stralloc_eady.o stralloc_pend.o stralloc_copy.o \
stralloc_opys.o stralloc_opyb.o stralloc_cat.o stralloc_cats.o \
stralloc_catb.o stralloc_arts.o
	./makelib stralloc.a stralloc_eady.o stralloc_pend.o \
	stralloc_copy.o stralloc_opys.o stralloc_opyb.o \
	stralloc_cat.o stralloc_cats.o stralloc_catb.o \
	stralloc_arts.o

stralloc_arts.o: \
compile stralloc_arts.c byte.h stralloc_arts.c str.h stralloc_arts.c \
stralloc.h gen_alloc.h stralloc.h stralloc_arts.c
	./compile stralloc_arts.c

stralloc_cat.o: \
compile stralloc_cat.c byte.h stralloc_cat.c stralloc.h gen_alloc.h \
stralloc.h stralloc_cat.c
	./compile stralloc_cat.c

stralloc_catb.o: \
compile stralloc_catb.c stralloc.h gen_alloc.h stralloc.h \
stralloc_catb.c byte.h stralloc_catb.c
	./compile stralloc_catb.c

stralloc_cats.o: \
compile stralloc_cats.c byte.h stralloc_cats.c str.h stralloc_cats.c \
stralloc.h gen_alloc.h stralloc.h stralloc_cats.c
	./compile stralloc_cats.c

stralloc_copy.o: \
compile stralloc_copy.c byte.h stralloc_copy.c stralloc.h gen_alloc.h \
stralloc.h stralloc_copy.c
	./compile stralloc_copy.c

stralloc_eady.o: \
compile stralloc_eady.c alloc.h stralloc_eady.c stralloc.h \
gen_alloc.h stralloc.h stralloc_eady.c gen_allocdefs.h \
gen_allocdefs.h gen_allocdefs.h stralloc_eady.c
	./compile stralloc_eady.c

stralloc_opyb.o: \
compile stralloc_opyb.c stralloc.h gen_alloc.h stralloc.h \
stralloc_opyb.c byte.h stralloc_opyb.c
	./compile stralloc_opyb.c

stralloc_opys.o: \
compile stralloc_opys.c byte.h stralloc_opys.c str.h stralloc_opys.c \
stralloc.h gen_alloc.h stralloc.h stralloc_opys.c
	./compile stralloc_opys.c

stralloc_pend.o: \
compile stralloc_pend.c alloc.h stralloc_pend.c stralloc.h \
gen_alloc.h stralloc.h stralloc_pend.c gen_allocdefs.h \
gen_allocdefs.h gen_allocdefs.h stralloc_pend.c
	./compile stralloc_pend.c

strerr.a: \
makelib strerr.o strerr_sys.o strerr_die.o
	./makelib strerr.a strerr.o strerr_sys.o strerr_die.o

strerr.o: \
compile strerr.c stralloc.h gen_alloc.h stralloc.h strerr.c strerr.h \
strerr.c
	./compile strerr.c

strerr_die.o: \
compile strerr_die.c substdio.h strerr_die.c subfd.h substdio.h \
substdio.h subfd.h strerr_die.c exit.h strerr_die.c strerr.h \
strerr_die.c
	./compile strerr_die.c

strerr_sys.o: \
compile strerr_sys.c error.h strerr_sys.c strerr.h strerr_sys.c
	./compile strerr_sys.c

subfd.0: \
subfd.3
	nroff -man subfd.3 > subfd.0

subfderr.o: \
compile subfderr.c readwrite.h subfderr.c substdio.h subfderr.c \
subfd.h substdio.h substdio.h subfd.h subfderr.c
	./compile subfderr.c

subgetopt.0: \
subgetopt.3
	nroff -man subgetopt.3 > subgetopt.0

subgetopt.o: \
compile subgetopt.c subgetopt.h subgetopt.h subgetopt.c
	./compile subgetopt.c

subscribe.o: \
compile subscribe.c stralloc.h gen_alloc.h stralloc.h subscribe.c \
getln.h subscribe.c readwrite.h subscribe.c substdio.h subscribe.c \
strerr.h subscribe.c open.h subscribe.c byte.h subscribe.c case.h \
str.h subscribe.c lock.h subscribe.c error.h subscribe.c uint32.h \
subscribe.c subscribe.h strerr.h strerr.h subscribe.h subscribe.c
	./compile subscribe.c

substdi.o: \
compile substdi.c substdio.h substdi.c byte.h substdi.c error.h \
substdi.c
	./compile substdi.c

substdio.0: \
substdio.3
	nroff -man substdio.3 > substdio.0

substdio.a: \
makelib substdio.o substdi.o substdo.o subfderr.o substdio_copy.o
	./makelib substdio.a substdio.o substdi.o substdo.o \
	subfderr.o substdio_copy.o

substdio.o: \
compile substdio.c substdio.h substdio.c
	./compile substdio.c

substdio_copy.0: \
substdio_copy.3
	nroff -man substdio_copy.3 > substdio_copy.0

substdio_copy.o: \
compile substdio_copy.c substdio.h substdio_copy.c
	./compile substdio_copy.c

substdio_in.0: \
substdio_in.3
	nroff -man substdio_in.3 > substdio_in.0

substdio_out.0: \
substdio_out.3
	nroff -man substdio_out.3 > substdio_out.0

substdo.o: \
compile substdo.c substdio.h substdo.c str.h substdo.c byte.h \
substdo.c error.h substdo.c
	./compile substdo.c

surf.0: \
surf.3
	nroff -man surf.3 > surf.0

surf.a: \
makelib surf.o surfpcs.o
	./makelib surf.a surf.o surfpcs.o

surf.o: \
compile surf.c surf.h surf.c uint32.h surf.c
	./compile surf.c

surfpcs.0: \
surfpcs.3
	nroff -man surfpcs.3 > surfpcs.0

surfpcs.o: \
compile surfpcs.c surf.h surfpcs.c surfpcs.h uint32.h surfpcs.h \
surfpcs.c
	./compile surfpcs.c

systype: \
find-systype trycpp.c
	./find-systype > systype

uint32.h: \
tryulong32.c compile load uint32.h1 uint32.h2
	( ( ./compile tryulong32.c && ./load tryulong32 && \
	./tryulong32 ) >/dev/null 2>&1 \
	&& cat uint32.h2 || cat uint32.h1 ) > uint32.h
	rm -f tryulong32.o tryulong32

wait.0: \
wait.3
	nroff -man wait.3 > wait.0

wait.a: \
makelib wait_pid.o
	./makelib wait.a wait_pid.o

wait_pid.o: \
compile wait_pid.c wait_pid.c wait_pid.c error.h wait_pid.c
	./compile wait_pid.c

ezmlm-queue: ezmlm-queue.in conf-prefix conf-sysconfdir warn-auto.sh
	cat warn-auto.sh ezmlm-queue.in \
	| sed s}PREFIX}"`head -1 conf-prefix`"}g \
	| sed s}SYSCONFDIR}"`head -1 conf-sysconfdir`"}g \
	> ezmlm-queue
	chmod 755 ezmlm-queue

clean: TARGETS
	/bin/rm -f `cat TARGETS`
