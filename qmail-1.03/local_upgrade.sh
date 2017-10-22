#!/bin/sh
# $Log: local_upgrade.sh,v $
# Revision 1.1  2017-10-22 15:27:47+05:30  Cprogrammer
# Initial revision
#
#
# $Id: local_upgrade.sh,v 1.1 2017-10-22 15:27:47+05:30 Cprogrammer Exp mbhangui $
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

do_post_upgrade()
{
if [ -x /bin/systemctl -o -x /usr/bin/systemctl ] ; then
  systemctl is-enabled svscan >/dev/null 2>&1
  if [ $? -ne 0 ] ; then
	  systemctl disable indimail > /dev/null 2>&1
	  systemctl enable svscan > /dev/null 2>&1
  fi
fi
/bin/rm -f /lib/systemd/system/indimail.service
/bin/rm -f /usr/lib/systemd/system/indimail.service
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
	$chmod 2775 /etc/indimail/certs
	if [ $? -ne 0 ] ; then
		exit 1
	fi
fi
# move existing certs in control directory to /etc/indimail/certs
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
# remove clientcert.pem link to servercert.pem in control directory
if [ -f /etc/indimail/control/clientcert.pem ] ; then
	$rm -f /etc/indimail/control/clientcert.pem
fi
if [ -f /etc/indimail/certs/clientcert.pem ] ; then
	$rm -f /etc/indimail/certs/clientcert.pem
fi
for i in servercert.pem dh2048.pem rsa2048.pem dh1024.pem rsa1024.pem dh512.pem rsa512.pem
do
	# roundcube (php) will require read access to certs
	if [ -f /etc/indimail/certs/$i ] ; then
		$chgrp apache /etc/indimail/certs/$i
	fi
done
$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/control/servercert.pem
$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/control/clientcert.pem
$ln -rsf /etc/indimail/certs/servercert.pem /etc/indimail/certs/clientcert.pem
# Certificate location changed from /etc/indimail/control to /etc/indimail/certs
for i in qmail-smtpd.25 qmail-smtpd.465 qmail-smtpd.587 qmail-send.25
do
	echo /etc/indimail/certs > /service/$i/variables/CERTDIR
	# increase for using dlmopen()
	if [ ! " $i" = " qmail-send.25" ] ; then
		echo 536870912 > /service/$i/variables/SOFT_MEM
	fi
done

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

# for bogofilter to send back X-Bogosity back to qmail-smtpd as well as log entry
# to /var/log/indimail/logfifo/current
# for qmail-send it is required if you run bogofilter during remote/local delivery,
# in which case it will be logged to /var/log/indimail/logfifo/current
for i in qmail-smtpd.25 qmail-smtpd.465 qmail-send.25
do
	if [ -d /service/$i -a -s /service/$i/variables/LOGFILTER ] ; then
		echo /tmp/logfifo > /service/$i/variables/LOGFILTER
	fi
done
if [ -s /etc/indimail/control/defaultqueue/LOGFILTER ] ; then
echo /tmp/logfifo > /etc/indimail/control/defaultqueue/LOGFILTER
fi
#
# tcpserver uses -c option to set concurrency and uses MAXDAEMON config file
# on sighup, since tcpserver is no longer root, it is unable to read MAXDAEMON config
# file. Better solution is to move MAXDAEMON config file out of /service/*/variables
# directory
for i in qmail-*qm?pd.* qmail-smtpd.*
do
	$chown root:indimail /service/$i/variables
	$chmod 755 /service/$i/variables
done

$uname -n > /service/qmail-send.25/variables/DEFAULT_DOMAIN
$uname -n > /etc/indimail/control/envnoathost
$uname -n > /etc/indimail/control/defaulthost
if [ -f /service/fetchmail/variables/QMAILDEFAULTHOST ] ; then
	$rm -f /service/fetchmamil/variables/QMAILDEFAULTHOST
fi
# qmail-greyd, greydaemon path changed to /usr/sbin
$sed -i 's{/bin/qmail-greyd{/sbin/qmail-greyd{' /service/greylist.1999/run
}

case $1 in
	post)
	do_post_upgrade
	;;
esac
