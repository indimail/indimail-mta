qmaildir=/var/indimail
mandir=/usr/share/man
qsysconfdir=/etc/indimail
sysconfdir=/etc
shareddir=/usr/share/indimail
libexecdir=/usr/libexec/indimail
logdir=/var/log/indimail
prefix=/usr
version=$(shell grep Version: ../indimail-mta.spec.in|cut -d: -f2 | cut -c2-)
all: rules indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm indimail-mta.postinst \
obs_deb_prep indimail-mta.install \
indimail-mta.dsc debian.tar.gz dummy.tar.gz

#bits=$(shell if [ -d /usr/lib64 ] ; then echo 64; else echo 32; fi)
bits=32
libdir=$(shell if [ $(bits) -eq 64 ] ; then echo $(prefix)/lib64; else echo $(prefix)/lib;fi)
acl_libdirstem=$(shell if [ $(bits) -eq 64 ] ; then echo lib64; else echo lib;fi)

edit = sed \
	-e 's,@version\@,$(version),g' \
	-e 's,@qmaildir\@,$(qmaildir),g' \
	-e 's,@mandir\@,$(mandir),g' \
	-e 's,@qsysconfdir\@,$(qsysconfdir),g' \
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
	indimail-mta.install indimail-mta.postinst obs_deb_prep debian.tar.gz dummy.tar.gz

noinst_SCRIPTS = indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
				 indimail-mta.postinst indimail-mta.install daemontools.install \
				 ucspi-tcp.install rules obs_deb_prep
MOSTLYCLEANFILES= indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
				  indimail-mta.postinst rules obs_deb_prep changelog \
				  debian.tar.gz dummy.tar.gz

dummy.tar.gz: README
	tar cf - README |gzip > $@

indimail-mta.postrm: indimail-mta.postrm.in
	$(edit) $@.in > $@
indimail-mta.prerm: indimail-mta.prerm.in
	$(edit) $@.in > $@
indimail-mta.postinst: indimail-mta.postinst.in
	$(edit) $@.in > $@
indimail-mta.preinst: indimail-mta.preinst.in
	$(edit) $@.in > $@

debian.tar.gz: copyright indimail-mta.preinst indimail-mta.postinst indimail-mta.prerm \
indimail-mta.postrm rules changelog compat obs_deb_prep control perm_list usr.bin.freshclam \
usr.sbin.clamd daemontools.install ucspi-tcp.install indimail-mta.install
	tar cf - copyright indimail-mta.preinst indimail-mta.postinst indimail-mta.prerm \
		indimail-mta.postrm rules changelog compat obs_deb_prep ucspi-tcp.install \
		daemontools.install control perm_list usr.bin.freshclam usr.sbin.clamd \
		indimail-mta.install | gzip > $@
