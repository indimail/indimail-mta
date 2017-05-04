#!/bin/sh
if [ ! -d training ] ; then
	mkdir -p training
fi
cd training
if [ $? -ne 0 ] ; then
	exit 1
fi
for i in 20030228_easy_ham_2.tar.bz2 20030228_easy_ham.tar.bz2 \
  20030228_hard_ham.tar.bz2 20030228_spam.tar.bz2 20050311_spam_2.tar.bz2
do
	dir=`echo $i|cut -c 10-|cut -d. -f1`
	echo "Checking $dir"
	if [ ! -d $dir ] ; then
		if [ ! -f $i ] ; then
			echo "Downloading http://spamassassin.apache.org/old/publiccorpus/$i"
			wget http://spamassassin.apache.org/old/publiccorpus/$i
			if [ $? -ne 0 ] ; then
				echo "Failed to download $i" 1>&2
				continue
			fi
		else
			echo "$i already present"
		fi
		echo "bzip2 -d -c < $i | tar xf -"
		bzip2 -d -c < $i | tar xf -
		if [ $? -eq 0 ] ; then
			echo "Extracted $i"
		fi
	else
		echo "$dir already present"
	fi
done

