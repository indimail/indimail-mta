echo 'main="$1"; shift'
set $LD
shift
echo "if [ -n \"\$CC\" ] ; then"
echo "  " echo \""\$CC" $* '-o "$main" "$main".o ${1+"$@"}'\"
echo "  " exec "\$CC" $* '-o "$main" "$main".o ${1+"$@"}'
echo "else"
echo "  " echo \""$LD" '-o "$main" "$main".o ${1+"$@"}'\"
echo "  " exec "$LD" '-o "$main" "$main".o ${1+"$@"}'
echo "fi"
