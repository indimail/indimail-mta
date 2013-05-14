# chkconfig: 345 14 91
# description: Starts qmail system and associated services
### BEGIN INIT INFO
# Provides:          indimail
# Required-Start:    $network
# Required-Stop:
# Default-Start:     3 4 5
# Default-Stop:      0 1 6
# Description:       Starts indimail system and associated services
# Short-Description: Start/Stop indimail
### END INIT INFO

# $Log: qmailctl.sh,v $
# Revision 1.39  2013-05-15 00:44:07+05:30  Cprogrammer
# fix for systems having inittab
#
# Revision 1.38  2013-05-07 15:51:52+05:30  Cprogrammer
# startup/shutdown script for non-indimail system
#
# Revision 1.37  2011-10-23 08:56:40+05:30  Cprogrammer
# qmail-qstat removed as qmail-qread has the functionality
#
# Revision 1.36  2011-07-22 19:05:23+05:30  Cprogrammer
# fixed service getting disabled on systems with systemctl
#
# Revision 1.35  2011-07-21 13:17:44+05:30  Cprogrammer
# added systemd support
#
# Revision 1.34  2011-05-28 21:25:04+05:30  Cprogrammer
# play nicely with upstart job control
#
# Revision 1.33  2011-05-26 22:37:51+05:30  Cprogrammer
# fix for debian systems
#
# Revision 1.32  2011-04-27 17:18:26+05:30  Cprogrammer
# added LSB header
#
# Revision 1.31  2011-02-07 22:09:29+05:30  Cprogrammer
# added case for ubuntu
#
# Revision 1.30  2010-07-27 10:55:23+05:30  Cprogrammer
# display status of log services also
#
# Revision 1.29  2010-07-26 20:09:35+05:30  Cprogrammer
# show status of all services in service directory
#
# Revision 1.28  2010-07-21 16:26:37+05:30  Cprogrammer
# fixed typo (/var/lock/subsys)
#
# Revision 1.27  2010-07-09 14:49:22+05:30  Cprogrammer
# added portable echo
#
# Revision 1.26  2010-07-07 10:18:10+05:30  Cprogrammer
# stop and start all services
#
# Revision 1.25  2010-07-06 13:16:31+05:30  Cprogrammer
# stop svscan on stop
#
# Revision 1.24  2010-07-02 16:20:22+05:30  Cprogrammer
# shutdown indimail early
#
# Revision 1.23  2010-06-24 16:20:46+05:30  Cprogrammer
# made messages fedora boot style
#
# Revision 1.22  2010-06-11 19:37:54+05:30  Cprogrammer
# added options for qmailctl to be installed as SYSV style startup script
#
# Revision 1.21  2010-05-28 11:52:48+05:30  Cprogrammer
# fix for Mac OS X
#
# Revision 1.20  2010-04-16 09:09:03+05:30  Cprogrammer
# added description for qmtp in usage
#
# Revision 1.19  2010-04-07 20:20:20+05:30  Cprogrammer
# change ownership of cdb files to indimail
#
# Revision 1.18  2010-04-03 16:48:00+05:30  Cprogrammer
# added option to create tcp.qmqp.cdb
#
# Revision 1.17  2010-03-26 15:59:24+05:30  Cprogrammer
# create tcp.qmtp.cdb
#
# Revision 1.16  2010-03-08 15:12:41+05:30  Cprogrammer
# removed duplicate options doqueue, alrm, hup
#
# Revision 1.15  2010-03-08 15:05:40+05:30  Cprogrammer
# removed redundant functions
#
# Revision 1.14  2009-11-12 15:07:15+05:30  Cprogrammer
# check return status of tcprules
#
# Revision 1.13  2009-11-11 13:33:53+05:30  Cprogrammer
# build cdb files matching wildcards tcp*.smtp, tcp*.imap, tcp*.pop3
#
# Revision 1.12  2009-08-15 20:36:57+05:30  Cprogrammer
# added poppass
#
# Revision 1.11  2009-08-13 18:35:51+05:30  Cprogrammer
# *** empty log message ***
#
# Revision 1.10  2009-06-17 14:14:51+05:30  Cprogrammer
# fix for mac os
#
# Revision 1.9  2009-03-08 10:19:12+05:30  Cprogrammer
# changes for non-redhat systems
#
# Revision 1.8  2008-08-14 14:51:30+05:30  Cprogrammer
# added cdb for imap and pop3
#
# Revision 1.7  2008-08-03 18:26:09+05:30  Cprogrammer
# hack for mac OS X
#
# Revision 1.6  2004-01-20 06:54:00+05:30  Cprogrammer
# renamed VPOPMAILDIR to INDIMAILDIR
#
# Revision 1.5  2003-07-30 19:07:51+05:30  Cprogrammer
# changed default user to indimail
#
# Revision 1.4  2003-07-11 20:30:15+05:30  Cprogrammer
# start or stop all smtp services
#
# Revision 1.3  2002-11-24 20:10:02+05:30  Cprogrammer
# corrected setting of PATH variable
# changed path of tcp.smtp to /var/vpopmail/etc
#
# Revision 1.2  2002-09-08 23:49:17+05:30  Cprogrammer
# made qmail home dependency on conf-qmail
#
# Revision 1.1  2002-08-20 23:42:57+05:30  Cprogrammer
# Initial revision
#
#
SERVICE=/service
#
#
#
if [ -f /etc/lsb-release -o -f /etc/debian_version ] ; then
	SYSTEM=Debian
