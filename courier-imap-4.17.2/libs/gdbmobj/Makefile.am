#
# Copyright 1998 - 2008 Double Precision, Inc.  See COPYING for
# distribution information.


LIBGDBMSOURCES=gdbmobj.h gdbmobj.c gdbmobj2.c gdbmobj3.c
TESTGDBMSOURCES=testgdbm.C

if FOUND_GDBM
noinst_LTLIBRARIES=libgdbmobj.la
noinst_PROGRAMS=testgdbm

libgdbmobj_la_SOURCES=$(LIBGDBMSOURCES)
libgdbmobj_la_LIBADD=@LIBGDBM@

testgdbm_SOURCES=$(TESTGDBMSOURCES)

testgdbm_DEPENDENCIES=libgdbmobj.la
testgdbm_LDADD=libgdbmobj.la
testgdbm_LDFLAGS=-static

check-am:
	$(srcdir)/testsuite | cmp -s - $(srcdir)/testsuite.txt

else
noinst_SCRIPTS=notfound

notfound:
	@exit 0

endif

EXTRA_DIST=$(LIBGDBMSOURCES) $(TESTGDBMSOURCES) testsuite testsuite.txt
