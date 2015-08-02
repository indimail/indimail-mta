# vim:tw=0
# $Id: Makefile.am 6993 2013-06-28 21:54:23Z m-a $

SUBDIRS = . tests
SYSCONFDIR = @sysconfdir@

PERL = @PERL@

if ENABLE_MEMDEBUG
MEMDEBUG_SRC = memdebug.c memdebug.h
endif

noinst_LIBRARIES = libbogofilter.a

if USE_GCC4
AM_CPPFLAGS = -iquote$(top_srcdir)/gnugetopt -iquote$(top_srcdir)/trio \
	      -I$(srcdir) @GSL_CFLAGS@ -I$(top_srcdir)/gsl/specfunc -I$(top_srcdir)
else
AM_CPPFLAGS = -I$(top_srcdir)/gnugetopt -I$(top_srcdir)/trio -I- -I. \
	      -I$(srcdir) @GSL_CFLAGS@ -I$(top_srcdir)/gsl/specfunc -I$(top_srcdir)
endif

AM_CFLAGS = -DBOGOFILTER

if ENABLE_MEMDEBUG
AM_CFLAGS += -DENABLE_MEMDEBUG
endif

BUILT_SOURCES=	version.c directories.c

# what to build
bin_PROGRAMS = bogofilter bogoutil bogolexer bogotune
bin_SCRIPTS = bogoupgrade
dist_sbin_SCRIPTS = bf_copy bf_compact bf_tar
if ENABLE_STATIC
bin_PROGRAMS += bogofilter_static bogoutil_static bogolexer_static bogotune_static
endif

check_PROGRAMS = debugtest configtest wordhash find_home.test \
		 fgetsl.test

TESTS=

LDADD = libbogofilter.a
bogofilter_LDADD = $(LDADD) $(LIBDB) $(GSL_LIBS)
bogoutil_LDADD = $(LDADD) $(LIBDB)
configtest_LDADD = $(LDADD) $(LIBDB)

if NEED_GSL
libbf_gsl_a_CFLAGS=$(AM_CFLAGS) -DRENAMEFILES
libbf_gsl_a_SOURCES= \
../gsl/cdf/chisq.c ../gsl/cdf/gamma.c \
../gsl/specfunc/gamma_inc.c ../gsl/err/error.c \
../gsl/sys/infnan.c ../gsl/sys/fdiv.c \
../gsl/specfunc/exp.c ../gsl/specfunc/trig.c \
../gsl/specfunc/log.c ../gsl/specfunc/psi.c \
../gsl/specfunc/zeta.c ../gsl/specfunc/elementary.c \
../gsl/err/stream.c ../gsl/specfunc/expint.c \
../gsl/specfunc/erfc.c ../gsl/sys/coerce.c \
../gsl/sys/pow_int.c ../gsl/sys/minmax.c
noinst_LIBRARIES += libbf_gsl.a
libbf_gsl_a_CPPFLAGS= -I$(top_srcdir)
BUILT_SOURCES += libbf_gsl.a
endif

EXTRA_PROGRAMS=panicenv

panicenv_SOURCES=panicenv.c
panicenv_LDADD=$(LIBDB)

if NEEDTRIO
noinst_LIBRARIES += libtrio.a
libtrio_a_SOURCES= ../trio/triostr.c ../trio/trio.c ../trio/trionan.c \
		   ../trio/trio.h ../trio/triop.h ../trio/triodef.h \
		   ../trio/trionan.h ../trio/triostr.h
check_PROGRAMS += regression
TESTS+=./regression
regression_SOURCES= ../trio/regression.c
LDADD += libtrio.a
endif

if !DISABLE_UNICODE
LDADD += @LIBICONV@
endif

if ENABLE_UNICODE
CHARSET_SOURCES = charset.c charset.h convert_unicode.c iconvert.h iconvert.c
else
if DISABLE_UNICODE
CHARSET_SOURCES = charset.c charset.h convert_charset.c chUnicodeTo866.h
else
CHARSET_SOURCES = charset.c charset.h \
		  convert_charset.c convert_charset.h chUnicodeTo866.h \
		  convert_unicode.c convert_unicode.h iconvert.c iconvert.h
endif
endif

