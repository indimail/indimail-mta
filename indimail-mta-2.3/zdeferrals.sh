#
# $Log: zdeferrals.sh,v $
# Revision 1.7  2017-04-12 14:53:10+05:30  Cprogrammer
# report programs moved to libexecdir
#
# Revision 1.6  2017-03-09 16:39:40+05:30  Cprogrammer
# FHS changes
#
# Revision 1.5  2016-06-17 20:20:26+05:30  Cprogrammer
# qmailanalog scripts moved to libexec dir
#
# Revision 1.4  2011-11-13 15:39:55+05:30  Cprogrammer
# fix for ubuntu (sort is /usr/bin/sort)
#
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
LIBEXEC/deferrals | sort -n -r -k 2,2
) | LIBEXEC/columnt | tr _ ' '
