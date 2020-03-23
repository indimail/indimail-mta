#
# $Log: recipients.sh,v $
# Revision 1.1  2004-01-02 23:40:28+05:30  Cprogrammer
# Initial revision
#
#
awk '
  /^d/ {
    recipient = $8
    xdelay[recipient] += $5 - $4
    if ($2 == "k") sbytes[recipient] += $6
    if ($2 == "k") succ[recipient] += 1
    if ($2 == "d") fail[recipient] += 1
    if ($2 == "z") temp[recipient] += 1
  }
  END {
    for (recipient in xdelay) {
      str = sprintf("%.2f",xdelay[recipient])
      print 0 + sbytes[recipient],succ[recipient] + fail[recipient],succ[recipient] + fail[recipient] + temp[recipient],str,recipient
    }
  }
'
