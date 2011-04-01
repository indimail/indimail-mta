# $Id: Makefile.am,v 1.37 2011/02/27 16:28:35 mrsam Exp $
#
# Copyright 2000-2011 Double Precision, Inc.  See COPYING for
# distribution information.


noinst_SCRIPTS=update.sh \
	mkultcase.pl unicodefilter.pl \
	mkeastasianwidth.pl \
	mkgraphemebreak.pl mklinebreak.pl mkcommon.pm

noinst_PROGRAMS=unicodetest graphemetest linebreaktest

update-www:
	@$(MAKE) update-www-unicode
	@$(MAKE) update-www-eastasia
	@$(MAKE) update-www-grapheme
	@$(MAKE) update-www-linebreak

update-www-unicode:
	@SHELL@ $(srcdir)/update.sh UnicodeData.txt http://www.unicode.org/Public/UNIDATA/UnicodeData.txt

update-www-eastasia:
	@SHELL@ $(srcdir)/update.sh Unihan-3.2.0.txt.gz http://www.unicode.org/Public/3.2-Update/Unihan-3.2.0.txt.gz
	@SHELL@ $(srcdir)/update.sh EastAsianWidth.txt http://www.unicode.org/Public/UNIDATA/EastAsianWidth.txt

update-www-grapheme:
	@SHELL@ $(srcdir)/update.sh GraphemeBreakProperty.txt http://www.unicode.org/Public/UNIDATA/auxiliary/GraphemeBreakProperty.txt

update-www-linebreak:
	@SHELL@ $(srcdir)/update.sh LineBreak.txt http://www.unicode.org/Public/UNIDATA/LineBreak.txt
	@SHELL@ $(srcdir)/update.sh LineBreakTest.txt http://www.unicode.org/Public/UNIDATA/auxiliary/LineBreakTest.txt

noinst_LTLIBRARIES=libunicode.la

libunicode_la_SOURCES=unicode.h unicode.c \
			unicodecpp.C \
			unicode_ultcase.c \
			unicode_ultcasetab.c \
			unicode_wcwidth.c eastasianwidth.h \
			unicode_tablookup.c \
			unicode_graphemebreak.c graphemebreaktab.h \
			unicode_linebreak.c linebreaktab.h \
			linebreaktab_internal.h

EXTRA_DIST=$(noinst_SCRIPTS)

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

check-am: unicodetest
	./unicodetest
	test -f ./LineBreakTest.txt || exit 0; ./linebreaktest
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
	test "`./graphemetest 0x0d 0x0a`" = "0"
	test "`./graphemetest 0x0d 0x41`" = "1"
	test "`./graphemetest 0x41 0x0d`" = "1"
	test "`./graphemetest 0x0a 0x41`" = "1"
	test "`./graphemetest 0x41 0x0a`" = "1"
	test "`./graphemetest 0x09 0x41`" = "1"
	test "`./graphemetest 0x41 0x09`" = "1"
	test "`./graphemetest 0x1100 0x1101`" = "0"
	test "`./graphemetest 0x1100 0x1160`" = "0"
	test "`./graphemetest 0xAC00 0x1160`" = "0"
	test "`./graphemetest 0xAC02 0xD7FB`" = "0"
	test "`./graphemetest 0xD7FB 0xD7FB`" = "0"
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
	test "`./graphemetest 0x0041 0x036F`" = "0"
	test "`./graphemetest 0x0041 0x0903`" = "0"
	test "`./graphemetest 0x0041 0xAABB`" = "1"
	test "`./graphemetest 0x036F 0x0041`" = "1"
	test "`./graphemetest 0x0903 0x0041`" = "1"
	test "`./graphemetest 0xAABB 0x0041`" = "0"
	test "`./graphemetest 0x0041 0x0041`" = "1"


