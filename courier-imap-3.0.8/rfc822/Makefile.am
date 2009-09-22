# $Id: Makefile.am,v 1.17 2004/04/10 03:33:02 mrsam Exp $
#
# Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

noinst_LIBRARIES=libencode.a librfc822.a

libencode_a_SOURCES=encode.c encode.h

librfc822_a_SOURCES=rfc822.c rfc822.h rfc822hdr.c rfc822hdr.h \
		  rfc822_getaddr.c rfc822_getaddrs.c \
                  rfc822_mkdate.c rfc822_parsedt.c rfc2047u.c \
                  rfc2047.c rfc2047.h imapsubj.c imaprefs.h imaprefs.c \
				  encodeautodetect.c

BUILT_SOURCES=rfc822.3 rfc822.html
noinst_DATA=$(BUILT_SOURCES)

noinst_PROGRAMS=testsuite reftest
testsuite_SOURCES=testsuite.c
testsuite_DEPENDENCIES=librfc822.a
testsuite_LDADD=librfc822.a
reftest_SOURCES=reftest.c imaprefs.h
reftest_DEPENDENCIES=librfc822.a
reftest_LDADD=librfc822.a

EXTRA_DIST=testsuite.txt reftest.txt $(BUILT_SOURCES)

if HAVE_SGML
rfc822.html: rfc822.sgml ../docbook/sgml2html
	../docbook/sgml2html rfc822.sgml rfc822.html

rfc822.3: rfc822.sgml ../docbook/sgml2html
	../docbook/sgml2man rfc822.sgml rfc822.3
endif

check-am:
	./testsuite | cmp -s - $(srcdir)/testsuite.txt
	./reftest | cmp -s - $(srcdir)/reftest.txt
