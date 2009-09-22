#
# $Log: zrecipients.sh,v $
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
QMAIL/bin/recipients | /bin/sort -n -r -k 1,1 -k 2,2 -k 4,4 -k 3,3
) | QMAIL/bin/columnt
