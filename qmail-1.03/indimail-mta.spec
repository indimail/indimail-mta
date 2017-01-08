#
#
# $Id: indimail-mta.spec,v 1.71 2017-01-08 19:03:25+05:30 Cprogrammer Exp mbhangui $
%undefine _missing_build_ids_terminate_build
%global _unpackaged_files_terminate_build 1

%global is_suse %(test -e /etc/SuSE-release && echo 1 || echo 0)
%global is_fedora %(test -e /etc/fedora-release && echo 1 || echo 0)

%global _hardened_build    1
%if 0%{?opensuse_bs}
# define to 1 if building on openSUSE build service
%global build_on_obs       1
%global reconfigure_mode   1
%else
%global build_on_obs       0
%global reconfigure_mode   1
%endif

%global qmaildir           /var/indimail
%global _prefix            /usr
%global libexecdir         %{_prefix}/libexec/indimail
%global shareddir          %{_prefix}/share/indimail
%global mandir             %{_prefix}/share/man
%global plugindir          %{_prefix}/lib/indimail/plugins
%global qsysconfdir        /etc/indimail
%global ucspi_version      0.88
%global qmail_version      1.03
%global libdkim_version    1.4
%global libsrs2_version    1.0.18
%global tcpserver_plugin   1
%global noperms            1
%global see_base           For a description of IndiMail visit http://www.indimail.org
%global nolibdkim          0
%global nolibsrs2          0
%global noclamav           1
%global nodksignatures     0

%global _verbose           0
%global qcount             5
%global qbase              %{qmaildir}/queue
%global logdir             /var/log/indimail
%global servicedir         /service
%global dkimkeyfn          default

%global fedorareview       0

%if %build_on_obs == 1
%global packager Manvendra Bhangui <manvendra@indimail.org>

%global dist redhat
%global disttag rh

%if %{is_suse} != 0
%global dist suse
%global disttag suse
%endif
%if %{is_fedora} != 0
%global dist fedora
%global disttag rhfc
%endif
%endif

Summary: A Flexible SMTP server
Name: indimail-mta
Version: 2.0
%if %fedorareview == 0
Provides: daemontools = %{version}, ucspi-tcp = %{version}
%if %build_on_obs == 1
Release: 1.<B_CNT>
%else
Release: 1.B_CCNT%{?dist}
%endif
%else
Release: 1.1%{?dist}
%endif

## user/group management
# Note: it is not necessary to assign 555 for uid, gid. The package will use any id assigned to username, groupname
# at runtime
%global uid                555
%global gid                555
%global username           indimail
%global groupname          indimail
# Note: 999 indimail-mta does not require any specific values for uids/gids. 999 is just
# for rpmlint to shut up and stop complaining
Provides: user(%username)   = %uid
Provides: user(alias)       > 999
Provides: user(qmaild)      > 999
Provides: user(qmaill)      > 999
Provides: user(qmailp)      > 999
Provides: user(qmailq)      > 999
Provides: user(qmailr)      > 999
Provides: user(qmails)      > 999
Provides: group(%groupname) = %gid
Provides: group(nofiles)    > 999
Provides: group(qmail)      > 999
Provides: group(qscand)     > 999
Requires(pre): shadow-utils
Requires(postun): shadow-utils

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
Source1: http://cr.yp.to/software/qmail-%{qmail_version}.tar.gz
Source2: http://cr.yp.to/ucspi-tcp/ucspi-tcp-%{ucspi_version}.tar.gz
%if %nolibdkim == 0
Source3: http://downloads.sourceforge.net/indimail/libdkim-%{libdkim_version}.tar.gz
%endif
%if %nolibsrs2 == 0
Source4: http://downloads.sourceforge.net/indimail/libsrs2-%{libsrs2_version}.tar.gz
%endif
Source5: http://downloads.sourceforge.net/indimail/qmail-rpmlintrc
Source6: http://downloads.sourceforge.net/indimail/svctool

%if %noperms == 0
%if 0%{?suse_version} >= 1120
Source7:%{name}-permissions.easy
Source8:%{name}-permissions.secure
Source9:%{name}-permissions.paranoid
%endif
%endif

Patch1: http://downloads.sourceforge.net/indimail/Patches/qmail-%{qmail_version}.patch.gz
Patch2: http://downloads.sourceforge.net/indimail/Patches/ucspi-tcp-%{ucspi_version}.patch.gz

%if 0 != 0
NoSource: 1
NoSource: 2
%if %nolibdkim == 0
NoSource: 3
%endif
%if %nolibsrs2 == 0
NoSource: 4
%endif
%endif

URL: http://www.indimail.org
#AutoReqProv: No
Conflicts: indimail, indimail-mini, indimail-mta < 2.0
BuildRequires: openssl-devel rpm gcc gcc-c++ make bison binutils coreutils grep
BuildRequires: glibc glibc-devel openssl procps readline-devel
BuildRequires: sed ncurses-devel gettext-devel
BuildRequires: python-devel flex findutils
BuildRequires: readline gzip autoconf pkgconfig diffutils
%if %build_on_obs == 1
BuildRequires: libidn-devel
%endif

%if %build_on_obs == 1
##################################### OBS ####################################
%if 0%{?rhel_version} == 700
BuildRequires: groff-doc
%else
BuildRequires: groff
%endif

%if 0%{?suse_version}
BuildRequires: -post-build-checks  
#!BuildIgnore: post-build-checks  
%endif
##############################################################################
%else
BuildRequires: groff
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
%else
%if 0%{?fedora_version} > 17 || 0%{?centos_version} > 600 || 0%{?rhel_version} > 600
BuildRequires: libdb-devel
%else
BuildRequires: db4-devel
%endif
%endif

# rpm -qf /bin/ls
# rpm -qp --queryformat '%' {arch}\n some-file.rpm
# rpm --showrc for all macros
# rpm -qlp some-file.rpm
#Requires: /usr/sbin/useradd /usr/sbin/groupadd
Requires: /sbin/chkconfig /usr/sbin/userdel /usr/sbin/groupdel procps /usr/bin/awk
Requires: coreutils grep /bin/sh glibc openssl
Requires: daemontools ucspi-tcp
#BuildRoot: '%'(mktemp -ud '%'{_tmppath}/'%'{name}-'%'{version}-'%'{release}-XXXXXXX)
#
# IndiMail is choosy and runs on reliable OS only
#
Excludeos: windows 

%description
indimail-mta provides a standalone mta server that is part of the IndiMail
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

qmail, daemontools, ucspi-tcp, serialmail, qmailanalog, fastforward, mess822

Original Author: Dan J. Bernstein <djb@cr.yp.to>

%{see_base}

%package -n daemontools
Summary: A collection of tools for managing UNIX services
%if %fedorareview == 0
Conflicts: indimail daemontools ucspi-tcp
%else
Conflicts: indimail
%endif
%if %{undefined suse_version} && %{undefined sles_version}
Group: System Environment/Base
%else
Group: System/Base
%endif

%description -n daemontools
daemontools is a collection of tools for managing UNIX services. 

supervise monitors a service. It starts the service and restarts the
service if it dies. Setting up a new service is easy: all supervise
needs is a directory with a run script that runs the service.

multilog saves error messages to one or more logs. It optionally
timestamps each line and, for each log, includes or excludes lines
matching specified patterns. It automatically rotates logs to limit the
amount of disk space used. If the disk fills up, it pauses and tries
again, without losing any data.

Original Author: Dan J. Bernstein <djb@cr.yp.to>

%package -n ucspi-tcp
%if %fedorareview == 0
Conflicts: indimail daemontools ucspi-tcp
%else
Conflicts: indimail
%endif
Summary: A collection of tools for building TCP client-server applications
%if %{undefined suse_version} && %{undefined sles_version}
Group: System Environment/Base
%else
Group: System/Base
%endif

%description -n ucspi-tcp
tcpserver and tcpclient are easy-to-use command-line tools for building
TCP client-server applications.

tcpserver waits for incoming connections and, for each connection, runs
a program of your choice. Your program receives environment variables
showing the local and remote host names, IP addresses, and port numbers.

tcpserver offers a concurrency limit to protect you from running out of
processes and memory. When you are handling 40 (by default) simultaneous
connections, tcpserver smoothly defers acceptance of new connections.

tcpserver also provides TCP access control features, similar to
tcp-wrappers/tcpd\'s hosts.allow but much faster. Its access control
rules are compiled into a hashed format with cdb, so it can easily deal
with thousands of different hosts.

This package includes a recordio tool that monitors all the input and
output of a server.

tcpclient makes a TCP connection and runs a program of your choice. It
sets up the same environment variables as tcpserver.

This package includes several sample clients built on top of tcpclient:
who@, date@, finger@, http@, tcpcat, and mconnect.

tcpserver and tcpclient conform to UCSPI, the UNIX Client-Server Program
Interface, using the TCP protocol. UCSPI tools are available for several
different networks.

Original Author: Dan J. Bernstein <djb@cr.yp.to>

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
#### LIBDKIM ######################
if [ -d libdkim-%{libdkim_version} ] ; then
cd libdkim-%{libdkim_version}

if [ %{reconfigure_mode} -eq 0 ] ; then
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

%configure --prefix=%{_prefix} --libdir=%{_libdir} --mandir=%{mandir}
cd ..
fi

#### LIBSRS2 ######################
if [ -d libsrs2-%{libsrs2_version} ] ; then
cd libsrs2-%{libsrs2_version}

if [ %{reconfigure_mode} -eq 0 ] ; then
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

%configure --prefix=%{_prefix} --libdir=%{_libdir} --mandir=%{mandir}
cd ..
fi

