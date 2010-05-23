@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

INSTALL_PROGRAM = ${INSTALL}       -o root -g qmail -m 555
install_sh_PROGRAM = $(install_sh) -c -o root -g qmail -m 555
INSTALL_SCRIPT = ${INSTALL}        -c -o root -g qmail -m 555
install_sh_SCRIPT = $(install_sh)  -c -o root -g qmail -m 555
INSTALL_STRIP_PROGRAM = $(install_sh) -c -o root -g qmail -m 555 -s

include_HEADERS = eps_line.h eps_buffer.h eps_unroll.h eps_content.h eps.h eps_int_stream.h rfc2822.h eps_misc.h eps_interface.h \
	eps_header.h eps_email.h eps_boundary.h eps_mime.h eps_address.h eps_base64.h eps_roll.h eps_int_buffer.h
COMMONSOURCES=eps_line.c eps_buffer.c eps_unroll.c eps_content.c eps.c eps_int_stream.c rfc2822.c eps_misc.c eps_header.c \
	eps_email.c eps_boundary.c eps_mime.c eps_address.c eps_base64.c eps_roll.c eps_int_buffer.c
INC = $(include_HEADERS)

libeps_LTLIBRARIES = libeps.la
libeps_la_SOURCES=$(COMMONSOURCES) 
libeps_la_LIBADD =
libeps_la_LDFLAGS = -version-info @LIBVER@ -release @VERSION@
libepsdir=@libdir@

eps_line.o: $(INC)
eps_buffer.o: $(INC)
eps_unroll.o: $(INC)
eps_content.o: $(INC)
eps.o: $(INC)
eps_int_stream.o: $(INC)
rfc2822.o: $(INC)
eps_misc.o: $(INC)
eps_header.o: $(INC)
eps_email.o: $(INC)
eps_boundary.o: $(INC)
eps_mime.o: $(INC)
eps_address.o: $(INC)
eps_base64.o: $(INC)
eps_roll.o: $(INC)
eps_int_buffer.o: $(INC)

install-data-local:
	$(mkinstalldirs)  $(DESTDIR)@prefix@/lib
	$(mkinstalldirs)  $(DESTDIR)@prefix@/include

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

AUTOMAKE_OPTIONS = foreign no-dependencies
