#
# Copyright 1998 - 2008 Double Precision, Inc.  See COPYING for
# distribution information.

@SET_MAKE@
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

TEMPLFILES=packaging/suse/courier-imap.init.in \
	packaging/suse/imap.pam \
	packaging/suse/pop3.pam

SUBDIRS=numlib md5 sha1 bdbobj gdbmobj soxwrap unicode rfc822 random128 \
	rfc1035 rfc2045 liblock maildir makedat libhmac authlib \
	waitlib tcpd rootcerts imap

sbin_SCRIPTS=sharedindexinstall sharedindexsplit
CLEANFILES=$(databin_SCRIPTS) $(man_MANS) $(sysconf_DATA) $(sbin_SCRIPTS)
databindir=$(datadir)
databin_SCRIPTS=mkimapdcert mkpop3dcert

binPROGRAMS=imapd pop3d maildirmake maildiracl deliverquota maildirkw
sbinPROGRAMS=imaplogin pop3login
libexecPROGRAMS=makedatprog couriertcpd

bin_PROGRAMS= @binPROGRAMS_exec@
sbin_PROGRAMS=@sbinPROGRAMS_exec@
libexec_PROGRAMS=@libexecPROGRAMS_exec@

EXTRA_PROGRAMS=

man_MANS=couriertcpd.1 deliverquota.8 maildirmake.1 maildiracl.1 imapd.8 mkimapdcert.8 mkpop3dcert.8 maildirkw.1
EXTRA_DIST=README README.cygwin\
	COPYING.GPL NEWS.html $(TEMPLFILES) pgpkeys.txt

sysconf_DATA=imapd.dist imapd-ssl.dist pop3d.dist pop3d-ssl.dist \
	imapd.cnf pop3d.cnf quotawarnmsg.example
libexec_SCRIPTS=imapd.rc imapd-ssl.rc pop3d.rc pop3d-ssl.rc

HTML2TXT=links -dump -no-numbering -no-references

NEWS: NEWS.html
	$(HTML2TXT) NEWS.html >NEWS

imaplogin$(EXEEXT): imap/imaplogin$(EXEEXT)
	cp imap/imaplogin$(EXEEXT) .

maildirkw$(EXEEXT): maildir/maildirkw$(EXEEXT)
	cp maildir/maildirkw$(EXEEXT) .

imapd$(EXEEXT): imap/imapd$(EXEEXT)
	cp imap/imapd$(EXEEXT) .

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

couriertcpd.1: tcpd/couriertcpd.1
	cp $(srcdir)/tcpd/couriertcpd.1 .

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

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(libexecdir)/authlib
	for f in `cat authlib/installlist` dummy ; do \
		test "$$f" = "dummy" && continue ; \
		$(INSTALL_PROGRAM) $(INSTALL_STRIP_FLAG) authlib/$$f \
			$(DESTDIR)$(libexecdir)/authlib/$$f ; done
	$(mkinstalldirs) $(DESTDIR)$(databindir)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	test ! -x tcpd/couriertls || $(INSTALL_PROGRAM) $(INSTALL_STRIP_FLAG) \
		tcpd/couriertls $(DESTDIR)$(bindir)/couriertls
	$(mkinstalldirs) $(DESTDIR)$(sbindir)
	for f in mkimapdcert mkpop3dcert ; \
		do rm -f $(DESTDIR)$(sbindir)/$$f ; \
		$(LN_S) $(databindir)/$$f $(DESTDIR)$(sbindir)/$$f ; done
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)

install-exec-hook:
	chmod 444 $(DESTDIR)$(sysconfdir)/quotawarnmsg.example
	mkdir -p $(DESTDIR)$(sysconfdir)/shared
	chmod 755 $(DESTDIR)$(sysconfdir)/shared
	mkdir -p $(DESTDIR)$(sysconfdir)/shared.tmp
	chmod 755 $(DESTDIR)$(sysconfdir)/shared.tmp

