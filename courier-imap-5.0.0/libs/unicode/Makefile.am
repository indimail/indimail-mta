#
# Copyright 2000-2017 Double Precision, Inc.  See COPYING for
# distribution information.

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4

AM_CXXFLAGS= @COURIER_UNICODE_CXXFLAGS@

noinst_SCRIPTS=update.sh \
	mkultcase.pl unicodefilter.pl \
	mkeastasianwidth.pl \
	mkgraphemebreak.pl mklinebreak.pl mkwordbreak.pl mkcommon.pm \
	mkhtmlent.pl

noinst_PROGRAMS=unicodetest graphemetest linebreaktest wordbreaktest \
	enttest scripttest

aclocaldir=$(datadir)/aclocal
aclocal_DATA=m4/courier-unicode.m4

update-www:
	@$(MAKE) update-www-unicode
	@$(MAKE) update-www-eastasia
	@$(MAKE) update-www-grapheme
	@$(MAKE) update-www-linebreak
	@$(MAKE) update-www-wordbreak
	@$(MAKE) update-www-scripts
	@$(MAKE) update-www-htmlent
	@$(MAKE) update-www-categories

update-www-unicode:
	@SHELL@ $(srcdir)/update.sh UnicodeData.txt http://www.unicode.org/Public/UNIDATA/UnicodeData.txt

update-www-eastasia:
	@SHELL@ $(srcdir)/update.sh Unihan.zip http://www.unicode.org/Public/UNIDATA/Unihan.zip
	@SHELL@ $(srcdir)/update.sh EastAsianWidth.txt http://www.unicode.org/Public/UNIDATA/EastAsianWidth.txt

update-www-grapheme:
	@SHELL@ $(srcdir)/update.sh GraphemeBreakProperty.txt http://www.unicode.org/Public/UNIDATA/auxiliary/GraphemeBreakProperty.txt

update-www-linebreak:
	@SHELL@ $(srcdir)/update.sh LineBreak.txt http://www.unicode.org/Public/UNIDATA/LineBreak.txt
	@SHELL@ $(srcdir)/update.sh LineBreakTest.txt http://www.unicode.org/Public/UNIDATA/auxiliary/LineBreakTest.txt

update-www-wordbreak:
	@SHELL@ $(srcdir)/update.sh WordBreakProperty.txt http://www.unicode.org/Public/UNIDATA/auxiliary/WordBreakProperty.txt
	@SHELL@ $(srcdir)/update.sh WordBreakTest.txt http://www.unicode.org/Public/UNIDATA/auxiliary/WordBreakTest.txt

update-www-scripts:
	@SHELL@ $(srcdir)/update.sh Scripts.txt http://unicode.org/Public/UNIDATA/Scripts.txt

update-www-htmlent:
	@SHELL@ $(srcdir)/update.sh xhtml-lat1.ent http://www.w3.org/TR/xhtml1/DTD/xhtml-lat1.ent
	@SHELL@ $(srcdir)/update.sh xhtml-symbol.ent http://www.w3.org/TR/xhtml1/DTD/xhtml-symbol.ent
	@SHELL@ $(srcdir)/update.sh xhtml-special.ent http://www.w3.org/TR/xhtml1/DTD/xhtml-special.ent

update-www-categories:
	@SHELL@ $(srcdir)/update.sh Categories.txt http://www.unicode.org/notes/tn36/Categories.txt

lib_LTLIBRARIES=libunicode.la
include_HEADERS=courier-unicode.h \
	courier-unicode-categories-tab.h \
	courier-unicode-script-tab.h

