# chkconfig: 345 14 91
# description: Starts svscan system and associated services
### BEGIN INIT INFO
# Provides:          svscan
# Required-Start:    $network
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Description:       Starts svscan system and associated services
# Short-Description: Start/Stop svscan
### END INIT INFO
#
# $Id: qmailctl.sh,v 1.70 2021-07-28 19:45:44+05:30 Cprogrammer Exp mbhangui $
#
#
SERVICE=@servicedir@

init()
{
if [ -f /etc/lsb-release -o -f /etc/debian_version ] ; then
	SYSTEM=Debian
elif [ -f /etc/SuSE-release ] ; then
	SYSTEM=SuSE
elif [ -f /etc/alpine-release ] ; then
	SYSTEM=alpine
else
	if [ -f /etc/os-release ] ; then
		NAME=`grep -w NAME /etc/os-release | cut -d= -f2 | sed -e 's{"{{g'`
		case "$NAME" in
			"openSUSE Tumbleweed")
			SYSTEM=SuSE
			;;
			*)
			SYSTEM=`uname -s`
			;;
		esac
	else
		SYSTEM=`uname -s`
	fi
fi
ECHO=echo
case "$SYSTEM" in
	Darwin*|Debian|SuSE|FreeBSD|alpine)
		RES_COL=60
		MOVE_TO_COL="$ECHO -en \\033[${RES_COL}G"
		SETCOLOR_SUCCESS="$ECHO -en \\033[1;32m"
		SETCOLOR_FAILURE="$ECHO -en \\033[1;31m"
		SETCOLOR_WARNING="$ECHO -en \\033[1;33m"
		SETCOLOR_NORMAL="$ECHO -en \\033[0;39m"
		;;
esac
if [ -f /etc/indimail/indimail-mta-release ] ; then
	have_qmail=1
else
	have_qmail=0
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
	Darwin*)
	. /etc/rc.common
	CheckForNetwork
	if [ "${NETWORKUP}" != "-YES-" ]
	then
		exit 0
	fi
	succ=mysuccess
	fail=myfailure
	;;
	Debian|SuSE)
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
		if [ "${NETWORKING}" = "no" ] ; then
			exit 0
		fi
	else # FC19, FC20, ...
		NETWORKING=yes
	fi
	;;
	*)
	succ=mysuccess
	fail=myfailure
	;;
esac
}

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
	$ECHO -n "FAILED"
	$SETCOLOR_NORMAL
	$ECHO -n "]"
	$ECHO -ne "\r"
	return 1
}

