#!/bin/sh -e

build="`pwd`/build"
source="`pwd`"
bin="${build}/bin"
man="${build}/man"
if [ $# -eq 1 ] ; then
DESTDIR=$1
fi

cat "${source}/dq/TARGETS" |\
while read x
do
  [ -x "${bin}/${x}" ] || \
    ( 
      echo "=== `date` === $x not compiled, compile first!"
      exit 111; 
    ) || exit 111
done || exit 111

echo "=== `date` === installing bin directory"
cat "${source}/dq/TARGETS" |\
while read x
do
  if [ x"${x}" = xdq ]; then
    confbin="`head -1 conf-bin`"
  elif [ x"${x}" = xdqcache ]; then
    confbin="`head -1 conf-bin`"
  else
    confbin="`head -1 conf-sbin`"
  fi
  echo "=== `date` ===   installing build/bin/${x} -> $DESTDIR${confbin}/${x}"
  mkdir -p $DESTDIR$confbin || exit 111
  cp ${bin}/${x} $DESTDIR$confbin || exit 111
  chmod 755 $DESTDIR${confbin}/${x} || exit 111
  #chown 0:0 $DESTDIR${confbin}/${x} || exit 111
done
echo "=== `date` === finishing"

#man
confman="`head -1 conf-man`"
echo "=== `date` === installing man directory"
ls "${man}" | sort |\
while read x
do
  n=`echo "${x}" | cut -d'.' -f2`
  mkdir -p $DESTDIR${confman}/man${n} || exit 111
  cp "${man}/${x}" $DESTDIR${confman}/man${n} || exit 111
  echo "=== `date` ===   installing ${man}/${x} -> $DESTDIR${confman}/man${n}/${x}"
done || exit 111
echo "=== `date` === finishing"

exit 0
