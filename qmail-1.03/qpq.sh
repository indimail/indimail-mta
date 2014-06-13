#
# $Log: $
#
# This script locates all messages currently in qmail's queue and 
# calls the 822header program.
#
# $Id: dk-filter.sh,v 1.18 2014-03-12 08:50:48+05:30 Cprogrammer Stab mbhangui $
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
	if [ -d QMAIL/queue/nqueue ] ; then
		queues=`echo QMAIL/queue/*`
	else
		queues=`echo QMAIL/queue*`
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
			/bin/cat $i | QMAIL/bin/822header $args822
			echo "----------------------------------- END ------------------------------------"
			count=`expr $count + 1`
		done
		#| QMAIL/bin/qmail-queue-print $*
	else
		for i in `find QMAIL/$queuedir/mess/ -type f`
		do
			echo "$count: Message $i"
			echo
			/bin/cat $i | QMAIL/bin/822header $args822
			echo "----------------------------------- END ------------------------------------"
			count=`expr $count + 1`
		done
	fi
done
