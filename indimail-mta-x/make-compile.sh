DEFS=`uname -s | tr "[:lower:]" "[:upper:]"`
if [ " $DEFS" = " DARWIN" ] ; then
	echo "" exec "$CC" -Wno-deprecated-declarations -D$DEFS -c '${1+"$@"}'
else
	extra=$(cat conf-cc-$DEFS)
	set $CC
	cc_arg=$1
	shift
	(
	echo "if [ -n \"\$CC\" ] ; then"
	echo "  " echo \""\$CC" \$CFLAGS $* $extra -D$DEFS -c '${1+"$@"}'\"
	echo "  " exec   "\$CC" \$CFLAGS $* $extra -D$DEFS -c '${1+"$@"}'
	echo "else"
	echo "  " echo \""$cc_arg" \$CFLAGS $* $extra -D$DEFS -c '${1+"$@"}'\"
	echo "  " exec   "$cc_arg" \$CFLAGS $* $extra -D$DEFS -c '${1+"$@"}'
	echo "fi"
	)
fi
