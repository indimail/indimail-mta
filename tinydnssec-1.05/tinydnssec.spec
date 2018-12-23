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
%global curvedns_version 0.88

%global _sysconfdir /etc/indimail
%global _prefix            /usr
Name: tinydnssec
Version: 1.05
Release: 1.1%{?dist}
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
Source0: http://downloads.sourceforge.net/indimail/%{name}-%{version}.tar.gz
# http://www.fefe.de/dns/
BuildRequires: perl
BuildRequires: libev-devel
%if 0%{?suse_version} < 1120
BuildRequires: elfutils
%endif
##################################### OBS ####################################
%if %build_on_obs == 1
%if 0%{?suse_version}
BuildRequires: -post-build-checks  
#!BuildIgnore: post-build-checks  
%endif
%endif
##############################################################################
%if %build_on_obs == 0
Requires(pre): shadow-utils
Requires(postun): shadow-utils
%else
Requires: /usr/sbin/useradd /usr/sbin/groupadd /usr/sbin/groupdel /usr/sbin/userdel
%endif
Requires: daemontools
Requires: group(nofiles)
Requires: user(qmaill)
Provides: user(Gtinydns)  > 999
Provides: user(Gdnscache) > 999

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

DQ: A package with DNS/DNSCurve related software.
It contains a recursive DNS server with DNSCurve support called
dqcache and also a commandline tool to debug DNS/DNScurve called dq.

See https://mojzis.com/software/dq/

CurveDNS: CurveDNS is the first publicly released forwarding implementation
that implements the DNSCurve protocol.  DNSCurve uses high-speed
high-security elliptic-curve cryptography to drastically improve every
dimension of DNS security.

See http://dnscurve.org/ for protocol details.

curvedns allows any authoritative DNS name server to act as a DNSCurve
capable one, without changing anything on your current DNS environment.
The only thing a DNS data manager (that is probably you) has to do is
to install CurveDNS on a machine, generate a keypair, and update NS
type records that were pointing towards your authoritative name server
and let them point to this machine running CurveDNS. Indeed, it is that
easy to become fully protected against almost any of the currently known
DNS flaws, such as active and passive cache poisoning.

CurveDNS supports:
* Forwarding of regular (non-protected) DNS packets;
* Unboxing of DNSCurve queries and forwarding the regular DNS packets
* Boxing of regular DNS responses to DNSCurve responses;
* Both DNSCurve's streamlined- and TXT-format;
* Caching of shared secrets;
* Both UDP and TCP;
* Both IPv4 and IPv6.

See http://curvedns.on2it.net/

%build
%{__sed} -i 's{/usr{%{_prefix}{' conf-home
%{__make} -s
pod2man -s 8 -c '' "tinydns-sign" >tinydns-sign.8
if [ -d dq-20161210 ] ; then
  cd dq-20161210
  %{__make} -s
  cd ..
fi
if [ -d curvedns-%{curvedns_version} ] ; then
  cd curvedns-%{curvedns_version}
  ./configure.nacl
  ./configure.curvedns
  %{__make} -s
  cd ..
fi

%install
%{__make} -s DESTDIR=%{buildroot} install-strip
if [ -d dq-20161210 ] ; then
  cd dq-20161210
  sh make-install.sh %{buildroot}
  cd ..
fi
if [ -d curvedns-%{curvedns_version} ] ; then
  cd curvedns-%{curvedns_version}
  %{__mkdir_p} %{buildroot}%{_prefix}/bin
  %{__mkdir_p} %{buildroot}%{_prefix}/sbin
  %{__mkdir_p} %{buildroot}%{_mandir}/man8
  %{__mkdir_p} %{buildroot}%{_sysconfdir}
  %{__install} -m 0755 curvedns %{buildroot}%{_prefix}/bin
  %{__install} -m 0755 curvedns-keygen %{buildroot}%{_prefix}/sbin
  %{__install} -m 0644 curvedns.8 curvedns-keygen.8 %{buildroot}%{_mandir}/man8
  %{__install} -m 0600 curvedns.private.key %{buildroot}%{_sysconfdir}/curvedns.private.key
  cd ..
fi

%clean
[ "%{buildroot}" != "/" ] && %{__rm} -fr %{buildroot}

