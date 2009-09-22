# Makefile for the RISC OS version of bogofilter


# Define which compiler to use:

CC = cc
#CC = gcc


# You might have to change the paths in the DBLIB and DBINCLUDE variables
# to suit your configuration.

DBINCLUDE = ^.^.qdbm
DBLIB = ^.^.qdbm.libqdbm


#########################################
# DO NOT EDIT ANYTHING BELOW THIS LINE! #
#########################################

ifeq (${CC},cc)
MAKE = amu
CC = cc
LD = link
AR = libfile
DEPEND = -depend !Depend
CC_FLAGS = -Wdp -throwback -Otime -I@,^,^.^,$(DBINCLUDE),Unix: -JUnix: -D__riscos__ -DHAVE_CONFIG_H
UNIXLIB = Unix:o.UnixLib
else
ifeq (${CC},gcc)
MAKE = make
CC = gcc
LD = gcc
AR = ar
CC_FLAGS = -mthrowback -O3 -I@ -I^ -I^.^ -I$(DBINCLUDE) -D__riscos__ -DHAVE_CONFIG_H
else
# No other compiler supported!
endif
endif

LIBGSL = ^.gsl.libgsl

COMMON_OBJS = o.base64 \
	o.bf_exit \
	o.bogoconfig \
	o.bogofilter \
	o.bogoreader \
	o.bool \
	o.buff \
	o.charset \
	o.collect \
	o.configfile \
	o.datastore \
	o.datastore_qdbm \
	o.debug \
	o.degen \
	o.error \
	o.fgetsl \
	o.find_home \
	o.find_home_tildeexpand \
	o.find_home_user \
	o.format \
	o.globals \
	o.graham \
	o.lexer \
	o.lexer_v3 \
	o.maint \
	o.memcmp \
	o.memdebug \
	o.mime \
	o.msgcounts \
	o.passthrough \
	o.paths \
	o.qp \
	o.register \
	o.rstats \
	o.score \
	o.strlcat \
	o.strlcpy \
	o.swap_32bit \
	o.system \
	o.textblock \
	o.token \
	o.transaction \
	o.tunelist \
	o.uudecode \
	o.version \
	o.word \
	o.wordhash \
	o.wordlists \
	o.wordlists_base \
	o.xatof \
	o.xatoi \
	o.xcalloc \
	o.xmalloc \
	o.xmemrchr \
	o.xmem_error \
	o.xrealloc \
	o.xstrdup \
	o.xstrlcat \
	o.xstrlcpy

BOGOFILTER_OBJS = o.main

BOGOLEXER_OBJS = o.bogolexer

BOGOTUNE_OBJS = o.bogotune

BOGOUTIL_OBJS = o.bogoutil

BOGOWORDFREQ_OBJS = o.bogowordfreq

CONFIGTEST_OBJS = o.configtest

.PHONY:	all dist install clean

.INIT:
	@cdir o

## Rule Patterns ##

.SUFFIXES:	.c .o

.c.o:
	$(CC) $(CC_FLAGS) $(DEPEND) -c -o $@ $<

# Static dependencies:

all: bogofilter bogolexer bogotune bogoutil bogowordfreq configtest

dist: install
	-cdir <Bogofilter$Dir>.doc
	-copy ^.AUTHORS <Bogofilter$Dir>.doc.* ~CF~V
	-copy ^.COPYING <Bogofilter$Dir>.doc.* ~CF~V
	-copy ^.NEWS <Bogofilter$Dir>.doc.* ~CF~V
	-copy ^.doc.bogofilter-faq/html <Bogofilter$Dir>.doc.* ~CF~V
	-copy ^.doc.bogofilter/htmlin <Bogofilter$Dir>.doc.bogofilter/html ~CF~V
	-copy ^.doc.bogolexer/htmlin <Bogofilter$Dir>.doc.bogolexer/html ~CF~V
	-copy ^.doc.bogotune/htmlin <Bogofilter$Dir>.doc.bogotune/html ~CF~V
	-copy ^.doc.bogoutil/htmlin <Bogofilter$Dir>.doc.bogoutil/html ~CF~V
	-settype <Bogofilter$Dir>.doc.AUTHORS Text
	-settype <Bogofilter$Dir>.doc.COPYING Text
	-settype <Bogofilter$Dir>.doc.NEWS Text
	-settype <Bogofilter$Dir>.doc.bogofilter-faq/html HTML
	-settype <Bogofilter$Dir>.doc.bogofilter/html HTML
	-settype <Bogofilter$Dir>.doc.bogolexer/html HTML
	-settype <Bogofilter$Dir>.doc.bogotune/html HTML
	-settype <Bogofilter$Dir>.doc.bogoutil/html HTML

install: bogofilter bogolexer bogotune bogoutil bogowordfreq configtest
	-copy bogofilter <Bogofilter$Dir>.bin.* ~CF~V
	-copy bogolexer <Bogofilter$Dir>.bin.* ~CF~V
	-copy bogotune <Bogofilter$Dir>.bin.* ~CF~V
	-copy bogoutil <Bogofilter$Dir>.bin.* ~CF~V
	-copy bogowordfreq <Bogofilter$Dir>.bin.* ~CF~V
	-copy configtest <Bogofilter$Dir>.bin.* ~CF~V

libbogo: $(COMMON_OBJS)
	$(AR) $(AR_FLAGS) -c -o $@ $(COMMON_OBJS)

$(LIBGSL):
	dir ^.gsl
	$(MAKE)
	dir ^.src

bogofilter: libbogo $(LIBGSL) $(BOGOFILTER_OBJS) $(DBLIB)
	$(LD) $(LD_FLAGS) -o $@ $(BOGOFILTER_OBJS) libbogo $(LIBGSL) $(DBLIB) $(UNIXLIB)

bogolexer: libbogo $(BOGOLEXER_OBJS) $(DBLIB)
	$(LD) $(LD_FLAGS) -o $@ $(BOGOLEXER_OBJS) libbogo $(DBLIB) $(UNIXLIB)

bogotune: libbogo $(LIBGSL) $(BOGOTUNE_OBJS) $(DBLIB)
	$(LD) $(LD_FLAGS) -o $@ $(BOGOTUNE_OBJS) libbogo $(LIBGSL) $(DBLIB) $(UNIXLIB)

bogoutil: libbogo $(BOGOUTIL_OBJS) $(DBLIB)
	$(LD) $(LD_FLAGS) -o $@ $(BOGOUTIL_OBJS) libbogo $(DBLIB) $(UNIXLIB)

bogowordfreq: libbogo $(BOGOWORDFREQ_OBJS) $(DBLIB)
	$(LD) $(LD_FLAGS) -o $@ $(BOGOWORDFREQ_OBJS) libbogo $(DBLIB) $(UNIXLIB)

configtest: libbogo $(CONFIGTEST_OBJS) $(DBLIB)
	$(LD) $(LD_FLAGS) -o $@ $(CONFIGTEST_OBJS) libbogo $(DBLIB) $(UNIXLIB)

clean:
	-ifthere bogofilter then wipe bogofilter ~CF~V
	-ifthere bogolexer then wipe bogolexer ~CF~V
	-ifthere bogotune then wipe bogotune ~CF~V
	-ifthere bogoutil then wipe bogoutil ~CF~V
	-ifthere bogowordfreq then wipe bogowordfreq ~CF~V
	-ifthere configtest then wipe configtest ~CF~V
	-ifthere libbogo then wipe libbogo ~CF~V
	-ifthere o.* then wipe o.* ~CF~V

# Dynamic dependencies:
