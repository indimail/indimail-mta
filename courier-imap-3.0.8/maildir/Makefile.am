#$Id: Makefile.am,v 1.51 2004/07/28 01:18:20 mrsam Exp $
#
# Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
# distribution information.

ACLOCAL_AMFLAGS=-I m4
AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

noinst_LIBRARIES=libmaildir.a


DOCS=	deliverquota.html.in deliverquota.8.in \
	maildiracl.1.in \
	maildir.html maildir.5 \
	maildiracl.html.in \
	maildirmake.html.in maildirmake.1.in maildirquota.html maildirquota.7 \
	maildirkw.html maildirkw.1

if HAVE_SGML
BUILT_SOURCES=maildirsharedrc.h maildirfilterconfig.h quotawarnmsg.h \
	mailbot.h autoresponsequota.h $(noinst_DATA) $(DOCS)
else
BUILT_SOURCES=maildirsharedrc.h maildirfilterconfig.h quotawarnmsg.h \
	mailbot.h autoresponsequota.h $(noinst_DATA)
endif

noinst_DATA=deliverquota.html maildirmake.html deliverquota.8 maildirmake.1 \
	maildiracl.html maildiracl.1 maildir.libdeps

libmaildir_a_SOURCES=autoresponse.c autoresponse.h \
	maildiraclt.c maildiraclt.h \
	maildircache.c maildircache.h \
	maildircreate.c maildircreate.h \
	maildirfilename.c maildirgetnew.c \
	maildirfilter.c maildirfilter2.c \
	maildirfilter.h maildirfiltertypelist.h\
	maildirflags.c maildirmkdir.c \
	maildirgetquota.c maildirgetquota.h \
	maildirinfo.c maildirinfo.h \
	maildirkeywords.c maildirkeywords2.c maildirkeywords3.c \
	maildirkeywords4.cpp \
	maildirkeywords.h maildirlist.c maildirlock.c \
	maildirmake2.c \
	maildirnewshared.c maildirnewshared.h maildirnewshared2.c \
	maildiropen.c maildirparsequota.c \
	maildirpath.c maildirpurgetmp.c maildirmisc.h \
	maildirrename.c \
	maildirsearch.c maildirsearchC.cpp maildirsearch.h \
	maildirshared.c maildirshared2.c maildirdelfolder.c\
	maildirquota.c maildirquota.h maildirrequota.c maildirrequota.h \
	maildirwatch.c maildirwatch.h loginexec.c loginexec.h

noinst_PROGRAMS=deliverquota maildirmake testmaildirfilter maildirkwtest \
	maildirkw maildiracl maildiraclttest

deliverquota_SOURCES=deliverquota.c
deliverquota_DEPENDENCIES=libmaildir.a ../rfc822/librfc822.la \
				../numlib/libnumlib.a
deliverquota_LDADD=libmaildir.a ../rfc822/librfc822.la ../numlib/libnumlib.a

maildirmake_SOURCES=maildirmake.c
maildirmake_DEPENDENCIES=libmaildir.a ../numlib/libnumlib.a \
			../rfc822/librfc822.la
maildirmake_LDADD=libmaildir.a ../numlib/libnumlib.a \
			../rfc822/librfc822.la

testmaildirfilter_SOURCES=maildirfiltertypelist.h testmaildirfilter.c
testmaildirfilter_DEPENDENCIES=libmaildir.a ../numlib/libnumlib.a
testmaildirfilter_LDADD=libmaildir.a ../numlib/libnumlib.a


maildirkwtest_SOURCES=maildirkwtest.c
maildirkwtest_LDADD=libmaildir.a
maildirkwtest_DEPENDENCIES=libmaildir.a

maildirkw_SOURCES=maildirkw.c
maildirkw_LDADD=libmaildir.a ../liblock/liblock.a ../numlib/libnumlib.a \
		`cat maildir.libdeps`
maildirkw_DEPENDENCIES=libmaildir.a ../liblock/liblock.a ../numlib/libnumlib.a\
		maildir.libdeps

maildiracl=maildiracl.c
maildiracl_LDADD=libmaildir.a ../liblock/liblock.a ../numlib/libnumlib.a \
			`cat maildir.libdeps`
maildiracl_DEPENDENCIES=libmaildir.a ../liblock/liblock.a \
		../numlib/libnumlib.a maildir.libdeps

