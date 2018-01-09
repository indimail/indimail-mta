#
# $Log: zsenders.sh,v $
# Revision 1.8  2017-04-12 14:53:43+05:30  Cprogrammer
# report programs moved to libexecdir
#
# Revision 1.7  2017-03-09 16:39:57+05:30  Cprogrammer
# FHS changes
#
# Revision 1.6  2016-06-17 20:21:08+05:30  Cprogrammer
# qmailanalog scripts moved to libexec dir
#
# Revision 1.5  2011-11-13 15:40:32+05:30  Cprogrammer
# fix for ubuntu (sort is /usr/bin/sort)
#
# Revision 1.4  2008-05-22 19:49:18+05:30  Cprogrammer
# change for posix compliant sort
#
# Revision 1.3  2004-01-03 13:45:21+05:30  Cprogrammer
# *** empty log message ***
#
# Revision 1.2  2004-01-03 00:32:40+05:30  Cprogrammer
# changed sort order to mess and bytes
#
# Revision 1.1  2004-01-02 23:40:59+05:30  Cprogrammer
# Initial revision
#
#
echo 'Senders sorted on mess and bytes

One line per sender. Information on each line:
* mess is the number of messages sent by this sender.
* bytes is the number of bytes sent by this sender.
* sbytes is the number of bytes successfully received from this sender.
* rbytes is the number of bytes from this sender, weighted by recipient.
* recips is the number of recipients (success plus failure).
* tries is the number of delivery attempts (success, failure, deferral).
* xdelay is the total xdelay incurred by this sender.
'
( 
echo mess bytes sbytes rbytes recips tries xdelay sender
LIBEXEC/senders | sort -n -r -k 1,1 -k 2,2 -k 3,3 -k 4,4 -k 6,6
) | LIBEXEC/columnt
