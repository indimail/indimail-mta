#
# $Log: suids.sh,v $
# Revision 1.1  2004-01-02 23:40:39+05:30  Cprogrammer
# Initial revision
#
#
awk '
  /^m/ {
    uid = $10
    messages[uid] += 1
    succ[uid] += $5
    fail[uid] += $6
    temp[uid] += $7
    mbytes[uid] += $4
    sbytes[uid] += $4 * $5
    rbytes[uid] += $4 * ($5 + $6)
  }
  /^d/ {
    uid = $10
    xdelay[uid] += $5 - $4
  }
  END {
    for (uid in messages) {
      str = sprintf("%.6f",xdelay[uid])
      print messages[uid],mbytes[uid],sbytes[uid],rbytes[uid],succ[uid] + fail[uid],succ[uid] + fail[uid] + temp[uid],str,uid
    }
  }
'
