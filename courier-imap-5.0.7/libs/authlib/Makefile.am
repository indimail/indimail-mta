ACLOCAL_AMFLAGS=-I ../../m4
AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../../automake.fix @AUTOMAKE@

noinst_LTLIBRARIES=libauth.la libauthmod.la
EXTRA_LTLIBRARIES=libauthpam.la libauthpwd.la libauthshadow.la libauthcustom.la

EXTRA_PROGRAMS= authshadow authpam authpwd authcustom

noinst_PROGRAMS= @AUTHMODULES@ authtest authenumerate

noinst_DATA=$(BUILT_SOURCES)

CLEANFILES= authstaticlist.c $(authpamlib) $(authshadowlib) $(authpwdlib) \
		$(authcustomlib) authlib.7 authchangepwdir.h

authpam_SOURCES=mod.h modauthpam.c
authpam_DEPENDENCIES=libauthpam.la libauthmod.la libauth.la ../numlib/libnumlib.la
authpam_LDADD=libauthpam.la libauthmod.la libauth.la ../numlib/libnumlib.la -lpam @SHADOWLIBS@ @AUTHPAMCRYPT@ @LIBDL@

authpwd_SOURCES=mod.h modauthpwd.c
authpwd_DEPENDENCIES=libauthpwd.la libauthmod.la libauth.la ../numlib/libnumlib.la @MD5LIB@ @SHA1LIB@
authpwd_LDADD=libauthpwd.la libauthmod.la libauth.la ../numlib/libnumlib.la @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS)

authshadow_SOURCES=mod.h modauthshadow.c
authshadow_DEPENDENCIES=libauthshadow.la libauthmod.la libauth.la ../numlib/libnumlib.la @MD5LIB@ @SHA1LIB@
authshadow_LDADD=libauthshadow.la libauthmod.la libauth.la ../numlib/libnumlib.la @MD5LIB@ @SHA1LIB@ @SHADOWLIBS@ $(CRYPTLIBS)

authcustom_SOURCES=mod.h modauthcustom.c
authcustom_DEPENDENCIES=libauthcustom.la libauthmod.la libauth.la ../numlib/libnumlib.la
authcustom_LDADD=libauthcustom.la libauthmod.la libauth.la ../numlib/libnumlib.la @HMACLIB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS)

authtest_SOURCES=authtest.c
authtest_DEPENDENCIES=libauth.la libauthmod.la
	$(authpamobj) $(authshadowobj) $(authpwdobj) $(authcustomobj) \
	../rfc2045/librfc2045.la ../random128/librandom128.la ../numlib/libnumlib.la
authtest_LDADD=libauth.la libauthmod.la \
	$(authpamobj) $(authshadowobj) $(authpwdobj) $(authcustomobj) \
	../rfc2045/librfc2045.la ../random128/librandom128.la ../numlib/libnumlib.la \
	@HMACLIB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS) -lpam

authenumerate_SOURCES=authenumerate.c
authenumerate_DEPENDENCIES=libauth.la libauthmod.la
	$(authpamobj) $(authshadowobj) $(authpwdobj) $(authcustomobj) \
	../rfc2045/librfc2045.la ../random128/librandom128.la ../numlib/libnumlib.la
authenumerate_LDADD=libauth.la libauthmod.la \
	$(authpamobj) $(authshadowobj) $(authpwdobj) $(authcustomobj) \
	../rfc2045/librfc2045.la ../random128/librandom128.la ../numlib/libnumlib.la \
	@HMACLIB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS) -lpam

if HAVE_MD5
checkpasswordmd5c=checkpasswordmd5.c
else
checkpasswordmd5c=
endif

if HAVE_SHA1
checkpasswordsha1c=checkpasswordsha1.c
else
checkpasswordsha1c=
endif

if HAVE_PAM
authpamc=authpam.c preauthpam.c authwait.h authpwd.c
authpamlib=libauthpam.la
authpamobj=authpam.lo preauthpam.lo
manpam=authman.7
else
authpamc=
authpamlib=
authpamobj=
manpam=
endif

if HAVE_PWD
authpwdc=authpwd.c preauthpwd.c
authpwdlib=libauthpwd.la
authpwdobj=authpwd.lo preauthpwd.lo
manpwd=authpwd.7
else
authpwdc=
authpwdlib=
authpwdobj=
manpwd=
endif

if HAVE_SHADOW
authshadowc=authshadow.c preauthshadow.c
authshadowlib=libauthshadow.la
authshadowobj=authshadow.lo preauthshadow.lo
manshadow=authshadow.7
else
authshadowc=
authshadowlib=
authshadowobj=
manshadow=authshadow.7
endif

