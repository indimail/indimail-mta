# $Log: rspamstat.sh,v $
# Revision 1.5  2016-06-17 20:19:23+05:30  Cprogrammer
# use SYSCONFDIR for bogofilter.cf
#
# Revision 1.4  2009-11-11 08:35:16+05:30  Cprogrammer
# assign X-Bogosity to spamheader_name by default
#
# Revision 1.3  2008-05-26 22:31:51+05:30  Cprogrammer
# removed debug statement left by mistake
#
# Revision 1.2  2008-05-26 22:22:52+05:30  Cprogrammer
# qmail-local:, qmail-remote: lines were not getting processed.
#
# Revision 1.1  2004-02-06 22:59:39+05:30  Cprogrammer
# Initial revision
#
#
# @40000000400e481a0adcc8dc qmail-local: pid 2426 MAIL 
# from <fldy6mconq@yahoo.com> RCPT <yahoo@technology.indicorp.com>
# Size: 1869 X-Bogocity: Yes, spam probability=0.998826, cutoff=5.11e-01

spamheader_name=`grep ^spam_header_name SYSCONFDIR/bogofilter.cf | cut -d= -f2`
if [ " $spamheader_name" = " " ] ; then
	spamheader_name="X-Bogosity"
fi
awk -v spamheader_name=$spamheader_name '
	/qmail-remote:|qmail-local:|qmail-smtpd:/ {
		if (num = index($9, "@"))
		{
			num2 = index($9, ">")
			host = substr($9, num + 1, num2 - num - 1)
		} else
			host = $9
		size[host] += $11
		spam_type=$13
		if (spam_type == "Yes,") {
			total_mail[host] += 1
			spam[host] += 1
		} else {
			total_mail[host] += 0
			spam[host] += 0
		}
		if (spam_type == "No,") {
			total_mail[host] += 1
			ham[host] += 1
		} else {
			total_mail[host] += 0
			ham[host] += 0
		}
		if (spam_type == "Unsure,") {
			total_mail[host] += 1
			unsure[host] += 1
		} else {
			total_mail[host] += 0
			unsure[host] += 0
		}
	}
	END {
		for (host in size) {
			total = spam[host] + unsure[host] + ham[host]
			if (total == 0)
				total = 1
			spam_percent = (spam[host] * 100)/total
			ham_percent = (ham[host] * 100)/total
			unsure_percent = (unsure[host] * 100)/total
			printf("%d %d %d (%.2f) %d (%.2f) %d (%.2f) %s\n", size[host], total, spam[host], 
				spam_percent, unsure[host], unsure_percent, ham[host], ham_percent, host)
			tsize += size[host]
			tspam += spam[host]
			tham += ham[host]
			tunsure += unsure[host]
		}
		total = tspam + tunsure + tham
		spam_percent = (tspam * 100)/total
		ham_percent = (tham * 100)/total
		unsure_percent = (tunsure * 100)/total
		printf("%d %d %d (%.2f) %d (%.2f) %d (%.2f) Total\n", tsize, total, tspam, 
			spam_percent, tunsure, unsure_percent, tham, ham_percent)
	}
'
