#
# $Log: tcpcat.sh,v $
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
exec HOME/bin/tcpclient -RHl0 -- "${1-0}" "${2-17}" sh -c 'exec cat <&6'
