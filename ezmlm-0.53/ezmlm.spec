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

Name: ezmlm
Version: 0.53
Release: 1%{?dist}
Summary: Easy Mailing List Manager
Group: Utilities/System
%if %build_on_obs == 1
License: GPL-2.0+
%else
License: GPLv2
%endif
Source0: http://ezmlm.org/archive/%{version}/%{name}-%{version}.tar.gz
URL: http://www.ezmlm.org

##################################### OBS ####################################
%if %build_on_obs == 1
%if 0%{?rhel_version} == 700
BuildRequires: groff-base
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
Requires: rpm >= 3.0.2
Requires: indimail-mta >= 2.0
Buildroot: %{_tmppath}/%{name}-%{version}-root

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
indimail-mta for blazingly fast parallel SMTP deliveries.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
%{__make} -s DESTDIR=%{buildroot} setup

%files
%defattr(-, root, root,-)
%{_prefix}/bin/ezmlm-list
%{_prefix}/bin/ezmlm-make
%{_prefix}/bin/ezmlm-manage
%{_prefix}/bin/ezmlm-queue
%{_prefix}/bin/ezmlm-reject
%{_prefix}/bin/ezmlm-return
%{_prefix}/bin/ezmlm-send
%{_prefix}/bin/ezmlm-sub
%{_prefix}/bin/ezmlm-unsub
%{_prefix}/bin/ezmlm-warn
%{_prefix}/bin/ezmlm-weed
%{_prefix}/share/man/cat1/ezmlm-list.0
%{_prefix}/share/man/cat1/ezmlm-make.0
%{_prefix}/share/man/cat1/ezmlm-manage.0
%{_prefix}/share/man/cat1/ezmlm-reject.0
%{_prefix}/share/man/cat1/ezmlm-return.0
%{_prefix}/share/man/cat1/ezmlm-send.0
%{_prefix}/share/man/cat1/ezmlm-sub.0
%{_prefix}/share/man/cat1/ezmlm-unsub.0
%{_prefix}/share/man/cat1/ezmlm-warn.0
%{_prefix}/share/man/cat1/ezmlm-weed.0
%{_prefix}/share/man/cat5/ezmlm.0
%{_prefix}/share/man/man1/ezmlm-list.1.gz
%{_prefix}/share/man/man1/ezmlm-make.1.gz
%{_prefix}/share/man/man1/ezmlm-manage.1.gz
%{_prefix}/share/man/man1/ezmlm-reject.1.gz
%{_prefix}/share/man/man1/ezmlm-return.1.gz
%{_prefix}/share/man/man1/ezmlm-send.1.gz
%{_prefix}/share/man/man1/ezmlm-sub.1.gz
%{_prefix}/share/man/man1/ezmlm-unsub.1.gz
%{_prefix}/share/man/man1/ezmlm-warn.1.gz
%{_prefix}/share/man/man1/ezmlm-weed.1.gz
%{_prefix}/share/man/man5/ezmlm.5.gz

%doc

%changelog
