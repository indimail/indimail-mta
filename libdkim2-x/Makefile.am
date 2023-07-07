@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
DEFS=@DEFS@ -g

include_HEADERS = dkim.h macros.h
COMMONSOURCES= dkimfuncs.cpp dns.cpp dkimbase.cpp dkimsign.cpp dkimverify.cpp
includedir=@prefix@/include/dkim

libdkim2_LTLIBRARIES=libdkim2.la
libdkim2_la_SOURCES=$(COMMONSOURCES) 
libdkim2_la_LIBADD = $(LIB_RESOLV) $(LIB_CRYPTO)
libdkim2_la_LDFLAGS = -version-info @LIBVER@ -pie
libdkim2_la_CXXFLAGS = -fPIE

libdkim2dir=@libdir@
dkimbindir=@prefix@/bin
dkimbin_PROGRAMS = dkim
noinst_DATA = libdkim2.spec libdkim2.changes PKGBUILD

dkim_SOURCES = dkim.cpp
dkim_LDFLAGS = $(LDFLAGS)
dkim_LDADD = libdkim2.la $(LIB_RESOLV) $(LIB_CRYPTO)
dkim_CXXFLAGS = $(CXXFLAGS) -fPIE

pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA = libdkim2.pc

man_MANS=dkim.8

dkimfuncs.$(OBJEXT): dkim.h dkimverify.h dkimsign.h config.h dkimfuncs.cpp
dns.$(OBJEXT): dns.h dns.cpp
dkimbase.$(OBJEXT): dkimbase.cpp dkim.h dkimbase.h
dkimsign.$(OBJEXT): dkimsign.cpp dkim.h dkimsign.h
dkimverify.$(OBJEXT): dns.h dkim.h dkimverify.h config.h dkimverify.cpp

edit = sed \
	-e 's}@PACKAGE\@}$(PACKAGE)}g' \
	-e 's}@PACKAGE_VERSION\@}$(PACKAGE_VERSION)}g' \
	-e 's}@version\@}$(VERSION)}g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's|@email\@|'"`cat conf-email`"'|g' \
	-e 's}@pkgconfigdir\@}$(pkgconfigdir)}g' \
	-e 's}@VERSION\@}$(VERSION)}g' \
	-e 's}@prefix\@}$(prefix)}g'
dkim.8: dkim.9
	cat dkim.9 | sed s}@controldir\@}$(sysconfdir)/indimail/control}g > $@
libdkim2.spec: libdkim2.spec.in catChangeLog doc/ChangeLog conf-version conf-release conf-email
	(cat $@.in;./catChangeLog) | $(edit) > $@
libdkim2.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in catChangeLog Makefile conf-email \
conf-version conf-release doc/ChangeLog libdkim2.changes
	cat $@.in | $(edit) > $@

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

install-data-local:
	$(mkinstalldirs) $(DESTDIR)@libdir@
	$(mkinstalldirs) $(DESTDIR)@prefix@/include/dkim
