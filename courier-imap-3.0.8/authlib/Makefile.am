# $Id: Makefile.am,v 1.83 2004/05/23 14:28:24 mrsam Exp $
#
# Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
# distribution information.

ACLOCAL_AMFLAGS=-I m4
AUTOMAKE = srcdir=${srcdir} @SHELL@ ${srcdir}/../automake.fix @AUTOMAKE@

EXTRA_PROGRAMS=authcram authpam authpwd authshadow authuserdb authvchkpw \
	authldap authmysql authpgsql authdaemon authdaemontest authmksock authcustom \
	authdaemon.passwd

CLEANFILES=authldaprc.h authmysqlrc.h authdaemonrc.h vpopmail_config.h \
	authpgsqlrc.h libauth-modules authdaemon \
	authstaticlist.c authdaemondstaticlist.c authlib.7 authlib.html \
	authchangepwdir.h sbindir.h authinfodaemondlist.c permissions.dat \
	libauth.dep libauth1.dep *.libsdep README.authdebug.html @LIBAUTH@

distclean-local:
	rm -rf libauth-modules.dir
	test -f authdaemond.versions.final || exit 0 ; \
	for f in `cat authdaemond.versions.final | awk ' { print $2 } '` ; \
		do rm -f $$f $$f.c ; done
	rm -f authdaemond.versions.final

DISTCLEANFILES=authconfiglist \
	authpwdprogs authdaemon.msg authvchkpw.flag

#
# AUTHMODULES should be listed in the best authentication order
#

noinst_PROGRAMS= @PWPROGS@ @AUTHMODULES_PROGS@ authinfo authtest \
	authenumerate @authdaemondprog@
noinst_SCRIPTS=libauth.dep authdaemond.in

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
authpamlibsdep=authpam.libsdep
else
authpamc=
authpamlibsdep=
endif

if HAVE_PWD
authpwdc=authpwd.c preauthpwd.c
authpwdlibsdep=authpwd.libsdep
else
authpwdc=
authpwdlibsdep=
endif

if HAVE_SHADOW
authshadowc=authshadow.c preauthshadow.c
authshadowlibsdep=authshadow.libsdep
else
authshadowc=
authshadowlibsdep=
endif

if HAVE_AUTHUSERDB
authuserdbc=authuserdb.c preauthuserdb.c preauthuserdbcommon.c authuserdbpwd.c
authuserdblibsdep=authuserdb.libsdep
else
authuserdbc=
authuserdblibsdep=
endif

if HAVE_AUTHPGSQL
authpgsqlc=authpgsql.c preauthpgsql.c authpgsqllib.c authpgsql.h
authpgsqllibsdep=authpgsql.libsdep

install-authpgsqlrc:
	$(mkinstalldirs) $(DESTDIR)`dirname @authpgsqlrc@`
	$(INSTALL_DATA) -m $(rcperms) $(srcdir)/authpgsqlrc \
		$(DESTDIR)@authpgsqlrc@.dist

uninstall-authpgsqlrc:
	rm -f $(DESTDIR)@authpgsqlrc@.dist

else
authpgsqlc=
authpgsqllibsdep=

install-authpgsqlrc:
	@:

uninstall-authpgsqlrc:
	@:
endif

if HAVE_AUTHMYSQL
authmysqlc=authmysql.c preauthmysql.c authmysqllib.c authmysql.h
authmysqllibsdep=authmysql.libsdep

install-authmysqlrc:
	$(mkinstalldirs) $(DESTDIR)`dirname @authmysqlrc@`
	$(INSTALL_DATA) -m $(rcperms) $(srcdir)/authmysqlrc \
		$(DESTDIR)@authmysqlrc@.dist

uninstall-authmysqlrc:
	rm -f $(DESTDIR)@authmysqlrc@.dist

else
authmysqlc=
authmysqllibsdep=

install-authmysqlrc:
	@:

uninstall-authmysqlrc:
	@:
endif

