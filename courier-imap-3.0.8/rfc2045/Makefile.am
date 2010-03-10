#$Id: Makefile.am,v 1.36 2009/11/22 19:39:52 mrsam Exp $
#
# Copyright 1998 - 2009 Double Precision, Inc.  See COPYING for
# distribution information.


ACLOCAL_AMFLAGS=-I m4
DISTCLEANFILES=testsuite.dat

noinst_LTLIBRARIES=librfc2045.la
noinst_PROGRAMS=reformime makemime headercheck

BUILT_SOURCES=reformime.html reformime.1 makemime.html makemime.1 \
	rfc2045.html rfc2045.3

EXTRA_DIST=$(BUILT_SOURCES) testsuite testsuite.txt

noinst_DATA=$(BUILT_SOURCES)

librfc2045_la_SOURCES=rfc2045.c rfc2045.h rfc2045acchk.c rfc2045acprep.c \
		     rfc2045appendurl.c rfc2045cdecode.c rfc2045decode.c \
		     rfc2045enomem.c \
		     rfc2045_fromfd.c \
                     rfc2045find.c rfc2045mkboundary.c rfc2045rewrite.c \
                     rfc2045tryboundary.c rfc2045xdump.c \
		     rfc2045searchcontenttype.c rfc2045decodemimesection.c \
		     rfc2045decodemimesectionu.c rfc2045header.c \
		     rfc2045replyunicode.c rfc2045reply.c \
		     rfc2231.c rfc2231encode.c \
		     rfc2646.h rfc2646.c rfc2646create.c rfc2646fwd.c \
		     rfc2646reply.c rfc2646rewrap.c \
		     base64.c base64.h

reformime_SOURCES=reformime.c
reformime_LDADD = librfc2045.la ../rfc822/libencode.la ../rfc822/librfc822.la \
		../unicode/libunicode.a ../numlib/libnumlib.a
reformime_DEPENDENCIES = $(reformime_LDADD)
reformime_LDFLAGS=-static

makemime_SOURCES=makemime.c
makemime_LDADD = ../rfc822/libencode.la ../rfc822/librfc822.la librfc2045.la ../unicode/libunicode.a ../numlib/libnumlib.a ../unicode/libunicode.a
makemime_DEPENDENCIES=$(makemime_LDADD)
makemime_LDFLAGS=-static

headercheck_SOURCES=headercheck.c
headercheck_LDADD=librfc2045.la ../rfc822/librfc822.la ../numlib/libnumlib.a \
		../unicode/libunicode.a
headercheck_DEPENDENCIES=$(headercheck_LDADD)
headercheck_LDFLAGS=-static

if HAVE_SGML
reformime.html: reformime.sgml ../docbook/sgml2html
	../docbook/sgml2html reformime.sgml reformime.html

reformime.1: reformime.sgml ../docbook/sgml2html
	../docbook/sgml2man reformime.sgml reformime.1

makemime.html: makemime.sgml ../docbook/sgml2html
	../docbook/sgml2html makemime.sgml makemime.html

makemime.1: makemime.sgml ../docbook/sgml2html
	../docbook/sgml2man makemime.sgml makemime.1

rfc2045.html: rfc2045.sgml ../docbook/sgml2html
	../docbook/sgml2html rfc2045.sgml rfc2045.html

rfc2045.3: rfc2045.sgml ../docbook/sgml2html
	../docbook/sgml2man rfc2045.sgml rfc2045.3
endif

check-am:
	@SHELL@ $(srcdir)/testsuite | cmp -s - $(srcdir)/testsuite.txt
