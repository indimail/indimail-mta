#
# $Log: daneprog.sh,v $
# Revision 1.2  2018-05-13 21:34:03+05:30  Cprogrammer
# fix for non-existing mxdomains
#
# Revision 1.1  2018-04-25 22:57:23+05:30  Cprogrammer
# Initial revision
#
#
# check DNS resource records for the DANE protocol.
#
# $ID: $
#
if [ ! -x /usr/bin/host ] ; then
	echo "/usr/bin/host from bind-utils package not found" 1>&2
	exit 111
elif [ ! -x /usr/bin/danetool ] ; then
	echo "/usr/bin/danetool from gnutls-utils package not found" 1>&2
	exit 111
fi
if [ -x /usr/bin/ipcalc ] ; then
	/usr/bin/ipcalc -c $1 >/dev/null 2>&1
	if [ $? -eq 0 ] ; then
		mx=$1
	else
		mx=`/usr/bin/host -tmx $1 | sort -g -k 6 | head -1 | awk '{print $7}' | sed 's/.$//'`
	fi
else
	if [[ $1 =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
		mx=$1
	else
		mx=`/usr/bin/host -tmx $1 | sort -g -k 6 | head -1 | awk '{print $7}' | sed 's/.$//'`
	fi
fi
cd /tmp
if [ -z "$mx" ] ; then
	exit 1
else
	exec /usr/bin/danetool --quiet --check $mx --proto tcp --starttls-proto=smtp
fi