%files
%defattr(-, root, root,-)
%attr(755,root,root)                   %{_prefix}/bin/dq
%attr(755,root,root)                   %{_prefix}/bin/axfr-get
%attr(755,root,root)                   %{_prefix}/bin/axfrdns
%attr(755,root,root)                   %{_prefix}/bin/dnscache
%attr(755,root,root)                   %{_prefix}/bin/dnsfilter
%attr(755,root,root)                   %{_prefix}/bin/dnsgetroot
%attr(755,root,root)                   %{_prefix}/bin/dnsip
%attr(755,root,root)                   %{_prefix}/bin/dnsip6
%attr(755,root,root)                   %{_prefix}/bin/dnsip6q
%attr(755,root,root)                   %{_prefix}/bin/dnsipq
%attr(755,root,root)                   %{_prefix}/bin/dnsmx
%attr(755,root,root)                   %{_prefix}/bin/dnsname
%attr(755,root,root)                   %{_prefix}/bin/dnsnamex
%attr(755,root,root)                   %{_prefix}/bin/dnsq
%attr(755,root,root)                   %{_prefix}/bin/dnsqr
%attr(755,root,root)                   %{_prefix}/bin/dnstrace
%attr(755,root,root)                   %{_prefix}/bin/dnstracesort
%attr(755,root,root)                   %{_prefix}/bin/dnstxt
%attr(755,root,root)                   %{_prefix}/bin/pickdns
%attr(755,root,root)                   %{_prefix}/bin/pickdns-data
%attr(755,root,root)                   %{_prefix}/bin/random-ip
%attr(755,root,root)                   %{_prefix}/bin/rbldns
%attr(755,root,root)                   %{_prefix}/bin/rbldns-data
%attr(755,root,root)                   %{_prefix}/bin/tinydns
%attr(755,root,root)                   %{_prefix}/bin/tinydns-data
%attr(755,root,root)                   %{_prefix}/bin/tinydns-edit
%attr(755,root,root)                   %{_prefix}/bin/tinydns-get
%attr(755,root,root)                   %{_prefix}/bin/tinydns-sign
%attr(755,root,root)                   %{_prefix}/bin/walldns
%attr(755,root,root)                   %{_prefix}/bin/curvedns
%attr(755,root,root)                   %{_prefix}/bin/dqcache
%attr(755,root,root)                   %{_prefix}/sbin/dqcache-makekey
%attr(755,root,root)                   %{_prefix}/sbin/curvedns-keygen
%attr(755,root,root)                   %{_prefix}/sbin/dqcache-start
%attr(755,root,root)                   %{_prefix}/sbin/axfrdns-conf
%attr(755,root,root)                   %{_prefix}/sbin/dnscache-conf
%attr(755,root,root)                   %{_prefix}/sbin/pickdns-conf
%attr(755,root,root)                   %{_prefix}/sbin/rbldns-conf
%attr(755,root,root)                   %{_prefix}/sbin/tinydns-conf
%attr(755,root,root)                   %{_prefix}/sbin/walldns-conf
%attr(755,root,root)                   %{_prefix}/sbin/dqcache-conf
%attr(755,root,root)                   %{_prefix}/sbin/curvedns-conf
%config(noreplace)                     %{_sysconfdir}/dnsroots.global
%config(noreplace)                     %{_sysconfdir}/curvedns.private.key

%doc dq-20161210/README.dq dq-20161210/INSTALL.dq doc/COPYING.tinydnssec doc/README.tinydnssec
%doc doc/README-ipv6.tinydnssec doc/djbdnsFAQ.pdf doc/HOWTO
%doc doc/LifeWithdjbdns.pdf doc/README.dnstransmit.bug doc/Thedjbway_djbdns.pdf
%doc curvedns-%{curvedns_version}/LICENSE.curvedns

