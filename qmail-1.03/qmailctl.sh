# chkconfig: 345 50 80
# description: Starts qmail system and associated services
# $Log: qmailctl.sh,v $
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
RETVAL=0
Start() {
        echo -n "Starting qmail services: "
        cd $SERVICE
        daemon svscanboot
        RETVAL=$?
        echo
        [ $RETVAL -eq 0 ] && touch /var/lock/subsys/qmail
        return $RETVAL
}
Stop() {
        echo -n "Stopping qmail services: "
        killproc svscanboot
        RETVAL=$?
        echo
        [ $RETVAL -eq 0 ] && rm -f /var/lock/subsys/qmail
        return $RETVAL
}

StartService() {
        /bin/echo -n "Starting qmail services: "
        cd $SERVICE
        svscanboot
        RETVAL=$?
        echo
        [ $RETVAL -eq 0 ] && touch /var/lock/subsys/qmail
        return $RETVAL
}
StopService() {
        echo -n "Stopping qmail services: "
        killall svscanboot
        RETVAL=$?
        echo
        [ $RETVAL -eq 0 ] && rm -f /var/lock/subsys/qmail
        return $RETVAL
}
RestartService() {
	StopService
	StartService
}
#
#
SYSTEM=`uname -s | tr "[:lower:]" "[:upper:]"`
case "$SYSTEM" in
	DARWIN*)
	. /etc/rc.common
	CheckForNetwork
	if [ ${NETWORKUP} != "-YES-" ]
	then
		exit 0
	fi
	;;
	LINUX)
	# Source function library.
	if [ -f /etc/init.d/functions ] ; then
		. /etc/init.d/functions
	fi
	# Get config.
	if [ -f /etc/sysconfig/network ] ; then
		. /etc/sysconfig/network
		# Check that networking is up.
		if [ ${NETWORKING} = "no" ] ; then
			exit 0
		fi
	fi
	;;
esac

PATH=$PATH:QMAIL/bin:/usr/local/bin:/usr/bin:/bin
export PATH

[ -x QMAIL/bin/svscanboot ] || exit 0

case "$SYSTEM" in
	DARWIN*)
		RunService $1
		exit $?
	;;
esac

case "$1" in
  start)
    Start
    ;;
  stop)
    StopService
    QMAIL/bin/svc -dx $SERVICE/* $SERVICE/*/log
    ;;
  restart)
    Stop
    Start
    ;;
  kill)
    kill `ps -ef|egrep "tcpserver|supervise|qmail-send" | grep -v grep | awk '{print $2}'`
    ;;
  stat)
    QMAIL/bin/svstat $SERVICE/* $SERVICE/*/log
    ;;
  doqueue|alrm|flush)
    echo "Flushing timeout table and sending ALRM signal to qmail-send."
    QMAIL/bin/qmail-tcpok
    QMAIL/bin/svc -a $SERVICE/qmail-send*
    ;;
  queue)
    QMAIL/bin/qmail-qstat
    QMAIL/bin/qmail-qread
    ;;
  reload|hup)
    echo "Sending HUP signal to qmail-send."
    QMAIL/bin/svc -h $SERVICE/qmail-send*
    ;;
  pause)
    echo "Pausing qmail-send"
    QMAIL/bin/svc -p $SERVICE/qmail-send*
    echo "Pausing qmail-smtpd"
    QMAIL/bin/svc -p $SERVICE/qmail-smtpd*
    ;;
  cont)
    echo "Continuing qmail-send"
    QMAIL/bin/svc -c $SERVICE/qmail-send*
    echo "Continuing qmail-smtpd"
    QMAIL/bin/svc -c $SERVICE/qmail-smtpd*
    ;;
  restart)
    echo "Restarting qmail:"
    echo "* Stopping qmail-smtpd."
    QMAIL/bin/svc -d $SERVICE/qmail-smtpd*
    echo "* Sending qmail-send SIGTERM and restarting."
    QMAIL/bin/svc -t $SERVICE/qmail-send*
    echo "* Restarting qmail-smtpd."
    QMAIL/bin/svc -u $SERVICE/qmail-smtpd*
    ;;
  cdb)
    INDIMAILDIR=`grep -w "^indimail" /etc/passwd | cut -d: -f6|head -1`
	for i in smtp imap pop3 poppass
	do
		for j in `/bin/ls $INDIMAILDIR/etc/tcp*.$i.cdb 2>/dev/null`
		do
			t_file=`echo $j | cut -d. -f1,2`
			if [ ! -f $t_file ] ; then
				/bin/rm -f $j
				if [ $? -eq 0 ] ; then
					echo "Deleted $j"
				else
					echo "Deleted $j: failed!!"
				fi
			fi
		done
		for j in `/bin/ls $INDIMAILDIR/etc/tcp*.$i 2>/dev/null`
		do
    		QMAIL/bin/tcprules $j.cdb $j.tmp < $j
    		chmod 644 $j.cdb
			if [ $? -eq 0 ] ; then
    			echo "Rebuilt $j.cdb"
			else
    			echo "Rebuild $j.cdb: failed!!"
			fi
		done
	done
    ;;
  help)
    cat <<HELP
   stop -- stops mail service (smtp connections refused, nothing goes out)
  start -- starts mail service (smtp connection accepted, mail can go out)
  pause -- temporarily stops mail service (connections accepted, nothing leaves)
   cont -- continues paused mail service
   stat -- displays status of mail service
    cdb -- rebuild the tcpserver cdb file for smtp, imap, pop3 and poppass
restart -- stops and restarts smtp, sends qmail-send a TERM & restarts it
doqueue -- Schedules queued messages for immediate delivery
  flush -- same as doqueue
 reload -- sends qmail-send HUP, rereading locals and virtualdomains
  queue -- shows status of queue
   alrm -- same as doqueue
    hup -- same as reload
HELP
    ;;
  *)
    echo "Usage: $0 {start|stop|restart|flush|doqueue|reload|stat|pause|cont|cdb|queue|help}"
    exit 1
    ;;
esac

exit $RETVAL
