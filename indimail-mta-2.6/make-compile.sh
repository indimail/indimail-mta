DEFS=`uname -s | tr "[:lower:]" "[:upper:]"`
if [ " $DEFS" = " DARWIN" ] ; then
	echo exec "$CC" -Wno-deprecated-declarations -D$DEFS -c '${1+"$@"}'
else
	echo exec "$CC" -D$DEFS -c '${1+"$@"}'
fi