if HAVE_AUTHCRAM
authcramc=authcram.c preauthcram.c preauthuserdbcommon.c
authcramlibsdep=authcram.libsdep
else
authcramc=
authcramlibsdep=
endif


if HAVE_VCHKPW
authvchkpwc=authvchkpw.c authvchkpwlib.c preauthvchkpw.c
authvchkpwlibsdep=authvchkpw.libsdep
else
authvchkpwc=
authvchkpwlibsdep=
endif

if HAVE_CUSTOM
authcustomc=authcustom.c preauthcustom.c authcustom.h
authcustomlibsdep=authcustom.libsdep
else
authcustomc=
authcustomlibsdep=
endif

if HAVE_LDAP
authldapc=authldap.c preauthldap.c authldaplib.c authldap.h
authldaplibsdep=authldap.libsdep

install-authldaprc:
	$(mkinstalldirs) $(DESTDIR)`dirname @authldaprc@` 
	$(INSTALL_DATA) -m $(rcperms) $(srcdir)/authldaprc \
		$(DESTDIR)@authldaprc@.dist

uninstall-authldaprc:
	rm -f $(DESTDIR)@authldaprc@.dist

else
authldapc=
authldaplibsdep=

install-authldaprc:
	@:

uninstall-authldaprc:
	@:
endif

if HAVE_AUTHDAEMON
authdaemonc=authdaemon.c authdaemonlib.c preauthdaemon.c
authdaemonlibsdep=authdaemon.libsdep

install-authdaemonrc:
	$(mkinstalldirs) $(DESTDIR)`dirname @authdaemonrc@` 
	$(INSTALL_DATA) -m $(rcperms) authdaemonrc $(DESTDIR)@authdaemonrc@.dist

uninstall-authdaemonrc:
	rm -f $(DESTDIR)@authdaemonrc@.dist
else
authdaemonc=
authdaemonlibsdep=

install-authdaemonrc:
	@:

uninstall-authdaemonrc:
	@:

endif

rcperms=`test -d ../courier && echo "660" && exit 0; echo 600`
user=`test -d ../courier && . ../courier/uidgid && echo $$mailuser && exit 0; echo root`
group=`test -d ../courier && . ../courier/uidgid && echo $$mailgroup && exit 0; echo root`

authldaprc.h:
	echo "#define AUTHLDAPRC \"@authldaprc@\"" >authldaprc.h

authmysqlrc.h:
	echo "#define AUTHMYSQLRC \"@authmysqlrc@\"" >authmysqlrc.h

authpgsqlrc.h:
	echo "#define AUTHPGSQLRC \"@authpgsqlrc@\"" >authpgsqlrc.h

authdaemonrc.h:
	echo "#define AUTHDAEMONRC \"@authdaemonrc@\"" >authdaemonrc.h
	echo "#define AUTHDAEMONVAR \"@authdaemonvar@\"" >>authdaemonrc.h
	echo "#define AUTHDAEMONPID AUTHDAEMONVAR \"/pid\"" >>authdaemonrc.h
	echo "#define AUTHDAEMONSOCK AUTHDAEMONVAR \"/socket\"" >>authdaemonrc.h
	echo "#define AUTHDAEMONLOCK AUTHDAEMONVAR \"/lock\"" >>authdaemonrc.h

vpopmail_config.h:
	echo '#include "@vpopmail_home@/include/config.h"' >vpopmail_config.h

