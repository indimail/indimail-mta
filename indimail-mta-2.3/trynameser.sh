#!/bin/sh
if [ -x /usr/bin/uname ] ; then
	os=`/usr/bin/uname -s`
elif [ -x /bin/uname ] ; then
	os=`/bin/uname -s`
else
	os="unknown"
fi
if [ " $os" = " Darwin" ] ; then
	if [ -f /usr/include/nameser8_compat.h ] ; then
		echo "#include <nameser8_compat.h>"
	elif [ -f /usr/include/arpa/nameser_compat.h ] ; then
		echo "#include <arpa/nameser_compat.h>"
	fi
fi
/bin/cat dns.h1
