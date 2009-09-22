%define _topdir /home/local/src/indimail/redhat
%define		prefix	/var
Summary: Qmail MTA
Name: qmail
Version: 1.03
Release: 1
URL: http://mail.indi.com
Source0: %{name}-%{version}.tar.gz
Patch0: %{name}-%{version}.patch
Copyright: Artistic License
Group: System Applications
#Buildrequires:	indimail
BuildRoot: %{_tmppath}/%{name}-root
AutoReqProv: No
Prereq:		/bin/sh indimail

%description
qmail is a secure, reliable, efficient, simple message transfer agent. It is designed for typical Internet-connected UNIX hosts.
As of October 2001, qmail is the second most common SMTP server on the Internet, and has by far the fastest growth of any SMTP
server.

Secure: Security isn't just a goal, but an absolute requirement. Mail delivery is critical for users; it cannot be turned off,
so it must be completely secure. (This is why I started writing qmail: I was sick of the security holes in sendmail and other
MTAs.)

Reliable: qmail's straight-paper-path philosophy guarantees that a message, once accepted into the system, will never be lost.
qmail also optionally supports maildir, a new, super-reliable user mailbox format. Maildirs, unlike mbox files and mh folders,
won't be corrupted if the system crashes during delivery. Even better, not only can a user safely read his mail over NFS, but
any number of NFS clients can deliver mail to him at the same time.

Efficient: On a Pentium under BSD/OS, qmail can easily sustain 200000 local messages per day---that's separate messages injected
and delivered to mailboxes in a real test! Although remote deliveries are inherently limited by the slowness of DNS and SMTP,
qmail overlaps 20 simultaneous deliveries by default, so it zooms quickly through mailing lists.

Simple: qmail is vastly smaller than any other Internet MTA.
Some reasons why:
(1) Other MTAs have separate forwarding, aliasing, and mailing list mechanisms. qmail has one simple forwarding mechanism that
    lets users handle their own mailing lists.
(2) Other MTAs offer a spectrum of delivery modes, from fast+unsafe to slow+queued. qmail-send is instantly triggered by new
    items in the queue, so the qmail system has just one delivery mode: fast+queued.
(3) Other MTAs include, in effect, a specialized version of inetd that watches the load average. qmail's design inherently
    limits the machine load, so qmail-smtpd can safely run from your system's inetd. 

%prep
%setup -q
patch -p1 < %{name}-%{version}.patch
%build
mkdir -p $RPM_BUILD_ROOT%{prefix}/qmail
home_path=%{prefix}/qmail
qmail_home=`head -1 conf-qmail`
if [ ! " $qmail_home" = " $home_path" ] ; then
	(
		echo $home_path
		tail +2 conf-qmail
	) > conf.home.$$
	mv conf-qmail conf-qmail.orig
	mv conf.qmail.$$ conf-qmail
fi
make
%install
echo "char auto_qmail[]=\"$RPM_BUILD_ROOT%{prefix}/qmail\";" > auto_Qmail.c
gcc -c auto_Qmail.c
./load install fifo.o hier.o auto_Qmail.o auto_split.o \
	auto_uids.o strerr.a substdio.a open.a error.a str.a fs.a 
make setup
if [ -f conf-qmail.orig ] ; then
	mv conf-qmail.orig conf-qmail
fi
%files
%defattr(-,root,qmail)
#%{prefix}/qmail/bin/*
%attr(4755, alias,qmail)%{prefix}/qmail/alias
%attr(4755, indimail,qmail)%{prefix}/qmail/autoturn
%attr(755, root,qmail)%{prefix}/qmail/bin
%attr(755, root,qmail)%{prefix}/qmail/boot
%attr(4755, indimail,qmail)%{prefix}/qmail/control
%attr(755, root,qmail)%{prefix}/qmail/doc
%attr(755, root,qmail)%{prefix}/qmail/man
%attr(755, root,qmail)%{prefix}/qmail/users
%attr(4711,qmailq,qmail)%{prefix}/qmail/bin/qmail-queue
