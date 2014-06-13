#
#
# $Id: qmail.spec,v 1.11 2014-06-13 17:58:13+05:30 Cprogrammer Exp mbhangui $
%undefine _missing_build_ids_terminate_build
%define _unpackaged_files_terminate_build 1

%define is_mandrake %(test -e /etc/mandrake-release && echo 1 || echo 0)
%define is_suse %(test -e /etc/SuSE-release && echo 1 || echo 0)
%define is_fedora %(test -e /etc/fedora-release && echo 1 || echo 0)

%define dist redhat
%define disttag rh

%if %{is_mandrake} != 0
%define dist mandrake
%define disttag mdk
%endif
%if %{is_suse} != 0
%define dist suse
%define disttag suse
%endif
%if %{is_fedora} != 0
%define dist fedora
%define disttag rhfc
%endif

%if 0%{?opensuse_bs}
# define to 1 if building on openSUSE build service
%define build_on_obs       1
%define fast_mode          1
%else
%define build_on_obs       0
%define fast_mode          0
%endif

%define _prefix            /var/indimail
%define ucspi_version      0.88
%define qmail_version      1.03
%define libdkim_version    1.4
%define libsrs2_version    1.0.18
%define noperms            1
%define see_base           For a description of IndiMail visit http://www.indimail.org
%define nolibdkim          0
%define nolibsrs2          0
%define noclamav           1
%define nodksignatures     0

%define _verbose           0
%define qcount             5
%define qbase              %{_prefix}/queue
%define logdir             /var/log/indimail
%define servicedir         /service

%if %build_on_obs == 1
%define packager Manvendra Bhangui <manvendra@indimail.org>
%endif

Summary: A Flexible SMTP server
Name: indimail-mta
Version: 1.8.6
%if %build_on_obs == 1
Release: 1.<B_CNT>
%else
Release: 1.1
%endif
%if %build_on_obs == 1
License: GPL-3.0+
%else
License: GPLv3
%endif
%if %{undefined suse_version} && %{undefined sles_version}
Group: System Environment/Base
%else
Group: Productivity/Networking/Email/Servers
%endif
Source1:  http://cr.yp.to/software/qmail-%{qmail_version}.tar.gz
Source2:  http://cr.yp.to/ucspi-tcp/ucspi-tcp-%{ucspi_version}.tar.gz
%if %nolibdkim == 0
Source3:  http://downloads.sourceforge.net/indimail/libdkim-%{libdkim_version}
%endif
%if %nolibsrs2 == 0
Source4:  http://downloads.sourceforge.net/indimail/libsrs2-%{libsrs2_version}
%endif
Source5:  http://downloads.sourceforge.net/indimail/qmail-rpmlintrc

%if %noperms == 0
%if 0%{?suse_version} >= 1120
Source6:%{name}-permissions.easy
Source7:%{name}-permissions.secure
Source8:%{name}-permissions.paranoid
%endif
%endif

Patch1: http://downloads.sourceforge.net/indimail/Patches/qmail-%{qmail_version}.patch.gz
Patch2: http://downloads.sourceforge.net/indimail/Patches/ucspi-tcp-%{ucspi_version}.patch.gz

NoSource: 1
NoSource: 2
%if %nolibdkim == 0
NoSource: 3
%endif
%if %nolibsrs2 == 0
NoSource: 4
%endif

URL: http://www.indimail.org
#AutoReqProv: No
Conflicts: indimail, indimail-mini
BuildRequires: openssl-devel rpm gcc gcc-c++ make bison binutils coreutils grep
BuildRequires: glibc glibc-devel openssl procps readline-devel
BuildRequires: sed ncurses-devel gettext-devel
BuildRequires: python-devel flex findutils
BuildRequires: readline gzip autoconf pkgconfig
BuildRequires: libidn-devel
BuildRequires: groff
%if %{undefined rhel_version}
BuildRequires: gdbm-devel sharutils
%else
%if 0%{?rhel_version} != 600
BuildRequires: gdbm-devel sharutils
%endif
%endif
%if 0%{?suse_version}
BuildRequires: openldap2-devel
%else
BuildRequires: openldap-devel
%endif
%if %{undefined centos_version} && %{undefined rhel_version}
BuildRequires: chrpath
%endif
%if 0%{?suse_version} == 1220 || 0%{?suse_version} == 1210 || 0%{?suse_version} == 1140 || 0%{?suse_version} == 1100 || 0%{?suse_version} == 1030 || 0%{?suse_version} == 1020
BuildRequires: chrpath
%endif
%if %noperms == 0
%if 0%{?suse_version} >= 1120
PreReq: permissions
%endif
%endif
%if 0%{?suse_version}
BuildRequires: db-devel
BuildRequires: -post-build-checks  
#!BuildIgnore: post-build-checks  
%else
%if 0%{?mandriva_version} == 201100
BuildRequires: db5.1-devel
%else
%if 0%{?mandriva_version} == 201010
BuildRequires: db4.7-devel
%else
%if 0%{?fedora_version} > 17
BuildRequires: libdb-devel
%else
BuildRequires: db4-devel
%endif
%endif
%endif
%endif

# rpm -qf /bin/ls
# rpm -qp --queryformat '%{arch}\n' some-file.rpm
# rpm --showrc for all macros
# rpm -qlp some-file.rpm
Requires: /usr/sbin/useradd /usr/sbin/groupadd
Requires: /sbin/chkconfig /usr/sbin/userdel /usr/sbin/groupdel procps /usr/bin/awk
Requires: coreutils grep /bin/sh glibc openssl
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXXX)
#
# IndiMail is choosy and runs on reliable OS only
#
Excludeos: windows 

%description
indimail-mta provides a standalone SMTP server that is part of the IndiMail
messaging platform. It comprises of the following packages

qmail,
serialmail,
qmailanalog,
dotforward,
fastforward,
mess822,
daemontools,
ucspi-tcp,

The package combines qmail with other packages like libdkim, libsrs2

%{see_base}

%prep
echo "---------------- INFORMATION ------------------------"
echo target        %_target
echo target_alias  %_target_alias
echo target_cpu    %_target_cpu
echo target_os     %_target_os
echo target_vendor %_target_vendor
echo Building %{name}-%{version}-%{release} Build %{_build} OS %{_os} Dist %dist disttag %disttag libs %{_lib} %{_libdir}
echo "------------------------------------------------------"

for i in qmail-%{qmail_version} ucspi-tcp-%{ucspi_version} \
libdkim-%{libdkim_version} libsrs2-%{libsrs2_version}
do
	(
	if [ -d $i ] ; then
		%{__rm} -rf $i
	fi
	if [ " $i" = " libdkim-%{libdkim_version}" -a %nolibdkim -ne 0 ] ; then
		continue
	fi
	if [ " $i" = " libsrs2-%{libsrs2_version}" -a %nolibsrs2 -ne 0 ] ; then
		continue
	fi
	if [ -f ../SOURCES/$i.tar.bz2 ] ; then
		%{__bzip2} -d -c ../SOURCES/$i.tar.bz2 |tar xf -
	elif [ -f ../SOURCES/$i.tar.gz ] ; then
		gunzip -c ../SOURCES/$i.tar.gz |tar xf -
	else
		echo "No Source Archive for $i"
		exit 1
	fi
	)
done

%patch1  -p0
%patch2  -p0

%build
ID=$(id -u)
if [ %{_verbose} -eq 0 ] ; then
	DEVICE=/dev/null
else
	DEVICE=/dev/tty
fi
echo "Will send output to $DEVICE"
#### Stupid Mandriva ######################
%if 0%{?mandriva_version} > 2009
%ifarch x86_64
%define _libdir %{_prefix}/lib64
%define _lib lib64
%else
%define _libdir %{_prefix}/lib
%define _lib lib
%endif
%endif
#### LIBDKIM ######################
if [ -d libdkim-%{libdkim_version} ] ; then
cd libdkim-%{libdkim_version}

if [ %{fast_mode} -eq 0 ] ; then
if [ %{build_on_obs} -eq 0 ] ; then
	echo "reconfiguring..."
	autoreconf -fi
else
	echo "reconfiguring..."
