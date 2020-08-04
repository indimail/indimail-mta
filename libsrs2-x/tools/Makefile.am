@SET_MAKE@

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS = srs
man_MANS = srs.1

srs_SOURCES = srs.c
srs_CFLAGS = -fPIC -fPIE
srs_LDFLAGS = -pie
srs_LDADD = -L../libsrs2 -lsrs2
