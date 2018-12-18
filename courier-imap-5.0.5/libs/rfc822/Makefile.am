#
# Copyright 1998 - 2009 Double Precision, Inc.  See COPYING for
# distribution information.


noinst_LTLIBRARIES=libencode.la librfc822.la

librfc822_la_SOURCES=rfc822.c rfc822.h rfc822hdr.c rfc822hdr.h \
		  rfc822_getaddr.c rfc822_getaddrs.c \
                  rfc822_mkdate.c rfc822_parsedt.c rfc2047u.c \
                  rfc2047.c rfc2047.h imapsubj.c imaprefs.h imaprefs.c \
		  encodeautodetect.c
librfc822_la_LIBADD = $(LIBIDN_LIBS)

DISTCLEANFILES=rfc822.config

AM_CFLAGS = $(LIBIDN_CFLAGS)

libencode_la_SOURCES=encode.c encode.h

BUILT_SOURCES=rfc822.3 rfc822.html
noinst_DATA=$(BUILT_SOURCES)

noinst_PROGRAMS=testsuite reftest
testsuite_SOURCES=testsuite.c
testsuite_DEPENDENCIES=librfc822.la
testsuite_LDADD=librfc822.la ../unicode/libunicode.la
testsuite_LDFLAGS=-static

reftest_SOURCES=reftest.c imaprefs.h
reftest_DEPENDENCIES=librfc822.la
reftest_LDADD=librfc822.la ../unicode/libunicode.la
reftest_LDFLAGS=-static

EXTRA_DIST=testsuite.txt reftest.txt $(BUILT_SOURCES)

if HAVE_SGML
rfc822.html: rfc822.sgml ../docbook/sgml2html
	../docbook/sgml2html rfc822.sgml rfc822.html

rfc822.3: rfc822.sgml ../docbook/sgml2html
	../docbook/sgml2man rfc822.sgml rfc822.3 "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
endif

check-am:
	./testsuite | cmp -s - $(srcdir)/testsuite.txt
	./reftest | cmp -s - $(srcdir)/reftest.txt
