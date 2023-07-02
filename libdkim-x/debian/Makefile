all: debian.tar.gz libdkim.dsc libdkim-Debian_12.dsc
version=$(shell cat ../conf-version)
arch=$(shell uname -m)
email=$(shell cat ../conf-email)

edit = sed \
	-e 's,@version\@,$(version),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e "s,@release\@,"`cat ../conf-release`",g" \
	-e 's,@email\@,$(email),g' \
	-e 's,@prefix\@,$(prefix),g'

changelog: ../doc/ChangeLog Makefile ../conf-version ../conf-release ../conf-email
	../catChangeLog --debian --name=libdkim --state=unstable \
		--urgency=low ../doc/ChangeLog > $@
control: control.in ../conf-email
	$(edit) $@.in > $@
copyright: copyright.in ../conf-email
	$(edit) $@.in > $@

libdkim.dsc: libdkim.dsc.in ../conf-release ../conf-version
	$(edit) $@.in > $@
libdkim-Debian_12.dsc: libdkim-Debian_12.dsc.in ../conf-release ../conf-version
	$(edit) $@.in > $@

libdkim.install: libdkim.install.in
	$(edit) $@.in > $@

libdkim-dev.install: libdkim-dev.install.in
	$(edit) $@.in > $@

clean:
	/bin/rm -f changelog libdkim.dsc libdkim.install libdkim-dev.install debian.tar.gz

debian.tar.gz: changelog compat control copyright libdkim-dev.docs \
libdkim.docs README.Debian rules libdkim.install libdkim-dev.install \
source/format
	cd .. && tar cf - debian/changelog debian/compat debian/control \
	debian/copyright debian/libdkim-dev.docs debian/libdkim.docs \
	debian/README.Debian debian/rules debian/libdkim.install \
	debian/libdkim-dev.install debian/source | gzip -c >debian/$@
