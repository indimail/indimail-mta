#
# $Log: who@.sh,v $
# Revision 1.2  2020-09-13 09:43:48+05:30  Cprogrammer
# replaced HOME with PREFIX
#
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
PREFIX/bin/tcpclient -RHl0 -- "${1-0}" 11 sh -c 'exec PREFIX/bin/delcr <&6' | cat -v
