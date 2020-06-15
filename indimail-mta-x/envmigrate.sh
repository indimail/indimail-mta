# $Log: envmigrate.sh,v $
# Revision 1.5  2020-06-15 19:49:57+05:30  Cprogrammer
# added error checks for all commands used in script
#
# Revision 1.4  2017-03-29 14:50:48+05:30  Cprogrammer
# fix for increase in arguments
#
# Revision 1.3  2017-03-01 22:53:59+05:30  Cprogrammer
# secure permission for env variables
#
# Revision 1.2  2004-02-13 14:49:09+05:30  Cprogrammer
# removed #!/bin/sh as it is auto generated
#
# Revision 1.1  2004-01-08 00:29:19+05:30  Cprogrammer
# Initial revision
#
#
if [ $# -lt 1 ] ; then
	echo "USAGE: $0 config_file [owner group permission]"
	exit 1
fi
if [ ! -f $1 ] ; then
	echo "$1: No such file or directory"
	exit 1
fi
dir=`dirname $1` # /service/qmail-imapd.143/variables
file=`basename $1`
mkdir=$(which mkdir)
rmdir=$(which rmdir)
mv=$(which mv)
chmod=$(which chmod)
chown=$(which chown)
grep=$(which grep)
if [ -z "$mkdir" -o -z "$rmdir" -o -z "$mv" -o -z "$chmod" -o -z "$chown" -o -z "$grep" ] ; then
	echo "envmigrate: missing essentials" 1>&2
	exit 1
fi
$mv $1 $1.tmp
if [ $? -ne 0 ] ; then
	echo "envmigrate: $mv $1 $1.tmp failed" 1>&2
	exit 1
fi
if [ $# -eq 4 ] ; then
	owner=$2
	group=$3
	perm=$4
else
	owner="root"
	group="root"
	perm="500"
fi
if [ $? = 0 ] ; then
	$mkdir $1
	if [ $? -ne 0 ] ; then
		echo "envmigrate: $1: mkdir failed" 1>&2
		$mv $1.tmp $1
		exit 1
	fi
	$chown "$owner":"$group" $1
	if [ $? -ne 0 ] ; then
		echo "envmigrate: chown: $owner:$group $1 failed" 1>&2
		rmdir $1
		$mv $1.tmp $1
		exit 1
	fi
	$chmod $perm $1
	if [ $? = 0 ] ; then
		$mv $1.tmp $1/."$file"
	else
		echo "envmigrate: $chmod $perm $1 failed" 1>&2
		rmdir $1
		$mv $1.tmp $1
		exit 1
	fi
fi
$grep -v "^#" $1/."$file" | while read line
do
	if [ ! " $line" = " " ] ; then
		file=`echo $line | cut -d= -f1`
		value=`echo $line | cut -d= -f2- | tr -d \"`
		if [ " $value" = " " ] ; then
			> $1/$file
		else
			echo $value > $1/$file
		fi
	fi
done
