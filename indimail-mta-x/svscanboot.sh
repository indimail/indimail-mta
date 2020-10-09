# $Log: svscanboot.sh,v $
# Revision 1.21  2020-10-09 17:32:11+05:30  Cprogrammer
# use conf-prefix, conf-servicedir for configuring paths
#
# Revision 1.20  2020-10-08 22:56:18+05:30  Cprogrammer
# servicedir changed to libexecdir/service
#
# Revision 1.19  2020-07-11 22:08:41+05:30  Cprogrammer
# removed svscan STATUSFILE
#
# Revision 1.18  2017-09-01 14:24:20+05:30  Cprogrammer
# skip envdir if .svscan/variables directory is missing
#
# Revision 1.17  2017-05-11 15:49:45+05:30  Cprogrammer
# removed RCS log.
#
#
# $Id: svscanboot.sh,v 1.21 2020-10-09 17:32:11+05:30 Cprogrammer Exp mbhangui $

PATH=@prefix@/bin:/bin:/usr/local/bin:@prefix@/sbin:/sbin

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
	elif [ -d @servicedir@ ] ; then
		SERVICEDIR=@servicedir@
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
	@prefix@/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log $SERVICEDIR/.svscan/log
	if [ $use_readproctitle -eq 1 ] ; then
		if [ -d $VARIABLES ] ; then
		exec $MOUNT_CMD envdir $VARIABLES \
			@prefix@/sbin/svscan $SERVICEDIR 2>&1 | \
			@prefix@/sbin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................
		else
		exec $MOUNT_CMD @prefix@/sbin/svscan $SERVICEDIR 2>&1 | \
			@prefix@/sbin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................
		fi
	else
		if [ -d $VARIABLES ] ; then
		exec $MOUNT_CMD envdir $VARIABLES \
			@prefix@/sbin/svscan $SERVICEDIR
		else
		exec $MOUNT_CMD @prefix@/sbin/svscan $SERVICEDIR
		fi
	fi
else
	for i in $*
	do
		if [ ! -d $i ] ; then
			continue
		fi
		SERVICEDIR=$i
		VARIABLES=$SERVICEDIR/.svscan/variables
		@prefix@/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log
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
			if [ -d $VARIABLES ] ; then
			eval $MOUNT_CMD envdir $VARIABLES \
				@prefix@/sbin/svscan $SERVICEDIR 2>&1 | \
				@prefix@/sbin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................ &
			else
			eval $MOUNT_CMD @prefix@/sbin/svscan $SERVICEDIR 2>&1 | \
				@prefix@/sbin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................ &
			fi
		else
			if [ -d $VARIABLES ] ; then
			eval $MOUNT_CMD envdir $VARIABLES \
				@prefix@/sbin/svscan $SERVICEDIR &
			else
			eval $MOUNT_CMD @prefix@/sbin/svscan $SERVICEDIR &
			fi
		fi
	done
	wait
fi
