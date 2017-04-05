#!/bin/sh
# $Log: local_upgrade.sh,v $
# Revision 2.7  2017-04-05 14:11:14+05:30  Cprogrammer
# upgraded soft mem to 536870912
#
# Revision 2.6  2017-04-03 15:56:50+05:30  Cprogrammer
# create FIFODIR
#
# Revision 2.5  2017-03-31 21:17:37+05:30  Cprogrammer
# fix DEFAULT_HOST, QMAILDEFAULTHOST, envnoathost, defaulthost settings
#
# Revision 2.4  2017-03-30 23:29:04+05:30  Cprogrammer
# added chgrp
#
# Revision 2.3  2017-03-29 19:31:59+05:30  Cprogrammer
# added rsa2048.pem, dh2048.pem
#
# Revision 2.2  2017-03-29 14:45:42+05:30  Cprogrammer
# fixed upgrade for v2.1
#
# Revision 2.1  2017-03-28 19:21:18+05:30  Cprogrammer
# upgrade script for indimail 2.1
#
#
# $ID:$
#
PATH=/bin:/usr/bin:/usr/sbin:/sbin
chown=$(which chown)
chgrp=$(which chgrp)
ln=$(which ln)
uname=$(which uname)
chmod=$(which chmod)
mkdir=$(which mkdir)
rm=$(which rm)
mv=$(which mv)
sed=$(which sed)
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
fi
if [ -f /etc/indimail/control/clientcert.pem ] ; then
	$rm -f /etc/indimail/control/clientcert.pem
fi
if [ -f /etc/indimail/certs/clientcert.pem ] ; then
	$rm -f /etc/indimail/certs/clientcert.pem
fi
for i in servercert.pem clientcert.pem dh1024.pem dh512.pem \
	rsa2048.pem dh2048.pem rsa512.pem couriersslcache servercert.cnf \
	servercert.rand tlshosts notlshosts
do
	if [ -f /etc/indimail/control/$i -a ! -L /etc/indimail/control/$i ] ; then
		$mv /etc/indimail/control/$i /etc/indimail/certs/$i
		if [ $? -ne 0 ] ; then
			exit 1
		fi
	fi
	if [ -d /etc/indimail/control/$i -a ! -L /etc/indimail/control/$i ] ; then
		$mv /etc/indimail/control/$i /etc/indimail/certs/$i
		if [ $? -ne 0 ] ; then
			exit 1
		fi
		$ln -rsf /etc/indimail/certs/$i /etc/indimail/control/$i
	fi
done
for i in servercert.pem dh2048.pem rsa2048.pem dh1024.pem rsa1024.pem dh512.pem rsa512.pem
do
	if [ -f /etc/indimail/certs/$i ] ; then
		$chgrp apache /etc/indimail/certs/$i
	fi
done
$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/control/servercert.pem
$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/control/clientcert.pem
$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/certs/clientcert.pem
# service qmail-spamlog has been renamed to qmail-logfifo
# fifo is now /tmp/logfifo instead of /tmp/spamfifo
if [ -d /service/qmail-spamlog ] ; then
	$mv /service/qmail-spamlog /service/qmail-logfifo
	$mkdir -p /service/qmail-logfifo/variables
	$chown root:indimail /service/qmail-logfifo/variables
	$chmod 775 /service/qmail-logfifo/variables
	echo /tmp/logfifo > /service/qmail-logfifo/variables/LOGFILTER
	$sed -e 's{smtpd.25{logfifo{' /service/qmail-logfifo/run > /tmp/logfifo.$$
	$mv /tmp/logfifo.$$ /service/qmail-logfifo/run
	$sed -e 's{spamlog{logfifo{' /service/qmail-logfifo/log/run > /tmp/logfifo.$$
	$mv /tmp/logfifo.$$ /service/qmail-logfifo/log/run
fi
for i in qmail-smtpd.25 qmail-smtpd.465 fetchmail qmail-send.25
do
	if [ -d /service/$i ] ; then
		echo /tmp/logfifo > /service/$i/variables/LOGFILTER
	fi
done
echo /tmp/logfifo > /etc/indimail/control/defaultqueue/LOGFILTER
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
	echo 536870912 > /service/$i/variables/SOFT_MEM
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
$uname -n > /service/qmail-send.25/variables/DEFAULT_DOMAIN
$uname -n > /etc/indimail/control/envnoathost
$uname -n > /etc/indimail/control/defaulthost
if [ -f /service/fetchmail/variables/QMAILDEFAULTHOST ] ; then
	$rm -f /service/fetchmamil/variables/QMAILDEFAULTHOST
fi
for i in inlookup.infifo qmail-imapd* qmail-pop3d* qmail-smtpd.25 qmail-smtpd.465 qmail-smtpd.587
do
	if [ -d /service/$i ] ; then
		echo /var/indimail/inquery > /service/$i/variables/FIFODIR
	fi
done
