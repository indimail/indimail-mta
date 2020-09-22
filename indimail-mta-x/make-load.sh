DEFS=`uname -s | tr "[:lower:]" "[:upper:]"`
extra=$(cat conf-ld-$DEFS)
echo 'main="$1"; shift'
set $LD
ld_args=$1
shift
echo "if [ -n \"\$CC\" ] ; then"
echo "  " echo \""\$CC" \$LDFLAGS $extra $* '-o "$main" "$main".o ${1+"$@"}'\"
echo "  " exec   "\$CC" \$LDFLAGS $extra $* '-o "$main" "$main".o ${1+"$@"}'
echo "else"
echo "  " echo \""$ld_args" \$LDFLAGS $extra $* '-o "$main" "$main".o ${1+"$@"}'\"
echo "  " exec   "$ld_args" \$LDFLAGS $extra $* '-o "$main" "$main".o ${1+"$@"}'
echo "fi"
