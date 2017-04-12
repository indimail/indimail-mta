@SET_MAKE@
COFLAGS=-z+05:30

CFLAGS = -g -O4 -Wall -fPIC
bin_PROGRAMS = ripmime ripole
man_MANS = ripmime.1 ripole.1
ripmime_SOURCES = mime.c ffget.c MIME_headers.c tnef.c rawget.c pldstr.c \
				  logger.c libmime-decoders.c boundary-stack.c uuencode.c \
				  filename-filters.c ole.c olestream-unwrap.c bytedecoders.c \
				  ripmime.c strstack.c bt-int.c

ripole_SOURCES = ole.c olestream-unwrap.c bytedecoders.c logger.c pldstr.c \
				 ripole.c bt-int.c

ripmime.o: buildcodes.h

ripmime.$(EXTEXT):
	gcc -Wall -DRIPOLE ripmime.o mime.o ffget.o MIME_headers.o tnef.o \
	rawget.o pldstr.o logger.o libmime-decoders.o boundary-stack.o uuencode.o \
	filename-filters.o ole.o strstack.o olestream-unwrap.o \
	bytedecoders.o bt-int.o -o ripmime

ripmime_bindir=@prefix@/bin

boundary-stack.o: logger.h pldstr.h boundary-stack.h
tnef.o: tnef.c
	$(CC) $(CFLAGS) -D___TNEF_BYTE_ORDER=4321 -c tnef.c

buildcodes.h:
	./generate-buildcodes.sh
AUTOMAKE_OPTIONS = foreign no-dependencies
