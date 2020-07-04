DEFS=`uname -s | tr "[:lower:]" "[:upper:]"`
CC="`head -1 conf-cc`"
set $CC
cc_arg=$1
shift
echo "if [ -n \"\$CC\" ] ; then"
echo "  " echo \""\$CC" \$CFLAGS $* -D$DEFS -c '${1+"$@"}'\"
echo "  "   exec "\$CC" \$CFLAGS $* -D$DEFS -c '${1+"$@"}'
echo "else"
echo "  " echo \""$cc_arg" \$CFLAGS $* -D$DEFS -c '${1+"$@"}'\"
echo "  "   exec "$cc_arg" \$CFLAGS $* -D$DEFS -c '${1+"$@"}'
echo "fi"