man_MANS=$(srcdir)/man/courier-unicode.7 $(srcdir)/man/unicode[\:][\:]iconvert[\:][\:]convert.3 $(srcdir)/man/unicode[\:][\:]iconvert[\:][\:]convert_tocase.3 $(srcdir)/man/unicode[\:][\:]iconvert[\:][\:]fromu.3 $(srcdir)/man/unicode[\:][\:]iconvert[\:][\:]tou.3 $(srcdir)/man/unicode[\:][\:]iso_8859_1.3 $(srcdir)/man/unicode[\:][\:]linebreak_callback_base.3 $(srcdir)/man/unicode[\:][\:]linebreak_callback_save_buf.3 $(srcdir)/man/unicode[\:][\:]linebreak_iter.3 $(srcdir)/man/unicode[\:][\:]linebreakc_callback_base.3 $(srcdir)/man/unicode[\:][\:]linebreakc_iter.3 $(srcdir)/man/unicode[\:][\:]tolower.3 $(srcdir)/man/unicode[\:][\:]toupper.3 $(srcdir)/man/unicode[\:][\:]ucs_2.3 $(srcdir)/man/unicode[\:][\:]ucs_4.3 $(srcdir)/man/unicode[\:][\:]utf_8.3 $(srcdir)/man/unicode[\:][\:]wordbreak_callback_base.3 $(srcdir)/man/unicode_category_lookup.3 $(srcdir)/man/unicode_convert.3 $(srcdir)/man/unicode_convert_deinit.3 $(srcdir)/man/unicode_convert_fromu_init.3 $(srcdir)/man/unicode_convert_fromu_tobuf.3 $(srcdir)/man/unicode_convert_fromutf8.3 $(srcdir)/man/unicode_convert_init.3 $(srcdir)/man/unicode_convert_tobuf.3 $(srcdir)/man/unicode_convert_tocase.3 $(srcdir)/man/unicode_convert_tocbuf_fromutf8_init.3 $(srcdir)/man/unicode_convert_tocbuf_init.3 $(srcdir)/man/unicode_convert_tocbuf_toutf8_init.3 $(srcdir)/man/unicode_convert_tou_init.3 $(srcdir)/man/unicode_convert_tou_tobuf.3 $(srcdir)/man/unicode_convert_toutf8.3 $(srcdir)/man/unicode_convert_uc.3 $(srcdir)/man/unicode_default_chset.3 $(srcdir)/man/unicode_grapheme_break.3 $(srcdir)/man/unicode_html40ent_lookup.3 $(srcdir)/man/unicode_isalnum.3 $(srcdir)/man/unicode_isalpha.3 $(srcdir)/man/unicode_isblank.3 $(srcdir)/man/unicode_isdigit.3 $(srcdir)/man/unicode_isgraph.3 $(srcdir)/man/unicode_islower.3 $(srcdir)/man/unicode_ispunct.3 $(srcdir)/man/unicode_isspace.3 $(srcdir)/man/unicode_isupper.3 $(srcdir)/man/unicode_lb_end.3 $(srcdir)/man/unicode_lb_init.3 $(srcdir)/man/unicode_lb_next.3 $(srcdir)/man/unicode_lb_next_cnt.3 $(srcdir)/man/unicode_lb_set_opts.3 $(srcdir)/man/unicode_lbc_end.3 $(srcdir)/man/unicode_lbc_init.3 $(srcdir)/man/unicode_lbc_next.3 $(srcdir)/man/unicode_lbc_next_cnt.3 $(srcdir)/man/unicode_lbc_set_opts.3 $(srcdir)/man/unicode_lc.3 $(srcdir)/man/unicode_locale_chset.3 $(srcdir)/man/unicode_script.3 $(srcdir)/man/unicode_tc.3 $(srcdir)/man/unicode_u_ucs2_native.3 $(srcdir)/man/unicode_u_ucs4_native.3 $(srcdir)/man/unicode_uc.3 $(srcdir)/man/unicode_wb_end.3 $(srcdir)/man/unicode_wb_init.3 $(srcdir)/man/unicode_wb_next.3 $(srcdir)/man/unicode_wb_next_cnt.3 $(srcdir)/man/unicode_wbscan_end.3 $(srcdir)/man/unicode_wbscan_init.3 $(srcdir)/man/unicode_wbscan_next.3

libunicode_la_SOURCES=\
			courier-unicode-categories-tab.h \
			courier-unicode-script-tab.h \
			unicode.c unicodebuf.c \
			unicodecpp.C \
			unicode_ultcase.c \
			unicode_ultcasetab.c \
			unicode_wcwidth.c eastasianwidth.h \
			unicode_tablookup.c \
			unicode_graphemebreak.c graphemebreaktab.h \
			unicode_linebreak.c linebreaktab.h \
			unicode_htmlent.c unicode_htmlent.h \
			linebreaktab_internal.h \
			unicode_wordbreak.c wordbreaktab.h scriptstab.h \
			wordbreaktab_internal.h \
			unicode_scripts.c \
			categoriestab.h \
			unicode_categories.c

libunicode_la_LDFLAGS=-version-info 5:0:1

EXTRA_DIST=$(noinst_SCRIPTS) $(man_MANS) $(PACKAGE).spec \
	m4/courier-unicode.m4

