# $Id: Makefile.am,v 1.13 2002/10/07 05:43:05 mrsam Exp $
#
# Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

noinst_LIBRARIES=libuserdb.a
libuserdb_a_SOURCES=userdb.c userdb.h userdb2.c userdbmkpw.c

BUILT_SOURCES=	makeuserdb.html.in makeuserdb.8.in \
		userdb.html.in userdb.8.in \
		userdbpw.html.in userdbpw.8.in

noinst_SCRIPTS=makeuserdb pw2userdb vchkpw2userdb dummy
noinst_PROGRAMS=userdbpw
noinst_DATA=makeuserdb.html userdb.html userdbpw.html

userdbpw_SOURCES=userdbpw.c
userdbpw_LDADD=libuserdb.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ @CRYPTLIBS@

man8=makeuserdb.8 userdb.8 userdbpw.8

CLEANFILES=$(man8) $(noinst_DATA) dummy

makeuserdb.html: makeuserdb.html.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

makeuserdb.8: makeuserdb.8.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

userdb.html: userdb.html.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

userdb.8: userdb.8.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

userdbpw.html: userdbpw.html.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

userdbpw.8: userdbpw.8.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

if HAVE_SGML
makeuserdb.html.in: makeuserdb.sgml ../docbook/sgml2html
	../docbook/sgml2html makeuserdb.sgml makeuserdb.html.in

makeuserdb.8.in: makeuserdb.sgml ../docbook/sgml2man
	../docbook/sgml2man makeuserdb.sgml makeuserdb.8.in

userdb.html.in: userdb.sgml ../docbook/sgml2html
	../docbook/sgml2html userdb.sgml userdb.html.in

userdb.8.in: userdb.sgml ../docbook/sgml2man
	../docbook/sgml2man userdb.sgml userdb.8.in

userdbpw.html.in: userdbpw.sgml ../docbook/sgml2html
	../docbook/sgml2html userdbpw.sgml userdbpw.html.in

userdbpw.8.in: userdbpw.sgml ../docbook/sgml2man
	../docbook/sgml2man userdbpw.sgml userdbpw.8.in
endif

EXTRA_DIST=$(BUILT_SOURCES)

# Temporary autoconf kludge:

dummy: $(man8)
	touch dummy
