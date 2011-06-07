LIBS = @LEXLIB@ @LIBPCRE_LIBS@

bin_PROGRAMS = logalert

logalert_SOURCES = lex.c utils.c log.c parser.y process.c monitor.c action.c sig.c user.c main.c 
logalert_LDADD = @LIBS@

MOSTLYCLEANFILES=lex.c

AM_LFLAGS  = -Pc
AM_YFLAGS  = -d
#$(am__skiplex) $(SHELL) $(YLWRAP) $< $(LEX_OUTPUT_ROOT).c $@ -- flex
lex.c: lex.l
	 $(LEX) $(LFLAGS) -o $@ $^
