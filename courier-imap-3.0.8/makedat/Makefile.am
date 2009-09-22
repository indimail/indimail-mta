# $Id: Makefile.am,v 1.5 2004/09/08 01:27:34 mrsam Exp $
#
# Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

noinst_PROGRAMS=@makedatprog_target@

EXTRA_PROGRAMS=makedatprog

makedatprog_SOURCES=makedatprog.c
makedatprog_DEPENDENCIES=@dblibrary@
makedatprog_LDADD=@dblibrary@ @LIBGDBM@ @LIBDB@
