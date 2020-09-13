#
# $Log: http@.sh,v $
# Revision 1.2  2020-09-13 09:43:38+05:30  Cprogrammer
# replaced HOME with PREFIX
#
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
echo "GET /${2-} HTTP/1.0
Host: ${1-0}:${3-80}
" | PREFIX/bin/tcpclient -RHl0 -- "${1-0}" "${3-80}" sh -c '
  PREFIX/bin/addcr >&7
  exec PREFIX/bin/delcr <&6
' | awk '/^$/ { body=1; next } { if (body) print }'