EXTRA_DIST=authwait.h authpam.c preauthpam.c authpwd.c preauthpwd.c \
	authshadow.c preauthshadow.c \
	authcustom.c preauthcustom.c authcustom.h \
	authdaemon.c authdaemond.c authdaemonlib.c preauthdaemon.c \
	authuserdb.c preauthuserdb.c preauthuserdbcommon.c \
	authuserdbpwd.c \
	authmysql.c preauthmysql.c authmysqllib.c authmysql.h \
	authpgsql.c preauthpgsql.c authpgsqllib.c authpgsql.h \
	authcram.c preauthcram.c \
	authvchkpw.c authvchkpwlib.c preauthvchkpw.c \
	authldap.c preauthldap.c authldaplib.c authldap.h authldap.schema\
	checkpasswordmd5.c checkpasswordsha1.c \
	libauth.sh libauth1.sh libauth2.sh \
	authsaslcram.c \
	authldaprc authmysqlrc authpgsqlrc README.ldap README.authmysql.html \
	README.authmysql.myownquery README.authpostgres.html\
	mkmodulelist.sh authdaemond.versions make-authdaemond.sh \
	authlib.html.in authlib.7.in README.authdebug.html.in

##
##  NOTE - we want to insert authpamc, authpwdc, et al, into libauth.a
##  HOWEVER, due to Makefile conditional, the resulting Makefile.in was
##  almost a megabyte long!!!!
##
##  So...  Here's plan B:
##
##  Put each authentication code in its own library.
##
##  First, let's define our temporary libraries.  Don't forget to change
##  configure.in to initialize @AUTHLIB@!

EXTRA_LIBRARIES=libauth-authpam.a \
	libauth-authpwd.a \
	libauth-authshadow.a \
	libauth-authuserdb.a \
	libauth-authcram.a \
	libauth-authvchkpw.a \
	libauth-authldap.a \
	libauth-authdaemon.a \
	libauth-authcustom.a \
	libauth-authmysql.a \
	libauth-authpgsql.a

libauth_authpam_a_SOURCES=$(authpamc)
libauth_authpwd_a_SOURCES=$(authpwdc)
libauth_authshadow_a_SOURCES=$(authshadowc)
libauth_authuserdb_a_SOURCES=$(authuserdbc)
libauth_authcram_a_SOURCES=$(authcramc)
libauth_authvchkpw_a_SOURCES=$(authvchkpwc)
libauth_authldap_a_SOURCES=$(authldapc)
libauth_authmysql_a_SOURCES=$(authmysqlc)
libauth_authpgsql_a_SOURCES=$(authpgsqlc)
libauth_authdaemon_a_SOURCES=$(authdaemonc)
libauth_authcustom_a_SOURCES=$(authcustomc)

#
#  Then, let's make sure that they're built together with libauth.a
#  and libauthmod.a
#

noinst_LIBRARIES=libauth.a libauthmod.a @LIBAUTH@

libauth_a_SOURCES=auth.h authexit.c chain.c checkpassword.c \
	cryptpassword.c authstaticlist.h authstaticlistsearch.c\
	copyargv.c success.c authoption.c \
	$(checkpasswordmd5c) $(checkpasswordsha1c) \
	$(authmysqlc) \
	$(authpgsqlc) \
	authpwdenumerate.c \
	authsasl.c authsasl.h authsaslcram.c authsasllogin.c authsaslplain.c \
	authsaslfrombase64.c authsasltobase64.c \
	authsaslclient.c authsaslclient.h authsaslclientcram.c \
	authsaslclientcrammd5.c authsaslclientcramsha1.c \
	authsaslclientlogin.c authsaslclientplain.c \
	authsyschangepwd.c \
	cramlib.c cramlib.h

# Now, let's add an additional dependency to libauth.a:

libauth_a_DEPENDENCIES=authstaticlist.o libauth-modules

#
# What we're going to do is to extract the contents of @LIBAUTH@
# into a temporary directory, and initialize the contents of this file
# with the directory's contents!

libauth_a_LIBADD=authstaticlist.o `cat libauth-modules`

#
# Now, here's how we're going to create this sucker
#

