#!/bin/sh
# $Log: qlocal_upgrade.sh,v $
# Revision 1.5  2018-01-31 16:21:36+05:30  Cprogrammer
# update QMAILLOCAL, QMAILREMOTE for qmail-local, qmail-remote in sbin
#
# Revision 1.4  2018-01-09 11:46:40+05:30  Cprogrammer
# updated for v2.3 indimail-mta
#
# Revision 1.2  2017-11-06 21:46:12+05:30  Cprogrammer
# fixed upgrade script for posttrans
#
# Revision 1.1  2017-10-22 15:27:47+05:30  Cprogrammer
# Initial revision
#
#
# $Id: qlocal_upgrade.sh,v 1.5 2018-01-31 16:21:36+05:30 Cprogrammer Exp mbhangui $
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

check_update_if_diff()
{
	val=`cat $1 2>/dev/null`
	if [ ! " $val" = " $2" ] ; then
		echo $2 > $1
	fi
}

do_post_upgrade()
{
date
if [ -x /bin/systemctl -o -x /usr/bin/systemctl ] ; then
  systemctl is-enabled svscan >/dev/null 2>&1
  if [ $? -ne 0 ] ; then
	  systemctl disable indimail > /dev/null 2>&1
	  systemctl enable svscan > /dev/null 2>&1
  fi
fi
/bin/rm -f /lib/systemd/system/indimail.service
/bin/rm -f /usr/lib/systemd/system/indimail.service
if [ -d /var/log/indimail -a ! -d /var/log/svc ] ; then
	$mv /var/log/indimail /var/log/svc
	if [ $? -eq 0 ] ; then
		$sed -i 's{/var/log/indimail{/var/log/svc{' /service/*/log/run
	fi
fi
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
	check_update_if_diff /service/$i/variables/CERTDIR /etc/indimail/certs
	# increase for using dlmopen()
	if [ ! " $i" = " qmail-send.25" ] ; then
		check_update_if_diff /service/$i/variables/SOFT_MEM 536870912
	fi
	if [ "$i" = "qmail-send.25" ] ; then
		continue
	fi
	if [ ! -f /service/$i/variables/DISABLE_PLUGIN ] ; then
	echo > /service/$i/variables/DISABLE_PLUGIN
	fi
done

# service qmail-spamlog has been renamed to qmail-logfifo
# fifo is now /tmp/logfifo instead of /tmp/spamfifo
if [ -d /service/qmail-spamlog ] ; then
	/bin/rm -rf /service/qmail-spamlog
	/usr/sbin/svctool --fifologger=/tmp/logfifo --servicedir=/service
fi

# for bogofilter to send back X-Bogosity back to qmail-smtpd as well as log entry
# to /var/log/svc/logfifo/current (fifologger service)
# for qmail-send it is required if you run bogofilter during remote/local delivery,
# in which case it will be logged to /var/log/svc/logfifo/current
for i in qmail-smtpd.25 qmail-smtpd.465 fetchmail qmail-send.25
do
	if [ -d /service/$i -a -s /service/$i/variables/LOGFILTER ] ; then
		check_update_if_diff /service/$i/variables/LOGFILTER /tmp/logfifo
	fi
done
if [ -s /service/qmail-send.25/variables/QMAILLOCAL ] ; then
	check_update_if_diff /service/qmail-send.25/variables/QMAILLOCAL /usr/sbin/qmail-local
fi
if [ -s /service/qmail-send.25/variables/QMAILREMOTE ] ; then
	check_update_if_diff /service/qmail-send.25/variables/QMAILREMOTE /usr/sbin/qmail-remote
fi
if [ -s /etc/indimail/control/defaultqueue/LOGFILTER ] ; then
	check_update_if_diff /etc/indimail/control/defaultqueue/LOGFILTER /tmp/logfifo
fi
#
# tcpserver uses -c option to set concurrency and uses MAXDAEMON config file
# on sighup, since tcpserver is no longer root, it is unable to read MAXDAEMON config
# file. Better solution is to move MAXDAEMON config file out of /service/*/variables
# directory
for i in /service/qmail-*qm?pd.* /service/qmail-smtpd.*
do
	$chown root:indimail $i/variables
	$chmod 755 $i/variables
done
host=`uname -n`
check_update_if_diff /service/qmail-send.25/variables/DEFAULT_DOMAIN $host
check_update_if_diff /etc/indimail/control/envnoathost $host
check_update_if_diff /etc/indimail/control/defaulthost $host

# qmail-greyd, greydaemon path changed to /usr/sbin
$sed -i 's{/bin/qmail-greyd{/sbin/qmail-greyd{' /service/greylist.1999/run
}

case $1 in
	post|posttrans)
	do_post_upgrade
	;;
esac
