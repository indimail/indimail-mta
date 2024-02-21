set -e
subject="$1"
shift
umask 077
out=$(mktemp -t mailsubjXXXXXXX)
(
  echo Subject: "$subject"
  echo To: ${1+"$@"}
  echo ''
  qmail-cat
) > $out
exec 0<$out
/bin/rm -f $out
PREFIX/bin/qmail-inject
