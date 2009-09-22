#
# $Log: zrhosts.sh,v $
# Revision 1.3  2008-05-22 19:49:15+05:30  Cprogrammer
# change for posix compliant sort
#
# Revision 1.2  2004-01-03 13:45:06+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.1  2004-01-02 23:40:56+05:30  Cprogrammer
# Initial revision
#
#
echo 'Recipient hosts sorted on sbytes and mess

One line per recipient host. Information on each line:
* sbytes is the number of bytes successfully delivered to this host.
* mess is the number of messages sent to this host (success plus failure).
* tries is the number of delivery attempts (success, failure, deferral).
* xdelay is the total xdelay incurred by this host.
'
(
echo sbytes mess tries xdelay host
QMAIL/bin/rhosts | /bin/sort -n -r -k 1.1,1.0 -k 2,2 -k 4,4 -k 3,3
) | QMAIL/bin/columnt
