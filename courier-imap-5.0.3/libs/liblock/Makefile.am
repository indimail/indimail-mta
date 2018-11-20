#
# Copyright 1998 - 2002 Double Precision, Inc.  See COPYING for
# distribution information.
#


noinst_LTLIBRARIES=liblock.la
noinst_PROGRAMS=lockmail

if USE_FCNTL
fcntl=lockfcntl.c
else
fcntl=
endif

if USE_FLOCK
flock=lockflock.c
else
flock=
endif

if USE_LOCKF
lockf=locklockf.c
else
lockf=
endif

liblock_la_SOURCES=$(fcntl) $(flock) $(lockf) liblock.h lockdaemon.c \
	mail.h mail.c

lockmail_SOURCES=lockmail.c
lockmail_DEPENDENCIES=liblock.la ../numlib/libnumlib.la
lockmail_LDADD=liblock.la ../numlib/libnumlib.la
lockmail_LDFLAGS=-static

noinst_DATA=lockmail.html lockmail.1
EXTRA_DIST=lockfcntl.c lockflock.c locklockf.c locktest.c $(noinst_DATA)

if HAVE_SGML
lockmail.html: lockmail.sgml ../docbook/sgml2html
	../docbook/sgml2html lockmail.sgml lockmail.html

lockmail.1: lockmail.sgml ../docbook/sgml2man
	../docbook/sgml2man lockmail.sgml lockmail.1 "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
endif
