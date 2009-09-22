#$Id: Makefile.am,v 1.3 2004/04/24 04:15:55 mrsam Exp $
#
# Copyright 2004 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

noinst_PROGRAMS=courierlogger

BUILT_SOURCES=courierlogger.html courierlogger.1

EXTRA_DIST=$(BUILT_SOURCES)

courierlogger_SOURCES=logger.c
courierlogger_DEPENDENCIES=../liblock/liblock.a \
	../numlib/libnumlib.a
courierlogger_LDADD=../liblock/liblock.a \
	../numlib/libnumlib.a

if HAVE_SGML
courierlogger.html: courierlogger.sgml ../docbook/sgml2html
	../docbook/sgml2html courierlogger.sgml courierlogger.html

courierlogger.1: courierlogger.sgml ../docbook/sgml2html
	../docbook/sgml2man courierlogger.sgml courierlogger.1
endif
