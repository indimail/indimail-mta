# $Id: Makefile.am,v 1.43 2004/09/12 23:25:55 mrsam Exp $
#
# Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
# distribution information.

ACLOCAL_AMFLAGS=-I m4
AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

DISTCLEANFILES=imapd.pam pop3d.pam
CLEANFILES=imapd.8 imapd.html mkimapdcert.html mkimapdcert.8 \
	courierpop3d.html courierpop3d.8 mkpop3dcert.html mkpop3dcert.8

distclean-local:
	rm -rf confmdtest*

dbDEPENDENCIES=@dblibrary@
dbLDADD=@dblibrary@ @LIBGDBM@ @LIBDB@

EXTRA_DIST=testsuite testsuite.txt smaptestsuite smaptestsuite.txt \
	BUGS BUGS.html README README.html imapd.authpam \
	pop3d.authpam system-auth.authpam \
	imapd.html.in imapd.8.in \
	mkimapdcert.html.in mkimapdcert.8.in \
	mkpop3dcert.html.in mkpop3dcert.8.in \
	courierpop3d.html.in courierpop3d.8.in

noinst_SCRIPTS=mkimapdcert mkpop3dcert
noinst_PROGRAMS=imaplogin imapd pop3login pop3d
noinst_LIBRARIES=libimapd.a

noinst_DATA=imapd.8 imapd.html imapd.cnf pop3d.cnf \
	mkimapdcert.html mkimapdcert.8 \
	mkpop3dcert.html mkpop3dcert.8 \
	courierpop3d.html courierpop3d.8

libimapd_a_SOURCES=mainloop.c imaptoken.c imaptoken.h imapwrite.c imapwrite.h \
	capability.c externalauth.c smap.c smapsnapshot.c

if HAVE_AUTH
authc=authenticate_auth.c
else
authc=authenticate_stub.c
endif

imaplogin_SOURCES=imaplogin.c $(authc)
imaplogin_DEPENDENCIES=libimapd.a ../authlib/libauthmod.a ../authlib/libauth.a \
	../random128/librandom128.a ../rfc2045/librfc2045.la \
	../rfc822/librfc822.la ../md5/libmd5.a ../tcpd/libspipe.a \
	../unicode/libunicode.a ../numlib/libnumlib.a
imaplogin_LDADD=libimapd.a ../authlib/libauthmod.a ../authlib/libauth.a \
	../random128/librandom128.a ../rfc2045/librfc2045.la \
	../rfc822/libencode.la ../rfc822/librfc822.la ../md5/libmd5.a ../tcpd/libspipe.a \
	../unicode/libunicode.a ../numlib/libnumlib.a @NETLIBS@

imapd_SOURCES=fetch.c fetchinfo.c fetchinfo.h imapd.c imapd.h \
	imapscanclient.c imapscanclient.h \
	mailboxlist.c mailboxlist.h \
	msgbodystructure.c msgenvelope.c \
	mysignal.c mysignal.h \
	outbox.c outbox.h \
	thread.c thread.h \
	search.c searchinfo.c searchinfo.h \
	storeinfo.c storeinfo.h

imapd_DEPENDENCIES=libimapd.a ../rfc2045/librfc2045.la ../maildir/libmaildir.a \
	../unicode/libunicode.a ../authlib/libauthmod.a ../authlib/libauth.a \
	../rfc822/librfc822.la ../liblock/liblock.a ../numlib/libnumlib.a \
	../maildir/maildir.libdeps @dblibrary@

imapd_LDADD=libimapd.a ../rfc2045/librfc2045.la ../maildir/libmaildir.a \
	../authlib/libauthmod.a ../authlib/libauth.a \
	../rfc822/librfc822.la ../liblock/liblock.a ../numlib/libnumlib.a \
	../unicode/libunicode.a `cat ../maildir/maildir.libdeps` \
	@dblibrary@ @LIBDB@ @LIBGDBM@ @DEBUGLIB@

pop3login_SOURCES=pop3login.c pop3dcapa.c externalauth.c
pop3login_DEPENDENCIES=../authlib/libauthmod.a ../authlib/libauth.a \
			../random128/librandom128.a ../md5/libmd5.a \
			../rfc2045/librfc2045.la ../rfc822/librfc822.la \
			../unicode/libunicode.a ../tcpd/libspipe.a
