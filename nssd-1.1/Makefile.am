@SET_MAKE@
AUTOMAKE_OPTIONS = foreign no-dependencies

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

INSTALL_PROGRAM = ${INSTALL} -c
install_sh_PROGRAM = $(install_sh) -c
INSTALL_SCRIPT = ${INSTALL}
install_sh_SCRIPT = $(install_sh) -c
INSTALL_STRIP_PROGRAM = $(install_sh) -c $(STRIP_OPT)

COMMONSOURCES= pwd.c spwd.c grp.c sock.c misc.c util.c

libnss_nssddir=@libdir@
libnss_nssd_LTLIBRARIES = libnss_nssd.la
libnss_nssd_la_SOURCES=$(COMMONSOURCES) 
libnss_nssd_la_LIBADD =
libnss_nssd_la_DEPENDENCIES = common.h config.h nsvs.h
libnss_nssd_la_LDFLAGS = -module -version-info @LIBVER@:0:0 -export-symbols $(srcdir)/@OS@.sym

nssdsbindir=@prefix@/sbin
nssdsbin_PROGRAMS = nssd
nssd_SOURCES = nssd.c nssd_misc.c util.c config.c
#nssd_CFLAGS  = -Wall @MYSQL_CFLAGS@ @PTHREAD_CFLAGS@
nssd_CFLAGS  = -Wall @auth_inc@ @PTHREAD_CFLAGS@
nssd_LDFLAGS = @auth_libs@ @PTHREAD_CFLAGS@ @PTHREAD_LIBS@
#nssd_LDFLAGS = @MYSQL_LIBS@ @PTHREAD_CFLAGS@ @PTHREAD_LIBS@
nssd_LDADD = 
nssd_DEPENDENCIES = config.h

nssdlibexecdir=@libexecdir@
nssdlibexec_PROGRAMS = check_getpw
man_MANS=nssd.8 nssd.conf.5

noinst_HEADERS = nssd.h

MOSTLYCLEANFILES=nssd.8 nssd.conf.5

pwd.$(OBJEXT): nsvs.h common.h config.h
grp.$(OBJEXT): nsvs.h common.h config.h
misc.$(OBJEXT): nsvs.h common.h config.h
sock.$(OBJEXT): nsvs.h common.h config.h
spwd.$(OBJEXT): nsvs.h common.h config.h
util.$(OBJEXT): common.h config.h

nssd-nssd.$(OBJEXT): nssd.h common.h config.h
nssd-config.$(OBJEXT): nssd.h common.h config.h
nssd-nssd_misc.$(OBJEXT): nssd.h common.h config.h
nssd-util.$(OBJEXT): common.h config.h

nssd.8: nssd.9
	cat $^ | sed s}INDIMAIL}$(prefix)}g > $@
nssd.conf.5: nssd.conf.9
	cat $^ | sed s}INDIMAIL}$(prefix)}g > $@

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

#install -data-hook:
#install-libnss_nssdLTLIBRARIES: $(libnss_nssd_LTLIBRARIES)
#	@$(NORMAL_INSTALL)
#	@if [ " $(ARCH_LIB)" = " " ] ; then \
#		if [ -d /usr/lib64 ] ; then \
#			$(LIBTOOL) --mode=install $(INSTALL) -c -s libnss_nssd.la $(DESTDIR)/usr/lib64/libnss_nssd.la; \
#		else \
#			$(LIBTOOL) --mode=install $(INSTALL) -c -s libnss_nssd.la $(DESTDIR)/usr/lib/libnss_nssd.la; \
#		fi; \
#	fi
