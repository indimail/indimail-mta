#
# Copyright 2001-2008 Double Precision, Inc.
# See COPYING for distribution information.
#
# $Id: Makefile.am,v 1.9 2008/12/18 12:08:25 mrsam Exp $


if HMACC
HMAC=hmac.c
else
HMAC=
endif

noinst_LIBRARIES=libsha1.a

libsha1_a_SOURCES=sha1.c sha1.h sha1_hash.c sha256.c sha256_hash.c sha512.c sha512_hash.c $(HMAC)

noinst_PROGRAMS=testsuite

testsuite_SOURCES=testsuite.c
testsuite_DEPENDENCIES=libsha1.a
testsuite_LDADD=$(testsuite_DEPENDENCIES)

EXTRA_DIST=testsuite.txt hmac.c

check-am:
	./testsuite | cmp -s - $(srcdir)/testsuite.txt
