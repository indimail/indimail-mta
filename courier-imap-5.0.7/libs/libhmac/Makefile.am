#
# Copyright 1998 - 2005 Double Precision, Inc.  See COPYING for
# distribution information.


noinst_LTLIBRARIES=libhmac.la

libhmac_la_SOURCES=hmac.c hmac.h

noinst_PROGRAMS= @TESTPROGS@

EXTRA_PROGRAMS=md5hmactest sha1hmactest

md5hmactest_SOURCES=md5hmactest.c
md5hmactest_DEPENDENCIES=libhmac.la @hashlibs@
md5hmactest_LDADD=$(md5hmactest_DEPENDENCIES)
md5hmactest_LDFLAGS=-static

sha1hmactest_SOURCES=sha1hmactest.c
sha1hmactest_DEPENDENCIES=libhmac.la @hashlibs@
sha1hmactest_LDADD=$(sha1hmactest_DEPENDENCIES)
sha1hmactest_LDFLAGS=-static

EXTRA_DIST=md5hmactest.txt sha1hmactest.txt

check-am:
	test ! -x ./md5hmactest || ./md5hmactest | cmp -s - $(srcdir)/md5hmactest.txt
	test ! -x ./sha1hmactest || ./sha1hmactest | cmp -s - $(srcdir)/sha1hmactest.txt
