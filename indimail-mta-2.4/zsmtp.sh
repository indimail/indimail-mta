# $Log: zsmtp.sh,v $
# Revision 1.10  2017-04-13 13:08:57+05:30  Cprogrammer
# fixed columnt path
#
# Revision 1.9  2017-04-12 14:53:48+05:30  Cprogrammer
# report programs moved to libexecdir
#
# Revision 1.8  2017-03-09 16:40:00+05:30  Cprogrammer
# FHS changes
#
# Revision 1.7  2016-06-17 20:21:21+05:30  Cprogrammer
# qmailanalog scripts moved to libexec dir
#
# Revision 1.6  2011-11-13 15:40:36+05:30  Cprogrammer
# fix for ubuntu (sort is /usr/bin/sort)
#
# Revision 1.5  2008-05-22 19:49:31+05:30  Cprogrammer
# change for posix compliant sort
#
# Revision 1.4  2005-12-29 14:32:06+05:30  Cprogrammer
# fixed a line where qmail's home wasn't getting configured at compile time
#
# Revision 1.3  2004-01-08 00:33:10+05:30  Cprogrammer
# Changed report description
#
# Revision 1.2  2004-01-05 14:05:08+05:30  Cprogrammer
# added stats for embryonic connections
#
# Revision 1.1  2004-01-04 23:12:47+05:30  Cprogrammer
# Initial revision
#
# Revision 1.2  2004-01-03 13:45:06+05:30  Cprogrammer
# replaced HOME with QMAIL
#
# Revision 1.1  2004-01-02 23:40:56+05:30  Cprogrammer
# Initial revision
#
#
cat > /tmp/smtp.$$
exec 0</tmp/smtp.$$
echo 'SMTP hosts sorted on sbytes

One line per SMTP delivery. Information on each line:
* sbytes is the number of bytes successfully delivered to this host.
* xdelay is the total xdelay incurred by this host.
* embryo is the number of connections that did not mature
* mess is the number of messages sent to this host (success plus failure).
* spam is number of messages tagged as spam
* success is the number of messages successfully received
* failure is number of messages rejected by SMTP

'
(
echo sbytes xdelay embryo mess spam success failure host
LIBEXEC/rsmtp | sort -n -r
) | LIBEXEC/columnt

echo
exec 0</tmp/smtp.$$
echo 'Reasons for failure

One line per reason for delivery failure. Information on each line:
* del is the number of deliveries that ended for this reason.
* xdelay is the total xdelay on those deliveries.
'
(
echo del xdelay reason
LIBEXEC/rsmtpfailures | sort -n -r -k 2,2
) | LIBEXEC/columnt | tr _ ' '

echo
exec 0</tmp/smtp.$$
echo 'SMTP sender domains sorted on sbytes

One line per sender host. Information on each line:
* sbytes is the number of bytes successfully delivered to this host.
* xdelay is the total xdelay incurred by this host.
* mess is the number of messages sent to this host (success plus failure).
* spam is number of messages tagged as spam

'
(
echo sbytes xdelay mess spam host
LIBEXEC/rsmtpsdomains | sort -n -r
) | LIBEXEC/columnt

echo
exec 0</tmp/smtp.$$
echo 'SMTP recipient domains sorted on sbytes

One line per sender host. Information on each line:
* sbytes is the number of bytes successfully delivered to this host.
* xdelay is the total xdelay incurred by this host.
* mess is the number of messages sent to this host (success plus failure).
* spam is number of messages tagged as spam

'
(
echo sbytes xdelay mess spam host
LIBEXEC/rsmtprdomains | sort -n -r
) | LIBEXEC/columnt

echo
exec 0</tmp/smtp.$$
echo 'SMTP senders sorted on sbytes

One line per sender host. Information on each line:
* sbytes is the number of bytes successfully delivered to this host.
* xdelay is the total xdelay incurred by this host.
* mess is the number of messages sent to this host (success plus failure).
* spam is number of messages tagged as spam

'
(
echo sbytes xdelay mess spam sender
LIBEXEC/rsmtpsenders | sort -n -r
) | LIBEXEC/columnt

echo
exec 0</tmp/smtp.$$
echo 'SMTP recipients sorted on sbytes

One line per sender host. Information on each line:
* sbytes is the number of bytes successfully delivered to this host.
* xdelay is the total xdelay incurred by this host.
* mess is the number of messages sent to this host (success plus failure).
* spam is number of messages tagged as spam

'
(
echo sbytes xdelay mess spam recipient
LIBEXEC/rsmtprecipients | sort -n -r
) | LIBEXEC/columnt

/bin/rm -f /tmp/smtp.$$