#### qmail ######################
if [ -d qmail-%{qmail_version} ] ; then
  %{__sed} 's{QMAIL{%{qmaildir}{' qmail-%{qmail_version}/conf-qmail.in > qmail-%{qmail_version}/conf-qmail
  %{__sed} 's{SYSCONFDIR{%{qsysconfdir}{' qmail-%{qmail_version}/conf-sysconfdir.in > qmail-%{qmail_version}/conf-sysconfdir
  %{__sed} 's{SHAREDDIR{%{shareddir}{' qmail-%{qmail_version}/conf-shared.in > qmail-%{qmail_version}/conf-shared
  %{__sed} 's{PREFIX{%{_prefix}{' qmail-%{qmail_version}/conf-prefix.in > qmail-%{qmail_version}/conf-prefix
  %if %{tcpserver_plugin} == 1
    echo "-DLOAD_SHARED_OBJECTS" > qmail-%{qmail_version}/conf-dlopen
  %else
    %{__rm} -f qmail-%{qmail_version}/conf-dlopen
  %endif
  # create conf-users
  (
  echo "alias"
  echo "qmaild"
  echo "qmaill"
  echo "root"
  echo "qmailp"
  echo "qmailq"
  echo "qmailr"
  echo "qmails"
  echo %username
  echo "qscand"
  echo 
  echo "The qmail system is heavily partitioned for security; it does almost"
  echo "nothing as root."
  echo
  echo "The first nine lines of this file are the alias user, the daemon user,"
  echo "the log user, the owner of miscellaneous files such as binaries, the"
  echo "passwd user, the queue user, the remote user, the send user, the"
  echo "indimail user and the virus scan user."
  ) > /tmp/conf-users
  diff /tmp/conf-users qmail-%{qmail_version}/conf-users > /dev/null 2>&1
  if [ $? -ne 0 ] ; then
    %{__mv} /tmp/conf-users qmail-%{qmail_version}/conf-users
  else
    %{__rm} -f /tmp/conf-users
  fi
   # create conf-groups
   (
   echo "qmail"
   echo "nofiles"
   echo %groupname
   echo "qscand"
   echo
   echo "These are the qmail groups. The second group should not have access to"
   echo "any files, but it must be usable for processes; this requirement"
   echo "excludes the \`\`nogroup'' and \`\`nobody'' groups on many systems."
   ) > /tmp/conf-groups
   diff /tmp/conf-groups qmail-%{qmail_version}/conf-groups > /dev/null 2>&1
   if [ $? -ne 0 ] ; then
     %{__mv} /tmp/conf-groups qmail-%{qmail_version}/conf-groups
   else
     %{__rm} -f /tmp/conf-groups
   fi
fi
#### ucspi-tcp ######################
if [ -d ucspi-tcp-%{ucspi_version} ] ; then
  %{__sed} 's{HOME{%{_prefix}{' ucspi-tcp-%{ucspi_version}/conf-home.in > ucspi-tcp-%{ucspi_version}/conf-home
  %{__sed} 's{SHAREDDIR{%{shareddir}{' ucspi-tcp-%{ucspi_version}/conf-shared.in > ucspi-tcp-%{ucspi_version}/conf-shared
  %if %{tcpserver_plugin} == 1
    echo "-DLOAD_SHARED_OBJECTS" > ucspi-tcp-%{ucspi_version}/conf-dlopen
  %else
    %{__rm} -f ucspi-tcp-%{ucspi_version}/conf-dlopen
  %endif
fi
#### svctool ########################
if [ -f ../SOURCES/svctool ] ; then
%{__cp} ../SOURCES/svctool .
fi
if [ -f ../SOURCES/svctool.gz ] ; then
gunzip -c ../SOURCES/svctool.gz > svctool
fi


%install
ID=$(id -u)
%{__mkdir_p} %{buildroot}%{_prefix}
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
  if [ -f ./svctool ] ; then
    %{__mkdir_p} %{buildroot}%{_prefix}/sbin
    %{__cp} ./svctool %{buildroot}%{_prefix}/sbin/svctool
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
install -m 644 %{S:6} %{buildroot}%{_sysconfdir}/permissions.d/%{name}-permissions
install -m 644 %{S:7} %{buildroot}%{_sysconfdir}/permissions.d/%{name}-permissions.secure
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
# remove devel files as we are not building a devel package
%{__rm} -rf %{buildroot}%{mandir}/man3
%{__rm} -rf %{buildroot}%{_prefix}/include
%{__rm} -rf %{buildroot}%{qmaildir}/queue
for i in libdkim libsrs2
do
%{__rm} -f  %{buildroot}%{_libdir}/$i.a
%{__rm} -f  %{buildroot}%{_libdir}/$i.so
done

# Compress the man pages
find %{buildroot}%{mandir} -type f -exec gzip -q {} \;

if [ -x /bin/touch ] ; then
  TOUCH=/bin/touch
elif [ -x /usr/bin/touch ] ; then
  TOUCH=/usr/bin/touch
else
  TOUCH=/bin/touch
fi
# Create these files so that %%ghost does not complain
for i in tcp.smtp tcp.smtp.cdb tcp.qmtp tcp.qmtp.cdb tcp.qmqp tcp.qmqp.cdb
do
  if [ ! -f %{buildroot}%{qsysconfdir}/$i ] ; then
    $TOUCH %{buildroot}%{qsysconfdir}/$i
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
  echo "%ghost %config(noreplace,missingok)               %{qsysconfdir}/control/$i"
  echo $i 1>&3
  $TOUCH %{buildroot}%{qsysconfdir}/control/$i
done
) > config_files.list 3>%{buildroot}%{qsysconfdir}/controlfiles

%files -f config_files.list
%defattr(-, root, root,-)
#
# Directories
#
%dir %attr(555,root,qmail)        %{qmaildir}
                                  %{qmaildir}/bin
                                  %{qmaildir}/sbin
                                  %{qmaildir}/users
                                  %{qmaildir}/control
                                  %{qmaildir}/libexec
%dir %attr(555,root,root)         %{libexecdir}
%dir %attr(555,root,qmail)        %{shareddir}
%dir %attr(555,root,qmail)        %{shareddir}/boot
%dir %attr(555,root,qmail)        %{shareddir}/doc
%dir %attr(2775,alias,qmail)      %{qmaildir}/alias
%dir %attr(750,qscand,qscand)     %{qmaildir}/qscanq
%dir %attr(750,qscand,qscand)     %{qmaildir}/qscanq/root
%dir %attr(750,qscand,qscand)     %{qmaildir}/qscanq/root/scanq
%dir %attr(775,indimail,qmail)    %{qsysconfdir}/control
%dir %attr(2775,qmailr,qmail)     %{qsysconfdir}/control/ratelimit
%dir %attr(775,indimail,qmail)    %{qsysconfdir}/control/domainkeys
%dir %attr(775,indimail,qmail)    %{qsysconfdir}/control/defaultqueue
%dir %attr(2775,indimail,qmail)   %{qmaildir}/autoturn
%if "%{mandir}" != "/usr/share/man"
%dir %attr(755,root,root)         %{mandir}
%dir %attr(755,root,root)         %{mandir}/man1
%dir %attr(755,root,root)         %{mandir}/man5
%dir %attr(755,root,root)         %{mandir}/man7
%dir %attr(755,root,root)         %{mandir}/man8
%dir %attr(755,root,root)         %{mandir}/cat1
%dir %attr(755,root,root)         %{mandir}/cat5
%dir %attr(755,root,root)         %{mandir}/cat7
%dir %attr(755,root,root)         %{mandir}/cat8
%else
%dir %attr(755,root,root)         %{mandir}/cat1
%dir %attr(755,root,root)         %{mandir}/cat5
%dir %attr(755,root,root)         %{mandir}/cat7
%dir %attr(755,root,root)         %{mandir}/cat8
%endif
%if "%{_prefix}" != "/usr"
%dir %attr(555,root,root)         %{_libdir}
%endif
%dir %attr(775,root,qmail)            %{qsysconfdir}/etc
%dir %attr(775,indimail,qmail)        %{qsysconfdir}/users
%dir %attr(555,root,qmail)            %{plugindir}

%attr(444,root,root) %config(noreplace)           %{qsysconfdir}/controlfiles

%ghost %config(noreplace,missingok)               %{qsysconfdir}/tcp.smtp
%ghost %config(noreplace,missingok)               %{qsysconfdir}/tcp.qmtp
%ghost %config(noreplace,missingok)               %{qsysconfdir}/tcp.qmqp
#
# These files will get removed during uninstallation
#
%ghost %attr(0644,indimail,indimail)              %{qsysconfdir}/tcp.smtp.cdb
%ghost %attr(0644,indimail,indimail)              %{qsysconfdir}/tcp.qmtp.cdb
%ghost %attr(0644,indimail,indimail)              %{qsysconfdir}/tcp.qmqp.cdb

%attr(644,root,qmail) %config(noreplace)           %{qsysconfdir}/indimail-mta.te
%attr(644,root,qmail) %config(noreplace)           %{qsysconfdir}/indimail-mta.fc
%attr(444,root,root)  %config(noreplace)           %{qsysconfdir}/qmailprog.list
%attr(644,root,qmail) %config(noreplace)           %{qsysconfdir}/etc/leapsecs.dat
%attr(644,root,qmail) %config(noreplace)           %{qsysconfdir}/etc/leapsecs.txt

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%attr(644,root,root)                              %{_sysconfdir}/permissions.d/%{name}-permissions
%attr(644,root,root)                              %{_sysconfdir}/permissions.d/%{name}-permissions.secure
%endif
%endif

#
# Binaries
#
%attr(6551,qscand,qmail)                %{_prefix}/sbin/qhpsi
%attr(6551,qmailq,qmail)                %{_prefix}/sbin/qmail-queue
%attr(4551,qscand,qscand)               %{_prefix}/sbin/qscanq
%attr(2551,root,qscand)                 %{_prefix}/sbin/run-cleanq