if HAVE_CUSTOM
authcustomc=authcustom.c preauthcustom.c authcustom.h
authcustomlib=libauthcustom.la
authcustomobj=authcustom.lo preauthcustom.lo
mancustom=authcustom.7
else
authcustomc=
authcustomlib=
authcustomobj=
mancustom=
endif

extradir=@libexecdir@/imapmodules

libauthpam_la_SOURCES=$(authpamc)
libauthpwd_la_SOURCES=$(authpwdc)
libauthshadow_la_SOURCES=$(authshadowc)
libauthcustom_la_SOURCES=$(authcustom)

libauth_la_SOURCES=auth.h authexit.c chain.c checkpassword.c \
	cryptpassword.c authstaticlist.h authstaticlistsearch.c\
	copyargv.c success.c authoption.c \
	$(checkpasswordmd5c) $(checkpasswordsha1c) \
	authpwdenumerate.c \
	authsasl.c authsasl.h authsaslcram.c authsasllogin.c authsaslplain.c \
	authsaslfrombase64.c authsasltobase64.c \
	authsaslclient.c authsaslclient.h authsaslclientcram.c \
	authsaslclientcrammd5.c authsaslclientcramsha1.c \
	authsaslclientlogin.c authsaslclientplain.c \
	authsyschangepwd.c \
	authstaticlist.c \
	cramlib.c cramlib.h

libauth_la_DEPENDENCIES=authstaticlist.o
libauthdir=@libdir@

libauthmod_la_SOURCES=authmod.c authmod.h authmodclient.c authmodfail.c \
	authmoduser.c authmoduser2.c authmoduser3.c debug.c debug.h
libauthmoddir=@libdir@

authstaticlist.c: config.status mkmodulelist.sh
	@SHELL@ $(srcdir)/mkmodulelist.sh @STATIC_LIST@ >authstaticlist.c

authsyschangepwd.lo: authchangepwdir.h

EXTRA_DIST=authwait.h authpam.c preauthpam.c authpwd.c preauthpwd.c \
	authshadow.c preauthshadow.c authcustom.c preauthcustom.c \
	checkpasswordmd5.c checkpasswordsha1.c authsaslcram.c authlib.7.in

authchangepwdir.h: config.status
	echo '#define AUTHCHANGEPWDIR "@authchangepwdir@"' >authchangepwdir.h

man7=authlib.7
man_LINKS=$(manpam) $(manpwd) $(manshadow) $(mancustom)
man7dir=$(mandir)/man7
man_MANS=authlib.7

authlib.7: authlib.7.in
	CONFIG_FILES=authlib.7 CONFIG_HEADERS= $(SHELL) ./config.status

install-data-local:
	test "$(man_LINKS)" = "" && exit 0; \
	$(mkinstalldirs) $(DESTDIR)$(man7dir) ; \
	for f in $(man_MANS) "" ; do \
		test "$$f" = "" && continue ; \
		$(INSTALL_DATA) $$f $(DESTDIR)$(man7dir) ; done ; \
	cd $(DESTDIR)$(man7dir) || exit 1 ; \
		for f in $(man_LINKS) "" ; do \
			test "$$f" = "" && continue ; \
			rm -f $$f; $(LN_S) authlib.7 $$f ; \
			echo "$(LN_S) authlib.7 $$f" ; \
			test -w /etc && chown bin $$f && chgrp bin $$f ; \
			done ; exit 0

uninstall-local:
	cd $(DESTDIR)$(man7dir) 2>/dev/null || exit 0 ; \
		test "$(man_LINKS)" = "" && exit 0; \
		rm -f $(man_LINKS) $(man_MANS)

dist-hook:
	for f in $(BUILT_SOURCES) ; do rm -f $(distdir)/$$f; done

install-perms:
	@$(MAKE) install-perms-recursive
	@$(MAKE) install-perms-local

install-perms-recursive:
	@>permissions.dat
	@SUBDIRS="$(SUBDIRS)" ; for f in $$SUBDIRS ; do ( cd $$f && $(MAKE) $(AM_MAKEFLAGS) install-perms ) ; cat /dev/null >>$$f/permissions.dat ; cat $$f/permissions.dat >>permissions.dat ; done

install-perms-local:
	@test "$(man_MANS)" = "" && exit 0 ; \
	for f in $(man_MANS) $(man_LINKS) ; do \
	echo $(man7dir)/$$f 644 bin bin man >>permissions.dat ; done