%if %{undefined centos_version} && %{undefined rhel_version} && %{undefined sles_version}
%if 0%{?fedora_version} > 10 || 0%{?suse_version} || 0%{?mandriva_version} > 2009
	autoreconf -fi
%endif
%endif
fi
fi

if [ " %dist" = " mandrake" ] ; then
./configure --prefix=%{_prefix} --libdir=%{_libdir} --libexecdir=%{_prefix}/libexec \
	--x-libraries=%{_libdir} --mandir=%{_prefix}/man > $DEVICE
else
%configure --prefix=%{_prefix} --mandir=%{_prefix}/man > $DEVICE
fi
cd ..
fi

#### LIBSRS2 ######################
if [ -d libsrs2-%{libsrs2_version} ] ; then
cd libsrs2-%{libsrs2_version}

if [ %{fast_mode} -eq 0 ] ; then
if [ %{build_on_obs} -eq 0 ] ; then
	echo "reconfiguring..."
	autoreconf -fi
else
	echo "reconfiguring..."
%if %{undefined centos_version} && %{undefined rhel_version} && %{undefined sles_version}
%if 0%{?fedora_version} > 10 || 0%{?suse_version} || 0%{?mandriva_version} > 2009
	autoreconf -fi
%endif
%endif
fi
fi

if [ " %dist" = " mandrake" ] ; then
./configure --prefix=%{_prefix} --libdir=%{_libdir} --libexecdir=%{_prefix}/libexec \
	--x-libraries=%{_libdir} --mandir=%{_prefix}/man > $DEVICE
else
%configure --prefix=%{_prefix} --mandir=%{_prefix}/man > $DEVICE
fi
cd ..
fi

#### qmail ######################
if [ -d qmail-%{qmail_version} ] ; then
	%{__sed} 's{QMAIL{%{_prefix}{' qmail-%{qmail_version}/conf-qmail.in > qmail-%{qmail_version}/conf-qmail
fi
#### ucspi-tcp ######################
if [ -d ucspi-tcp-%{ucspi_version} ] ; then
	%{__sed} 's{HOME{%{_prefix}{' ucspi-tcp-%{ucspi_version}/conf-home.in > ucspi-tcp-%{ucspi_version}/conf-home
fi

%install
[ "$RPM_BUILD_ROOT" != "/" ] && %{__rm} -fr $RPM_BUILD_ROOT
ID=$(id -u)
%{__mkdir_p} $RPM_BUILD_ROOT%{_prefix}
for i in libdkim-%{libdkim_version} libsrs2-%{libsrs2_version} \
qmail-%{qmail_version} ucspi-tcp-%{ucspi_version}
do
	if [ " $i" = " libdkim-%{libdkim_version}" -a %nolibdkim -ne 0 ] ; then
		continue
	fi
	if [ " $i" = " libsrs2-%{libsrs2_version}" -a %nolibsrs2 -ne 0 ] ; then
		continue
	fi
	if [ -d $i ] ; then
		cd $i
		if [ %{_verbose} -eq 0 ] ; then
			%{__make} -s DESTDIR=%{buildroot}
			%{__make} -s DESTDIR=%{buildroot} install-strip
		else
			%{__make} DESTDIR=%{buildroot}
			%{__make} DESTDIR=%{buildroot} install-strip
		fi
		cd ..
	fi
done
if [ %nolibdkim -eq 0 ] ; then
	%{__rm} -f %{buildroot}%{_libdir}/libdkim.la
	if [ -x /usr/bin/chrpath ] ; then
    	/usr/bin/chrpath -d %{buildroot}%{_prefix}/bin/dkim
	fi
fi
if [ %nolibsrs2 -eq 0 ] ; then
	%{__rm} -f %{buildroot}%{_libdir}/libsrs2.la
	if [ -x /usr/bin/chrpath ] ; then
    	/usr/bin/chrpath -d %{buildroot}%{_prefix}/bin/srsfilter
    	/usr/bin/chrpath -d %{buildroot}%{_prefix}/bin/srs
	fi
fi

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%{__mkdir_p} %{buildroot}%{_sysconfdir}/permissions.d/
install -m 644 %{S:16} %{buildroot}%{_sysconfdir}/permissions.d/%{name}-permissions
install -m 644 %{S:17} %{buildroot}%{_sysconfdir}/permissions.d/%{name}-permissions.secure
%endif
%endif

