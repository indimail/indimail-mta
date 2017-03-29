#!/bin/sh
# $Log: upgrade.sh,v $
# Revision 2.3  2017-03-29 14:49:56+05:30  Cprogrammer
# fixes for v2.1
#
# Revision 2.2  2017-03-28 19:15:10+05:30  Cprogrammer
# added do_upgrade scriptlet
#
# Revision 2.1  2017-03-28 19:12:18+05:30  Cprogrammer
# generic upgrade script for indimail
#
#
# $Id: upgrade.sh,v 2.3 2017-03-29 14:49:56+05:30 Cprogrammer Exp mbhangui $

do_upgrade()
{
	if [ -f /usr/libexec/indimail/local_upgrade.sh ] ; then
		echo "Running upgrade script for $1"
		sh /usr/libexec/indimail/local_upgrade.sh
		#if [ $? -eq 0 ] ; then
		#	/bin/rm -f /usr/libexec/indimail/local_upgrade.sh
		#fi
	fi
}

do_pre()
{
	RPM_VER=`rpm -qf /etc/indimail/indimail-release`
	echo "RPM Version  $RPM_VER"
	if [ -f /etc/indimail/indimail-release ] ; then
		. /etc/indimail/indimail-release
	fi
	case $1 in
		install)
		echo do_pre install
		;;
		upgrade)
		echo do_pre upgrade
		do_upgrade pre
		;;
	esac
}

do_post()
{
	RPM_VER=`rpm -qf /etc/indimail/indimail-release`
	echo "RPM Version  $RPM_VER"
	if [ -f /etc/indimail/indimail-release ] ; then
		. /etc/indimail/indimail-release
	fi
	case $1 in
		install)
		echo do_post install
		;;
		upgrade)
		echo do_post upgrade
		do_upgrade post
		;;
	esac
}

do_preun()
{
	RPM_VER=`rpm -qf /etc/indimail/indimail-release`
	echo "RPM Version  $RPM_VER"
	if [ -f /etc/indimail/indimail-release ] ; then
		. /etc/indimail/indimail-release
	fi
	case $1 in
		upgrade)
		echo do_preun upgrade
		;;
		uninstall)
		echo do_preun uninstall
		;;
		## debian ###
		remove)
		echo do_preun remove
		;;
		deconfigure)
		echo do_preun deconfigure
		;;
	esac
}

do_postun()
{
	RPM_VER=`rpm -qf /etc/indimail/indimail-release`
	echo "RPM Version  $RPM_VER"
	if [ -f /etc/indimail/indimail-release ] ; then
		. /etc/indimail/indimail-release
	fi
	case $1 in
		upgrade)
		echo do_postun upgrade
		;;
		uninstall|remove)
		echo do_postun uninstall
		;;
		## debian ##
    	purge|failed-upgrade|abort-install|abort-upgrade|disappear)
		echo do_postun purge
		;;
	esac
}

do_posttrans()
{
	RPM_VER=`rpm -qf /etc/indimail/indimail-release`
	echo "RPM Version  $RPM_VER"
	if [ -f /etc/indimail/indimail-release ] ; then
		. /etc/indimail/indimail-release
	fi
	case $1 in
		noargs)
		echo do_posttrans noargs
		do_upgrade posttrans
		;;
	esac
}

version=$3
case $1 in
	pre)
	do_pre $2
	;;
	post)
	do_post $2
	;;
	preun|prerm)
	do_preun $2
	;;
	postun)
	do_postun $2
	;;
	posttrans)
	do_posttrans $2
	;;
esac
