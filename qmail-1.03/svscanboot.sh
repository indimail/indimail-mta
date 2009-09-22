# $Log: svscanboot.sh,v $
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
# $Id: svscanboot.sh,v 1.8 2009-06-18 19:47:48+05:30 Cprogrammer Stab mbhangui $

PATH=QMAIL/bin:/usr/local/bin:/bin:/sbin:/usr/bin:/usr/sbin

exec </dev/null
exec >/dev/null
exec 2>/dev/null

if [ -f  QMAIL/control/scaninterval ] ; then
	SCANINTERVAL=`cat QMAIL/control/scaninterval`
else
	SCANINTERVAL=300
fi
if [ $# -eq 0 ] ; then
	if [ -d /service1 ] ; then
		SERVICEDIR=/service1
	elif [ -d /service2 ] ; then
		SERVICEDIR=/service2
	else
		SERVICEDIR=/service
	fi
	QMAIL/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log
	env - PATH=$PATH SCANINTERVAL=$SCANINTERVAL \
		QMAIL/bin/svscan $SERVICEDIR 2>&1 | \
	env - PATH=$PATH QMAIL/bin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................
else
	for i in $*
	do
		if [ ! -d $i ] ; then
			continue
		fi
		SERVICEDIR=$i
		QMAIL/bin/svc -dx $SERVICEDIR/* $SERVICEDIR/*/log
		env - PATH=$PATH SCANINTERVAL=$SCANINTERVAL \
			QMAIL/bin/svscan $SERVICEDIR 2>&1 | \
		env - PATH=$PATH QMAIL/bin/readproctitle $SERVICEDIR errors: ................................................................................................................................................................................................................................................................................................................................................................................................................ &
	done
	wait
fi
