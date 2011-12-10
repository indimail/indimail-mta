#
# Copyright 1998 - 2008 Double Precision, Inc.  See COPYING for
# distribution information.


ACLOCAL_AMFLAGS=-I ../m4
noinst_PROGRAMS=@makedatprog_target@

EXTRA_PROGRAMS=makedatprog

makedatprog_SOURCES=makedatprog.c
makedatprog_DEPENDENCIES=@dblibrary@
makedatprog_LDADD=@dblibrary@
makedatprog_LDFLAGS=-static
