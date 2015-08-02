#! /bin/sh
#Test bogoutil and bogoupgrade

#Test 1 -- combined count/data text file
#Test 2 -- separate count, BDB data file.
#Test 3 -- Same as 2 except that BDB file has a .count token.
#Test 4 -- Same as 2 except that BDB file has a  .MSG_COUNT token.
#          Technically already upgraded.

set -e

yday="-y 0"

( setopt SH_WORD_SPLIT 2> /dev/null ) && setopt SH_WORD_SPLIT

while : ; do
    tdir=${BF_TESTDIR=.}/checks.$$.`date +"%Y%m%dT%H%M%S"`
    mkdir $tdir && break
    sleep 1
done

trap "rm -f -r $tdir" 0 1 2 3 15

inputfile="input-${num}"
for ext in txt count; do
  t="${inputfile}.$ext" 
  if [ -f $srcdir/$t ]; then
     inputfile=$t
   break
  fi
done

if [ ! -f "$srcdir/$inputfile" ]; then
  exit 2;
fi

cp -f $srcdir/$inputfile $tdir/

datafile="$srcdir/input-${num}-data.txt"
inputdb="input-${num}.db"

if [ -f "$datafile" ]; then
	rm -f $tdir/$inputdb
	../../bogoutil $yday -l $tdir/$inputdb <  $datafile
fi

outputdb="output-${num}.db"
rm -f $tdir/$outputdb

perl ${srcdir}/../../bogoupgrade $yday -b ../../bogoutil \
    -i $tdir/$inputfile -o $tdir/$outputdb \
&& ../../bogoutil $yday -d $tdir/$outputdb \
|LC_COLLATE=C sort\
|diff - ${srcdir}/output-${num}.txt
