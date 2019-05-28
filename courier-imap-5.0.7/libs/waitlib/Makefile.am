#
# Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
# distribution information.


noinst_LTLIBRARIES=libwaitlib.la
noinst_PROGRAMS=testwait

EXTRA_DIST=confwait.c

libwaitlib_la_SOURCES=waitlib.c waitlib.h waitlib2.c

testwait_SOURCES=testwait.c testwait2.c
testwait_DEPENDENCIES=libwaitlib.la
testwait_LDADD=libwaitlib.la

check-am:
	./testwait
