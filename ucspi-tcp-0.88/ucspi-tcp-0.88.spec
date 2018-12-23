%define _topdir /hom/local/src/indimail/redhat
%define prefix /var
Summary: ucspi-tcp
Name: ucspi-tcp
Version: 0.88
Release: 1
URL: http://mail.indi.com
Source0: %{name}-%{version}.tar.gz
Copyright: Artistic License
Group: System Applications
#Buildrequires: indimail
BuildRoot: %{_tmppath}/%{name}-root
AutoReqProv: No
Prereq: /bin/sh

%description
ucspi-tcp provides easy-to-use command line tools for building TCP client-server applications.

It replaces several programs
inetd, a root-only TCP server supplied by all UNIX vendors
xinetd, a replacement for inetd
the mconnect client supplied as part of SunOS
the socket program
faucet and hose, part of netpipes package
the netcat program, which also supports UDP

%prep
%setup -q
%build
mkdir -p $RPM_BUILD_ROOT%{prefix}/qmail
home_path=$RPM_BUILD_ROOT%{prefix}/qmail
ucspitcp_home=`head -1 conf-home`
if [ ! " $ucspitcp_home" = " $home_path" ] ; then
  (
    echo $home_path
    tail +2 conf-home
  ) > conf.home.$$
  mv conf-home conf-home.orig
  mv conf.home.$$ conf-home
fi
make
%install
make setup
if [ -f conf-home.orig ] ; then
  mv conf-home.orig conf-home
fi
%files
%defattr(-,root,root)
%{prefix}/qmail/bin/*
