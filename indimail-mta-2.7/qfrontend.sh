#
# $Log: qfrontend.sh,v $
# Revision 1.1  2019-02-21 14:17:16+05:30  Cprogrammer
# Initial revision
#
#
qf=""
for i in `echo LIBEXEC/qfilters/qf-*`
do
	if [ -n "$qf" ] ; then
		qf="$qf -- $i"
	else
		qf="$i"
	fi
done
eval qmail-qfilter $qf
exit $?
