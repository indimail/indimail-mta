#
# $Log: zsendmail.sh,v $
# Revision 1.1  2004-01-02 23:41:01+05:30  Cprogrammer
# Initial revision
#
#
awk '
  /^d/ {
    if ($2 == "k") stat="stat=Sent"
    else if ($2 == "d") stat="stat=Failed"
    else stat="stat=Deferred"
    str1 = sprintf("%.6f",$5-$3)
    str2 = sprintf("%.6f",$5-$4)
    print $5" qp "$9": to="$8", uid="$10", ddelay="str1", xdelay="str2", "stat" ("$11")"
    next
  }
  /^m/ {
    str1 = sprintf("%.6f",$3-$2)
    print $3" qp "$9": from="$8", uid="$10", size="$4", nrcpts="$5+$6", deferrals="$7", qtime="str1
    next
  }
  { print }
'
