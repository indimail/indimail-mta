# $Id: Makefile.am 6909 2010-07-03 08:49:08Z m-a $ 

# Tell BSD make(1) to not remake the .1in and .htmlin files.
SUFFIXES= .1 .1in .htmlin .html

# requires:
#   xmllint (libxml2)
#   xmlto (cyberelk.net)
#   nsgmls (opensp)

# what to build
man_MANS = bogofilter.1 bogoutil.1 bogoupgrade.1 bogolexer.1 bogotune.1 \
	   bf_copy.1 bf_tar.1 bf_compact.1

html_MANS = bogofilter.html bogoutil.html bogoupgrade.html \
	    bogolexer.html bogotune.html \
	    bf_copy.html bf_tar.html bf_compact.html

all: $(man_MANS) $(html_MANS)

CLEANFILES = $(man_MANS) $(html_MANS)

HTML_DOCS = bogofilter-faq.html bogofilter-faq-fr.html bogofilter-faq-it.html bogotune-faq.html

# what to distribute
EXTRA_DIST = \
	     bogofilter.xml bogofilter.1in bogofilter.htmlin \
	     bogolexer.xml bogolexer.1in bogolexer.htmlin \
	     bogotune.xml bogotune.1in bogotune.htmlin \
	     bogoupgrade.xml bogoupgrade.1in bogoupgrade.htmlin \
	     bogoutil.xml bogoutil.1in bogoutil.htmlin \
	     bf_compact.xml bf_compact.1in bf_compact.htmlin \
	     bf_copy.xml bf_copy.1in bf_copy.htmlin \
	     bf_tar.xml bf_tar.1in bf_tar.htmlin \
	     $(HTML_DOCS) \
	     bogofilter-tuning.HOWTO.html \
	     bogofilter-SA-2002-01 bogofilter-SA-2004-01 \
	     bogofilter-SA-2005-01 bogofilter-SA-2005-02 \
	     bogofilter-SA-2010-01 \
	     integrating-with-postfix \
	     integrating-with-qmail \
	     programmer/OS2/configure.os2 \
	     programmer/README.freebsd programmer/README.hp-ux \
	     programmer/README.RISC-OS programmer/README.memory-checking \
	     programmer/README.osx \
	     programmer/RISC-OS/src/Makefile \
	     programmer/RISC-OS/gsl/Makefile \
	     rpm.notes.BerkeleyDB \
	     README.db README.sqlite \
	     README.validation

#	     bogofilter.cf.example 
#	     strlcat.3 strlcpy.3

.1in.1:
	sed -e "s|[@]sysconfdir@|$(sysconfdir)|g" \
		-e "s|[@]bindir@|$(bindir)|g" <$< >$@ \
	|| { rm -f $@ ; exit 1 ; }

# same as .1in.1: above, but POSIX doesn't allow two targets on one line.
.htmlin.html:
	sed -e "s|[@]sysconfdir@|$(sysconfdir)|g" \
		-e "s|[@]bindir@|$(bindir)|g" <$< >$@ \
	|| { rm -f $@ ; exit 1 ; }

if ENABLE_XMLTO
dist-hook:
	@if [ ! -f catalog ] ; then \
	echo ; echo "Warning: 'catalog' file not present; see 'README.validation'". ; echo ; else \
	case "$(MFLAGS)" in *s*) : ;; *) echo >&2 "validating $(HTML_DOCS)" ;; esac ; \
	( set -e ; for i in $(HTML_DOCS) ; do $(NSGMLS) -s $(srcdir)/$$i \
	    || { echo >&2 "ERROR: $$i does not validate." ; exit 1 ; } ; done ) ; \
	fi

# Use Perl to fix up br0ken 0xa0 characters that the XML DocBook style
# sheets emit instead of "\ " for roff or "&nbsp;" for HTML,
# respectively.
.xml.htmlin:
	xmllint --noout --postvalid $<
	if test -f $(srcdir)/$@ && test $(srcdir)/$@ -nt $< ; then \
	    cp $(srcdir)/$@ . ; else \
	  if xmlto html-nochunks $< ; then f=$@ ; mv $${f%%in} $@ ; else rm -f $@ ; exit 1 ; fi ; \
	  perl -pl -e "s|\xA0|&nbsp;|g" -i $@ || { rm -f $@ ; exit 1 ; } \
	fi

.xml.1in:
	xmllint --noout --postvalid $<
	if test -f $(srcdir)/$@ && test $(srcdir)/$@ -nt $< ; then \
	    cp $(srcdir)/$@ . ; else \
	if xmlto man $< ; then f=$@ ; mv $${f%in} $@ ; else rm -f $@ ; exit 1 ; fi ; \
	perl -pl -e "s|\xA0|\\ |g" -i $@ || { rm -f $@ ; exit 1 ; } \
	fi
else
dist-hook:
	true

.xml.1in:
	@echo 1>&2
	@echo 1>&2 "You need xmlto to build bogofilter documentation."
	@echo 1>&2
	@exit 1

.xml.htmlin:
	@echo 1>&2
	@echo 1>&2 "You need xmlto to build bogofilter documentation."
	@echo 1>&2
	@exit 1
endif

.PHONY: check
check:
	@if echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd"><HTML><HEAD><TITLE></TITLE></HEAD><BODY><P></P></BODY></HTML>' | $(NSGMLS) -s 2>/dev/null; then \
	set +e ; fail=0 ; for i in $(HTML_DOCS) ; do if $(NSGMLS) -s $(srcdir)/$$i ; then echo PASS: $$i ; else echo FAIL: $$i: ; fail=1 ; fi ; done ; \
	else echo >&2 "*** Not validating HTML, nsgmls/HTML DTDs improperly installed ***" ; fi ; \
	if test "$$fail" = "1" ; then echo >&2 "*** HTML document invalid ***" ; false ; fi
