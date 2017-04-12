@SET_MAKE@
COFLAGS=-z+05:30
AM_CFLAGS = -s -O4 -Wall -fPIC
bin_PROGRAMS = ripmime ripole
man_MANS = ripmime.1 ripole.1
ripmime_SOURCES = strstack.c mime.c ffget.c MIME_headers.c tnef.c \
				  rawget.c pldstr.c logger.c libmime-decoders.c boundary-stack.c \
				  uuencode.c filename-filters.c ole.c \
				  olestream-unwrap.c bytedecoders.c bt-int.c
ripole_SOURCES = ole.c olestream-unwrap.c bytedecoders.c bt-int.c logger.c pldstr.c

ripmime.o: buildcodes.h

ripmime_bindir=@prefix@/bin

buildcodes.h: 
	./generate-buildcodes.sh

ripmime$(EXEEXT): $(ripmime_DEPENDENCIES) $(ripmime_OBJECTS) ripmime.$(OBJEXT)
	$(LINK) $@.$(OBJEXT) $(ripmime_OBJECTS)

ripole$(EXEEXT): $(ripole_DEPENDENCIES) $(ripole_OBJECTS) ripole.$(OBJEXT)
	$(LINK) $@.$(OBJEXT) $(ripole_OBJECTS)

tnef.o: tnef.c
	$(CC) $(CFLAGS) -D___TNEF_BYTE_ORDER=4321 -c tnef.c

clean:
	rm -f *.o *core ${OBJ} buildcodes.h
AUTOMAKE_OPTIONS = foreign no-dependencies
