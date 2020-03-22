#
# $Log: maildirqmtp.sh,v $
# Revision 1.2  2017-03-09 16:38:41+05:30  Cprogrammer
# FHS changes
#
# Revision 1.1  2004-05-14 00:43:54+05:30  Cprogrammer
# Initial revision
#
#
exec \
PREFIX/bin/maildirserial -b -t 1209600 -- "$1" "$2" \
PREFIX/bin/tcpclient -RHl0 -- "$3" 209 \
PREFIX/bin/serialqmtp "$2"
