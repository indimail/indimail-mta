#!/bin/sh
# $Log: qlocal_upgrade.sh,v $
# Revision 1.37  2020-09-25 21:43:56+05:30  Cprogrammer
# remove libwatch service
#
# Revision 1.36  2020-07-30 11:29:43+05:30  Cprogrammer
# reverted interpreter back to /bin/sh
#
# Revision 1.35  2020-07-11 22:07:55+05:30  Cprogrammer
# removed svscan STATUSFILE
#
# Revision 1.34  2020-05-24 23:55:25+05:30  Cprogrammer
# fix permission for certs
#
# Revision 1.33  2020-04-27 22:11:27+05:30  Cprogrammer
# added install routine
#
# Revision 1.32  2020-04-11 08:38:02+05:30  Cprogrammer
# use /usr/bin/sh to suppress rpmlint errors
#
# Revision 1.31  2020-03-23 16:18:28+05:30  Cprogrammer
# changed --fixmysql_libs to --fixsharedlibs
#
# Revision 1.30  2020-03-16 22:39:12+05:30  Cprogrammer
# disable freshclam svscan service if systemd freshclam is enabled
#
# Revision 1.29  2019-10-27 19:22:59+05:30  Cprogrammer
# removed svctool --config=foxhole since freshclam now updates foxhole on startup
#
# Revision 1.28  2019-10-01 14:06:53+05:30  Cprogrammer
# use svctool to update libindimail, mysql_lib control files
#
# Revision 1.27  2019-06-17 18:17:15+05:30  Cprogrammer
# update with mysql_lib control file with either libmsyqlclient or libmariadbclient
#
# Revision 1.26  2019-06-16 19:12:53+05:30  Cprogrammer
# look for libmariadb in addition to libmysqlclient
#
# Revision 1.25  2019-06-07 19:19:28+05:30  Cprogrammer
# set mysql_lib control file
#
# Revision 1.24  2019-05-27 12:36:48+05:30  Cprogrammer
# create libindimail control file
#
# Revision 1.23  2019-05-26 11:00:27+05:30  Cprogrammer
# create /etc/indimail/control/mysql_lib control file
#
# Revision 1.22  2019-05-24 14:13:58+05:30  Cprogrammer
# create /etc/indimail/control/cache directory
#
# Revision 1.21  2019-04-21 11:57:50+05:30  Cprogrammer
# create MYSQL_LIB for dynamically loading MySQL shared libs
#
# Revision 1.20  2018-10-31 07:42:34+05:30  Cprogrammer
# migrate clamd.conf to scan.conf
#
# Revision 1.19  2018-10-31 00:21:55+05:30  Cprogrammer
# create scan.conf from clamd.conf
#
# Revision 1.18  2018-10-29 21:48:56+05:30  Cprogrammer
# fix for missing ln -r option in CentOS6
#
# Revision 1.17  2018-09-10 13:02:27+05:30  Cprogrammer
# move tcpserver tcp access files to /etc/indimail/tcp
#
# Revision 1.16  2018-09-10 12:22:45+05:30  Cprogrammer
# create directory /etc/indimail/tcp and move tcpserver access control files
#
# Revision 1.15  2018-09-02 14:25:02+05:30  Cprogrammer
# fixed syntax error
#
# Revision 1.14  2018-07-15 13:58:08+05:30  Cprogrammer
# create env variable ROUTE_NULL_USER, LOCK_LOGS for qmail-send
#
# Revision 1.13  2018-07-03 11:01:27+05:30  Cprogrammer
# update envnoathost, defaulthost, defaultdomain if hostname has changed
#
# Revision 1.12  2018-06-30 19:00:22+05:30  Cprogrammer
# added check for apache group and clamd.conf
#
# Revision 1.11  2018-06-25 08:47:11+05:30  Cprogrammer
# removed creation of getdns-root.key
#
# Revision 1.10  2018-06-21 23:10:37+05:30  Cprogrammer
# check for /usr/sbin/unbound-anchor
#
# Revision 1.9  2018-05-20 23:14:29+05:30  Cprogrammer
# create getdns-root.key
#
# Revision 1.8  2018-05-18 19:40:01+05:30  Cprogrammer
# added --config=foxhole to create foxhole_all.cdb for clamd
#
# Revision 1.7  2018-03-25 20:05:19+05:30  Cprogrammer
# removed chmod of variables directory as it is redundant now with read perm for indimail group
#
# Revision 1.6  2018-02-18 22:23:31+05:30  Cprogrammer
# pass argument to do_post_upgrade()
#
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
# $Id: qlocal_upgrade.sh,v 1.37 2020-09-25 21:43:56+05:30 Cprogrammer Exp mbhangui $
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
cp=$(which cp)

check_update_if_diff()
{
	val=`cat $1 2>/dev/null`
	if [ ! " $val" = " $2" ] ; then
		echo $2 > $1
	fi
}

do_install()
{
date
echo "Running $1 $Id: qlocal_upgrade.sh,v 1.37 2020-09-25 21:43:56+05:30 Cprogrammer Exp mbhangui $"
# upgrade libindimail (VIRTUAL_PKG_LIB) for dynamic loading of libindimail
# upgrade libmysqlclient path in /etc/indimail/control/mysql_lib
/usr/sbin/svctool --fixsharedlibs
}

