all: debian.tar.gz libdkim2.dsc libdkim2-Debian_12.dsc
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
	../catChangeLog --debian --name=libdkim2 --state=unstable \
		--urgency=low ../doc/ChangeLog > $@
control: control.in ../conf-email
	$(edit) $@.in > $@
copyright: copyright.in ../conf-email
	$(edit) $@.in > $@

libdkim2.dsc: libdkim2.dsc.in ../conf-release ../conf-version
	$(edit) $@.in > $@
libdkim2-Debian_12.dsc: libdkim2-Debian_12.dsc.in ../conf-release ../conf-version
	$(edit) $@.in > $@

libdkim2.install: libdkim2.install.in
	$(edit) $@.in > $@

libdkim2-dev.install: libdkim2-dev.install.in
	$(edit) $@.in > $@

clean:
	/bin/rm -f changelog libdkim2.dsc libdkim2.install libdkim2-dev.install debian.tar.gz

debian.tar.gz: changelog compat control copyright libdkim2-dev.docs \
libdkim2.docs README.Debian rules libdkim2.install libdkim2-dev.install \
source/format
	cd .. && tar cf - debian/changelog debian/compat debian/control \
	debian/copyright debian/libdkim2-dev.docs debian/libdkim2.docs \
	debian/README.Debian debian/rules debian/libdkim2.install \
	debian/libdkim2-dev.install debian/source | gzip -c >debian/$@
