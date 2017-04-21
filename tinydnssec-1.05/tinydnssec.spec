#
#
# $Id: $
%undefine _missing_build_ids_terminate_build
%global _unpackaged_files_terminate_build 1
%global debug_package %{nil}

%if %{defined _project}
# define if building on openSUSE build service
%global build_on_obs       1
%global reconfigure_mode   0
%else
%define _project           local
%global build_on_obs       0
%global reconfigure_mode   0
%global _hardened_build    1
%endif
%if %build_on_obs == 1
%global packager Manvendra Bhangui <manvendra@indimail.org>
%endif
%global fedorareview       0

%if 0%{?suse_version}
%global dist suse
%global disttag suse
%endif

%if 0%{?fedora_version}
%global dist %{?dist}
%global disttag fedora
%endif

%global _sysconfdir /etc/indimail
%global _prefix            /usr
Name: tinydnssec
Version: 1.05
%if %fedorareview == 0
%if %build_on_obs == 1
Release: 1.<B_CNT>
%else
Release: 1.1%{?dist}
%endif
%else
Release: 1.1%{?dist}
%endif
Summary: DNS suite
%if %{undefined suse_version} && %{undefined sles_version}
Group: System Environment/Base
%else
Group: Productivity/Networking/Email/Servers
%endif
%if %build_on_obs == 1
License: GPL-3.0+
%else
License: GPLv3
%endif
URL: http://cr.yp.to/djbdns.html
Source0: %{name}-%{version}.tar.gz
# http://www.fefe.de/dns/
BuildRequires: perl
##################################### OBS ####################################
%if %build_on_obs == 1
Requires: /usr/sbin/useradd /usr/sbin/groupadd /usr/sbin/groupdel
BuildRequires: libidn-devel
%if 0%{?rhel_version} == 700
BuildRequires: groff-doc
%else
BuildRequires: groff
%endif

%if 0%{?suse_version}
BuildRequires: -post-build-checks  
#!BuildIgnore: post-build-checks  
%endif
%else
BuildRequires: groff
%endif
##############################################################################
Requires: daemontools
Provides: user(tinydns)  > 999
Provides: user(dnscache) > 999
Provides: user(dnslog)   > 999

%description
A collection of Domain Name System tools
This package includes software for all the fundamental DNS operations:

DNS cache: finding addresses of Internet hosts.  When a browser wants to
contact www.yahoo.com, it first asks a DNS cache, such as djbdns's
dnscache, to find the IP address of www.yahoo.com.  Internet service
providers run dnscache to find IP addresses requested by their customers.
If you're running a home computer or a workstation, you can run your own
dnscache to speed up your web browsing.

DNS server: publishing addresses of Internet hosts.  The IP address of
www.yahoo.com is published by Yahoo's DNS servers.  djbdns includes
a general-purpose DNS server, tinydns; network administrators run tinydns
to publish the IP addresses of their computers.  djbdns also includes
special-purpose servers for publishing DNS walls and RBLs.

DNS client: talking to a DNS cache.  djbdns includes a DNS client C
library and several command-line DNS client utilities.  Programmers use
these tools to send requests to DNS caches.

djbdns also includes several DNS debugging tools, notably dnstrace, which
administrators use to diagnose misconfigured remote servers.

See http://cr.yp.to/djbdns.html

%build
sed -i 's{/usr{%{_prefix}{' conf-home
%{__make} -s
pod2man -s 8 -c '' "tinydns-sign" >tinydns-sign.8

%install
%{__make} -s DESTDIR=%{buildroot} install-strip

%clean
[ "%{buildroot}" != "/" ] && %{__rm} -fr %{buildroot}

%files
%defattr(-,root,root,-)
%config(noreplace)             %{_sysconfdir}/dnsroots.global

%{_prefix}/bin/*

%%doc doc/COPYING.tinydnssec doc/README.tinydnssec doc/README-ipv6.tinydnssec doc/djbdnsFAQ.pdf doc/HOWTO doc/LifeWithdjbdns.pdf doc/README.dnstransmit.bug doc/Thedjbway_djbdns.pdf

%doc %{_mandir}/man1/*
%doc %{_mandir}/man5/*
%doc %{_mandir}/man8/*

%prep
%setup -q

%pre
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
# we are doing upgrade
if [ $argv1 -eq 2 ] ; then
  echo "Giving dnscache exactly 5 seconds to exit nicely" 1>&2
  /usr/bin/svc -dx %{_sysconfdir}/dnscache %{_sysconfdir}/dnscache/log
  sleep 5
  exit 0
fi
for i in dnscache dnslog tinydns
do
  %{__rm} -f /var/spool/mail/$i
done
/usr/bin/getent group  nofiles  > /dev/null || /usr/sbin/groupadd nofiles  || true
/usr/bin/getent passwd dnscache > /dev/null || /usr/sbin/useradd -l -M -g nofiles  -d %{_sysconfdir} -s /sbin/nologin dnscache || true
/usr/bin/getent passwd dnslog   > /dev/null || /usr/sbin/useradd -l -M -g nofiles  -d %{_sysconfdir} -s /sbin/nologin dnslog   || true
/usr/bin/getent passwd tinydns  > /dev/null || /usr/sbin/useradd -l -M -g nofiles  -d %{_sysconfdir} -s /sbin/nologin tinydns  || true

%preun
argv1=$1
echo "Giving tinydns/dnscache exactly 5 seconds to exit nicely" 1>&2
for i in tinydns dnscache
do
  /usr/bin/svc -dx %{_sysconfdir}/$i %{_sysconfdir}/$i/log >/dev/null 2>&1 || true
done
sleep 5
# we are doing upgrade
if [ $argv1 -eq 1 ] ; then
  exit 0
fi
for i in tinydns dnslog dnscache
do
  echo "Removing user $i"
  /usr/bin/getent passwd $i > /dev/null && /usr/sbin/userdel $i >/dev/null || true
  %{__rm} -f /var/spool/$i
done
for i in tinydns dnscache
do
  if [ -d /service/$i ] ; then
    echo "Removing service /service/$i"
    %{__rm} -f /service/$i
  fi
  if [ -d /service/$i ] ; then
    echo "Removing preinstalled %{_sysconfdir}/$i"
    %{__rm} -rf %{_sysconfdir}/$i
  fi
done

%post
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
if [ $argv1 -eq 2 ] ; then # upgrade
  echo "doing post upgrade activities"
  exit 0
fi
# created due to ghost definition in files section
/bin/rmdir --ignore-fail-on-non-empty %{_sysconfdir}/dnscache 2>/dev/null
/bin/rmdir --ignore-fail-on-non-empty %{_sysconfdir}/tinydns 2>/dev/null
if [ ! -d %{_sysconfdir}/dnscache ] ; then
  %{_prefix}/bin/dnscache-conf dnscache dnslog %{_sysconfdir}/dnscache 127.0.0.1
  if [ $? -eq 0 ] ; then
    ln -s %{_sysconfdir}/dnscache /service/dnscache
  fi
fi
