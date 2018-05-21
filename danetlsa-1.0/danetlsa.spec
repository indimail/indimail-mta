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

Name: danetlsa
Version: 1.0
Release: 1%{?dist}
Summary: DNS recursive lookup for DANE TLSA RR

Group: System Environment/Libraries
%if %build_on_obs == 1
License: GPL-3.0+
%else
License: GPLv3
%endif
URL: http://www.indimail.org
Source0:  http://downloads.sourceforge.net/indimail/%{name}-%{version}.tar.gz
BuildRequires: libevent-devel unbound-devel getdns-devel
BuildRequires: openssl-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Provides: libdanetlsa = %{version}

%description
danetlsa library provides DANE TLSA RR lookups.

%package devel
Summary: danetlsa - Development header files and libraries
Group: Development/Libraries/Other
Requires: libdanetlsa

%description devel
This package contains the development header files and libraries
for TLSA RR lookups.

%package -n libdanetlsa
Summary: danetlsa - Shared libraries
Group: Development/Libraries/Other

%description -n libdanetlsa
This package contains the shared libraries (*.so*) which certain
applications need to dynamically load and use danetlsa.

%prep
%setup -q


%build
./configure --prefix=%{_prefix} --libdir=%{_libdir} \
	--libexecdir=%{libexecdir} --datarootdir=%{shareddir}

make %{?_smp_mflags}

%install
%make_install
%{__rm} -f %{buildroot}%{_libdir}/libdanetlsa.la

%files
%{libexecdir}/tlsarr
%{_libdir}/libdanetlsa-1.0.so.0
%{_libdir}/libdanetlsa-1.0.so.0.0.0

%files devel
%attr(644,root,root)                   %{_prefix}/include/indimail/query-getdns.h
%{_libdir}/libdanetlsa.so
%{_libdir}/libdanetlsa.a

%files -n libdanetlsa
%{_libdir}/libdanetlsa-1.0.so.0
%{_libdir}/libdanetlsa-1.0.so.0.0.0

%docdir %{shareddir}/doc
%doc

%changelog

