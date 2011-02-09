# $Id: Makefile.am,v 1.96 2004/08/29 19:03:28 mrsam Exp $
#
# Copyright 1998 - 2002 Double Precision, Inc.  See COPYING for
# distribution information.

@SET_MAKE@

ACLOCAL_AMFLAGS=-I m4
TEMPLFILES=packaging/suse/courier-imap.init.in \
	packaging/suse/imap.pam \
	packaging/suse/pop3.pam

DIST_SUBDIRS=numlib bdbobj gdbmobj soxwrap unicode rfc822 rfc1035 rfc2045 \
        md5 sha1 random128 liblock liblog maildir makedat libhmac userdb authlib \
	waitlib tcpd imap
SUBDIRS=numlib @DBSUBDIR@ soxwrap unicode rfc822 rfc2045 random128 md5 sha1 \
	rfc1035 liblock liblog maildir makedat libhmac userdb authlib waitlib tcpd \
	imap

sbin_SCRIPTS=sharedindexinstall sharedindexsplit
CLEANFILES=$(databin_SCRIPTS) $(man_MANS) $(sysconf_DATA) $(sbin_SCRIPTS)
databindir=$(datadir)
databin_SCRIPTS=mkimapdcert mkpop3dcert vchkpw2userdb makeuserdb pw2userdb

binPROGRAMS=imapd pop3d maildirmake maildiracl deliverquota maildirkw
sbinPROGRAMS=authenumerate imaplogin pop3login userdbpw courierlogger
libexecPROGRAMS=makedatprog couriertcpd

bin_PROGRAMS= @binPROGRAMS_exec@
sbin_PROGRAMS=@sbinPROGRAMS_exec@
libexec_PROGRAMS=@libexecPROGRAMS_exec@

EXTRA_PROGRAMS=

man_MANS=makeuserdb.8 userdb.8 userdbpw.8 couriertcpd.1 courierlogger.1 deliverquota.8 maildirmake.1 maildiracl.1 imapd.8 mkimapdcert.8 mkpop3dcert.8 maildirkw.1
EXTRA_DIST=00README.NOW.OR.SUFFER README README.cygwin automake.fix \
	COPYING.GPL NEWS.html $(TEMPLFILES)

sysconf_DATA=imapd.dist imapd-ssl.dist pop3d.dist pop3d-ssl.dist \
	imapd.cnf pop3d.cnf quotawarnmsg.example
libexec_SCRIPTS=imapd.rc imapd-ssl.rc pop3d.rc pop3d-ssl.rc

HTML2TXT=`which lynx 2>/dev/null && echo "lynx -dump -nolist" && exit; echo "links -dump"`

NEWS: NEWS.html
	$(HTML2TXT) NEWS.html >NEWS

imaplogin$(EXEEXT): imap/imaplogin$(EXEEXT)
	cp imap/imaplogin$(EXEEXT) .

maildirkw$(EXEEXT): maildir/maildirkw$(EXEEXT)
	cp maildir/maildirkw$(EXEEXT) .

imapd$(EXEEXT): imap/imapd$(EXEEXT)
	cp imap/imapd$(EXEEXT) .

authenumerate$(EXEEXT): authlib/authenumerate$(EXEEXT)
	cp authlib/authenumerate$(EXEEXT) .

sharedindexinstall: maildir/sharedindexinstall
	cp maildir/sharedindexinstall .

sharedindexsplit: maildir/sharedindexsplit
	cp maildir/sharedindexsplit .

pop3login$(EXEEXT): imap/pop3login$(EXEEXT)
	cp imap/pop3login$(EXEEXT) .

pop3d$(EXEEXT): imap/pop3d$(EXEEXT)
	cp imap/pop3d$(EXEEXT) .

makedatprog$(EXEEXT): makedat/makedatprog$(EXEEXT)
	cp makedat/makedatprog$(EXEEXT) .

couriertcpd$(EXEEXT): tcpd/couriertcpd$(EXEEXT)
	cp tcpd/couriertcpd$(EXEEXT) .

courierlogger$(EXEEXT): liblog/courierlogger$(EXEEXT)
	cp liblog/courierlogger$(EXEEXT) .

makeuserdb: userdb/makeuserdb
	cp userdb/makeuserdb .

userdbpw: userdb/userdbpw
	cp userdb/userdbpw .

