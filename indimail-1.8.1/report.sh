#!/bin/sh
LOGDIR=/var/log/indimail
(
if [ -f $LOGDIR/pend.1 ] ; then
	cat $LOGDIR/pend.1
fi
cat $LOGDIR/deliver.25/current 
) | @qmaildir@/multilog-matchup 5>$LOGDIR/pend.2
/bin/mv $LOGDIR/pend.2 $LOGDIR/pend.1
