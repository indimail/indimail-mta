#!/bin/sh
# $Log: iupgrade.sh,v $
# Revision 2.6  2017-11-06 21:45:55+05:30  Cprogrammer
# fixed upgrade script for posttrans
#
# Revision 2.5  2017-10-22 18:53:22+05:30  Cprogrammer
# refactored upgrade.sh
#
# Revision 2.4  2017-03-29 22:37:06+05:30  Cprogrammer
# added case for pretrans
#
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
# $Id: iupgrade.sh,v 2.6 2017-11-06 21:45:55+05:30 Cprogrammer Exp mbhangui $

do_upgrade()
{
	if [ -f /usr/libexec/indimail/local_upgrade.sh ] ; then
		echo "Running upgrade script for $1"
		sh /usr/libexec/indimail/local_upgrade.sh $1
		#if [ $? -eq 0 ] ; then
		#	/bin/rm -f /usr/libexec/indimail/local_upgrade.sh
		#fi
	fi
}

do_pre()
{
	if [ -f /usr/bin/rpm ] ; then
	PKG_VER=`rpm -qf /etc/indimail/indimail-release`
	else
	PKG_VER=`dpkg -S /etc/indimail/indimail-release`
	fi
	echo "RPM/DEB Version  $PKG_VER"
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
	if [ -f /usr/bin/rpm ] ; then
	PKG_VER=`rpm -qf /etc/indimail/indimail-release`
	else
	PKG_VER=`dpkg -S /etc/indimail/indimail-release`
	fi
	echo "RPM/DEB Version  $PKG_VER"
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
	if [ -f /usr/bin/rpm ] ; then
	PKG_VER=`rpm -qf /etc/indimail/indimail-release`
	else
	PKG_VER=`dpkg -S /etc/indimail/indimail-release`
	fi
	echo "RPM/DEB Version  $PKG_VER"
	echo $PKG_VER > /tmp/indimail-pkg.old
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
	if [ -f /usr/bin/rpm ] ; then
	PKG_VER=`rpm -qf /etc/indimail/indimail-release`
	else
	PKG_VER=`dpkg -S /etc/indimail/indimail-release`
	fi
	echo "RPM/DEB Version  $PKG_VER"
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

do_prettrans()
{
	if [ -f /usr/bin/rpm ] ; then
	PKG_VER=`rpm -qf /etc/indimail/indimail-release`
	else
	PKG_VER=`dpkg -S /etc/indimail/indimail-release`
	fi
	echo "RPM/DEB Version  $PKG_VER"
	if [ -f /etc/indimail/indimail-release ] ; then
		. /etc/indimail/indimail-release
	fi
	case $1 in
		noargs)
		echo do_prettrans noargs
		do_upgrade prettrans
		;;
	esac
}

do_posttrans()
{
	if [ -f /usr/bin/rpm ] ; then
	PKG_VER=`rpm -qf /etc/indimail/indimail-release`
	else
	PKG_VER=`dpkg -S /etc/indimail/indimail-release`
	fi
	if [ -f /tmp/indimail-pkg.old ] ; then
		OLD_PKG_VER=`cat /tmp/indimail-pkg.old`
	fi
	echo "RPM/DEB Version old $OLD_PKG_VER new $PKG_VER"
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
	prettrans)
	do_prettrans $2
	;;
	posttrans)
	do_posttrans $2
	;;
esac
