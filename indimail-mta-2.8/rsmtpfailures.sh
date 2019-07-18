# $Log: rsmtpfailures.sh,v $
# Revision 1.1  2004-01-04 23:12:54+05:30  Cprogrammer
# Initial revision
#
# Revision 1.1  2004-01-02 23:40:30+05:30  Cprogrammer
# Initial revision
#
#
# tcpserver: pid 30906 from 210.210.122.80
# qmail-smtpd: pid 1821 from 127.0.0.1 Invalid RELAY client: MAIL from: <mbhangui@indi.com>, RCPT <mbhangui@yahoo.com>
# tcpserver: end 30906 status 0
# qmail-smtpd: pid 1821 from 127.0.0.1 HELO <localhost.localdomain> MAIL from: <jbks@tca-os.de> RCPT <yahoo> AUTH <local-rcpt> Size: 2281 X-Bogocity: No, spam probability=0.208748, cutoff=5.11e-01
# qmail-smtpd: pid 1821 from 127.0.0.1 ETRN
# qmail-smtpd: pid 1821 from 127.0.0.1 ATRN
# qmail-smtpd: pid 1821 from 127.0.0.1 Invalid RELAY client: MAIL from <a@indi.com> RCPT <b@indi.com> AUTH <a@indi.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 SMTP Access denied to <a@indi.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 Non-existing DNS_MX: MAIL mbhangui@yahoo.com
# qmail-smtpd: pid 1821 from 127.0.0.1 Non-existing DNS_MX: HELO mbhangui@yahoo.com
# qmail-smtpd: pid 1821 from 127.0.0.1 Invalid HELO greeting: HELO <helo > FQDN <a@t.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 Invalid SENDER address: MAIL from <a@indi.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 Invalid SENDER address: MAIL from <a@indi.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 Invalid RECIPIENT address: MAIL from <a@indi.com> RCPT <b@indi.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 Invalid RECIPIENT address: MAIL from <a@indi.com> RCPT <a@INDI.COM> state <inactive|bounce|not present>
# qmail-smtpd: pid 1821 from 127.0.0.1 Routing SENDER address: MAIL from <a@indi.com> ...
# qmail-smtpd: pid 1821 from 127.0.0.1 Blackholed SENDER address: MAIL <a@indi.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 Blackholed SENDER address: MAIL <a@indi.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 Blackholed SENDER address: MAIL <a@indi.com> Reason ...
# qmail-smtpd: pid 1821 from 127.0.0.1 Too many RECIPIENTS: MAIL from <a@indi.com> Last RCPT <a@indi.com>
# qmail-smtpd: pid 1821 from 127.0.0.1 qqt failure
# qmail-smtpd: pid 1821 from 127.0.0.1 ... permanent failure
# qmail-smtpd: pid 1821 from 127.0.0.1 ... temporary failure
# qmail-smtpd: pid 1821 from 127.0.0.1 going down on SIGTERM
# qmail-smtpd: pid 1821 from 127.0.0.1 Database error

awk '
  /tcpserver:|qmail-smtpd:/ {
  	if ($2 == "tcpserver:" && $3 == "pid") {
    	pid = $4
		line[pid] = $0
		start[pid] = $1
		finish[pid] = -1
	}
  	if ($2 == "tcpserver:" && $3 == "end")
		finish[pid] = $4
  	if ($2 == "qmail-smtpd:") {
		pid = $4
		if ($7 != "HELO" && $7 != "ETRN" && $7 != "ATRN") {
			tmp = $7
			if (num = index($0, tmp)) {
				tmp = substr($0, num, length($0))
				if (num = index(tmp, ":")) {
					reason = substr(tmp, 0, num - 1)
					gsub(/ /, "_", reason)
					fail[reason] += 1
					if (start[pid])
    					xdelay[reason] += ($1 - start[pid])
					else
						xdelay[reason] = 0
				}
			} 
		}
		start[pid] = $1
	}
  }
  END {
    for (reason in fail) {
      str = sprintf("%.2f",xdelay[reason])
      print fail[reason], str, reason
	}
  }
'
