%undefine _missing_build_ids_terminate_build
%define _unpackaged_files_terminate_build 1
%global debug_package %{nil}

%if %{defined _project}
# define if building on openSUSE build service
%define build_on_obs       1
%else
%define _project           local
%define build_on_obs       0
%global _hardened_build    1
%endif

%global ezcgidir /var/www/cgi-bin}
%global rcdir /etc/indimail/ezmlm}
%global mandir %_prefix/share/man
%global ezdocdir %_prefix/share/doc/ezmlm-idx

Name: ezmlm-idx
Version: 7.2.2
Release: 1
Summary: Easy Mailing List Manager + IDX patches
BuildRequires: rpm >= 3.0.2
Buildroot: %_tmppath/%name-%version-root
%if %build_on_obs == 1
License: GPL-3.0+
%else
License: GPLv3
%endif
Group: Utilities/System
Packager: Manvendra Bhangui <manvendra@indimail.org>
BuildRequires: mysql-devel
BuildRequires: postgresql-devel
BuildRequires: sqlite-devel
##################################### OBS ####################################
%if %build_on_obs == 1
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
Requires: rpm >= 3.0.2
Requires: indimail-mta >= 2.0
Source0: http://ezmlm.org/archive/%{version}/%{name}-%{version}.tar.gz
URL: http://www.ezmlm.org

%description
ezmlm lets users set up their own mailing lists within qmail's address
hierarchy. A user, Joe, types

   ezmlm-make ~/SOS ~/.qmail-sos joe-sos isp.net

and instantly has a functioning mailing list, joe-sos@isp.net, with all
relevant information stored in a new ~/SOS directory.

ezmlm sets up joe-sos-subscribe and joe-sos-unsubscribe for automatic
processing of subscription and unsubscription requests. Any message to
joe-sos-subscribe will work; Joe doesn't have to explain any tricky
command formats. ezmlm will send back instructions if a subscriber sends
a message to joe-sos-request or joe-sos-help.

ezmlm automatically archives new messages. Messages are labelled with
sequence numbers; a subscriber can fetch message 123 by sending mail to
joe-sos-get.123. The archive format supports fast message retrieval even
when there are thousands of messages.

ezmlm takes advantage of qmail's VERPs to reliably determine the
recipient address and message number for every incoming bounce message.
It waits ten days and then sends the subscriber a list of message
numbers that bounced. If that warning bounces, ezmlm sends a probe; if
the probe bounces, ezmlm automatically removes the subscriber from the
mailing list.