noqmail() {
	if [ $have_qmail -ne 1 ] ; then
		echo "qmail binaries not present" 1>&2
		echo "Usage: `basename $0` {start|stop|restart|status|rotate|help}" 1>&2
		myhelp
		exit 1
	fi
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

myhelp()
{
if [ $have_qmail -eq 1 ] ; then
/bin/cat <<HELP
  start -- starts mail service (smtp connection accepted, mail can go out)
   stop -- stops mail service (smtp connections refused, nothing goes out)
restart -- stops and restarts smtp, sends qmail-send a TERM & restarts it
   kill -- Send kill signal to tcpserver, supvervise, qmail-send processes.
  flush -- Schedules queued messages for immediate delivery
 reload -- sends qmail-send HUP, rereading locals and virtualdomains
 status -- status of svscan process & IndiMail services
 rotate -- rotate all logfiles (sending ALRM to multilog)
  queue -- shows status of queue
  pause -- temporarily stops mail service (connections accepted, nothing goes out)
   cont -- continues paused mail service
    cdb -- rebuild the tcpserver cdb file for smtp, qmtp, qmqp, imap, pop3 and poppass
HELP
else
/bin/cat <<HELP
  start -- starts svscan service
   stop -- stops  svscan service
restart -- stops and restarts svscan service
 status -- status of svscan process and configured services
 rotate -- rotate all logfiles (sending ALRM to multilog)
HELP
fi
}

stop()
{
	if [ -d /var/lock/subsys -a ! -f /var/lock/subsys/svscan ] ; then
		ps ax|grep svscan|egrep -v "grep|supervise|multilog" >/dev/null || exit 0
	fi
	if [ -d /run ] ; then
		rundir=/run
	elif [ -d /var/run ] ; then
		rundir=/var/run
	else
		rundir=/service
	fi
	local ret=0
	case "$SYSTEM" in
		FreeBSD|alpine)
		sv_pid=$rundir/svscan/.svscan.pid
		daemon_pid=$rundir/svscan/sv_daemon.pid
		if [ ! -f $sv_pid -a ! -f $daemon_pid ] ; then
			echo "WARNING: svscan is already stopped"
			return $ret
		fi
		;;
	esac
	sv_all_down
	if [ -s /tmp/sv.err ] ; then
		/bin/cat /tmp/sv.err
	fi
	/bin/rm -f /tmp/sv.err
	if [ -f /usr/bin/systemctl ] ; then
		if [ $PPID -ne 1 ] ; then
			$ECHO -n "Stopping svscan: "
			/usr/bin/systemctl stop svscan && $succ || $fail
			echo ""
		fi
	elif [ -f /sbin/initctl ] ; then
		$ECHO -n "Stopping svscan: "
		/sbin/initctl stop svscan && $succ || $fail
		echo ""
	elif [ -f /usr/sbin/daemon ] ; then
		$ECHO -n "Stopping svscan: "
		if [ -f $rundir/sv_daemon.pid ] ; then
			kill `cat $rundir/sv_daemon.pid` && $succ || $fail
		else
			ps ax|grep svscan|egrep -v "grep" >/dev/null && $fail || $succ
		fi
		echo ""
	elif [ -f /etc/inittab ] ; then
		$ECHO -n "Stopping svscan: "
		if [ "$SYSTEM" = "alpine" ] ; then
  			grep "svscan" /etc/inittab | grep respawn >/dev/null 2>&1
		else
  			grep "^SV:" /etc/inittab | grep svscan | grep respawn >/dev/null 2>&1
		fi
		if [ $? -eq 0 ] ; then
			sed -i '/svscan/d' /etc/inittab
			if [ $? -eq 0 ] ; then
				kill -1 1
				sv_pid=$rundir/svscan/.svscan.pid
				if [ -f $sv_pid ] ; then
					sleep 4
					if [ -f $sv_pid ] ; then
						kill -0 `sed -n 2p $sv_pid` && $fail || $succ && rm -f $sv_pid
					else
						$succ
					fi
				else
					ps ax|grep svscan|egrep -v "grep" >/dev/null && $fail || $succ
				fi
				RETVAL=1
			else
				echo "failed to remove svscan from /etc/inittab" 1>&2
				RETVAL=1
			fi
		else
			$ECHO -n "Stopping svscan: "
			case "$SYSTEM" in
				FreeBSD)
				/usr/bin/killall -c svscan && $succ || $fail
				;;
				alpine)
				/usr/bin/killall svscan && $succ || $fail
				;;
				*)
				/usr/bin/killall -e -w svscan && $succ || $fail
				;;
			esac
			RETVAL=$?
		fi
		echo ""
	fi
	if [ -d /var/lock/subsys ] ; then
		[ $ret -eq 0 ] && rm -f /var/lock/subsys/svscan
	fi
	return $ret
}

sv_all_up()
{
	for i in `echo $SERVICE/*`
	do
		if [ -d $i -a ! -f $i/down ] ; then
			@prefix@/bin/svstat $i >/dev/null 2>&1
			if [ $? -eq 0 ] ; then
				continue
			fi
			$ECHO -n "Starting $i: "
			@prefix@/bin/svc -u $i 2>/tmp/sv.err && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		fi
	done
}

sv_all_down()
{
	for i in `echo $SERVICE/*`
	do
		if [ -d $i ] ; then
			@prefix@/bin/svstat $i >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				continue
			fi
			$ECHO -n "Stopping $i: "
			@prefix@/bin/svc -d $i 2>>/tmp/sv.err && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		fi
	done
}

