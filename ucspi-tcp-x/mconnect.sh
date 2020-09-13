#
# $Log: mconnect.sh,v $
# Revision 1.2  2020-09-13 09:43:42+05:30  Cprogrammer
# replaced HOME with PREFIX
#
# Revision 1.1  2003-12-31 19:50:30+05:30  Cprogrammer
# Initial revision
#
#
exec PREFIX/bin/tcpclient -RHl0 -- "${1-0}" "${2-25}" PREFIX/bin/mconnect-io