maildiraclttest_SOURCES=testmaildiraclt.c
maildiraclttest_LDADD=libmaildir.a ../liblock/liblock.a ../numlib/libnumlib.a \
			`cat maildir.libdeps`
maildiraclttest_DEPENDENCIES=libmaildir.a ../liblock/liblock.a \
		../numlib/libnumlib.a maildir.libdeps

EXTRA_DIST=README.maildirquota.html README.maildirquota.txt \
	README.imapkeywords.html \
	README.maildirfilter.html \
	README.sharedfolders.html README.sharedfolders.txt \
	maildirkwtest.txt \
	testsuite testsuite.txt quotawarnmsg $(DOCS)

HTML2TXT=`which lynx 2>/dev/null && echo "lynx -dump -nolist" && exit; echo "links -dump"`

README.maildirquota.txt: README.maildirquota.html
	$(HTML2TXT) README.maildirquota.html >README.maildirquota.txt

README.sharedfolders.txt: README.sharedfolders.html
	$(HTML2TXT) README.sharedfolders.html >README.sharedfolders.txt

mailbot.h: config.status
	echo '#define MAILBOT "@MAILBOT@"' >mailbot.h

maildirsharedrc.h: config.status
	echo '#define MAILDIRSHAREDRC "$(sysconfdir)/maildirshared"' >maildirsharedrc.h

maildirfilterconfig.h: config.status
	echo '#define MAILDIRFILTERCONFIG "$(sysconfdir)/maildirfilterconfig"' >maildirfilterconfig.h

autoresponsequota.h: config.status
	echo '#define AUTORESPONSEQUOTA "$(sysconfdir)/autoresponsesquota"' >autoresponsequota.h

quotawarnmsg.h: config.status
	echo '#define QUOTAWARNMSG "$(sysconfdir)/quotawarnmsg"' >quotawarnmsg.h
maildir.libdeps: config.status
	echo @LIBFAM@ >maildir.libdeps

clean-local:
	rm -rf maildir.libdeps testmd

check-am:
	@SHELL@ $(srcdir)/testsuite 2>&1 | cmp - $(srcdir)/testsuite.txt
	LC_ALL=C; export LC_ALL; ./maildirkwtest | cmp -s - $(srcdir)/maildirkwtest.txt
	LC_ALL=C; export LC_ALL; ./maildiraclttest

if HAVE_SGML

deliverquota.html.in: deliverquota.sgml ../docbook/sgml2html
	../docbook/sgml2html deliverquota.sgml deliverquota.html.in

deliverquota.8.in: deliverquota.sgml ../docbook/sgml2html
	../docbook/sgml2man deliverquota.sgml deliverquota.8.in

maildirmake.html.in: maildirmake.sgml ../docbook/sgml2html
	../docbook/sgml2html maildirmake.sgml maildirmake.html.in

maildirmake.1.in: maildirmake.sgml ../docbook/sgml2html
	../docbook/sgml2man maildirmake.sgml maildirmake.1.in

maildiracl.html.in: maildiracl.sgml ../docbook/sgml2html
	../docbook/sgml2html maildiracl.sgml maildiracl.html.in

maildiracl.1.in: maildiracl.sgml ../docbook/sgml2html
	../docbook/sgml2man maildiracl.sgml maildiracl.1.in

maildirquota.html: maildirquota.sgml ../docbook/sgml2html
	../docbook/sgml2html maildirquota.sgml maildirquota.html

maildirquota.7: maildirquota.sgml ../docbook/sgml2man
	../docbook/sgml2man maildirquota.sgml maildirquota.7

maildir.html: maildir.sgml ../docbook/sgml2html
	../docbook/sgml2html maildir.sgml maildir.html

maildir.5: maildir.sgml ../docbook/sgml2man
	../docbook/sgml2man maildir.sgml maildir.5

maildirkw.html: maildirkw.sgml ../docbook/sgml2html
	../docbook/sgml2html maildirkw.sgml maildirkw.html

maildirkw.1: maildirkw.sgml ../docbook/sgml2man
	../docbook/sgml2man maildirkw.sgml maildirkw.1

endif

deliverquota.html: deliverquota.html.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

deliverquota.8: deliverquota.8.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

maildirmake.html: maildirmake.html.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

maildirmake.1: maildirmake.1.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

maildiracl.html: maildiracl.html.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

maildiracl.1: maildiracl.1.in
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

# autoresponse.c: autoresponsequota.h

DISTCLEANFILES=$(BUILT_SOURCES)
