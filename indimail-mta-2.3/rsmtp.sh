# $Log: rsmtp.sh,v $
# Revision 1.7  2016-06-17 20:18:43+05:30  Cprogrammer
# use SYSCONFDIR for bogofilter.cf
#
# Revision 1.6  2009-12-17 16:03:14+05:30  Cprogrammer
# fix for null host obtained from smtp logs
#
# Revision 1.5  2009-11-11 08:35:04+05:30  Cprogrammer
# assign X-Bogosity to spamheader_name by default
#
# Revision 1.4  2004-02-03 13:52:15+05:30  Cprogrammer
# pick up spam header from bogofilter.cf
#
# Revision 1.3  2004-01-08 00:32:14+05:30  Cprogrammer
# added totals
#
# Revision 1.2  2004-01-05 14:04:27+05:30  Cprogrammer
# added stats for embryonic connections
#
# Revision 1.1  2004-01-04 23:13:02+05:30  Cprogrammer
# Initial revision
#
# Revision 1.1  2004-01-02 23:40:30+05:30  Cprogrammer
# Initial revision
#
#
# tcpserver: pid 30906 from 210.210.122.80
# qmail-smtpd: pid 1821 from 127.0.0.1 HELO <localhost.localdomain> MAIL from: <jbks@tca-os.de> RCPT <yahoo> AUTH <local-rcpt> Size: 2281 X-Bogocity: No, spam probability=0.208748, cutoff=5.11e-01
# tcpserver: end 30906 status 0

spamheader_name=`grep ^spam_header_name SYSCONFDIR/bogofilter.cf | cut -d= -f2`
if [ " $spamheader_name" = " " ] ; then
	spamheader_name="X-Bogosity"
fi
awk -v spamheader_name=$spamheader_name '
  /tcpserver:|qmail-smtpd:/ {
  	if ($2 == "tcpserver:" && $3 == "pid")
	{
    	pid = $4
		line[pid] = $0
		start[pid] = $1
		host_addr[pid] = $5
		immature[$5] += 0
		embryo[pid] = 1
	}
  	if ($2 == "tcpserver:" && $3 == "end") {
		if(embryo[pid] == 1) {
			embryo[pid] = 3
			immature[host_addr[pid]] += 1
		} else {
			immature[host_addr[pid]] += 0
		}
	}
  	if ($2 == "qmail-smtpd:")
	{
		pid = $4
		embryo[pid] = 2
		ip[pid] = $6
		from[pid] = $11
		rcpt[pid] = $13
		host = ip[pid]
		immature[host] += 0
    	if (num = index($0,"Size"))
		{
    		if (num2 = index($0, spamheader_name))
      			size[pid] = substr($0,num + 5, num2 - num - 5)
			else
				size[pid] = substr($0, num + 5)
			tmp = substr($0, num2 + length(spamheader_name) + 2)
			if(num2 = index(tmp, ","))
			{
				tspam = substr(tmp, 0, num2 - 1)
				if (tspam == "Yes") {
					spam[host] += 1
				} else {
					spam[host] += 0
				}
			} else {
				spam[host] += 0
			}
		} else {
			spam[host] += 0
			size[pid] = 0
		}
		tsize[host] += size[pid]
		xdelay[host] += ($1 - start[pid])
		start[pid] = $1
		tmess[host] += 1
		if ($7 == "HELO" || $7 == "ETRN" || $7 == "ATRN") {
			success[host] += 1
			failure[host] += 0;
		} else {
			success[host] += 0
			failure[host] += 1;
		}
	}
  }
  END {
    for (host in tsize) {
		str = sprintf("%.2f",xdelay[host])
		print tsize[host], str, immature[host], tmess[host], spam[host], success[host], failure[host], host
		ttsize += tsize[host]
		txdelay += xdelay[host]
		timmature += immature[host]
		ttmess += tmess[host]
		ttspam += spam[host]
		tsuccess += success[host]
		tfailure += failure[host]
	}
	str = sprintf("%.2f", txdelay)
	print ttsize, str, timmature, ttmess, ttspam, tsuccess, tfailure, "Total"
  }
'
