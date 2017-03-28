#!/bin/sh
# $Log: .upgrade.sh,v $
# Revision 2.1  2017-03-28 19:21:18+05:30  Cprogrammer
# upgrade script for indimail 2.1
#
#
# $ID:$
#
PATH=/bin:/usr/bin:/usr/sbin:/sbin
chown=$(which chown)
ln=$(which ln)
chmod=$(which chmod)
mkdir=$(which mkdir)
rm=$(which rm)
mv=$(which mv)
#
# certs were in /etc/indimail/control
# they have been moved to /etc/indimail/certs
#
if [ ! -d /etc/indimail/certs ] ; then
	$mkdir -p /etc/indimail/certs
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$chown indimail:qmail /etc/indimail/certs
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$chmod 775 /etc/indimail/certs
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	if [ -f /etc/indimail/control/clientcert.pem ] ; then
		$rm -f /etc/indimail/control/clientcert.pem
	fi
	if [ -f /etc/indimail/certs/clientcert.pem ] ; then
		$rm -f /etc/indimail/certs/clientcert.pem
	fi
	for i in servercert.pem clientcert.pem dh1024.pem dh512.pem \
		rsa512.pem servercert.pem couriersslcache servercert.cnf \
		servercert.rand
	do
		if [ -f /etc/indimail/control/$i -a ! -L /etc/indimail/control/$i ] ; then
			$mv /etc/indimail/control/$i /etc/indimail/certs/$i
			if [ $? -ne 0 ] ; then
				exit 1
			fi
		fi
	done
	$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/control/servercert.pem
	$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/control/clientcert.pem
	$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/certs/clientcert.pem
fi
#
# tcpserver uses -c option to set concurrency and uses MAXDAEMON config file
# on sighup, since tcpserver is no longer root, it is unable to read MAXDAEMON config
# file. Better solution is to move MAXDAEMON config file out of /service/*/variables
# directory
for i in indisrvr.4000 proxy-* qmail-*imapd* qmail-*pop3d* qmail-*qm?pd.* qmail-smtpd.* qmail-poppass.*
do
	$chown root:indimail /service/$i/variables
	$chmod 755 /service/$i/variables
done
for i in qmail-smtpd.25 qmail-smtpd.465 qmail-smtpd.587 qmail-send.25
do
	echo /etc/indimail/certs > /service/$i/variables/CERTDIR
done
for i in /service/qmail-imapd* /service/qmail-pop3d* /service/proxy-imapd* /service/proxy-pop3d*
do
	echo /etc/indimail/certs/couriersslcache > $i/variables/TLS_CACHEFILE
	echo /etc/indimail/certs/servercert.pem  > $i/variables/TLS_CERTFILE
done
for i in /service/qmail-poppass* /service/indisrvr.*
do
	echo /etc/indimail/certs/servercert.pem  > $i/variables/CERTFILE
done
