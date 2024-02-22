# $Log: zspam.sh,v $
# Revision 1.10  2024-02-22 01:05:27+05:30  Cprogrammer
# replace cat with qcat
#
# Revision 1.9  2017-04-13 13:09:02+05:30  Cprogrammer
# fixed columnt path
#
# Revision 1.8  2017-04-12 14:53:52+05:30  Cprogrammer
# report programs moved to libexecdir
#
# Revision 1.7  2017-03-09 16:40:04+05:30  Cprogrammer
# FHS changes
#
# Revision 1.6  2016-06-17 20:21:33+05:30  Cprogrammer
# qmailanalog scripts moved to libexec dir
#
# Revision 1.5  2011-11-13 15:40:41+05:30  Cprogrammer
# fix for ubuntu (sort is /usr/bin/sort)
#
# Revision 1.4  2008-05-22 19:49:41+05:30  Cprogrammer
# change for posix compliant sort
#
# Revision 1.3  2004-02-13 14:51:37+05:30  Cprogrammer
# added rspamstat, rspamhist
#
# Revision 1.2  2004-02-03 13:53:02+05:30  Cprogrammer
# changed order of sender, recipient reports
#
# Revision 1.1  2004-01-08 00:28:23+05:30  Cprogrammer
# Initial revision
#
#
#
qcat > /tmp/smtp.$$
exec 0</tmp/smtp.$$
echo 'Recipient domains SPAM filter messages sorted on spambytes and spam

One line per delivery through SPAM Filter. Information on each line:
* spambytes is the number of bytes in SPAM messages
* spam is number of messages tagged as spam
* sbytes is the number of bytes successfully filtered by SPAM filter.
* mess is the number of messages sent to this host (success plus failure).
* host is the RECIPIENT domain

'
(
echo spambytes spam sbytes mess host
LIBEXEC/rspamrdomain | sort -n -r -k 1,1 -k 2,2 -k 4,4 -k 3,3
) | LIBEXEC/columnt
echo
exec 0</tmp/smtp.$$
echo 'Sender domains SPAM filter messages sorted on spambytes and spam

One line per delivery through SPAM Filter. Information on each line:
* spambytes is the number of bytes in SPAM messages
* spam is number of messages tagged as spam
* sbytes is the number of bytes successfully filtered by SPAM filter.
* mess is the number of messages sent to this host (success plus failure).
* host is the SENDER domain

'
(
echo spambytes spam sbytes mess host
LIBEXEC/rspamsdomain | sort -n -r -k 1,1 -k 2,2 -k 4,4 -k 3,3
) | LIBEXEC/columnt

echo
exec 0</tmp/smtp.$$
echo 'Breakup Of Spam Classification

One line per host/domain. Information on each line:
* sbytes is the number of bytes of mail
* mess is the number of messages sent to this host (success plus failure).
* spam is number of messages tagged as spam
* spam_pct is the percentage of mail tagged as spam
* unsure is the number of messages tagged as unsure
* unsure_pct is the percentage of mail tagged as unsure
* ham is number of messages tagged as ham
* ham_pct is the percentage of mail tagged as ham

'
(
echo sbytes mess spam spam_pct unsure unsure_pct ham ham_pct
LIBEXEC/rspamstat | sort -n -r -k 3,3 -k 2,2 -k 1,1
) | LIBEXEC/columnt

echo
exec 0</tmp/smtp.$$
echo 'Histogram of Spam Distribution

* Spamicity is the spamicity value of the mails
* Hist is Pictorial representation of no of mails in the given range
* mess is the number of mails in the given range

'
(
echo spamicity hist mess
LIBEXEC/rspamhist
) | LIBEXEC/columnt

/bin/rm -f /tmp/smtp.$$
