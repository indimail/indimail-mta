#
# $Log: zsuids.sh,v $
# Revision 1.7  2017-04-12 14:54:01+05:30  Cprogrammer
# report programs moved to libexecdir
#
# Revision 1.6  2017-03-09 16:40:12+05:30  Cprogrammer
# FHS changes
#
# Revision 1.5  2016-06-17 20:21:53+05:30  Cprogrammer
# qmailanalog scripts moved to libexec dir
#
# Revision 1.4  2011-11-13 15:40:47+05:30  Cprogrammer
# fix for ubuntu (sort is /usr/bin/sort)
#
# Revision 1.3  2008-05-22 19:49:57+05:30  Cprogrammer
# change for posix compliant sort
#
# Revision 1.2  2004-01-03 13:45:29+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.1  2004-01-02 23:41:04+05:30  Cprogrammer
# Initial revision
#
#
echo 'Sender uids sorted on mess and bytes

One line per sender uid. Information on each line:
* mess is the number of messages sent by this uid.
* bytes is the number of bytes sent by this uid.
* sbytes is the number of bytes successfully received from this uid.
* rbytes is the number of bytes from this uid, weighted by recipient.
* recips is the number of recipients (success plus failure).
* tries is the number of delivery attempts (success, failure, deferral).
* xdelay is the total xdelay incurred by this uid.
'
(
echo mess bytes sbytes rbytes recips tries xdelay uid
LIBEXEC/suids | sort -n -r -k 1,1 -k 2,2 -k 3,3 -k 4,4
) | LIBEXEC/columnt
