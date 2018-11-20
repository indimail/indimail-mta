#
# Copyright 1998 - 2015 Double Precision, Inc.  See COPYING for
# distribution information.

ACLOCAL_AMFLAGS=-I ../../m4

#CONFIG_STATUS_DEPENDENCIES = ../tcpd/couriertls.config

DISTCLEANFILES=imapd.pam pop3d.pam imapd.cnf pop3d.cnf
CLEANFILES=imapd.8 imapd.html mkimapdcert.html mkimapdcert.8 \
	makeimapaccess.html makeimapaccess.8 \
	mkdhparams.html mkdhparams.8 \
	pop3d.html pop3d.8 mkpop3dcert.html mkpop3dcert.8

EXTRA_DIST=testsuite testsuite.txt smaptestsuite smaptestsuite.txt \
	BUGS BUGS.html README README.html imapd.authpam \
	pop3d.authpam system-auth.authpam system-auth2.authpam\
	makeimapaccess.8.in makeimapaccess.html.in \
	imapd.html.in imapd.8.in \
	mkdhparams.html.in mkdhparams.8.in \
	mkimapdcert.html.in mkimapdcert.8.in \
	mkpop3dcert.html.in mkpop3dcert.8.in \
	pop3d.html.in pop3d.8.in \
	imapd.cnf.gnutls pop3d.cnf.gnutls

bin_PROGRAMS=imapd pop3d
sbin_PROGRAMS=imaplogin pop3login
libexec_SCRIPTS=mkimapdcert mkpop3dcert mkdhparams

sysconf_DATA = imapd.dist imapd-ssl.dist pop3d.dist pop3d-ssl.dist imapd.cnf pop3d.cnf
noinst_DATA=imapd.html makeimapaccess.html mkdhparams.html mkimapdcert.html \
	mkpop3dcert.html pop3d.html 
man_MANS=mkdhparams.8 mkimapdcert.8 mkpop3dcert.8 imapd.8 pop3d.8 imaplogin.8 pop3login.8 \
		 makeimapaccess.8

imapd.cnf: imapd.cnf.@ssllib@
	cp imapd.cnf.@ssllib@ imapd.cnf
	touch imapd.cnf

imapd.cnf: $(top_builddir)/config.status

pop3d.cnf: pop3d.cnf.@ssllib@
	cp pop3d.cnf.@ssllib@ pop3d.cnf
	touch pop3d.cnf

pop3d.cnf: $(top_builddir)/config.status

noinst_LTLIBRARIES=libimaplogin.la libimapd.la libpop3d.la
libimaplogin_la_SOURCES=
libimaplogin_la_LIBADD=../tcpd/libspipe.la ../tcpd/libtlsclient.la \
	../numlib/libnumlib.la
libimaplogin_la_LDFLAGS=-static

libimapd_la_SOURCES=mainloop.c imaptoken.c imaptoken.h imapwrite.c \
	imapwrite.h capability.c externalauth.c smap.c smapsnapshot.c
libimapd_la_LIBADD= ../rfc2045/librfc2045.la ../maildir/libmaildir.la \
	../rfc822/librfc822.la ../liblock/liblock.la ../numlib/libnumlib.la \
	../unicode/libunicode.la
libimapd_la_LDFLAGS=-static

imaplogin_SOURCES=imaplogin.c authenticate_auth.c
imaplogin_DEPENDENCIES=libimapd.la libimaplogin.la ../authlib/libauthmod.la ../authlib/libauth.la
imaplogin_LDADD=libimapd.la libimaplogin.la ../authlib/libauthmod.la ../authlib/libauth.la \
	../rfc822/libencode.la ../rfc822/librfc822.la ../md5/libmd5.la ../tcpd/libspipe.la \
	../random128/librandom128.la ../md5/libmd5.la \
	@NETLIBS@

