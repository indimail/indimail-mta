#
# $Log: xqp.sh,v $
# Revision 1.1  2004-01-02 23:40:44+05:30  Cprogrammer
# Initial revision
#
#

awk '
  /^d/ {
    if ($9 == x) print
  }
  /^m/ {
    if ($9 == x) print
  }
' x="$1"