pw2userdb: userdb/pw2userdb
	cp userdb/pw2userdb .

vchkpw2userdb: userdb/vchkpw2userdb
	cp userdb/vchkpw2userdb vchkpw2userdb

maildirmake$(EXEEXT): maildir/maildirmake$(EXEEXT)
	cp maildir/maildirmake$(EXEEXT) .

maildiracl$(EXEEXT): maildir/maildiracl$(EXEEXT)
	cp maildir/maildiracl$(EXEEXT) .

deliverquota$(EXEEXT): maildir/deliverquota$(EXEEXT)
	cp maildir/deliverquota$(EXEEXT) .

pop3d.dist: imap/pop3d.dist
	cp imap/pop3d.dist .

pop3d-ssl.dist: imap/pop3d-ssl.dist
	cp imap/pop3d-ssl.dist .

imapd.dist: imap/imapd.dist
	cp imap/imapd.dist .

imapd-ssl.dist: imap/imapd-ssl.dist
	cp imap/imapd-ssl.dist .

makeuserdb.8: userdb/makeuserdb.8
	cp userdb/makeuserdb.8 .

userdb.8: userdb/userdb.8
	cp userdb/userdb.8 .

userdbpw.8: userdb/userdbpw.8
	cp userdb/userdbpw.8 .

couriertcpd.1: tcpd/couriertcpd.1
	cp $(srcdir)/tcpd/couriertcpd.1 .

courierlogger.1: liblog/courierlogger.1
	cp $(srcdir)/liblog/courierlogger.1 .

deliverquota.8: maildir/deliverquota.8
	cp maildir/deliverquota.8 .

maildirmake.1: maildir/maildirmake.1
	cp maildir/maildirmake.1 .

maildiracl.1: maildir/maildiracl.1
	cp maildir/maildiracl.1 .

maildirkw.1: maildir/maildirkw.1
	cp $(srcdir)/maildir/maildirkw.1 .

imapd.8: imap/imapd.8
	cp imap/imapd.8 .

mkimapdcert: imap/mkimapdcert
	cp imap/mkimapdcert .

mkpop3dcert: imap/mkpop3dcert
	cp imap/mkpop3dcert .

mkimapdcert.8: imap/mkimapdcert.8
	cp -f imap/mkimapdcert.8 .

mkpop3dcert.8: imap/mkpop3dcert.8
	cp -f imap/mkpop3dcert.8 .

imapd.cnf: imap/imapd.cnf
	cp imap/imapd.cnf .

pop3d.cnf: imap/pop3d.cnf
	cp imap/pop3d.cnf .

quotawarnmsg.example: maildir/quotawarnmsg
	cp -f $(srcdir)/maildir/quotawarnmsg quotawarnmsg.example

# Must manually copy over authlib modules

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(libexecdir)/authlib
	for f in `cat authlib/installlist` dummy ; do \
		test "$$f" = "authpam" && chown root:root \
		$(DESTDIR)$(libexecdir)/authlib/$$f &&
		chmod 4555 \ $(DESTDIR)$(libexecdir)/authlib/$$f ; \
		test "$$f" = "authshadow" && chown root:root \
		$(DESTDIR)$(libexecdir)/authlib/$$f &&
		chmod 4555 \ $(DESTDIR)$(libexecdir)/authlib/$$f ; \
		test "$$f" = "dummy" && continue ; \
		$(INSTALL_PROGRAM) $(INSTALL_STRIP_FLAG) authlib/$$f \
			$(DESTDIR)$(libexecdir)/authlib/$$f ; done
	for f in `cat authlib/installlist.sh` dummy ; do \
		test "$$f" = "dummy" && continue ; \
		$(INSTALL_SCRIPT) authlib/$$f \
			$(DESTDIR)$(libexecdir)/authlib/$$f ; done
	$(mkinstalldirs) $(DESTDIR)$(databindir)
	$(INSTALL_SCRIPT) userdb/userdb.pl $(DESTDIR)$(databindir)/userdb
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	test ! -x tcpd/couriertls || $(INSTALL_PROGRAM) $(INSTALL_STRIP_FLAG) \
		tcpd/couriertls $(DESTDIR)$(bindir)/couriertls
	$(mkinstalldirs) $(DESTDIR)$(sbindir)
	for f in mkimapdcert mkpop3dcert vchkpw2userdb makeuserdb pw2userdb userdb ; \
		do rm -f $(DESTDIR)$(sbindir)/$$f ; \
		$(LN_S) $(databindir)/$$f $(DESTDIR)$(sbindir)/$$f ; done
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)