# what to build that from
version_sources= \
	common.h system.h bftypes.h \
	globals.h globals.c \
	base64.h base64.c \
	bf_exit.c \
	bogoconfig.h bogoconfig.c \
	bogomain.h bogomain.c \
	bogoreader.h bogoreader.c \
	bool.h bool.c \
	bsdqueue.h \
	buff.h buff.c \
	$(CHARSET_SOURCES) \
	collect.h collect.c \
	configfile.h configfile.c \
	datastore.h datastore.c \
	datastore_dbcommon.h datastore_db_private.h \
	db_lock.h db_lock.c \
	debug.h debug.c \
	error.h error.c \
	fgetsl.h fgetsl.c \
	find_home.h find_home.c find_home_user.c find_home_tildeexpand.c \
	format.h format.c \
	lexer.h lexer.c lexer_v3.l \
	listsort.h listsort.c \
	longoptions.h \
	maint.h maint.c \
	memstr.h memstr.c \
	mime.h mime.c \
	msgcounts.h msgcounts.c \
	mxcat.h mxcat.c \
	passthrough.h passthrough.c \
	paths.h paths.c \
	prob.h prob.c \
	qp.h qp.c \
	rand_sleep.h rand_sleep.c \
	register.h register.c \
	robx.h robx.c \
	rstats.h rstats.c \
	score.h score.c \
	sighandler.h sighandler.c \
	swap.h swap_32bit.c system.c \
	textblock.h textblock.c \
	token.h token.c \
	transaction.h transaction.c \
	uudecode.h uudecode.c \
	word.h word.c \
	wordhash.h wordhash.c wordlists.h wordlists.c \
	wordlists_base.h wordlists_base.c \
	xmalloc.h xcalloc.c xmalloc.c xmem_error.c xrealloc.c \
	xmemrchr.h xmemrchr.c \
	xstrdup.h xstrdup.c \
	xstrlcat.h xstrlcat.c \
	xstrlcpy.h xstrlcpy.c \
	$(MEMDEBUG_SRC) \
	xatox.h xatof.c xatoi.c \
	getopt_long_chk.c \
	datastore_db.h

libbogofilter_a_SOURCES = \
	version.c \
	$(version_sources)

nodist_libbogofilter_a_SOURCES = \
	directories.c \
	unicode_runtime_selection unicode_enabled unicode_disabled

if NEED_GETOPT
noinst_LIBRARIES	+= libgnugetopt.a
LDADD			+= libgnugetopt.a
libgnugetopt_a_SOURCES	 = ../gnugetopt/getopt1.c ../gnugetopt/getopt.c
endif

# this must be last so any library we may have added has access to the
# AC_REPLACE objects, for instance, trio may need strtoul:
LDADD += @LIBOBJS@

if ENABLE_QDBM_DATASTORE
datastore_SOURCE = datastore_qdbm.c datastore_qdbm_cmpkey.c \
		   datastore_txn_dummies.c datastore_opthelp_dummies.c \
		   datastore_dummies.c

bin_PROGRAMS += bogoQDBMupgrade
bogoQDBMupgrade_SOURCES = bogoQDBMupgrade.c datastore_qdbm_cmpkey.c
bogoQDBMupgrade_LDADD = $(LDADD) $(LIBDB)
else
if ENABLE_SQLITE_DATASTORE
datastore_SOURCE = datastore_sqlite.c datastore_opthelp_dummies.c \
		   datastore_dummies.c
else
if ENABLE_TOKYOCABINET_DATASTORE
datastore_SOURCE = datastore_tc.c \
		   datastore_opthelp_dummies.c \
		   datastore_dummies.c
else
if ENABLE_TRANSACTIONS
datastore_SOURCE = datastore_db.c datastore_db_trans.c
else
if DISABLE_TRANSACTIONS
datastore_SOURCE = datastore_db.c datastore_db_trad.c \
		   datastore_opthelp_dummies.c
else
datastore_SOURCE = datastore_db.c datastore_db_trad.c datastore_db_trans.c
endif
endif
endif
endif
endif

datastore_OBJECT = $(datastore_SOURCE:.c=.o)

libbogofilter_a_SOURCES += $(datastore_SOURCE)
EXTRA_libbogofilter_a_SOURCES = datastore_db.c datastore_db_trad.c \
				datastore_db_trans.c \
				datastore_opthelp_dummies.c \
				datastore_txn_dummies.c \
				datastore_qdbm.h datastore_qdbm.c \
				datastore_sqlite.c \
				charset.h convert_charset.h convert_unicode.h \
				charset.c \
				convert_charset.c chUnicodeTo866.h \
				convert_unicode.c iconvert.c iconvert.h

