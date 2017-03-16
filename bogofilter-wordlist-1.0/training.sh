#!/bin/sh
if [ ! -d training ] ; then
	echo "Training folder not found!!" 1>&2
	exit 1
fi
if [ ! -x /usr/bin/bogofilter ] ; then
	echo "bogofilter not found" 1>&2
	exit 1
fi
cd training
for i in *ham*
do
	echo "bogofilter -d . -B -n $i"
	bogofilter -d . -B -n $i
done
for i in *spam*
do
	echo "bogofilter -d . -B -s $i"
	bogofilter -d . -B -s $i
done
echo "Created wordlist.db"
/bin/ls -l wordlist.db
exec mv wordlist.db ..
