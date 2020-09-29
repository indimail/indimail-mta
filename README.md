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

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ cd /usr/local/src/libqmail
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in libqmail/conf-version)

NOTE: for FreeBSD, install automake, autoconf, libtool, pkgconf using pkg

## Download indimail-mta

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-mta.git
```

## Compile libdkim-x (with dynamic libaries)

```
$ cd /usr/local/src/indimail-mta/libdkim-x
$ ./default.configure
$ ./make
$ sudo ./make -s install-strip
```

(check version in indimail-mta/libdkim-x/conf-version)

## Compile libsrs2-x (with dynamic libaries)

```
$ cd /usr/local/src/indimail-mta/libsrs2-x
$ ./default.configure
$ ./make
$ sudo ./make install-strip
```

(check version in indimail-mta/libsrs2-x/conf-version)

## Build ucspi-tcp

To configure the build for ucspi-tcp, you need to configure conf-prefix, conf-sysconfdir and conf-shared. Defaults are given in the table below. You can also use the script default.configure to set the below values.

**Linux**

config file|value
-----------|------
conf-prefix|/usr
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail

**FreeBSD**

config file|value
-----------|------
conf-prefix|/usr/local
conf-sysconfdir|/usr/local/etc/indimail
conf-shared|/usr/local/share/indimail

```
$ cd /usr/local/src/indimail-mta/ucspi-tcp-x
$ make
$ sudo make install-strip
```

(check version in indimail-mta/ucspi-tcp-x/conf-version)

## Build indimail-mta

To configure the build for indimail-mta, you need to configure conf-prefix, conf-qmail, conf-sysconfdir, conf-shared and conf-libexec. Defaults are given in the table below. You can also use the script default.configure to set the below values.

**Linux**

config file|value
-----------|------
conf-prefix|/usr
conf-qmail|/var/indimail
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail
conf-libexec|/usr/libexec/indimail

**FreeBSD**

config file|value
-----------|------
conf-prefix|/usr/local
conf-qmail|/var/indimail
conf-sysconfdir|/usr/local/etc/indimail
conf-shared|/usr/local/share/indimail
conf-libexec|/usr/local/libexec/indimail

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-mta/indimail-mta-x/conf-version)

## Download optional components

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
```

### Build nssd

Optional component. Required only if you require the Standard C library routines to use Name Service Switch to authenticate from a MySQL db (e.g. for authenticated SMTP, IMAP, POP3, etc). Your passwd(5) database gets extended to indimail's MySQL database. You will also need to edit /etc/nsswitch.conf and have a line like this `passwd: files nssd`. Check the man page for nssd(8) and nsswitch.conf(5)

```
$ cd /usr/local/src/indimail-virtualdomans/nssd-x
$ ./default.configure
$ make
$ sudo make install-strip
```

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

## Start Services

```
$ sudo systemctl start svscan # Linux
or
$ sudo service svscan start # Universal
or
$ /etc/init.d/svscan start # Linux
or
$ qmailctl start # Universal
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
