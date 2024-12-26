#
# $Log: instcheck.sh,v $
# Revision 1.3  2022-09-18 23:01:10+05:30  Cprogrammer
# use uinstaller only for ucspi-tcp
#
# Revision 1.2  2022-07-01 18:30:28+05:30  Cprogrammer
# common instcheck for daemontools, ucspi-tcp, indimail-mta
#
# Revision 1.1  2021-08-03 21:22:32+05:30  Cprogrammer
# Initial revision
#
#
# $Id: instcheck.sh,v 1.3 2022-09-18 23:01:10+05:30 Cprogrammer Exp mbhangui $
#
if [ $# -eq 0 ] ; then
	name=$(basename $0)
	package=$(echo $name | cut -d. -f2)
	if [ x"$package" = x"$name" ] ; then
		echo "package name must be specified" 1>&2
		exit 100
	fi
	opt=-cfm
elif [ $# -eq 1 ] ; then
	package=$1
	opt=-cfm
elif [ $# -eq 2 ] ; then
	package=$1
	opt=$2
else
	echo "USAGE: $0 package [options]" 1>&2
	exit 100
fi
if [ -f INPUT ] ; then
	dir="./"
else
	dir=@libexecdir@
	conf_dir=@qsysconfdir@
	if [ ! -d $conf_dir/perms.d/$package ] ; then
		echo "$package: missing permission files dir [$conf_dir/perms.d/$package]" 1>&2
		exit 111
	else
		cd $conf_dir/perms.d/$package
		if [ $? -ne 0 ] ; then
			exit 111
		fi
	fi
	if [ ! -f INPUT ] ; then
		echo "$package: missing INPUT definition" 1>&2
		exit 111
	fi
fi
if [ "$package" = "ucspi-tcp" -a -x $dir/uinstaller ] ; then
	installer=$dir/uinstaller #ucspi-tcp
else
	installer=$dir/installer
fi
status=0
set -e
while read line
do
	set $line
	if [ "$1" = "/" ] ; then
		x=""
	else
		x="$1"
	fi
	input=$2
	echo "$installer $opt $DESTDIR"$x" < $input"
	eval $installer $opt $DESTDIR"$x" < $input
	if [ $? -ne 0 ] ; then
		break
	fi
done < INPUT
