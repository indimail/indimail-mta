#
# $Log: choose.sh,v $
# Revision 1.2  2008-07-30 12:11:54+05:30  Cprogrammer
# added option 'R' which doesn't delete
#
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
result="$4"

case "$1" in
  *c*) ./compile $2.c >/dev/null 2>&1 || result="$3" ;;
esac

case "$1" in
  *l*) ./load $2 >/dev/null 2>&1 || result="$3" ;;
esac

case "$1" in
  *r*) ./$2 >/dev/null 2>&1 || result="$3" ;;
esac

case "$1" in
  *R*) ./$2 >/dev/null 2>&1 || result="$3" ;;
esac

case "$1" in
	*c* | *l* | *r*) rm -f $2.o $2 ;;
esac

exec cat "$result"
