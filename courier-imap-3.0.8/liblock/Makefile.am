# $Id: Makefile.am,v 1.6 2002/10/07 05:43:04 mrsam Exp $
#
# Copyright 1998 - 2002 Double Precision, Inc.  See COPYING for
# distribution information.
#

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

noinst_LIBRARIES=liblock.a
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

liblock_a_SOURCES=$(fcntl) $(flock) $(lockf) liblock.h lockdaemon.c \
	mail.h mail.c

lockmail_SOURCES=lockmail.c
lockmail_DEPENDENCIES=liblock.a ../numlib/libnumlib.a
lockmail_LDADD=liblock.a ../numlib/libnumlib.a

noinst_DATA=lockmail.html lockmail.1
EXTRA_DIST=lockfcntl.c lockflock.c locklockf.c locktest.c $(noinst_DATA)

if HAVE_SGML
lockmail.html: lockmail.sgml ../docbook/sgml2html
	../docbook/sgml2html lockmail.sgml lockmail.html

lockmail.1: lockmail.sgml ../docbook/sgml2man
	../docbook/sgml2man lockmail.sgml lockmail.1
endif
