@SET_MAKE@
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

INSTALL_PROGRAM = ${INSTALL} -c
install_sh_PROGRAM = $(install_sh) -c
INSTALL_SCRIPT = ${INSTALL}
install_sh_SCRIPT = $(install_sh) -c

DEFS=@DEFS@ -fno-strict-aliasing

pamsbindir=@prefix@/sbin
pam_multidir=$(moduledir)
noinst_FILES=pam-multi.la pam-multi.a

pam_multi_LTLIBRARIES=pam-multi.la
pam_multi_la_SOURCES=pam-multi.c md5_crypt.c sha256_crypt.c sha512_crypt.c
pam_multi_la_LDFLAGS=-module -avoid-version -version-info @LIBVER@:0:0
pam_multi_la_LIBADD=@LIB_CRYPT@ @LIB_SSL@ @auth_libs@ @LIB_PAM@
pam_multi_la_CPPFLAGS=$(DEFS) @auth_inc@

pamsbin_PROGRAMS = pam-checkpwd$(EXEEXT)

man_MANS=pam-multi.8

all-local: pam-multi.spec

pam-checkpwd$(EXEEXT): pam-checkpwd.o pam-support.o 
	$(LINK) $@.o pam-support.o @LIB_PAM@
pam-checkpwd.o: pam-checkpwd.c pam-support.h config.h
	$(COMPILE) $(DEFS) @auth_inc@ -c $^
pam-support.o: config.h pam-support.c

MOSTLYCLEANFILES=pam-multi.la pam-checkpwd pam-support.h.gch config.h.gch pam-multi.8
EXTRA_DIST=COPYING NEWS README CREDITS ChangeLog INSTALL pam-multi.spec.in install-sh missing mkinstalldirs pam-multi.spec
DISTCLEANFILES=pam-multi.settings

pam-multi.8: pam-multi.9
	cat $^ | sed s}INDIMAIL}$(prefix)}g > $@

install-data-local:  pam-multi.8 pam-checkpwd.8
	$(mkinstalldirs)  $(DESTDIR)$(moduledir)
	$(mkinstalldirs)  $(DESTDIR)$(libdir)
	$(mkinstalldirs)  $(DESTDIR)$(pam_confdir)
	$(mkinstalldirs)  $(DESTDIR)@prefix@/man/man8
	$(INSTALL_DATA) -c `basename $(pam_confdir)`/pam-multi $(DESTDIR)$(pam_confdir)
	$(INSTALL_DATA) -c `basename $(pam_confdir)`/pop3 $(DESTDIR)$(pam_confdir)
	$(INSTALL_DATA) -c `basename $(pam_confdir)`/imap $(DESTDIR)$(pam_confdir)
	$(INSTALL_DATA) -c pam-multi.8 $(DESTDIR)@prefix@/man/man8
	$(INSTALL_DATA) -c pam-checkpwd.8 $(DESTDIR)@prefix@/man/man8

