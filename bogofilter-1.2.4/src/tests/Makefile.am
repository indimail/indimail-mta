# $Id: Makefile.am 6975 2012-11-06 02:18:15Z m-a $

check_PROGRAMS=dehex spam_header_name dumbhead deqp deb64 escnp abortme \
	       u_fpe wantcore leakmem ctype

AM_CPPFLAGS = -I$(srcdir)/..
AM_LDFLAGS = -L..
LDADD = -lbogofilter

BUILT_SOURCES = t.config t.query.config
CLEANFILES= $(BUILT_SOURCES)

t.config: Makefile
	( echo  >$@ 'EXE_EXT="@EXEEXT@"' ; \
	  echo >>$@  'DB_EXT="@DB_EXT@"' ; \
	  echo >>$@ 'DB_TYPE="@DB_TYPE@"'; \
	  echo >>$@ 'USE_TRANSACTIONS="@USE_TRANSACTIONS@"' ; \
	  echo >>$@ 'USE_UNICODE="@USE_UNICODE@"' ) || rm -f $@

t.query.config: t.query.config.in Makefile
	rm -f $@
	cat $(srcdir)/t.query.config.in | \
	sed 's/[@]ENCODING[@]/@ENCODING@/' | \
	sed 's/[@]DEFAULT_CHARSET[@]/@DEFAULT_CHARSET@/' > $@ || rm -f $@
	chmod +x t.query.config

ENVIRON_TESTS = t.abort t.env t.ctype t.bogodir t.leakfind t.u_fpe

if ENABLE_UNICODE
ENCODING_TESTS=t.encoding
endif

PARSING_TESTS = \
	t.longoptions \
	t.ignore_spam_header \
	t.nullstatsprefix \
	t.integrity t.integrity2 t.integrity3 \
	t.passthrough-hb \
	t.escaped.html t.escaped.url \
	t.base64 t.split t.parsing \
	t.lexer t.lexer.mbx t.lexer.qpcr t.lexer.eoh \
	t.spam.header.place \
	t.block.on.subnets \
	t.token.count \
	t.multiple.tokens.head t.multiple.tokens.body t.multiple.tokens.min.mul \
	$(ENCODING_TESTS) \
	t.rfc2047_broken t.rfc2047_folded \
	t.crash-invalid-base64 \
	t.message_addr t.message_id t.queue_id

WORDLIST_TESTS = t.dump.load t.nonascii.replace t.maint t.robx t.regtest t.upgrade.subnet.prefix t.multiple.wordlists t.probe t.bf_compact

SCORING_TESTS = t.score1 t.score2 t.systest t.grftest t.wordhist

BULKMODE_TESTS = t.bulkmode t.MH t.maildir t.bogoutil

INTEGRITY_TESTS = t.lock1 t.lock3 t.valgrind
# INTEGRITY_TESTS += t.lock2

# these tests are built, but must not be shipped:
BUILT_TESTS = t.query.config

# test scripts expected to fail
XFAIL_TESTS =

TESTSCRIPTS = ${ENVIRON_TESTS} ${PARSING_TESTS} ${WORDLIST_TESTS} ${SCORING_TESTS} ${BULKMODE_TESTS} ${INTEGRITY_TESTS}

TESTS=$(BUILT_TESTS) $(TESTSCRIPTS)

TESTS_ENVIRONMENT=RUN_FROM_MAKE=1 AWK=$(AWK) srcdir=$(srcdir) SHELL="$(SHELL)" $(SHELL) $(VERBOSE)

EXTRA_DIST=$(TESTSCRIPTS) t.frame t.save t.skel \
	printcore t._abort unsort.pl \
	t.query.config.in \
	run.sh \
	inputs/dump.load.inp \
	inputs/dump.load.upd \
	inputs/good.mbx \
	inputs/headerbody.txt \
	inputs/lexer.inp \
	inputs/lexer.mbx \
	inputs/lexer.out \
	inputs/mime-qp-cont-with-cr.txt \
	inputs/msg.1.txt \
	inputs/msg.2.txt \
	inputs/msg.3.txt \
	inputs/msg.4.txt \
	inputs/msg.5.txt \
	inputs/msg.6.txt \
	inputs/msg.7.txt \
	inputs/msg.8.txt \
	inputs/msg.flex \
	inputs/msg.parsing.txt \
	inputs/msg.regtest.n \
	inputs/msg.regtest.s \
	inputs/msg.split.bg.0117.text \
	inputs/msg.split.dr.0118.base64 \
	inputs/msg.split.gs.0119.text \
	inputs/spam.mbx \
	outputs/MH.out \
	outputs/bogolex.out \
	outputs/bulkmode.out \
	outputs/dump.load-1.out \
	outputs/dump.load-2.out \
	outputs/dump.load-3.out \
	outputs/dump.load-4.out \
	outputs/dump.load-5.out \
	outputs/dump.load-6.out \
	outputs/grftest.out \
	outputs/lexer.out \
	outputs/maildir.out \
	outputs/message_addr.ref \
	outputs/message_id.ref \
	outputs/mime-qp-cont-with-cr.exp \
	outputs/msg.1.v \
	outputs/msg.2a.vv \
	outputs/msg.2a.vvv \
	outputs/msg.2b.vv \
	outputs/msg.2b.vvv \
	outputs/msg.3.vvv \
	outputs/multiple.wordlists.ref \
	outputs/parsing.out \
	outputs/queue_id.ref \
	outputs/split.out \
	outputs/token.count.ref
