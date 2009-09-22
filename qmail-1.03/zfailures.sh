#
# $Log: zfailures.sh,v $
# Revision 1.3  2008-05-22 19:48:55+05:30  Cprogrammer
# change for posix compliant sort
#
# Revision 1.2  2004-01-03 13:44:48+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.1  2004-01-02 23:40:52+05:30  Cprogrammer
# Initial revision
#
#
echo 'Reasons for failure

One line per reason for delivery failure. Information on each line:
* del is the number of deliveries that ended for this reason.
* xdelay is the total xdelay on those deliveries.
'
(
echo del xdelay reason
QMAIL/bin/failures | /bin/sort -n -r -k 2,2
) | QMAIL/bin/columnt | tr _ ' '
