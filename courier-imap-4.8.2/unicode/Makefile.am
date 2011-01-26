# $Id: Makefile.am,v 1.31 2011/01/22 22:07:04 mrsam Exp $
#
# Copyright 2000-2011 Double Precision, Inc.  See COPYING for
# distribution information.


noinst_SCRIPTS=update.sh \
	mkultcase.pl unicodefilter.pl \
	mkeastasianwidth.pl
noinst_PROGRAMS=unicodetest

update-www:
	@$(MAKE) update-www-unicode
	@$(MAKE) update-www-eastasia

update-www-unicode:
	@SHELL@ $(srcdir)/update.sh UnicodeData.txt http://www.unicode.org/Public/UNIDATA/UnicodeData.txt

update-www-eastasia:
	@SHELL@ $(srcdir)/update.sh Unihan-3.2.0.txt.gz http://www.unicode.org/Public/3.2-Update/Unihan-3.2.0.txt.gz
	@SHELL@ $(srcdir)/update.sh EastAsianWidth.txt http://www.unicode.org/Public/UNIDATA/EastAsianWidth.txt

noinst_LTLIBRARIES=libunicode.la

libunicode_la_SOURCES=unicode.h unicode.c \
			unicodecpp.C \
			unicode_ultcase.c \
			unicode_ultcasetab.c \
			unicode_wcwidth.c eastasianwidth.h

EXTRA_DIST=$(noinst_SCRIPTS)

if UPDATE_UNICODE

unicode_ultcasetab.c: UnicodeData.txt mkultcase.pl
	@PERL@ $(srcdir)/mkultcase.pl >unicode_ultcasetab.c

eastasianwidth.h: EastAsianWidth.txt mkeastasianwidth.pl
	@PERL@ $(srcdir)/mkeastasianwidth.pl >eastasianwidth.h
endif

unicodetest_SOURCES=unicodetest.c
unicodetest_DEPENDENCIES=libunicode.la
unicodetest_LDADD=libunicode.la
unicodetest_LDFLAGS=-static

check-am: unicodetest
	./unicodetest
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