# $Log: rspamrdomain.sh,v $
# Revision 1.5  2016-06-17 20:19:02+05:30  Cprogrammer
# use SYSCONFDIR for bogofilter.cf
#
# Revision 1.4  2009-11-11 08:35:27+05:30  Cprogrammer
# assign X-Bogosity to spamheader_name by default
#
# Revision 1.3  2004-05-19 23:12:08+05:30  Cprogrammer
# fixed qmail-smtpd line getting skipped
#
# Revision 1.2  2004-02-03 13:52:18+05:30  Cprogrammer
# pick up spam header from bogofilter.cf
#
# Revision 1.1  2004-01-08 00:28:30+05:30  Cprogrammer
# Initial revision
#
#
#
#qmail-smtpd:  pid 18359 MAIL from <mbhangui@yahoo.com> RCPT <mbhangui> Size: 19430 X-Bogocity: No, spam probability=0.000000, cutoff=5.11e-01
#qmail-local:  pid 18367 MAIL from <mbhangui@yahoo.com> RCPT <mbhangui@technology.indicorp.com> Size: 19666 X-Bogocity: No, spam probability=0.000000, cutoff=5.11e-01
#qmail-remote: pid 18860 MAIL from <mbhangui@yahoo.com> RCPT <manvendra_bhangui@indicorp.com> Size: 19708 X-Bogocity: No, spam probability=0.000000, cutoff=5.11e-01

spamheader_name=`grep ^spam_header_name SYSCONFDIR/bogofilter.cf | cut -d= -f2`
if [ " $spamheader_name" = " " ] ; then
	spamheader_name="X-Bogosity"
fi
awk -v spamheader_name=$spamheader_name '
  /qmail-local:|qmail-remote:|qmail-smtpd:/ {
	pid = $4
	from[pid] = $7
	rcpt[pid] = $9
	size[pid] = $11
   	if (num = index(rcpt[pid],"@")) {
		num2 = index(rcpt[pid], ">")
		host = substr(rcpt[pid], num + 1, num2 - num - 1)
	} else {
		host = rcpt[pid]
	}
   	if (num = index($0, spamheader_name)) {
		tmp = substr($0, num + length(spamheader_name) + 2)
		if(num = index(tmp, ","))
		{
			tspam = substr(tmp, 0, num - 1)
			if (tspam == "Yes") {
				spam[host] += 1
				spambytes[host] += size[pid]
			} else {
				spambytes[host] += 0
				spam[host] += 0
			}
		} else {
			spambytes[host] += 0
			spam[host] += 0
		}
	} else {
		spambytes[host] += 0
		spam[host] += 0
	}
	tsize[host] += size[pid]
	tmess[host] += 1
  }
  END {
    for (host in tsize) {
		print spambytes[host], spam[host], tsize[host], tmess[host], host
		ttspambyte += spambytes[host]
		ttspam += spam[host]
		ttsize += tsize[host]
		ttmess += tmess[host]
	}
	print ttspambyte, ttspam, ttsize, ttmess, "Total"
  }
'