install-data-local: install-man
	for f in `cat authlib/modulelist ; echo $$authmodulelist ` ; do \
		test "$$f" = "authpam" || test "$$f" = "authshadow" || continue ; \
		chmod 4555 $(DESTDIR)/$(prefix)/libexec/authlib/$$f || exit 1; \
	done
	test -d $(DESTDIR)/etc/pam.d || exit 0 ; \
	test -w $(DESTDIR)/etc/pam.d || exit 0 ; \
	test -f $(DESTDIR)/etc/pam.d/imap || \
		$(INSTALL_DATA) $(srcdir)/imap/imapd.pam \
		$(DESTDIR)/etc/pam.d/imap || exit 1 ;\
	test -f $(DESTDIR)/etc/pam.d/pop3 || \
		$(INSTALL_DATA) $(srcdir)/imap/pop3d.pam \
		$(DESTDIR)/etc/pam.d/pop3 || exit 1
	for f in $(man_MANS); do chmod a+r $(DESTDIR)$(mandir)/*/$$f; done
	@echo "Do not forget to run make install-configure"

uninstall-local:
	for f in mkimapdcert mkpop3dcert ; \
		do rm -f $(DESTDIR)$(sbindir)/$$f ; done
	rm -f $(DESTDIR)$(bindir)/couriertls

INSTALL.txt: INSTALL.html
	$(HTML2TXT) INSTALL.html >INSTALL.txt

#
# For maintainer's use only
#

dist-hook: courier-imap.spec courier-imap.lpspec
	test -f $(srcdir)/rpm.release && cp $(srcdir)/rpm.release $(distdir) \
		; exit 0
	rm -f $(distdir)/INSTALL
	$(HTML2TXT) INSTALL.html >$(distdir)/INSTALL
	cp $(srcdir)/courier-imap.spec $(srcdir)/courier-imap.lpspec $(distdir)

if HAVE_HTML

README: html/main.html
	$(HTML2TXT) $(srcdir)/html/index2.html >README
endif

distrelease:
	umask 022; $(MAKE) distcheck
	$(MAKE) rpm
	$(MAKE)
	$(MAKE)
	$(MAKE) release

release: dist

rpm: build-rpm

build-rpm:
	$(MAKE) courier-imap.spec courier-imap.lpspec
	$(MAKE) bump.rpm.release
	$(MAKE) dist
	rm -rf rpm/BUILD/*
	rm -f rpm/RPMS/*/*
	rm -f rpm/SOURCES/*
	rm -f rpm/SPECS/*
	rm -f rpm/SRPMS/*
	rpmbuild -ta --clean \
		--define "_topdir `pwd`/rpm" \
		--define '_rpmdir %{_topdir}/RPMS' \
		--define '_srcrpmdir %{_topdir}/SRPMS' \
		--define '_sourcedir %{_topdir}/SOURCES' \
		--define '_specdir %{_topdir}/SPECS' \
		--define '_builddir %{_topdir}/BUILD' \
		--define '_build_name_fmt %%{ARCH}/%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm' \
		--define '_tmppath %{_var}/tmp' \
		--define '__spec_prep_pre %{___build_pre}' \
		@PACKAGE@-@VERSION@.tar.bz2

lpm: lpm-build

lpm-build:
	$(MAKE) -$(MAKEFLAGS) bump.rpm.release
	$(MAKE) -$(MAKEFLAGS) dist
	rm -rf lpm
	mkdir lpm
	lpbuild --srcbuild -o lpm @PACKAGE@-@VERSION@.tar.bz2

publish:
	lpbuild --resign lpm/*
	rm -rf lpmtmp
	mkdir lpmtmp
	ln lpm/* lpmtmp
	mv lpmtmp/*.src.lp repository/`lpm -q --distribution`/sources
	mv lpmtmp/*.lp repository/`lpm -q --distribution`/packages
	rm -rf lpmtmp
	lpm --sync repository && rsync -a -v --delete-after repository/. headache:lpmtool.com/www/repositories/courier-imap

bump.rpm.release: @PACKAGE@.spec @PACKAGE@.lpspec
	VERSION="x"; \
	test -f $(srcdir)/rpm.release && . $(srcdir)/rpm.release; \
	NEXT_RELEASE=0; \
	test "$$VERSION" = "@VERSION@" && NEXT_RELEASE="$$RELEASE"; \
	RELEASE=`expr $$NEXT_RELEASE + 1`; \
	echo "VERSION=@VERSION@" >$(srcdir)/rpm.release; \
	echo "RELEASE=$$RELEASE" >>$(srcdir)/rpm.release; \
	sed 's/^Release: .*/Release: '$$RELEASE'%{courier_release}/' \
		courier-imap.spec >courier-imap.spec.new ; \
	mv courier-imap.spec.new courier-imap.spec ; \
	sed 's/^Release: .*/Release: '$$RELEASE'/' \
		courier-imap.lpspec >courier-imap.lpspec.new
	mv courier-imap.lpspec.new courier-imap.lpspec

