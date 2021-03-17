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
	echo " -- libdkim <libdkim@indimail.org>  `date +'%a, %d %b %Y %H:%M:%S %z'`" ) \
	| $(edit) > $@

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
