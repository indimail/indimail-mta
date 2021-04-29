#
# $Log: qpq.sh,v $
# Revision 1.7  2021-04-29 10:04:27+05:30  Cprogrammer
# replaced QMAIL with QMAILHOME
#
# Revision 1.6  2016-06-17 18:27:32+05:30  Cprogrammer
# FHS compliance
#
# Revision 1.5  2014-06-13 17:57:44+05:30  Cprogrammer
# replaced qmail-queue-print with 822header
#
#
# This script locates all messages currently in qmail's queue and 
# calls the 822header program.
#
# $Id: qpq.sh,v 1.7 2021-04-29 10:04:27+05:30 Cprogrammer Exp mbhangui $
#

usage ()
{
	echo "Wrong Usage"
	echo "$0 [options]"
	echo "-h	Print Headers only"
	echo "-H	Pass arguments to 822header(1)"
}

args822="-I From -I To -I Date"
while getopts hH:q: opt
do
	case $opt in
		q) qbase=$OPTARG;;
		h) args822="";;
		H) args822=$OPTARG;;
		?) usage $0;exit 1;;
	esac
done

if [ -z "$qbase" ] ; then
	if [ -d QMAILHOME/queue/nqueue ] ; then
		queues=`echo QMAILHOME/queue/*`
	else
		queues=`echo QMAILHOME/queue*`
	fi
else
	if [ -d $qbase/nqueue ] ; then
		queues=`echo $qbase/*`
	else
		queues=`echo $qbase/queue*`
	fi
fi
count=1
for i in $queues
do
	queuedir=$i
	first_char=`echo $queuedir | cut -c1`
	if [ " $first_char" = " /" -o " $first_char" = " ." ] ; then
		for i in `find $queuedir/mess/ -type f`
		do
			echo "$count: Message $i"
			echo
			/bin/cat $i | PREFIX/bin/822header $args822
			echo "----------------------------------- END ------------------------------------"
			count=`expr $count + 1`
		done
		#| PREFIX/bin/qmail-queue-print $*
	else
		for i in `find QMAILHOME/$queuedir/mess/ -type f`
		do
			echo "$count: Message $i"
			echo
			/bin/cat $i | PREFIX/bin/822header $args822
			echo "----------------------------------- END ------------------------------------"
			count=`expr $count + 1`
		done
	fi
done
