sysconfdir=/etc/indimail
mandir=/usr/share/man
prefix=/usr
version=1.0
#

all: rules debian.tar.gz dummy.tar.gz  \
	bogofilter-wordlist.dsc

clean:
	/bin/rm -f rules *.dsc debian.tar.gz dummy.tar.gz

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@prefix\@,$(prefix),g'

rules: rules.in Makefile
	$(edit) $@.in > $@; chmod +x $@;
bogofilter-wordlist.dsc: bogofilter-wordlist.dsc.in Makefile
	$(edit) $@.in > $@
changelog:changelog.in
	$(edit) $@.in > $@

dummy.tar.gz: README
	tar cf - README |gzip > $@
debian.tar.gz: copyright rules control changelog compat bogofilter-wordlist.install
	tar cf - copyright rules control changelog compat bogofilter-wordlist.install|gzip > $@
