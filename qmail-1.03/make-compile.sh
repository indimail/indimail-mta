DEFS=`uname -s | tr "[:lower:]" "[:upper:]"`
echo exec "$CC" -D$DEFS -c '${1+"$@"}'
