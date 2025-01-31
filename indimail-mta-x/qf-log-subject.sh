#
# $Log: qf-log-subject.sh,v $
# Revision 1.3  2024-02-22 01:04:53+05:30  Cprogrammer
# replace cat with qcat
#
# Revision 1.2  2024-02-20 22:20:27+05:30  Cprogrammer
# make script compatible with FILTERARGS as well as qmail-qfilter
#
# Revision 1.1  2021-08-03 16:15:05+05:30  Cprogrammer
# Initial revision
#
#
# $Id: qf-log-subject.sh,v 1.3 2024-02-22 01:04:53+05:30 Cprogrammer Exp mbhangui $
#

[ -n "$ENVSIZE" -a -n "$MSGSIZE" ] && qmail_qfilter=1 || qmail_qfilter=0
if [ ! -f /proc/$$/3 ] ; then
	[ $qmail_qfilter -eq 1 ] && exit 0 || exec @prefix@/bin/qcat
fi
# read headers
while read line
do
	if [ -z "$line" ] ; then
		break
	fi
	echo $line | grep "^Subject:" > /dev/null
	if [ $? -eq 0 ] ; then
		subject=$(echo $line | cut -d: -f2-)
	fi
	echo $line | grep "^Date:" > /dev/null
	if [ $? -eq 0 ] ; then
		date=$(echo $line | cut -d: -f2-)
	fi
done

# read envelope
var=$(tr ["\0"] ["\n"] 0<&3)
for i in $var
do
	first=$(echo $i|cut -c1)
	case "$first" in
		F)
		FROM=$(echo $i | cut -c2-)
		;;
		T)
		TO=$(echo $i|cut -c2-)
		;;
	esac
done
if [ -n "$TO" -a -n "$FROM" -a -n "$subject" -a -n "$date" ] ; then
	if [ -n "$LOGFILTER" ] ; then
		echo "Date:$date, Subject:$subject, From: $FROM, To: $TO" > $LOGFILTER
	else
		echo "Date:$date, Subject:$subject, From: $FROM, To: $TO" | @prefix@/sbin/splogger
	fi
	ret=$?
	[ $qmail_qfilter -eq 1 ] && exit $ret || @prefix@/bin/qcat && exit $ret
else
	[ $qmail_qfilter -eq 1 ] && exit 0 || exec @prefix@/bin/qcat
fi
