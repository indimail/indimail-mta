# indimail-mta

[qmail](http://cr.yp.to/qmail.html "qmail") fork with [DKIM](https://github.com/mbhangui/indimail-mta/tree/master/libdkim-x), [SRS2](https://github.com/mbhangui/indimail-mta/tree/master/libsrs2-x) &amp; [IndiMail Virtual Domains](https://github.com/mbhangui/indimail-virtualdomains).  indimail-mta additionally includes forks of the following packages

* [ucspi-tcp](https://cr.yp.to/ucspi-tcp.html "ucspi-tcp")
* [serialmail](https://cr.yp.to/serialmail.html "serialmail")
* [qmailanalog](http://cr.yp.to/qmailanalog.html "qmailanalog")
* [dotforward](https://cr.yp.to/dot-forward.html "dot-foward")
* [fastforward](https://cr.yp.to/fastforward.html "fastforward")
* [mess822](https://cr.yp.to/mess822.html "mess822")
* [daemontools](https://cr.yp.to/daemontools.html "daemontools")

indimail-mta is the default MTA installed when you install [IndiMail Virtual Domains](https://github.com/mbhangui/indimail-virtualdomains).
Refer to this [README](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/README-indimail.md) for a detailed understanding of Indimail Virtual Domains and indimail-mta.

This document contains instructions for building indimail-mta from source.

To install you need to do the following

# Source Compiling/Linking

The steps below give instructions to build from source. If you need to deploy indimail-mta on multiple hosts, it is better to create a set of RPM / Deb binary packages. Once generated, the package/packages can be deployed on multiple hosts. To generate RPM packages for all components refer to [Binary Packages](.github/CREATE-Packages.md)

You can also use docker / podman images to deploy indimail-mta. Look at the chapter [Docker / Podman Repository](#docker-/-podman-repository) below on how to do that. The big advantage of using a docker / podman image is you can save your configuration with the `docker commit ..` or `podman commit` to checkpoint your entire build and deploy the exact configuration on multiple hosts.

Doing a source build can be daunting for many. You can always use the pre-built binaries from the DNF / YUM / APT repositories given in the chapter [Binary Builds on OBS](#binary-builds-on-opensuse-build-service) towards the end of this document.

Doing a source build requires you to have all the development packages installed. Linux distributions are known to be crazy. You will have different package names for different distirbutions. e.g.

db-devel, libdb-devel, db4-devel on different systems, just to get Berkeley db installed. There is an easy way out to find out what your distribution needs.

* For RPM based distribtions, locate your .spec file (e.g. indimail-mta.spec in indimail-mta/indimail-mta-x directory, libqmail/libqmail.spec). Open the RPM spec file and look for `BuildRequires`. This will tell you what you require for your distribution. If there is a specific version of development package required, you will find `%if %else` statements. Use dnf / yum / zypper to install your development package.
* For debian based distribution, locate your debian subdirectory (e.g. indimail-mta/indimail-mtaa-x/debian, libqmail/debian). In this directory you will find files with `.dsc` extension. Look at the line having `Build-Depends`. Use `apt-get install package` to install the package. If your debian distribution has few libraries different than the default, you will find a `.dsc` filename with a name corresponding to your distribution. (e.g. indimail-mta-Debain_10.dsc)

**Note**

This is a rough list of packages required. If you want the exact packages, look BuildRequires in the spec file or Build-Depends in the debian/control or debian/\*.dsc files

**RPM Based Distributions**
Install the following packages using dnf/yum

```
Universal
gcc gcc-c++ make autoconf automaake libtool pkgconfig
sed findutils diffutils gzip binutils coreutils grep
glibc glibc-devel procps openssl openssl-devel mysql-devel
libqmail-devel libqmail

opensuse - openldap2-devel instead of openldap-devel
```

**Debian Based Distributions**
Install the following packages using apt

```
Universal
cdbs, debhelper, gcc, g++, automake, autoconf, libtool libqmail-dev, libqmail,
libldap2-dev, libssl-dev, mime-support, m4, gawk, openssl, procps, sed,
findutils, diffutils, readline, gzip, binutils, coreutils, grep

Debian 9, Debian 10 - default-libmysqlclient-dev
Remaining - libmysqlclient-dev,
Ubuntu 16.04 - libcom-err2, libmysqlclient-dev
```

## Download / clone libqmail

libqmail uses GNU autotools. You need to haave autoconf, automake, libtool and pkg config package. Follow the instructions below to have them installed in case you don't have them.

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ cd /usr/local/src/libqmail
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in libqmail/conf-version)

NOTE: for FreeBSD, install packages using pkg

```
# pkg install automake autoconf libtool pkgconf
```

NOTE: For Darwin (Mac OSX), install [MacPorts](https://www.macports.org/) or brew. You can look at this [document](https://paolozaino.wordpress.com/2015/05/05/how-to-install-and-use-autotools-on-mac-os-x/) for installing MacPorts.

```
# port install autoconf libtool automake pkgconfig
# port install openssl
# port update outdated
```

## Download indimail-mta

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-mta.git
```

After you clone indimail-mta, you will find two subfolders under indimail-mta directory (libdkim-x and libsrs2-x). We now need to build libdkim and libsrs2. If you skip these two steps, indimail-mta will get built without DKIM and SRS2 support.

## Compile libdkim-x (with dynamic libaries)

This library from ALT-N Technologies is required for qmail-dkim. If you don't have this library, indimail-mta will be built without DKIM support.

```
$ cd /usr/local/src/indimail-mta/libdkim-x
$ ./default.configure
$ ./make
$ sudo ./make -s install-strip
```

(check version in indimail-mta/libdkim-x/conf-version)

## Compile libsrs2-x (with dynamic libaries)

This library from [libsrs2.org](https://www.libsrs2.org/) is required for [SRS](https://en.wikipedia.org/wiki/Sender_Rewriting_Scheme). Else indimail-mta will get built without SRS support.

```
$ cd /usr/local/src/indimail-mta/libsrs2-x
$ ./default.configure
$ ./make
$ sudo ./make install-strip
```

(check version in indimail-mta/libsrs2-x/conf-version)

## Build daemontools

To configure the build for daemontools, you need to configure conf-prefix, conf-qmail, conf-sysconfdir, conf-shared, conf-libexec and conf-servicedir. Defaults are given in the table below. If you are ok with the defaults, you can run the script default.configure to set the below values.

**Linux**

config file|value
-----------|------
conf-prefix|/usr
conf-qmail|/var/indimail
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail
conf-libexec|/usr/libexec/indimail
conf-servicedir|/service

**FreeBSD**, **Darwin**

config file|value
-----------|------
conf-prefix|/usr/local
conf-qmail|/var/indimail
conf-sysconfdir|/usr/local/etc/indimail
conf-shared|/usr/local/share/indimail
conf-libexec|/usr/local/libexec/indimail
conf-servicedir|/usr/local/etc/indimail/sv

The build below depends on several Makefiles. For the build to operate without errors, you need to run default.configure the first time and everytime after you do a `make clean`. If you don't run default.configure, you can run replace `make` with `./qmake`

```
$ ./default.configure
$ cd /usr/local/src/indimail-mta/daemontools-x
$ make or ./qmake
$ sudo make install or sudo ./qmake install
```

(check version in indimail-mta/daemontools-x/conf-version)

## Build ucspi-tcp

To configure the build for ucspi-tcp, you need to configure conf-prefix, conf-sysconfdir and conf-shared. Defaults are given in the table below. If you are ok with the defaults, you can run the script default.configure to set the below values.

**Linux**

config file|value
-----------|------
conf-prefix|/usr
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail

**FreeBSD** / **Darwin**

config file|value
-----------|------
conf-prefix|/usr/local
conf-sysconfdir|/usr/local/etc/indimail
conf-shared|/usr/local/share/indimail

The build below depends on several Makefiles. For the build to operate without errors, you need to run default.configure the first time and everytime after you do a `make clean`. If you don't run default.configure, you can run replace `make` with `./qmake`

```
$ ./default.configure
$ cd /usr/local/src/indimail-mta/ucspi-tcp-x
$ make or ./qmake
$ sudo make install or sudo ./qmake install
```

(check version in indimail-mta/ucspi-tcp-x/conf-version)

## Build indimail-mta

To configure the build for indimail-mta, you need to configure conf-prefix, conf-qmail, conf-sysconfdir, conf-shared, conf-libexec and conf-servicedir. Defaults are given in the table below. If you are ok with the defaults, you can also use the script default.configure.

**Linux**

config file|value
-----------|------
conf-prefix|/usr
conf-qmail|/var/indimail
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail
conf-libexec|/usr/libexec/indimail
conf-servicedir|/service

**FreeBSD** / **Darwin**

config file|value
-----------|------
conf-prefix|/usr/local
conf-qmail|/var/indimail
conf-sysconfdir|/usr/local/etc/indimail
conf-shared|/usr/local/share/indimail
conf-libexec|/usr/local/libexec/indimail
conf-servicedir|/usr/local/libexec/indimail/service

The build below depends on several Makefiles. For the build to operate without errors, you need to run default.configure the first time and everytime after you do a `make clean`. If you don't run default.configure, you can run replace `make` with `./qmake`

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ ./default.configure
$ make or ./qmake
$ sudo make install or sudo qmake install
```

(check version in indimail-mta/indimail-mta-x/conf-version)

Note: for Darwin

```
$ sudo port install openldap mrtg
```

## Download/Build Optional Components

If you don't need any of the optional components, you can directly jump to the 'Setup & Configuration' section below.

-----------------------------------------------------
## indimail-virtualdomains

Optional. Required only if you want to use virtualdomains to create many domains on a single host

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
```

## Build bogofilter

Optional. Required only if you want to use bogofilter for spam filtering

```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-x
$ ./default.configure
$ make
$ sudo make install-strip
```

NOTE: for Darwin

```
$ sudo port install db48
```

(check version in indimail-virtualdomains/bogofilter-x/conf-version)

## Build bogofilter-wordlist

Optional. Required only if you want to use bogofilter for spam filtering

```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-wordlist-1.0
$ ./default.configure
$ make
$ sudo make install-strip
```

### Build nssd

Optional component. Required only if you require the Standard C library routines to use Name Service Switch to authenticate from a MySQL db (e.g. for authenticated SMTP, IMAP, POP3, etc). Your passwd(5) database gets extended to indimail's MySQL database. You will also need to edit /etc/nsswitch.conf and have a line like this.

```
passwd: files nssd
```

Check the man page for nssd(8) and nsswitch.conf(5)

```
$ cd /usr/local/src/indimail-virtualdomans/nssd-x
$ ./default.configure
$ make
$ sudo make install-strip
```

NOTE: Darwin doesn't have nsswitch. So don't waste time compiling this package

(check version in indimail-virtualdomains/nssd-x/conf-version)

### Build pam-multi

Optional. Required only if you require PAM authentication for authenticated SMTP or extra PAM other than /etc/shadow authentication for IMAP / POP3

```
$ cd /usr/local/src/indimail-virtualdomans/pam-multi-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/pam-multi-x/conf-version)

### Build courier-imap

Optional. Required only if you want IMAP, POP3 to retrieve your mails

```
$ cd /usr/local/src/indimail-virtualdomans/courier-imap-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/courier-imap-x/conf-version)

NOTE: for Darwin

```
# port install libidn2 pcre db48
```

### Build fetchmail

Optional. Required only if you want fetchmail to retrieve your mails

```
$ cd /usr/local/src/indimail-virtualdomans/fetchmail-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/fetchmail-x/conf-version)

### Build altermime

Optional. Required only if you want altermime to add content to your emails before delivery. e.g. adding disclaimers

```
$ cd /usr/local/src/indimail-virtualdomans/altermime-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/altermime-x/conf-version)

### Build ripmime

Optional. Required only if you want extract attachments from your emails

```
$ cd /usr/local/src/indimail-virtualdomans/ripmime-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/ripmime-x/conf-version)

### Build mpack

Optional. Required only if you want to pack a zip file and attach it to your email.

```
$ cd /usr/local/src/indimail-virtualdomans/mpack-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/mpack-x/conf-version)

### Build flash

Optional. Required only if you want a configurable ncurses based menu system to configure a system for administering emails using a dumb terminal

```
$ cd /usr/local/src/indimail-virtualdomans/flash-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/flash-x/conf-version)

### Build fortune

Optional. Required only if you want fortune cookies to be sent out in your outgoing emails.

```
$ cd /usr/local/src/indimail-virtualdomans/fortune-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/fortune-x/conf-version)

# Setup & Configuration

You are here because you decided to do a complete source installation. If you use source installation method, you need to setup various configuration and services. You can configure indimail-mta using /usr/sbin/svctool. svctool is a general purpose utility to configure indimail-mta services and configuration.

You can also run the script `create_services` which invokes svctool to setup few default services to start a fully configured system. `create_services` will also put a systemd(1) unit file `svscan.service` in `/lib/systemd/system` for systems using systemd. For FreeBSD, it will create /usr/local/etc/rc.d/svscan. It will use alternatives command to setup indimail-mta as your default MTA. On FreeBSD, it will configure mailwrapper by modifying /etc/mail/mailer.conf.

```
$ cd /usr/local/src/indimail-mta-x
$ sudo ./create_services
```

NOTE: The Darwin Mac OSX system is broken for sending emails because you can't remove /usr/sbin/sendmail. [System Integrity Protection (SIP)](https://en.wikipedia.org/wiki/System_Integrity_Protection) ensures that you cannot modify anything in /bin, /sbin, /usr, /System, etc. You could disable it by using csrutil in recover mode but that is not adviseable. See [this](https://www.howtogeek.com/230424/how-to-disable-system-integrity-protection-on-a-mac-and-why-you-shouldnt/). indimail-mta requires services in /service to configure all startup items. On Mac OS X, it uses `/etc/synthetic.conf' to create a virtual symlink of /service to /usr/local/libexec/indimail/service. This file is created/modified by 'svctool --add-boot' command. For program that need to send mails, you will need to call /usr/local/bin/sendmail (indimal-mta's sendmail replacement). The OS and all utilites like cron, mailx, etc will continue to use /usr/sbin/sendmail. There is nothing you can do about it, other than fooling around with SIP.

## Enable svscan to be started at boot

```
$ sudo /usr/sbin/svctool --config=add-boot
```

You can enable indimail-mta as an alternative mta (if your system supports the `alternatives` commaand)

```
$ sudo  /usr/bin/svctool --config=add-alt
```

You can remove automatic startup at boot by running the command

```
$ sudo /usr/sbin/svctool --config=rm-boot
```

## Start Services

```
$ sudo systemctl start svscan # Linux
or
$ sudo service svscan start # Linux/FreeBSD
or
$ /etc/init.d/svscan start # Linux
or
$ sudo launchctl start org.indimail.svscan # Mac OSX
or
$ qmailctl start # Universal
```

After starting svscan as given above, your system will be ready to send and receive mails, provided you have set your system hostname, domain name IP addresses and setup mail exchanger in DNS. You can look at this [guide](https://www.godaddy.com/garage/configuring-dns-for-email-a-quick-beginners-guide/) to do that.

## Check Status of Services

The svstat command can be used to query the status of various services. You can query for all services like below. You can query the status of a single service like running a command like this.

```
% sudo svstat /service/qmail-smtpd.25
```
The argument to svstat should be a directory in /service. Each directory in /service refers to an indimail-mta/indimail service. e.g. `/service/qmail-smtpd.25` refers to the SMTP service serving port 25.

If you don't have /service create a link to /usr/libexec/indimail/service (/usr/local/libexec/indimail/service on FreeBSD and Darwin). I'm working on creating this link automatically during setup.

```
$ sudo svstat /service/*
/service/greylist.1999: up (pid 120502) 4394 seconds
/service/pwdlookup: up (pid 120515) 4394 seconds
/service/qmail-daned.1998: up (pid 120413) 4394 seconds
/service/qmail-logfifo: up (pid 120499) 4394 seconds
/service/qmail-qmqpd.628: down 4394 seconds
/service/qmail-qmtpd.209: up (pid 120487) 4394 seconds
/service/qmail-send.25: up (pid 120469) 4394 seconds
/service/qmail-smtpd.25: up (pid 120416) 4394 seconds
/service/qmail-smtpd.366: up (pid 120516) 4394 seconds
/service/qmail-smtpd.465: up (pid 120492) 4394 seconds
/service/qmail-smtpd.587: up (pid 120473) 4394 seconds
/service/qscanq: up (pid 120500) 4394 seconds
/service/udplogger.3000: up (pid 120463) 4394 seconds
```

# Binary Packages Build

If you need to have indimail-mta on multiple machines, you can build binary packages once and install the same package on multiple machines. The other big advantage of using a binary build is that the binary installation will give you fully functional, configured system using your hostname for defaults. You can always change these configuration files in /etc/indimail to cater to your requirements later. With a binary build, you don't need to run the `create_services` command. To generate RPM packages locally for all components refer to [Binary Packages](.github/CREATE-Packages.md)
You can also download pre-built binary packages from [openSUSE Build Service](https://build.opensuse.org/), described in the chapter [Binary Builds on OBS](#binary-builds-on-opensuse-build Service) .

NOTE: binary package for FreeBSD and OSX is in my TODO list.

## Some Notes on directory structure

indimail-mta has files in standard unix directories. You can change
the locationsby editing the following files in indimail-mta source
directory

configuration file|Purpose
------------------|-------
conf-prefix|this is where bin, sbin go
conf-shared|this is where boot, doc go (conf-prefix/share/indimail)
conf-sysconfdir|this is where etc, control, users go
conf-libexec|this is where private scripts/executables go
conf-qmail|domains, alias, queue, autoturn, qscanq, symlinks for control, users, bin and sbin

You can have the old non-fhs behaviour by having /var/indimail in the above 5 files. In addition to the above, indimail uses the hardcoded directory /usr/lib/indimail in build scripts

```
/usr/lib/indimail - plugins, modules (architecture-dependent files)
```

# Binary Builds on openSUSE Build Service

You can get binary RPM / Debian packages at

* [Stable Releases](http://download.opensuse.org/repositories/home:/indimail/)
* [Experimental Releases](http://download.opensuse.org/repositories/home:/mbhangui/)

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories, depending on whether you want to install a stable or an experimental release, are

* [Stable](https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail-mta)
* [Experimental](https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail-mta)

NOTE: Once you have setup your DNF / YUM / apt-get repo, you an also decide to install the additional software

1. [indimail-access](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-access) - IMAP/POP3 & fetchmail for mail retreival
2. [indimail-auth](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-auth) (nssd - providing Name Service Switch and pam-multi providing multiple PAM modules for flexible, configurable authentication methods)
3. [indimail-spamfilter](https://github.com/mbhangui/indimail-virtualdomains/tree/master/bogofilter-x) - SPAM filter capabillity using bogofilter - https://bogofilter.sourceforge.io
4. [indimail-utils](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-utils) (Multiple utilities that can work with indimail/indimail-mta - [altermime](http://pldaniels.com/altermime/), [ripMIME](https://pldaniels.com/ripmime/), [mpack](https://github.com/mbhangui/indimail-virtualdomains/tree/master/mpack-x), [fortune](https://en.wikipedia.org/wiki/Fortune_(Unix)) and [flash](https://github.com/mbhangui/indimail-virtualdomains/tree/master/flash-x) - customizable menu based admin interface)

```
Currently, the list of supported distributions for IndiMail is

    * SUSE
          o openSUSE_Leap_15.0
          o openSUSE_Leap_15.1
          o openSUSE_Leap_15.2
          o openSUSE_Tumbleweed
          o SUSE Linux Enterprise 12
          o SUSE Linux Enterprise 12 SP1
          o SUSE Linux Enterprise 12 SP2
          o SUSE Linux Enterprise 12 SP3
          o SUSE Linux Enterprise 12 SP4
          o SUSE Linux Enterprise 12 SP5
          o SUSE Linux Enterprise 15
          o SUSE Linux Enterprise 15 SP1

    * Red Hat
          o Fedora 32
          o Fedora 31
          o Red Hat Enterprise Linux 6
          o Red Hat Enterprise Linux 7
          o CentOS 6
          o CentOS 7

    * Debian
          o Debian  8.0
          o Debian  9.0
          o Debian 10.0
          o Ubuntu 16.04
          o Ubuntu 17.04
          o Ubuntu 18.04
          o Ubuntu 19.04
          o Ubuntu 19.10
          o Ubuntu 20.04
```

## NOTE for binary builds on debian

debian/ubuntu repositories already has daemontools and ucspi-tcp which is
far behind in terms of feature list that the indimail-mta repo provides.
When you install indimail-mta, you get both. But in case you want to install
daemontools, ucspi-tcp or both you can ensure that the two packages get
installed from the indimail-mta repository instead of the debian repository.

All you need to do is set a higher preference for the indimail-mta
repository by creating /etc/apt/preferences.d/preferences with
the following conents

```
$ sudo /bin/bash
# cat > /etc/apt/preferences.d/preferences <<EOF
Package: *
Pin: origin download.opensuse.org
Pin-Priority: 1001
EOF
```

You can verify this by doing

```
$ apt policy daemontools ucspi-tcp
daemontools:
  Installed: 2.11-1.1+1.1
  Candidate: 2.11-1.1+1.1
  Version table:
     1:0.76-7 500
        500 http://raspbian.raspberrypi.org/raspbian buster/main armhf Packages
 *** 2.11-1.1+1.1 1001
       1001 http://download.opensuse.org/repositories/home:/indimail/Debian_10  Packages
        100 /var/lib/dpkg/status
ucspi-tcp:
  Installed: 2.11-1.1+1.1
  Candidate: 2.11-1.1+1.1
  Version table:
     1:0.88-6 500
        500 http://raspbian.raspberrypi.org/raspbian buster/main armhf Packages
 *** 2.11-1.1+1.1 1001
       1001 http://download.opensuse.org/repositories/home:/indimail/Debian_9.0/ Packages
        100 /var/lib/dpkg/status
```

# Docker / Podman Repository

The [docker repository](https://hub.docker.com/r/cprogrammer/indimail) can be used to pull docker/podman images
for indimail.

For latest details refer to [README](https://github.com/mbhangui/docker/blob/master/README.md)

# SUPPORT INFORMATION

## IRC / Matrix

* Join me [#indimail:matrix.org](https://matrix.to/#/#indimail:matrix.org)
* IndiMail has an IRC channel #indimail-mta

## Mailing list

There are four Mailing Lists for IndiMail

1. indimail-support  - You can subscribe for Support [here](https://lists.sourceforge.net/lists/listinfo/indimail-support). You can mail [indimail-support](mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here](https://lists.sourceforge.net/lists/listinfo/indimail-devel). You can mail [indimail-devel](mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe [here](http://groups.google.com/group/indimail)
4. Archive at [Google Groups](http://groups.google.com/group/indimail). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker](http://sourceforge.net/tracker/?group_id=230686) for IndiMail (Bugs, Feature Requests, Patches, Support Requests)

# History

Both indimail-mta and indimail-virtualdomains started in late 1999 as a combined package of unmodified qmail and modified vpopmail.

indimail-mta started as a unmodified qmail-1.03. This was when I was employed by an ISP in late 1999. The ISP was using [Critical Path's ISOCOR](https://www.wsj.com/articles/SB940514435217077581) for providing Free and Paid email service. Then the dot com burst happened and ISP didn't have money to spend on upgrading the Sun Enterprise servers. The mandate was to move to an OSS/FS solution. After evaluating sendmail, postfix and qmail, we chose qmail. During production deployment, qmail couldn't scale on these servers. The issue was the queue's todo count kept on increasing. We applied the qmail-todo patch, but still we couldn't handle the incoming email rate. By now the customers were screaming, the corporate users were shooting out nasty emails. We tried a small hack the solved this problem. Compiled 20 different qmail setups, with conf-qmail as /var/qmail1, /var/qmail2, etc. Run qmail-send for each of these instance. A small shim was written which would get the current time and divide by 20. The remainder was used to do exec of /var/qmail1/bin/qmail-queue, /var/qmail2/bin/qmail-queue, etc. The shim was copied as /var/qmail/bin/qmail-queue. The IO problem got solved. But the problem with this solution was compiling the qmail source 20 times and copying the shim as qmail-queue. You couldn't compile qmail on one machine and use the backup of binaries on another machine. Things like uid, gid, the paths were all hardcoded in the source. That is how the base of indimail-mta took form by removing each and every hard coded uids, gids and paths. indimail-mta still does the same thing that was done in the year 2000. The installation creates multiple queues - /var/indimail/queue/queue1, /var/indimail/queue/queue2, etc. A new daemon named qmail-daemon uses QUEUE\_COUNT env variable to run multiple qmail-send instances. Each qmail-send instance can instruct qmail-queue to deposit mail in any of the queues installed. All programs use qmail-multi, a qmail-queue frontend to load balance the incoming email across multiple queues.

indimail-virtualdomain started with a modified vpopmail base that could handle a distributed setup - Same domain on multiple servers. Having this kind of setup made the control file smtproutes unviable. email would arrive at a relay server for user@domain. But the domain '@domain' was preset on multiple hosts, with each host having it's own set of users. This required special routing and modification of qmail (especially qmail-remote) to route the traffic to the correct host. vdelivermail to had to be written to deliver email for a local domain to a remote host, in case the user wasn't present on the current host. New MySQL tables were created to store the host information for a user. This table would be used by qmail-local, qmail-remote, vdelivermail to route the mail to the write host. All this complicated stuff had to be done because the ISP where I worked, had no money to buy/upgrade costly servers to cater to users, who were multiplying at an exponential rate. The govt had just opened the license for providing internet services to private players. These were Indians who were tasting internet and free email for the first time. So the solution we decided was to buy multiple intel servers [Compaq Proliant](https://en.wikipedia.org/wiki/ProLiant) running Linux and make the qmail/vpopmail solution horizontally scalable. This was the origin of indimail-1.x which borrowed code from vpopmail, modified it for a domain on multiple hosts. indimail-2.x was a complete rewrite using djb style, using [libqmail](https://github.com/mbhangui/libqmail) as the standard library for all basic operations. All functions were discarded because they used the standard C library. The problem with indimail-2.x was linking with MySQL libraries, which caused significant issues building binary packages on [openSUSE build service](https://build.opensuse.org/). Binaries got built with MySQL/MariaDB libraries pulled by OBS, but when installed on a target machine, the user would have a completely different MySQL/MariaDB setup. Hence a decision was taken to load the library at runtime using dlopen/dlsym. This was the start of indimail-3.x. The source was moved from sourceforge.net to github and the project renamed as indimail-virtualdomains from the original name IndiMail. The modified source code of qmail was moved to github as indimail-mta.
