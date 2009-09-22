# $Id: Makefile.am,v 1.25 2008/07/20 16:24:52 mrsam Exp $
#
# Copyright 2000-2008 Double Precision, Inc.  See COPYING for
# distribution information.


CLEANFILES=charsetlist.c

noinst_SCRIPTS=mkiso8859.pl mkkoi8r.pl mkkoi8u.pl mkibm864.pl big5.pl gb2312.pl \
	update.sh \
	mkultcase.pl unicodefilter.pl charsetlist.pl mkiso8859.pl \
	iso2022jp.pl ksx1001.pl mkeastasianwidth.pl
noinst_PROGRAMS=unicode-info

unicode_info_SOURCES=unicodeinfo.c
unicode_info_DEPENDENCIES=libunicode.a
unicode_info_LDADD=libunicode.a

update-www:
	@$(MAKE) update-www-unicode
	@$(MAKE) update-www-iso8859
	@$(MAKE) update-www-koi8r
	@$(MAKE) update-www-koi8u
	@$(MAKE) update-www-windows
	@$(MAKE) update-www-dos
	@$(MAKE) update-www-eastasia

update-www-unicode:
	@SHELL@ $(srcdir)/update.sh UnicodeData.txt http://www.unicode.org/Public/UNIDATA/UnicodeData.txt

update-www-iso8859:
	for n in 1 2 3 4 5 6 7 8 9 10 13 14 15 ; do \
		@SHELL@ $(srcdir)/update.sh 8859-$$n.txt \
		http://www.unicode.org/Public/MAPPINGS/ISO8859/8859-$$n.TXT \
		|| exit 1 ; done

update-www-koi8r:
	@SHELL@ $(srcdir)/update.sh koi8-r.txt \
		http://www.unicode.org/Public/MAPPINGS/VENDORS/MISC/KOI8-R.TXT

update-www-koi8u:
	@SHELL@ $(srcdir)/update.sh koi8-u.txt \
		http://svn.python.org/projects/python/trunk/Tools/unicode/python-mappings/KOI8-U.TXT

update-www-windows:
	for n in 874 1250 1251 1252 1253 1254 1255 1256 1257 1258 ; do \
		@SHELL@ $(srcdir)/update.sh windows-$$n.txt \
			http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP$$n.TXT \
			|| exit 1 ; done

update-www-dos:
	for n in 437 775 850 852 855 857 860 861 862 863 864 865 866 869 ; do \
		@SHELL@ $(srcdir)/update.sh ibm-$$n.txt \
			http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP$$n.TXT \
			|| exit 1 ; done

update-www-eastasia:
	@SHELL@ $(srcdir)/update.sh Unihan-3.2.0.txt.gz http://www.unicode.org/Public/3.2-Update/Unihan-3.2.0.txt.gz
	@SHELL@ $(srcdir)/update.sh CP949.TXT http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP949.TXT
	@SHELL@ $(srcdir)/update.sh big5-iso.txt http://www.info.gov.hk/digital21/chi/hkscs/download/big5-iso.txt
	@SHELL@ $(srcdir)/update.sh EastAsianWidth.txt http://www.unicode.org/Public/UNIDATA/EastAsianWidth.txt

ISO8859CFILES=iso88591.c iso88592.c iso88593.c iso88594.c iso88595.c\
	iso88596.c iso88597.c iso88598.c iso88599.c iso885910.c iso885913.c\
	iso885914.c iso885915.c

WINDOWSCFILES=windows1250.c windows1251.c windows1252.c windows1253.c\
	windows1254.c windows1255.c windows1256.c windows1257.c windows1258.c\
	windows874.c

IBMCFILES=ibm437.c ibm775.c ibm850.c ibm852.c ibm855.c ibm857.c ibm860.c \
	ibm861.c ibm862.c ibm863.c ibm864.c ibm865.c ibm866.c ibm869.c

noinst_LIBRARIES=libunicode.a

libunicode_a_SOURCES=unicode.h iso8859.c windows874u.c unicode.c unicode2.c \
			iso8859_convert.c ibm864convert.c \
			utf7imap.c xtest.c \
			unicode_ultcase.c utf8.c utf8_chset.c utf7.c koi8r.c koi8u.c \
			unicode_ultcasetab.c $(ISO8859CFILES) $(WINDOWSCFILES) \
			$(IBMCFILES) big5.c big5.h gb2312.c gb2312.h \
			iso2022jp.c iso2022jp.h shiftjis.c eucjp.c \
			ksx1001.h ksx1001.c \
			unicode_wcwidth.c eastasianwidth.h \
			charsetlist.c

