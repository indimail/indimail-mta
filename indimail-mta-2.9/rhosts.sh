#
# $Log: rhosts.sh,v $
# Revision 1.1  2004-01-02 23:40:30+05:30  Cprogrammer
# Initial revision
#
#
awk '
  /^d/ {
    host = $8
    while (num = index(host,"@"))
      host = substr(host,num + 1)
    xdelay[host] += $5 - $4
    if ($2 == "k") sbytes[host] += $6
    if ($2 == "k") succ[host] += 1
    if ($2 == "d") fail[host] += 1
    if ($2 == "z") temp[host] += 1
  }
  END {
    for (host in xdelay) {
      str = sprintf("%.2f",xdelay[host])
      print 0 + sbytes[host],succ[host] + fail[host],succ[host] + fail[host] + temp[host],str,host
    }
  }
'
