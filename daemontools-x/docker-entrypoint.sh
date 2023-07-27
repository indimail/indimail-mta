#
# $Id: docker-entrypoint.sh,v 1.17 2023-07-26 22:17:03+05:30 Cprogrammer Exp mbhangui $
#

usage()
{
	(
	echo "Usage: svscan|indimail|indimail-mta|webmail"
	echo "              [ -r | --repair"
	echo "              [ -d | --domain   domain]"
	echo "              [ -t | --timezone timezone ]"
	echo "              [command] [args]"
	echo "default command is svscan"
	) 1>&2
	exit 100
}

fix_podman()
{
if [ -f @prefix@/sbin/qmail-queue -a ! -p @indimaildir@/queue/queue1/lock/trigger ] ; then
	echo "Your podman/docker container suffers from named pipe bug. Applying hotfix" 1>&2
	if [ -d @indimaildir@/queue -a ! -f @servicedir@/.svscan/run ] ; then
		cd @indimaildir@/queue
		for i in queue*; do if [ -d $i -a ! -p $i/lock/trigger ] ; then @prefix@/bin/queue-fix -v $i; fi; done
		if [ -d nqueue ] ; then @prefix@/bin/queue-fix -v nqueue; fi
		if [ -d slowq ] ; then @prefix@/bin/queue-fix -rv slowq; fi
		if [ -d qmta ] ; then @prefix@/bin/queue-fix -mv qmta; fi
	fi
fi
}

options=$(getopt -a -n entrypoint -o "rd:t:" -l repair,domain:,timezone: -- "$@")
if [ $? != 0 ]; then
	usage
fi

eval set -- "$options"
while :
do
	case "$1" in
	-d | --domain)
		domain="$2"
		shift 2
	;;
	-t | --timezone)
		timezone="$2"
		shift 2
		if [ -f /usr/share/zoneinfo/$timezone ] ; then
			cd /etc
			/bin/rm -f localtime
			ln -s /usr/share/zoneinfo/$timezone localtime
			echo $timezone > /etc/timezone
		else
			(
			echo "ERROR: unable to open /usr/share/zoneinfo/$timezone"
			echo "WARNING: unable to set timezone $timezone"
			) 1>&2
			exit 1
		fi
	;;
	-r | --repair)
		/bin/sh
	;;
	--) # end of options
		shift
		break
	;;
	*)
		echo "Unexpected option: $1 - this should not happen." 1>&2
		usage
	;;
	esac
done

# fix for podman bug which drops fifos
fix_podman

set -e
cd /
if [ -z "$domain" ] ; then
	domain=$(echo $([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n) | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./')
fi
if [ -f @sysconfdir@/control/defaultdomain ] ; then
	orig=$(cat @sysconfdir@/control/defaultdomain)
fi
if [ ! "$orig" = "$domain" ] ; then
	@prefix@/sbin/svctool --default-domain=$domain --config=recontrol
fi

if [ $# -eq 0 ] ; then
	program=svscan
else
	program=$1
fi
case "$program" in
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
	case "$program" in
		webmail)
		if [ -f @servicedir@/php-fpm/down ] ; then
			echo "enabling @servicedir@/php-fpm"
			/bin/rm -f @servicedir@/php-fpm/down
		fi
		if [ -d @servicedir@/httpd ] ; then
			if [ -f @servicedir@/httpd/down ] ; then
				echo "enabling @servicedir@/httpd"
				/bin/rm -f @servicedir@/httpd/down
			fi
		else
			if [ -x /usr/sbin/apachectl ] ; then
				echo "/usr/sbin/apachectl start"
				/usr/sbin/apachectl start
			elif [ -x /usr/sbin/start_apache2 ] ; then
				/usr/sbin/start_apache2 -k start
			fi
		fi
		;;
	esac
	if [ -d @servicedir@/.svscan/variables ] ; then
   		echo "docker-entrypoint: [$program] @prefix@/bin/envdir @servicedir@/.svscan/variables @prefix@/sbin/svscan @servicedir@"
   		exec @prefix@/bin/envdir @servicedir@/.svscan/variables @prefix@/sbin/svscan @servicedir@
	else
   		echo "docker-entrypoint: [$program] executing @prefix@/sbin/svscan @servicedir@"
   		exec @prefix@/sbin/svscan @servicedir@
	fi
	;;
	*)
	echo "docker-entrypoint: executing $@"
	exec "$@"
	;;
esac
#
# $Log: docker-entrypoint.sh,v $
# Revision 1.17  2023-07-26 22:17:03+05:30  Cprogrammer
# use start_apache2 for openSUSE tumbleweed
#
# Revision 1.16  2023-07-22 23:23:39+05:30  Cprogrammer
# removed now redundant inlookup fix of creating named piples
# fixed indentation
#
# Revision 1.15  2022-11-09 09:59:43+05:30  Cprogrammer
# fixed fix-queue argument for slowq
#
# Revision 1.14  2022-09-08 17:55:47+05:30  Cprogrammer
# set svscan as default when no arguments are passed
#
# Revision 1.13  2021-08-22 23:03:36+05:30  Cprogrammer
# added -r, --repair option to drop to shell
#
# Revision 1.12  2021-08-20 18:13:44+05:30  Cprogrammer
# removed host component from domain name
#
# Revision 1.11  2021-08-18 15:28:19+05:30  Cprogrammer
# removed timedatectl as it doesn't work without systemd
#
# Revision 1.10  2021-08-18 00:10:16+05:30  Cprogrammer
# added hotfix for podman named pipe bug
#
# Revision 1.9  2021-08-11 23:23:15+05:30  Cprogrammer
# use getopt to get options to set domain, timezone
#
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
