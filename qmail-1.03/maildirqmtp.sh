#
# $Log: maildirqmtp.sh,v $
# Revision 1.1  2004-05-14 00:43:54+05:30  Cprogrammer
# Initial revision
#
#
exec \
HOME/bin/maildirserial -b -t 1209600 -- "$1" "$2" \
HOME/bin/tcpclient -RHl0 -- "$3" 209 \
HOME/bin/serialqmtp "$2"
