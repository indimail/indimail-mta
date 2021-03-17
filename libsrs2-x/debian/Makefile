all: debian.tar.gz
version=$(shell cat ../conf-version)
arch=$(shell uname -m)

edit = sed \
	-e 's,@version\@,$(version),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e "s,@release\@,"`cat ../conf-release`",g" \
	-e 's,@prefix\@,$(prefix),g'

changelog: changelog.in ../conf-release ../conf-version
	(cat $@.in; echo ""; \
	echo " -- libsrs2 <libsrs2@indimail.org>  `date +'%a, %d %b %Y %H:%M:%S %z'`" ) \
	| $(edit) > $@

libsrs2.dsc: libsrs2.dsc.in ../conf-release ../conf-version
	$(edit) $@.in > $@

libsrs2.install: libsrs2.install.in
	$(edit) $@.in > $@

libsrs2-dev.install: libsrs2-dev.install.in
	$(edit) $@.in > $@

clean:
	/bin/rm -f changelog libsrs2.dsc libsrs2.install libsrs2-dev.install debian.tar.gz

debian.tar.gz: changelog compat control copyright \
libsrs2-dev.docs libsrs2.docs \
libsrs2.dsc rules libsrs2.install \
libsrs2-dev.install libsrs2-dev.docs libsrs2.docs 
	tar cf - changelog compat control copyright \
		libsrs2-dev.docs libsrs2.docs libsrs2.dsc \
		rules libsrs2.install \
		libsrs2-dev.install libsrs2-dev.docs \
		libsrs2.docs | gzip -c >$@
