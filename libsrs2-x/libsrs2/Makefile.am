@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

include_HEADERS = srs2.h
noinst_HEADERS = win32.h
includedir=@prefix@/include/indimail

lib_LTLIBRARIES = libsrs2.la
libsrs2_la_SOURCES = srs2.c sha1.c
libsrs2_la_LDFLAGS = -version-info @LIBVER@
libsrs2_la_CFLAGS = -fPIC

srs2.$(OBJEXT): srs2.h win32.h

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

install-data-local:
	$(mkinstalldirs)  $(DESTDIR)@libdir@
	$(mkinstalldirs)  $(DESTDIR)@prefix@/include/indimail
