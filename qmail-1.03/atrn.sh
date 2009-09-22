#
# $Log: atrn.sh,v $
# Revision 1.8  2009-04-19 13:38:58+05:30  Cprogrammer
# replaced indimail/bin/echo with echo 1>&2
#
# Revision 1.7  2004-07-13 23:10:41+05:30  Cprogrammer
# use resetquota to set right quota
#
# Revision 1.6  2004-06-17 22:18:27+05:30  Cprogrammer
# prevent argument list too long
#
# Revision 1.5  2003-10-28 20:00:23+05:30  Cprogrammer
# added comment for .qvirtual
#
# Revision 1.4  2003-10-17 21:02:35+05:30  Cprogrammer
# corrected USAGE
# use full path of rm
#
# Revision 1.3  2003-09-16 17:56:25+05:30  Cprogrammer
# include cur in mail count
#
# Revision 1.2  2003-08-22 23:08:58+05:30  Cprogrammer
# added file .qvirtual to use a different qvirtual
#
# Revision 1.1  2003-08-22 22:59:06+05:30  Cprogrammer
# Initial revision
#
#
# 0 - queueing started
# 1 - System Error
# 2 - Domain Rejected
# 3 - No Pending message for node
# 4 - Pending message for node
#
# $Id: atrn.sh,v 1.8 2009-04-19 13:38:58+05:30 Cprogrammer Stab mbhangui $
#
trap "" 1 2 3
if [ $# -lt 2 ] ; then
	echo "atrn: ERROR: USAGE: domain_name(s) remoteip" 1>&2
	exit 1
elif [ " $TCPREMOTEIP" = " " ] ; then
	echo "atrn: ERROR: TCPREMOTEIP not set" 1>&2
	exit 1
fi
INDIMAILDIR=`grep -w "^indimail" /etc/passwd | cut -d: -f6|head -1`
if [ " $INDIMAILDIR" = " " ] ; then
	echo "atrn: Could not determine indimail home" 1>&2
	exit 1
fi
if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR="control"
fi
if [ -f QMAIL/"$CONTROLDIR"/queuelifetime ] ; then
	LIFETIME=`cat QMAIL/"$CONTROLDIR"/queuelifetime`
else
	LIFETIME=1209600
fi
PATH=/bin:/usr/bin:$PATH
cd QMAIL/autoturn
total=0
pend=0
for domains in $1
do
	count=`for i in $domains/Maildir/new $domains/Maildir/cur ; do /bin/ls $i; done|wc -l`
	QMAIL/bin/setlock -nx $domains/seriallock /bin/rm $domains/seriallock
	if [ -f $domains/seriallock ] ; then
		pend=1
	fi
	total=`expr $total + $count`
done
if [ $total -eq 0 ] ; then
	exit 3
else
	if [ $pend -eq 1 ] ; then
		exit 4
	fi
fi
#
# .qvirtual allows mails for a main domain to be
# distributed across multiple directories in 
# QMAIL/autoturn directory. i.e. if etrn.dom
# is the main domain and mails have been split
# into directories location1.etrn.dom and location2.etrn.dom
# specify .qvirtual having etrn.dom in QMAIL/autoturn/location1.etrn.dom
# and QMAIL/autoturn/location2.etrn.dom.
# .qvirtual also allows mails for a domain to be delivered to
# any directory and the domain identified by looking up the .qvirtual file
#
echo "250 OK now reversing the connection"
for domains in $1
do
	if [ -f $domains/.qvirtual ] ; then
		qvirtual=`cat $domains/.qvirtual`
	else
		qvirtual=$domains
	fi
	QMAIL/bin/setlock -nx $domains/seriallock QMAIL/bin/maildirserial \
		-b -t $LIFETIME $domains/Maildir "$qvirtual"- \
		QMAIL/bin/serialsmtp "$qvirtual"- AutoTURN 1
	QMAIL/bin/setlock -nx $domains/seriallock $INDIMAILDIR/bin/resetquota \
		$domains/Maildir
	QMAIL/bin/setlock -nx $domains/seriallock /bin/rm $domains/seriallock
done
exit 0
