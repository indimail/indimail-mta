#!/bin/sh
f1=INDIDIR/include/indimail.h
f2=INDIDIR/include/config.h
if [ -f $f1 ] ; then
	if [ $f1 -nt $0 ] ; then
		touch $0
	fi
fi
if [ -f $f2 ] ; then
	if [ $f2 -nt $0 ] ; then
		touch $0
	fi
fi