if UPDATE_UNICODE

unicode_ultcasetab.c: UnicodeData.txt mkultcase.pl
	@PERL@ $(srcdir)/mkultcase.pl >unicode_ultcasetab.c

eastasianwidth.h: EastAsianWidth.txt mkeastasianwidth.pl
	@PERL@ $(srcdir)/mkeastasianwidth.pl >eastasianwidth.h

graphemebreaktab.h: GraphemeBreakProperty.txt mkgraphemebreak.pl mkcommon.pm
	@PERL@ $(srcdir)/mkgraphemebreak.pl >graphemebreaktab.h.tmp
	mv graphemebreaktab.h.tmp graphemebreaktab.h

linebreaktab.h: LineBreak.txt UnicodeData.txt mklinebreak.pl mkcommon.pm
	@PERL@ $(srcdir)/mklinebreak.pl >linebreaktab.h.tmp
	mv linebreaktab.h.tmp linebreaktab.h

wordbreaktab.h: WordBreakProperty.txt mkwordbreak.pl mkcommon.pm
	@PERL@ $(srcdir)/mkwordbreak.pl >wordbreaktab.h.tmp
	mv wordbreaktab.h.tmp wordbreaktab.h

scriptstab.h: Scripts.txt mkscripts.pl mkcommon.pm
	@PERL@ $(srcdir)/mkscripts.pl >scriptstab.h.tmp
	mv scriptstab.h.tmp scriptstab.h

categoriestab.h: Categories.txt mkcategories.pl mkcommon.pm
	@PERL@ $(srcdir)/mkcategories.pl >categoriestab.h.tmp
	mv categoriestab.h.tmp categoriestab.h

unicode_htmlent.h: xhtml-lat1.ent xhtml-special.ent xhtml-symbol.ent mkhtmlent.pl
	@PERL@ $(srcdir)/mkhtmlent.pl >unicode_htmlent.h.tmp
	mv unicode_htmlent.h.tmp unicode_htmlent.h
endif

unicodetest_SOURCES=unicodetest.c
unicodetest_DEPENDENCIES=libunicode.la
unicodetest_LDADD=libunicode.la
unicodetest_LDFLAGS=-static

graphemetest_SOURCES=graphemetest.c
graphemetest_DEPENDENCIES=libunicode.la
graphemetest_LDADD=libunicode.la
graphemetest_LDFLAGS=-static

linebreaktest_SOURCES=linebreaktest.C
linebreaktest_DEPENDENCIES=libunicode.la
linebreaktest_LDADD=libunicode.la
linebreaktest_LDFLAGS=-static

wordbreaktest_SOURCES=wordbreaktest.C
wordbreaktest_DEPENDENCIES=libunicode.la
wordbreaktest_LDADD=libunicode.la
wordbreaktest_LDFLAGS=-static

enttest_SOURCES=enttest.c
enttest_DEPENDENCIES=libunicode.la
enttest_LDADD=libunicode.la
enttest_LDFLAGS=-static

scripttest_SOURCES=scripttest.c
scripttest_DEPENDENCIES=libunicode.la
scripttest_LDADD=libunicode.la
scripttest_LDFLAGS=-static

