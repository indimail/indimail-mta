#
# Copyright 2001-2008 Double Precision, Inc.
# See COPYING for distribution information.
#


if HMACC
HMAC=hmac.c
else
HMAC=
endif

noinst_LTLIBRARIES=libsha1.la

libsha1_la_SOURCES=sha1.c sha1.h sha1_hash.c sha256.c sha256_hash.c sha512.c sha512_hash.c $(HMAC)

noinst_PROGRAMS=testsuite

testsuite_SOURCES=testsuite.c
testsuite_DEPENDENCIES=libsha1.la
testsuite_LDADD=$(testsuite_DEPENDENCIES)
testsuite_LDFLAGS=-static

EXTRA_DIST=testsuite.txt hmac.c

check-am:
	./testsuite | cmp -s - $(srcdir)/testsuite.txt
