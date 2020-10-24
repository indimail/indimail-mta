#
# $Id: docker-entrypoint.sh,v 1.8 2020-10-08 22:53:43+05:30 Cprogrammer Exp mbhangui $
#
# $Log: docker-entrypoint.sh,v $
# Revision 1.8  2020-10-08 22:53:43+05:30  Cprogrammer
# use variables from Makefile
#
# Revision 1.7  2020-05-06 15:30:52+05:30  Cprogrammer
# remove down file to start services
#
# Revision 1.6  2020-05-06 11:09:11+05:30  Cprogrammer
# start httpd, php-fpm for webmail entrypoint
#
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
		if [ -f @servicedir@/php-fpm/down ] ; then
			echo "enabling @servicedir@/php-fpm"
			/bin/rm -f @servicedir@/php-fpm/down
		fi
		if [ -f @servicedir@/httpd/down ] ; then
			echo "enabling @servicedir@/httpd"
			/bin/rm -f @servicedir@/httpd/down
		else
			echo "/usr/sbin/apachectl start"
			/usr/sbin/apachectl start
		fi
	;;
	esac
	if [ -d @servicedir@/.svscan/variables ] ; then
   		echo "docker-entrypoint: [$1] @prefix@/bin/envdir @servicedir@/.svscan/variables @prefix@/sbin/svscan @servicedir@"
   		exec @prefix@/bin/envdir @servicedir@/.svscan/variables @prefix@/sbin/svscan @servicedir@
	else
   		echo "docker-entrypoint: [$1] executing @prefix@/sbin/svscan @servicedir@"
   		exec @prefix@/sbin/svscan @servicedir@
	fi
;;
*)
	echo "docker-entrypoint: executing $@"
	exec "$@"
;;
esac
