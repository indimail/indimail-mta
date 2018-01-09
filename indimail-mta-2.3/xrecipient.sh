#
# $Log: xrecipient.sh,v $
# Revision 1.1  2004-01-02 23:40:46+05:30  Cprogrammer
# Initial revision
#
#

awk '
  /^d/ {
    if ($8 == x) print
  }
' x="$1"