pop3login_LDADD=../authlib/libauthmod.a ../authlib/libauth.a \
			../random128/librandom128.a ../md5/libmd5.a \
			../rfc2045/librfc2045.la ../rfc822/libencode.la ../rfc822/librfc822.la \
			../unicode/libunicode.a ../tcpd/libspipe.a

pop3d_SOURCES=pop3dserver.c pop3dcapa.c externalauth.c
pop3d_DEPENDENCIES=../authlib/libauthmod.a ../authlib/libauth.a \
	../maildir/libmaildir.a ../rfc822/librfc822.la ../numlib/libnumlib.a
pop3d_LDADD=../authlib/libauthmod.a ../authlib/libauth.a \
	../maildir/libmaildir.a ../rfc822/librfc822.la ../numlib/libnumlib.a

HTML2TXT=`which lynx 2>/dev/null && echo "lynx -dump -nolist" && exit; echo "links -dump"`

README: README.html
	$(HTML2TXT) README.html >README

BUGS: BUGS.html
	$(HTML2TXT) BUGS.html >BUGS

imapd.html: imapd.html.in
	CONFIG_FILES=imapd.html CONFIG_HEADERS= $(SHELL) ./config.status

imapd.8: imapd.8.in
	CONFIG_FILES=imapd.8 CONFIG_HEADERS= $(SHELL) ./config.status

mkimapdcert.html: mkimapdcert.html.in
	CONFIG_FILES=mkimapdcert.html CONFIG_HEADERS= $(SHELL) ./config.status

mkimapdcert.8: mkimapdcert.8.in
	CONFIG_FILES=mkimapdcert.8 CONFIG_HEADERS= $(SHELL) ./config.status

mkpop3dcert.html: mkpop3dcert.html.in
	CONFIG_FILES=mkpop3dcert.html CONFIG_HEADERS= $(SHELL) ./config.status

mkpop3dcert.8: mkpop3dcert.8.in
	CONFIG_FILES=mkpop3dcert.8 CONFIG_HEADERS= $(SHELL) ./config.status

courierpop3d.html: courierpop3d.html.in
	CONFIG_FILES=courierpop3d.html CONFIG_HEADERS= $(SHELL) ./config.status

courierpop3d.8: courierpop3d.8.in
	CONFIG_FILES=courierpop3d.8 CONFIG_HEADERS= $(SHELL) ./config.status

if HAVE_SGML
imapd.html.in: imapd.sgml ../docbook/sgml2html
	../docbook/sgml2html imapd.sgml imapd.html.in

imapd.8.in: imapd.sgml ../docbook/sgml2man
	../docbook/sgml2man imapd.sgml imapd.8.in

mkimapdcert.html.in: mkimapdcert.sgml ../docbook/sgml2html
	../docbook/sgml2html mkimapdcert.sgml mkimapdcert.html.in

mkimapdcert.8.in: mkimapdcert.sgml ../docbook/sgml2man
	../docbook/sgml2man mkimapdcert.sgml mkimapdcert.8.in

mkpop3dcert.html.in: mkpop3dcert.sgml ../docbook/sgml2html
	../docbook/sgml2html mkpop3dcert.sgml mkpop3dcert.html.in

mkpop3dcert.8.in: mkpop3dcert.sgml ../docbook/sgml2man
	../docbook/sgml2man mkpop3dcert.sgml mkpop3dcert.8.in

courierpop3d.html.in: courierpop3d.sgml ../docbook/sgml2html
	../docbook/sgml2html courierpop3d.sgml courierpop3d.html.in

courierpop3d.8.in: courierpop3d.sgml ../docbook/sgml2man
	../docbook/sgml2man courierpop3d.sgml courierpop3d.8.in

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

testsuite-imap:
	@LC_ALL=C; export LC_ALL; $(srcdir)/testsuite | ./testsuitefix.pl | sort

testsuite-smap:
	@LC_ALL=C; export LC_ALL; test "@smap@" = "yes" || exit 0; @SHELL@ $(srcdir)/smaptestsuite | ./testsuitefix.pl | sort