start()
{
	if [ -d /run ] ; then
		rundir=/run
	elif [ -d /var/run ] ; then
		rundir=/var/run
	else
		rundir=/service
	fi
	local ret=0
	if [ -d /var/lock/subsys -a -f /var/lock/subsys/svscan ] ; then
		ps ax|grep svscan|egrep -v "grep|supervise|multilog" >/dev/null && sv_all_up && return $ret || rm -f /var/lock/subsys/svscan
	fi
	if [ -d $SERVICE/.svscan/log ] ; then
		@prefix@/bin/svstat $SERVICE/.svscan/log > /dev/null 2>&1
		status=$?
	else
		status=1
	fi
	if [ $status -eq 0 ] ; then
		sv_all_up
	else
		if [ -f /usr/bin/systemctl ] ; then
			if [ $PPID -ne 1 ] ; then
				$ECHO -n "Starting svscan: "
				systemctl start svscan
			fi
		elif [ -f /sbin/initctl ] ; then
			$ECHO -n "Starting svscan: "
			/sbin/initctl start svscan >/dev/null 2>>/tmp/sv.err && $succ || $fail
		elif [ -f /usr/sbin/daemon ] ; then
			$ECHO -n "Starting svscan: "
			daemon_pid=$rundir/sv_daemon.pid
			env SETSID=1 /usr/sbin/daemon -cS -P $daemon_pid -R 5 \
				-t "$SYSTEM"_svscan @libexecdir@/svscanboot && $succ || $fail
		else
			$ECHO -n "Starting svscan: "
			if [ -c /dev/console -a -w /dev/console ] ; then
				device=/dev/console
			else
				device=/dev/null
			fi
			if [ "$SYSTEM" = "alpine" ] ; then
				grep "svscanboot" /etc/inittab | grep respawn >/dev/null 2>&1
			else
				grep "^SV:" /etc/inittab | grep svscan | grep respawn >/dev/null 2>&1
			fi
			if [ $? -ne 0 ]; then
				grep -v "svscanboot" /etc/inittab > /etc/inittab.qmailctl.$$ 2>&1
				if [ " $SYSTEM" = " Debian" ] ; then
					echo "SV:2345:respawn:@libexecdir@/svscanboot $SERVICE <>$device 2<>$device" >> /etc/inittab.qmailctl.$$
				elif [ " $SYSTEM" = " alpine" ] ; then
					echo "::respawn:@libexecdir@/svscanboot $SERVICE" >> /etc/inittab.qmailctl.$$
				else
					echo "SV:345:respawn:@libexecdir@/svscanboot $SERVICE <>$device 2<>$device" >> /etc/inittab.qmailctl.$$
				fi
				if [ $? -eq 0 ] ; then
					/bin/mv /etc/inittab.qmailctl.$$ /etc/inittab
				fi
				sv_pid=$rundir/svscan/.svscan.pid
				if [ ! -f $sv_pid ] ; then
					echo $0 | egrep "qmailctl" > /dev/null
					if [ $? -eq 0 ] ;then
						if [ "$SYSTEM" = "alpine" ] ; then
							kill -1 1
						else
							/sbin/init q
						fi
					fi
				fi
				if [ -f $sv_pid ] ; then
					if [ -f $sv_pid ] ; then
						kill -0 `sed -n 2p $sv_pid` && $succ || $fail
					fi
				else
					ps ax|grep svscan|egrep -v "grep|supervise|multilog" >/dev/null && $succ || $fail
				fi
			fi
		fi
		RETVAL=$?
		[ $RETVAL -eq 0 ] && $succ || $fail
		echo ""
		let ret+=$RETVAL
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

# Check that we're a privileged user
[ `id -u` = 0 ] || exit 4
if [ "x$1" = "xstop" ] ; then
	if [ $PPID -ne 1 ] ; then
		SYSTEMCTL_SKIP_REDIRECT=1
	fi
fi
init $1

if [ -x /usr/bin/systemctl ] ; then
	/usr/bin/systemctl is-enabled svscan > /dev/null 2>&1
	if [ $? -ne 0 ] ; then
		if [ -f @prefix@/sbin/initsvc ] ; then
			@prefix@/sbin/initsvc -on > /dev/null && $succ || $fail
			$ECHO -ne "\n"
		fi
		/usr/bin/systemctl is-enabled svscan > /dev/null 2>&1
	fi
	if [ $? -ne 0 ] ; then
		SYSTEMCTL_SKIP_REDIRECT=1
		if [ -f /etc/rc.d/init.d/functions ] ; then
			. /etc/rc.d/init.d/functions
		elif [ -f /etc/init.d/functions ] ; then
			. /etc/init.d/functions
		elif [ -f /lib/lsb/init-functions ] ; then
			. /lib/lsb/init-functions
		fi
		$ECHO -n "svscan is disabled: "
		$fail
		$ECHO -ne "\n"
		exit 1
	fi
fi

PATH=$PATH:@prefix@/bin:@prefix@/sbin:/usr/bin:/bin
export PATH
cmmd=$1
if [ "$0" = "/lib/rc/sh/gendepends.sh" -a -z "$cmmd" ] ; then #alpine linux
	cmmd="start"
fi
case "$cmmd" in
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
		noqmail
		ret=0
		for i in `echo $SERVICE/qmail-smtpd.* $SERVICE/qmail-qmqpd.* $SERVICE/qmail-qmtpd.*`
		do
			$ECHO -n "Stopping $i: "
			@prefix@/bin/svc -d $i && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		done
		for i in `echo $SERVICE/qmail-send.*`
		do
			$ECHO -n "Terminating $i: "
			@prefix@/bin/svc -t $i && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		done
		for i in `echo $SERVICE/qmail-smtpd.* $SERVICE/qmail-qmqpd.* $SERVICE/qmail-qmtpd.*`
		do
			if [ ! -f $i/down ] ; then
			$ECHO -n "Starting $i: "
				@prefix@/bin/svc -u $i && $succ || $fail
				RETVAL=$?
				echo
				let ret+=$RETVAL
			fi
		done
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	kill)
		noqmail
		$ECHO -n "killing tcpserver,supervise,qmail-send: "
		kill `ps ax| egrep "tcpserver|supervise|qmail-send" | grep -v grep | awk '{print $1}'` && $succ || $fail
		ret=$?
		echo
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	rotate)
		ret=0
		for i in `echo $SERVICE/* $SERVICE/.svscan`
		do
			if [ -d $i/log ] ; then
				$ECHO -n "Rotating $i: "
				@prefix@/bin/svc -a $i/log && $succ || $fail
				RETVAL=$?
				echo
				let ret+=$RETVAL
			fi
		done
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	flush)
		noqmail
		ret=0
		$ECHO -n "Flushing timeout table + ALRM signal to qmail-send."
		@prefix@/sbin/qmail-tcpok > /dev/null && $succ || $fail
		echo
		for i in `echo $SERVICE/qmail-send.*`
		do
			$ECHO -n "Flushing $i: "
			@prefix@/bin/svc -a $i && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		done
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	status|stat)
		ret=0
		if [ -x /sbin/initctl ] ; then
			/sbin/initctl status svscan
		else
			ps ax| grep svscanboot| grep -v grep
		fi
		RETVAL=$?
		args=`ls -d $SERVICE/* $SERVICE/*/log $SERVICE/.svscan/log`
		if [ -n "$args" ] ; then
			@prefix@/bin/svstat $SERVICE/.svscan/log $SERVICE/* $SERVICE/*/log
		else
			echo "No services configured under svscan"
		fi
		let ret+=$RETVAL
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	queue)
		noqmail
		@prefix@/bin/qmail-qread -c
		ret=$?
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	reload)
		noqmail
		ret=0
		for i in `echo $SERVICE/qmail-send.*`
		do
			$ECHO -n "sending HUP signal to $i: "
			@prefix@/bin/svc -h $i && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		done
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	pause)
		noqmail
		ret=0
		for i in `echo $SERVICE/qmail-send.* $SERVICE/qmail-smtpd.*`
		do
			$ECHO -n "pausing $i: "
			@prefix@/bin/svc -p $i && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		done
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	cont)
		noqmail
		ret=0
		for i in `echo $SERVICE/qmail-send.* $SERVICE/qmail-smtpd.*`
		do
			$ECHO -n "continuing $i: "
			@prefix@/bin/svc -c $i && $succ || $fail
			RETVAL=$?
			echo
			let ret+=$RETVAL
		done
		[ $ret -eq 0 ] && exit 0 || exit 1
		;;
	cdb)
		noqmail
		ret=0
		for i in smtp qmtp qmqp imap pop3 poppass
		do
			for j in `/bin/ls @sysconfdir@/tcp*.$i.cdb 2>/dev/null`
			do
				t_file=`echo $j | cut -d. -f1,2`
				if [ ! -f $t_file ] ; then
					$ECHO -n "deleting $j: "
					/bin/rm -f $j && $succ || $fail
					RETVAL=$?
					echo
					let ret+=$RETVAL
				fi
			done
			for j in `/bin/ls @sysconfdir@/tcp/tcp*.$i 2>/dev/null`
			do
				t1=`date +'%s' -r $j`
				if [ -f $j.cdb ] ; then
					t2=`date +'%s' -r $j.cdb`
				else
					t2=0
				fi
				if [ $t1 -gt $t2 ] ; then
					$ECHO -n "building $j.cdb: "
					@prefix@/bin/tcprules $j.cdb $j.tmp < $j && /bin/chmod 664 $j.cdb \
						&& /bin/chown indimail:indimail $j.cdb && $succ || $fail
					RETVAL=$?
					echo
				else
					RETVAL=0
				fi
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
		if [ $have_qmail -eq 1 ] ; then
			echo "Usage: `basename $0` {start|stop|condrestart|restart|kill|flush|rotate|reload|stat|queue|pause|cont|cdb|help}"
		else
			echo "Usage: `basename $0` {start|stop|restart|stat|rotate|help}"
		fi
		myhelp
		exit 1
	;;
esac
exit 0
