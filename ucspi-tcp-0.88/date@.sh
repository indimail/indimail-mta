# $Log: date@.sh,v $
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
HOME/bin/tcpclient -RHl0 -- "${1-0}" 13 sh -c 'exec HOME/bin/delcr <&6' | cat -v
