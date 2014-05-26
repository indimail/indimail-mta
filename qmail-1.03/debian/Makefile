indimaildir=/var/indimail
all: rules indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm indimail-mta.postinst \
obs_deb_prep \
indimail-mta-xUbuntu_14.04.dsc indimail-mta-xUbuntu_13.10.dsc indimail-mta-xUbuntu_13.04.dsc \
indimail-mta-xUbuntu_12.10.dsc indimail-mta-xUbuntu_12.04.dsc indimail-mta-xUbuntu_11.10.dsc \
indimail-mta-xUbuntu_11.04.dsc indimail-mta-xUbuntu_10.10.dsc indimail-mta-xUbuntu_10.04.dsc \
indimail-mta-Debian_7.0.dsc indimail-mta-Debian_6.0.dsc \
debian.tar.gz dummy.tar.gz

install: all
install-strip: install
distclean: clean

clean:
	/bin/rm -f rules indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
	indimail-mta.postinst obs_deb_prep debian.tar.gz dummy.tar.gz *.dsc

noinst_SCRIPTS = indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
				 indimail-mta.postinst rules obs_deb_prep
MOSTLYCLEANFILES= indimail-mta.prerm indimail-mta.preinst indimail-mta.postrm \
				  indimail-mta.postinst rules obs_deb_prep changelog \
				  debian.tar.gz dummy.tar.gz

dummy.tar.gz: README
	tar cf - README |gzip > $@
debian.tar.gz: copyright indimail-mta.preinst indimail-mta.postinst indimail-mta.prerm \
indimail-mta.postrm rules changelog compat obs_deb_prep control 
	tar cf - copyright indimail-mta.preinst indimail-mta.postinst indimail-mta.prerm \
		indimail-mta.postrm rules changelog compat obs_deb_prep \
		control | gzip > $@
