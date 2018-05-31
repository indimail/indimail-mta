#
# $Log: zrhosts.sh,v $
# Revision 1.7  2017-04-12 14:53:34+05:30  Cprogrammer
# report programs moved to libexecdir
#
# Revision 1.6  2017-03-09 16:39:50+05:30  Cprogrammer
# FHS changes
#
# Revision 1.5  2016-06-17 20:20:52+05:30  Cprogrammer
# *** empty log message ***
#
# Revision 1.4  2011-11-13 15:40:29+05:30  Cprogrammer
# fix for ubuntu (sort is /usr/bin/sort)
#
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
LIBEXEC/rhosts | sort -n -r -k 1.1,1.0 -k 2,2 -k 4,4 -k 3,3
) | LIBEXEC/columnt