EXTRA_DIST=$(noinst_SCRIPTS) $(ISO8859CFILES) charsetlist.txt

charsetlist.c: charsetlist.txt charsetlist.pl Makefile config.status
	@PERL@ $(srcdir)/unicodefilter.pl "@unicode@" \
			"@unicode_xtest@" <${srcdir}/charsetlist.txt | \
		@PERL@ $(srcdir)/charsetlist.pl >charsetlist.c

dist-hook:
	rm -f $(distdir)/charsetlist.c

if UPDATE_UNICODE
MAKEISO8859CFILES=dummy1
MAKEKOIR8=dummy2
MAKEKOIU8=dummy3

update-csrc:
	if test -f UnicodeData.txt ; then $(MAKE) "MAKEISO8859CFILES=$(ISO8859CFILES)" ISO8859OFILES=dummy MAKEKOIR8=koi8r.c MAKEKOIU8=koi8u.c $(ISO8859CFILES) koi8r.c koi8u.c ; fi

$(ISO8859CFILES): UnicodeData.txt mkiso8859.pl
	@n=`echo $@ | cut -c8- | sed 's/..$$//'` ; \
		echo @PERL@ $(srcdir)/mkiso8859.pl 8859-$${n}.txt iso8859$${n}_unicode iso8859$${n}_uc iso8859$${n}_lc iso8859$${n}_tc unicode_ISO8859_$${n} ISO-8859-$${n} '>'$@; \
		@PERL@ $(srcdir)/mkiso8859.pl 8859-$${n}.txt iso8859$${n}_unicode iso8859$${n}_uc iso8859$${n}_lc iso8859$${n}_tc unicode_ISO8859_$${n} ISO-8859-$${n} >$@ && exit 0; rm -f $@

$(WINDOWSCFILES): UnicodeData.txt mkiso8859.pl
	@n=`echo $@ | tr -d -c '[0-9]'` ; \
		echo @PERL@ $(srcdir)/mkiso8859.pl windows-$${n}.txt windows$${n}_unicode windows$${n}_uc windows$${n}_lc windows$${n}_tc unicode_WINDOWS_$${n} WINDOWS-$${n} '>'$@; \
		@PERL@ $(srcdir)/mkiso8859.pl windows-$${n}.txt windows$${n}_unicode windows$${n}_uc windows$${n}_lc windows$${n}_tc unicode_WINDOWS_$${n} WINDOWS-$${n} >$@ && exit 0; rm -f $@

$(IBMCFILES): UnicodeData.txt mkiso8859.pl mkibm864.pl
	@n=`echo $@ | cut -c4-6` ; s=$(srcdir)/mkiso8859.pl; \
		if test "$$n" = 864 ; then s=$(srcdir)/mkibm864.pl; fi; \
		echo @PERL@ $$s ibm-$${n}.txt ibm$${n}_unicode ibm$${n}_uc ibm$${n}_lc ibm$${n}_tc unicode_IBM_$${n} IBM$${n} '>'$@; \
		@PERL@ $$s ibm-$${n}.txt ibm$${n}_unicode ibm$${n}_uc ibm$${n}_lc ibm$${n}_tc unicode_IBM_$${n} IBM$${n} >$@ && exit 0; rm -f $@

$(MAKEKOIR8): UnicodeData.txt mkkoi8r.pl
	@PERL@ $(srcdir)/mkkoi8r.pl >koi8r.c

$(MAKEKOIU8): UnicodeData.txt mkkoi8u.pl
	@PERL@ $(srcdir)/mkkoi8u.pl >koi8u.c

unicode_ultcasetab.c: UnicodeData.txt mkultcase.pl
	@PERL@ $(srcdir)/mkultcase.pl >unicode_ultcasetab.c

big5.h: Unihan-3.2.0.txt.gz big5.pl big5-iso.txt
	@PERL@ $(srcdir)/big5.pl >big5.h

gb2312.h: Unihan-3.2.0.txt.gz gb2312.pl
	@PERL@ $(srcdir)/gb2312.pl >gb2312.h

iso2022jp.h: Unihan-3.2.0.txt.gz iso2022jp.pl
	@PERL@ $(srcdir)/iso2022jp.pl > iso2022jp.h

ksx1001.h: CP949.TXT ksx1001.pl
	@PERL@ $(srcdir)/ksx1001.pl > ksx1001.h

eastasianwidth.h: EastAsianWidth.txt mkeastasianwidth.pl
	@PERL@ $(srcdir)/mkeastasianwidth.pl >eastasianwidth.h
endif
