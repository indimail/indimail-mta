all: debian.tar.gz libsrs2.dsc
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
	../catChangeLog --debian --name=libsrs2 --state=unstable \
		--urgency=low ../doc/ChangeLog > $@
control: control.in ../conf-email
	$(edit) $@.in > $@
copyright: copyright.in ../conf-email
	$(edit) $@.in > $@

libsrs2.dsc: libsrs2.dsc.in ../conf-release ../conf-version
	$(edit) $@.in > $@

libsrs2.install: libsrs2.install.in
	$(edit) $@.in > $@

libsrs2-dev.install: libsrs2-dev.install.in
	$(edit) $@.in > $@

clean:
	/bin/rm -f changelog libsrs2.dsc libsrs2.install libsrs2-dev.install debian.tar.gz

debian.tar.gz: changelog compat control copyright libsrs2-dev.docs \
libsrs2.docs libsrs2.dsc rules libsrs2.install libsrs2-dev.install \
source/format Makefile
	cd .. && tar cf - debian/changelog debian/compat debian/control debian/copyright \
	debian/libsrs2-dev.docs debian/libsrs2.docs debian/libsrs2.dsc \
	debian/rules debian/libsrs2.install debian/libsrs2-dev.install \
	debian/source | gzip -c >debian/$@