check-am: unicodetest
	./unicodetest
	./enttest
	set -vx; test -f ./LineBreakTest.txt || exit 0; ./linebreaktest
	set -vx; test -f ./WordBreakTest.txt || exit 0; ./wordbreaktest
	test "`./unicodetest foo`" = "foo"
	test "`./unicodetest 'foo&bar.~'`" = "foo&-bar.~"
	test "`./unicodetest 'foobаr'`" = "foob&BDA-r"
	test "`./unicodetest 'foobааr'`" = "foob&BDAEMA-r"
	test "`./unicodetest 'foobаааr'`" = "foob&BDAEMAQw-r"
	test "`./unicodetest 'foobааааr'`" = "foob&BDAEMAQwBDA-r"
	test "`./unicodetest 'foobаааааr'`" = "foob&BDAEMAQwBDAEMA-r"
	test "`./unicodetest 'foobааааааr'`" = "foob&BDAEMAQwBDAEMAQw-r"
	test "`./unicodetest 'foobаaаr'`" = "foob&BDA-a&BDA-r"
	test "`./unicodetest 'foobааaааr'`" = "foob&BDAEMA-a&BDAEMA-r"
	test "`./unicodetest --smaputf8 'hello world'`" = 'hello world'
	test "`./unicodetest --smaputf8 'hello\\\\world'`" = 'hello\134\134world'
	test "`./unicodetest --smaputf8 ':hello:world:'`" = '\072hello\072world\072'
	n="aaaaaaaa"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n"; n="`echo $$n | cut -c1-1023`"; test "`./unicodetest $$n`" = "$$n"
	n="aaaaaaaa"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n"; test "`./unicodetest $$n`" = "$$n"
	n="aaaaaaaa"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n"; test "`./unicodetest a$$n`" = "a$$n"
	n="aaaaaaaa"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n"; n="`echo $$n | cut -c1-1023`"; test "`./unicodetest $$n'&'`" = "$$n""&-"
	n="aaaaaaaa"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n"; test "`./unicodetest $$n'&'`" = "$$n""&-"
	n="aaaaaaaa"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n"; test "`./unicodetest $$n'a&'`" = "$$n""a&-"
	n="aaaaaaaa"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n"; test "`./unicodetest $$n'a&a'`" = "$$n""a&-a"
	n="aaaaaaaa"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n$$n$$n"; n="$$n$$n"; n="`echo $$n | cut -c1-1023`"; test "`./unicodetest $$n'а'`" = "$$n""&BDA-"
	test "`./unicodetest --smap foo`" = "foo"
	test "`./unicodetest --smap 'foo&bar'`" = 'foo&-bar'
	test "`./unicodetest --smap 'foo.bar'`" = 'foo&AC4-bar'
	test "`./unicodetest --totitle 'tÄst'`" = 'Täst'
	test "`./unicodetest --modutf7toutf8 'foo&bar'`" = '[error]'
	test "`./unicodetest --modutf7toutf8 'foo&-bar'`" = 'foo&bar'
	test "`./unicodetest --modutf7toutf8 'foo:bar'`" = 'foo\072bar'
	test "`./graphemetest 0x0d 0x0a`" = "0" # GB3
	test "`./graphemetest 0x0d 0x41`" = "1" # GB4
	test "`./graphemetest 0x41 0x0d`" = "1" # GB5
	test "`./graphemetest 0x0a 0x41`" = "1" # GB4
	test "`./graphemetest 0x41 0x0a`" = "1" # GB5
	test "`./graphemetest 0x09 0x41`" = "1" # GB4
	test "`./graphemetest 0x41 0x09`" = "1" # GB5
	test "`./graphemetest 0x1100 0x1101`" = "0"  # GB6: L x L
	test "`./graphemetest 0x1100 0x1160`" = "0"  # GB6: L x V
	test "`./graphemetest 0xAC00 0x1160`" = "0"  # GB7: LV x V
	test "`./graphemetest 0xAC02 0xD7FB`" = "0"  # GB8: LVT x T
	test "`./graphemetest 0xD7FB 0xD7FB`" = "0"  # GB8: T x T
	test "`./graphemetest 0x1F1E6 0x1F1FF`" = "0"  # GB8a
	test "`./graphemetest 0x0041 0x1101`" = "1"
	test "`./graphemetest 0x0041 0x1160`" = "1"
	test "`./graphemetest 0x0041 0x1160`" = "1"
	test "`./graphemetest 0x0041 0xD7FB`" = "1"
	test "`./graphemetest 0x0041 0xD7FB`" = "1"
	test "`./graphemetest 0x1100 0x0041`" = "1"
	test "`./graphemetest 0x1100 0x0041`" = "1"
	test "`./graphemetest 0xAC00 0x0041`" = "1"
	test "`./graphemetest 0xAC02 0x0041`" = "1"
	test "`./graphemetest 0xD7FB 0x0041`" = "1"
	test "`./graphemetest 0x0041 0x036F`" = "0"  # GB9
	test "`./graphemetest 0x0041 0x0903`" = "0"  # GB9a
	test "`./graphemetest 0x0041 0xAABB`" = "1"
	test "`./graphemetest 0x036F 0x0041`" = "1"
	test "`./graphemetest 0x0903 0x0041`" = "1"
	exit 0; test "`./graphemetest 0xAABB 0x0041`" = "0" # GB9b N/A
	test "`./graphemetest 0x0041 0x0041`" = "1"

