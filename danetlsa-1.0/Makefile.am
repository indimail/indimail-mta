@SET_MAKE@
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

INSTALL_PROGRAM = ${INSTALL} -c
install_sh_PROGRAM = $(install_sh) -c
INSTALL_SCRIPT = ${INSTALL}
install_sh_SCRIPT = $(install_sh) -c

DEFS=@DEFS@ -fno-strict-aliasing

noinst_FILES=libdanetlsa.la libdanetlsa.a

danetlsalibexec_PROGRAMS = tlsarr

tlsarr_SOURCES = tlsarr.c
tlsarr_LDADD = libdanetlsa.la $(LIBS)

libdanetlsa_LTLIBRARIES=libdanetlsa.la
libdanetlsa_la_SOURCES=danetlsa.c tlsa_variables.c
libdanetlsa_la_LDFLAGS=-version-info @LIBVER@ -release @VERSION@
libdanetlsa_la_LIBADD=$(LIB_SSL) $(LIBS)
libdanetlsa_la_CPPFLAGS=$(DEFS)
libdanetlsa_la_CFLAGS=-fPIC

include_HEADERS=query-getdns.h
includedir=@prefix@/include/indimail

doc_list= COPYING README.danetlsa CREDITS.danetlsa

libdanetlsadir=@libdir@
danetlsalibexecdir=@libexecdir@

#LIBS_GETDNS2  = -lssl -lcrypto -lgetdns_ext_event -lgetdns -levent_core -lunbound

MOSTLYCLEANFILES=libdanetlsa.la
EXTRA_DIST=COPYING NEWS README CREDITS ChangeLog INSTALL.danetlsa install-sh missing mkinstalldirs

danetlsa.$(OBJEXT): danetlsa.c query-getdns.h
#tlsarr$(EXEEXT): tlsarr.$(OBJEXT) libdanetlsa.la

install-data-hook: 
	$(mkinstalldirs) $(DESTDIR)@prefix@/include/indimail || exit 1
	$(INSTALL_DATA) query-getdns.h $(DESTDIR)@prefix@/include/indimail || exit 1