%doc %{_mandir}/man1/*
%doc %{_mandir}/man5/*
%doc %{_mandir}/man7/*
%doc %{_mandir}/man8/*

%prep
%setup -q

%pretrans
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
if [ -f %{_prefix}/bin/svok ] ; then
  status=0
  for i in tinydns tinydns/log dnscache dnscache/log curvedns curvedns/log dqcache dqcache/log
  do
    %{_prefix}/bin/svok %{_sysconfdir}/$i >/dev/null 2>&1
    if [ $? -eq 0 ] ; then
      status=1
    fi
  done
  if [ $status -eq 1 ] ; then
    echo "Giving dns services exactly 5 seconds to exit nicely" 1>&2
    for i in tinydns dnscache curvedns dqcache
    do
      %{_prefix}/bin/svc -dx %{_sysconfdir}/$i %{_sysconfdir}/$i/log >/dev/null 2>&1
    done
    sleep 5
  fi
fi

%pre
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
if [ -z "$argv1" ] ; then
  argv1=0
fi
# we are doing upgrade
if [ $argv1 -eq 2 ] ; then
  exit 0
fi
for i in Gdnscache Gtinydns
do
  %{__rm} -f /var/spool/mail/$i
  /usr/bin/getent passwd $i > /dev/null || /usr/sbin/useradd -l -M -g nofiles  -d %{_sysconfdir} -s /sbin/nologin $i || true
done

%preun
argv1=$1
if [ -z "$argv1" ] ; then
  argv1=0
fi
# we are doing upgrade
if [ $argv1 -eq 1 ] ; then
  exit 0
fi
if [ -f %{_prefix}/bin/svok ] ; then
  status=0
  for i in tinydns tinydns/log dnscache dnscache/log curvedns curvedns/log dqcache dqcache/log
  do
    %{_prefix}/bin/svok %{_sysconfdir}/$i >/dev/null 2>&1
    if [ $? -eq 0 ] ; then
      status=1
    fi
  done
  if [ $status -eq 1 ] ; then
    echo "Giving dns services exactly 5 seconds to exit nicely" 1>&2
    for i in tinydns dnscache curvedns dqcache
    do
      %{_prefix}/bin/svc -dx %{_sysconfdir}/$i %{_sysconfdir}/$i/log >/dev/null 2>&1
    done
    sleep 5
  fi
fi

%post
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
if [ -z "$argv1" ] ; then
  argv1=0
fi
if [ $argv1 -eq 2 ] ; then # upgrade
  echo "doing post upgrade activities"
  exit 0
fi
# created due to ghost definition in files section
/bin/rmdir --ignore-fail-on-non-empty %{_sysconfdir}/dnscache 2>/dev/null
/bin/rmdir --ignore-fail-on-non-empty %{_sysconfdir}/tinydns 2>/dev/null
if [ ! -d %{_sysconfdir}/dnscache ] ; then
  %{_prefix}/sbin/dnscache-conf Gdnscache qmaill %{_sysconfdir}/dnscache 127.0.0.1
  if [ $? -eq 0 ] ; then
    if [ ! -h /service/dnscache ] ; then
      ln -s %{_sysconfdir}/dnscache /service/dnscache
    fi
  fi
else
  if [ ! -h /service/dnscache ] ; then
    ln -s %{_sysconfdir}/dnscache /service/dnscache
  fi
fi
for i in tinydns dqcache curvedns
do
  if [ " $i" = " dqcache" ] ; then
    acct=Gdnscache
  else
    acct=Gtinydns
  fi
  if [ " $i" = " curvedns" ] ; then
    ip=x.x.x.x
  else
    ip=127.0.0.1
  fi
  if [ ! -d %{_sysconfdir}/$i ] ; then
    %{_prefix}/sbin/$i-conf $acct qmaill %{_sysconfdir}/$i $ip
    if [ ! " $i" = " curvedns" ] ; then
      continue
    fi
    if [ -d %{_sysconfdir}/$i ] ; then
      mask=`umask`
      umask 077
      # Generate private key
      %{__sed} -n \$p %{_sysconfdir}/curvedns.private.key > %{_sysconfdir}/$i/curvedns.keygen
      /bin/sh %{_sysconfdir}/$i/curvedns.keygen && %{__rm} -f %{_sysconfdir}/$i/curvedns.keygen
      umask $mask
    fi
  fi
done

%postun
argv1=$1
ID=$(id -u)
if [ $ID -ne 0 ] ; then
  echo "You are not root" 1>&2
  exit 1
fi
if [ -z "$argv1" ] ; then
  argv1=0
fi
# we are doing upgrade
if [ $argv1 -eq 1 ] ; then
  exit 0
fi
for i in tinydns dnscache curvedns dqcache
do
  if [ -L /service/$i ] ; then
    echo "Removing service /service/$i"
    %{__rm} -f /service/$i
  fi
  if [ -d %{_sysconfdir}/$i ] ; then
    echo "Removing preinstalled %{_sysconfdir}/$i"
    %{__rm} -rf %{_sysconfdir}/$i
  fi
done
for i in Gtinydns Gdnscache
do
  echo "Removing user $i"
  /usr/bin/getent passwd $i > /dev/null && /usr/sbin/userdel $i >/dev/null || true
  %{__rm} -f /var/spool/$i
done

%changelog
* Tue May 22 2018 12:45:06 +0530 mbhangui@gmail.com 1.05--1.1%{?dist}
Release 1.1 Start 11/04/2017
1.  Added dnsgetroot
2.  added str_diffn()
3.  added comments for dempsky's patch djbdns<=1.05 lets AXFRed subdomains overwrite domains
4.  fixed debian/prerm script
5.  added selinux rules for tinydns
6.  added Pre-Depends daemontools
7.  remove tinydns, dnscache service on uninstall
8.  shutdown tinydns, dnsccache service on uninstall
9.  added compile time option to add dnssec support
10. tinydnssec - added SRV patch
11. added native SRV type to tinydns-data
12. makes axfr-get decompose SRV and PTR records and write them out in native format
13. added curvedns
14. fixed typo in debian/postrm.in
15. (rpm, debian) - create tinydnssec, dqcache, curvedns service config
16. added dqcache-conf, curvedns-conf programs
17. added compile time option to add dnssec, curvedns support
18. added djbdns.7 man page
19. Changed dns accounts to Gtinydns, Gdnslog, Gdnscache
20. added man page for random-ip, dnsgetroot
21. use qmaill for loguser
22. upgraded dnscurve to version 0.88