if [ -x /usr/bin/chrpath ] ; then
/usr/bin/chrpath -d %{buildroot}%{_libdir}/*.so
for i in tcpserver ismaildup
do
	if [ -f %{buildroot}%{_prefix}/bin/$i ] ; then
    	/usr/bin/chrpath -d %{buildroot}%{_prefix}/bin/$i
	fi
done
fi
%{__rm} -rf %{buildroot}%{_prefix}/man/man3
%{__rm} -rf %{buildroot}%{_prefix}/include
%{__rm} -rf %{buildroot}%{_prefix}/lib/libdkim.a
%{__rm} -rf %{buildroot}%{_prefix}/lib64/libdkim.a
%{__rm} -rf %{buildroot}%{_prefix}/lib/libsrs2.a
%{__rm} -rf %{buildroot}%{_prefix}/lib64/libsrs2.a
%{__rm} -rf %{buildroot}%{_prefix}/queue


if [ -f ../SOURCES/svctool ] ; then
%{__cp} ../SOURCES/svctool %{buildroot}%{_prefix}/sbin
fi

# Compress the man pages
find %{buildroot}%{_prefix}/man -type f -exec gzip -q {} \;

if [ -x /bin/touch ] ; then
	TOUCH=/bin/touch
elif [ -x /usr/bin/touch ] ; then
	TOUCH=/usr/bin/touch
else
	TOUCH=/bin/touch
fi
# Create these files so that %ghost does not complain
for i in tcp.smtp tcp.smtp.cdb tcp.qmtp tcp.qmtp.cdb tcp.qmqp tcp.qmqp.cdb \
services.log
do
	if [ ! -f %{buildroot}%{_prefix}/etc/$i ] ; then
		$TOUCH %{buildroot}%{_prefix}/etc/$i
	fi
done
(
for i in accesslist authdomains badext badextpatterns badhelo badmailfrom \
badmailpatterns badrcptpatterns badrcptto bindroutes blackholedpatterns \
blackholedsender bodycheck bouncefrom bouncehost bouncemaxbytes bouncemessage \
bouncesubject chkrcptdomains clientca.pem clientcert.pem concurrencyincoming \
concurrencylocal concurrencyremote databytes defaultdelivery defaultdomain defaulthost \
dh1024.pem dh512.pem disclaimer domainbindings doublebouncehost doublebouncemessage \
doublebouncesubject doublebounceto earlytalkerdroptime envnoathost etrnhosts extraqueue \
filterargs from.envrules globalspamredirect helohost holdlocal holdremote \
hostaccess hostid hostip idhost localdomains localiphost locals maxhops \
maxrecipients me moreipme morercpthosts morercpthosts.cdb nodnscheck \
notipme outgoingip percenthack plusdomain qmqpservers qregex \
quarantine queueforward queuelifetime rcptdomains rcpt.envrules rcpthosts \
recipients rejectspam relayclients relaydomains relayhosts relaymailfrom \
rsa512.pem servercert.pem signatures smtpgreeting smtproutes spamignore \
spamignorepatterns spamredirect spfipv6 spfbehavior spfexp spfguess spfrules \
tarpitcount tarpitdelay timeoutconnect timeoutread timeoutremote timeoutsmtpd \
timeoutwrite tlsclientciphers tlsclients tlsserverciphers todointerval virtualdomains \
signaturedomains nosignaturedomains goodrcptto goodrcptpatterns qbase greylist.white
do
	echo "%ghost %config(noreplace,missingok)               %{_prefix}/control/$i"
	echo $i 1>&3
	$TOUCH %{buildroot}%{_prefix}/control/$i
done
) > config_files.list 3>%{buildroot}%{_prefix}/etc/controlfiles

%files -f config_files.list
%defattr(-, root, root,-)
#
# Directories
#
%dir %attr(555,root,qmail)        %{_prefix}
%dir %attr(555,root,qmail)        %{_prefix}/boot
%dir %attr(555,root,qmail)        %{_prefix}/doc
%dir %attr(2555,alias,qmail)      %{_prefix}/alias
%dir %attr(750,qscand,qscand)     %{_prefix}/qscanq
%dir %attr(750,qscand,qscand)     %{_prefix}/qscanq/root
%dir %attr(750,qscand,qscand)     %{_prefix}/qscanq/root/scanq
%dir %attr(755,indimail,indimail) %{_prefix}/control
%dir %attr(755,indimail,indimail) %{_prefix}/control/domainkeys
#%dir %attr(2755,indimail,qmail)   %{_prefix}/autoturn
%dir %attr(555,root,qmail)        %{_prefix}/man
%dir %attr(555,root,qmail)        %{_prefix}/man/man1
%dir %attr(555,root,qmail)        %{_prefix}/man/cat1
%dir %attr(555,root,qmail)        %{_prefix}/man/man5
%dir %attr(555,root,qmail)        %{_prefix}/man/cat5
%dir %attr(555,root,qmail)        %{_prefix}/man/man7
%dir %attr(555,root,qmail)        %{_prefix}/man/cat7
%dir %attr(555,root,qmail)        %{_prefix}/man/man8
%dir %attr(555,root,qmail)        %{_prefix}/man/cat8
%dir %attr(555,root,root)         %{_libdir}

%dir %attr(555,root,qmail)        %{_prefix}/users
%dir %attr(555,root,qmail)        %{_prefix}/plugins
%dir %attr(775,root,indimail)     %{_prefix}/etc
%dir %attr(555,root,indimail)     %{_prefix}/sbin
%dir %attr(555,root,qmail)        %{_prefix}/bin

%attr(444,root,root)                              %{_prefix}/etc/controlfiles

%ghost %config(noreplace,missingok)               %{_prefix}/etc/tcp.smtp
%ghost %config(noreplace,missingok)               %{_prefix}/etc/tcp.qmtp
%ghost %config(noreplace,missingok)               %{_prefix}/etc/tcp.qmqp
#
# These files will get removed during uninstallation
#
%ghost %attr(0644,indimail,indimail)              %{_prefix}/etc/tcp.smtp.cdb
%ghost %attr(0644,indimail,indimail)              %{_prefix}/etc/tcp.qmtp.cdb
%ghost %attr(0644,indimail,indimail)              %{_prefix}/etc/tcp.qmqp.cdb
%ghost %attr(0644,root,root)                      %{_prefix}/etc/services.log

%attr(444,root,root)                              %{_prefix}/etc/qmailprog.list

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%attr(644,root,root)                              %{_sysconfdir}/permissions.d/%{name}-permissions
%attr(644,root,root)                              %{_sysconfdir}/permissions.d/%{name}-permissions.secure
%endif
%endif

#
# Binaries
#
%attr(6511,qscand,qmail)                %{_prefix}/bin/qhpsi
%attr(6511,qmailq,qmail)                %{_prefix}/bin/qmail-queue
%attr(4511,qscand,qscand)               %{_prefix}/bin/qscanq
%attr(2511,root,qscand)                 %{_prefix}/bin/run-cleanq

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%verify (not user group mode) %attr(6511, qscand, qmail)   %{_prefix}/bin/qhpsi
%verify (not user group mode) %attr(2511, root, qscand)    %{_prefix}/bin/run-cleanq
%verify (not user group mode) %attr(6511, qmailq, qmail)   %{_prefix}/bin/qmail-queue
%verify (not user group mode) %attr(4555, qscand, qscand)  %{_prefix}/bin/qscanq
%verify (not user group mode) %attr(2555, alias, qmail)    %{_prefix}/alias
#%verify (not user group mode) %attr(2755, indimail, qmail) %{_prefix}/autoturn
%endif
%endif

%attr(555,root,qmail)                   %{_prefix}/bin/qmail-popbull
%attr(555,root,qmail)                   %{_prefix}/bin/cleanq
%attr(555,root,qmail)                   %{_prefix}/bin/zsuccesses
%attr(555,root,qmail)                   %{_prefix}/bin/ofmipd
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-lagcheck
%attr(555,root,qmail)                   %{_prefix}/bin/mbox2maildir.pl
%attr(555,root,qmail)                   %{_prefix}/bin/deferrals
%attr(555,root,qmail)                   %{_prefix}/bin/maildirwatch
%attr(555,root,qmail)                   %{_prefix}/bin/bouncesaying
%attr(511,root,qmail)                   %{_prefix}/bin/relaytest
%attr(555,root,qmail)                   %{_prefix}/bin/checkaddr
%attr(555,root,qmail)                   %{_prefix}/bin/rsmtprecipients
%attr(555,root,qmail)                   %{_prefix}/bin/autoresponder
%attr(555,root,qmail)                   %{_prefix}/bin/idedit
%attr(555,root,qmail)                   %{_prefix}/bin/svstat
%attr(555,root,qmail)                   %{_prefix}/bin/ddist
%attr(555,root,qmail)                   %{_prefix}/bin/822headerfilter
%attr(555,root,qmail)                   %{_prefix}/bin/smtp-matchup
%attr(500,root,qmail)                   %{_prefix}/bin/qmail-newu
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-pw2u
%attr(555,root,qmail)                   %{_prefix}/bin/qfilelog
%attr(555,root,qmail)                   %{_prefix}/bin/checkdomain
%attr(555,root,qmail)                   %{_prefix}/bin/svok
%attr(555,root,qmail)                   %{_prefix}/bin/maildirqmtp
%attr(555,root,qmail)                   %{_prefix}/bin/qmailctl
%attr(500,root,qmail)                   %{_prefix}/bin/qmail-lspawn
%attr(555,root,qmail)                   %{_prefix}/bin/dnstxt
%attr(555,root,qmail)                   %{_prefix}/bin/printmaillist
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-todo
%attr(555,root,qmail)                   %{_prefix}/bin/tai64nunix
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-tcpto
%attr(555,root,qmail)                   %{_prefix}/bin/newinclude
%attr(555,root,qmail)                   %{_prefix}/bin/recordio
%attr(555,root,qmail)                   %{_prefix}/bin/logselect
%attr(555,root,qmail)                   %{_prefix}/bin/teepipe
%attr(555,root,qmail)                   %{_prefix}/bin/ipmeprint
%attr(555,root,qmail)                   %{_prefix}/bin/uacl
%attr(555,root,qmail)                   %{_prefix}/bin/envdir
%attr(555,root,qmail)                   %{_prefix}/bin/dnsmxip
%attr(555,root,qmail)                   %{_prefix}/bin/zrecipients
%attr(555,root,qmail)                   %{_prefix}/bin/xsender
%attr(555,root,qmail)                   %{_prefix}/bin/rxdelay
%attr(555,root,qmail)                   %{_prefix}/bin/cdbmake
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-remote
%attr(555,root,qmail)                   %{_prefix}/bin/pinq
%attr(555,root,qmail)                   %{_prefix}/bin/mlmatchup
%attr(555,root,qmail)                   %{_prefix}/bin/822print
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-send
%attr(555,root,qmail)                   %{_prefix}/bin/sendmail
%attr(555,root,qmail)                   %{_prefix}/bin/rmail
%attr(555,root,qmail)                   %{_prefix}/bin/ifaddr
%attr(555,root,qmail)                   %{_prefix}/bin/svscan
%attr(555,root,qmail)                   %{_prefix}/bin/matchup
%attr(555,root,qmail)                   %{_prefix}/bin/zspam
%attr(555,root,qmail)                   %{_prefix}/bin/recipients
%attr(555,root,qmail)                   %{_prefix}/bin/update_tmprsadh
%attr(555,root,qmail)                   %{_prefix}/bin/softlimit
%attr(555,root,qmail)                   %{_prefix}/bin/setforward
%attr(555,root,qmail)                   %{_prefix}/bin/822date
%attr(555,root,qmail)                   %{_prefix}/bin/cdbdump
%attr(555,root,qmail)                   %{_prefix}/bin/iftocc
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-qmqpc
%attr(555,root,qmail)                   %{_prefix}/bin/serialqmtp
%attr(555,root,qmail)                   %{_prefix}/bin/rsmtprdomains
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-getpw
%attr(555,root,qmail)                   %{_prefix}/bin/balance_outgoing
%attr(555,root,qmail)                   %{_prefix}/bin/rsmtpfailures
%attr(555,root,qmail)                   %{_prefix}/bin/fghack
%attr(555,root,qmail)                   %{_prefix}/bin/cdbmake-sv
%attr(555,root,qmail)                   %{_prefix}/bin/svscanboot
%attr(555,root,qmail)                   %{_prefix}/bin/successes
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-sigterm
%attr(555,root,qmail)                   %{_prefix}/bin/spipe
%attr(555,root,qmail)                   %{_prefix}/bin/delcr
%attr(555,root,qmail)                   %{_prefix}/bin/svc
%attr(555,root,qmail)                   %{_prefix}/bin/setmaillist
%attr(555,root,qmail)                   %{_prefix}/bin/zrxdelay
%attr(555,root,qmail)                   %{_prefix}/bin/maildirdeliver
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-showctl
%attr(555,root,qmail)                   %{_prefix}/bin/envmigrate
%attr(555,root,qmail)                   %{_prefix}/bin/rspamrdomain
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-sighup
%attr(555,root,qmail)                   %{_prefix}/bin/multilog-matchup
%attr(555,root,qmail)                   %{_prefix}/bin/822headerok
%attr(511,root,qmail)                   %{_prefix}/bin/splogger
%attr(555,root,qmail)                   %{_prefix}/bin/mbox2maildir
%attr(555,root,qmail)                   %{_prefix}/bin/printforward
%attr(555,root,qmail)                   %{_prefix}/bin/zddist
%attr(555,root,qmail)                   %{_prefix}/bin/elq
%attr(500,root,qmail)                   %{_prefix}/bin/qmail-newmrh
%attr(555,root,qmail)                   %{_prefix}/bin/supervise
%attr(555,root,qmail)                   %{_prefix}/bin/preline
%attr(555,root,qmail)                   %{_prefix}/bin/cdbget
%attr(555,root,qmail)                   %{_prefix}/bin/cdbgetm
%attr(555,root,qmail)                   %{_prefix}/bin/822addr
%attr(555,root,qmail)                   %{_prefix}/bin/atrn
%attr(555,root,qmail)                   %{_prefix}/bin/cdbstats
%attr(555,root,qmail)                   %{_prefix}/bin/zsenders
%attr(500,root,qmail)                   %{_prefix}/bin/instcheck
%attr(555,root,qmail)                   %{_prefix}/bin/suids
%attr(555,root,qmail)                   %{_prefix}/bin/maildirserial
%attr(500,root,qmail)                   %{_prefix}/bin/qmail-start
%attr(555,root,qmail)                   %{_prefix}/bin/822fields
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-popup
%attr(555,root,qmail)                   %{_prefix}/bin/ofmipname
%attr(555,root,qmail)                   %{_prefix}/bin/822bodyfilter
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-local
%attr(555,root,qmail)                   %{_prefix}/bin/serialsmtp
%attr(555,root,qmail)                   %{_prefix}/bin/predate
%attr(555,root,qmail)                   %{_prefix}/bin/condredirect
%attr(555,root,qmail)                   %{_prefix}/bin/fastforward
%attr(555,root,qmail)                   %{_prefix}/bin/rspamsdomain
%attr(555,root,qmail)                   %{_prefix}/bin/newaliases
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-rm
%attr(555,root,qmail)                   %{_prefix}/bin/zsmtp
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-pop3d
%attr(555,root,qmail)                   %{_prefix}/bin/zoverall
%attr(555,root,qmail)                   %{_prefix}/bin/xqp
%attr(555,root,qmail)                   %{_prefix}/bin/dot-forward
%attr(555,root,qmail)                   %{_prefix}/bin/datemail
%attr(555,root,qmail)                   %{_prefix}/bin/base64
%attr(555,root,qmail)                   %{_prefix}/bin/swaks
%attr(555,root,qmail)                   %{_prefix}/bin/rhosts
%attr(555,root,qmail)                   %{_prefix}/bin/replier
%attr(500,root,qmail)                   %{_prefix}/bin/qmail-cdb
%attr(555,root,qmail)                   %{_prefix}/bin/cdbtest
%attr(555,root,qmail)                   %{_prefix}/bin/dnsfq
%attr(555,root,qmail)                   %{_prefix}/bin/tai64n
%attr(555,root,qmail)                   %{_prefix}/bin/822header
%attr(555,root,qmail)                   %{_prefix}/bin/qmaildirmake
%attr(555,root,qmail)                   %{_prefix}/bin/maildir2mbox
%attr(555,root,qmail)                   %{_prefix}/bin/columnt
%attr(555,root,qmail)                   %{_prefix}/bin/zrhosts
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-smtpd
%attr(555,root,qmail)                   %{_prefix}/bin/qarf
%attr(555,root,qmail)                   %{_prefix}/bin/qaes
%attr(555,root,qmail)                   %{_prefix}/bin/qnotify
%attr(555,root,qmail)                   %{_prefix}/bin/rrt
%attr(555,root,qmail)                   %{_prefix}/bin/greydaemon
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-greyd
%attr(555,root,qmail)                   %{_prefix}/bin/drate
%attr(555,root,qmail)                   %{_prefix}/bin/cidr
%attr(555,root,qmail)                   %{_prefix}/bin/spawn-filter
%attr(555,root,qmail)                   %{_prefix}/bin/tcp-env
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-lint
%attr(555,root,qmail)                   %{_prefix}/bin/rrforward
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-dk
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-dkim
%attr(555,root,qmail)                   %{_prefix}/bin/dk-filter
%attr(555,root,qmail)                   %{_prefix}/bin/mailsubj
%attr(555,root,qmail)                   %{_prefix}/bin/condtomaildir
%attr(555,root,qmail)                   %{_prefix}/bin/multirotate
%attr(555,root,qmail)                   %{_prefix}/bin/822field
%attr(555,root,qmail)                   %{_prefix}/bin/testzero
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-cat
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-poppass
%attr(555,root,qmail)                   %{_prefix}/bin/failures
%attr(555,root,qmail)                   %{_prefix}/bin/rsmtp
%attr(555,root,qmail)                   %{_prefix}/bin/qail
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-tcpok
%attr(555,root,qmail)                   %{_prefix}/bin/qpq
%attr(555,root,qmail)                   %{_prefix}/bin/replier-config
%attr(555,root,qmail)                   %{_prefix}/bin/etrn
%attr(555,root,qmail)                   %{_prefix}/bin/forward
%attr(555,root,qmail)                   %{_prefix}/bin/new-inject
%attr(555,root,qmail)                   %{_prefix}/bin/filterto
%attr(555,root,qmail)                   %{_prefix}/bin/stripmime.pl
%attr(555,root,qmail)                   %{_prefix}/bin/fixcrio
%attr(555,root,qmail)                   %{_prefix}/bin/qscanq-stdin
%attr(555,root,qmail)                   %{_prefix}/bin/rsmtpsenders
%attr(555,root,qmail)                   %{_prefix}/bin/queue-fix
%attr(555,root,qmail)                   %{_prefix}/bin/822body
%attr(555,root,qmail)                   %{_prefix}/bin/readproctitle
%attr(555,root,qmail)                   %{_prefix}/bin/zdeferrals
%attr(555,root,qmail)                   %{_prefix}/bin/dktest
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-scanner-queue.pl
%attr(555,root,qmail)                   %{_prefix}/bin/rspamstat
%attr(555,root,qmail)                   %{_prefix}/bin/iftoccfrom
%attr(555,root,qmail)                   %{_prefix}/bin/dnsptr
%attr(555,root,qmail)                   %{_prefix}/bin/zsuids
%attr(555,root,qmail)                   %{_prefix}/bin/setlock
%attr(555,root,qmail)                   %{_prefix}/bin/qreceipt
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-qmqpd
%attr(555,root,qmail)                   %{_prefix}/bin/multitail
%attr(555,root,qmail)                   %{_prefix}/bin/except
%attr(555,root,qmail)                   %{_prefix}/bin/dknewkey
%attr(555,root,qmail)                   %{_prefix}/bin/maildirsmtp
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-clean
%attr(511,root,qmail)                   %{_prefix}/bin/qmail-rspawn
%attr(555,root,qmail)                   %{_prefix}/bin/pgrphack
%attr(555,root,qmail)                   %{_prefix}/bin/xrecipient
%attr(555,root,qmail)                   %{_prefix}/bin/argv0
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-qmtpd
%attr(555,root,qmail)                   %{_prefix}/bin/addcr
%attr(555,root,qmail)                   %{_prefix}/bin/dnsip
%attr(555,root,qmail)                   %{_prefix}/bin/qsmhook
%attr(555,root,qmail)                   %{_prefix}/bin/rspamhist
%attr(500,root,qmail)                   %{_prefix}/bin/recipient-cdb
%attr(555,root,qmail)                   %{_prefix}/bin/rsmtpsdomains
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-inject
%attr(555,root,qmail)                   %{_prefix}/bin/serialcmd
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-qread
%attr(555,root,qmail)                   %{_prefix}/bin/zfailures
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-nullqueue
%attr(500,root,qmail)                   %{_prefix}/bin/qmail-daemon
%attr(555,root,qmail)                   %{_prefix}/bin/zsendmail
%attr(555,root,qmail)                   %{_prefix}/bin/multipipe
%attr(555,root,qmail)                   %{_prefix}/bin/822received
%attr(555,root,qmail)                   %{_prefix}/bin/cdbmake-12
%attr(555,root,qmail)                   %{_prefix}/bin/dnscname
%attr(555,root,qmail)                   %{_prefix}/bin/envuidgid
%attr(555,root,qmail)                   %{_prefix}/bin/maildircmd
%attr(555,root,qmail)                   %{_prefix}/bin/multilog
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-multi
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-qfilter
%attr(555,root,qmail)                   %{_prefix}/bin/surblfilter
%attr(555,root,qmail)                   %{_prefix}/bin/surblqueue
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-sigalrm
%attr(555,root,qmail)                   %{_prefix}/bin/rpmattr
%attr(555,root,qmail)                   %{_prefix}/bin/senders
%attr(555,root,qmail)                   %{_prefix}/bin/setuidgid
%attr(555,root,qmail)                   %{_prefix}/bin/spfquery
%attr(555,root,qmail)                   %{_prefix}/bin/srsfilter
%attr(555,root,qmail)                   %{_prefix}/bin/tai64nlocal
%attr(555,root,qmail)                   %{_prefix}/sbin/hostname
%attr(555,root,qmail)                   %{_prefix}/sbin/config-fast
%attr(555,root,qmail)                   %{_prefix}/sbin/qmailconfig
%attr(555,root,qmail)                   %{_prefix}/sbin/batv
%attr(555,root,qmail)                   %{_prefix}/sbin/plugtest
%attr(555,root,qmail)                   %{_prefix}/sbin/sys-checkpwd
%attr(555,root,qmail)                   %{_prefix}/sbin/ldap-checkpwd
%attr(555,root,qmail)                   %{_prefix}/sbin/svctool

# ucspi-tcp
%attr(555,root,root)                    %{_prefix}/bin/mconnect-io
%attr(555,root,root)                    %{_prefix}/bin/rblsmtpd
%attr(555,root,root)                    %{_prefix}/bin/tcprulescheck
%attr(555,root,root)                    %{_prefix}/bin/tcpcat
%attr(555,root,root)                    %{_prefix}/bin/date@
%attr(555,root,root)                    %{_prefix}/bin/who@
%attr(555,root,root)                    %{_prefix}/bin/tcpclient
%attr(555,root,root)                    %{_prefix}/bin/tcpserver
%attr(555,root,root)                    %{_prefix}/bin/mconnect
%attr(555,root,root)                    %{_prefix}/bin/finger@
%attr(555,root,root)                    %{_prefix}/bin/http@
%attr(555,root,root)                    %{_prefix}/bin/tcprules
%if %nolibdkim == 0
%attr(555,root,root)                    %{_prefix}/bin/dkim
%endif

%if %nolibsrs2 == 0
%attr(555,root,root)                    %{_prefix}/bin/srs
%endif
%attr(555,root,qmail)                   %{_prefix}/boot/binm2
%attr(555,root,qmail)                   %{_prefix}/boot/proc+df
%attr(555,root,qmail)                   %{_prefix}/boot/binm1+df
%attr(555,root,qmail)                   %{_prefix}/boot/binm3+df
%attr(555,root,qmail)                   %{_prefix}/boot/home+df
%attr(555,root,qmail)                   %{_prefix}/boot/binm3
%attr(555,root,qmail)                   %{_prefix}/boot/binm1
%attr(555,root,qmail)                   %{_prefix}/boot/proc
%attr(555,root,qmail)                   %{_prefix}/boot/home
%attr(555,root,qmail)                   %{_prefix}/boot/binm2+df
%attr(444,root,qmail)                   %{_prefix}/boot/upstart
%attr(444,root,qmail)                   %{_prefix}/boot/systemd

%attr(555,root,qmail)                   %{_prefix}/plugins/generic.so
%attr(555,root,qmail)                   %{_prefix}/plugins/smtpd-plugin.so
%attr(555,root,qmail)                   %{_prefix}/plugins/smtpd-plugin0.so

%docdir %{_prefix}/doc
%docdir %{_prefix}/man
%attr(444,root,qmail)                   %{_prefix}/man/man[1,4,5,7,8]/*
%attr(444,root,qmail)                   %{_prefix}/man/cat[1,4,5,7,8]/*

%attr(444,root,qmail)                   %{_prefix}/doc/README.indimail
%attr(444,root,qmail)                   %{_prefix}/doc/README.filters
%attr(444,root,qmail)                   %{_prefix}/doc/README.qmail
%attr(444,root,qmail)                   %{_prefix}/doc/README.auth
%attr(444,root,qmail)                   %{_prefix}/doc/README.clamav
%attr(444,root,qmail)                   %{_prefix}/doc/README.logselect
%attr(444,root,qmail)                   %{_prefix}/doc/README.moreipme
%attr(444,root,qmail)                   %{_prefix}/doc/README.newline
%attr(444,root,qmail)                   %{_prefix}/doc/README.qhpsi
%attr(444,root,qmail)                   %{_prefix}/doc/README.qregex
%attr(444,root,qmail)                   %{_prefix}/doc/README.queue-fix
%attr(444,root,qmail)                   %{_prefix}/doc/README.greylist
%attr(444,root,qmail)                   %{_prefix}/doc/README.recipients
%attr(444,root,qmail)                   %{_prefix}/doc/README.remote-auth
%attr(444,root,qmail)                   %{_prefix}/doc/README.spamcontrol
%attr(444,root,qmail)                   %{_prefix}/doc/README.srs
%attr(444,root,qmail)                   %{_prefix}/doc/README.starttls
%attr(444,root,qmail)                   %{_prefix}/doc/README.status
%attr(444,root,qmail)                   %{_prefix}/doc/README.tls
%attr(444,root,qmail)                   %{_prefix}/doc/README.wildmat
%attr(444,root,qmail)                   %{_prefix}/doc/README.surbl
%attr(444,root,qmail)                   %{_prefix}/doc/INSTALL.maildir
%attr(444,root,qmail)                   %{_prefix}/doc/TEST.receive
%attr(444,root,qmail)                   %{_prefix}/doc/INSTALL.alias
%attr(444,root,qmail)                   %{_prefix}/doc/UPGRADE
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.local2alias
%attr(444,root,qmail)                   %{_prefix}/doc/INSTALL.mbox
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.local2local
%attr(444,root,qmail)                   %{_prefix}/doc/INSTALL.ids
%attr(444,root,qmail)                   %{_prefix}/doc/INSTALL.qmail
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.nullclient
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.rem2local
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.relaybad
%attr(444,root,qmail)                   %{_prefix}/doc/SENDMAIL
%attr(444,root,qmail)                   %{_prefix}/doc/FROMISP
%attr(444,root,qmail)                   %{_prefix}/doc/REMOVE.binmail
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.local2ext
%attr(444,root,qmail)                   %{_prefix}/doc/REMOVE.sendmail
%attr(444,root,qmail)                   %{_prefix}/doc/AUTOTURN
%attr(444,root,qmail)                   %{_prefix}/doc/INSTALL.vsm
%attr(444,root,qmail)                   %{_prefix}/doc/QMAILFAQ
%attr(444,root,qmail)                   %{_prefix}/doc/INTERNALS
%attr(444,root,qmail)                   %{_prefix}/doc/INSTALL.ctl
%attr(444,root,qmail)                   %{_prefix}/doc/TEST.deliver
%attr(444,root,qmail)                   %{_prefix}/doc/EXTTODO
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.local2virt
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.relaygood
%attr(444,root,qmail)                   %{_prefix}/doc/PIC.local2rem
%attr(444,root,qmail)                   %{_prefix}/doc/TOISP

# Shared libraries (omit for architectures that don't support them)


%if %nolibdkim == 0
%{_libdir}/libdkim.so
%{_libdir}/libdkim-%{libdkim_version}.so.0
%{_libdir}/libdkim-%{libdkim_version}.so.0.0.0
%endif

%if %nolibsrs2 == 0
%{_libdir}/libsrs2.so
%{_libdir}/libsrs2-%{libsrs2_version}.so.0
%{_libdir}/libsrs2-%{libsrs2_version}.so.0.0.0
%endif

# a copy of /var/qmail/control/me, /var/qmail/control/defaultdomain,
# and /var/qmail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and
# this host's name in /var/qmail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision

%clean
%{__rm} -rf %{buildroot}

#            install   erase   upgrade  reinstall
# % pretrans      0        -         0
# % pre           1        -         2         2
# % post          1        -         2         2
# % preun         -        0         1         -
# % postun        -        0         1         -
# % posttrans     0        -         0
# The scriptlets in %pre and %post are respectively run before and after a package is installed.
# The scriptlets %preun and %postun are run before and after a package is uninstalled.
# The scriptlets %pretrans and %posttrans are run at start and end of a transaction.
# On upgrade, the scripts are run in the following order:
#
#   1. %pretrans of new package
#   2. %pre of new package
#   3. (package install)
#   4. %post of new package
#   5. %preun of old package
#   6. (removal of old package)
#   7. %postun of old package
#   8. %posttrans of new package 

### SCRIPTLET ###############################################################################
%verifyscript
ID=$(id -u)
if [ $ID -ne 0 ] ; then
	echo "You are not root" 1>&2
	exit 1
fi
%{_prefix}/sbin/svctool --check-install --servicedir=%{servicedir} \
	--qbase=%{qbase} --qcount=%{qcount} --qstart=1

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%verify_permissions -e %{_prefix}/bin/qhpsi
%verify_permissions -e %{_prefix}/bin/run-cleanq
%verify_permissions -e %{_prefix}/bin/qmail-queue
%verify_permissions -e %{_prefix}/bin/qscanq
%verify_permissions -e %{_prefix}/alias
#%verify_permissions -e %{_prefix}/autoturn
%endif
%endif

### SCRIPTLET ###############################################################################
%pretrans
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
	echo "You are not root" 1>&2
	exit 1
fi
if [ -f %{_prefix}/sbin/initsvc ] ; then
	%{_prefix}/sbin/initsvc -status
fi

echo "Giving IndiMail exactly 5 seconds to exit nicely"
if test -f %{_sysconfdir}/init/svscan.conf
then
	/sbin/initctl emit qmailstop > /dev/null 2>&1
elif test -f %{_sysconfdir}/event.d/svscan
then
	/sbin/initctl emit qmailstop > /dev/null 2>&1
elif test -f %{_sysconfdir}/systemd/system/multi-user.target.wants/indimail.service
then
	/bin/systemctl stop indimail.service > /dev/null 2>&1
elif test -x %{_prefix}/sbin/initsvc
then
	%{_prefix}/sbin/initsvc -off
fi
sleep 5

### SCRIPTLET ###############################################################################
%pre 
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
	echo "You are not root" 1>&2
	exit 1
fi
# we are doing upgrade
if [ $argv1 -eq 2 ] ; then
	exit 0
fi
#
# Create a users and groups. Do not report any problems if they already
# exists.
#
nscd_up=`ps -ef |grep nscd |grep -v grep|wc -l`
if [ $nscd_up -ge 1 ] ; then
	if [ -x %{_sysconfdir}/init.d/nscd ] ; then
		%{_sysconfdir}/init.d/nscd stop
	fi
fi
echo "Adding IndiMail users/groups"
/usr/bin/getent group indimail  > /dev/null || /usr/sbin/groupadd -r -g 555 indimail || true
if [ $? = 4 ] ; then
	/usr/sbin/groupadd indimail
fi
/usr/bin/getent group nofiles   > /dev/null || /usr/sbin/groupadd nofiles  || true
/usr/bin/getent group qmail     > /dev/null || /usr/sbin/groupadd qmail    || true
/usr/bin/getent group qscand    > /dev/null || /usr/sbin/groupadd qscand   || true

for i in indimail alias qmaild qmaill qmailp qmailq qmailr qmails qscand
do
	%{__rm} -f /var/spool/mail/$i
done

/usr/bin/getent passwd indimail > /dev/null || /usr/sbin/useradd -r -g indimail -u 555 -d %{_prefix} indimail || true
if [ $? = 4 ] ; then
	/usr/sbin/useradd -r -g indimail -d %{_prefix} indimail
fi
/usr/bin/getent passwd alias    > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{_prefix}/alias  -s /sbin/nologin  alias || true
/usr/bin/getent passwd qmaild   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{_prefix}        -s /sbin/nologin qmaild || true
/usr/bin/getent passwd qmaill   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{_prefix}        -s /sbin/nologin qmaill || true
/usr/bin/getent passwd qmailp   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{_prefix}        -s /sbin/nologin qmailp || true
/usr/bin/getent passwd qmailq   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{_prefix}        -s /sbin/nologin qmailq || true
/usr/bin/getent passwd qmailr   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{_prefix}        -s /sbin/nologin qmailr || true
/usr/bin/getent passwd qmails   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{_prefix}        -s /sbin/nologin qmails || true
/usr/bin/getent passwd qscand   > /dev/null || /usr/sbin/useradd -M -g qscand   -d %{_prefix}/qscanq -G qmail,qscand -s /sbin/nologin qscand || true
if [ $nscd_up -ge 1 ] ; then
	if [ -x %{_sysconfdir}/init.d/nscd ] ; then
		%{_sysconfdir}/init.d/nscd start
	fi
fi

### SCRIPTLET ###############################################################################
%post
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
	echo "You are not root" 1>&2
	exit 1
fi
if [ $argv1 -eq 2 ] ; then # upgrade
	if [ -f %{_prefix}/boot/rpm.init ] ; then
		/sbin/ldconfig
		echo "Running Custom Upgrade Script for post"
		/bin/sh %{_prefix}/boot/rpm.init upgrade
	fi
	exit 0
fi
if [ -x /bin/touch ] ; then
	TOUCH=/bin/touch
elif [ -x /usr/bin/touch ] ; then
	TOUCH=/usr/bin/touch
else
	TOUCH=/bin/touch
fi

echo "Creating %{logdir}"
if [ ! -d %{logdir} ] ; then
	%{__mkdir_p} %{logdir}
fi
%{__chown} -R qmaill:nofiles %{logdir}

if [ -d /service ] ; then
	echo "UFO found in /service. Moving it to /service.org"
	%{__mv} -f %{servicedir} %{servicedir}.org
fi

if [ %nodksignatures -eq 0 ] ; then
	if [ -x %{_prefix}/bin/dknewkey ] ; then
		ver_opt="both"
		sign_opt="both"
		%{_prefix}/bin/dknewkey %{_prefix}/control/domainkeys/default 1024
	else
		ver_opt="none"
		sign_opt="none"
	fi
else
	ver_opt="none"
	sign_opt="none"
fi

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%if 0%{?set_permissions:1} > 0
	if [ ! -f /tmp/no_permissions ] ; then
    	%set_permissions %{name}
	fi
%else
	if [ ! -f /tmp/no_permissions ] ; then
    	%run_permissions
	fi
%endif
%endif
%endif

# SMTP
%ifarch x86_64
%define smtp_soft_mem 104857600
%define qmtp_soft_mem 104857600
%define qmqp_soft_mem 104857600
%else
%define smtp_soft_mem 52428800
%define qmtp_soft_mem 52428800
%define qmqp_soft_mem 52428800
%endif
if [ %noclamav -eq 0 ] ; then
	echo "Checking if clamav is installed"
	clamav_os=0
	clamdPrefix=%{_prefix}
	qhpsi="%{_prefix}/bin/clamdscan %s --fdpass --quiet --no-summary"
	mysysconfdir=%{_prefix}/etc
else
	if [ -f /usr/sbin/clamd -a -f /usr/bin/clamdscan ] ; then
		clamav_os=1
		clamdPrefix="/usr"
		qhpsi="/usr/bin/clamdscan %s --fdpass --quiet --no-summary"
		mysysconfdir=%{_sysconfdir}
	else
		clamav_os=0
	fi
fi
for port in 465 25 587
do
	if [ $port -eq 465 ] ; then
		extra_opt="--skipsend --ssl"
		extra_opt="$extra_opt --rbl=-rzen.spamhaus.org --rbl=-rdnsbl-1.uceprotect.net"
	elif [ $port -eq 587 ] ; then
		extra_opt="--skipsend --authsmtp --antispoof"
	else
		extra_opt="--remote-authsmtp=plain --localfilter --remotefilter"
		extra_opt="$extra_opt --deliverylimit-count=-1 --deliverylimit-size=-1"
		extra_opt="$extra_opt --rbl=-rzen.spamhaus.org --rbl=-rdnsbl-1.uceprotect.net"
	fi
	if [ %noclamav -eq 0 -o $clamav_os -eq 1 ] ; then
		%{_prefix}/sbin/svctool --smtp=$port --servicedir=%{servicedir} \
			--qbase=%{qbase} --qcount=%{qcount} --qstart=1 \
			--query-cache --dnscheck --password-cache \
			--cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 --persistdb \
			--starttls --fsync --syncdir --memory=%{smtp_soft_mem} --chkrecipient --chkrelay --masquerade \
			--min-free=52428800 --content-filter \
			--qhpsi="$qhpsi" \
			--dmasquerade \
			--dkverify=both \
			--dksign=both --private_key=%{_prefix}/control/domainkeys/%/%dkimkeyfn \
			$extra_opt
	else
		%{_prefix}/sbin/svctool --smtp=$port --servicedir=%{servicedir} \
			--qbase=%{qbase} --qcount=%{qcount} --qstart=1 \
			--query-cache --dnscheck --password-cache \
			--cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 --persistdb \
			--starttls --fsync --syncdir --memory=%{smtp_soft_mem} --chkrecipient --chkrelay --masquerade \
			--min-free=52428800 --content-filter --virus-filter \
			--dmasquerade \
			--dkverify=both \
			--dksign=both --private_key=%{_prefix}/control/domainkeys/%/%dkimkeyfn \
			$extra_opt
	fi
	echo "1" > %{servicedir}/qmail-smtpd.$port/variables/DISABLE_PLUGIN
done
if [ %noclamav -eq 0 -o $clamav_os -eq 1 ] ; then
	%{_prefix}/sbin/svctool --queueParam=defaultqueue \
		--qbase=%{qbase} --qcount=%{qcount} --qstart=1 \
		--min-free=52428800 --fsync --syncdir \
		--qhpsi="$qhpsi" \
		--dkverify="none" --dksign=$sign_opt \
		--private_key=%{_prefix}/control/domainkeys/%/%dkimkeyfn \
		$extra_opt
else
	%{_prefix}/sbin/svctool --queueParam=defaultqueue \
		--qbase=%{qbase} --qcount=%{qcount} --qstart=1 \
		--min-free=52428800 --fsync --syncdir --virus-filter \
		--dkverify="none" --dksign=$sign_opt \
		--private_key=%{_prefix}/control/domainkeys/%/%dkimkeyfn \
		$extra_opt
fi

%{_prefix}/sbin/svctool --smtp=366 --odmr --servicedir=%{servicedir} \
	--query-cache --password-cache
echo "1" > %{servicedir}/qmail-smtpd.366/variables/DISABLE_PLUGIN
# Greylist daemon
%{_prefix}/sbin/svctool --greylist=1999 --servicedir=%{servicedir} --min-resend-min=2 \
    --resend-win-hr=24 --timeout-days=30 --context-file=greylist.context \
    --hash-size=65536 --save-interval=5 --whitelist=greylist.white --use-greydaemon
# qmail-qmtpd service
%{_prefix}/sbin/svctool --qmtp=209 --servicedir=%{servicedir} --qbase=%{qbase} \
	--qcount=%{qcount} --qstart=1 --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
	--fsync --syncdir --memory=%{qmtp_soft_mem} --min-free=52428800
# qmail-qmqpd service
%{_prefix}/sbin/svctool --qmqp=628 --servicedir=%{servicedir} --qbase=%{qbase} \
	--qcount=%{qcount} --qstart=1 --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
	--fsync --syncdir --memory=%{qmqp_soft_mem} --min-free=52428800
$TOUCH %{servicedir}/qmail-qmqpd.628/down

%{_prefix}/sbin/svctool --config=qmail --postmaster=%{_prefix}/alias/Maildir/ \
	--default-domain=indimail.org

%{__cat} <<EOF > %{_prefix}/control/signatures
# Windows executables seen in active virii
TVqQAAMAA:
TVpQAAIAA
# Additional windows executable signatures not yet seen in virii
TVpAALQAc
TVpyAXkAX
TVrmAU4AA
TVrhARwAk
TVoFAQUAA
TVoAAAQAA
TVoIARMAA
TVouARsAA
TVrQAT8AA
# .ZIPfile signature seen in SoBig.E and mydoom:
#UEsDBBQAA:SoBig.e Virus
#UEsDBAoAAA:mydoom Virus
# .GIF file found in a previous Microsoft virus making the rounds.
R0lGODlhaAA7APcAAP///+rp6puSp6GZrDUjUUc6Zn53mFJMdbGvvVtXh2xre8bF1x8cU4yLprOy:Virus in .gif files
# http://www.gossamer-threads.com/lists/qmail/users/114447
UEsDBAoAAQAAAEBHYzCf4kJRDDAAAAAwAAAKAAAAc3ZtaXhlLmV4ZcuI1rOkjfn48VwCkMYHRTfM
EOF
%{__chown} qscand:qscand %{_prefix}/control/signatures

# Recreate ld.so links and cache
if [ -d %{_sysconfdir}/ld.so.conf.d ] ; then
	(
		echo %{_libdir}
	) > %{_sysconfdir}/ld.so.conf.d/indimail-%{_arch}.conf
fi
/sbin/ldconfig

# rebuild cdb
for i in smtp qmtp qmqp
do
	for j in `/bin/ls %{_prefix}/etc/tcp*.$i 2>/dev/null`
	do
		echo "Creating CDB $j.cdb"
   		%{_prefix}/bin/tcprules $j.cdb $j.tmp < $j && /bin/chmod 664 $j.cdb \
			&& chown indimail:indimail $j.cdb
	done
done

# Misc/Configuration
%{_prefix}/sbin/svctool --svscanlog --servicedir=%{servicedir}
if [ -f %{_prefix}/boot/rpm.init ] ; then
	echo "Running Custom Installation Script for post"
	/bin/sh %{_prefix}/boot/rpm.init post
fi

#
# Install IndiMail to be started on system boot
# The add-boot command installs svscan to be started by init, upstart or launchctl
#
echo "adding indimail startup"
%{_prefix}/sbin/svctool --config=add-boot

if [ -f %{_sysconfdir}/init/svscan.conf -o -f %{_sysconfdir}/event.d/svscan ] ; then
	echo "1. Issue /sbin/initctl emit qmailstart to start services"
	count=1
elif [ -f %{_sysconfdir}/systemd/system/multi-user.target.wants/indimail.service ] ; then
	echo "1. Issue /bin/systemctl start indimail.service to start services"
	count=1
else
	echo "1. Issue %{_prefix}/sbin/initsvc -on"
	echo "2. Issue /sbin/init q to start services"
	count=2
fi
count=`expr $count + 1`
echo "$count. Change your default domain in %{_prefix}/control/defaultdomain"
count=`expr $count + 1`
echo "$count. You can optionally run the following command to verify installation"
echo "   sudo rpm -V indimail"
if [ ! -f %{_prefix}/control/servercert.pem ] ; then
count=`expr $count + 1`
echo "$count. You need to create CERTS for STARTTLS."
echo "   Run the following command to create the Certificate"
echo "   %{_prefix}/sbin/svctool --postmaster=postmaster@indimail.org --config=cert"
fi

### SCRIPTLET ###############################################################################
%preun
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
	echo "You are not root" 1>&2
	exit 1
fi

if test -f %{_sysconfdir}/init/svscan.conf
then
	/sbin/initctl emit qmailstop > /dev/null 2>&1
	echo "Giving IndiMail exactly 5 seconds to exit nicely"
	sleep 5
	if [ $argv1 -ne 1 ] ; then # not an upgrade
		%{__rm} -f %{_sysconfdir}/init/svscan.conf
	fi
elif test -f %{_sysconfdir}/event.d/svscan
then
	/sbin/initctl emit qmailstop > /dev/null 2>&1
	echo "Giving IndiMail exactly 5 seconds to exit nicely"
	sleep 5
	if [ $argv1 -ne 1 ] ; then # not an upgrade
		%{__rm} -f %{_sysconfdir}/event.d/svscan
	fi
elif test -f %{_sysconfdir}/systemd/system/multi-user.target.wants/indimail.service
then
	/bin/systemctl stop indimail.service > /dev/null 2>&1
	echo "Giving IndiMail exactly 5 seconds to exit nicely"
	sleep 5
elif test -x %{_prefix}/sbin/initsvc
then
	%{_prefix}/sbin/initsvc -off
	echo "Giving IndiMail exactly 5 seconds to exit nicely"
	sleep 5
fi
if [ -f %{_prefix}/boot/rpm.init ] ; then
	echo "Running Custom Un-Installation Script for preun"
	/bin/sh %{_prefix}/boot/rpm.init preun "$argv1"
fi

# we are doing upgrade
if [ $argv1 -eq 1 ] ; then
	exit 0
fi

# Remove IndiMail being started on system boot
echo "removing indimail startup"
if [ -f /etc/init.d/indimail ] ; then
	if [ -f /sbin/chkconfig ] ; then
		/sbin/chkconfig --del indimail
	fi
	%{__rm} -f /etc/init.d/indimail
fi
if [ -f /etc/init.d/sendmail ] ; then
	if [ -f /sbin/chkconfig ] ; then
		/sbin/chkconfig --add sendmail
	fi
fi
if [ -x /usr/sbin/alternatives ] ; then
	/usr/sbin/alternatives --remove mta %{_prefix}/bin/sendmail
	/usr/sbin/alternatives --auto mta
else
	for i in /usr/lib/sendmail /usr/sbin/sendmail; do
		if [ -f $i.old -o -L $i.old ]; then
			echo "restoring sendmail"
			%{__rm} -f $i
			%{__mv} $i.old $i
		fi
	done
fi

### SCRIPTLET ###############################################################################
%postun
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
	echo "You are not root" 1>&2
	exit 1
fi
# we are doing upgrade
if [ $argv1 -eq 1 ] ; then
	echo "recreating ld.so cache"
	/sbin/ldconfig
	exit 0
fi

# remove users / groups
nscd_up=`ps -ef |grep nscd |grep -v grep|wc -l`
if [ $nscd_up -ge 1 ] ; then
	if [ -x %{_sysconfdir}/init.d/nscd ] ; then
		%{_sysconfdir}/init.d/nscd stop
	fi
fi
for i in alias qmaild qmaill qmailp qmailq qmailr qmails qscand indimail
do
	echo "Removing user $i"
	/usr/bin/getent passwd $i > /dev/null && /usr/sbin/userdel $i >/dev/null || true
done
for i in nofiles qmail qscand indimail
do
	echo "Removing group $i"
	/usr/bin/getent group $i > /dev/null && /usr/sbin/groupdel $i  >/dev/null || true
done
if [ $nscd_up -ge 1 ] ; then
	if [ -x %{_sysconfdir}/init.d/nscd ] ; then
		%{_sysconfdir}/init.d/nscd start
	fi
fi

if [ ! %{_prefix} = "/usr" ] ; then
	echo "removing binaries, libraries, queues, man pages"
	for i in %{_prefix}/bin %{_prefix}/sbin %{_prefix}/lib %{_prefix}/queue \
		%{_prefix}/man/man1 %{_prefix}/man/cat1 %{_prefix}/man/man5 %{_prefix}/man/cat5 \
		%{_prefix}/man/man7 %{_prefix}/man/cat7 %{_prefix}/man/man8 %{_prefix}/man/cat8
	do
		%{__rm} -rf $i || true
	done
	/bin/rmdir --ignore-fail-on-non-empty %{_prefix}/man 2>/dev/null
	for i in `/bin/ls %{_prefix}/share 2>/dev/null`
	do
		if [ " $i" = " clamd" ] ; then # leave the signatures
			continue;
		fi
		%{__rm} -rf %{_prefix}/share/$i || true
	done
	/bin/rmdir --ignore-fail-on-non-empty %{_prefix}/share 2>/dev/null
fi

echo "removing configuration"
for i in smtp qmtp qmqp
do
	for j in `/bin/ls %{_prefix}/etc/tcp*.$i 2>/dev/null`
	do
			%{_rm} -f $j.cdb
	done
done
/bin/rmdir --ignore-fail-on-non-empty %{_prefix}/etc 2>/dev/null
for i in assign cdb
do
		%{__rm} -f %{_prefix}/users/$i
done
/bin/rmdir --ignore-fail-on-non-empty %{_prefix}/users 2>/dev/null

if [ -f %{_prefix}/etc/controlfiles ] ; then
	for i in `%{__cat} %{_prefix}/etc/controlfiles`
	do
		%{__rm} -f %{_prefix}/control/$i
	done
else
	for i in databytes defaultdelivery defaultdomain localiphost locals \
		me nodnscheck plusdomain queue_base smtpgreeting signatures \
		chkrcptdomains defaulthost envnoathost filterargs greylist.white \
		hostip timeoutremote timeoutsmtpd
	do
		%{__rm} -f %{_prefix}/control/$i
	done
fi
%{__rm} -f %{_prefix}/control/controlfiles
%{__rm} -f %{_prefix}/control/domainkeys/default.pub %{_prefix}/control/domainkeys/default 1024
/bin/rmdir --ignore-fail-on-non-empty %{_prefix}/control/domainkeys 2>/dev/null
/bin/rmdir --ignore-fail-on-non-empty %{_prefix}/control 2>/dev/null

for i in postmaster mailer-daemon root ham spam register-ham register-spam
do
	%{__rm} -f %{_prefix}/alias/.qmail-"$i"
done
/bin/rmdir --ignore-fail-on-non-empty %{_prefix}/alias 2>/dev/null

echo "removing startup services"
for i in qmail-send.25 qmail-smtpd.25 qmail-smtpd.366 \
qmail-spamlog qscanq qmail-smtpd.465 qmail-smtpd.587 qmail-qmtpd.209 \
qmail-qmqpd.628 greylist.1999 .svscan
do
	if [ -d %{servicedir}/$i ] ; then
		%{__rm} -rf %{servicedir}/$i || true
	fi
done
echo "removing logs"
count=`/bin/ls %{servicedir} 2>/dev/null| /usr/bin/wc -l`
if [ $count -eq 0 ] ; then
	%{__rm} -rf %{servicedir} || true
fi
if [ -h %{logdir} ] ; then
	log_dir=`/bin/ls -ld %{logdir} | /usr/bin/awk '{print $10}'`
else
	log_dir=%{logdir}
fi
[ "$log_dir" != "/" ] && %{__rm} -fr $log_dir
echo "recreating ld.so cache"
/sbin/ldconfig
if [ -f %{_prefix}/boot/rpm.init ] ; then
	echo "Running Custom Un-Installation Script for postun"
	/bin/sh %{_prefix}/boot/rpm.init postun
fi

### SCRIPTLET ###############################################################################
%posttrans
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
	echo "You are not root" 1>&2
	exit 1
fi
#echo "Running postrans script for %{version}-%{release} : arg=$1"
if [ -f %{_prefix}/boot/rpm.init ] ; then
	echo "Running Custom Installation Script for posttrans"
	/bin/sh %{_prefix}/boot/rpm.init posttrans
fi
echo ""

# fix changelog for openSUSE buildservice
%changelog
