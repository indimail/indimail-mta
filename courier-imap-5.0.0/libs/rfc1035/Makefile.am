#
# Copyright 1998 - 2011 Double Precision, Inc.  See COPYING for
# distribution information.


noinst_LIBRARIES=librfc1035.a

librfc1035_a_SOURCES= \
		rfc1035.c rfc1035.h rfc1035an.c \
		rfc1035dump.c rfc1035dumprrdata.c rfc1035fmttime.c \
		rfc1035gettxt.c rfc1035ifconf.c \
		rfc1035ipv6to4.c rfc1035mkaddress.c \
		rfc1035mksocket.c rfc1035mxlist.c rfc1035mxlist.h rfc1035qa.c \
		rfc1035qptr.c rfc1035reply.c rfc1035resolve.c rfc1035sameip.c \
		rfc1035search.c rfc1035sockaddrip.c rfc1035sockaddrport.c \
		rfc1035str.c rfc1035tcp.c rfc1035udp.c rfc1035bindsource.c \
\
		spf.c spf.h

noinst_PROGRAMS=testlookup testspf

testlookup_SOURCES=testlookup.c
testlookup_DEPENDENCIES=librfc1035.a ../rfc822/libencode.la ../md5/libmd5.la \
	../random128/librandom128.la ../soxwrap/libsoxwrap.a \
	../soxwrap/soxlibs.dep
testlookup_LDADD=librfc1035.a ../rfc822/libencode.la ../soxwrap/libsoxwrap.a ../md5/libmd5.la \
	../random128/librandom128.la `cat ../soxwrap/soxlibs.dep`
testlookup_LDFLAGS=-static

EXTRA_DIST=testsuite.txt

testspf_SOURCES=testspf.c
testspf_DEPENDENCIES=librfc1035.a ../rfc822/libencode.la ../md5/libmd5.la \
	../random128/librandom128.la ../soxwrap/libsoxwrap.a \
	../soxwrap/soxlibs.dep
testspf_LDADD=librfc1035.a ../rfc822/libencode.la ../md5/libmd5.la \
	../random128/librandom128.la ../soxwrap/libsoxwrap.a \
	`cat ../soxwrap/soxlibs.dep`
testspf_LDFLAGS=-static

mycheck:
	./testspf -test=1 | diff -U 3 - $(srcdir)/testsuite.txt
