#
# $Id: etrn.sh,v 1.13 2023-12-03 15:30:08+05:30 Cprogrammer Exp mbhangui $
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
	echo "$nm: USAGE: domain_name dir smtp_host" 1>&2
	exit 1
elif [ -z "$TCPREMOTEIP" ] ; then
	echo "$nm: TCPREMOTEIP not set" 1>&2
	exit 1
fi
domain=$1
dir=$2
smtp_host=$3
cd QMAILHOME/autoturn
if [ -d $dir ] ; then
	count=`for i in $dir/new $dir/cur ; do /bin/ls $i; done|wc -l`
	if [ $count -eq 0 ] ; then
		echo "$nm: No pending messages for domain $domain, ip=$3 dir=$dir, pid=$$" 1>&2
		exit 3
	fi
else
	echo "$nm: Trouble accessing directory for domain $domain, ip=$3 dir=$dir, pid=$$" 1>&2
	exit 1
fi
set -o pipefail
#
# we need to ensure that the host / ip address to which we
# are going to send emails for the domain has a valid MX
# record for the domain. Else we reject the request as we don't
# want any joe, o[b,s]ama, israel or any terrorist to fetch
# mails not meant for them
#
if [ -d $domain ] ; then
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
	if [ -z "$IP" -a -f $domain/ipauth ] ; then
		for i in $(cat $domain/ipauth)
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
	if [  -z "$IP" ] ; then
		echo "$nm: server is not mail exchanger for $domain" 1>&2
		exit 2
	fi
fi
(
if [ -d $domain ] ; then
	prefix="$domain""-"
elif [ -d $smtp_host ] ; then
	prefix=autoturn-"$smtp_host"-
else
	echo "$nm: Trouble accessing directory for domain $domain, ip=$3 dir=$dir, pid=$$" 1>&2
	exit 1
fi
PREFIX/bin/maildirsmtp $dir $prefix $smtp_host AutoTURN
if [ -f $dir/maildirsize ] ; then
	PREFIX/bin/resetquota $dir
fi
) &
exit 0

#
# $Log: etrn.sh,v $
# Revision 1.13  2023-12-03 15:30:08+05:30  Cprogrammer
# use ipauth as IP address whitellist
# refactored code
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