else
	SYSTEM=`uname -s | tr "[:lower:]" "[:upper:]"`
fi
if [ -x QMAIL/bin/echo ] ; then
	ECHO=QMAIL/bin/echo
else
	ECHO=echo
fi
case "$SYSTEM" in
	DARWIN*|Debian)
		RES_COL=60
		MOVE_TO_COL="$ECHO -en \\033[${RES_COL}G"
		SETCOLOR_SUCCESS="$ECHO -en \\033[1;32m"
		SETCOLOR_FAILURE="$ECHO -en \\033[1;31m"
		SETCOLOR_WARNING="$ECHO -en \\033[1;33m"
		SETCOLOR_NORMAL="$ECHO -en \\033[0;39m"
		;;
esac

myecho_success() {
  $MOVE_TO_COL
  $ECHO -n "["
  $SETCOLOR_SUCCESS
  $ECHO -n "  OK  "
  $SETCOLOR_NORMAL
  $ECHO -n "]"
  $ECHO -ne "\r"
  return 0
}

myecho_failure() {
  $MOVE_TO_COL
  $ECHO -n "["
  $SETCOLOR_FAILURE
  $ECHO -n $"FAILED"
  $SETCOLOR_NORMAL
  $ECHO -n "]"
  $ECHO -ne "\r"
  return 1
}

# Log that something succeeded
mysuccess() {
  myecho_success
  return 0
}

# Log that something failed
myfailure() {
  local rc=$?
  myecho_failure
  return $rc
}

# Check that we're a privileged user
[ `id -u` = 0 ] || exit 4
if [ -x /bin/systemctl ] ; then
	/bin/systemctl is-enabled indimail.service
	if [ $? -ne 0 ] ; then
		SYSTEMCTL_SKIP_REDIRECT=1
		if [ -f /etc/rc.d/init.d/functions ] ; then
			. /etc/rc.d/init.d/functions
		elif [ -f /etc/init.d/functions ] ; then
			. /etc/init.d/functions
		elif [ -f /lib/lsb/init-functions ] ; then
			. /lib/lsb/init-functions
		fi
		$ECHO -n $"indimail is disabled: "
		failure
		$ECHO -ne "\n"
		exit 1
	fi
fi
# Source function library.
if [ -f /etc/rc.d/init.d/functions ] ; then
	. /etc/rc.d/init.d/functions
elif [ -f /etc/init.d/functions ] ; then
	. /etc/init.d/functions
elif [ -f /lib/lsb/init-functions ] ; then
	. /lib/lsb/init-functions
fi
case "$SYSTEM" in
	DARWIN*)
	. /etc/rc.common
	CheckForNetwork
	if [ ${NETWORKUP} != "-YES-" ]
	then
		exit 0
	fi
	succ=mysuccess
	fail=myfailure
	;;
	Debian)
	succ=mysuccess
	fail=myfailure
	;;
	LINUX)
	succ=success
	fail=failure
	# Get config.
	if [ -f /etc/sysconfig/network ] ; then
		. /etc/sysconfig/network
		# Check that networking is up.
		if [ ${NETWORKING} = "no" ] ; then
			exit 0
		fi
	fi
	;;
	*)
	succ=mysuccess
	fail=myfailure
	;;
esac

PATH=$PATH:QMAIL/bin:QMAIL/sbin:/usr/local/bin:/usr/bin:/bin
export PATH
myhelp()
{
    /bin/cat <<HELP
  start -- starts mail service (smtp connection accepted, mail can go out)
   stop -- stops mail service (smtp connections refused, nothing goes out)
restart -- stops and restarts smtp, sends qmail-send a TERM & restarts it
   kill -- Send kill signal to tcpserver, supvervise, qmail-send processes.
   shut -- Shutdown entire indimail service.
  flush -- Schedules queued messages for immediate delivery
 reload -- sends qmail-send HUP, rereading locals and virtualdomains
 status -- status of svscan process & IndiMail services
  queue -- shows status of queue
  pause -- temporarily stops mail service (connections accepted, nothing leaves)
   cont -- continues paused mail service
    cdb -- rebuild the tcpserver cdb file for smtp, qmtp, qmqp, imap, pop3 and poppass
HELP
}

