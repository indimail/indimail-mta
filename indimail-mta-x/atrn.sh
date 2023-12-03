#
# $Id: atrn.sh,v 1.13 2023-12-03 15:25:54+05:30 Cprogrammer Exp mbhangui $
#
# 0 - queueing started
# 1 - System Error
# 2 - Domain Rejected
# 3 - No Pending message for node
# 4 - Pending message for node
#
trap "" 1 2 3
if [ $# -lt 2 ] ; then
	echo "atrn: ERROR: USAGE: domain_name(s) remoteip" 1>&2
	exit 1
elif [ " $TCPREMOTEIP" = " " ] ; then
	echo "atrn: ERROR: TCPREMOTEIP not set" 1>&2
	exit 1
fi
INDIMAILDIR=$(grep -w "^indimail" /etc/passwd | cut -d: -f6|head -1)
if [ " $INDIMAILDIR" = " " ] ; then
	echo "atrn: Could not determine indimail home" 1>&2
	exit 1
fi
if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR=@controldir@
fi
slash=$(echo $CONTROLDIR | cut -c1)
if [ ! " $slash" = " /" ] ; then
	cd SYSCONFDIR
fi
if [ -f "$CONTROLDIR"/queuelifetime ] ; then
	LIFETIME=$(cat "$CONTROLDIR"/queuelifetime)
else
	LIFETIME=1209600
fi
PATH=/bin:/usr/bin:$PATH
cd QMAILHOME/autoturn
total=0
pend=0
domain=$1
count=$(for i in $domain/Maildir/new $domain/Maildir/cur ; do /bin/ls $i; done| wc -l)
if [ $count -eq 0 ] ; then
	exit 3 # no messages in queue
fi
#
# .qvirtual allows mails for a main domain to be
# distributed across multiple directories in 
# QMAILHOME/autoturn directory. i.e. if etrn.dom
# is the main domain and mails have been split
# into directories location1.etrn.dom and location2.etrn.dom
# specify .qvirtual having etrn.dom in QMAILHOME/autoturn/location1.etrn.dom
# and QMAILHOME/autoturn/location2.etrn.dom.
# once you have distributed mails for your users to
# location1.etrn.dom, location2.etrn.dom using valias or any other mechanism, this
# script can identify the domain identified by looking up the .qvirtual file
#
echo "250 OK now reversing the connection"
if [ -f $domain/.qvirtual ] ; then
	qvirtual=$(cat $domain/.qvirtual)
else
	qvirtual=$domain
fi
prefix="autoturn-""$qvirtual""-"
# serialsmtp prefix helohost [do-not-quit]
PREFIX/bin/maildirserial -b -t $LIFETIME $domain/Maildir "$prefix" \
	PREFIX/bin/serialsmtp "$prefix" AutoTURN 1
if [ -f $domain/Maildir/maildirsize ] ; then
	PREFIX/sbin/resetquota $domain/Maildir
fi
exit 0
#
# $Log: atrn.sh,v $
# Revision 1.13  2023-12-03 15:25:54+05:30  Cprogrammer
# factored code
#
# Revision 1.12  2023-11-26 19:51:03+05:30  Cprogrammer
# fixed path for resetquota
#
# Revision 1.11  2021-04-29 10:04:52+05:30  Cprogrammer
# replaced QMAIL with QMAILHOME
#
# Revision 1.10  2017-03-09 16:37:26+05:30  Cprogrammer
# FHS changes
#
# Revision 1.9  2016-05-17 23:11:42+05:30  Cprogrammer
# fix for configurable control directory
#
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
