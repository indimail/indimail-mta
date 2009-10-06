@SET_MAKE@
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

INSTALL_PROGRAM = ${INSTALL} -c
install_sh_PROGRAM = $(install_sh) -c
INSTALL_SCRIPT = ${INSTALL}
install_sh_SCRIPT = $(install_sh) -c

libpamsbindir=@prefix@/sbin
libpam_multidir=$(moduledir)
libpam_multi_LTLIBRARIES=pam-multi.la
noinst_FILES=pam-multi.la pam-multi.a

pam_multi_la_SOURCES=pam-multi.c md5_crypt.c sha256_crypt.c sha512_crypt.c
pam_multi_la_LDFLAGS=-module -avoid-version
pam_multi_la_CPPFLAGS=$(DEFS) @auth_inc@

libpamsbin_PROGRAMS = pam-checkpwd

all-local: authenticate.so pam-multi.spec

authenticate.so: authenticate.o
	$(COMPILE) $(MODFLAGS) -o $@ authenticate.o -Xlinker -R -Xlinker @indimaildir@/lib \
		-L@indimaildir@/lib -lindimail -leps
authenticate.o: authenticate.c
	$(COMPILE) $(DEFS) @auth_inc@ -c $^

pam-checkpwd: pam-checkpwd.o pam-support.o
	$(LINK) $@.o pam-support.o -Xlinker -R -Xlinker @indimaildir@/lib -L@indimaildir@/lib \
		-lindimail -leps $(LIB_CRYPT) $(LIB_SSL) @LIB_PAM@ @LIB_DL@
pam-checkpwd.o: pam-checkpwd.c
	$(COMPILE) $(DEFS) @auth_inc@ -c $^

MOSTLYCLEANFILES=pam-multi.la  authenticate.so pam-checkpwd
EXTRA_DIST=COPYING NEWS README CREDITS ChangeLog INSTALL pam-multi.spec.in install-sh missing mkinstalldirs pam-multi.spec

install-data-local: authenticate.so
	$(mkinstalldirs)  $(DESTDIR)$(moduledir)
	$(mkinstalldirs)  $(DESTDIR)$(libdir)
	$(mkinstalldirs)  $(DESTDIR)@prefix@/man/man8
	$(INSTALL_PROGRAM) -c authenticate.so $(DESTDIR)$(libdir)/authenticate.so
	$(INSTALL_DATA) -c pam-multi.8 $(DESTDIR)@prefix@/man/man8
	$(INSTALL_DATA) -c pam-checkpwd.8 $(DESTDIR)@prefix@/man/man8

