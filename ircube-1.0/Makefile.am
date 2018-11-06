@SET_MAKE@
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

ircubelibexecdir=@libexecdir@
ircubelibexec_SCRIPTS = bogo-learn

install-data-hook: 
	$(mkinstalldirs) $(DESTDIR)@plugindir@ || exit 1
	cp -R plugins $(DESTDIR)@plugindir@ || exit 1
	$(mkinstalldirs) $(DESTDIR)@prefix@/share/indimail/doc || exit 1
	cp README $(DESTDIR)@prefix@/share/indimail/doc/README.ircube || exit 1
	cp COPYING $(DESTDIR)@prefix@/share/indimail/doc/COPYING.ircube || exit 1
