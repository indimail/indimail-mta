#
# $Log: xsender.sh,v $
# Revision 1.1  2004-01-02 23:40:47+05:30  Cprogrammer
# Initial revision
#
#

awk '
  /^d/ {
    if ($7 == x) print
  }
  /^m/ {
    if ($8 == x) print
  }
' x="<$1>"
