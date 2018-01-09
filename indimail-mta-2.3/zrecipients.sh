#
# $Log: zrecipients.sh,v $
# Revision 1.8  2017-04-12 14:53:30+05:30  Cprogrammer
# report programs moved to libexecdir
#
# Revision 1.7  2017-03-09 16:39:47+05:30  Cprogrammer
# FHS changes
#
# Revision 1.6  2016-06-17 20:20:45+05:30  Cprogrammer
# qmailanalog scripts moved to libexec dir
#
# Revision 1.5  2011-11-13 15:40:26+05:30  Cprogrammer
# fix for ubuntu (sort is /usr/bin/sort)
#
# Revision 1.4  2008-05-22 19:49:11+05:30  Cprogrammer
# change for posix compliant sort
#
# Revision 1.3  2004-01-03 13:44:58+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.2  2004-01-03 00:32:27+05:30  Cprogrammer
# changed sort order to sbytes and mess
#
# Revision 1.1  2004-01-02 23:40:55+05:30  Cprogrammer
# Initial revision
#
#
echo 'Recipients sorted on sbytes and mess

One line per recipient. Information on each line:
* sbytes is the number of bytes successfully delivered to this recipient.
* mess is the number of messages sent to this recipient (success plus failure).
* tries is the number of delivery attempts (success, failure, deferral).
* xdelay is the total xdelay incurred by this recipient.
'
(
echo sbytes mess tries xdelay recipient
LIBEXEC/recipients | sort -n -r -k 1,1 -k 2,2 -k 4,4 -k 3,3
) | LIBEXEC/columnt
