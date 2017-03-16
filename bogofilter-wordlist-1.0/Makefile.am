@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
release=1

SYSCONFDIR = @sysconfdir@
sysconf_DATA = data/wordlist.db

noinst_SCRIPTS = bogofilter-wordlist.spec
#MOSTLYCLEANFILES= bogofilter-wordlist.spec

install-data-local: data/wordlist.db bogofilter-wordlist.spec

install-data-hook: 
	$(mkinstalldirs) $(DESTDIR)@sysconfdir@ || exit 1

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@version\@,$(VERSION),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@prefix\@,$(prefix),g'

bogofilter-wordlist.spec: bogofilter-wordlist.spec.in catChangeLog
	($(edit) $@.in;./catChangeLog) | $(edit) > $@
