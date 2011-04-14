#
# Copyright 1998 - 2005 Double Precision, Inc.  See COPYING for
# distribution information.


LIBDBOBJSOURCES=bdbobj.h bdbobj.c bdbobj2.c bdbobj3.c
TESTBDBSOURCES=testbdb.C

if FOUND_DB
noinst_LTLIBRARIES=libbdbobj.la
noinst_PROGRAMS=testbdb

libbdbobj_la_SOURCES=$(LIBDBOBJSOURCES)
libbdbobj_la_LIBADD=@LIBDB@

testbdb_SOURCES=$(TESTBDBSOURCES)

testbdb_DEPENDENCIES=libbdbobj.la
testbdb_LDADD=libbdbobj.la
testbdb_LDFLAGS=-static

check-am:
	$(srcdir)/testsuite | cmp -s - $(srcdir)/testsuite.txt
else
noinst_SCRIPTS=notfound

notfound:
	@exit 0
endif

EXTRA_DIST=$(LIBDBOBJSOURCES) $(TESTBDBSOURCES) testsuite testsuite.txt