docs:
	rm -rf html.tmp
	mkdir html.tmp
	xsltproc --nonet -o html.tmp/ --xinclude \
		--stringparam use.id.as.filename 1 \
		--stringparam html.stylesheet book.css \
		--stringparam root.filename 'index' \
		--stringparam generate.id.attributes 1 \
		$(srcdir)/docbook/docbook.xsl $(srcdir)/book.xml
	for f in html.tmp/*; do tidy -i -q --tidy-mark no --doctype transitional --input-encoding utf8 --output-encoding utf8 -asxml <$$f >$$f.tmp; xsltproc --nonet $$HOME/www/www.courier-mta.org/xsl/unicode.xsl $$f.tmp | perl -ne 's~\$(id):.*\$$~\$(id):'"$$h"'\$$~; print' >$$f; rm -f $$f.tmp; done

	mkdir -p html
	rm -f html/*
	mv html.tmp/* html
	cp -p docbook/frame.js docbook/icon.gif docbook/book.css html
	rm -rf man.tmp
	mkdir man.tmp
	d=`cd $(srcdir); pwd`; cd man.tmp; xsltproc --nonet --xinclude \
	http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl\
		$$d/book.xml
	mkdir -p man
	rm -f man/*.[123456789]
	mv man.tmp/* man
	rm -rf html.tmp man.tmp
	perl -p -i -e 's/:/[\\:]/g if s@^man_MANS=.*@"man_MANS=" . join(" ", map { "\$$(srcdir)/$$_" } glob("man/*.[123456789]"))@e' Makefile.am

if HAVE_DOCS
all-am: INSTALL README

README: html/index.html
	elinks -dump -no-numbering -no-references html/index.html >README.tmp
	mv -f README.tmp README

INSTALL: html/INSTALL.html
	elinks -dump -no-numbering -no-references html/INSTALL.html >INSTALL.tmp
	mv -f INSTALL.tmp INSTALL
endif

.PHONY: rpm

rpm:
	$(MAKE) bump.rpm.release
	$(MAKE) dist
	$(MAKE) dorpm
	$$HOME/repos.sh

dorpm:
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
		$(RPMFLAGS) @PACKAGE@-@VERSION@.tar.bz2

bump.rpm.release: $(PACKAGE).spec
	VERSION="x"; \
	test -f $(srcdir)/rpm.release && . $(srcdir)/rpm.release; \
	NEXT_RELEASE=0; \
	test "$$VERSION" = "@VERSION@" && NEXT_RELEASE="$$RELEASE"; \
	RELEASE=`expr $$NEXT_RELEASE + 1`; \
	echo "VERSION=@VERSION@" >$(srcdir)/rpm.release; \
	echo "RELEASE=$$RELEASE" >>$(srcdir)/rpm.release; \
	sed 's/^Release: .*/Release: '$$RELEASE'%{?dist}%{?courier_release}/' \
		<$(PACKAGE).spec >$(PACKAGE).spec.new; \
	mv $(PACKAGE).spec.new $(PACKAGE).spec

upload-beta:
	rm -f @PACKAGE@-@VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE@-@VERSION@.tar.bz2; test -f @PACKAGE@-@VERSION@.tar.bz2.sig || exit 1
	~/src/sftools/release.pl mrsam courier @PACKAGE@-devel `echo @VERSION@|sed 's/.*\(........\)/\1/'` @PACKAGE@-@VERSION@.tar.bz2 @PACKAGE@-@VERSION@.tar.bz2.sig

upload-dist:
	rm -f @PACKAGE@-@VERSION@.tar.bz2.sig; gpg --detach-sign --default-key mrsam@courier-mta.com @PACKAGE@-@VERSION@.tar.bz2; test -f @PACKAGE@-@VERSION@.tar.bz2.sig || exit 1
	set -e; v="@PACKAGE@/@VERSION@/`date +%Y%m%d%H%M%S`"; git tag -s $$v -m 'Tag version @VERSION@'
	socksify ~/src/sftools/release.pl mrsam courier $(PACKAGE) @VERSION@ @PACKAGE@-@VERSION@.tar.bz2 @PACKAGE@-@VERSION@.tar.bz2.sig

distrelease:
	$(MAKE) distcheck
	$(MAKE) rpm
	$(MAKE) -$(MAKEFLAGS)
	$(MAKE) -$(MAKEFLAGS)
	rm -f courier-unicode.spec
	$(MAKE) courier-unicode.spec
	$(MAKE) dist

www:
	rsync -a html/. $$HOME/www/www.courier-mta.org/unicode
