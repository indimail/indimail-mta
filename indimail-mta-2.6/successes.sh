#
# $Log: successes.sh,v $
# Revision 1.1  2004-01-02 23:40:37+05:30  Cprogrammer
# Initial revision
#
#
awk '
  /^d k/ {
    reason = $11
    succ[reason] += 1
    xdelay[reason] += $5 - $4
  }
  END {
    for (reason in succ) {
      str = sprintf("%.2f",xdelay[reason])
      print succ[reason],str,reason
    }
  }
'
