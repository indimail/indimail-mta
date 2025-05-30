#
# $Log: qf-smtp-ratelimit.sh,v $
# Revision 1.5  2024-02-22 01:04:57+05:30  Cprogrammer
# replace cat with qcat
#
# Revision 1.4  2024-02-20 22:23:49+05:30  Cprogrammer
# make script compatible with FILTERARGS as well as qmail-qfilter
#
# Revision 1.3  2021-08-03 15:54:52+05:30  Cprogrammer
# replaced /bin/cat with exit 0 which does the same thing
#
# Revision 1.2  2021-08-03 00:31:16+05:30  Cprogrammer
# multiply by 3600 to convert to hourly rate
#
# Revision 1.1  2019-02-21 14:17:30+05:30  Cprogrammer
# Initial revision
#
#

set -e
[ -n "$ENVSIZE" -a -n "$MSGSIZE" ] && qmail_qfilter=1 || qmail_qfilter=0
if [ ! -d HOME/domains/$QMAILHOST ] ; then
	[ $qmail_qfilter -eq 1 ] && exit 0 || exec @prefix@/bin/qcat
fi
if [ ! -d HOME/domains/$QMAILHOST/smtp_timestamps ] ; then
	[ $qmail_qfilter -eq 1 ] && exit 0 || exec @prefix@/bin/qcat
fi
[ -z "$SMTP_RATE" ] && SMTP_RATE=30
cur_time=$(date +%s)
if [ -f HOME/domains/$QMAILHOST/smtp_timestamps/"$QMAILUSER"@"$QMAILHOST".t ] ; then
	ftime=$(stat -c "%Y" HOME/domains/$QMAILHOST/smtp_timestamps/"$QMAILUSER"@"$QMAILHOST".t)
	diff=$(expr $cur_time - $ftime)
	mcount=$(qcat HOME/domains/$QMAILHOST/smtp_timestamps/"$QMAILUSER"@"$QMAILHOST".t)
	rate=$(expr 3600 \* $mcount / $diff)
	if [ $rate -gt $SMTP_RATE ] ; then
		echo "Zquota rate [$rate] > $SMTP_RATE exceeded for $QMAILUSER@$QMAILHOST" 1>&2
		exit 88
	else
		diff=$(expr $cur_time - $ftime)
		if [ $diff -gt 1800 ] ; then
			echo 1 > HOME/domains/$QMAILHOST/smtp_timestamps/"$QMAILUSER"@"$QMAILHOST".t
			[ $qmail_qfilter -eq 1 ] && exit 0 || exec @prefix@/bin/qcat
		else
			mcount=$(expr $mcount + 1)
			echo $mcount > HOME/domains/$QMAILHOST/smtp_timestamps/"$QMAILUSER"@"$QMAILHOST".t
			[ $qmail_qfilter -eq 1 ] && exit 0 || exec @prefix@/bin/qcat
		fi
	fi
else
	echo 1 > HOME/domains/$QMAILHOST/smtp_timestamps/"$QMAILUSER"@"$QMAILHOST".t
	[ $qmail_qfilter -eq 1 ] && exit 0 || exec @prefix@/bin/qcat
fi
