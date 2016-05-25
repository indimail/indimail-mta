@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

INSTALL_PROGRAM = ${INSTALL}       -o root -g qmail -m 555
install_sh_PROGRAM = $(install_sh) -c -o root -g qmail -m 555
INSTALL_SCRIPT = ${INSTALL}        -c -o root -g qmail -m 555
install_sh_SCRIPT = $(install_sh)  -c -o root -g qmail -m 555
INSTALL_STRIP_PROGRAM = $(install_sh) -c -o root -g qmail -m 555 -s

doc_list= README.cdb

include_HEADERS = cdb.h cdbmake.h uint32.h
COMMONSOURCES= cdb_hash.c cdbmake_add.c cdbmake_hash.c cdbmake_pack.c cdb_seek.c cdb_unpack.c
INC = $(include_HEADERS)

libcdb_LTLIBRARIES = libcdb.la
libcdb_la_SOURCES=$(COMMONSOURCES) 
libcdb_la_LIBADD =
libcdb_la_LDFLAGS = -version-info @LIBVER@ -release @VERSION@
libcdbdir=@libdir@

MOSTLYCLEANFILES= uint32.h load compile

cdb_hash.lo: cdb.h uint32.h
cdb_seek.lo: cdb.h uint32.h config.h
cdb_unpack.lo: cdb.h uint32.h
cdbmake_add.lo: cdbmake.h uint32.h
cdbmake_hash.lo: cdbmake.h uint32.h
cdbmake_pack.lo: cdbmake.h uint32.h

uint32.h: tryulong32.c compile load uint32.h1 uint32.h2
	( ( ./compile tryulong32.c && ./load tryulong32 && \
	./tryulong32 ) >/dev/null 2>&1 \
	&& cat uint32.h2 || cat uint32.h1 ) > uint32.h
	rm -f tryulong32.$(OBJEXT) tryulong32

compile: warn-auto.sh conf-cc
	( cat warn-auto.sh; \
	echo exec "`head -1 conf-cc`" '-c $${1+"$$@"}' \
	) > compile
	chmod 755 compile

load: warn-auto.sh conf-ld
	( cat warn-auto.sh; \
	echo 'main="$$1"; shift'; \
	echo exec "`head -1 conf-ld`" \
	'-o "$$main" "$$main".$(OBJEXT) $${1+"$$@"}' \
	) > load
	chmod 755 load

install-data-local:
	$(mkinstalldirs)  $(DESTDIR)@prefix@/lib
	$(mkinstalldirs)  $(DESTDIR)@prefix@/include

install-data-hook:
	for f in $(doc_list); do \
		/bin/mkdir -p $(DESTDIR)@datarootdir@/doc; \
		$(INSTALL_DATA) $$f $(DESTDIR)@datarootdir@/doc/$$f || exit 1 ;\
	done

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

AUTOMAKE_OPTIONS = foreign no-dependencies
