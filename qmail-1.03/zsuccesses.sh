#
# $Log: zsuccesses.sh,v $
# Revision 1.4  2011-11-13 15:40:44+05:30  Cprogrammer
# fix for ubuntu (sort is /usr/bin/sort)
#
# Revision 1.3  2008-05-22 19:49:49+05:30  Cprogrammer
# change for posix compliant sort
#
# Revision 1.2  2004-01-03 13:45:23+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.1  2004-01-02 23:41:02+05:30  Cprogrammer
# Initial revision
#
#
echo 'Reasons for success

One line per reason for successful delivery. Information on each line:
* del is the number of deliveries that ended for this reason.
* xdelay is the total xdelay on those deliveries.
'
(
echo del xdelay reason
QMAIL/bin/successes | sort -k 2,2 -k 1,1
) | QMAIL/bin/columnt | tr _ ' '
