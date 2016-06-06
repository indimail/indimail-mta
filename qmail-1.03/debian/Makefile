qmaildir=/var/indimail
mandir=/usr/share/man
sysconfdir=/etc/indimail
shareddir=/usr/share/indimail
libexecdir=/usr/libexec/indimail
logdir=/var/log/indimail
prefix=/usr
all: rules indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm indimail-mta.postinst \
obs_deb_prep indimail-mta.install \
indimail-mta-xUbuntu_16.04.dsc indimail-mta-xUbuntu_15.10.dsc \
indimail-mta-xUbuntu_15.04.dsc indimail-mta-xUbuntu_14.10.dsc \
indimail-mta-xUbuntu_14.04.dsc indimail-mta-xUbuntu_13.10.dsc indimail-mta-xUbuntu_13.04.dsc \
indimail-mta-xUbuntu_12.10.dsc indimail-mta-xUbuntu_12.04.dsc indimail-mta-xUbuntu_11.10.dsc \
indimail-mta-xUbuntu_11.04.dsc indimail-mta-xUbuntu_10.10.dsc indimail-mta-xUbuntu_10.04.dsc \
indimail-mta-Debian_8.0.dsc indimail-mta-Debian_7.0.dsc indimail-mta-Debian_6.0.dsc \
debian.tar.gz dummy.tar.gz

#bits=$(shell if [ -d /usr/lib64 ] ; then echo 64; else echo 32; fi)
bits=32
libdir=$(shell if [ $(bits) -eq 64 ] ; then echo $(prefix)/lib64; else echo $(prefix)/lib;fi)
acl_libdirstem=$(shell if [ $(bits) -eq 64 ] ; then echo lib64; else echo lib;fi)

edit = sed \
	-e 's,@qmaildir\@,$(qmaildir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@shareddir\@,$(shareddir),g' \
	-e 's,@libexecdir\@,$(libexecdir),g' \
	-e 's,@logdir\@,$(logdir),g' \
	-e 's,@acl_libdirstem\@,$(acl_libdirstem),g' \
	-e 's,@prefix\@,$(prefix),g'

perm_list: ../perm_list.in
	$(edit) ../$@.in > $@

install: all
install-strip: install
distclean: clean

indimail-mta.install: indimail-mta.install.in
	$(edit) $@.in > $@

clean:
	/bin/rm -f rules indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
	indimail-mta.install indimail-mta.postinst obs_deb_prep debian.tar.gz dummy.tar.gz *.dsc

noinst_SCRIPTS = indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
				 indimail-mta.postinst indimail-mta.install daemontools.install \
				 ucspi-tcp.install rules obs_deb_prep
MOSTLYCLEANFILES= indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
				  indimail-mta.postinst rules obs_deb_prep changelog \
				  debian.tar.gz dummy.tar.gz

dummy.tar.gz: README
	tar cf - README |gzip > $@
debian.tar.gz: copyright indimail-mta.preinst indimail-mta.postinst indimail-mta.prerm \
indimail-mta.postrm rules changelog compat obs_deb_prep control perm_list usr.bin.freshclam \
usr.sbin.clamd daemontools.install ucspi-tcp.install indimail-mta.install
	tar cf - copyright indimail-mta.preinst indimail-mta.postinst indimail-mta.prerm \
		indimail-mta.postrm rules changelog compat obs_deb_prep ucspi-tcp.install \
		daemontools.install control perm_list usr.bin.freshclam usr.sbin.clamd \
		indimail-mta.install | gzip > $@
