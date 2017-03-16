@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
release=1

SYSCONFDIR = @sysconfdir@
sysconf_DATA = wordlist.db

noinst_SCRIPTS = bogofilter-wordlist.spec
MOSTLYCLEANFILES= create_rpm

all-local: create_rpm bogofilter-wordlist.spec
install-data-local: wordlist.db

install-data-hook: 
	$(mkinstalldirs) $(DESTDIR)@sysconfdir@ || exit 1

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@version\@,$(VERSION),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@prefix\@,$(prefix),g'

bogofilter-wordlist.spec: bogofilter-wordlist.spec.in catChangeLog
	($(edit) $@.in;./catChangeLog) | $(edit) > $@

create_rpm: create_rpm.in
	$(edit) $@.in > $@
wordlist.db: training/easy_ham training/easy_ham2 training/hard_ham \
training/spam training/spam_2 training/spam_3
	./training.sh

