#
# $Log: qfrontend.sh,v $
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
[ -n "$CONTROLDIR" ] && controldir=$CONTROLDIR || controldir=@controldir@
[ -n "$LIBEXECDIR" ] && libexecdir=$LIBEXECDIR || libexecdir=@libexecdir@
if [ ! -f $controldir/qfilters ] ; then
	exec qmail-multi
fi
for i in $(cat $controldir/qfilters)
do
	j=$libexecdir/qfilters/$i
	if [ -n "$qf" ] ; then
		qf="$qf -- $j"
	else
		qf="$j"
	fi
done
eval qmail-qfilter $qf
exit $?
