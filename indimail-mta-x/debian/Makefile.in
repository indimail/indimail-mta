prefix=/usr
qmaildir=/var/indimail
sysconfdir=/etc
qsysconfdir=/etc/indimail
mbase=/home/mail
servicedir=/service
mandir=$(prefix)/share/man
libexecdir=$(prefix)/libexec/indimail
shareddir=$(prefix)/share/indimail
logdir=/var/log/svc

libdkim_version=1.5
libsrs2_version=1.0.18
version=$(shell cat ../conf-version)
release := $(shell cat ../conf-release)
all: rules indimail-mta.prerm indimail-mta.preinst \
indimail-mta.postrm indimail-mta.postinst \
indimail-mini.prerm indimail-mini.preinst \
indimail-mini.postrm indimail-mini.postinst \
obs_deb_prep indimail-mta.install \
indimail-mta.dsc \
indimail-mta-Debian_8.0.dsc \
indimail-mta-xUbuntu_14.04.dsc \
indimail-mta-xUbuntu_16.04.dsc \
indimail-mta-xUbuntu_18.04.dsc \
debian.tar.gz dummy.tar.gz

#bits=$(shell if [ -d /usr/lib64 ] ; then echo 64; else echo 32; fi)
bits=32
libdir=$(shell if [ $(bits) -eq 64 ] ; then echo $(prefix)/lib64; else echo $(prefix)/lib;fi)
acl_libdirstem=$(shell if [ $(bits) -eq 64 ] ; then echo lib64; else echo lib;fi)

edit = sed \
	-e 's,@version\@,$(version),g' \
	-e 's,@qmaildir\@,$(qmaildir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@mbase\@,$(mbase),g' \
	-e 's,@servicedir\@,$(servicedir),g' \
	-e 's,@qsysconfdir\@,$(qsysconfdir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@acl_libdirstem\@,$(acl_libdirstem),g' \
	-e 's,@libdkim_version\@,$(libdkim_version),g' \
	-e 's,@libsrs2_version\@,$(libsrs2_version),g' \
	-e 's,@release\@,$(release),g' \
	-e 's,@prefix\@,$(prefix),g'

install: all
install-strip: install
distclean: clean

indimail-mta.install: indimail-mta.install.in Makefile
	$(edit) $@.in > $@

clean:
	/bin/rm -f rules indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
	indimail-mini.prerm indimail-mini.preinst indimail-mini.postrm \
	indimail-mini.postinst *.dsc changelog \
	indimail-mta.install indimail-mta.postinst obs_deb_prep debian.tar.gz dummy.tar.gz

noinst_SCRIPTS = indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
				 indimail-mta.postinst indimail-mta.install daemontools.install \
				 indimail-mini.prerm indimail-mini.preinst \
				 indimail-mini.postrm indimail-mini.postinst \
				 daemontools.preinst daemontools.postinst daemontools.prerm daemontools.postrm \
				 ucspi-tcp.install rules obs_deb_prep
MOSTLYCLEANFILES= indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
				  indimail-mta.postinst rules obs_deb_prep changelog \
				  indimail-mini.prerm indimail-mini.preinst \
				  indimail-mini.postrm indimail-mini.postinst \
				  debian.tar.gz dummy.tar.gz

dummy.tar.gz: README
	tar cf - README |gzip > $@
obs_deb_prep: obs_deb_prep.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
rules: rules.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
changelog: changelog.in ../conf-version ../conf-release
	(cat $@.in; echo ""; \
	echo " -- Manvendra Bhangui <indimail-mta@indimail.org>  `date +'%a, %d %b %Y %H:%M:%S %z'`" ) \
	| $(edit) > $@
indimail-mta.dsc: indimail-mta.dsc.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mta-Debian_8.0.dsc: indimail-mta-Debian_8.0.dsc.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mta-xUbuntu_14.04.dsc: indimail-mta-xUbuntu_14.04.dsc.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mta-xUbuntu_16.04.dsc: indimail-mta-xUbuntu_16.04.dsc.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mta-xUbuntu_18.04.dsc: indimail-mta-xUbuntu_18.04.dsc.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mta.postrm: indimail-mta.postrm.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mta.prerm: indimail-mta.prerm.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mta.postinst: indimail-mta.postinst.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mta.preinst: indimail-mta.preinst.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@
indimail-mini.prerm: indimail-mini.prerm.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@;
indimail-mini.preinst: indimail-mini.preinst.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@;
indimail-mini.postrm: indimail-mini.postrm.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@;
indimail-mini.postinst: indimail-mini.postinst.in Makefile ../conf-version ../conf-release
	$(edit) $@.in > $@;
indimail-mta.cron.d: indimail-mta.cron.d.in Makefile
	$(edit) $@.in > $@

debian.tar.gz: copyright indimail-mta.preinst indimail-mta.postinst indimail-mta.prerm \
indimail-mta.postrm rules changelog compat obs_deb_prep control usr.bin.freshclam \
usr.sbin.clamd daemontools.install daemontools.preinst daemontools.postinst daemontools.prerm \
indimail-mini.prerm indimail-mini.preinst indimail-mta.cron.d \
indimail-mini.postrm indimail-mini.postinst indimail-mini.install \
daemontools.postrm ucspi-tcp.install indimail-mta.install
	tar cf - copyright indimail-mta.preinst indimail-mta.postinst indimail-mta.prerm \
		indimail-mta.postrm rules changelog compat obs_deb_prep ucspi-tcp.install \
		daemontools.install daemontools.preinst daemontools.postinst daemontools.prerm \
		daemontools.postrm control usr.bin.freshclam usr.sbin.clamd \
		indimail-mini.prerm indimail-mini.preinst indimail-mta.cron.d \
		indimail-mini.postrm indimail-mini.postinst indimail-mini.install \
		indimail-mta.install | gzip > $@
