@SET_MAKE@
COFLAGS=-z+05:30

INSTALL_PROGRAM = ${INSTALL} -c
install_sh_PROGRAM = $(install_sh) -c
INSTALL_SCRIPT = ${INSTALL}
install_sh_SCRIPT = $(install_sh) -c
INSTALL_STRIP_PROGRAM = $(install_sh) -c $(STRIP_OPT)

doc_list= README.mpack

bin_PROGRAMS=mpack munpack
mpack_SOURCES=unixpk.c encode.c codes.c magic.c unixos.c xmalloc.c \
	md5c.c
munpack_SOURCES=unixunpk.c decode.c uudecode.c codes.c unixos.c \
	part.c xmalloc.c md5c.c

mpack_LDADD=
munpack_LDADD=

noinst_HEADERS=common.h md5.h part.h version.h xmalloc.h
man_MANS=mpack.1 munpack.1

BUILT_SOURCES=mpack.1 munpack.1

mpack.1: unixpk.man
	/bin/cp $? $@
munpack.1: unixunpk.man
	/bin/cp $? $@

install-data-hook:
	for f in $(doc_list); do \
		/bin/mkdir -p $(DESTDIR)@datarootdir@/doc; \
		$(INSTALL_DATA) $$f $(DESTDIR)@datarootdir@/doc/$$f || exit 1 ;\
	done
