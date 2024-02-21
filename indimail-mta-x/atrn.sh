#
# $Id: atrn.sh,v 1.14 2024-02-22 01:04:03+05:30 Cprogrammer Exp mbhangui $
#
# 0 - queueing started
# 1 - System Error
# 2 - Domain Rejected
# 3 - No Pending message for node
# 4 - Pending message for node
#
trap "" 1 2 3
PATH=/bin:/usr/bin:$PATH
nm=$(basename $0)

if [ $# -ne 3 ] ; then
	echo "$nm: ERROR: USAGE: domain_name autoturn_dir dir" 1>&2
	exit 1
elif [ -z "$TCPREMOTEIP" -a -z "$TCP6REMOTEIP" ] ; then
	echo "$nm: ERROR: TCPREMOTEIP not set" 1>&2
	exit 1
fi

domain=$1
autoturn_dir=$2
domain_dir=$3
echo "$nm: $$: domain=$domain, autoturn=$2, domain_dir=$domain_dir" 1>&2

[ -z "$CONTROLDIR" ] && CONTROLDIR=@controldir@
slash=$(echo $CONTROLDIR | cut -c1)
[ ! " $slash" = " /" ] && cd SYSCONFDIR
[ -f "$CONTROLDIR"/queuelifetime ] && LIFETIME=$(qmail-cat "$CONTROLDIR"/queuelifetime) || LIFETIME=1209600
[ -z "$LIFETIME" ] && LIFETIME=1209600

t=$(basename $domain_dir)
qfn=$(echo $t | sed -e 's{\.{:{g')
cd $autoturn_dir
if [ -f $autoturn_dir/.qmail-$qfn-default ] ; then
	dir=$(qmail-cat $autoturn_dir/.qmail-$qfn-default)
else
	echo "$nm: $autoturn_dir/.qmail-$qfn-default: No such file or directory"
	exit 1
fi

# remember that $domain_dir and $dir need not be same
if [ -d $dir ] ; then
	count=$(for i in $dir/new $dir/cur ; do /bin/ls $i; done| wc -l)
	if [ $? -ne 0 ] ; then
		echo "$nm: Trouble accessing files in dir $dir for $domain, pid=$$" 1>&2
		exit 1
	fi
	if [ $count -eq 0 ] ; then
		echo "$nm: No pending messages for domain $domain, dir=$dir, pid=$$" 1>&2
		exit 3 # no messages in queue
	fi
else
	echo "$nm: Trouble accessing directory for domain $domain, dir=$dir, pid=$$" 1>&2
	exit 1
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
[ -f $domain_dir/.qvirtual ] && qvirtual=$(qmail-cat $domain_dir/.qvirtual) || qvirtual=$domain

prefix="autoturn-""$qvirtual""-"
echo "$nm: Executing maildirserial -b -t $LIFETIME $dir $prefix serialsmtp $prefix AutoTURN 1 qv=$qvirtual count=$count" 1>&2
# serialsmtp prefix helohost [do-not-quit]
PREFIX/bin/maildirserial -b -t $LIFETIME $dir "$prefix" \
	PREFIX/bin/serialsmtp "$prefix" AutoTURN
if [ -f $dir/maildirsize ] ; then
	PREFIX/sbin/resetquota $dir
fi
exit 0
#
# $Log: atrn.sh,v $
# Revision 1.14  2024-02-22 01:04:03+05:30  Cprogrammer
# replace cat with qmail-cat
#
# Revision 1.13  2023-12-09 11:46:33+05:30  Cprogrammer
# read .qmail-domain-default to get Maildir directory
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