stop()
{
	local ret=0
	if [ -d /var/lock/subsys -a ! -f /var/lock/subsys/svscan ] ; then
		exit 0
	fi
	for i in `echo $SERVICE/*`
	do
		$ECHO -n $"Stopping $i: "
		QMAIL/bin/svc -d $i 2>>/tmp/sv.err && $succ || $fail
		RETVAL=$?
		echo
		let ret+=$RETVAL
	done
	if [ ! -f /bin/systemctl ] ; then
		$ECHO -n $"Stopping svscan: "
		if [ -f QMAIL/sbin/initsvc ] ; then
			QMAIL/sbin/initsvc -off > /dev/null 2>>/tmp/sv.err && $succ || $fail
		elif [ -f /sbin/initctl ] ; then
			/sbin/initctl stop svscan >/dev/null 2>>/tmp/sv.err && $succ || $fail
		else
  			/bin/grep "^SV:" /etc/inittab |/bin/grep svscan |/bin/grep respawn >/dev/null
			if [ $? -ne 0 ]; then
				/usr/bin/killall -e -w svscan && $succ || $fail
			else
				RETVAL=0
			fi
		fi
		RETVAL=$?
		echo
		if [ -s /tmp/sv.err ] ; then
			/bin/cat /tmp/sv.err
		fi
		/bin/rm -f /tmp/sv.err
	fi
	if [ -d /var/lock/subsys ] ; then
		[ $ret -eq 0 ] && rm -f /var/lock/subsys/svscan
	fi
	return $ret
}

start()
{
	local ret=0
	if [ -d /var/lock/subsys -a -f /var/lock/subsys/svscan ] ; then
		exit 0
	fi
	QMAIL/bin/svstat $SERVICE/.svscan/log > /dev/null
	if [ $? -ne 0 ] ; then
		$ECHO -n $"Starting svscan: "
		if [ -f QMAIL/sbin/initsvc ] ; then
			QMAIL/sbin/initsvc -on > /dev/null 2>/tmp/sv.err && $succ || $fail
		elif [ -f /sbin/initctl ] ; then
			/sbin/initctl start svscan >/dev/null 2>>/tmp/sv.err && $succ || $fail
		else
			if [ -w /dev/console ] ; then
				device=/dev/console
			else
				device=/dev/null
			fi
  			/bin/grep "^SV:" /etc/inittab |/bin/grep svscan |/bin/grep respawn >/dev/null
			if [ $? -ne 0 ]; then
				/bin/grep -v "svscan" /etc/inittab > /etc/inittab.svctool.$$
				echo "SV:345:respawn:QMAIL/bin/svscanboot $servicedir <>$device 2<>$device" >> /etc/inittab.svctool.$$
				if [ $? -eq 0 ] ; then
					/bin/mv /etc/inittab.svctool.$$ /etc/inittab
				fi
				create_svscan $servicedir
				/sbin/init q
			fi
		fi
		RETVAL=$?
		echo
		let ret+=$RETVAL
	else
		for i in `echo $SERVICE/*`
		do
			if [ ! -f $i/down ] ; then
				$ECHO -n $"Starting $i: "
				QMAIL/bin/svc -u $i 2>/tmp/sv.err && $succ || $fail
				RETVAL=$?
				echo
				let ret+=$RETVAL
			fi
		done
	fi
	if [ -s /tmp/sv.err ] ; then
		/bin/cat /tmp/sv.err
	fi
	/bin/rm -f /tmp/sv.err
	if [ -d /var/lock/subsys ] ; then
		[ $ret -eq 0 ] && touch /var/lock/subsys/svscan
	fi
	return $ret
}

