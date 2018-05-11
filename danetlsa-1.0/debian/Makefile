prefix=/usr
version=1.0
release=1.1
sysconfdir=/etc/indimail
#

all: rules debian.tar.gz dummy.tar.gz danetlsa-dev.install \
	danetlsa.install libdanetlsa.install

clean:
	/bin/rm -f debian.tar.gz dummy.tar.gz

dummy.tar.gz: README
	tar cf - README |gzip > $@
debian.tar.gz: copyright rules control changelog compat Makefile \
danetlsa-dev.install danetlsa.install libdanetlsa.install \
README.source source
	tar cf - copyright rules control changelog compat \
		danetlsa-dev.install danetlsa.install \
		libdanetlsa.install source |gzip > $@
