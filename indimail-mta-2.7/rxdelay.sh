#
# $Log: rxdelay.sh,v $
# Revision 1.1  2004-01-02 23:40:31+05:30  Cprogrammer
# Initial revision
#
#

awk '
  {
    str = sprintf("%.2f",$4/$3)
    print str,$3,$5
  }
' | sort -n -r
