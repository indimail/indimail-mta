#
# $Id: qupgrade.sh,v 1.13 2024-02-22 01:05:05+05:30 Cprogrammer Exp mbhangui $
#

do_upgrade()
{
	if [ -f @libexecdir@/qlocal_upgrade ] ; then
		echo "Running upgrade script for $1"
		sh @libexecdir@/qlocal_upgrade $1
	fi
}

do_pre()
{
	if [ -f @sysconfdir@/indimail-mta-release ] ; then
		if [ -f /bin/rpm -o -f /usr/bin/rpm ] ; then
			PKG_VER=$(rpm -qf @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/bin/dpkg ] ; then
			PKG_VER=$(dpkg -S @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/sbin/pacman ] ; then
			PKG_VER=$(pacman -Qo @sysconfdir@/indimail-mta-release|awk '{print $6}')
		else
			echo "Unknown binary package" 1>&2
			return
		fi
		echo "RPM/DEB Version $PKG_VER"
		. @sysconfdir@/indimail-mta-release
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
	if [ -f @sysconfdir@/indimail-mta-release ] ; then
		if [ -f /bin/rpm -o -f /usr/bin/rpm ] ; then
			PKG_VER=$(rpm -qf @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/bin/dpkg ] ; then
			PKG_VER=$(dpkg -S @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/sbin/pacman ] ; then
			PKG_VER=$(pacman -Qo @sysconfdir@/indimail-mta-release|awk '{print $6}')
		else
			echo "Unknown binary package" 1>&2
			return
		fi
		echo "RPM/DEB Version  $PKG_VER"
		. @sysconfdir@/indimail-mta-release
	fi
	case $1 in
		install)
		echo do_post install
		do_upgrade install
		;;
		upgrade)
		echo do_post upgrade
		do_upgrade post
		;;
	esac
}

do_preun()
{
	if [ -f @sysconfdir@/indimail-mta-release ] ; then
		if [ -f /bin/rpm -o -f /usr/bin/rpm ] ; then
			PKG_VER=$(rpm -qf @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/bin/dpkg ] ; then
			PKG_VER=$(dpkg -S @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/sbin/pacman ] ; then
			PKG_VER=$(pacman -Qo @sysconfdir@/indimail-mta-release|awk '{print $6}')
		else
			echo "Unknown binary package" 1>&2
			return
		fi
		echo "RPM/DEB Version  $PKG_VER"
		echo $PKG_VER > /tmp/indimail-mta-pkg.old
		. @sysconfdir@/indimail-mta-release
	else
		/bin/rm -f /tmp/indimail-mta-pkg.old
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
	if [ -f @sysconfdir@/indimail-mta-release ] ; then
		if [ -f /usr/bin/rpm -o -f /bin/rpm ] ; then
			PKG_VER=$(rpm -qf @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/bin/dpkg ] ; then
			PKG_VER=$(dpkg -S @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/sbin/pacman ] ; then
			PKG_VER=$(pacman -Qo @sysconfdir@/indimail-mta-release|awk '{print $6}')
		else
			echo "Unknown binary package" 1>&2
			return
		fi
		echo "RPM/DEB Version  $PKG_VER"
		. @sysconfdir@/indimail-mta-release
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
	if [ -f @sysconfdir@/indimail-mta-release ] ; then
		if [ -f /usr/bin/rpm -o -f /bin/rpm ] ; then
			PKG_VER=$(rpm -qf @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/bin/dpkg ] ; then
			PKG_VER=$(dpkg -S @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/sbin/pacman ] ; then
			PKG_VER=$(pacman -Qo @sysconfdir@/indimail-mta-release|awk '{print $6}')
		else
			echo "Unknown binary package" 1>&2
			return
		fi
		echo "RPM/DEB Version  $PKG_VER"
		. @sysconfdir@/indimail-mta-release
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
	if [ -f /tmp/indimail-mta-pkg.old ] ; then
		OLD_PKG_VER=$(qcat /tmp/indimail-mta-pkg.old)
	fi
	if [ -f @sysconfdir@/indimail-mta-release ] ; then
		if [ -f /usr/bin/rpm -o -f /bin/rpm ] ; then
			PKG_VER=$(rpm -qf @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/bin/dpkg ] ; then
			PKG_VER=$(dpkg -S @sysconfdir@/indimail-mta-release)
		elif [ -f /usr/sbin/pacman ] ; then
			PKG_VER=$(pacman -Qo @sysconfdir@/indimail-mta-release|awk '{print $6}')
		else
			echo "Unknown binary package" 1>&2
			return
		fi
		echo "RPM/DEB Version old [$OLD_PKG_VER] new [$PKG_VER]"
		. @sysconfdir@/indimail-mta-release
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
#
# $Log: qupgrade.sh,v $
# Revision 1.13  2024-02-22 01:05:05+05:30  Cprogrammer
# replace cat with qcat
#
# Revision 1.12  2022-04-06 09:21:45+05:30  Cprogrammer
# use configure.ac variables
#
# Revision 1.11  2021-08-01 09:56:31+05:30  Cprogrammer
# added case for arch linux
#
# Revision 1.10  2020-07-30 11:30:07+05:30  Cprogrammer
# reverted interpreter back to /bin/sh
#
# Revision 1.9  2020-04-27 22:10:27+05:30  Cprogrammer
# added install routine
#
# Revision 1.8  2020-04-11 08:38:30+05:30  Cprogrammer
# use /usr/bin/sh to suppress rpmlint errors
#
# Revision 1.7  2018-03-13 14:13:52+05:30  Cprogrammer
# fixed syntax error
#
# Revision 1.6  2018-02-18 19:07:04+05:30  Cprogrammer
# fix for rpm path on suse
#
# Revision 1.5  2018-01-09 11:57:28+05:30  Cprogrammer
# updated for v2.3 indimail-mta
#
# Revision 1.2  2017-11-06 21:46:24+05:30  Cprogrammer
# fixed upgrade script for posttrans
#
# Revision 1.1  2017-10-22 18:53:47+05:30  Cprogrammer
# Initial revision
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