libauth-modules: @LIBAUTH@
	rm -rf libauth-modules.dir
	mkdir libauth-modules.dir
	cd libauth-modules.dir ; for f in @LIBAUTH@ "" ; do \
		test ! -f ../$$f || ar x ../$$f ; done ; cd ..
	-ls libauth-modules.dir/*.o >libauth-modules 2>/dev/null

libauthmod_a_SOURCES=authmod.c authmod.h authmodclient.c authmodfail.c \
	authmoduser.c authmoduser2.c authmoduser3.c debug.c debug.h

libauth.dep: libauth1.dep
	cp libauth1.dep libauth.dep
	. ./authdaemonrc ; \
		test "$$authdaemonvar" = "" || cp authdaemon.libsdep libauth.dep

libauth1.dep: $(authpamlibsdep) $(authpwdlibsdep) $(authshadowlibsdep) \
		$(authuserdblibsdep) $(authvchkpwlibsdep) $(authcramlibsdep) \
		$(authldaplibsdep) $(authmysqllibsdep) $(authpgsqllibsdep) $(authdaemonlibsdep) \
		$(authcustomlibsdep)
	cat $(authpamlibsdep) $(authpwdlibsdep) $(authshadowlibsdep) \
		$(authvchkpwlibsdep) $(authuserdblibsdep) $(authcramlibsdep) \
		$(authldaplibsdep) $(authmysqllibsdep) $(authpgsqllibsdep) $(authdaemonlibsdep) \
		$(authcustomlibsdep) \
		/dev/null | @SHELL@ $(srcdir)/libauth.sh @SHELL@ $(srcdir) >libauth1.dep

authpam_SOURCES=mod.h modauthpam.c
authpam_DEPENDENCIES=libauthmod.a libauth.a
authpam_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a -lpam @SHADOWLIBS@ @AUTHPAMCRYPT@ @LIBDL@
authpam.libsdep: authpam
	echo ../numlib/libnumlib.a -lpam @SHADOWLIBS@ @AUTHPAMCRYPT@ @LIBDL@ >authpam.libsdep

authpwd_SOURCES=mod.h modauthpwd.c
authpwd_DEPENDENCIES=libauthmod.a libauth.a @MD5LIB@ @SHA1LIB@
authpwd_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS)
authpwd.libsdep: authpwd
	echo ../numlib/libnumlib.a @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS) >authpwd.libsdep

authshadow_SOURCES=mod.h modauthshadow.c
authshadow_DEPENDENCIES=libauthmod.a libauth.a @MD5LIB@ @SHA1LIB@
authshadow_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a @MD5LIB@ @SHA1LIB@ @SHADOWLIBS@ $(CRYPTLIBS)
authshadow.libsdep: authshadow
	echo ../numlib/libnumlib.a @MD5LIB@ @SHA1LIB@ @SHADOWLIBS@ $(CRYPTLIBS) >authshadow.libsdep

authuserdb_SOURCES=mod.h modauthuserdb.c
authuserdb_DEPENDENCIES=libauthmod.a libauth.a ../userdb/libuserdb.a
authuserdb_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a ../userdb/libuserdb.a @dblibrary@ @HMACLIB@ @LIBGDBM@ @LIBDB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS)
authuserdb.libsdep: authuserdb
	echo ../numlib/libnumlib.a ../userdb/libuserdb.a @dblibrary@ @HMACLIB@ @LIBGDBM@ @LIBDB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS) >authuserdb.libsdep

authpgsql_SOURCES=mod.h modauthpgsql.c
authpgsql_DEPENDENCIES=libauthmod.a libauth.a
authpgsql_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ @SHA1LIB@ \
	$(CRYPTLIBS) @PGSQL_LIBS@ @LIBM@ @NETLIBS@

authpgsql.libsdep: authpgsql
	echo ../numlib/libnumlib.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS) @PGSQL_LIBS@ @LIBM@ @NETLIBS@ > authpgsql.libsdep

authmysql_SOURCES=mod.h modauthmysql.c
authmysql_DEPENDENCIES=libauthmod.a libauth.a
authmysql_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ @SHA1LIB@ \
	$(CRYPTLIBS) @MYSQL_LIBS@ @LIBM@ @NETLIBS@

authmysql.libsdep: authmysql
	echo ../numlib/libnumlib.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS) @MYSQL_LIBS@ @LIBM@ @NETLIBS@ > authmysql.libsdep

authvchkpw_SOURCES=mod.h modauthvchkpw.c
authvchkpw_DEPENDENCIES=libauthmod.a libauth.a @MD5LIB@ @SHA1LIB@
authvchkpw_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a @MD5LIB@ @SHA1LIB@ @VPOPMAILLIBS@ @LIBM@ $(CRYPTLIBS)
authvchkpw.libsdep: authvchkpw
	echo ../numlib/libnumlib.a @MD5LIB@ @SHA1LIB@ @LIBM@ -L@vpopmail_home@/lib @VPOPMAILLIBS@ $(CRYPTLIBS) >authvchkpw.libsdep

authcram_SOURCES=mod.h modauthcram.c
authcram_DEPENDENCIES=libauthmod.a libauth.a @HMACLIB@ ../userdb/libuserdb.a \
	@dblibrary@ @MD5LIB@ @SHA1LIB@ @SHA1LIB@
authcram_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a \
	@HMACLIB@ ../userdb/libuserdb.a \
	@dblibrary@ @MD5LIB@ @SHA1LIB@ @LIBGDBM@ @LIBDB@
authcram.libsdep: authcram
	echo @HMACLIB@ ../userdb/libuserdb.a @dblibrary@ @MD5LIB@ \
	../numlib/libnumlib.a @SHA1LIB@ @LIBGDBM@ @LIBDB@ >authcram.libsdep

authldap_SOURCES=mod.h modauthldap.c
authldap_DEPENDENCIES=libauthmod.a libauth.a @MD5LIB@ @SHA1LIB@
authldap_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ \
	@LDAPLIBS@ $(CRYPTLIBS) @NETLIBS@
authldap.libsdep: authldap
	echo ../numlib/libnumlib.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ @LDAPLIBS@ $(CRYPTLIBS) @NETLIBS@\
		>authldap.libsdep

authcustom_SOURCES=mod.h modauthcustom.c
authcustom_DEPENDENCIES=libauthmod.a libauth.a
authcustom_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS)
authcustom.libsdep: authcustom
	echo ../numlib/libnumlib.a @HMACLIB@ @MD5LIB@ @SHA1LIB@ $(CRYPTLIBS) >authcustom.libsdep

authdaemon_SOURCES=mod.h modauthdaemon.c
authdaemon_DEPENDENCIES=libauthmod.a libauth.a ../numlib/libnumlib.a ../soxwrap/libsoxwrap.a ../soxwrap/soxlibs.dep
authdaemon_LDADD=libauthmod.a libauth.a ../numlib/libnumlib.a ../soxwrap/libsoxwrap.a `cat ../soxwrap/soxlibs.dep` @NETLIBS@
authdaemon.libsdep: authdaemon
	echo ../numlib/libnumlib.a ../soxwrap/libsoxwrap.a `cat ../soxwrap/soxlibs.dep` @NETLIBS@ >authdaemon.libsdep

authdaemon_passwd_SOURCES=authdaemonpwd.c
authdaemon_passwd_DEPENDENCIES=libauth.a ../numlib/libnumlib.a ../soxwrap/libsoxwrap.a ../soxwrap/soxlibs.dep
authdaemon_passwd_LDADD=libauth.a ../numlib/libnumlib.a  ../soxwrap/libsoxwrap.a `cat ../soxwrap/soxlibs.dep` @NETLIBS@

authinfo_SOURCES=authinfo.c authinfodaemondlist.c
authinfo_DEPENDENCIES=libauth.a libauth.dep libauthmod.a\
	../rfc2045/librfc2045.la @UNICODE_LIB@ ../rfc822/libencode.la ../rfc822/librfc822.la\
	../random128/librandom128.a
authinfo_LDADD=libauth.a libauthmod.a ../rfc2045/librfc2045.la \
	../rfc822/libencode.la ../rfc822/librfc822.la @UNICODE_LIB@\
	../random128/librandom128.a `cat libauth.dep libauth1.dep`

authtest_SOURCES=authtest.c
authtest_DEPENDENCIES=libauth.a libauth1.dep libauthmod.a\
	../rfc2045/librfc2045.la ../random128/librandom128.a
authtest_LDADD=libauth.a libauthmod.a\
	../rfc2045/librfc2045.la ../random128/librandom128.a `cat libauth1.dep`

authenumerate_SOURCES=authenumerate.c
authenumerate_DEPENDENCIES=libauth.a libauthmod.a ../numlib/libnumlib.a libauth.dep\
	../rfc822/librfc822.la ../rfc2045/librfc2045.la \
	../random128/librandom128.a @UNICODE_LIB@
authenumerate_LDADD=libauth.a libauthmod.a\
	../rfc2045/librfc2045.la ../rfc822/librfc822.la ../numlib/libnumlib.a \
	../random128/librandom128.a `cat libauth.dep` @UNICODE_LIB@

#
# make-authdaemond target is inserted into noinst_PROGRAMS when authdaemon
# is selected.  What we need to do is read authdaemond.versions.final,
# and build the selected authdaemonds.
#

AUTHDAEMOND_DEP_COMMON=authdaemond.o libauth.a libauthmod.a\
 ../rfc2045/librfc2045.la ../random128/librandom128.a\
 ../liblock/liblock.a ../numlib/libnumlib.a

make-authdaemond: $(AUTHDAEMOND_DEP_COMMON) libauth.dep libauth1.dep
	@SHELL@ $(srcdir)/make-authdaemond.sh "$(MAKE)" "$(EXEEXT)"

# The following rules are used by make-authdaemond.sh to build each individual
# authdaemond

AUTHDAEMOND_TARGET=dummy.dummy   # Overridden by make-authdaemond.sh
AUTHDAEMONDLIST_O=dummy.dummy.o
AUTHDAEMONDLIST_C=dummy.dummy.c
AUTHDAEMOND_DEPS=

$(AUTHDAEMOND_TARGET): $(AUTHDAEMOND_DEP_COMMON) $(AUTHDAEMONDLIST_O) \
			$(AUTHDAEMOND_DEPS)
	$(LINK) authdaemond.o $(AUTHDAEMONDLIST_O) \
		libauth.a libauthmod.a ../rfc2045/librfc2045.la \
		../random128/librandom128.a \
		../liblock/liblock.a ../numlib/libnumlib.a \
		`cat $(AUTHDAEMOND_DEPS) /dev/null | @SHELL@ $(srcdir)/libauth.sh @SHELL@ $(srcdir)` @NETLIBS@

$(AUTHDAEMONDLIST_C): config.status
	@SHELL@ $(srcdir)/mkmodulelist.sh $(AUTHDAEMOND_MODS) >$(AUTHDAEMONDLIST_C)	


authdaemontest_SOURCES=authdaemontest.c
authdaemontest_DEPENDENCIES=libauth.a libauthmod.a ../numlib/libnumlib.a
authdaemontest_LDADD=libauth.a libauthmod.a ../numlib/libnumlib.a @NETLIBS@

authmksock_SOURCES=authmksock.c
authmksock_LDADD=@NETLIBS@

man7=authlib.7
manlinks=authpwd.7 authshadow.7 authuserdb.7 authpam.7 authvchkpw.7 authcram.7 \
	authldap.7 authmysql.7 authdaemon.7 authdaemond.7

man7dir=$(mandir)/man7

man_MANS=@manFILES@
man_LINKS=@manLINKS@

BUILT_SOURCES=$(man7) authlib.html authldaprc.h authmysqlrc.h authpgsqlrc.h \
	authdaemonrc.h vpopmail_config.h authstaticlist.c \
	authdaemondstaticlist.c authinfodaemondlist.c \
	authchangepwdir.h sbindir.h

authchangepwdir.h: config.status
	echo '#define AUTHCHANGEPWDIR "@authchangepwdir@"' >authchangepwdir.h

sbindir.h: config.status
	echo '#define SBINDIR "@sbindir@"' >sbindir.h

noinst_DATA=$(BUILT_SOURCES)

dist-hook:
	for f in $(BUILT_SOURCES) ; do rm -f $(distdir)/$$f; done

authstaticlist.c: config.status
	@SHELL@ $(srcdir)/mkmodulelist.sh @STATIC_LIST@ >authstaticlist.c

authinfodaemondlist.c: authdaemondstaticlist.c
	sed 's/authstaticmodulelist/authdaemonmodulelist/' <authdaemondstaticlist.c >authinfodaemondlist.c

authdaemondstaticlist.c: config.status
	@SHELL@ $(srcdir)/mkmodulelist.sh @DAEMON_STATIC_LIST@ >authdaemondstaticlist.c

# automake still a bit stupid...

install-data-local: install-authldaprc install-authdaemonrc install-authmysqlrc install-authpgsqlrc
	test "$(man_LINKS)" = "" && exit 0; \
	$(mkinstalldirs) $(DESTDIR)$(man7dir) ; \
	for f in $(man_MANS) "" ; do \
		test "$$f" = "" && continue ; \
		$(INSTALL_DATA) $$f $(DESTDIR)$(man7dir) ; done ; \
	cd $(DESTDIR)$(man7dir) || exit 1 ; \
		for f in $(man_LINKS) "" ; do \
			test "$$f" = "" && continue ; \
			rm -f $$f; $(LN_S) authlib.7 $$f ; \
			test -w /etc && chown bin $$f && chgrp bin $$f ; \
			done ; exit 0

uninstall-local: uninstall-authldaprc uninstall-authdaemonrc uninstall-authmysqlrc uninstall-authpgsqlrc
	cd $(DESTDIR)$(man7dir) 2>/dev/null || exit 0 ; \
		test "$(man_LINKS)" = "" && exit 0; \
		rm -f $(man_LINKS) $(man_MANS)

authlib.html: authlib.html.in
	CONFIG_FILES=authlib.html CONFIG_HEADERS= $(SHELL) ./config.status

authlib.7: authlib.7.in
	CONFIG_FILES=authlib.7 CONFIG_HEADERS= $(SHELL) ./config.status

if HAVE_SGML
authlib.html.in: authlib.sgml ../docbook/sgml2html
	../docbook/sgml2html authlib.sgml authlib.html.in

authlib.7.in: authlib.sgml ../docbook/sgml2man
	../docbook/sgml2man authlib.sgml authlib.7.in
endif

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
	@for f in `cat modulelist` ; do \
		test "$$f" = "authldap" || continue ; \
		echo $(sysconfdir)/authldaprc.dist $(rcperms) $(user) $(group) config >>permissions.dat; \
		done
	@for f in `cat modulelist` ; do \
		test "$$f" = "authmysql" || continue ; \
		echo $(sysconfdir)/authmysqlrc.dist $(rcperms) $(user) $(group) config >>permissions.dat; \
		done
	@for f in `cat modulelist` ; do \
		test "$$f" = "authpgsql" || continue ; \
		echo $(sysconfdir)/authpgsqlrc.dist $(rcperms) $(user) $(group) config >>permissions.dat; \
		done

authvchkpw.o: authvchkpw.c vpopmail_config.h

authmysqllib.o: authmysqllib.c authmysqlrc.h

authpgsqllib.o: authpgsqllib.c authpgsqlrc.h

preauthvchkpw.o: preauthvchkpw.c vpopmail_config.h

authldaplib.o: authldaplib.c authldaprc.h

authdaemond.o: authdaemonrc.h authdaemond.c

authdaemonlib.o: authdaemonrc.h authdaemonlib.c


install-configure:

