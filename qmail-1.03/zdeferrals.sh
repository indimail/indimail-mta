#
# $Log: zdeferrals.sh,v $
# Revision 1.3  2008-05-22 19:48:34+05:30  Cprogrammer
# changes for posix compliant sort
#
# Revision 1.2  2004-01-03 13:44:38+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.1  2004-01-02 23:40:50+05:30  Cprogrammer
# Initial revision
#
#
echo 'Reasons for deferral

One line per reason for deferral sorted by xdelay. Information on each line:
* del is the number of deliveries that ended for this reason.
* xdelay is the total xdelay on those deliveries.
'
(
echo del xdelay reason
QMAIL/bin/deferrals | /bin/sort -n -r -k 2,2
) | QMAIL/bin/columnt | tr _ ' '
