indimaildir=/var/indimail
prefix=/usr
cgibindir=/var/www/cgi-bin
version=7.2.2
release=1
sysconfdir=/etc/indimail
#

all: rules debian.tar.gz dummy.tar.gz ezmlm-idx.dsc _service

clean:
	/bin/rm -f rules postinst postrm *.dsc debian.tar.gz dummy.tar.gz _service

edit = sed \
	-e 's,@indimaildir\@,$(indimaildir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@cgibindir\@,$(cgibindir),g' \
	-e 's,@version\@,$(version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@prefix\@,$(prefix),g'

ezmlm-idx.dsc: ezmlm-idx.dsc.in Makefile
	$(edit) $@.in > $@
_service: _service.in
	$(edit) $@.in > $@
dummy.tar.gz: README
	tar cf - README |gzip > $@
debian.tar.gz: copyright rules control changelog compat
	tar cf - copyright rules control changelog compat |gzip > $@