%attr(755,root,qmail)                   %{_prefix}/sbin/qscanq-stdin
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-popbull
%attr(755,root,qmail)                   %{_prefix}/bin/ofmipd
%attr(755,root,qmail)                   %{_prefix}/bin/queue-fix
%attr(755,root,qmail)                   %{_prefix}/bin/maildirwatch
%attr(755,root,qmail)                   %{_prefix}/bin/bouncesaying
%attr(755,root,qmail)                   %{_prefix}/bin/checkaddr
%attr(755,root,qmail)                   %{_prefix}/bin/autoresponder
%attr(755,root,qmail)                   %{_prefix}/bin/822headerfilter
%attr(755,root,qmail)                   %{_prefix}/bin/checkdomain
%attr(755,root,qmail)                   %{_prefix}/bin/maildirqmtp
%attr(755,root,qmail)                   %{_prefix}/bin/qmailctl
%attr(755,root,qmail)                   %{_prefix}/bin/printmaillist
%attr(755,root,qmail)                   %{_prefix}/bin/newinclude
%attr(755,root,qmail)                   %{_prefix}/bin/recordio
%attr(755,root,qmail)                   %{_prefix}/bin/uacl
%attr(755,root,qmail)                   %{_prefix}/bin/cdbmake
%attr(755,root,qmail)                   %{_prefix}/bin/mlmatchup
%attr(755,root,qmail)                   %{_prefix}/bin/822print
%attr(755,root,qmail)                   %{_prefix}/bin/sendmail
%attr(755,root,qmail)                   %{_prefix}/bin/irmail
%attr(755,root,qmail)                   %{_prefix}/bin/ifaddr
%attr(755,root,qmail)                   %{_prefix}/bin/matchup
%attr(755,root,qmail)                   %{_prefix}/bin/setforward
%attr(755,root,qmail)                   %{_prefix}/bin/822date
%attr(755,root,qmail)                   %{_prefix}/bin/cdbdump
%attr(755,root,qmail)                   %{_prefix}/bin/iftocc
%attr(755,root,qmail)                   %{_prefix}/bin/serialqmtp
%attr(755,root,qmail)                   %{_prefix}/bin/delcr
%attr(755,root,qmail)                   %{_prefix}/bin/setmaillist
%attr(755,root,qmail)                   %{_prefix}/bin/maildirdeliver
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-showctl
%attr(755,root,qmail)                   %{_prefix}/bin/822headerok
%attr(755,root,qmail)                   %{_prefix}/bin/mbox2maildir
%attr(755,root,qmail)                   %{_prefix}/bin/printforward
%attr(755,root,qmail)                   %{_prefix}/bin/preline
%attr(755,root,qmail)                   %{_prefix}/bin/cdbget
%attr(755,root,qmail)                   %{_prefix}/bin/cdbgetm
%attr(755,root,qmail)                   %{_prefix}/bin/822addr
%attr(755,root,qmail)                   %{_prefix}/bin/cdbstats
%attr(755,root,qmail)                   %{_prefix}/bin/maildirserial
%attr(755,root,qmail)                   %{_prefix}/bin/822fields
%attr(755,root,qmail)                   %{_prefix}/bin/ofmipname
%attr(755,root,qmail)                   %{_prefix}/bin/822bodyfilter
%attr(755,root,qmail)                   %{_prefix}/bin/serialsmtp
%attr(755,root,qmail)                   %{_prefix}/bin/predate
%attr(755,root,qmail)                   %{_prefix}/bin/condredirect
%attr(755,root,qmail)                   %{_prefix}/bin/fastforward
%attr(755,root,qmail)                   %{_prefix}/bin/inewaliases
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-rm
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-pop3d
%attr(755,root,qmail)                   %{_prefix}/bin/xqp
%attr(755,root,qmail)                   %{_prefix}/bin/dot-forward
%attr(755,root,qmail)                   %{_prefix}/bin/datemail
%attr(755,root,qmail)                   %{_prefix}/bin/qbase64
%attr(755,root,qmail)                   %{_prefix}/bin/swaks
%attr(755,root,qmail)                   %{_prefix}/bin/replier
%attr(755,root,qmail)                   %{_prefix}/bin/cdbtest
%attr(755,root,qmail)                   %{_prefix}/bin/822header
%attr(755,root,qmail)                   %{_prefix}/bin/qmaildirmake
%attr(755,root,qmail)                   %{_prefix}/bin/maildir2mbox
%attr(755,root,qmail)                   %{_prefix}/bin/columnt
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-smtpd
%attr(755,root,qmail)                   %{_prefix}/bin/qarf
%attr(755,root,qmail)                   %{_prefix}/bin/qaes
%attr(755,root,qmail)                   %{_prefix}/bin/qnotify
%attr(755,root,qmail)                   %{_prefix}/bin/rrt
%attr(755,root,qmail)                   %{_prefix}/bin/drate
%attr(755,root,qmail)                   %{_prefix}/bin/cidr
%attr(755,root,qmail)                   %{_prefix}/bin/spawn-filter
%attr(755,root,qmail)                   %{_prefix}/bin/tcp-env
%attr(755,root,qmail)                   %{_prefix}/bin/rrforward
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-dk
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-dkim
%attr(755,root,qmail)                   %{_prefix}/bin/dk-filter
%attr(755,root,qmail)                   %{_prefix}/bin/mailsubj
%attr(755,root,qmail)                   %{_prefix}/bin/condtomaildir
%attr(755,root,qmail)                   %{_prefix}/bin/822field
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-cat
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-poppass
%attr(755,root,qmail)                   %{_prefix}/bin/replier-config
%attr(755,root,qmail)                   %{_prefix}/bin/forward
%attr(755,root,qmail)                   %{_prefix}/bin/new-inject
%attr(755,root,qmail)                   %{_prefix}/bin/filterto
%attr(755,root,qmail)                   %{_prefix}/bin/fixcrio
%attr(755,root,qmail)                   %{_prefix}/bin/822body
%attr(755,root,qmail)                   %{_prefix}/bin/dktest
%attr(755,root,qmail)                   %{_prefix}/bin/iftoccfrom
%attr(755,root,qmail)                   %{_prefix}/bin/qreceipt
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-qmqpd
%attr(755,root,qmail)                   %{_prefix}/bin/except
%attr(755,root,qmail)                   %{_prefix}/bin/dknewkey
%attr(755,root,qmail)                   %{_prefix}/bin/maildirsmtp
%attr(755,root,qmail)                   %{_prefix}/bin/argv0
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-qmtpd
%attr(755,root,qmail)                   %{_prefix}/bin/addcr
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-inject
%attr(755,root,qmail)                   %{_prefix}/bin/serialcmd
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-qread
%attr(755,root,qmail)                   %{_prefix}/bin/822received
%attr(755,root,qmail)                   %{_prefix}/bin/maildircmd
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-qfilter
%attr(755,root,qmail)                   %{_prefix}/bin/surblfilter
%attr(755,root,qmail)                   %{_prefix}/bin/surblqueue
%attr(755,root,qmail)                   %{_prefix}/bin/spfquery
%attr(755,root,qmail)                   %{_prefix}/bin/srsfilter

%attr(755,root,qmail)                   %{libexecdir}/rpmattr
%attr(755,root,qmail)                   %{libexecdir}/leapsecs
%attr(755,root,qmail)                   %{libexecdir}/yearcal
%attr(755,root,qmail)                   %{libexecdir}/nowutc
%attr(755,root,qmail)                   %{libexecdir}/qmail-lagcheck
%attr(755,root,qmail)                   %{libexecdir}/batv
%attr(755,root,qmail)                   %{libexecdir}/zsuccesses
%attr(755,root,qmail)                   %{libexecdir}/deferrals
%attr(755,root,qmail)                   %{libexecdir}/rsmtprecipients
%attr(755,root,qmail)                   %{libexecdir}/smtp-matchup
%attr(755,root,qmail)                   %{libexecdir}/zrecipients
%attr(755,root,qmail)                   %{libexecdir}/xsender
%attr(755,root,qmail)                   %{libexecdir}/rxdelay
%attr(755,root,qmail)                   %{libexecdir}/zspam
%attr(755,root,qmail)                   %{libexecdir}/recipients
%attr(755,root,qmail)                   %{libexecdir}/rsmtprdomains
%attr(755,root,qmail)                   %{libexecdir}/rsmtpfailures
%attr(755,root,qmail)                   %{libexecdir}/successes
%attr(755,root,qmail)                   %{libexecdir}/rspamrdomain
%attr(755,root,qmail)                   %{libexecdir}/multilog-matchup
%attr(755,root,qmail)                   %{libexecdir}/zddist
%attr(755,root,qmail)                   %{libexecdir}/zsenders
%attr(755,root,qmail)                   %{libexecdir}/senders
%attr(755,root,qmail)                   %{libexecdir}/suids
%attr(755,root,qmail)                   %{libexecdir}/zsmtp
%attr(755,root,qmail)                   %{libexecdir}/zoverall
%attr(755,root,qmail)                   %{libexecdir}/rhosts
%attr(755,root,qmail)                   %{libexecdir}/zrhosts
%attr(755,root,qmail)                   %{libexecdir}/failures
%attr(755,root,qmail)                   %{libexecdir}/rsmtp
%attr(755,root,qmail)                   %{libexecdir}/rsmtpsenders
%attr(755,root,qmail)                   %{libexecdir}/zdeferrals
%attr(755,root,qmail)                   %{libexecdir}/rspamstat
%attr(755,root,qmail)                   %{libexecdir}/zsuids
%attr(755,root,qmail)                   %{libexecdir}/xrecipient
%attr(755,root,qmail)                   %{libexecdir}/rspamhist
%attr(755,root,qmail)                   %{libexecdir}/rsmtpsdomains
%attr(755,root,qmail)                   %{libexecdir}/zfailures
%attr(755,root,qmail)                   %{libexecdir}/zsendmail
%attr(755,root,qmail)                   %{libexecdir}/ddist
%attr(755,root,qmail)                   %{libexecdir}/rspamsdomain
%attr(755,root,qmail)                   %{libexecdir}/zrxdelay

%attr(755,root,qmail)                   %{_prefix}/sbin/plugtest
%attr(755,root,qmail)                   %{_prefix}/sbin/sys-checkpwd
%attr(755,root,qmail)                   %{_prefix}/sbin/ldap-checkpwd
%attr(755,root,qmail)                   %{_prefix}/sbin/svctool
%attr(755,root,qmail)                   %{_prefix}/sbin/qmail-nullqueue
%attr(755,root,qmail)                   %{_prefix}/sbin/qmail-multi
%attr(755,root,qmail)                   %{_prefix}/sbin/cleanq
%attr(755,root,qmail)                   %{_prefix}/sbin/qmail-tcpok
%attr(755,root,qmail)                   %{_prefix}/sbin/qmail-tcpto
%attr(755,root,qmail)                   %{_prefix}/sbin/qmail-qmqpc

%attr(751,root,qmail)                   %{_prefix}/bin/qmail-newu
%attr(751,root,qmail)                   %{_prefix}/bin/qmail-newmrh
%attr(751,root,qmail)                   %{_prefix}/bin/qmail-cdb
%attr(751,root,qmail)                   %{_prefix}/bin/recipient-cdb

%attr(751,root,qmail)                   %{_prefix}/sbin/qmail-daemon
%attr(751,root,qmail)                   %{_prefix}/sbin/qmail-start
%attr(751,root,qmail)                   %{_prefix}/sbin/qmail-lspawn
%attr(751,root,qmail)                   %{_prefix}/sbin/qmail-rspawn
%attr(751,root,qmail)                   %{_prefix}/sbin/qmail-clean
%attr(751,root,qmail)                   %{_prefix}/sbin/qmail-send
%attr(751,root,qmail)                   %{_prefix}/sbin/qmail-todo

%attr(755,root,qmail)                   %{_prefix}/bin/qmail-getpw
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-local
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-remote
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-pw2u
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-popup

