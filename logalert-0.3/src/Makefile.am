LIBS = -ly @LEXLIB@ @LIBPCRE_LIBS@

bin_PROGRAMS = logalert

logalert_SOURCES = utils.c log.c parser.y lex.l process.c monitor.c action.c sig.c user.c main.c 
logalert_LDADD = @LIBS@

YACC = bison
#LEX_OUTPUT_ROOT = lex.c
#AM_LFLAGS  = -Pc
AM_YFLAGS  = -d -y