do_post_upgrade()
{
date
echo "Running $1 $Id: qlocal_upgrade.sh,v 1.37 2020-09-25 21:43:56+05:30 Cprogrammer Exp mbhangui $"
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
# move tcpserver access control to /etc/indimail/tcp
if [ ! -d /etc/indimail/tcp ] ; then
	$mkdir -p /etc/indimail/tcp
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$chown indimail:qmail /etc/indimail/tcp
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$chmod 2775 /etc/indimail/tcp
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$mv /etc/indimail/tcp.* /etc/indimail/tcp > /dev/null
else
	$mv /etc/indimail/tcp.* /etc/indimail/tcp > /dev/null 2>&1
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
	# move tlshosts, notlshosts directory to certs
	if [ -d /etc/indimail/control/$i -a ! -L /etc/indimail/control/$i ] ; then
		$mv /etc/indimail/control/$i /etc/indimail/certs/$i
		if [ $? -ne 0 ] ; then
			exit 1
		fi
		cd /etc/indimail/control
		if [ $? -eq 0 ] ; then
			$ln ../certs/$i $i
		fi
	fi
done
# remove clientcert.pem link to servercert.pem in control directory
if [ -L /etc/indimail/control/clientcert.pem ] ; then
	$rm -f /etc/indimail/control/clientcert.pem
fi
if [ ! -f /etc/indimail/certs/clientcert.pem -a ! -L /etc/indimail/certs/clientcert.pem ] ; then
	cd /etc/indimail/certs
	$ln -s servercert.pem clientcert.pem
fi

getent group apache > /dev/null
if [ $? -ne 2 ] ; then
	for i in servercert.pem dh2048.pem rsa2048.pem dh1024.pem rsa1024.pem dh512.pem rsa512.pem
	do
		# roundcube (php) will require read access to certs
		if [ -f /etc/indimail/certs/$i ] ; then
			$chgrp qmail /etc/indimail/certs/$i
		fi
	done
fi
cd /etc/indimail/control
if [ $? -eq 0 ] ; then
	if [ ! -f servercert.pem ] ; then
	$ln -sf ../certs/servercert.pem servercert.pem
	fi
	if [ ! -f clientcert.pem ] ; then
	$ln -sf ../certs/servercert.pem clientcert.pem
	fi
fi
# Certificate location changed from /etc/indimail/control to /etc/indimail/certs
for i in qmail-smtpd.25 qmail-smtpd.465 qmail-smtpd.587 qmail-send.25
do
	check_update_if_diff /service/$i/variables/CERTDIR /etc/indimail/certs
	# increase for using dlmopen()
	if [ ! " $i" = " qmail-send.25" ] ; then
		check_update_if_diff /service/$i/variables/SOFT_MEM 536870912
	fi
	if [ "$i" = "qmail-send.25" ] ; then
		if [ -f /etc/indimail/control/virtualdomains -a ! -f /service/$i/variables/ROUTE_NULL_USER ] ; then
			echo > /service/$i/variables/ROUTE_NULL_USER
		fi
		if [ ! -f /service/$i/variables/LOCK_LOGS ] ; then
			echo > /service/$i/variables/LOCK_LOGS
		fi
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

host=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || `uname -n`)
echo $host | grep "\." > /dev/null
if [ $? -eq 0 ] ; then
	check_update_if_diff /etc/indimail/control/envnoathost $host
	check_update_if_diff /etc/indimail/control/defaultdomain $host
	check_update_if_diff /etc/indimail/control/defaulthost $host
fi

# qmail-greyd, greydaemon path changed to /usr/sbin
$sed -i 's{/bin/qmail-greyd{/sbin/qmail-greyd{' /service/greylist.1999/run

# remove STATUSFILE as .svlock serves the same purpose
rm -f /service/.svscan/variables/STATUSFILE

# copy updated cron entries
if [ -f /etc/indimail/cronlist.q -a -d /etc/cron.d ] ; then
	diff /etc/indimail/cronlist.q /etc/cron.d/cronlist.q >/dev/null 2>&1
	if [ $? -ne 0 ] ; then
		$cp /etc/indimail/cronlist.q /etc/cron.d/cronlist.q
	fi
fi
# migrate clamd.conf to scan.conf
if [ -f /etc/indimail/clamd.conf -a ! -f /etc/indimail/scan.conf ] ; then
	$mv /etc/indimail/clamd.conf /etc/indimail/scan.conf
fi
if [ -d /etc/clamd.d -a -f /etc/indimail/scan.conf ] ; then
	/bin/rm -f clamd.conf
	if [ ! -f /etc/clamd.d/scan.conf -a ! -L /etc/clamd.d/scan.conf ] ; then
		cd /etc/clamd.d
		$ln -sf /etc/indimail/scan.conf
	fi
fi
if [ -x /bin/systemctl -o -x /usr/bin/systemctl ] ; then
	systemctl is-enabled clamav-freshclam >/dev/null 2>&1
	if [ $? -eq 0 ] ; then
		touch /service/freshclam/down
	else
		/bin/rm -f /service/freshclam/down
	fi
fi

# upgrade libindimail (VIRTUAL_PKG_LIB) for dynamic loading of libindimail
# upgrade libmysqlclient path in /etc/indimail/control/mysql_lib
/usr/sbin/svctool --fixsharedlibs
if [ -d /service/libwatch ] ; then
	mv /service/libwatch /service/.libwatch
	svc -dx /service/.libwatch /service/.libwatch/log
	sleep 1
	/bin/rm -rf /service/.libwatch
fi

# for surbl
if [ ! -d /etc/indimail/control/cache ] ; then
	$mkdir /etc/indimail/control/cache
	$chown indimail:qmail /etc/indimail/control/cache
	$chmod 2775 /etc/indimail/control/cache
fi
}

case $1 in
	post|posttrans)
	do_post_upgrade $1
	;;
	install)
	do_install $1
	;;
esac
