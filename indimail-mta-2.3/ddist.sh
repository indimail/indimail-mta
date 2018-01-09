#
# $Log: ddist.sh,v $
# Revision 1.1  2004-01-02 23:40:12+05:30  Cprogrammer
# Initial revision
#
#
awk '/^d k/ { print $5 - $3 }' \
| sort -n \
| awk '
  { x += 1; cumulative[$1] = x }
  END {
    if (x > 0) {
      for (p = 0;p <= 100;++p) mindel[p] = -1
      for (d in cumulative) {
        p = int((cumulative[d] * 100) / x)
        if (mindel[p] == -1) mindel[p] = d
        else if (d < mindel[p]) mindel[p] = d
        totdel[p] += d
        numdel[p] += 1
      }
      td = 0
      nd = 0
      for (p = 0;p <= 100;++p) {
        td += totdel[p]
        nd += numdel[p]
        if (p >= 10)
	  if (nd > 0)
            if (mindel[p] >= 0) {
	      str1 = sprintf("%.2f",mindel[p])
	      str2 = sprintf("%.2f",td / nd)
	      print str1, str2, p
	    }
      }
    }
  }
'