CLEANFILES=version.c directories.c bogoupgrade

bogofilter_SOURCES = bogofilter.c bogofilter.h main.c \
		     common.h
bogofilter_static_SOURCES = $(bogofilter_SOURCES)
bogofilter_static_LDFLAGS = $(STATICLDFLAGS)
bogofilter_static_LDADD = $(LDADD) $(STATIC_DB) $(GSL_LIBS)

bogolexer_static_SOURCES = bogolexer.c
bogolexer_static_LDFLAGS = $(STATICLDFLAGS)

bogoutil_SOURCES = bogoutil.c bogohist.c bogohist.h
bogoutil_static_SOURCES = bogoutil.c bogohist.c bogohist.h
bogoutil_static_LDFLAGS = $(STATICLDFLAGS)
bogoutil_static_LDADD = $(LDADD) $(STATIC_DB)

bogotune_SOURCES = bogotune.c bogotune.h \
		   tunelist.c tunelist.h \
		   common.h
bogotune_LDADD = $(LDADD) $(LIBDB) $(GSL_LIBS)

bogotune_static_SOURCES = bogotune.c bogotune.h \
		   tunelist.c tunelist.h \
		   common.h
bogotune_static_LDFLAGS = $(STATICLDFLAGS)
bogotune_static_LDADD= $(LDADD) $(STATIC_DB) $(GSL_LIBS)

debugtest_SOURCES = debug.main.c

wordhash_SOURCES = wordhash.main.c

find_home_test_SOURCES = find_home.test.c

fgetsl_test_SOURCES = fgetsl.c
fgetsl_test_CFLAGS= -DMAIN

# what to distribute
EXTRA_DIST = bogoupgrade.in \
	     version.sh \
	     strlcat.3 strlcpy.3 \
	     patch.lexer.254.txt patch.lexer.253x.txt
#
version.c: version.sh ../configure.ac Makefile.am	\
	$(version_sources)				\
	$(bogofilter_SOURCES)				\
	$(bogolexer_SOURCES)				\
	$(bogoutil_SOURCES)				\
	$(bogotune_SOURCES)
	$(SHELL) $(srcdir)/version.sh $(top_srcdir) >"$@.new" && mv -f "$@.new" "$@" \
	    || { rm -f "$@" ; false ; }
#
directories.c: ../config.status
	rm -f $@
	echo "const char *const system_config_file = \"$(SYSCONFDIR)/bogofilter.cf\";" >"$@" \
	    || { rm -f "$@" ; false ; }
#
$(datastore_OBJECT): Makefile $(datastore_SOURCE)
#
bogoupgrade: bogoupgrade.in Makefile
	rm -f $@
	sed 's}[@]PERL@}$(PERL)}' <$(srcdir)/bogoupgrade.in >$@ || rm -f $@
	chmod +x bogoupgrade

#splint - weak
splint.check: $(version_sources) $(bogofilter_SOURCES) $(bogolexer_SOURCES) $(bogoutil_SOURCES) $(bogotune_SOURCES)
	splint -I. -f $(srcdir)/.lclintrc -weak `for i in \
	$(version_sources)	\
	$(bogofilter_SOURCES)	\
	$(bogolexer_SOURCES)	\
	$(bogoutil_SOURCES)	\
	$(bogotune_SOURCES) ; do echo $(srcdir)/$$i ; done | sort -u | egrep -v "(lexer_v3.l|db_handle_props.h)"` > splint.tmp || :
	grep ^F splint.tmp
	wc -l splint.tmp

lexer_v3.c: $(srcdir)/lexer_v3.l $(srcdir)/Makefile.in
	$(LEX) $(srcdir)/lexer_v3.l
	(echo '/* Include config.h early to avoid _FILE_OFFSET_BITS redefinition on Solaris */'; echo '#include "config.h"' ; sed '/^#/ s|lex.yy\.c|$@|; s/\(accepting rule at line\) %d/\\1 %ld/' lex.yy.c) >$@
if PATCH_FLEX_254
	patch $@ < $(srcdir)/patch.lexer.254.txt
endif
if PATCH_FLEX_253X
	patch $@ < $(srcdir)/patch.lexer.253x.txt
endif
	rm -f lex.yy.c

