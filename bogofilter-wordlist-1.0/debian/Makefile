sysconfdir=/etc/indimail
mandir=/usr/share/man
prefix=/usr
version=1.0
#

all: rules debian.tar.gz dummy.tar.gz  \
	bogofilter-wordlist-Debian_8.0.dsc \
	bogofilter-wordlist-Debian_7.0.dsc \
	bogofilter-wordlist-xUbuntu_16.10.dsc \
	bogofilter-wordlist-xUbuntu_16.04.dsc

clean:
	/bin/rm -f rules *.dsc debian.tar.gz dummy.tar.gz

edit = sed \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@prefix\@,$(prefix),g'

rules: rules.in Makefile
	$(edit) $@.in > $@; chmod +x $@;
bogofilter-wordlist-Debian_8.0.dsc: bogofilter-wordlist-Debian_8.0.dsc.in Makefile
	$(edit) $@.in > $@
bogofilter-wordlist-Debian_7.0.dsc: bogofilter-wordlist-Debian_7.0.dsc.in Makefile
	$(edit) $@.in > $@
bogofilter-wordlist-xUbuntu_16.10.dsc: bogofilter-wordlist-xUbuntu_16.10.dsc.in Makefile
	$(edit) $@.in > $@
bogofilter-wordlist-xUbuntu_16.04.dsc: bogofilter-wordlist-xUbuntu_16.04.dsc.in Makefile
	$(edit) $@.in > $@
changelog:changelog.in
	$(edit) $@.in > $@

dummy.tar.gz: README
	tar cf - README |gzip > $@
debian.tar.gz: copyright rules control changelog compat
	tar cf - copyright rules control changelog compat|gzip > $@
