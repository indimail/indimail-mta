@SET_MAKE@
ACLOCAL_AMFLAGS=-I m4

EXTRA_DIST=BUGS FAQ TRANSLATORS README.hooks html lang images contrib

htmllibdir=@htmllibdir@
htmldir=@htmldir@
imagedir=@imagedir@

#
# Install qmailadmin setuid to @indimailuser@
#
INSTALL_PROGRAM = ${INSTALL}          -c
INSTALL_STRIP_PROGRAM=${INSTALL_PROGRAM} -s

bindir=@cgibindir@
bin_PROGRAMS=@CGIBINTARGETS@
EXTRA_PROGRAMS=qmailadmin

SYSCONFDIR = @sysconfdir@
sysconf_DATA = iwebadmin_perm_list

qmailadmin_DEPENDENCIES=$(COMMONDEPENDENCIES)

COMMONSOURCES=qmailadmin.c alias.c autorespond.c forward.c mailinglist.c \
	      user.c util.c printh.c auth.c template.c \
	      command.c show.c cgi.c limits.c dotqmail.c \
	      qmailadmin.h qmailadminx.h

qmailadmin_SOURCES=$(COMMONSOURCES) 
qmailadmin_LDADD=@auth_libs@

noinst_SCRIPTS = iwebadmin.spec iwebadmin-@VERSION@-rpmlintrc

MOSTLYCLEANFILES = iwebadmin.spec iwebadmin-@VERSION@-rpmlintrc \
				   iwebadmin_perm_list

edit = sed \
	-e 's,@indimaildir\@,$(indimaildir),g' \
	-e 's,@mysql_prefix\@,$(mysql_prefix),g' \
	-e 's,@htmldir\@,$(htmldir),g' \
	-e 's,@htmllibdir\@,$(htmllibdir),g' \
	-e 's,@sys_libdir\@,$(sys_libdir),g' \
	-e 's,@cgibindir\@,$(cgibindir),g' \
	-e 's,@imagedir\@,$(imagedir),g' \
	-e 's,@version\@,$(VERSION),g' \
	-e 's,@prefix\@,$(prefix),g'

qmailadmin.o: config.h qmailadmin.h 
alias.o: config.h qmailadmin.h qmailadminx.h 
autorespond.o: config.h qmailadmin.h qmailadminx.h 
forward.o: config.h qmailadmin.h qmailadminx.h 
mailinglist.o: config.h qmailadmin.h qmailadminx.h 
user.o: config.h qmailadmin.h qmailadminx.h 
util.o: config.h qmailadmin.h qmailadminx.h 
printh.o: printh.h
auth.o: config.h qmailadmin.h qmailadminx.h 
template.o: config.h qmailadmin.h qmailadminx.h 
command.o: qmailadmin.h qmailadminx.h 
show.o: config.h qmailadmin.h qmailadminx.h 
cgi.o: config.h qmailadmin.h qmailadminx.h 
limits.o: config.h qmailadmin.h qmailadminx.h 
dotqmail.o: config.h qmailadmin.h qmailadminx.h 

iwebadmin.spec: iwebadmin.spec.in
	($(edit) $@.in;cat doc/ChangeLog) > $@
iwebadmin-@VERSION@-rpmlintrc: iwebadmin-rpmlintrc.in
	$(edit) iwebadmin-rpmlintrc.in > $@
obs.mk: debian/obs.mk.in
	$(edit) debian/$@.in > debian/$@;/bin/rm -f $@;ln -s debian/$@ $@
iwebadmin_perm_list: iwebadmin_perm_list.in
	$(edit) $@.in > $@

DEFS=-I. @auth_incs@

# Maintainer stuff

dist-hook:

# use the [^C]* pattern to skip the CVS directory
install-data-local:
	$(mkinstalldirs)         $(DESTDIR)@htmllibdir@
	rm -rf                   $(DESTDIR)@htmllibdir@/html
	$(mkinstalldirs)         $(DESTDIR)@htmllibdir@/html 
	cp $(srcdir)/html/*.html $(DESTDIR)@htmllibdir@/html
	cp $(srcdir)/html/colortable $(DESTDIR)@htmllibdir@/html
	chmod 755                $(DESTDIR)@htmllibdir@/html
	chmod 644                $(DESTDIR)@htmllibdir@/html/*
	rm -rf                   $(DESTDIR)@htmllibdir@/lang
	$(mkinstalldirs)         $(DESTDIR)@htmllibdir@/lang 
	cp $(srcdir)/lang/??     $(DESTDIR)@htmllibdir@/lang
	cp $(srcdir)/lang/??-??  $(DESTDIR)@htmllibdir@/lang
	chmod 755                $(DESTDIR)@htmllibdir@/lang
	chmod 644                $(DESTDIR)@htmllibdir@/lang/*
	$(mkinstalldirs)         $(DESTDIR)@imagedir@
	cp $(srcdir)/images/*.png $(DESTDIR)@imagedir@

	if test `@idcommand@` = "0" ; then \
		echo "chown root $(DESTDIR)$(bindir)/qmailadmin"; \
		chown root $(DESTDIR)$(bindir)/qmailadmin; \
		echo "chmod 4555 $(DESTDIR)$(bindir)/qmailadmin"; \
		chmod 4555 $(DESTDIR)$(bindir)/qmailadmin; \
	fi
install-data-hook:
	$(mkinstalldirs)         $(DESTDIR)@indimaildir@/etc
	$(INSTALL_DATA) iwebadmin_perm_list $(DESTDIR)@indimaildir@/etc/iwebadmin_perm_list  || exit 1
