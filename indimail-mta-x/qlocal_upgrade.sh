#
# $Id: qlocal_upgrade.sh,v 1.64 2024-02-22 09:10:41+05:30 Cprogrammer Exp mbhangui $
#
PATH=/bin:/usr/bin:/usr/sbin:/sbin
chown=$(which chown)
chgrp=$(which chgrp)
ln=$(which ln)
uname=$(which uname)
chmod=$(which chmod)
mkdir=$(which mkdir)
cpio=$(which cpio)
rm=$(which rm)
mv=$(which mv)
sed=$(which sed)
cp=$(which cp)
servicedir=@servicedir@
sysconfdir=@sysconfdir@

check_update_if_diff()
{
	val=$(qcat $1 2>/dev/null)
	if [ ! " $val" = " $2" ] ; then
		echo $2 > $1
	fi
}

do_install()
{
echo "Running $1 $Id: qlocal_upgrade.sh,v 1.64 2024-02-22 09:10:41+05:30 Cprogrammer Exp mbhangui $"
# upgrade libindimail (VIRTUAL_PKG_LIB) for dynamic loading of libindimail
# upgrade libmysqlclient path in /etc/indimail/control/libmysql
/usr/sbin/svctool --fixsharedlibs
}

do_post_upgrade()
{
echo "Running $1 $Id: qlocal_upgrade.sh,v 1.64 2024-02-22 09:10:41+05:30 Cprogrammer Exp mbhangui $"
if [ -x /bin/systemctl -o -x /usr/bin/systemctl ] ; then
	systemctl is-enabled svscan >/dev/null 2>&1
	if [ $? -ne 0 ] ; then
		systemctl disable indimail > /dev/null 2>&1
		systemctl enable svscan > /dev/null 2>&1
	fi
fi
/bin/rm -f /lib/systemd/system/indimail.service
/bin/rm -f /usr/lib/systemd/system/indimail.service
if [ -d /run ] ; then
	rundir=/run
elif [ -d /private/var/run ] ; then
	rundir=/private/var/run
elif [ -d /var/run ] ; then
	rundir=/var/run
else
	rundir=""
fi
# migrate from old style /service
if [ "$servicedir" != "/service" ] && [ ! -d $servicedir ] && [ -d /service -o -L /service ] ; then
	$mkdir -p $servicedir
	if [ $? -eq 0 ] ; then
		cd /service
		if [ -n "$cpio" ] ; then
			find . -depth -print | $cpio -pdm $servicedir \
				&& $mv /service /service.old
		else
			$cp -rp /service $servicedir \
				&& $mv /service /service.old
		fi
		if [ $? -eq 0 ] ; then
			$sed -i \
  			-e 's{ /service{ @servicedir@{' \
  			-e 's{"/service{"@servicedir@{' \
				$servicedir/*/run $servicedir/*/log/run \
				$servicedir/*/variables/.options
			if [ -n "$rundir" ] ; then
				$ln -s $rundir/svscan /service
			else
				$ln -s $servicedir /service
			fi
		fi
	fi
fi
# migrate from old /var/log/indimail to /var/log/svc
if [ -d /var/log/indimail -a ! -d /var/log/svc ] ; then
	$mv /var/log/indimail /var/log/svc
	if [ $? -eq 0 ] ; then
		$sed -i 's{/var/log/indimail{@logdir@{' $servicedir/*/log/run
	fi
fi
#
# certs were in /etc/indimail/control
# they have been moved to /etc/indimail/certs
#
if [ ! -d $sysconfdir/certs ] ; then
	$mkdir -p $sysconfdir/certs
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$chown indimail:qmail $sysconfdir/certs
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$chmod 2775 $sysconfdir/certs
	if [ $? -ne 0 ] ; then
		exit 1
	fi
fi
# move tcpserver access control to /etc/indimail/tcp
if [ ! -d $sysconfdir/tcp ] ; then
	$mkdir -p $sysconfdir/tcp
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$chown indimail:qmail $sysconfdir/tcp
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$chmod 2775 $sysconfdir/tcp
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	$mv $sysconfdir/tcp.* $sysconfdir/tcp > /dev/null
else
	$mv $sysconfdir/tcp.* $sysconfdir/tcp > /dev/null 2>&1
fi
# move existing certs in control directory to $sysconfdir/certs
for i in servercert.pem clientcert.pem dh1024.pem dh512.pem \
	rsa2048.pem dh2048.pem rsa512.pem couriersslcache servercert.cnf \
	servercert.rand tlshosts notlshosts
do
	if [ -f $sysconfdir/control/$i -a ! -L $sysconfdir/control/$i ] ; then
		$mv $sysconfdir/control/$i $sysconfdir/certs/$i
		if [ $? -ne 0 ] ; then
			exit 1
		fi
	fi
	# move tlshosts, notlshosts directory to certs
	if [ -d $sysconfdir/control/$i -a ! -L $sysconfdir/control/$i ] ; then
		$mv $sysconfdir/control/$i $sysconfdir/certs/$i
		if [ $? -ne 0 ] ; then
			exit 1
		fi
		cd $sysconfdir/control
		if [ $? -eq 0 ] ; then
			$ln ../certs/$i $i
		fi
	fi
done
# remove clientcert.pem link to servercert.pem in control directory
if [ -L $sysconfdir/control/clientcert.pem ] ; then
	$rm -f $sysconfdir/control/clientcert.pem
fi
if [ ! -f $sysconfdir/certs/clientcert.pem -a ! -L $sysconfdir/certs/clientcert.pem ] ; then
	cd $sysconfdir/certs
	$ln -s servercert.pem clientcert.pem
fi

t=$(getent group qcerts)
if [ $? -eq 2 ] ; then
	groupadd qcerts
	usermod -aG qcerts qmaild
	usermod -aG qcerts qmailr
	/usr/bin/getent group apache > /dev/null && /usr/sbin/usermod -aG qcerts apache
	chgrp -R qcerts $sysconfdir/certs
	chgrp -R qcerts $sysconfdir/control/domainkeys
else
	t1=0
	t2=0
	t3=0
	t4=0
	for t in $(echo $t | cut -d: -f4 | $sed -e 's{,{ {g')
	do
		if [ "$t" = "qmailr" ] ; then
			t1=1
		elif [ "$t" = "qmaild" ] ; then
			t2=1
		elif [ "$t" = "qmails" ] ; then
			t3=1
		elif [ "$t" = "apache" ] ; then
			t4=1
		fi
	done
	if [ $t1 -eq 0 ] ; then
		echo usermod -aG qcerts qmailr
		usermod -aG qcerts qmailr
	fi
	if [ $t2 -eq 0 ] ; then
		echo usermod -aG qcerts qmaild
		usermod -aG qcerts qmaild
	fi
	if [ $t3 -eq 0 ] ; then
		echo usermod -aG qcerts qmails
		usermod -aG qcerts qmails
	fi
	if [ $t4 -eq 0 ] ; then
		/usr/bin/getent group apache > /dev/null && usermod -aG qcerts apache
	fi
fi

for i in rsa dh
do
	for j in 512 1024 2048 4096
	do
	if [ -f $sysconfdir/certs/$i"$j".pem ] ; then
		$chgrp qcerts $sysconfdir/certs/$i"$j".pem
	fi
	done
done
for i in servercert.cnf servercert.pem servercert.rand
do
	if [ -f $sysconfdir/certs/$i ] ; then
		$chgrp qcerts $sysconfdir/certs/$i
	fi
done

cd $sysconfdir/control
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
	check_update_if_diff $servicedir/$i/variables/CERTDIR $sysconfdir/certs
	# increase for using dlmopen()
	if [ ! " $i" = " qmail-send.25" ] ; then
		check_update_if_diff $servicedir/$i/variables/SOFT_MEM 536870912
	fi
	if [ "$i" = "qmail-send.25" ] ; then
		if [ -f $sysconfdir/control/virtualdomains -a ! -f $servicedir/$i/variables/ROUTE_NULL_USER ] ; then
			echo > $servicedir/$i/variables/ROUTE_NULL_USER
		fi
		/bin/rm -f $servicedir/$i/variables/LOCK_LOGS
		if [ ! -f $servicedir/$i/variables/LOGLOCK ] ; then
			echo > $servicedir/$i/variables/LOGLOCK
		fi
		continue
	fi
	if [ ! -f $servicedir/$i/variables/DISABLE_PLUGIN ] ; then
	echo > $servicedir/$i/variables/DISABLE_PLUGIN
	fi
done

# service qmail-spamlog has been renamed to qmail-logfifo
# fifo is now /tmp/logfifo instead of /tmp/spamfifo
if [ -d /run ] ; then
	logfifo="/run/indimail/logfifo"
	$mkdir -p /run/indimail
elif [ -d /var/run ] ; then
	logfifo="/var/run/indimail/logfifo"
	$mkdir -p /var/run/indimail
else
	logfifo="/tmp/logfifo"
fi
if [ -d $servicedir/qmail-spamlog ] ; then
	/bin/rm -rf $servicedir/qmail-spamlog
	/usr/sbin/svctool --fifologger=$logfifo --servicedir=$servicedir
fi

# for bogofilter to send back X-Bogosity back to qmail-smtpd as well as log entry
# to /var/log/svc/logfifo/current (fifologger service)
# for qmail-send it is required if you run bogofilter during remote/local delivery,
# in which case it will be logged to /var/log/svc/logfifo/current
for i in qmail-smtpd.25 qmail-smtpd.465 fetchmail qmail-send.25
do
	if [ -d $servicedir/$i -a -s $servicedir/$i/variables/LOGFILTER ] ; then
		check_update_if_diff $servicedir/$i/variables/LOGFILTER $logfifo
	fi
done
if [ -s $sysconfdir/control/defaultqueue/LOGFILTER ] ; then
	check_update_if_diff $sysconfdir/control/defaultqueue/LOGFILTER $logfifo
fi
if [ ! -f $sysconfdir/control/defaultqueue/DYNAMIC_QUEUE ] ; then
	> $sysconfdir/control/defaultqueue/DYNAMIC_QUEUE
fi

default_host=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n)
default_domain=$(echo $default_host | $sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./')
echo $host | grep "\." > /dev/null
if [ $? -eq 0 ] ; then
	check_update_if_diff $sysconfdir/control/envnoathost   $default_host
	check_update_if_diff $sysconfdir/control/defaulthost   $default_host
	check_update_if_diff $sysconfdir/control/defaultdomain $default_domain
fi

# qmail-greyd, greydaemon path changed to /usr/sbin
$sed -i 's{/bin/qmail-greyd{/sbin/qmail-greyd{' $servicedir/greylist.1999/run

# qmail-daemon changed to qscheduler -s
$sed -i 's{/sbin/qmail-daemon{/sbin/qscheduler -s{' $servicedir/qmail-send.25/run

# qmail-smtpd.587 add secureauth
$sed -i 's{--forceauthsmtp --antispoof{--forceauthsmtp --secureauth --antispoof{' \
		$servicedir/qmail-smtpd.587/variables/.options

# qmail-logfifo
$sed -i 's{qmail-cat{qcat{' $servicedir/qmail-logfifo/run

# remove STATUSFILE as .svscan.pid serves the same purpose
rm -f $servicedir/.svscan/variables/STATUSFILE
if [ ! -f $servicedir/.svscan/variables/AUTOSCAN -a -d $servicedir/.svscan/variables ] ; then
	echo 1 > $servicedir/.svscan/variables/AUTOSCAN
fi

# migrate clamd.conf to scan.conf
if [ -f $sysconfdir/clamd.conf -a ! -f $sysconfdir/scan.conf ] ; then
	$mv $sysconfdir/clamd.conf $sysconfdir/scan.conf
fi
if [ -d /etc/clamd.d -a -f $sysconfdir/scan.conf ] ; then
	/bin/rm -f clamd.conf
	if [ ! -f /etc/clamd.d/scan.conf -a ! -L /etc/clamd.d/scan.conf ] ; then
		cd /etc/clamd.d
		$ln -sf $sysconfdir/scan.conf
	fi
fi
if [ -x /bin/systemctl -o -x /usr/bin/systemctl ] ; then
	systemctl is-enabled clamav-freshclam >/dev/null 2>&1
	if [ $? -eq 0 ] ; then
		touch $servicedir/freshclam/down
	else
		/bin/rm -f $servicedir/freshclam/down
	fi
fi

# upgrade libindimail (VIRTUAL_PKG_LIB) for dynamic loading of libindimail
# upgrade libmysqlclient path in /etc/indimail/control/libmysql
if [ -f $sysconfdir/control/mysql_lib ] ; then
	$mv $sysconfdir/control/mysql_lib $sysconfdir/control/libmysql
fi
/usr/sbin/svctool --fixsharedlibs
if [ -d $servicedir/libwatch ] ; then
	$mv $servicedir/libwatch $servicedir/.libwatch
	/usr/bin/svc -dx $servicedir/.libwatch $servicedir/.libwatch/log
	sleep 1
	/bin/rm -rf $servicedir/.libwatch
fi

# for surbl
if [ ! -d $sysconfdir/control/cache ] ; then
	$mkdir $sysconfdir/control/cache
	$chown indimail:qmail $sysconfdir/control/cache
	$chmod 2775 $sysconfdir/control/cache
fi

# global variable directory
if [ ! -d $sysconfdir/control/global_vars ] ; then
	mkdir -p $sysconfdir/control/global_vars
	chown root:qmail $sysconfdir/control/global_vars
	chmod 755 $sysconfdir/control/global_vars
fi
for i in $(echo $servicedir/*/variables)
do
	if [ ! -f $i/.envdir -a ! -d $i/.envdir -a ! -L $i/.envdir ] ; then
		echo $sysconfdir/control/global_vars > $i/.envdir
	fi
done
for i in $(grep -l fsync /service/*/variables/.options)
do
	$sed -i -e 's{--fsync{--fdatasync{g' $i
done
for i in $(ls -d /service/*-send*/variables)
do
	grep -- '--setgroups' $i/.options > /dev/null
	if [ $? -ne 0 ] ; then
		t="$(qcat $i/.options)"
		echo "$t --setgroups" > $i/.options
	fi
	if [ ! -f $i/USE_SETGROUPS ] ; then
		echo 1 > $i/USE_SETGROUPS
	fi
	grep -- '--setuser-priv' $i/.options > /dev/null
	if [ $? -ne 0 ] ; then
		t="$(qcat $i/.options)"
		echo "$t --setuser-priv" > $i/.options
	fi
	if [ ! -f $i/SETUSER_PRIVILEGES ] ; then
		echo 1 > $i/SETUSER_PRIVILEGES
	fi
done
if  [ -f $sysconfdir/control/tlsclientciphers ] ; then
	$mv $sysconfdir/control/tlsclientciphers $sysconfdir/control/clientcipherlist
fi
if  [ -f $sysconfdir/control/tlsserverciphers ] ; then
	$mv $sysconfdir/control/tlsserverciphers $sysconfdir/control/servercipherlist
	if [ $? -eq 0 -a -f $sysconfdir/control/global_vars/TLS_CIPHER_LIST ] ; then
		ln -sf $sysconfdir/control/servercipherlist \
			$sysconfdir/control/global_vars/TLS_CIPHER_LIST
	fi
fi

# copy updated cron entries
if [ -f /etc/cron.d/cronlist.q ] ; then
  $mv /etc/cron.d/cronlist.q /etc/cron.d/indimail-mta.cron
fi
if [ ! -f @sysconfdir@/indimail-mta.cron -a -d /etc/cron.d ] ; then
	$cp @sysconfdir@/indimail-mta.cron /etc/cron.d/indimail-mta.cron
fi

if [ -f @sysconfdir@/control/filterargs ] ; then
	$sed -i 's{/bin/cat{@prefix@/bin/qcat{' @sysconfdir@/control/filterargs
	out=$(/usr/bin/mktemp -t qlocalXXXXXXXXX)
	exec 3>$out
	qcat @sysconfdir@/control/filterargs | while read line
	do
		echo $line | grep dk-filter >/dev/null
		if [ $? -ne 0 ] ; then
			echo "$line" 1>&3
			continue;
		fi
		echo $line | grep DKIMQUEUE >/dev/null
		if [ $? -eq 0 ] ; then
			line=$(echo $line | sed -e 's{bin/dk-filter{sbin/qmail-dkim{g' -e 's{/bin/cat{@prefix@/bin/qcat{g') 
		else
			line=$(echo $line | sed -e 's{bin/dk-filter{sbin/qmail-dkim{g' -e 's{${,DKIMQUEUE=@prefix@/bin/qcat{')
		fi
		echo "$line" 1>&3
	done
	diff $out @sysconfdir@/control/filterargs >/dev/null
	if [ $? -eq 0 ] ; then
		/bin/rm -f $out
	else
		echo "Updated @sysconfdir@/control/filterargs"
		mv $out @sysconfdir@/control/filterargs
	fi
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
#
# $Log: qlocal_upgrade.sh,v $
# Revision 1.64  2024-02-22 09:10:41+05:30  Cprogrammer
# replace qmail-cat with qcat for qmail-logfifo service and filterargs control file
#
# Revision 1.63  2024-02-22 01:05:00+05:30  Cprogrammer
# replace cat with qcat
#
# Revision 1.62  2024-01-09 12:38:01+05:30  Cprogrammer
# replace dk-filter with qmail-dkim
#
# Revision 1.61  2024-01-06 11:07:19+05:30  Cprogrammer
# add supplementary group qcerts for qmails user
#
# Revision 1.60  2023-12-29 21:53:04+05:30  Cprogrammer
# fixed qlocal_upgrade
#
# Revision 1.59  2023-12-23 00:16:10+05:30  Cprogrammer
# renamed cronlist.q to indimail-mta.cron
#
# Revision 1.58  2023-12-05 23:59:42+05:30  Cprogrammer
# removed force update of QMAILLOCAL, QMAILREMOTE for qmail-send service
#
# Revision 1.57  2023-10-16 18:41:20+05:30  Cprogrammer
# add --secureauth option for SMTP service on port 587
#
# Revision 1.56  2023-09-20 08:22:42+05:30  Cprogrammer
# append --setuser-priv svctool option to .options
#
# Revision 1.55  2023-08-19 11:33:26+05:30  Cprogrammer
# move tlsclientciphers to clientcipherlist
# move tlsserverciphers to servercipherlist
#
# Revision 1.54  2023-06-08 23:18:30+05:30  Cprogrammer
# removed date command
#
# Revision 1.53  2023-03-30 07:53:11+05:30  Cprogrammer
# upgrade qmail-send, slowq-send service to use --setgroups
#
# Revision 1.52  2023-02-14 08:40:32+05:30  Cprogrammer
# added group ID qcerts
# fix permissions of existing certificates to group permissions qcerts
# added qcerts as supplementary group for qmaild, qmailr and apache
#
# Revision 1.51  2022-12-02 10:45:09+05:30  Cprogrammer
# add AUTOSCAN env variable for svscan
#
# Revision 1.50  2022-11-23 15:07:42+05:30  Cprogrammer
# rename mysql_lib to libmysql on upgrade
#
# Revision 1.49  2022-04-06 09:21:24+05:30  Cprogrammer
# replace --fsync with --fdatasync
#
# Revision 1.48  2022-01-30 08:37:23+05:30  Cprogrammer
# replaced qmail-daemon with qscheduler
#
# Revision 1.47  2021-08-23 17:34:36+05:30  Cprogrammer
# fixed default_domain variable
#
# Revision 1.46  2021-07-13 00:24:35+05:30  Cprogrammer
# fixed typo for servicedir
#
# Revision 1.45  2021-07-12 18:23:00+05:30  Cprogrammer
# create service/*/variables/.envdir file for control/global_vars
#
# Revision 1.44  2021-07-10 23:18:22+05:30  Cprogrammer
# removed updation of cronlist
#
# Revision 1.43  2021-06-27 11:35:09+05:30  Cprogrammer
# renamed LOCK_LOGS env variable to LOGLOCK
#
# Revision 1.42  2021-01-23 17:07:59+05:30  Cprogrammer
# fixed logfifo path
#
# Revision 1.41  2020-10-09 17:30:18+05:30  Cprogrammer
# migration only when servicedir != /service
#
# Revision 1.40  2020-10-09 12:47:30+05:30  Cprogrammer
# use variables from conf-* files
#
# Revision 1.39  2020-10-09 11:26:37+05:30  Cprogrammer
# use servicedir from Makefile
#
# Revision 1.38  2020-10-08 22:55:18+05:30  Cprogrammer
# use variables from Makefile
#
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
