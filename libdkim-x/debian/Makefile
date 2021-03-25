all: debian.tar.gz
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

libdkim.install: libdkim.install.in
	$(edit) $@.in > $@

libdkim-dev.install: libdkim-dev.install.in
	$(edit) $@.in > $@

clean:
	/bin/rm -f changelog libdkim.dsc libdkim.install libdkim-dev.install debian.tar.gz

debian.tar.gz: changelog compat control copyright \
libdkim-dev.docs libdkim.docs \
libdkim.dsc README.Debian rules libdkim.install \
libdkim-dev.install libdkim-dev.docs libdkim.docs 
	tar cf - changelog compat control copyright \
		libdkim-dev.docs libdkim.docs libdkim.dsc \
		README.Debian rules libdkim.install \
		libdkim-dev.install libdkim-dev.docs \
		libdkim.docs | gzip -c >$@