all-recursive: all-make-check

all-make-check:
	$(MAKE) all-gmake-check FOO=BAR

all-gmake-check:
	@test "$(FOO)" = "BAR" && exit 0 ; echo "" ; \
		echo "ERROR: Something's wrong.  If you're using xBSD, you must use gmake" ; \
		echo "instead of make, as you were told by the INSTALL file (you did read the" ; \
		echo "INSTALL file, did you?)" ; echo "" ; exit 1


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
	@list="$(SYSCONFFILES)"; \
		for file in $$list; do \
		file="$(DESTDIR)$(sysconfdir)/$$file" ; \
		case "$$file" in *.dist) \
			@SYSCONFTOOL@ $$file;; esac ; done
upload-beta:
	test -z "$(SFPASSWD)" && exit 1; exit 0
	rm -f @PACKAGE@-@VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE@-@VERSION@.tar.bz2; test -f @PACKAGE@-@VERSION@.tar.bz2.sig || exit 1
	../sftools/release.pl mrsam $(SFPASSWD) courier imap-devel `echo @VERSION@|sed 's/.*\(........\)/\1/'` @PACKAGE@-@VERSION@.tar.bz2 @PACKAGE@-@VERSION@.tar.bz2.sig

upload-dist:
	test -z "$(SFPASSWD)" && exit 1; exit 0
	rm -f @PACKAGE@-@VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE@-@VERSION@.tar.bz2; test -f @PACKAGE@-@VERSION@.tar.bz2.sig || exit 1
	r="`svn info . | sed -n '/Repository Root:/p' | cut -d: -f2-`"; \
	  svn rm -m 'Remove previous tag' $$r/tags/@PACKAGE@/@VERSION@ 2>/dev/null; \
	  svn cp -m 'Tag @PACKAGE@' $$r/trunk $$r/tags/@PACKAGE@/@VERSION@
	socksify ../sftools/release.pl mrsam $(SFPASSWD) courier imap @VERSION@ @PACKAGE@-@VERSION@.tar.bz2 @PACKAGE@-@VERSION@.tar.bz2.sig

dollar=$$
id=$(dollar)Id

WWW=$$HOME/www/www.courier-mta.org

htmldoc: INSTALL.html imap/README.html maildir/README.maildirquota.html maildir/README.sharedfolders.html maildir/README.imapkeywords.html imap/BUGS.html imap/README.proxy.html imap/README.html
	for f in $^; do xsltproc --nonet $(WWW)/xsl/imap.xsl $$f | perl -ne 's~\$(id):.*\$$~\$(id):'"$$h"'\$$~; print' >$(WWW)/imap/`basename $$f`; done
	rm -rf doc
	mkdir doc
	xsltproc -o doc/ $(srcdir)/imap.xsl $(srcdir)/imap.xml
	mv doc/index.html doc/index2.html
	for f in doc/*.html; do \
		echo $$f; \
		tidy -i -asxml --doctype transitional \
			--input-encoding utf8 --output-encoding ascii \
			-q --tidy-mark no $$f >$$f.tmp; \
		xsltproc --nonet $(WWW)/xsl/postproc.xsl \
			$$f.tmp > $$f.tmp2; \
		xsltproc --nonet $(WWW)/xsl/imap.xsl \
			 $$f.tmp2 >$$f; \
		rm -f $$f.tmp*; done
	mv -f doc/*.html $(WWW)/imap
	rm -rf doc

AUTOMAKE_OPTIONS=dist-bzip2
