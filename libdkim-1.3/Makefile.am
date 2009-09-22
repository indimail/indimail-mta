@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

include_HEADERS = dkim.h macros.h
COMMONSOURCES= dkimfuncs.cpp dns.cpp dkimbase.cpp dkimsign.cpp dkimverify.cpp

libdkim_LIBRARIES=libdkim.a
libdkim_a_SOURCES=$(COMMONSOURCES) 
libdkim_a_LIBADD =
libdkim_a_CFLAGS = -fPIC

libdkimdir=@libdir@
dkimbindir=@prefix@/bin
dkimbin_PROGRAMS = dkim

dkim_SOURCES = dkim.c
dkim_LDADD = libdkim.a $(LIBS)

dkim: dkim.o libdkim.a
	$(CXX) -o $@ $@.$(OBJEXT) -L. -ldkim $(LIB_RESOLV) $(LIB_CRYPTO)
dkim.$(OBJEXT): dkim.h dkim.c dns.h config.h
	$(CXX) $(DEFS) -c dkim.c
dkimfuncs.$(OBJEXT): dkim.h dkimverify.h dkimsign.h config.h dkimfuncs.cpp
dns.$(OBJEXT): dns.h dns.cpp
dkimbase.$(OBJEXT): dkimbase.cpp dkim.h dkimbase.h
dkimsign.$(OBJEXT): dkimsign.cpp dkim.h dkimsign.h
dkimverify.$(OBJEXT): dns.h dkim.h dkimverify.h config.h dkimverify.cpp

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
