# chkconfig: 345 50 80
# description: Starts qmail system and associated services
# $Log: qmailctl.sh,v $
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

PATH=$PATH:QMAIL/bin:QMAIL/sbin:/usr/local/bin:/usr/bin:/bin
export PATH

[ -x QMAIL/bin/svscanboot ] || exit 0

case "$SYSTEM" in
	DARWIN*)
		RunService $1
		exit $?
	;;
esac

case "$1" in
  stop)
    QMAIL/bin/svc -d /service/qmail-smtpd.*
    QMAIL/bin/svc -d /service/qmail-send.*
    ;;
  start)
    QMAIL/bin/svc -u /service/qmail-smtpd.*
    QMAIL/bin/svc -u /service/qmail-send.*
    ;;
  restart)
    QMAIL/bin/svc -d /service/qmail-smtpd.*
    QMAIL/bin/svc -t /service/qmail-send.*
    QMAIL/bin/svc -u /service/qmail-smtpd.*
    ;;
  kill)
    kill `ps -ef|egrep "tcpserver|supervise|qmail-send" | grep -v grep | awk '{print $2}'`
    ;;
  stat)
    QMAIL/bin/svstat $SERVICE/* $SERVICE/*/log
    ;;
  flush)
    echo "Flushing timeout table and sending ALRM signal to qmail-send."
    QMAIL/bin/qmail-tcpok
    QMAIL/bin/svc -a $SERVICE/qmail-send*
    ;;
  queue)
    QMAIL/bin/qmail-qstat
    QMAIL/bin/qmail-qread
    ;;
  reload)
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
  cdb)
    INDIMAILDIR=`grep -w "^indimail" /etc/passwd | cut -d: -f6|head -1`
	for i in smtp qmtp qmqp imap pop3 poppass
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
    		QMAIL/bin/tcprules $j.cdb $j.tmp < $j && /bin/chmod 664 $j.cdb \
				&& /bin/chown indimail:indimail $j.cdb
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
  start -- starts mail service (smtp connection accepted, mail can go out)
   stop -- stops mail service (smtp connections refused, nothing goes out)
restart -- stops and restarts smtp, sends qmail-send a TERM & restarts it
   kill -- Send kill signal to tcpserver, supvervise, qmail-send processes.
  flush -- Schedules queued messages for immediate delivery
 reload -- sends qmail-send HUP, rereading locals and virtualdomains
   stat -- displays status of mail service
  queue -- shows status of queue
  pause -- temporarily stops mail service (connections accepted, nothing leaves)
   cont -- continues paused mail service
    cdb -- rebuild the tcpserver cdb file for smtp, imap, pop3 and poppass
HELP
    ;;
  *)
    echo "Usage: `basename $0` {start|stop|restart|flush|reload|stat|queue|pause|cont|cdb|help}"
    exit 1
    ;;
esac

exit $RETVAL
