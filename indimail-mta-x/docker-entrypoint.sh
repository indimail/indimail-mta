#
# $Id: docker-entrypoint.sh,v 1.5 2020-05-04 11:11:01+05:30 Cprogrammer Exp mbhangui $
#
# $Log: docker-entrypoint.sh,v $
# Revision 1.5  2020-05-04 11:11:01+05:30  Cprogrammer
# create /etc/mtab as link to /proc/self/mounts / /proc/mounts if missing
# start apache if argument to podman/docker entrypoint is webmail
#
# Revision 1.4  2020-04-29 11:31:56+05:30  Cprogrammer
# removed deletion of mysql.3306/down
#
# Revision 1.3  2020-03-21 23:41:17+05:30  Cprogrammer
# use envdir to set env variables in /service/.svscan/variables
#
# Revision 1.2  2020-03-20 15:08:32+05:30  Cprogrammer
# display command being run during docker entrypoint
#
# Revision 1.1  2019-12-08 18:07:36+05:30  Cprogrammer
# Initial revision
#
set -e

case "$1" in
indimail|indimail-mta|svscan|webmail)
	if [ ! -f /etc/mtab ] ; then
		if [ -f /proc/self/mounts ] ; then
			echo "Warning  linking /etc/mtab to /proc/self/mounts" 1>&2
			ln -s /proc/self/mounts /etc/mtab
		elif [ -f /proc/mounts ] ; then
			echo "Warning  linking /etc/mtab to /proc/mounts" 1>&2
			ln -s /proc/mounts /etc/mtab
		else
			echo "Warning /etc/mtab: No such file or directory" 1>&2
		fi
	fi
	case "$1" in
	webmail)
		/usr/sbin/apachectl start
	;;
	esac
	if [ -d /service/.svscan/variables ] ; then
   		echo "docker-entrypoint: [$1] PREFIX/bin/envdir /service/.svscan/variables PREFIX/sbin/svscan /service"
   		exec PREFIX/bin/envdir /service/.svscan/variables PREFIX/sbin/svscan /service
	else
   		echo "docker-entrypoint: [$1] executing PREFIX/sbin/svscan /service"
   		exec PREFIX/sbin/svscan /service
	fi
;;
*)
	echo "docker-entrypoint: executing $@"
	exec "$@"
;;
esac