install-exec-hook:
	chmod 444 $(DESTDIR)$(sysconfdir)/quotawarnmsg.example
	mkdir -p $(DESTDIR)$(sysconfdir)/shared
	chmod 755 $(DESTDIR)$(sysconfdir)/shared
	mkdir -p $(DESTDIR)$(sysconfdir)/shared.tmp
	chmod 755 $(DESTDIR)$(sysconfdir)/shared.tmp

# If authlib built authpam, install the PAM configuration file

install-data-local: install-man
	for f in `cat authlib/modulelist ; . authlib/authdaemonrc ; echo $$authmodulelist ` ; do \
		test "$$f" = "authpam" || continue ; \
		test -d $(DESTDIR)/etc/pam.d || continue ; \
		test -w $(DESTDIR)/etc/pam.d || continue ; \
		test -f $(DESTDIR)/etc/pam.d/imap || \
			$(INSTALL_DATA) $(srcdir)/imap/imapd.pam \
			$(DESTDIR)/etc/pam.d/imap || exit 1 ;\
		test -f $(DESTDIR)/etc/pam.d/pop3 || \
			$(INSTALL_DATA) $(srcdir)/imap/pop3d.pam \
			$(DESTDIR)/etc/pam.d/pop3 || exit 1 ;\
	done
	rm -f $(DESTDIR)$(man8dir)/pw2userdb.8
	rm -f $(DESTDIR)$(man8dir)/vchkpw2userdb.8
	$(LN_S) makeuserdb.8 $(DESTDIR)$(man8dir)/pw2userdb.8
	$(LN_S) makeuserdb.8 $(DESTDIR)$(man8dir)/vchkpw2userdb.8
	chmod 4755 $(DESTDIR)/$(prefix)/libexec/authlib/authshadow || true;chown root:root $(DESTDIR)/$(prefix)/man/man7/authlib.7 || true
	. authlib/authdaemonrc ; test "$$authdaemonvar" = "" && exit 0; \
		$(mkinstalldirs) $(DESTDIR)$$authdaemonvar ; \
		chmod 700 $(DESTDIR)$$authdaemonvar
	for f in $(man_MANS); do chmod a+r $(DESTDIR)$(mandir)/*/$$f; done
	@cat authlib/authdaemon.msg
	@echo "Do not forget to run make install-configure"

uninstall-local:
	rm -f $(DESTDIR)$(man8dir)/pw2userdb.8
	rm -f $(DESTDIR)$(man8dir)/vchkpw2userdb.8
	rm -f $(DESTDIR)$(databindir)/userdb
	for f in userdb mkimapdcert mkpop3dcert vchkpw2userdb makeuserdb pw2userdb ; \
		do rm -f $(DESTDIR)$(sbindir)/$$f ; done
	for f in `cat authlib/installlist authlib/installlist.sh` ; do \
			rm -f $(DESTDIR)$(libexecdir)/authlib/$$f ; done
	rm -f $(DESTDIR)$(bindir)/couriertls

INSTALL.txt: INSTALL.html
	$(HTML2TXT) INSTALL.html >INSTALL.txt

check:
	rm -rf `pwd`/=install-check
	$(MAKE) install DESTDIR=`pwd`/=install-check
	$(MAKE) uninstall DESTDIR=`pwd`/=install-check
	@files="`find =install-check ! -type d -print`"; \
		test "$$files" = "" && exit 0; \
	echo "---------------------------------------------------------" ; \
	echo "Warning:  I don't know how to clean up after myself!" ;\
	echo ""	;\
	echo "$$files" | tr ' ' '\012' ; \
	echo "" ;\
	echo "---------------------------------------------------------" ; \
	echo ""; echo "Send all flames to the address in the AUTHORS file."
#
# For maintainer's use only
#

dist-hook: courier-imap.spec
	test -f $(srcdir)/rpm.release && cp $(srcdir)/rpm.release $(distdir) \
		; exit 0
	rm -f $(distdir)/INSTALL
	$(HTML2TXT) INSTALL.html >$(distdir)/INSTALL
	cp $(srcdir)/courier-imap.spec $(distdir)

if HAVE_HTML

README: html/main.html
	$(HTML2TXT) $(srcdir)/html/main.html >README
endif

distrelease:
	socksify cvs update
	umask 022; $(MAKE) distcheck
	$(MAKE) rpm
	socksify cvs commit
	socksify cvs tag -F courier-imap-latest
	$(MAKE)
	$(MAKE)
	$(MAKE) release


rpm: build-rpm

build-rpm:
	$(MAKE) courier-imap.spec
	$(MAKE) bump.rpm.release
	$(MAKE) dist
	rm -rf rpm/BUILD/*
	rm -f rpm/RPMS/*/*
	rm -f rpm/SOURCES/*
	rm -f rpm/SPECS/*
	rm -f rpm/SRPMS/*
	unset MAKEFLAGS ; test ! -f $$HOME/.bashrc || . $$HOME/.bashrc; rpmbuild -ta courier-imap-@VERSION@.tar.bz2

release:
	rm -f courier-imap.spec
	$(MAKE) courier-imap.spec
	$(MAKE) dist
	unset MAKEFLAGS ; test ! -f $$HOME/.bashrc || . $$HOME/.bashrc; rpmbuild -ta courier-imap-@VERSION@.tar.bz2

bump.rpm.release:
	VERSION="x"; \
	test -f $(srcdir)/rpm.release && . $(srcdir)/rpm.release; \
	NEXT_RELEASE=0; \
	test "$$VERSION" = "@VERSION@" && NEXT_RELEASE="$$RELEASE"; \
	RELEASE=`expr $$NEXT_RELEASE + 1`; \
	echo "VERSION=@VERSION@" >$(srcdir)/rpm.release; \
	echo "RELEASE=$$RELEASE" >>$(srcdir)/rpm.release; \
	sed "s/\%define .*courier_release_no .*[0-9].*/\%define courier_release_no $$RELEASE/" \
		courier-imap.spec >courier-imap.spec.new
	mv courier-imap.spec.new courier-imap.spec