%attr(755,root,qmail)                   %{_prefix}/sbin/relaytest
%attr(755,root,qmail)                   %{_prefix}/sbin/splogger

%attr(755,root,qmail)                   %{libexecdir}/svscanboot
%attr(755,root,qmail)                   %{libexecdir}/atrn
%attr(755,root,qmail)                   %{libexecdir}/etrn
%attr(755,root,qmail)                   %{libexecdir}/cdbmake-12
%attr(755,root,qmail)                   %{libexecdir}/cdbmake-sv
%attr(755,root,qmail)                   %{libexecdir}/config-fast
%attr(755,root,qmail)                   %{libexecdir}/idedit
%attr(755,root,qmail)                   %{libexecdir}/hostname
%attr(755,root,qmail)                   %{libexecdir}/qmailconfig
%attr(755,root,qmail)                   %{libexecdir}/dnstxt
%attr(755,root,qmail)                   %{libexecdir}/dnsmxip
%attr(755,root,qmail)                   %{libexecdir}/dnsfq
%attr(755,root,qmail)                   %{libexecdir}/dnsptr
%attr(755,root,qmail)                   %{libexecdir}/dnsip
%attr(755,root,qmail)                   %{libexecdir}/dnscname
%attr(755,root,qmail)                   %{libexecdir}/envmigrate
%attr(755,root,qmail)                   %{libexecdir}/qsmhook
%attr(755,root,qmail)                   %{libexecdir}/update_tmprsadh
%attr(751,root,qmail)                   %{libexecdir}/instcheck
%attr(755,root,qmail)                   %{libexecdir}/whois
%attr(755,root,qmail)                   %{libexecdir}/testzero
%attr(755,root,qmail)                   %{libexecdir}/qmail-lint
%attr(755,root,qmail)                   %{libexecdir}/ipmeprint
%attr(755,root,qmail)                   %{libexecdir}/elq
%attr(755,root,qmail)                   %{libexecdir}/pinq
%attr(755,root,qmail)                   %{libexecdir}/qail
%attr(755,root,qmail)                   %{libexecdir}/qpq

