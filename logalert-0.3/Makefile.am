@SET_MAKE@
ACLOCAL_AMFLAGS = -I m4
COFLAGS=-z+05:30
DEFS = @DEFS@

INSTALL_PROGRAM = ${INSTALL}          -c
install_sh_PROGRAM = $(install_sh)    -c
INSTALL_SCRIPT = ${INSTALL}           -c
install_sh_SCRIPT = $(install_sh)     -c
INSTALL_STRIP_PROGRAM = $(install_sh) -c $(STRIP_OPT)

#DIST_COMMON=ChangeLog COPYING AUTHORS
SUBDIRS = src

bin_PROGRAMS=logclient incrmesg
sbin_PROGRAMS=logsrv rpclog

man_MANS = doc/logalert.8 logsrv.8 rpclog.8 incrmesg.1 logclient.1

MOSTLYCLEANFILES=logalert.spec logalert-@VERSION@-rpmlintrc \
				   logalert_perm_list obs.mk

noinst_SCRIPTS = logalert.spec logalert-@VERSION@-rpmlintrc

DISTCLEANFILES = *~

SYSCONFDIR = @sysconfdir@
sysconf_DATA = logalert_perm_list

edit = sed \
	-e 's,@indimaildir\@,$(prefix),g' \
	-e 's,@version\@,$(VERSION),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@DESTDIR\@,$(DESTDIR),g' \
	-e 's,@HOST\@,$(host),g' \
	-e 's,@DATE\@,$(DATE),g' \
	-e 's,@prefix\@,$(prefix),g'

logclient_SOURCES=logclient.c common.c
logclient_LDADD = @LIB_INDIMAIL@

logsrv_SOURCES=logsrv.c common.c
logsrv_LDADD = @LIB_INDIMAIL@

incrmesg_SOURCES=incrmesg.c common.c
incrmesg_LDADD = @LIB_INDIMAIL@

logalert-@VERSION@-rpmlintrc: logalert-rpmlintrc.in
	$(edit) logalert-rpmlintrc.in > $@
logalert.spec: logalert.spec.in
	($(edit) $@.in;cat ChangeLog) > $@
obs.mk: debian/obs.mk.in
	$(edit) debian/$@.in > debian/$@;/bin/rm -f $@;ln -s debian/$@ $@
logalert_perm_list: logalert_perm_list.in
	$(edit) $@.in > $@
install-data-hook:
	$(mkinstalldirs)         $(DESTDIR)$(SYSCONFDIR)
	$(INSTALL_DATA) logalert_perm_list $(DESTDIR)$(SYSCONFDIR)/logalert_perm_list  || exit 1
