# $Log: svscanboot.sh,v $
# Revision 1.17  2017-05-11 15:49:45+05:30  Cprogrammer
# removed RCS log.
#
#
# $Id: svscanboot.sh,v 1.17 2017-05-11 15:49:45+05:30 Cprogrammer Exp mbhangui $

PATH=PREFIX/bin:/bin:/usr/local/bin:PREFIX/sbin:/sbin

exec </dev/null
exec >/dev/null
exec 2>/dev/null

cd /
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
if [ $# -eq 0 -o $# -eq 1 ] ; then
	VARIABLES=$SERVICEDIR/.svscan/variables
	if [ ! -f $VARIABLES/SCANLOG -o -z $VARIABLES/SCANLOG ] ; then
		use_readproctitle=1
	fi
	if [ -s $VARIABLES/UNSHARE -a -x /usr/bin/unshare ] ; then
		MOUNT_CMD="/usr/bin/unshare --mount"
	else
		MOUNT_CMD=""
	fi
	PREFIX/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log $SERVICEDIR/.svscan/log
	if [ $use_readproctitle -eq 1 ] ; then
		exec $MOUNT_CMD envdir $VARIABLES \
			PREFIX/sbin/svscan $SERVICEDIR 2>&1 | \
			PREFIX/sbin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................
	else
		exec $MOUNT_CMD envdir $VARIABLES \
			PREFIX/sbin/svscan $SERVICEDIR
	fi
else
	for i in $*
	do
		if [ ! -d $i ] ; then
			continue
		fi
		SERVICEDIR=$i
		VARIABLES=$SERVICEDIR/.svscan/variables
		PREFIX/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log
		if [ ! -f $VARIABLES/SCANLOG -o -z $VARIABLES/SCANLOG ] ; then
			use_readproctitle=1
		else
			use_readproctitle=0
		fi
		if [ -s $VARIABLES/UNSHARE -a -x /usr/bin/unshare ] ; then
			MOUNT_CMD="/usr/bin/unshare --mount"
		else
			MOUNT_CMD=""
		fi
		if [ $use_readproctitle -eq 1 ] ; then
			eval $MOUNT_CMD envdir $VARIABLES \
				PREFIX/sbin/svscan $SERVICEDIR 2>&1 | \
				PREFIX/sbin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................ &
		else
			eval $MOUNT_CMD envdir $VARIABLES \
				PREFIX/sbin/svscan $SERVICEDIR &
		fi
	done
	if [ -d /var/lock/subsys ] ; then
		STATUSFILE=/var/lock/subsys/svscan
		touch $STATUSFILE
	fi
	wait
	if [ -d /var/lock/subsys ] ; then
		/bin/rm -f $STATUSFILE
	fi
fi
