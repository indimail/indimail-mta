# $Log: smtp-matchup.sh,v $
# Revision 1.2  2004-01-05 14:08:23+05:30  Cprogrammer
# print all lines on stdout
#
# Revision 1.1  2004-01-04 23:49:23+05:30  Cprogrammer
# Initial revision
#
#
# tcpserver: pid 30906 from 210.210.122.80
# qmail-smtpd: pid 1821 from 127.0.0.1 HELO <localhost.localdomain> MAIL from: <jbks@tca-os.de> RCPT <yahoo> AUTH <local-rcpt> Size: 2281 X-Bogocity: No, spam probability=0.208748, cutoff=5.11e-01
# tcpserver: end 30906 status 0
awk '
  /tcpserver:|qmail-smtpd:/ {
  	if ($2 == "tcpserver:" && $3 == "pid")
	{
    	pid = $4
		line[pid] = $0
		start[pid] = $1
		finish[pid] = -1
	}
  	if ($2 == "qmail-smtpd:")
	{
		pid = $4
		finish[pid] = $1
	} else {
  		if ($2 == "tcpserver:" && $3 == "end")
		{
			pid = $4
			finish[pid] = $1
		}
	}
	print $0
  }
  END {
	for (pid in start) {
		if (finish[pid] == -1)
			print line[pid] | "cat 1>&5"
	}
  }
'