imapd_SOURCES=fetch.c fetchinfo.c fetchinfo.h imapd.c imapd.h \
	imapscanclient.c imapscanclient.h \
	mailboxlist.c mailboxlist.h \
	msgbodystructure.c msgenvelope.c \
	mysignal.c mysignal.h \
	outbox.c outbox.h \
	thread.c thread.h \
	search.c searchinfo.c searchinfo.h \
	storeinfo.c storeinfo.h

imapd_DEPENDENCIES=libimapd.la \
	../authlib/libauthmod.la ../authlib/libauth.la \
	../maildir/maildir.libdeps @dblibrary@

imapd_LDADD=libimapd.la ../authlib/libauthmod.la ../authlib/libauth.la \
	`cat ../maildir/maildir.libdeps` @dblibrary@ @DEBUGLIB@

pop3login_SOURCES=pop3login.c pop3dcapa.c
pop3login_DEPENDENCIES=libpop3d.la \
	../authlib/libauthmod.la ../authlib/libauth.la \
	../rfc822/librfc822.la ../liblock/liblock.la ../numlib/libnumlib.la
pop3login_LDADD=../tcpd/libtlsclient.la libpop3d.la \
	../authlib/libauthmod.la ../authlib/libauth.la \
	../rfc822/libencode.la ../rfc822/librfc822.la ../md5/libmd5.la ../tcpd/libspipe.la \
	../random128/librandom128.la ../md5/libmd5.la \
	@NETLIBS@

libpop3d_la_SOURCES=externalauth.c
libpop3d_la_LIBADD=../maildir/libmaildir.la \
	../rfc2045/librfc2045.la \
	../rfc822/librfc822.la \
	../numlib/libnumlib.la
libpop3d_la_DEPENDENCIES=$(libpop3d_la_LIBADD)
libpop3d_la_LDFLAGS=-static

pop3d_SOURCES=pop3dserver.c pop3dcapa.c
pop3d_DEPENDENCIES=libpop3d.la
pop3d_LDADD=libpop3d.la ../authlib/libauthmod.la ../authlib/libauth.la ../unicode/libunicode.la

HTML2TXT=links -dump -no-numbering

README: README.html
	$(HTML2TXT) README.html >README

BUGS: BUGS.html
	$(HTML2TXT) BUGS.html >BUGS

imapd.html: imapd.html.in
	./config.status --file=imapd.html

imapd.8: imapd.8.in
	./config.status --file=imapd.8

makeimapaccess.html: makeimapaccess.html.in
	./config.status --file=makeimapaccess.html

makeimapaccess.8: makeimapaccess.8.in
	./config.status --file=makeimapaccess.8

mkdhparams.html: mkdhparams.html.in
	./config.status --file=mkdhparams.html

mkdhparams.8: mkdhparams.8.in
	./config.status --file=mkdhparams.8

mkimapdcert.html: mkimapdcert.html.in
	./config.status --file=mkimapdcert.html

mkimapdcert.8: mkimapdcert.8.in
	./config.status --file=mkimapdcert.8

mkpop3dcert.html: mkpop3dcert.html.in
	./config.status --file=mkpop3dcert.html

mkpop3dcert.8: mkpop3dcert.8.in
	./config.status --file=mkpop3dcert.8

pop3d.html: pop3d.html.in
	./config.status --file=pop3d.html

pop3d.8: pop3d.8.in
	./config.status --file=pop3d.8

if HAVE_SGML

GITFILES=imapd.dist.in imapd-ssl.dist.in pop3d.dist.in pop3d-ssl.dist.in

BUILT_SOURCES = $(GITFILES)

$(GITFILES): %: %.git
	perl -p -e 's/\$$Id:[^\$$]*/\$$Id: '"`git log --pretty=format:\"%H\" -n 1 -- $<`-`date +%Y%m%d%H%M%S`/" <$< >$@.tmp
	mv -f $@.tmp $@

imapd.html.in: imapd.sgml ../docbook/sgml2html
	../docbook/sgml2html imapd.sgml imapd.html.in

