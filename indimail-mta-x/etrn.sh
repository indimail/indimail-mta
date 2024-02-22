#
# $Id: etrn.sh,v 1.14 2024-02-22 01:04:32+05:30 Cprogrammer Exp mbhangui $
#
# 0 - queueing started
# 1 - System Error
# 2 - Domain Rejected
# 3 - No Pending message for node
# 4 - Pending message for node
#
#
trap "" 1 2 3
PATH=/bin:/usr/bin:$PATH
nm=$(basename $0)

if [ $# -ne 3 ] ; then
	echo "$nm: USAGE: domain_name autoturn_dir dir" 1>&2
	exit 1
elif [ -z "$TCPREMOTEIP" -a -z "$TCP6REMOTEIP" ] ; then
	echo "$nm: TCPREMOTEIP not set" 1>&2
	exit 1
fi
domain=$1
autoturn_dir=$2
domain_dir=$3
echo "$nm: $$: domain=$domain, autoturn=$2, domain_dir=$domain_dir" 1>&2

t=$(basename $domain_dir)
qfn=$(echo $t | sed -e 's{\.{:{g')
if [ -f $autoturn_dir/.qmail-$qfn-default ] ; then
	dir=$(qcat $autoturn_dir/.qmail-$qfn-default)
	if [ $? -ne 0 ] ; then
		echo "$nm: $autoturn_dir/.qmail-$qfn-default: read error"
		exit 1
	fi
else
	echo "$nm: $autoturn_dir/.qmail-$qfn-default: No such file or directory"
	exit 1
fi

if [ -d $dir ] ; then
	count=`for i in $dir/new $dir/cur ; do /bin/ls $i; done | wc -l`
	if [ $? -ne 0 ] ; then
		echo "$nm: Trouble accessing files in dir $dir for $domain [$domain_dir], pid=$$" 1>&2
		exit 1
	fi
	if [ $count -eq 0 ] ; then
		echo "$nm: No pending messages for domain $domain, dir=$dir [$domain_dir], pid=$$" 1>&2
		exit 3
	fi
else
	echo "$nm: Trouble accessing directory dir $dir for $domain [$domain_dir], pid=$$" 1>&2
	exit 1
fi
set -o pipefail
#
# we need to ensure the following
#
# 1. For domain based etrn domain, the host / ip address to which we
#    are going to send emails for the domain must ba valid MX
#    record for the domain or $TCPREMOTEIP should be present in
#    $domain/ipauth
#    in virtualdomains entry for domain based etrn domain will be like this
#    if domain is etrn1.dom
#    etrn2.dom:autoturn-etrn1.dom
#    domain based etrn can also be created by using vadddomain -t etrn1.dom
#
# 2. For IP based etrn domain if $TCPREMOTEIP is same as the IP
#    send emails to the IP. Else do same as 1.
#    in virtualdomains entry for IP based etrn domain will be like this
#    if domain is etrn2.dom
#    etrn2.dom:autoturn-etrn2.dom
#    IP based etrn can also be created by using vadddomain -T IP -t etrn2.dom
#
# Else we reject the request as we don't want any
# genocidal joes, o[b,s]amas, israel or any terrorist to fetch
# mails not meant for them
#

# First check if TCPREMOTEIP matches ip of the ETRN domain
if [ "$t" = "$TCPREMOTEIP" ] ; then
	IP=$TCPREMOTEIP
elif [ "$t" = "$TCP6REMOTEIP" ] ; then
	IP=$TCP6REMOTEIP
else
	IP=""
fi

# second check if $TCPREMOTEIP is in $domain/ipauth
if [ -z "$IP" -a -d $domain_dir -a -f $domain_dir/ipauth ] ; then
	for i in $(qcat $domain_dir/ipauth)
	do
		for j in $TCPREMOTEIP $TCP6REMOTEIP
		do
			if [ "$i" = "$j" ] ; then
				IP="$i"
				break
			fi
		done
	done
fi

# third check to see if $TCPREMOTEIP is a MX record
# for the ETRN domain
if [ -z "$IP" -a -d $domain ] ; then
	ipme=$(LIBEXEC/ipmeprint | awk '{print $3}')
	if [ $? -ne 0 ] ; then
		echo "$nm: Unable to get local ip addresses" 1>&2
		exit 1
	fi
	mxip=$(LIBEXEC/dnsmxip $domain | awk '{print $2}')
	if [ $? -ne 0 ] ; then
		echo "$nm: Unable to get MX for $domain" 1>&2
		exit 1
	fi
	IP=""
	for i in $mxip
	do
		for j in $TCPREMOTEIP $TCP6REMOTEIP
		do
			if [ "$i" = "$j" ] ; then
				IP="$i"
				break
			fi
		done
	done
	if [  -z "$IP" ] ; then
		if [ -n "$TCPREMOTEIP" ] ; then
			echo "$nm: client $TCPREMOTEIP is not mail exchanger for $domain" 1>&2
		elif [ -n "$TCPREMOTEIP" -a -n "$TCP6REMOTEIP" ] ; then
			echo "$nm: client $TCPREMOTEIP, $TCP6REMOTEIP are not mail exchangers for $domain" 1>&2
		elif [ -n "$TCP6REMOTEIP" ] ; then
			echo "$nm: client $TCP6REMOTEIP is not mail exchanger for $domain" 1>&2
		fi
		exit 2
	fi
	IP=""
	for i in $mxip
	do
		for j in $ipme
		do
			if [ "$i" = "$j" ] ; then
				IP="$i"
				break
			fi
		done
	done
fi
if [  -z "$IP" ] ; then
	echo "$nm:server not mail exchanger for $domain" 1>&2
	exit 2
fi
prefix=autoturn-"$t""-"
echo "$nm: Executing maildirsmtp $dir $prefix $IP AutoTURN count=$count" 1>&2
(
PREFIX/bin/maildirsmtp $dir $prefix $IP AutoTURN
if [ -f $dir/maildirsize ] ; then
	PREFIX/bin/resetquota $dir
fi
) &
exit 0

#
# $Log: etrn.sh,v $
# Revision 1.14  2024-02-22 01:04:32+05:30  Cprogrammer
# replace cat with qcat
#
# Revision 1.13  2023-12-09 11:47:51+05:30  Cprogrammer
# read .qmail-domain-default to get Maildir directory
#
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
