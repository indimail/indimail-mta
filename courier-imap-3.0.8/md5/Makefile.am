# $Id: Makefile.am,v 1.10 2003/11/15 18:38:33 mrsam Exp $
#
# Copyright 1998 - 2002 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

if HMACC
HMAC=hmac.c
else
HMAC=
endif

libmd5_a_SOURCES=md5.c md5.h md5_hash.c redhat-crypt-md5.c $(HMAC)
noinst_LIBRARIES=libmd5.a

noinst_PROGRAMS=md5test

md5test_SOURCES=md5test.c
md5test_DEPENDENCIES=libmd5.a
md5test_LDADD=libmd5.a

EXTRA_DIST=md5test.txt hmac.c

check-am:
	./md5test | cmp -s - $(srcdir)/md5test.txt
