@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
DEFS=@DEFS@ -g

include_HEADERS = dkim.h macros.h
COMMONSOURCES= dkimfuncs.cpp dns.cpp dkimbase.cpp dkimsign.cpp dkimverify.cpp
includedir=@prefix@/include/indimail

libdkim_LTLIBRARIES=libdkim.la
libdkim_la_SOURCES=$(COMMONSOURCES) 
libdkim_la_LIBADD = -lresolv
libdkim_la_LDFLAGS = -version-info @LIBVER@ -release @VERSION@
libdkim_la_CFLAGS = -fPIC

libdkimdir=@libdir@
dkimbindir=@prefix@/bin
dkimbin_PROGRAMS = dkim

dkim_SOURCES = dkim.c
dkim_LDADD = libdkim.la $(LIBS)

man_MANS=dkim.8

dkim$(EXEEXT): $(dkim_OBJECTS) dkim.$(OBJEXT) libdkim.la
	$(CXXLINK) $(dkim_OBJECTS) $(dkim_LDADD) $(LIB_RESOLV) $(LIB_CRYPTO)
dkim.$(OBJEXT): dkim.h dkim.c dns.h config.h
	$(LIBTOOL) --mode=compile --tag=CXX g++ $(DEFS) -fPIC -DPIC -c dkim.c
dkimfuncs.$(OBJEXT): dkim.h dkimverify.h dkimsign.h config.h dkimfuncs.cpp
dns.$(OBJEXT): dns.h dns.cpp
dkimbase.$(OBJEXT): dkimbase.cpp dkim.h dkimbase.h
dkimsign.$(OBJEXT): dkimsign.cpp dkim.h dkimsign.h
dkimverify.$(OBJEXT): dns.h dkim.h dkimverify.h config.h dkimverify.cpp

dkim.8: dkim.9
	cat $^ | sed s}INDIMAIL}$(prefix)}g > $@

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

install-data-local:
	$(mkinstalldirs)  $(DESTDIR)@libdir@
	$(mkinstalldirs)  $(DESTDIR)@prefix@/include/indimail
