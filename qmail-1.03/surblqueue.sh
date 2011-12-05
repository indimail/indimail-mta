#!/bin/sh
#
# $Log: surblqueue.sh,v $
# Revision 1.1  2011-07-15 11:48:51+05:30  Cprogrammer
# Initial revision
#
#
# $Id: surblqueue.sh,v 1.1 2011-07-15 11:48:51+05:30 Cprogrammer Stab mbhangui $
#
# I should be called by qmail-smtpd or anything that calls qmail-queue
#
out=`/bin/mktemp -t surblXXXXXXXXXX`
if [ $? -ne 0 ] ; then
	exit 111
fi
#
# Redirect standard error to 4 so that qmail_open() will pick up the error
#
QMAIL/bin/surblfilter > $out
status=$?
if [ $status -eq 0 ] ; then
	exec 0<$out
	/bin/rm -f $out
	exec QMAIL/bin/qmail-queue
else
	/bin/rm -f $out
	exit $status
fi
