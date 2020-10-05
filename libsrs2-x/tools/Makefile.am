@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS = srs
man_MANS = srs.1

srs_SOURCES = srs.c
srs_CFLAGS = -fPIC -fPIE
srs_LDFLAGS = $(LDFLAGS)
srs_LDADD = ../libsrs2/libsrs2.la
