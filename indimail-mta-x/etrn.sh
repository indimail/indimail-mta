#
# $Log: etrn.sh,v $
# Revision 1.12  2021-04-29 10:03:52+05:30  Cprogrammer
# replaced QMAIL with QMAILHOME
#
# Revision 1.11  2019-02-20 19:55:10+05:30  Cprogrammer
# set pipefail to catch error if ipmeprint fails
#
# Revision 1.10  2016-06-17 17:20:30+05:30  Cprogrammer
# FHS compliance
#
# Revision 1.9  2016-06-05 13:17:53+05:30  Cprogrammer
# moved dnsmxip to sbin
#
# Revision 1.8  2009-04-19 13:39:33+05:30  Cprogrammer
# replaced indimail/bin/echo with echo 1>&2
#
# Revision 1.7  2008-09-16 11:35:36+05:30  Cprogrammer
# ipmeprint now displays ipv4/ipv6 in the first column
#
# Revision 1.6  2004-07-13 23:10:58+05:30  Cprogrammer
# use resetquota to set right quota
#
# Revision 1.5  2004-06-17 22:18:54+05:30  Cprogrammer
# prevent argument list too long
#
# Revision 1.4  2003-09-16 17:56:48+05:30  Cprogrammer
# include cur in mail count
#
# Revision 1.3  2003-08-22 22:58:44+05:30  Cprogrammer
# added RCS identifiers
#
#
# 0 - queueing started
# 1 - System Error
# 2 - Domain Rejected
# 3 - No Pending message for node
# 4 - Pending message for node
#
# $Id: etrn.sh,v 1.12 2021-04-29 10:03:52+05:30 Cprogrammer Exp mbhangui $
#
trap "" 1 2 3
if [ $# -ne 2 ] ; then
	echo "ERROR: USAGE: domain_name smtp_host" 1>&2
	exit 1
elif [ " $TCPREMOTEIP" = " " ] ; then
	echo "ERROR: TCPREMOTEIP not set" 1>&2
	exit 1
fi
PATH=/bin:/usr/bin:$PATH
cd QMAILHOME/autoturn
if [ -d $1 ] ; then
	count=`for i in $1/Maildir/new $1/Maildir/cur ; do /bin/ls $i; done|wc -l`
	PREFIX/bin/setlock -nx $1/seriallock rm $1/seriallock
	if [ -f $1/seriallock ] ; then
		if [ $count -eq 0 ] ; then
			exit 3
		fi
		exit 4
	fi
elif [ -d $TCPREMOTEIP ] ; then
	count=`ls $TCPREMOTEIP/Maildir/new/* $TCPREMOTEIP/Maildir/cur/* 2>/dev/null | wc -l`
	PREFIX/bin/setlock -nx $TCPREMOTEIP/seriallock rm $TCPREMOTEIP/seriallock
	if [ -f $TCPREMOTEIP/seriallock ] ; then
		if [ $count -eq 0 ] ; then
			exit 3
		fi
		exit 4
	fi
fi
if [ $count -eq 0 ] ; then
	exit 3
fi
set -o pipefail
if [ -d $1 ] ; then
	ipme=`LIBEXEC/ipmeprint | awk '{print $3]'`
	if [ $? -ne 0 ] ; then
		echo "Unable to get local ip addresses" 1>&2
		exit 1
	fi
	mxip=`LIBEXEC/dnsmxip`
	if [ $? -ne 0 ] ; then
		echo "Unable to get MX for $1" 1>&2
		exit 1
	fi
	for i in $mxip
	do
		for j in $ipme
		do
			if [ ! " $i" = " $j" ] ; then
				IP=$i
				break
			fi
		done
		if [ ! " $IP" = " " ] ; then
			break
		fi
	done
	if [  " $IP" = " " ] ; then
		echo "Unable to locate MX for $1" 1>&2
		exit 1
	fi
fi
(
if [ -d $1 ] ; then
	PREFIX/bin/setlock -nx $1/seriallock PREFIX/bin/maildirsmtp $1/Maildir/ "$1"- $2 AutoTURN
	PREFIX/bin/setlock -nx $1/seriallock PREFIX/bin/resetquota \
		$1/Maildir
	PREFIX/bin/setlock -nx $1/seriallock rm $1/seriallock
elif [ -d $TCPREMOTEIP ] ; then
	PREFIX/bin/setlock -nx $TCPREMOTEIP/seriallock PREFIX/bin/maildirsmtp $TCPREMOTEIP/Maildir/ \
		autoturn-$TCPREMOTEIP- $TCPREMOTEIP AutoTURN
	PREFIX/bin/setlock -nx $1/seriallock PREFIX/bin/resetquota \
		$1/Maildir
	PREFIX/bin/setlock -nx $TCPREMOTEIP/seriallock rm $TCPREMOTEIP/seriallock
else
	exit 4
fi
) &
exit 0