all-recursive: all-make-check

all-make-check:
	$(MAKE) all-gmake-check FOO=BAR
	@test -f authlib/authvchkpw.flag || exit 0; cat authlib/authvchkpw.flag

all-gmake-check:
	@test "$(FOO)" = "BAR" && exit 0 ; echo "" ; \
		echo "ERROR: Something's wrong.  If you're using xBSD, you must use gmake" ; \
		echo "instead of make, as you were told by the INSTALL file (you did read the" ; \
		echo "INSTALL file, did you?)" ; echo "" ; exit 1

# Added by sysconftoolize: $Id: Makefile.am,v 1.96 2004/08/29 19:03:28 mrsam Exp $

# Copyright 2000 Double Precision, Inc.  See COPYING for
# distribution information.

# Need to add sysconftoolize to package:

EXTRA_DIST += sysconftool

# NOTE: when using a complex directory hierarchy it is only necessary to
# include sysconftool in the top level directory, and it is not necessary
# to add sysconftool to EXTRA_DIST in subdirectories.

SYSCONFFILES=$(sysconf_DATA)

# SYSCONFFILES should be all of your .dist files, this is a default.
# You can also change install-configure-local target below, instead.

# If -- in a complex directory hierarchy -- a certain subdirectory does not
# need install-configure, leave it defined as an empty rule to prevent
# sysconftoolize from appending another form template to this Makefile.am

install-configure:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		( case "$$subdir" in .) continue;; \
			*) target=install-configure;; esac ; \
		cd $$subdir ; $(MAKE) install-configure DESTDIR=$(DESTDIR) 2>/dev/null) \
			; done ; exit 0
	$(MAKE) install-configure-local DESTDIR=$(DESTDIR)

# Modify the following target as appropriate:

install-configure-local:
	@list="$(SYSCONFFILES) `cat authlib/authconfiglist`"; \
		for file in $$list; do \
		file="$(DESTDIR)$(sysconfdir)/$$file" ; \
		case "$$file" in *.dist) \
			@SYSCONFTOOL@ $$file;; esac ; done
