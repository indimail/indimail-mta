#!/bin/bash
# Version 0.1
# This script locates all messages currently in qmail's queue and 
# calls the qmail-queue-print script, which in part will extract the
# from, to and date headers from every message and display them to
# the user <STDOUT>
# if called wih "-h" argument then the message's header will also be 
# displayed
# Written by Alex Kramarov, comments and suggestions please send to 
# alex@incredimail.com 
if [ " $QUEUE_BASE" = " " ] ; then
	queues=`echo QMAIL/queue*`
else
	queues=`echo $QUEUE_BASE/queue*`
fi
for i in $queues
do
	queuedir=$i
	echo "$queuedir"
	first_char=`echo $queuedir | cut -c1`
	if [ " $first_char" = " /" -o " $first_char" = " ." ] ; then
		find $queuedir/mess/ -type f| QMAIL/bin/qmail-queue-print $*
	else
		find QMAIL/$queuedir/mess/ -type f| QMAIL/bin/qmail-queue-print $*
	fi
done
