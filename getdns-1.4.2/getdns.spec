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
%global shareddir          /usr/share/indimail
%global libexecdir         /usr/libexec/indimail

Name: getdns
Version: 1.4.2
Release: 1%{?dist}
Summary: DNS recursive lookup for DANE TLSA RR

Group: System Environment/Libraries
%if %build_on_obs == 1
License: GPL-3.0+
%else
License: GPLv3
%endif
URL: http://www.indimail.org
Source0: http://downloads.sourceforge.net/indimail/%{name}-%{version}.tar.gz
Source1: http://downloads.sourceforge.net/indimail/%{name}-rpmlintrc
BuildRequires: unbound-devel
BuildRequires: libevent-devel pkgconfig openssl-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root
%if %{undefined centos_version} && %{undefined rhel_version}
BuildRequires: chrpath
%endif
%if 0%{?suse_version} == 1220 || 0%{?suse_version} == 1210 || 0%{?suse_version} == 1140 || 0%{?suse_version} == 1100 || 0%{?suse_version} == 1030 || 0%{?suse_version} == 1020
BuildRequires: chrpath
%endif

%if %build_on_obs == 1
%if 0%{?suse_version}
BuildRequires: -post-build-checks
#!BuildIgnore: post-build-checks
#!BuildIgnore: brp-check-suse
%endif
%endif

%description
danetlsa library provides DANE TLSA RR lookups.

%package devel
Summary: danetlsa - Development header files and libraries
Group: Development/Libraries/Other
Requires: getdns

%description devel
This package contains the development header files and libraries
for TLSA RR lookups.

%prep
%setup -q

%build
./configure --prefix=%{_prefix} --libdir=%{_libdir} \
  --with-trust-anchor=/etc/indimail/certs/getdns-root.key \
  --with-libevent --without-libidn --without-libidn2
make %{?_smp_mflags}

%install
%make_install
%{__rm} -f %{buildroot}%{_libdir}/libgetdns.la
%{__rm} -f %{buildroot}%{_libdir}/libgetdns_ext_event.la
%{__rm} -f %{buildroot}%{_prefix}/share/doc/getdns/INSTALL
%{__rm} -f %{buildroot}%{_prefix}/share/doc/getdns/LICENSE
if [ -x /usr/bin/chrpath ] ; then
  for i in getdns_query getdns_server_mon
  do
  /usr/bin/chrpath -d %{buildroot}%{_prefix}/bin/$i
  done
  /usr/bin/chrpath -d %{buildroot}%{_libdir}/libgetdns_ext_event.so.10.0.2
fi

%files
%defattr(-, root, root,-)
%attr(755,root,root)                    %{_prefix}/bin/getdns_query
%attr(755,root,root)                    %{_prefix}/bin/getdns_server_mon

%{_libdir}/libgetdns.so.10
%{_libdir}/libgetdns.so.10.0.2
%{_libdir}/libgetdns_ext_event.so.10
%{_libdir}/libgetdns_ext_event.so.10.0.2

%dir %attr(775,root,root)               %{_prefix}/share/doc/getdns
%dir %attr(775,root,root)               %{_prefix}/share/doc/getdns/spec
%attr(644,root,root)                    %{_prefix}/share/doc/getdns/AUTHORS
%attr(644,root,root)                    %{_prefix}/share/doc/getdns/ChangeLog
%attr(644,root,root)                    %{_prefix}/share/doc/getdns/NEWS
%attr(644,root,root)                    %{_prefix}/share/doc/getdns/README.md
%attr(644,root,root)                    %{_prefix}/share/doc/getdns/spec/index.html

%files devel
%defattr(-, root, root,-)
%dir %attr(775,root,root)     %{_prefix}/include/getdns

%attr(644,root,root)                    %{_prefix}/include/getdns/getdns.h
%attr(644,root,root)                    %{_prefix}/include/getdns/getdns_ext_libevent.h
%attr(644,root,root)                    %{_prefix}/include/getdns/getdns_extra.h
%{_libdir}/libgetdns.a
%{_libdir}/libgetdns_ext_event.a
%{_libdir}/pkgconfig/getdns.pc
%{_libdir}/pkgconfig/getdns_ext_event.pc

%{_libdir}/libgetdns.so
%{_libdir}/libgetdns_ext_event.so

%docdir %{shareddir}/doc
%attr(644,root,root)                    %{_mandir}/man3/*.gz

%changelog
