# $Id: Makefile.am,v 1.6 2003/11/15 19:16:25 mrsam Exp $
#
# Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

noinst_LIBRARIES=libwaitlib.a
noinst_PROGRAMS=testwait

EXTRA_DIST=confwait.c

libwaitlib_a_SOURCES=waitlib.c waitlib.h waitlib2.c

testwait_SOURCES=testwait.c testwait2.c
testwait_DEPENDENCIES=libwaitlib.a
testwait_LDADD=libwaitlib.a

check-am:
	./testwait
