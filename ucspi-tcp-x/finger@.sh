#
# $Log: finger@.sh,v $
# Revision 1.2  2020-09-13 09:43:34+05:30  Cprogrammer
# replaced HOME with PREFIX
#
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
echo "${2-}" | PREFIX/bin/tcpclient -RHl0 -- "${1-0}" 79 sh -c '
  PREFIX/bin/addcr >&7
  exec PREFIX/bin/delcr <&6
' | cat -v
