#
# $Log: deferrals.sh,v $
# Revision 1.1  2004-01-02 23:40:14+05:30  Cprogrammer
# Initial revision
#
#
awk '
  /^d z/ {
    reason = $11
    temp[reason] += 1
    xdelay[reason] += $5 - $4
  }
  END {
    for (reason in temp) {
      str = sprintf("%.2f",xdelay[reason])
      print temp[reason],str,reason
    }
  }
'
