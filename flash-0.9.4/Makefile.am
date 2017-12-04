@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

COMMONSOURCES= debug.c event.c exec.c menu.c misc.c module.c parse.c parseline.c rc.c set.c sha.c \
	mystring.c tree.c nc_about.c nc_init.c nc_job.c nc_lock.c nc_menus.c nc_misc.c nc_module.c variables.c

libflash_LTLIBRARIES = libflash.la
libflash_la_SOURCES=$(COMMONSOURCES) 
libflash_la_LIBADD = @LIB_CURSES@ @LIB_DLOPEN@
libflash_la_LDFLAGS = -version-info @LIBVER@
libflashdir=@libdir@

INSTALL_PROGRAM = ${INSTALL} -c
install_sh_PROGRAM = $(install_sh) -c
INSTALL_SCRIPT = ${INSTALL}
install_sh_SCRIPT = $(install_sh) -c
INSTALL_STRIP_PROGRAM = $(install_sh) -c $(STRIP_OPT)

flashbindir=@prefix@/bin
flashbin_PROGRAMS = flash crypt
modules_list = alarms background countdown

flash_SOURCES = main.c
flash_LDADD = @auth_libs@ @LIB_CURSES@ @LIB_DLOPEN@
flash_DEPENDENCIES = libflash.la

crypt_SOURCES = crypt.c
crypt_LDADD = @LIB_CRYPT@

MOSTLYCLEANFILES=$(modules_list) system.menu system.rc
all-local: system.menu system.rc modules

modules: $(modules_list)
alarms: $(alarms_DEPENDENCIES) alarms.o
	$(COMPILE) $(MODFLAGS) -o $@ $@.o
background: $(background_DEPENDENCIES) background.o
	$(COMPILE) $(MODFLAGS) -o $@ $@.o 
countdown: $(countdown_DEPENDENCIES) countdown.o
	$(COMPILE) $(MODFLAGS) -o $@ $@.o

countdown: countdown.c ncr_scr.h mystring.h misc.h config.h
background: background.c ncr_scr.h mystring.h config.h
alarms: alarms.c ncr_scr.h mystring.h menu.h misc.h exec.h config.h
debug.o: tree.h set.h menu.h misc.h config.h
event.o: event.h config.h
exec.o: exec.h misc.h screen.h module.h menu.h set.h rc.h mystring.h parseline.h config.h
main.o: parse.h mystring.h screen.h event.h exec.h set.h rc.h debug.h misc.h config.h
menu.o: menu.h parse.h misc.h tree.h mystring.h config.h
misc.o: exec.h config.h
module.o: set.h rc.h exec.h parse.h misc.h mystring.h parseline.h module.h config.h
mystring.o: mystring.h misc.h config.h
nc_about.o: ncr_scr.h exec.h menu.h misc.h parse.h screen.h set.h config.h
nc_init.o: ncr_scr.h mystring.h screen.h parse.h misc.h exec.h menu.h set.h config.h
nc_job.o: exec.h menu.h screen.h ncr_scr.h config.h
nc_lock.o: ncr_scr.h screen.h exec.h misc.h set.h config.h
nc_menus.o: ncr_scr.h mystring.h screen.h parse.h misc.h exec.h menu.h set.h config.h
nc_misc.o: ncr_scr.h screen.h misc.h exec.h set.h config.h
nc_module.o: ncr_scr.h screen.h mystring.h misc.h exec.h set.h module.h config.h
parse.o: parseline.h mystring.h parse.h module.h menu.h misc.h exec.h rc.h config.h
parseline.o: set.h parseline.h misc.h config.h
rc.o: set.h rc.h exec.h parse.h misc.h mystring.h parseline.h config.h
set.o: set.h tree.h misc.h config.h
tree.o: tree.h misc.h config.h
alarms.o: alarms.c config.h
background.o: background.c config.h
countdown.o: countdown.c config.h

edit = sed \
	-e 's,@indimaildir\@,$(indimaildir),g' \
	-e 's,@prefix\@,$(prefix),g'

system.rc: system.rc.in
	$(edit) $@.in > $@
system.menu: system.menu.in
	$(edit) $@.in > $@

install-data-local: modules
	$(mkinstalldirs)  $(DESTDIR)@mandir@/man1
	$(mkinstalldirs)  $(DESTDIR)@flashmoduledir@/modules
	$(mkinstalldirs)  $(DESTDIR)@sysconfdir@
	$(mkinstalldirs)  $(DESTDIR)@prefix@/lib
	for f in "system.menu system.module system.rc system.flashlogin"; do \
		$(INSTALL_DATA) $$f $(DESTDIR)@sysconfdir@ ; \
	done; \
	for f in $(modules_list); do \
		/bin/mkdir -p $(DESTDIR)@flashmoduledir@/modules; \
		$(LIBTOOL) --mode=install $(INSTALL_DATA) $$f $(DESTDIR)@flashmoduledir@/modules/$$f; \
	done; \
	$(INSTALL_DATA) flash.1 $(DESTDIR)@mandir@/man1

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

AUTOMAKE_OPTIONS = foreign no-dependencies