imapd.8.in: imapd.sgml ../docbook/sgml2man
	../docbook/sgml2man imapd.sgml imapd.8.in "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
	mv imapd.8 imapd.8.in

makeimapaccess.html.in: makeimapaccess.sgml ../docbook/sgml2html
	../docbook/sgml2html makeimapaccess.sgml makeimapaccess.html.in

makeimapaccess.8.in: makeimapaccess.sgml ../docbook/sgml2man
	../docbook/sgml2man makeimapaccess.sgml makeimapaccess.8.in "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
	mv makeimapaccess.8 makeimapaccess.8.in

mkdhparams.html.in: mkdhparams.sgml ../docbook/sgml2html
	../docbook/sgml2html mkdhparams.sgml mkdhparams.html.in

mkdhparams.8.in: mkdhparams.sgml ../docbook/sgml2man
	../docbook/sgml2man mkdhparams.sgml mkdhparams.8.in "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
	mv mkdhparams.8 mkdhparams.8.in

mkimapdcert.html.in: mkimapdcert.sgml ../docbook/sgml2html
	../docbook/sgml2html mkimapdcert.sgml mkimapdcert.html.in

mkimapdcert.8.in: mkimapdcert.sgml ../docbook/sgml2man
	../docbook/sgml2man mkimapdcert.sgml mkimapdcert.8.in "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
	mv mkimapdcert.8 mkimapdcert.8.in

mkpop3dcert.html.in: mkpop3dcert.sgml ../docbook/sgml2html
	../docbook/sgml2html mkpop3dcert.sgml mkpop3dcert.html.in

mkpop3dcert.8.in: mkpop3dcert.sgml ../docbook/sgml2man
	../docbook/sgml2man mkpop3dcert.sgml mkpop3dcert.8.in "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
	mv mkpop3dcert.8 mkpop3dcert.8.in

pop3d.html.in: pop3d.sgml ../docbook/sgml2html
	../docbook/sgml2html pop3d.sgml pop3d.html.in

pop3d.8.in: pop3d.sgml ../docbook/sgml2man
	../docbook/sgml2man pop3d.sgml pop3d.8.in "--stringparam man.base.url.for.relative.links http://www.courier-mta.org/"
	mv pop3d.8 pop3d.8.in

endif

check-am:
	@test "@MAKECHECKBROKEN@" = "Y" || exit 0; echo "" ; echo "Error: --with-trashquota or the --enable-workarounds-for-imap-client-bugs" ; echo "option was specified to the configure script."; echo ""; echo "As INSTALL told you, make check fails if these options are used, and I wasn't"; echo "kidding when I wrote it.  Reconfigure and rebuild without these options, then"; echo "rerun make and make check.  If make check passes, reconfigure again with your"; echo "original options, and proceed with installing this server.  Have fun!"; exit 1
	@cp /dev/null conftest1 ; chmod 000 conftest1 ; test -w conftest1 || \
		exit 0; echo "=============================" ; \
		echo "Do not run make check as root" ; \
		echo "=============================" ; exit 1
	@rm -f conftest1
	@chmod +x testsuitefix.pl
	LC_ALL=C; export LC_ALL; $(srcdir)/testsuite | ./testsuitefix.pl | sort | cmp -s - $(srcdir)/testsuite.txt
	LC_ALL=C; export LC_ALL; test "@smap@" = "yes" || exit 0; @SHELL@ $(srcdir)/smaptestsuite | ./testsuitefix.pl | sort | cmp -s - $(srcdir)/smaptestsuite.txt
	rm -rf confmdtest

testsuite-imap:
	@LC_ALL=C; export LC_ALL; $(srcdir)/testsuite | ./testsuitefix.pl | sort

testsuite-smap:
	@LC_ALL=C; export LC_ALL; test "@smap@" = "yes" || exit 0; @SHELL@ $(srcdir)/smaptestsuite | ./testsuitefix.pl | sort
