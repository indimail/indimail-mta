SUBDIRS = libsrs2 tools
EXTRA_DIST = perl python win32 dist debian LICENSE.BSD LICENSE.GPL-2
man_MANS=srs.1
doc_list = AUTHORS COPYING LICENSE.BSD LICENSE.GPL-2

pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA = libsrs2.pc
noinst_DATA = libsrs2.spec libsrs2.changes PKGBUILD

edit = sed \
	-e 's}@PACKAGE\@}$(PACKAGE)}g' \
	-e 's}@PACKAGE_VERSION\@}$(PACKAGE_VERSION)}g' \
	-e 's}@version\@}$(VERSION)}g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's|@email\@|'"`cat conf-email`"'|g' \
	-e 's}@pkgconfigdir\@}$(pkgconfigdir)}g' \
	-e 's}@VERSION\@}$(VERSION)}g' \
	-e 's}@prefix\@}$(prefix)}g'

libsrs2.spec: libsrs2.spec.in catChangeLog doc/ChangeLog conf-version conf-release conf-email
	(cat $@.in;./catChangeLog) | $(edit) > $@
libsrs2.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in catChangeLog Makefile conf-email \
conf-version conf-release doc/ChangeLog libsrs2.changes
	cat $@.in | $(edit) > $@

dist-hook:
	@for file in $(EXTRA_DIST); do \
		find $(distdir)/$$file -name .svn | xargs rm -rf; \
	done
	rm -f $(distdir)/perl/Makefile.old