%if %fedorareview == 0
%docdir %{mandir}
%attr(0644,root,root)                   %{mandir}/man[1,4,5,7,8]/*
%attr(0644,root,root)                   %{mandir}/cat[1,4,5,7,8]/*
%else
%attr(0644,root,root)                   %{mandir}/man1/qmail-qfilter.1.gz
%attr(0644,root,root)                   %{mandir}/man1/drate.1.gz
%attr(0644,root,root)                   %{mandir}/man1/replier-config.1.gz
%attr(0644,root,root)                   %{mandir}/man1/replier.1.gz
%attr(0644,root,root)                   %{mandir}/man1/condtomaildir.1.gz
%attr(0644,root,root)                   %{mandir}/man1/filterto.1.gz
%attr(0644,root,root)                   %{mandir}/man1/checkdomain.1.gz
%attr(0644,root,root)                   %{mandir}/man1/checkaddr.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822fields.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822addr.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822headerfilter.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822bodyfilter.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822headerok.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822body.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822print.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822received.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822date.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822header.1.gz
%attr(0644,root,root)                   %{mandir}/man1/822field.1.gz
%attr(0644,root,root)                   %{mandir}/man1/new-inject.1.gz
%attr(0644,root,root)                   %{mandir}/man1/ifaddr.1.gz
%attr(0644,root,root)                   %{mandir}/man1/iftoccfrom.1.gz
%attr(0644,root,root)                   %{mandir}/man1/iftocc.1.gz
%attr(0644,root,root)                   %{mandir}/man1/newinclude.1.gz
%attr(0644,root,root)                   %{mandir}/man1/setmaillist.1.gz
%attr(0644,root,root)                   %{mandir}/man1/printmaillist.1.gz
%attr(0644,root,root)                   %{mandir}/man1/inewaliases.1.gz
%attr(0644,root,root)                   %{mandir}/man1/setforward.1.gz
%attr(0644,root,root)                   %{mandir}/man1/printforward.1.gz
%attr(0644,root,root)                   %{mandir}/man1/fastforward.1.gz
%attr(0644,root,root)                   %{mandir}/man1/dot-forward.1.gz
%attr(0644,root,root)                   %{mandir}/man1/plugtest.1.gz
%attr(0644,root,root)                   %{mandir}/man1/qmail-rm.1.gz
%attr(0644,root,root)                   %{mandir}/man1/qaes.1.gz
%attr(0644,root,root)                   %{mandir}/man1/cdbdump.1.gz
%attr(0644,root,root)                   %{mandir}/man1/cdbmake.1.gz
%attr(0644,root,root)                   %{mandir}/man1/cdbstats.1.gz
%attr(0644,root,root)                   %{mandir}/man1/cdbtest.1.gz
%attr(0644,root,root)                   %{mandir}/man1/cdbgetm.1.gz
%attr(0644,root,root)                   %{mandir}/man1/cdbget.1.gz
%attr(0644,root,root)                   %{mandir}/man1/columnt.1.gz
%attr(0644,root,root)                   %{mandir}/man1/xrecipient.1.gz
%attr(0644,root,root)                   %{mandir}/man1/xsender.1.gz
%attr(0644,root,root)                   %{mandir}/man1/xqp.1.gz
%attr(0644,root,root)                   %{mandir}/man1/matchup.1.gz
%attr(0644,root,root)                   %{mandir}/man1/maildirserial.1.gz
%attr(0644,root,root)                   %{mandir}/man1/maildirsmtp.1.gz
%attr(0644,root,root)                   %{mandir}/man1/maildirqmtp.1.gz
%attr(0644,root,root)                   %{mandir}/man1/maildircmd.1.gz
%attr(0644,root,root)                   %{mandir}/man1/serialsmtp.1.gz
%attr(0644,root,root)                   %{mandir}/man1/serialqmtp.1.gz
%attr(0644,root,root)                   %{mandir}/man1/serialcmd.0.gz
%attr(0644,root,root)                   %{mandir}/man1/serialcmd.1.gz
%attr(0644,root,root)                   %{mandir}/man1/qarf.1.gz
%attr(0644,root,root)                   %{mandir}/man1/rrt.1.gz
%attr(0644,root,root)                   %{mandir}/man1/qnotify.1.gz
%attr(0644,root,root)                   %{mandir}/man1/autoresponder.1.gz
%attr(0644,root,root)                   %{mandir}/man1/maildirdeliver.1.gz
%attr(0644,root,root)                   %{mandir}/man1/relaytest.1.gz
%attr(0644,root,root)                   %{mandir}/man1/tcp-env.1.gz
%attr(0644,root,root)                   %{mandir}/man1/preline.1.gz
%attr(0644,root,root)                   %{mandir}/man1/qreceipt.1.gz
%attr(0644,root,root)                   %{mandir}/man1/mailsubj.1.gz
%attr(0644,root,root)                   %{mandir}/man1/maildirwatch.1.gz
%attr(0644,root,root)                   %{mandir}/man1/maildir2mbox.1.gz
%attr(0644,root,root)                   %{mandir}/man1/mbox2maildir.1.gz
%attr(0644,root,root)                   %{mandir}/man1/qmaildirmake.1.gz
%attr(0644,root,root)                   %{mandir}/man1/except.1.gz
%attr(0644,root,root)                   %{mandir}/man1/bouncesaying.1.gz
%attr(0644,root,root)                   %{mandir}/man1/condredirect.1.gz
%attr(0644,root,root)                   %{mandir}/man1/rrforward.1.gz
%attr(0644,root,root)                   %{mandir}/man1/forward.1.gz
%attr(0644,root,root)                   %{mandir}/man1/swaks.1.gz
%attr(0644,root,root)                   %{mandir}/man1/qbase64.1.gz
%attr(0644,root,root)                   %{mandir}/man1/uacl.1.gz
%attr(0644,root,root)                   %{mandir}/man1/recordio.1.gz
%attr(0644,root,root)                   %{mandir}/man1/fixcrio.1.gz
%attr(0644,root,root)                   %{mandir}/man1/delcr.1.gz
%attr(0644,root,root)                   %{mandir}/man1/addcr.1.gz
%attr(0644,root,root)                   %{mandir}/man1/argv0.1.gz
%attr(0644,root,root)                   %{mandir}/man1/datemail.1.gz
%attr(0644,root,root)                   %{mandir}/man1/predate.1.gz
%attr(0644,root,root)                   %{mandir}/man1/qmail-cat.1.gz
%attr(0644,root,root)                   %{mandir}/man1/cidr.1.gz
%attr(0644,root,root)                   %{mandir}/man1/spfquery.1.gz
%if %nolibsrs2 == 0
%attr(0644,root,root)                   %{mandir}/man1/srs.1.gz
%attr(0644,root,root)                   %{mandir}/man1/srsfilter.1.gz
%endif
%attr(0644,root,root)                   %{mandir}/man5/rfc-4870.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-4871.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-3798.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-3834.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-2476.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-2635.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-2505.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-2554.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-2645.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-2104.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-1321.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-1985.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-1894.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-1893.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-2821.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-1845.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rfc-822.5.gz
%attr(0644,root,root)                   %{mandir}/man5/qmtp.5.gz
%attr(0644,root,root)                   %{mandir}/man5/rewriting.5.gz
%attr(0644,root,root)                   %{mandir}/man5/tcp-environ.5.gz
%attr(0644,root,root)                   %{mandir}/man5/qmail-users.5.gz
%attr(0644,root,root)                   %{mandir}/man5/qmail-log.5.gz
%attr(0644,root,root)                   %{mandir}/man5/qmail-header.5.gz
%attr(0644,root,root)                   %{mandir}/man5/qmail-control.5.gz
%attr(0644,root,root)                   %{mandir}/man5/dot-qmail.5.gz
%attr(0644,root,root)                   %{mandir}/man5/mbox.5.gz
%attr(0644,root,root)                   %{mandir}/man5/maildir.5.gz
%attr(0644,root,root)                   %{mandir}/man5/envelopes.5.gz
%attr(0644,root,root)                   %{mandir}/man5/addresses.5.gz
%if %nolibsrs2 == 0
%attr(0644,root,root)                   %{mandir}/man5/qmail-srs.5.gz
%endif
%attr(0644,root,root)                   %{mandir}/man7/forgeries.7.gz
%attr(0644,root,root)                   %{mandir}/man7/qmail.7.gz
%attr(0644,root,root)                   %{mandir}/man7/qmail-limits.7.gz
%attr(0644,root,root)                   %{mandir}/man8/run-cleanq.8.gz
%attr(0644,root,root)                   %{mandir}/man8/cleanq.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qscanq-stdin.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qscanq.8.gz
%attr(0644,root,root)                   %{mandir}/man8/ldap-checkpwd.8.gz
%attr(0644,root,root)                   %{mandir}/man8/sys-checkpwd.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-command.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-poppass.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-smtpd.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-qmtpd.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-qmqpd.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-qmqpc.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-popup.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-pop3d.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-tcpto.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-tcpok.8.gz
%attr(0644,root,root)                   %{mandir}/man8/queue-fix.0.gz
%attr(0644,root,root)                   %{mandir}/man8/queue-fix.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-popbull.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-qread.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-pw2u.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-newu.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-cdb.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-sql.8.gz
%attr(0644,root,root)                   %{mandir}/man8/recipient-cdb.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-newmrh.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-showctl.8.gz
%attr(0644,root,root)                   %{mandir}/man8/irmail.8.gz
%attr(0644,root,root)                   %{mandir}/man8/isendmail.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-inject.8.gz
%attr(0644,root,root)                   %{mandir}/man8/spawn-filter.8.gz
%attr(0644,root,root)                   %{mandir}/man8/surblfilter.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-multi.8.gz
%attr(0644,root,root)                   %{mandir}/man8/dktest.8.gz
%attr(0644,root,root)                   %{mandir}/man8/dknewkey.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-dk.8.gz
%attr(0644,root,root)                   %{mandir}/man8/dk-filter.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-dkim.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-nullqueue.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-queue.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qhpsi.8.gz
%attr(0644,root,root)                   %{mandir}/man8/surblqueue.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-internals.8.gz
%attr(0644,root,root)                   %{mandir}/man8/splogger.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-start.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-daemon.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-todo.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-send.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-clean.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-rspawn.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-remote.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-getpw.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-lspawn.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmail-local.8.gz
%attr(0644,root,root)                   %{mandir}/man8/qmailctl.8.gz
%attr(0644,root,root)                   %{mandir}/man8/ofmipname.8.gz
%attr(0644,root,root)                   %{mandir}/man8/ofmipd.8.gz
%attr(0644,root,root)                   %{mandir}/man8/mlmatchup.8.gz
%if %nolibdkim == 0
%attr(0644,root,root)                   %{mandir}/man8/dkim.8.gz
%endif

%attr(0644,root,root)                   %{mandir}/cat1/qmail-qfilter.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/replier-config.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/replier.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/condtomaildir.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/filterto.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/checkdomain.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/checkaddr.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822fields.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822addr.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822headerfilter.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822bodyfilter.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822headerok.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822body.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822print.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822received.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822date.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822header.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/822field.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/new-inject.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/ifaddr.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/iftoccfrom.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/iftocc.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/newinclude.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/setmaillist.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/printmaillist.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/newaliases.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/setforward.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/printforward.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/fastforward.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/dot-forward.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/qmail-rm.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/columnt.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/xrecipient.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/xsender.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/xqp.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/matchup.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/maildirserial.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/maildirsmtp.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/maildirqmtp.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/maildircmd.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/serialsmtp.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/serialqmtp.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/autoresponder.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/maildirdeliver.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/tcp-env.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/preline.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/qreceipt.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/mailsubj.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/maildirwatch.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/maildir2mbox.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/qmaildirmake.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/except.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/bouncesaying.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/condredirect.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/rrforward.0.gz
%attr(0644,root,root)                   %{mandir}/cat1/forward.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/rewriting.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/tcp-environ.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/qmail-users.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/qmail-log.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/qmail-header.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/qmail-control.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/dot-qmail.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/mbox.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/maildir.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/envelopes.0.gz
%attr(0644,root,root)                   %{mandir}/cat5/addresses.0.gz
%attr(0644,root,root)                   %{mandir}/cat7/qmail.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/run-cleanq.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/cleanq.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qscanq-stdin.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qscanq.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-command.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-poppass.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-smtpd.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-qmtpd.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-qmqpd.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-qmqpc.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-popup.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-pop3d.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-tcpto.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-tcpok.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-qread.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-pw2u.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-newu.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-cdb.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/recipient-cdb.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-newmrh.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-showctl.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-inject.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/spawn-filter.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-multi.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-dk.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-queue.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/splogger.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-start.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-todo.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-send.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-clean.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-rspawn.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-remote.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-getpw.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-lspawn.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/qmail-local.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/ofmipname.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/ofmipd.0.gz
%attr(0644,root,root)                   %{mandir}/cat8/mlmatchup.0.gz
%attr(0644,root,root)                   %{mandir}/cat7/qmail-limits.0.gz
%attr(0644,root,root)                   %{mandir}/cat7/forgeries.0.gz
%endif

# daemontools
%if %fedorareview == 0
%attr(755,root,qmail)                   %{_prefix}/bin/envdir
%attr(755,root,qmail)                   %{_prefix}/bin/envuidgid
%attr(755,root,qmail)                   %{_prefix}/bin/fghack
%attr(755,root,qmail)                   %{_prefix}/bin/multilog
%attr(755,root,qmail)                   %{_prefix}/bin/pgrphack
%attr(755,root,qmail)                   %{_prefix}/bin/setlock
%attr(755,root,qmail)                   %{_prefix}/bin/setuidgid
%attr(755,root,qmail)                   %{_prefix}/bin/softlimit
%attr(755,root,qmail)                   %{_prefix}/bin/supervise
%attr(755,root,qmail)                   %{_prefix}/bin/svc
%attr(755,root,qmail)                   %{_prefix}/bin/svok
%attr(755,root,qmail)                   %{_prefix}/bin/svstat
%attr(755,root,qmail)                   %{_prefix}/bin/tai64n
%attr(755,root,qmail)                   %{_prefix}/bin/tai64nlocal
%attr(755,root,qmail)                   %{_prefix}/bin/tai64nunix
%attr(755,root,qmail)                   %{_prefix}/bin/tai2tai64n
%attr(755,root,qmail)                   %{_prefix}/bin/tai64n2tai
%attr(755,root,qmail)                   %{_prefix}/bin/spipe
%attr(755,root,qmail)                   %{_prefix}/bin/qfilelog
%attr(755,root,qmail)                   %{_prefix}/bin/multipipe
%attr(755,root,qmail)                   %{_prefix}/bin/teepipe
%attr(755,root,qmail)                   %{_prefix}/bin/multitail
%attr(755,root,qmail)                   %{_prefix}/bin/logselect
%attr(755,root,qmail)                   %{_prefix}/bin/qlogselect
%attr(755,root,qmail)                   %{_prefix}/sbin/svscan
%attr(755,root,qmail)                   %{_prefix}/sbin/readproctitle

# ucspi-tcp
%attr(755,root,qmail)                   %{_prefix}/bin/greydaemon
%attr(755,root,qmail)                   %{_prefix}/bin/qmail-greyd
%attr(755,root,root)                    %{_prefix}/bin/mconnect-io
%attr(755,root,root)                    %{_prefix}/bin/rblsmtpd
%attr(755,root,root)                    %{_prefix}/bin/tcprulescheck
%attr(755,root,root)                    %{_prefix}/bin/tcpcat
%attr(755,root,root)                    %{_prefix}/bin/date@
%attr(755,root,root)                    %{_prefix}/bin/who@
%attr(755,root,root)                    %{_prefix}/bin/tcpclient
%attr(755,root,root)                    %{_prefix}/bin/tcpserver
%attr(755,root,root)                    %{_prefix}/bin/mconnect
%attr(755,root,root)                    %{_prefix}/bin/finger@
%attr(755,root,root)                    %{_prefix}/bin/http@
%attr(755,root,root)                    %{_prefix}/bin/tcprules
%attr(755,root,qmail)                   %{_prefix}/bin/udpclient
%attr(755,root,qmail)                   %{_prefix}/bin/udplogger
%endif

%if %nolibdkim == 0
%attr(555,root,root)                    %{_prefix}/bin/dkim
%endif

%if %nolibsrs2 == 0
%attr(555,root,root)                    %{_prefix}/bin/srs
%endif
%attr(755,root,qmail)                   %{shareddir}/boot/binm2
%attr(755,root,qmail)                   %{shareddir}/boot/proc+df
%attr(755,root,qmail)                   %{shareddir}/boot/binm1+df
%attr(755,root,qmail)                   %{shareddir}/boot/binm3+df
%attr(755,root,qmail)                   %{shareddir}/boot/home+df
%attr(755,root,qmail)                   %{shareddir}/boot/binm3
%attr(755,root,qmail)                   %{shareddir}/boot/binm1
%attr(755,root,qmail)                   %{shareddir}/boot/proc
%attr(755,root,qmail)                   %{shareddir}/boot/home
%attr(755,root,qmail)                   %{shareddir}/boot/binm2+df
%attr(444,root,qmail)                   %{shareddir}/boot/upstart
%attr(444,root,qmail)                   %{shareddir}/boot/systemd

%attr(755,root,qmail)                   %{plugindir}/generic.so
%attr(755,root,qmail)                   %{plugindir}/smtpd-plugin.so
%attr(755,root,qmail)                   %{plugindir}/smtpd-plugin0.so
%if %tcpserver_plugin != 0
%attr(755,root,qmail)                   %{plugindir}/qmail_smtpd.so
%attr(755,root,qmail)                   %{plugindir}/rblsmtpd.so
%endif

%license %attr(444,root,qmail)          %{shareddir}/doc/COPYING
%license %attr(444,root,qmail)          %{shareddir}/doc/README.licenses
%attr(444,root,qmail)                   %{shareddir}/doc/CREDITS
%attr(444,root,qmail)                   %{shareddir}/doc/FROMISP
%attr(444,root,qmail)                   %{shareddir}/doc/TOISP
%attr(444,root,qmail)                   %{shareddir}/doc/AUTOTURN
%attr(444,root,qmail)                   %{shareddir}/doc/INTERNALS
%attr(444,root,qmail)                   %{shareddir}/doc/EXTTODO
%attr(444,root,qmail)                   %{shareddir}/doc/README.qmail
%attr(444,root,qmail)                   %{shareddir}/doc/README.clamav
%attr(444,root,qmail)                   %{shareddir}/doc/README.greylist
%attr(444,root,qmail)                   %{shareddir}/doc/README.filters
%attr(444,root,qmail)                   %{shareddir}/doc/README.moreipme
%attr(444,root,qmail)                   %{shareddir}/doc/README.recipients
%if %fedorareview == 0
%attr(444,root,qmail)                   %{shareddir}/doc/README.logselect
%endif
%attr(444,root,qmail)                   %{shareddir}/doc/README.srs
%attr(444,root,qmail)                   %{shareddir}/doc/README.surbl
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.local2alias
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.local2local
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.nullclient
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.rem2local
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.relaybad
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.local2ext
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.local2virt
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.relaygood
%attr(444,root,qmail)                   %{shareddir}/doc/PIC.local2rem

%if %noperms == 0
%if 0%{?suse_version} >= 1120
%verify (not user group mode) %attr(6551, qscand, qmail)   %{_prefix}/sbin/qhpsi
%verify (not user group mode) %attr(2551, root, qscand)    %{_prefix}/sbin/run-cleanq
%verify (not user group mode) %attr(6551, qmailq, qmail)   %{_prefix}/sbin/qmail-queue
%verify (not user group mode) %attr(4555, qscand, qscand)  %{_prefix}/sbin/qscanq
%verify (not user group mode) %attr(2555, alias, qmail)    %{qmaildir}/alias
%verify (not user group mode) %attr(2755, indimail, qmail) %{qmaildir}/autoturn
%endif
%endif

# Shared libraries (omit for architectures that don't support them)


%if %nolibdkim == 0
%{_libdir}/libdkim-%{libdkim_version}.so.0
%{_libdir}/libdkim-%{libdkim_version}.so.0.0.0
%endif

%if %nolibsrs2 == 0
%{_libdir}/libsrs2-%{libsrs2_version}.so.0
%{_libdir}/libsrs2-%{libsrs2_version}.so.0.0.0
%endif

# a copy of /var/qmail/control/me, /var/qmail/control/defaultdomain,
# and /var/qmail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and
# this host's name in /var/qmail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision

%files -n daemontools
%defattr(-, root, root,-)
#
# Directories
#
%dir %attr(555,root,qmail)              %{shareddir}/doc
%if "%{mandir}" != "/usr/share/man"
%dir %attr(755,root,root)               %{mandir}
%dir %attr(755,root,root)               %{mandir}/man1
%dir %attr(755,root,root)               %{mandir}/man8
%endif

%attr(555,root,qmail)                   %{_prefix}/bin/envdir
%attr(555,root,qmail)                   %{_prefix}/bin/envuidgid
%attr(555,root,qmail)                   %{_prefix}/bin/fghack
%attr(555,root,qmail)                   %{_prefix}/bin/multilog
%attr(555,root,qmail)                   %{_prefix}/bin/pgrphack
%attr(555,root,qmail)                   %{_prefix}/bin/setlock
%attr(555,root,qmail)                   %{_prefix}/bin/setuidgid
%attr(555,root,qmail)                   %{_prefix}/bin/softlimit
%attr(555,root,qmail)                   %{_prefix}/bin/supervise
%attr(555,root,qmail)                   %{_prefix}/bin/svc
%attr(555,root,qmail)                   %{_prefix}/bin/svok
%attr(555,root,qmail)                   %{_prefix}/bin/svstat
%attr(555,root,qmail)                   %{_prefix}/bin/tai64n
%attr(555,root,qmail)                   %{_prefix}/bin/tai64nlocal
%attr(555,root,qmail)                   %{_prefix}/bin/tai64nunix
%attr(555,root,qmail)                   %{_prefix}/bin/tai2tai64n
%attr(555,root,qmail)                   %{_prefix}/bin/tai64n2tai
%attr(555,root,qmail)                   %{_prefix}/bin/spipe
%attr(555,root,qmail)                   %{_prefix}/bin/qfilelog
%attr(555,root,qmail)                   %{_prefix}/bin/multipipe
%attr(555,root,qmail)                   %{_prefix}/bin/teepipe
%attr(555,root,qmail)                   %{_prefix}/bin/multitail
%attr(555,root,qmail)                   %{_prefix}/bin/logselect
%attr(555,root,qmail)                   %{_prefix}/bin/qlogselect
%attr(555,root,qmail)                   %{_prefix}/sbin/svscan
%attr(555,root,qmail)                   %{_prefix}/sbin/readproctitle

%attr(555,root,qmail)                   %{libexecdir}/svscanboot

%if %fedorareview == 0
%docdir %{mandir}
%endif
%docdir %{shareddir}/doc
%attr(0644,root,qmail)                  %{shareddir}/doc/README.logselect
%attr(0644,root,qmail)                  %{mandir}/man1/spipe.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/qfilelog.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/multipipe.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/teepipe.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/multitail.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/qlogselect.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/tai2tai64n.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/tai64n2tai.1.gz
%attr(0644,root,qmail)                  %{mandir}/man8/envdir.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/envuidgid.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/fghack.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/multilog.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/pgrphack.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/readproctitle.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/setlock.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/setuidgid.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/softlimit.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/supervise.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/svc.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/svok.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/svscan.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/svstat.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/tai64n.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/tai64nlocal.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/tai64nunix.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/logselect.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/svscanboot.8.gz

%files -n ucspi-tcp
%attr(555,root,qmail)                   %{_prefix}/bin/tcpserver
%attr(555,root,qmail)                   %{_prefix}/bin/tcprules
%attr(555,root,qmail)                   %{_prefix}/bin/tcprulescheck
%attr(555,root,qmail)                   %{_prefix}/bin/tcpclient
%attr(555,root,qmail)                   %{_prefix}/bin/who@
%attr(555,root,qmail)                   %{_prefix}/bin/date@
%attr(555,root,qmail)                   %{_prefix}/bin/finger@
%attr(555,root,qmail)                   %{_prefix}/bin/http@
%attr(555,root,qmail)                   %{_prefix}/bin/tcpcat
%attr(555,root,qmail)                   %{_prefix}/bin/mconnect
%attr(555,root,qmail)                   %{_prefix}/bin/mconnect-io
%attr(555,root,qmail)                   %{_prefix}/bin/rblsmtpd
%attr(555,root,qmail)                   %{_prefix}/bin/udpclient
%attr(555,root,qmail)                   %{_prefix}/bin/udplogger
%attr(555,root,qmail)                   %{_prefix}/bin/greydaemon
%attr(555,root,qmail)                   %{_prefix}/bin/qmail-greyd

%if "%{mandir}" != "/usr/share/man"
%dir %attr(755,root,root)         %{mandir}
%dir %attr(755,root,root)         %{mandir}/man1
%dir %attr(755,root,root)         %{mandir}/man8
%dir %attr(755,root,root)         %{mandir}/cat1
%dir %attr(755,root,root)         %{mandir}/cat8
%else
%dir %attr(755,root,root)         %{mandir}/cat1
%dir %attr(755,root,root)         %{mandir}/cat8
%endif

%if %fedorareview == 0
%docdir %{mandir}
%endif
%attr(0644,root,qmail)                  %{mandir}/man1/tcpserver.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/tcprules.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/tcprulescheck.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/tcpclient.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/who@.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/date@.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/finger@.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/http@.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/tcpcat.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/mconnect.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/mconnect-io.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/rblsmtpd.1.gz
%attr(0644,root,qmail)                  %{mandir}/man1/udpclient.1.gz
%attr(0644,root,qmail)                  %{mandir}/man8/udplogger.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/greydaemon.8.gz
%attr(0644,root,qmail)                  %{mandir}/man8/qmail-greyd.8.gz
%attr(0644,root,qmail)                  %{mandir}/cat8/greydaemon.0.gz
%attr(0644,root,qmail)                  %{mandir}/cat8/qmail-greyd.0.gz

%clean
[ "%{buildroot}" != "/" ] && %{__rm} -fr %{buildroot}

#            install   erase   upgrade  reinstall
# pretrans      0        -         0
# pre           1        -         2         2
# post          1        -         2         2
# preun         -        0         1         -
# postun        -        0         1         -
# posttrans     0        -         0
# The scriptlets in %%pre and %%post are respectively run before and after a package is installed.
# The scriptlets %%preun and %%postun are run before and after a package is uninstalled.
# The scriptlets %%pretrans and %%posttrans are run at start and end of a transaction.
# On upgrade, the scripts are run in the following order:
#
#   1. pretrans of new package
#   2. pre of new package
#   3. (package install)
#   4. post of new package
#   5. preun of old package
#   6. (removal of old package)
#   7. postun of old package
#   8. posttrans of new package 

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
%verify_permissions -e %{_prefix}/sbin/qhpsi
%verify_permissions -e %{_prefix}/sbin/run-cleanq
%verify_permissions -e %{_prefix}/sbin/qmail-queue
%verify_permissions -e %{_prefix}/sbin/qscanq
%verify_permissions -e %{_prefix}/alias
%verify_permissions -e %{_prefix}/autoturn
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
  if [ -x /etc/init.d/indimail ] ; then
    %{_prefix}/sbin/initsvc -status || /etc/init.d/indimail stop || true
  else
    %{_prefix}/sbin/initsvc -status || true
  fi
  %{_prefix}/sbin/initsvc -off || true
else
  if [ -x /etc/init.d/indimail ] ; then
    /etc/init.d/indimail stop
  fi
  /bin/grep "^SV:" /etc/inittab |/bin/grep svscan |/bin/grep respawn >/dev/null
  if [ $? -eq 0 ] ; then
    /bin/grep -v "svscan" /etc/inittab > /etc/inittab.svctool.$$
    if [ $? -eq 0 ] ; then
      %{__mv} /etc/inittab.svctool.$$ /etc/inittab
      /sbin/init q
    else
      %{__rm} -f /etc/inittab.svctool.$$
    fi
  fi
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
/usr/bin/getent group %groupname  > /dev/null || /usr/sbin/groupadd -r -g %gid %groupname || true
if [ $? = 4 ] ; then
  /usr/sbin/groupadd %groupname
fi
/usr/bin/getent group nofiles   > /dev/null || /usr/sbin/groupadd nofiles  || true
/usr/bin/getent group qmail     > /dev/null || /usr/sbin/groupadd qmail    || true
/usr/bin/getent group qscand    > /dev/null || /usr/sbin/groupadd qscand   || true

/usr/bin/getent passwd %username > /dev/null || /usr/sbin/useradd -r -g %groupname -u %uid -d %{qmaildir} %username || true
if [ $? = 4 ] ; then
  /usr/sbin/useradd -r -g %groupname -d %{qmaildir} %username
fi
/usr/bin/getent passwd alias    > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{qmaildir}/alias  -s /sbin/nologin alias  || true
/usr/bin/getent passwd qmaild   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{qmaildir}        -s /sbin/nologin qmaild || true
/usr/bin/getent passwd qmaill   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{qmaildir}        -s /sbin/nologin qmaill || true
/usr/bin/getent passwd qmailp   > /dev/null || /usr/sbin/useradd -M -g nofiles  -d %{qmaildir}        -s /sbin/nologin qmailp || true
/usr/bin/getent passwd qmailq   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{qmaildir}        -s /sbin/nologin qmailq || true
/usr/bin/getent passwd qmailr   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{qmaildir}        -s /sbin/nologin qmailr || true
/usr/bin/getent passwd qmails   > /dev/null || /usr/sbin/useradd -M -g qmail    -d %{qmaildir}        -s /sbin/nologin qmails || true
/usr/bin/getent passwd qscand   > /dev/null || /usr/sbin/useradd -M -g qscand   -d %{qmaildir}/qscanq -G qmail,qscand -s /sbin/nologin qscand || true

for i in %username alias qmaild qmaill qmailp qmailq qmailr qmails qscand
do
  %{__rm} -f /var/spool/mail/$i
done
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
  if [ -f %{shareddir}/boot/rpm.init ] ; then
    /sbin/ldconfig
    echo "Running Custom Upgrade Script for post"
    /bin/sh %{shareddir}/boot/rpm.init upgrade
  fi
  echo "doing post upgrade activities"
  (
  # selinux
  %{_prefix}/sbin/svctool --servicedir=%{servicedir} --config=selinux
  ) >> /tmp/indimail-mta-install.log 2>&1
  exit 0
fi
if [ -x /bin/touch ] ; then
  TOUCH=/bin/touch
elif [ -x /usr/bin/touch ] ; then
  TOUCH=/usr/bin/touch
else
  TOUCH=/bin/touch
fi

echo "Doing post installation activities for the following"
echo ""
echo " 1. Configure %{logdir} for multilog"
echo " 2. Configure %{servicedir}. Move existing services to %{servicedir}.org"
echo " 3. Configure svscanlog service"
echo " 4. Configure standard catch-all accounts, default qmail configuration"
echo " 5. Configure DKIM, Domainkeys signature"
echo " 6. Configure QHPSI for inline virus scanning"
echo " 7. Configure SMTP services on port in 465 25 587"
echo " 8. Configure default queue configuration for sendmail, qmail-inject"
echo " 9. Configure ODMR service"
echo "10. Configure greylisting service"
echo "11. Configure QMTP service"
echo "12. Configure QMQP service"
echo "13. Configure udplogger service"
echo "14. Configure qscanq/clamd/freshclam service"
echo "15. Configure %{qsysconfdir}/control/signatures"
echo "16. Configure tcprules database for SMTP, QMTP, QMQP"
echo "17. Configure indimail-mta startup"
echo ""

(
echo "Creating %{logdir}"
if [ ! -d %{logdir} ] ; then
  %{__mkdir_p} %{logdir}
fi
%{__chown} -R qmaill:nofiles %{logdir}

if [ -d /service ] ; then
  echo "UFO found in /service. Moving it to /service.org"
  %{__mv} -f %{servicedir} %{servicedir}.org
fi

# svscanlog service
%{_prefix}/sbin/svctool --svscanlog --servicedir=%{servicedir}

%{_prefix}/sbin/svctool --config=qmail --postmaster=%{qmaildir}/alias/Maildir/ \
  --default-domain=indimail.org

if [ -f %{shareddir}/boot/rpm.init ] ; then
  echo "Running Custom Installation Script for post"
  /bin/sh %{shareddir}/boot/rpm.init post
fi

if [ %nodksignatures -eq 0 ] ; then
  if [ -x %{_prefix}/bin/dknewkey ] ; then
    ver_opt="both"
    sign_opt="both"
    if [ " $key_bit" = " " ] ; then
      key_bit=1024
    fi
    %{_prefix}/bin/dknewkey %{qsysconfdir}/control/domainkeys/%{dkimkeyfn} $key_bit
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
%global smtp_soft_mem 104857600
%global qmtp_soft_mem 104857600
%global qmqp_soft_mem 104857600
%else
%global smtp_soft_mem 52428800
%global qmtp_soft_mem 52428800
%global qmqp_soft_mem 52428800
%endif

# Define QHPSI for inline virus scanning by qmail-queue
if [ %{noclamav} -eq 0 ] ; then
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

# SMTP ports
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
%if %{tcpserver_plugin} == 1
    extra_opt="$extra_opt --shared-objects=1"
%endif
  if [ %{noclamav} -eq 0 -o $clamav_os -eq 1 ] ; then
    %{_prefix}/sbin/svctool --smtp=$port --servicedir=%{servicedir} \
      --qbase=%{qbase} --qcount=%{qcount} --qstart=1 \
      --query-cache --dnscheck --password-cache \
      --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 --persistdb \
      --starttls --fsync --syncdir --memory=%{smtp_soft_mem} --chkrecipient --chkrelay --masquerade \
      --min-free=52428800 --content-filter \
      --qhpsi="$qhpsi" \
      --dmasquerade \
      --dkverify=both \
      --dksign=both --private_key=%{qsysconfdir}/control/domainkeys/%/%{dkimkeyfn} \
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
      --dksign=both --private_key=%{qsysconfdir}/control/domainkeys/%/%{dkimkeyfn} \
      $extra_opt
  fi
  echo "1" > %{servicedir}/qmail-smtpd.$port/variables/DISABLE_PLUGIN
done

# queue parameters in control/defaultqueue for qmail-inject, sendmail
if [ %{noclamav} -eq 0 -o $clamav_os -eq 1 ] ; then
  %{_prefix}/sbin/svctool --queueParam=defaultqueue \
    --qbase=%{qbase} --qcount=%{qcount} --qstart=1 \
    --min-free=52428800 --fsync --syncdir \
    --qhpsi="$qhpsi" \
    --dkverify="none" --dksign=$sign_opt \
    --private_key=%{qsysconfdir}/control/domainkeys/%/%{dkimkeyfn} \
    $extra_opt
else
  %{_prefix}/sbin/svctool --queueParam=defaultqueue \
    --qbase=%{qbase} --qcount=%{qcount} --qstart=1 \
    --min-free=52428800 --fsync --syncdir --virus-filter \
    --dkverify="none" --dksign=$sign_opt \
    --private_key=%{qsysconfdir}/control/domainkeys/%/%{dkimkeyfn} \
    $extra_opt
fi

# ODMR service
%{_prefix}/sbin/svctool --smtp=366 --odmr --servicedir=%{servicedir} \
  --query-cache --password-cache --memory=%{smtp_soft_mem}
echo "1" > %{servicedir}/qmail-smtpd.366/variables/DISABLE_PLUGIN

# Greylist daemon
%{_prefix}/sbin/svctool --greylist=1999 --servicedir=%{servicedir} --min-resend-min=2 \
    --resend-win-hr=24 --timeout-days=30 --context-file=greylist.context \
    --hash-size=65536 --save-interval=5 --whitelist=greylist.white
# qmail-qmtpd service
%{_prefix}/sbin/svctool --qmtp=209 --servicedir=%{servicedir} --qbase=%{qbase} \
  --qcount=%{qcount} --qstart=1 --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
  --fsync --syncdir --memory=%{qmtp_soft_mem} --min-free=52428800
# qmail-qmqpd service
%{_prefix}/sbin/svctool --qmqp=628 --servicedir=%{servicedir} --qbase=%{qbase} \
  --qcount=%{qcount} --qstart=1 --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
  --fsync --syncdir --memory=%{qmqp_soft_mem} --min-free=52428800
$TOUCH %{servicedir}/qmail-qmqpd.628/down

# virus/spam filtering
%{_prefix}/sbin/svctool --qscanq --servicedir=%{servicedir} --scanint=200
if [ %{noclamav} -eq 0 -o $clamav_os -eq 1 ] ; then
  %{_prefix}/sbin/svctool --config=clamd
  # create clamd, freshclam service
  %{_prefix}/sbin/svctool --clamd --servicedir=%{servicedir} --clamdPrefix=$clamdPrefix \
    --sysconfdir=$mysysconfdir
  if [ $clamav_os -eq 1 ] ; then
    echo "Checking if clamd/freshclam is running"
    count=`ps -e|grep clamd|wc -l`
    if [ $count -gt 0 ] ; then
      echo "Disabling clamd service"
      $TOUCH %{servicedir}/clamd/down
    fi
    count=`ps -e|grep freshclam|wc -l`
    if [ $count -gt 0 ] ; then
      echo "Disabling freshclam service"
      $TOUCH %{servicedir}/freshclam/down
    fi
  fi
fi

# udplogger service
%{_prefix}/sbin/svctool --udplogger=3000 --localip=0 --timeout=10 --servicedir=%{servicedir}

%{__cat} <<EOF > %{qsysconfdir}/control/signatures
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
%{__chown} qscand:qscand %{qsysconfdir}/control/signatures

# Recreate ld.so links and cache
if [ ! " %{_libdir}" = " /usr/lib" -a ! " ${_libdir}" = " /usr/lib64" ] ; then
if [ -d %{_sysconfdir}/ld.so.conf.d ] ; then
  (
    echo %{_libdir}
  ) > %{_sysconfdir}/ld.so.conf.d/indimail-%{_arch}.conf
fi
/sbin/ldconfig
fi

# rebuild cdb
for i in smtp qmtp qmqp
do
  for j in `/bin/ls %{qsysconfdir}/tcp*.$i 2>/dev/null`
  do
    echo "Creating CDB $j.cdb"
       %{_prefix}/bin/tcprules $j.cdb $j.tmp < $j && /bin/chmod 664 $j.cdb \
      && chown indimail:indimail $j.cdb
  done
done

# selinux
%{_prefix}/sbin/svctool --servicedir=/service --config=selinux
#
# Install IndiMail to be started on system boot
# The add-boot command installs svscan to be started by init, systemd, upstart or launchctl
#
echo "adding indimail startup"
%{_prefix}/sbin/svctool --config=add-boot
) >> /tmp/indimail-mta.install.log 2>&1

if [ -x /bin/systemctl ] ; then
  /bin/systemctl enable indimail.service
fi
if [ -f %{_sysconfdir}/init/svscan.conf -o -f %{_sysconfdir}/event.d/svscan ] ; then
  echo "1. Issue /sbin/initctl emit qmailstart to start services"
  count=1
elif [ -f %{_sysconfdir}/systemd/system/multi-user.target.wants/indimail.service ] ; then
  echo "1. Issue /bin/systemctl start indimail.service to start services"
  count=1
else
  if [ -f %{_prefix}/sbin/initsvc ] ; then
  echo "1. Issue %{_prefix}/sbin/initsvc -on"
  else
  echo "1. Issue /etc/init.d/indimail start"
  fi
  echo "2. Issue /sbin/init q to start services"
  count=2
fi
count=`expr $count + 1`
echo "$count. Change your default domain in %{qsysconfdir}/control/defaultdomain"
count=`expr $count + 1`
echo "$count. You can optionally run the following command to verify installation"
echo "   sudo rpm -V indimail"
if [ ! -f %{qsysconfdir}/control/servercert.pem ] ; then
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
(
echo "Giving IndiMail exactly 5 seconds to exit nicely"
if test -f %{_sysconfdir}/init/svscan.conf
then
  /sbin/initctl emit qmailstop > /dev/null 2>&1
  if [ $argv1 -ne 1 ] ; then # not an upgrade
    %{__rm} -f %{_sysconfdir}/init/svscan.conf
  fi
elif test -f %{_sysconfdir}/event.d/svscan
then
  /sbin/initctl emit qmailstop > /dev/null 2>&1
  if [ $argv1 -ne 1 ] ; then # not an upgrade
    %{__rm} -f %{_sysconfdir}/event.d/svscan
  fi
elif test -f %{_sysconfdir}/systemd/system/multi-user.target.wants/indimail.service
then
  /bin/systemctl stop indimail.service > /dev/null 2>&1
elif test -x %{_prefix}/sbin/initsvc
then
  %{_prefix}/sbin/initsvc -status || /etc/init.d/indimail stop || true
  %{_prefix}/sbin/initsvc -off || true
else
  /etc/init.d/indimail stop || true
    /bin/grep "^sv:" /etc/inittab |/bin/grep svscan |/bin/grep respawn >/dev/null
  if [ $? -eq 0 ] ; then
    /bin/grep -v "svscan" /etc/inittab > /etc/inittab.svctool.$$
    if [ $? -eq 0 ] ; then
      /bin/mv /etc/inittab.svctool.$$ /etc/inittab
      /sbin/init q
    else
      %{__rm} -f /etc/inittab.svctool.$$
    fi
  fi
fi
sleep 5
if [ -f %{shareddir}/boot/rpm.init ] ; then
  echo "Running Custom Un-Installation Script for preun"
  /bin/sh %{shareddir}/boot/rpm.init preun "$argv1"
fi

# we are doing upgrade
if [ $argv1 -eq 1 ] ; then
  exit 0
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
# Remove IndiMail being started on system boot
echo "removing indimail startup"
%{_prefix}/sbin/svctool --config=rm-boot
) > /tmp/indimail-mta.uninstall.log 2>&1

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
(
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

if [ %{_prefix} = "/var/indimail" -o %{_prefix} = "/var/qmail" ] ; then
  echo "removing binaries, libraries, queues, man pages"
  for i in %{_prefix}/bin %{_prefix}/sbin %{_prefix}/lib %{qmaildir}/queue \
    %{mandir}/man1 %{mandir}/cat1 %{mandir}/man5 %{mandir}/cat5 \
    %{mandir}/man7 %{mandir}/cat7 %{mandir}/man8 %{mandir}/cat8
  do
    %{__rm} -rf $i || true
  done
  /bin/rmdir --ignore-fail-on-non-empty %{mandir} 2>/dev/null
  for i in `/bin/ls %{shareddir} 2>/dev/null`
  do
    %{__rm} -rf %{shareddir}/$i || true
  done
else
  for i in bin sbin control users
  do
    echo "removing link $i"
    %{__rm} -f %{qmaildir}/$i || true
  done
  # queue
  %{__rm} -rf %{qmaildir}/queue || true
  # modules
  %{__rm} -rf %{_prefix}/lib/indimail || true
  # shareddir
  if [ ! " %{shareddir}" = " /usr/share" ] ; then
    for i in `/bin/ls %{shareddir} 2>/dev/null`
    do
      %{__rm} -rf %{shareddir}/$i || true
    done
  fi
  # libexecdir
  if [ ! " %{libexecdir}" = " /usr/libexec" ] ; then
    %{__rm} -rf %{libexecdir} || true
  fi
fi
if [ -d %{mandir} ] ; then
  echo "removing man pages"
  /bin/rmdir --ignore-fail-on-non-empty %{mandir} 2>/dev/null
fi
if [ -d %{shareddir} ] ; then
  echo "removing architecture-independent shared directory"
  /bin/rmdir --ignore-fail-on-non-empty %{shareddir} 2>/dev/null
fi

echo "removing configuration"
for i in smtp qmtp qmqp
do
  for j in `/bin/ls %{qsysconfdir}/tcp*.$i 2>/dev/null`
  do
      %{__rm} -f $j.cdb
  done
done
%{__rm} -f %{qsysconfdir}/indimail-mta.te %{qsysconfdir}/indimail-mta.mod %{qsysconfdir}/indimail-mta.pp
/bin/rmdir --ignore-fail-on-non-empty %{qsysconfdir} 2>/dev/null
for i in assign cdb
do
    %{__rm} -f %{qsysconfdir}/users/$i
done
/bin/rmdir --ignore-fail-on-non-empty %{qsysconfdir}/users 2>/dev/null

if [ -f %{qsysconfdir}/controlfiles ] ; then
  for i in `%{__cat} %{qsysconfdir}/controlfiles`
  do
    %{__rm} -f %{qsysconfdir}/control/$i
  done
else
  for i in databytes defaultdelivery defaultdomain localiphost locals \
    me nodnscheck plusdomain queue_base smtpgreeting signatures \
    chkrcptdomains defaulthost envnoathost filterargs greylist.white \
    hostip timeoutremote timeoutsmtpd
  do
    %{__rm} -f %{qsysconfdir}/control/$i
  done
fi
%{__rm} -f %{qsysconfdir}/controlfiles
%{__rm} -f %{qsysconfdir}/control/domainkeys/%{dkimkeyfn}.pub %{qsysconfdir}/control/domainkeys/%dkimkeyfn
/bin/rmdir --ignore-fail-on-non-empty %{qsysconfdir}/control/domainkeys 2>/dev/null
/bin/rmdir --ignore-fail-on-non-empty %{qsysconfdir}/control 2>/dev/null

for i in postmaster mailer-daemon root ham spam register-ham register-spam
do
  %{__rm} -f %{qmaildir}/alias/.qmail-"$i"
done
/bin/rmdir --ignore-fail-on-non-empty %{qmaildir}/alias 2>/dev/null

echo "removing startup services"
if [ %{noclamav} -eq 0 ] ; then
  clamav_os=0
else
  if [ -f /usr/sbin/clamd -a -f /usr/bin/clamdscan ] ; then
    clamav_os=1
  else
    clamav_os=0
  fi
fi
if [ %{noclamav} -eq 0 -o $clamav_os -eq 1 ] ; then
for i in clamd freshclam
do
  if [ -d %{servicedir}/$i ] ; then
    %{__rm} -rf %{servicedir}/$i || true
  fi
done
fi
for i in qmail-send.25 qmail-smtpd.25 qmail-smtpd.366 \
qmail-spamlog qscanq qmail-smtpd.465 qmail-smtpd.587 qmail-qmtpd.209 \
qmail-qmqpd.628 greylist.1999 udplogger.3000 .svscan
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
if [ -x /usr/sbin/selinuxenabled ] ; then
  /usr/sbin/selinuxenabled
  if [ $? -eq 0 -a -x /usr/sbin/semodule ] ; then
	echo "disabling selinux module"
    /usr/sbin/semodule -r indimail-mta
  fi
fi
if [ -f %{shareddir}/boot/rpm.init ] ; then
  echo "Running Custom Un-Installation Script for postun"
  /bin/sh %{shareddir}/boot/rpm.init postun
fi
) >> /tmp/indimail-mta.uninstall.log 2>&1

### SCRIPTLET ###############################################################################
%posttrans
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
if [ -f %{shareddir}/boot/rpm.init ] ; then
  echo "Running Custom Installation Script for posttrans"
  /bin/sh %{shareddir}/boot/rpm.init posttrans
fi
echo ""

# fix changelog for openSUSE buildservice
%changelog
* Wed May 25 2016 mbhangui@gmail.com %{version}-%{release}
Release 1.9.2 Start 13/08/2015
1.  Added leapsecs program, leapsecs.dat
2.  Added yearcal, nowutc programs
3.  prefix queue name in qmail-send, qmail-todo logs
4.  BUG - qmail-queue.c: fixed missing null character in extraqueue when EXTRAQUEUE is defined
5.  dkim.c - Use basename of private key as the selector in absense of -y option
6.  spawn.c - add environment variable MESSID (queue filename)
7.  str_end.c - added str_end() function
8.  qmail-send.c, qsutil.c, sig.c, qmail-daemon.c - flush qmail-send logs only on line completion
9.  various fixes for debian packaging (lib64, devel, libindimail)
10. renamed package indimail-shared to libindimail
11. hier.c - create ratelimit directory
12. qmail-greyd.c - added missing flush statement to flush logs
13. greylist.c - fixed IPV6 address for connect, fixed incorrect formating of context filename, fixed buffer overflow with ip_str
14. ip.c - fixed mapped ipv4 address in ip6_scan()
15. smtpd.c - pass ip6 address to greylist()
16. ipv6 version of udplogger
17. greylist.c - create ipv4 socket if ipv6 stack is disabled (reported by Andrzej Boreczko)
18. qmail-greyd.c - set noipv6 if LIBC_HAS_IP6 is not defined
19. fixed debian mysql config, database creation in postinst.
20. udpclient - added -r, -t option to allow reading responses from server
21. create udplogger service during rpm/deb installation
22. qmailctl.sh - BUG, replaced failure with $fail
23. qmail.spec, indimail.spec, indimail, indimail-mta postinst script fixes for debian8/ubuntu15 migration to systemd from upstart
24. docker images for indimail, indimail-mta
25. tcpserver: ip4_bit.c fix stack smashing - num defined as int instead of unsigned long
26. added tcpserver_plugin.c to dynamically load shared objects in tcpserver main()
27. use RTLD_NOLOAD in load_shared() to load shared objects only once
28. svctool, indimail.spec, qmail.spec, debain postinst scripts modified to load qmail_smtpd.so instead of qmail-smtpd
29. smtpd.c - smtp_init() function to load all control files only once
30. Massive FHS changes
31. qmail-dk.c - BUG - removed extra semicolon after if () statement
32. tcpopen.c - close socket on connect() failure
