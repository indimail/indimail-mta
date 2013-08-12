#!/bin/sh
#
# $Log: surblqueue.sh,v $
# Revision 1.3  2013-08-12 18:56:37+05:30  Cprogrammer
# added error message if mktemp fails
#
# Revision 1.2  2013-08-12 15:00:54+05:30  Cprogrammer
# figure out mktemp path
#
# Revision 1.1  2011-07-15 11:48:51+05:30  Cprogrammer
# Initial revision
#
#
# $Id: surblqueue.sh,v 1.3 2013-08-12 18:56:37+05:30 Cprogrammer Exp mbhangui $
#
# I should be called by qmail-smtpd or anything that calls qmail-queue
#
if [ -f /bin/mktemp ] ; then
	MKTEMP=/bin/mktemp
elif [ -f /usr/bin/mktemp ] ; then
	MKTEMP=/usr/bin/mktemp
else
	MKTEMP=mktemp
fi
out=`$MKTEMP -t surblXXXXXXXXXX`
if [ $? -ne 0 ] ; then
	echo "mktemp: unable to create temp files" 1>&2
	exit 111
fi
#
# run surblfilter and feed output to qmail-queue
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
