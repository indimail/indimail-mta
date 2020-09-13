#
# $Log: tcpcat.sh,v $
# Revision 1.2  2020-09-13 09:43:45+05:30  Cprogrammer
# replaced HOME with PREFIX
#
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
exec PREFIX/bin/tcpclient -RHl0 -- "${1-0}" "${2-17}" sh -c 'exec cat <&6'
