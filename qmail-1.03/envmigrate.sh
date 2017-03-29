# $Log: envmigrate.sh,v $
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
dir=`dirname $1`
file=`basename $1`
mkdir=$(which mkdir)
mv=$(which mv)
chmod=$(which chmod)
chown=$(which chown)
grep=$(which grep)
$mv $1 $1.tmp
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
	$chown "$owner":"$group" $1
	$chmod $perm $1
	if [ $? = 0 ] ; then
		$mv $1.tmp $1/."$file"
	else
		$mv $1.tmp $1
		echo "Error Creating Directory $1"
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
