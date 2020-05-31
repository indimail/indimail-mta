#
# $Log: mconnect.sh,v $
# Revision 1.1  2003-12-31 19:50:30+05:30  Cprogrammer
# Initial revision
#
#
exec HOME/bin/tcpclient -RHl0 -- "${1-0}" "${2-25}" HOME/bin/mconnect-io
