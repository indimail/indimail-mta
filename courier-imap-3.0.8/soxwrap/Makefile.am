#$Id: Makefile.am,v 1.9 2004/07/24 03:36:53 mrsam Exp $
#
# Copyright 2000-2004 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

DISTCLEANFILES=soxlibs.dep

noinst_LIBRARIES=libsoxwrap.a

libsoxwrap_a_SOURCES=soxwrap.h \
	sconnect.c sconnect.h

noinst_PROGRAMS=testprog

testprog_SOURCES=testprog.c
testprog_DEPENDENCIES=libsoxwrap.a config.status
testprog_LDADD=libsoxwrap.a @SOCKLIBS@

check-am:
	./testprog
