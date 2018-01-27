#
# Copyright 1998 - 2008 Double Precision, Inc.  See COPYING for
# distribution information.


DISTCLEANFILES=couriertls.config

noinst_PROGRAMS=@COURIERTCPD@ @STARTTLS@ tlscachetest
EXTRA_PROGRAMS=couriertls couriertcpd
EXTRA_LTLIBRARIES=libcouriertls.la libcouriertlsopenssl.la libcouriertlsgnutls.la

BUILT_SOURCES=  couriertcpd.html couriertcpd.1 \
		couriertls.html couriertls.1

EXTRA_DIST=$(BUILT_SOURCES) couriertls.html README.couriertls \
	testsuite testsuite.txt

noinst_LTLIBRARIES=libspipe.la @BUILDLIBCOURIERTLS@ @LIBCOURIERTLSOPENSSL@ @LIBCOURIERTLSGNUTLS@ libtlsclient.la

libcouriertls_la_SOURCES=tlscache.c tlscache.h
libcouriertls_la_LIBADD=@TLSLIBRARY@
libcouriertls_la_DEPENDENCIES=@TLSLIBRARY@

couriertcpd_SOURCES=argparse.c argparse.h \
	tcpd.c tcpdaccess.c tcpremoteinfo.c tcpremoteinfo.h

couriertcpd_DEPENDENCIES= libspipe.la \
	../rfc1035/librfc1035.a \
	@dblibrary@ \
	../liblock/liblock.la\
	../numlib/libnumlib.la\
	../waitlib/libwaitlib.a\
	../soxwrap/libsoxwrap.a\
	../md5/libmd5.la ../random128/librandom128.la

couriertcpd_t=@NETLIBS@ @soxdep@

couriertcpd_LDADD= libspipe.la \
	../rfc1035/librfc1035.a \
        @dblibrary@ \
	../liblock/liblock.la\
        ../numlib/libnumlib.la\
        ../waitlib/libwaitlib.a\
	../soxwrap/libsoxwrap.a\
	../md5/libmd5.la ../random128/librandom128.la \
	$(couriertcpd_t:%=-Wl,%)
couriertcpd_LDFLAGS=-static


tlscachetest_SOURCES=tlscachetest.c
tlscachetest_DEPENDENCIES=../numlib/libnumlib.la ../liblock/liblock.la
tlscachetest_LDADD=../numlib/libnumlib.la ../liblock/liblock.la
tlscachetest_LDFLAGS=-static

libspipe_la_SOURCES=spipe.c spipe.h

couriertls_SOURCES=starttls.c argparse.c argparse.h

couriertls_t= @soxdep@

couriertls_DEPENDENCIES=libcouriertls.la libspipe.la ../rfc1035/librfc1035.a \
	../md5/libmd5.la ../random128/librandom128.la \
	../numlib/libnumlib.la ../liblock/liblock.la \
	../soxwrap/libsoxwrap.a
couriertls_LDADD=libcouriertls.la libspipe.la ../rfc1035/librfc1035.a \
	../md5/libmd5.la ../random128/librandom128.la \
	../numlib/libnumlib.la ../liblock/liblock.la \
	../soxwrap/libsoxwrap.a $(couriertls_t:%=-Wl,%)
couriertls_LDFLAGS=-static

libcouriertlsopenssl_la_SOURCES=libcouriertls.c libcouriertls.h tlsinfo.c
libcouriertlsopenssl_la_LIBADD=@openssldep@

libcouriertlsgnutls_la_SOURCES=libcouriergnutls.c tlsinfo.c
libcouriertlsgnutls_la_LIBADD=@gnutlsdep@

libtlsclient_la_SOURCES=tlsclient.c tlsclient.h \
	tlspasswordcache.c tlspasswordcache.h

if HAVE_SGML
couriertcpd.html: couriertcpd.sgml ../docbook/sgml2html
	../docbook/sgml2html couriertcpd.sgml couriertcpd.html

couriertcpd.1: couriertcpd.sgml ../docbook/sgml2html
	../docbook/sgml2man couriertcpd.sgml couriertcpd.1 "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"

couriertls.html: couriertls.sgml ../docbook/sgml2html
	../docbook/sgml2html couriertls.sgml couriertls.html

couriertls.1: couriertls.sgml ../docbook/sgml2html
	../docbook/sgml2man couriertls.sgml couriertls.1 "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
endif

check-am:
	sh $(srcdir)/testsuite 2>&1 | cmp -s - $(srcdir)/testsuite.txt
