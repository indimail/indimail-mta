#$Id: Makefile.am,v 1.25 2004/04/18 16:02:41 mrsam Exp $
#
# Copyright 1998 - 2003 Double Precision, Inc.  See COPYING for
# distribution information.

AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

DISTCLEANFILES=couriertls.config libcouriertls.dep test.dat

noinst_PROGRAMS=@COURIERTCPD@ @STARTTLS@ tlscachetest
EXTRA_PROGRAMS=couriertls couriertcpd
EXTRA_LIBRARIES=libcouriertls.a

BUILT_SOURCES=  couriertcpd.html couriertcpd.1 \
		couriertls.html couriertls.1

EXTRA_DIST=$(BUILT_SOURCES) couriertls.html README.couriertls \
	testsuite testsuite.txt

noinst_LIBRARIES=libspipe.a libtlsclient.a @LIBCOURIERTLS@

couriertcpd_SOURCES=argparse.c argparse.h \
	tcpd.c tcpdaccess.c tcpremoteinfo.c tcpremoteinfo.h

couriertcpd_DEPENDENCIES= libspipe.a \
	../rfc1035/librfc1035.a \
	@dblibrary@ \
	../liblock/liblock.a\
	../numlib/libnumlib.a\
	../waitlib/libwaitlib.a\
	../soxwrap/libsoxwrap.a\
	../md5/libmd5.a ../random128/librandom128.a \
	../soxwrap/soxlibs.dep

couriertcpd_LDADD= libspipe.a \
	../rfc1035/librfc1035.a \
        @dblibrary@ \
	../liblock/liblock.a\
        ../numlib/libnumlib.a\
        ../waitlib/libwaitlib.a\
	../md5/libmd5.a ../random128/librandom128.a \
        ../soxwrap/libsoxwrap.a\
	`cat ../soxwrap/soxlibs.dep`\
	@LIBGDBM@ @LIBDB@ @NETLIBS@


tlscachetest_SOURCES=tlscachetest.c
tlscachetest_DEPENDENCIES=../numlib/libnumlib.a ../liblock/liblock.a
tlscachetest_LDADD=../numlib/libnumlib.a ../liblock/liblock.a

libspipe_a_SOURCES=spipe.c spipe.h

couriertls_SOURCES=starttls.c argparse.c argparse.h

couriertls_DEPENDENCIES=libcouriertls.a libspipe.a ../rfc1035/librfc1035.a \
	../md5/libmd5.a ../random128/librandom128.a \
	../numlib/libnumlib.a ../liblock/liblock.a \
	../soxwrap/libsoxwrap.a ../soxwrap/soxlibs.dep
couriertls_LDADD=libcouriertls.a libspipe.a ../rfc1035/librfc1035.a \
	../md5/libmd5.a ../random128/librandom128.a \
	../numlib/libnumlib.a ../liblock/liblock.a \
	../soxwrap/libsoxwrap.a `cat ../soxwrap/soxlibs.dep` `cat libcouriertls.dep`

libcouriertls_a_SOURCES=libcouriertls.c libcouriertls.h tlsinfo.c \
	tlscache.c tlscache.h

libtlsclient_a_SOURCES=tlsclient.c tlsclient.h \
	tlspasswordcache.c tlspasswordcache.h

if HAVE_SGML
couriertcpd.html: couriertcpd.sgml ../docbook/sgml2html
	../docbook/sgml2html couriertcpd.sgml couriertcpd.html

couriertcpd.1: couriertcpd.sgml ../docbook/sgml2html
	../docbook/sgml2man couriertcpd.sgml couriertcpd.1

couriertls.html: couriertls.sgml ../docbook/sgml2html
	../docbook/sgml2html couriertls.sgml couriertls.html

couriertls.1: couriertls.sgml ../docbook/sgml2html
	../docbook/sgml2man couriertls.sgml couriertls.1
endif

check-am:
	sh $(srcdir)/testsuite 2>&1 | cmp -s - $(srcdir)/testsuite.txt
