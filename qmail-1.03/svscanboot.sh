# $Log: svscanboot.sh,v $
# Revision 1.12  2011-08-05 15:49:43+05:30  Cprogrammer
# check /var/lock/subsys before creating status file
#
# Revision 1.11  2011-08-05 14:30:33+05:30  Cprogrammer
# added option to use readproctitle
# added option to use multiple service directories
#
# Revision 1.10  2011-07-21 13:18:05+05:30  Cprogrammer
# create lockfile to enable working with systemd
#
# Revision 1.9  2011-05-26 23:37:41+05:30  Cprogrammer
# removed readproctitle
#
# Revision 1.8  2009-06-18 19:47:48+05:30  Cprogrammer
# do wait only for job in background
#
# Revision 1.7  2008-07-25 16:54:33+05:30  Cprogrammer
# set SCANINTERVAL in all cases
# use wait in all cases
#
# Revision 1.6  2008-07-15 19:54:07+05:30  Cprogrammer
# removed /usr/X11R6/bin from path
#
# Revision 1.5  2003-10-12 01:14:37+05:30  Cprogrammer
# added SCANINTERVAL
#
# Revision 1.4  2003-01-05 23:53:11+05:30  Cprogrammer
# skip directories if not present
#
# Revision 1.3  2002-12-28 09:21:30+05:30  Cprogrammer
# added option of specifying multiple directories as command line arguments
#
# Revision 1.2  2002-09-26 20:56:02+05:30  Cprogrammer
# made service directory configurable
#
# $Id: svscanboot.sh,v 1.12 2011-08-05 15:49:43+05:30 Cprogrammer Stab mbhangui $

PATH=QMAIL/bin:/usr/local/bin:/bin:/sbin:/usr/bin:/usr/sbin

exec </dev/null
exec >/dev/null
exec 2>/dev/null

if [ -f  QMAIL/control/scaninterval ] ; then
	SCANINTERVAL=`cat QMAIL/control/scaninterval`
else
	SCANINTERVAL=300
fi
use_readproctitle=0
if [ $# -eq 0 ] ; then
	if [ -d /service1 ] ; then
		SERVICEDIR=/service1
	elif [ -d /service2 ] ; then
		SERVICEDIR=/service2
	else
		SERVICEDIR=/service
	fi
elif [ $# -eq 1 ] ; then
	SERVICEDIR=$1
fi
if [ -d /var/lock/subsys ] ; then
	STATUSFILE=/var/lock/subsys/svscan
else
	STATUSFILE=/tmp/svscan
fi
if [ $# -eq 0 -o $# -eq 1 ] ; then
	QMAIL/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log $SERVICEDIR/.svscan/log
	if [ $use_readproctitle -eq 1 ] ; then
		exec env - PATH=$PATH SCANINTERVAL=$SCANINTERVAL \
			STATUSFILE=$STATUSFILE \
			QMAIL/bin/svscan $SERVICEDIR 2>&1 | \
		env - PATH=$PATH QMAIL/bin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................
	else
		exec env - PATH=$PATH SCANINTERVAL=$SCANINTERVAL SCANLOG="" \
			STATUSFILE=$STATUSFILE \
			QMAIL/bin/svscan $SERVICEDIR
	fi
else
	for i in $*
	do
		if [ ! -d $i ] ; then
			continue
		fi
		SERVICEDIR=$i
		QMAIL/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log
		if [ $use_readproctitle -eq 1 ] ; then
			env - PATH=$PATH SCANINTERVAL=$SCANINTERVAL \
				QMAIL/bin/svscan $SERVICEDIR 2>&1 | \
			env - PATH=$PATH QMAIL/bin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................ &
		else
			env - PATH=$PATH SCANINTERVAL=$SCANINTERVAL SCANLOG="" \
				QMAIL/bin/svscan $SERVICEDIR &
		fi
	done
	if [ -d /var/lock/subsys ] ; then
		touch $STATUSFILE
	fi
	wait
	if [ -d /var/lock/subsys ] ; then
		/bin/rm -f $STATUSFILE
	fi
fi