ezmlm is easy for users to control. Joe can edit ~/SOS/text/* to change
any of the administrative messages sent to subscribers. He can remove
~/SOS/public and ~/SOS/archived to disable automatic subscription and
archiving. He can put his own address into ~/SOS/editor to set up a
moderated mailing list. He can edit ~/SOS/{headeradd,headerremove} to
control outgoing headers. ezmlm has several utilities to manually
inspect and manage mailing lists.

ezmlm uses Delivered-To to stop forwarding loops, Mailing-List to
protect other mailing lists against false subscription requests, and
real cryptographic cookies to protect normal users against false
subscription requests. ezmlm can also be used for a sublist,
redistributing messages from another list.

ezmlm is reliable, even in the face of system crashes. It writes each
new subscription and each new message safely to disk before it reports
success to qmail.

ezmlm doesn't mind huge mailing lists. Lists don't even have to fit into
memory. ezmlm hashes the subscription list into a set of independent
files so that it can handle subscription requests quickly. ezmlm uses
qmail for blazingly fast parallel SMTP deliveries.

The IDX patches add: Indexing, (Remote) Moderation, digest, make
patches, multi-language, MIME, global interface, SQL database support.

%description -l pl
Menad<BF>er pocztowych list dyskusyjnych, ca<B3>kowicie spolszczony, mo<BF>liwo
<B6><F6> zdalnego moderowania, MIME.

%package mysql
Summary: MySQL support module for ezmlm-idx
Group: Utilities/System 
Requires: ezmlm-idx
Conflicts: ezmlm ezmlm-idx-std ezmlm-idx-mysql < 6.0

%description mysql
MySQL support module for ezmlm-idx. Provides sub-mysql.so module

%package pgsql
Summary: PostgreSQL support module for ezmlm-idx
Group: Utilities/System 
Requires: ezmlm-idx
Conflicts: ezmlm ezmlm-idx-std ezmlm-idx-pgsql < 6.0

%description pgsql
PostgreSQL support module for ezmlm-idx. Provides sub-pgsql.so module

%package sqlite3
Summary: SQLite3 support module for ezmlm-idx
Group: Utilities/System
Requires: ezmlm-idx
Conflicts: ezmlm ezmlm-idx-std ezmlm-idx-pgsql < 6.0

%description sqlite3
SQLite3 support module for ezmlm-idx. Provides sub-sqlite3.so module

%package cgi
Prefix: %ezcgidir
Summary: Web archiver for %name
Group: Utilities/System 
Requires: ezmlm-idx

%description cgi
www archiver for %name.
 

%prep 
%setup

%build 
echo %rcdir                      >conf-etc
echo %sysconfdir/indimail        >conf-sysconfdir
echo %_prefix                    >conf-prefix
echo %_bindir                    >conf-bin
echo %{mandir}                   >conf-man
echo %_prefix/lib/indimail/ezmlm >conf-lib
echo gcc %optflags -I%_includedir/mysql -I%_includedir/pgsql >conf-cc
echo gcc %optflags -s -L%{_libdir}/mysql >conf-ld

make all

# Create INSTALL file for how to set up ezmlm-cgi

(
echo "The binary ezmlm-cgi is installed as  %ezcgidir/ezmlm-cgi with"
echo "permissions 04555."
echo ""
echo "Please see INSTALL 16-22) in this package's doc directory and the"
echo "man page ezmlm-cgi.1 for more details on setting up and using ezmlm-cgi."
echo ""
) > INSTALL.cgi

%install
/bin/rm -rf %buildroot

%{__mkdir_p} %{buildroot}%{_bindir} 
%{__mkdir_p} %{buildroot}%{_prefix}/sbin
%{__mkdir_p} %{buildroot}%{_prefix}/lib/indimail/ezmlm
%{__mkdir_p} %{buildroot}%{rcdir}
%{__mkdir_p} %{buildroot}%{ezcgidir}
%{__mkdir_p} %buildroot%{ezdocdir}
%{__mkdir_p} %buildroot%{mandir}

./installer %buildroot/%rcdir                      < ETC
./installer %buildroot/%_bindir                    < BIN
./installer %buildroot/%{mandir}                   < MAN
./installer %buildroot/%_prefix/lib/indimail/ezmlm < LIB
%{__mv} %buildroot/%_bindir/ezmlm-queue %buildroot/%_prefix/sbin/ezmlm-queue
%{__mv} %buildroot/%_bindir/ezmlm-cgi   %buildroot/%{ezcgidir}/ezmlm-cgi
ln -s `head -n 1 conf-lang` %buildroot/%rcdir/default

# Compress the man pages
find %{buildroot}%{mandir} -type f -exec gzip -q {} \;
# create file list for man pages
find %buildroot/%{mandir} -type f | sed -e "s}%buildroot}}" -e "s}$}*}" > man-list

# copy documents
for i in BLURB CHANGES FAQ INSTALL README README.mysql README.pgsql README.std \
	THANKS TODO UPGRADE ChangeLog INSTALL.cgi ezcgirc ezcgi.css
do
	install -m 0644 $i %{buildroot}%{ezdocdir}
done

%post
echo To create an ezmlmrc file for a language other than US English
echo go to this package\'s doc directory, and type 
echo "    make iso"
echo 'where "iso" is the ISO language designation.' 
echo For currently supported languages, see the INSTALL
echo file, section 7.

%clean
/bin/rm -rf %buildroot

%files -f man-list

%defattr(-,root,root)

%dir %rcdir
%config(noreplace) %rcdir/*
%attr(644,root,root) %ezdocdir/BLURB
%attr(644,root,root) %ezdocdir/CHANGES* 
%attr(644,root,root) %ezdocdir/FAQ
%attr(644,root,root) %ezdocdir/INSTALL
%attr(644,root,root) %ezdocdir/README*
%attr(644,root,root) %ezdocdir/THANKS
%attr(644,root,root) %ezdocdir/TODO
%attr(644,root,root) %ezdocdir/UPGRADE
%attr(644,root,root) %ezdocdir/ChangeLog

%_bindir/*
%_prefix/sbin/*
%_prefix/lib/indimail/ezmlm/sub-std.so

%files cgi
%defattr(-,root,root)
%attr(4555,root,root) %ezcgidir/ezmlm-cgi
%attr(644,root,root)  %ezdocdir/INSTALL.cgi
%attr(644,root,root)  %ezdocdir/ezcgirc
%attr(644,root,root)  %ezdocdir/ezcgi.css
%{mandir}/man1/ezmlm-cgi.1*

%files mysql
%defattr(-,root,root)
%_prefix/lib/indimail/ezmlm/sub-mysql.so

%attr(644,root,root)  %ezdocdir/README.mysql

%files pgsql
%defattr(-,root,root)
%_prefix/lib/indimail/ezmlm/sub-pgsql.so

%attr(644,root,root)  %ezdocdir/README.pgsql

%files sqlite3
%defattr(-,root,root)
%_prefix/lib/indimail/ezmlm/sub-sqlite3.so
