#
# Copyright 1998 - 2005 Double Precision, Inc.  See COPYING for
# distribution information.


if HMACC
HMAC=hmac.c
else
HMAC=
endif

libmd5_la_SOURCES=md5.c md5.h md5_hash.c redhat-crypt-md5.c $(HMAC)

CLEANFILES=$(noinst_DATA)

noinst_LTLIBRARIES=libmd5.la

noinst_PROGRAMS=md5test

md5test_SOURCES=md5test.c
md5test_DEPENDENCIES=libmd5.la
md5test_LDADD=libmd5.la
md5test_LDFLAGS=-static

EXTRA_DIST=md5test.txt hmac.c

check-am:
	./md5test | cmp -s - $(srcdir)/md5test.txt