upload-beta:
	rm -f @PACKAGE@-@VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE@-@VERSION@.tar.bz2; test -f @PACKAGE@-@VERSION@.tar.bz2.sig || exit 1
	socksify scp @PACKAGE@-@VERSION@.tar.bz2 @PACKAGE@-@VERSION@.tar.bz2.sig courier.sourceforge.net:/home/groups/c/co/courier/htdocs/beta/imap/.
	socksify ssh courier.sourceforge.net echo @PACKAGE@-@VERSION@.tar.bz2 \>/home/groups/c/co/courier/courier-imap.beta

upload-dist:
	rm -f @PACKAGE@-@VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE@-@VERSION@.tar.bz2; test -f @PACKAGE@-@VERSION@.tar.bz2.sig || exit 1
	socksify cvs rtag -F -r courier-imap-latest \
           `echo courier-imap-@VERSION@ | tr '.' '-'` courier-imap
	socksify perl -e 'use Net::FTP; $$ftp=Net::FTP->new("upload.sourceforge.net.", Debug => 1, Passive => 1 ) || die; $$ftp->login("anonymous", "anonymous") || die; $$ftp->cwd("/incoming") && $$ftp->binary() && $$ftp->hash(1) && $$ftp->put("@PACKAGE@-@VERSION@.tar.bz2") && $$ftp->put("@PACKAGE@-@VERSION@.tar.bz2.sig") && exit 0; die;'

release-dist:
	test -z "$(SFPASSWD)" && exit 1; exit 0
	./changelog.pl >changelog.txt
	emacs releasenotes.txt
	test -f releasenotes.txt || exit 1
	socksify ../sftools/release.pl mrsam $(SFPASSWD) courier imap @VERSION@ releasenotes.txt changelog.txt @PACKAGE@-@VERSION@.tar.bz2 @PACKAGE@-@VERSION@.tar.bz2.sig
	socksify ssh courier.sourceforge.net rm -f /home/groups/c/co/courier/courier-imap.beta

upload-www:
	rm -f html.tar.gz & cd html && tar czf ../html.tar.gz `find . -name CVS -prune -o ! -name '*~' ! -name '.' -print`
	socksify scp html.tar.gz courier.sourceforge.net:
	socksify ssh courier.sourceforge.net 'cd /home/groups/c/co/courier/htdocs/imap && tar xzvf ~/html.tar.gz && rm -f ~/html.tar.gz'

htmldoc: INSTALL.html imap/README.html authlib/authlib.html maildir/README.maildirquota.html maildir/README.sharedfolders.html maildir/README.imapkeywords.html imap/BUGS.html
	echo '/^<head/a'"\\" >htmldoc.sed
	echo '<link href="style.css" rel="stylesheet" type="text/css" />'"\\" >>htmldoc.sed
	echo '<link rel="icon" href="icon.gif" type="image/gif" />'"\\" >>htmldoc.sed
	echo '<script type="text/javascript" language="Javascript" src="frame.js"></script>' >>htmldoc.sed
	for f in INSTALL.html maildir/README.*.html imap/BUGS.html imap/README.html; do b=`basename $$f`; h=`perl -ne ' print $$1 if /\\$$Id: Makefile.am,v 1.96 2004/08/29 19:03:28 mrsam Exp $$]*)\\$$/' <html/$$b`; sed -f htmldoc.sed <$$f | perl -ne 's~\$$Id: Makefile.am,v 1.96 2004/08/29 19:03:28 mrsam Exp $$~\$$Id: Makefile.am,v 1.96 2004/08/29 19:03:28 mrsam Exp $$h"'\$$~; print' >html/$$b; done
	echo '/^<link rel/a'"\\" >htmldoc.sed
	echo '<link href="style.css" rel="stylesheet" type="text/css" />'"\\" >>htmldoc.sed
	echo '<script type="text/javascript" language="Javascript" src="frame.js"></script>' >>htmldoc.sed
	for f in authlib/authlib.html; do b=`basename $$f`; h=`perl -ne ' print $$1 if /\\$$Id: Makefile.am,v 1.96 2004/08/29 19:03:28 mrsam Exp $$]*)\\$$/' <html/$$b`; sed -f htmldoc.sed <$$f | perl -ne 's~\$$Id: Makefile.am,v 1.96 2004/08/29 19:03:28 mrsam Exp $$~\$$Id: Makefile.am,v 1.96 2004/08/29 19:03:28 mrsam Exp $$h"'\$$~; print' >html/$$b; done

AUTOMAKE_OPTIONS=dist-bzip2
