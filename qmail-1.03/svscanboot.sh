# $Log: svscanboot.sh,v $
# Revision 1.16  2017-05-11 13:29:40+05:30  Cprogrammer
# use envdir from .svscan/variables and use unshare --mount to run svscan
#
# Revision 1.15  2017-03-09 16:39:32+05:30  Cprogrammer
# FHS changes
#
# Revision 1.14  2016-06-03 09:58:42+05:30  Cprogrammer
# moved svscan to sbin
#
# Revision 1.13  2016-05-17 23:11:42+05:30  Cprogrammer
# fix for configurable control directory
#
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
# $Id: svscanboot.sh,v 1.16 2017-05-11 13:29:40+05:30 Cprogrammer Exp mbhangui $

PATH=PREFIX/bin:PREFIX/sbin:/bin:/sbin:/usr/bin:/usr/sbin

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
if [ -d /var/lock/subsys ] ; then
	STATUSFILE=/var/lock/subsys/svscan
else
	STATUSFILE=/tmp/svscan
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
		exec $MOUNT_CMD envdir  $VARIABLES \
			PREFIX/sbin/svscan $SERVICEDIR 2>&1 | \
			PREFIX/sbin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................
	else
		exec $MOUNT_CMD envdir  $VARIABLES \
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
		touch $STATUSFILE
	fi
	wait
	if [ -d /var/lock/subsys ] ; then
		/bin/rm -f $STATUSFILE
	fi
fi
