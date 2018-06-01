#
# $Log: zrxdelay.sh,v $
# Revision 1.6  2017-04-12 14:53:39+05:30  Cprogrammer
# report programs moved to libexecdir
#
# Revision 1.5  2017-03-09 16:39:53+05:30  Cprogrammer
# FHS changes
#
# Revision 1.4  2016-06-17 20:21:00+05:30  Cprogrammer
# qmailanalog scripts moved to libexec dir
#
# Revision 1.3  2008-05-25 17:21:57+05:30  Cprogrammer
# sort on the first field
#
# Revision 1.2  2004-01-03 13:45:11+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.1  2004-01-02 23:40:58+05:30  Cprogrammer
# Initial revision
#
#
echo 'Recipients in the worst order for mailing lists

One line per recipient, sorted by avg. Information on each line:
* avg is the _average_ xdelay for the recipient.
* tries is the number of deliveries that avg is based on.
'
(
echo avg tries recipient
LIBEXEC/recipients | LIBEXEC/rxdelay | sort -n -r -k 1,1
) | LIBEXEC/columnt
