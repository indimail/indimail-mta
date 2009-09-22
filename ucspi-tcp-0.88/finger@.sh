#
# $Log: finger@.sh,v $
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
echo "${2-}" | HOME/bin/tcpclient -RHl0 -- "${1-0}" 79 sh -c '
  HOME/bin/addcr >&7
  exec HOME/bin/delcr <&6
' | cat -v
