#
# $Log: qfrontend.sh,v $
# Revision 1.5  2024-02-20 22:21:46+05:30  Cprogrammer
# use control/filters.d directory for user defined filters
# make script compatible with both FILTERARGS and qmail-qfilter
#
# Revision 1.4  2024-02-19 22:46:16+05:30  Cprogrammer
# skip comments
#
# Revision 1.3  2023-09-08 00:55:40+05:30  Cprogrammer
# make libexecdir, controldir configurable using env variables
#
# Revision 1.2  2021-08-03 22:35:32+05:30  Cprogrammer
# use control file control/qfilters to configure qmail-qfilter filters
#
# Revision 1.1  2019-02-21 14:17:16+05:30  Cprogrammer
# Initial revision
#
#
qf=""
if [ -n "$USE_QMAILQFILTER" ] ; then
	qqfilter=1
else
	qqfilter=0
fi
unset SPAMFILTER
unset FILTERARGS
[ -n "$CONTROLDIR" ] && controldir=$CONTROLDIR || controldir=@controldir@
[ -n "$LIBEXECDIR" ] && libexecdir=$LIBEXECDIR || libexecdir=@libexecdir@
if [ -d $controldir/filters.d ] ; then
	for i in $(find $controldir/filters.d -maxdepth 1 -type f)
	do
		if [ $qqfilter -eq 1 ] ; then
			if [ -n "$qf" ] ; then
				qf="$qf -- $i"
			else
				qf="$i"
			fi
		else
			if [ -n "$qf" ] ; then
				qf="$qf | $i"
			else
				qf=$i
			fi
		fi
	done
else
	if [ ! -f $controldir/qfilters ] ; then
		exec @prefix@/sbin/qmail-queue
	fi
	for i in $(grep -v "^#" $controldir/qfilters)
	do
		j=$libexecdir/qfilters/$i
		if [ $qqfilter -eq 1 ] ; then
			if [ -n "$qf" ] ; then
				qf="$qf -- $j"
			else
				qf="$j"
			fi
		else
			if [ -n "$qf" ] ; then
				qf="$qf | $j"
			else
				qf=$j
			fi
		fi
	done
fi
if [ -z "$qf" ] ; then
	exec @prefix@/sbin/qmail-queue
else
	if [ $qqfilter -eq 1 ] ; then
		exec @prefix@/bin/qmail-qfilter $qf
	else
		FILTERARGS="$qf" exec @prefix@/sbin/qmail-multi
	fi
fi
exit $?
