@SET_MAKE@

#noinst_LIBRARIES = libfbsql.a libtai64.a

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

DEFS=-I. -I/usr/include/tcl8.5 -I/usr/include/tcl8.4 @auth_inc@ 

libfbsql_LTLIBRARIES = libfbsql.la
libfbsql_la_SOURCES = fbsql.c
libfbsql_la_LIBADD = @auth_libs@
libfbsql_la_LDFLAGS = -version-info @LIBVER@ -release @VERSION@

libtai64_LTLIBRARIES = libtai64.la
libtai64_la_SOURCES = tai64.c
libtai64_la_LDFLAGS = -version-info @LIBVER@ -release @VERSION@

libfbsqldir=@libdir@
libtai64dir=@libdir@
guibindir=@prefix@/bin
guisbindir=@prefix@/sbin
tcldir=@shareddir@/indium/scripts
tcletcdir=@sysconfdir@
tcllibdir=@shareddir@/indium/lib

tcl_SCRIPTS = global.tcl balloon.tcl base64.tcl combobox.tcl fileman.tcl help.tcl \
	hugecombo.tcl hugelist.tcl madHelp.tcl mclistbox.tcl notebook.tcl progressbar.tcl \
	ranges.tcl common.tcl convert_history.tcl convert_profile.tcl gui.tcl \
	keydistribute.tcl listserver.tcl secpanel.dist secpanel.wait dppw.tcl \
	export_profiles.tcl spdistkey secpanel_remoteconf.sh dppw.tcl

tcletc_DATA = default.config default.profile filemanrc termdefs.txt
tcllib_DATA = userhelp.txt

all-local: .gitignore

guisbin_PROGRAMS = pwhelper

guibin_SCRIPTS = indium litesrc supernotepad secpanel FMbrowse tkdiff

noinst_SCRIPTS = indium.spec indium-@VERSION@-rpmlintrc

MOSTLYCLEANFILES = indium litesrc common.tcl fileman.tcl supernotepad secpanel FMbrowse \
				   indium.spec indium-@VERSION@-rpmlintrc tkdiff

pwhelper_SOURCES = pwhelper.c
pwhelper_DEPENDENCIES = pipe_exec.lo

edit = sed \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@indimaildir\@,"/var/indimail",g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@mysql_prefix\@,$(mysql_prefix),g' \
	-e 's,@auth_inc\@,$(auth_inc),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@scriptdir\@,$(shareddir)/indium/scripts,g' \
	-e 's,@sharelibdir\@,$(shareddir)/indium/lib,g' \
	-e 's,@imagedir\@,$(shareddir)/indium/images,g' \
	-e 's,@shareetcdir\@,$(sysconfdir),g' \
	-e 's,@defaultdomain\@,$(defaultdomain),g' \
	-e 's,@version\@,$(VERSION),g' \
	-e 's,@prefix\@,$(prefix),g'

pwhelper.lo: config.h

indium: indium.in common.tcl fileman.tcl
	$(edit) $@.in > $@
common.tcl: common.tcl.in
	$(edit) $@.in > $@
litesrc: litesrc.in common.tcl fileman.tcl
	$(edit) $@.in > $@
supernotepad: supernotepad.tcl
	$(edit) $@.tcl > $@
secpanel: secpanel.tcl.in
	$(edit) $@.tcl.in > $@
fileman.tcl: fileman.tcl.in
	$(edit) $@.in > $@
FMbrowse: FMbrowse.tcl
	$(edit) $@.tcl > $@
tkdiff: tkdiff.tcl
	$(edit) $@.tcl > $@
indium.spec: indium.spec.in
	($(edit) $@.in;cat doc/ChangeLog) > $@
pwhelper.o: pwhelper.c
	$(COMPILE) $(DEFS) -c pwhelper.c
pwhelper$(EXEEXT): $(pwhelper_DEPENDENCIES) $(pwhelper_OBJECTS)
	$(LINK) $@.o pipe_exec.lo $(pwhelper_LDADD)
indium-@VERSION@-rpmlintrc: indium-rpmlintrc.in
	$(edit) indium-rpmlintrc.in > $@
obs.mk: debian/obs.mk.in
	$(edit) debian/$@.in > debian/$@;/bin/rm -f $@;ln -s debian/$@ $@

install-data-hook:
	for f in `ls images`; do \
		/bin/mkdir -p $(DESTDIR)$(shareddir)/indium/images; \
		$(INSTALL_DATA) images/$$f $(DESTDIR)$(shareddir)/indium/images/$$f || exit 1 ;\
	done
.gitignore: $(PROGRAMS) $(guibin_SCRIPTS) $(MOSTLYCLEANFILES) $(noinst_SCRIPTS) gitignore
	(for f in $^; do \
		echo $$f ; \
	done ;cat gitignore) |grep -v gitignore >> .gitignore; sort -u .gitignore -o .gitignore

AUTOMAKE_OPTIONS = foreign no-dependencies
