#$Id: Makefile.am,v 1.6 2003/11/15 18:38:33 mrsam Exp $
#
# Copyright 1998 - 2002 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

noinst_LIBRARIES=libhmac.a

libhmac_a_SOURCES=hmac.c hmac.h

noinst_PROGRAMS= @TESTPROGS@

EXTRA_PROGRAMS=md5hmactest sha1hmactest

md5hmactest_SOURCES=md5hmactest.c
md5hmactest_DEPENDENCIES=libhmac.a @hashlibs@
md5hmactest_LDADD=$(md5hmactest_DEPENDENCIES)

sha1hmactest_SOURCES=sha1hmactest.c
sha1hmactest_DEPENDENCIES=libhmac.a @hashlibs@
sha1hmactest_LDADD=$(sha1hmactest_DEPENDENCIES)

EXTRA_DIST=md5hmactest.txt sha1hmactest.txt

check-am:
	test ! -x ./md5hmactest || ./md5hmactest | cmp -s - $(srcdir)/md5hmactest.txt
	test ! -x ./sha1hmactest || ./sha1hmactest | cmp -s - $(srcdir)/sha1hmactest.txt
