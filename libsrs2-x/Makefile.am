SUBDIRS = libsrs2 tools
EXTRA_DIST = perl python win32 dist debian LICENSE.BSD LICENSE.GPL-2
man_MANS=srs.1

dist-hook:
	@for file in $(EXTRA_DIST); do \
		find $(distdir)/$$file -name .svn | xargs rm -rf; \
	done
	rm -f $(distdir)/perl/Makefile.old
