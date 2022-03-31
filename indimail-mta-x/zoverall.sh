#
# $Log: zoverall.sh,v $
# Revision 1.2  2022-03-31 22:39:37+05:30  Cprogrammer
# display time span in seconds if < 86400
#
# Revision 1.1  2004-01-02 23:40:54+05:30  Cprogrammer
# Initial revision
#
#
echo 'Basic statistics

qtime is the time spent by a message in the queue.

ddelay is the latency for a successful delivery to one recipient---the
end of successful delivery, minus the time when the message was queued.

xdelay is the latency for a delivery attempt---the time when the attempt
finished, minus the time when it started.

The queue concurrency is the total xdelay for all deliveries divided by the
time span; this is a good measure of how busy the mailer is.
'

awk '
  BEGIN {
    messages = 0
    recips = 0
    tries = 0
    deliveries = 0
    succ = 0
    fail = 0
    mbytes = 0
    rbytes = 0
  }
  /^m/ {
    ++messages
    mbytes += $4
    rbytes += $4 * $5
    qtime += $3 - $2
    recips += $5 + $6
    tries += $5 + $6 + $7
    if (!seen || ($2 < first)) first = $2
    if (!seen || ($3 > last)) last = $3
    seen = 1
  }
  /^d k/ { ++succ; ddelay += $5 - $3 }
  /^d d/ { ++fail }
  /^d/ {
    ++deliveries
    xdelay += $5 - $4
    if (!seen || ($3 < first)) first = $3
    if (!seen || ($5 > last)) last = $5
    seen = 1
  }
  END {
    print "Completed messages:", messages
    if (messages) {
      print "Recipients for completed messages:", recips
      print "Total delivery attempts for completed messages:", tries
      print "Average delivery attempts per completed message:", tries / messages
      print "Bytes in completed messages:", mbytes
      print "Bytes weighted by success:", rbytes
      print "Average message qtime (s):", qtime / messages
    }
    print ""
    print "Total delivery attempts:", deliveries
    if (deliveries) {
      print "  success:", succ
      print "  failure:", fail
      print "  deferral:", deliveries - succ - fail
      str = sprintf("%.6f",ddelay)
      print "Total ddelay (s):", str
      if (succ) {
        str = sprintf("%.6f",ddelay / succ)
        print "Average ddelay per success (s):", str
      }
      str = sprintf("%.6f",xdelay)
      print "Total xdelay (s):", str
      str = sprintf("%.6f",xdelay / deliveries)
      print "Average xdelay per delivery attempt (s):", str
      if (last > first) {
        if ((last - first) > 86400)
          print "Time span (days):", (last - first) / 86400
        else
          print "Time span (s):", last - first
        print "Queue concurrency:", xdelay / (last - first)
      }
    }
  }
'
