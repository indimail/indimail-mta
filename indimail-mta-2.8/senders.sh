#
# $Log: senders.sh,v $
# Revision 1.2  2004-01-03 13:53:56+05:30  Cprogrammer
# break up long print statement
#
# Revision 1.1  2004-01-02 23:40:32+05:30  Cprogrammer
# Initial revision
#
#
awk '
  /^m/ {
    sender = $10"/"$8
    messages[sender] += 1
    succ[sender] += $5
    fail[sender] += $6
    temp[sender] += $7
    mbytes[sender] += $4
    sbytes[sender] += $4 * $5
    rbytes[sender] += $4 * ($5 + $6)
  }
  /^d/ {
    sender = $10"/"$7
    xdelay[sender] += $5 - $4
  }
  END {
    for (sender in messages) {
      str = sprintf("%.6f",xdelay[sender])
      print messages[sender], mbytes[sender],
	  sbytes[sender], rbytes[sender],
	  succ[sender] + fail[sender], 
	  succ[sender] + fail[sender] + temp[sender],
	  str, sender
    }
  }
'