case "$1" in
  stop)
	stop
	[ $? -eq 0 ] && exit 0 || exit 1
	;;
  start)
	start
	[ $? -eq 0 ] && exit 0 || exit 1
	;;
  restart|force-reload)
	stop
	RETVAL=$?
	let ret+=$RETVAL
	start
	RETVAL=$?
	let ret+=$RETVAL
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  condrestart|try-restart)
	ret=0
	for i in `echo $SERVICE/qmail-smtpd.* $SERVICE/qmail-qmqpd.* $SERVICE/qmail-qmtpd.*`
	do
		$ECHO -n $"Stopping $i: "
		QMAIL/bin/svc -d $i && $succ || $fail
		RETVAL=$?
		echo
		let ret+=$RETVAL
	done
	for i in `echo $SERVICE/qmail-send.*`
	do
		$ECHO -n $"Terminating $i: "
		QMAIL/bin/svc -t $i && $succ || $fail
		RETVAL=$?
		echo
		let ret+=$RETVAL
	done
	for i in `echo $SERVICE/qmail-smtpd.* $SERVICE/qmail-qmqpd.* $SERVICE/qmail-qmtpd.*`
	do
		if [ ! -f $i/down ] ; then
		$ECHO -n $"Starting $i: "
			QMAIL/bin/svc -u $i && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		fi
	done
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  shut)
	$ECHO -n $"shutdown svscan: "
	if [ -f QMAIL/sbin/initsvc ] ; then
		QMAIL/sbin/initsvc -off > /dev/null && $succ || $fail
	elif [ -f /sbin/initctl ] ; then
		/sbin/initctl stop svscan >/dev/null && $succ || $fail
	else
		/usr/bin/killall -e -w svscan && $succ || $fail
	fi
	ret=$?
	echo
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  kill)
	$ECHO -n $"killing tcpserver,supervise,qmail-send: "
	kill `ps -ef|/bin/egrep "tcpserver|supervise|qmail-send" | /bin/grep -v grep | awk '{print $2}'` && $succ || $fail
	ret=$?
	echo
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  flush)
	ret=0
	$ECHO -n $"Flushing timeout table + ALRM signal to qmail-send."
	QMAIL/bin/qmail-tcpok > /dev/null && $succ || $fail
	echo
	for i in `echo $SERVICE/qmail-send.*`
	do
		$ECHO -n $"Flushing $i: "
		QMAIL/bin/svc -a $i && $succ || $fail
		RETVAL=$?
		echo
		let ret+=$RETVAL
	done
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  status|stat)
	if [ -x /sbin/initctl ] ; then
		/sbin/initctl status svscan
	else
		ps -ef|/bin/grep svscanboot|/bin/grep -v grep
	fi
	RETVAL=$?
	QMAIL/bin/svstat $SERVICE/.svscan/log $SERVICE/* $SERVICE/*/log
	let ret+=$RETVAL
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  queue)
	QMAIL/bin/qmail-qread -c
	ret=$?
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  reload)
    ret=0
	for i in `echo $SERVICE/qmail-send.*`
	do
		$ECHO -n $"sending HUP signal to $i: "
		QMAIL/bin/svc -h $i && $succ || $fail
		RETVAL=$?
		echo
		let ret+=$RETVAL
	done
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  pause)
    ret=0
	for i in `echo $SERVICE/qmail-send.* $SERVICE/qmail-smtpd.*`
	do
		$ECHO -n $"pausing $i: "
		QMAIL/bin/svc -p $i && $succ || $fail
		RETVAL=$?
		echo
		let ret+=$RETVAL
	done
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  cont)
    ret=0
	for i in `echo $SERVICE/qmail-send.* $SERVICE/qmail-smtpd.*`
	do
		$ECHO -n $"continuing $i: "
		QMAIL/bin/svc -c $i && $succ || $fail
		RETVAL=$?
		echo
		let ret+=$RETVAL
	done
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  cdb)
	case "$SYSTEM" in
		DARWIN*)
		INDIMAILDIR=`dscl . -read /Users/indimail|/bin/grep NFSHomeDi|awk '{print $2}'`
		;;
		*)
		INDIMAILDIR=`/bin/grep -w "^indimail" /etc/passwd | cut -d: -f6|head -1`
		;;
	esac
    ret=0
	for i in smtp qmtp qmqp imap pop3 poppass
	do
		for j in `/bin/ls $INDIMAILDIR/etc/tcp*.$i.cdb 2>/dev/null`
		do
			t_file=`echo $j | cut -d. -f1,2`
			if [ ! -f $t_file ] ; then
				$ECHO -n $"deleting $j: "
				/bin/rm -f $j && $succ || $fail
				RETVAL=$?
				echo
				let ret+=$RETVAL
			fi
		done
		for j in `/bin/ls $INDIMAILDIR/etc/tcp*.$i 2>/dev/null`
		do
			$ECHO -n $"building $j.cdb: "
			QMAIL/bin/tcprules $j.cdb $j.tmp < $j && /bin/chmod 664 $j.cdb \
				&& /bin/chown indimail:indimail $j.cdb && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		done
	done
	[ $ret -eq 0 ] && exit 0 || exit 1
	;;
  help)
	myhelp
	exit 0
	;;
  *)
    echo "Usage: `basename $0` {start|stop|condrestart|restart|shut|kill|flush|reload|stat|queue|pause|cont|cdb|help}"
	myhelp
	exit 1
	;;
esac

exit 0
