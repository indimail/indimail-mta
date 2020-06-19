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

This document contains installation instructions for install indimail-mta from source as well as building binary packages from the source.

To install you need to do the following

# Source Compiling/Linking

The steps below give instructions on building from source. If you need to deploy indimail-mta on multiple hosts, it is better to create a set of RPM / Deb binary packages. Once generated, the same package can be deployed on multiple hosts. To generate RPM packages for all components refer to [CREATE-Packages.md](https://github.com/mbhangui/indimail-mta/blob/master/.github/CREATE-Packages.md)

You can also use docker / podman images to deploy indimail-mta. Look at the chapter `# Docker / Podman Repository` below on how to do that. The big advantage of using a docker / podman image is you can change your configuration with the `docker commit ..` or `podman commit` to save your entire build and deploy the exact configuration on multiple hosts.

Doing a source build can be daunting for many. You can always use the pre-built binaries from the DNF / YUM / APT repositories given in the chapter `# Binary Builds on openSUSE Build Service` towards the end of this document.

Doing a source build requires you to have all the development packages installed. Linux distributions are known to be crazy. You will have different package names for different distirbutions. e.g.

db-devel, libdb-devel, db4-devel on different systems, just to get Berkeley db installed. There is an easy way out to find out what your distribution needs.

* For RPM based distribtions, locate your .spec file (e.g. indimail-mta.spec in indimail-mta/indimail-mta-x directory, libqmail/libqmail.spec). Open the RPM spec file and look for `BuildRequires`. This will tell you what you require for your distribution. If there is a specific version of development package required, you will find `%if %else` statements. Use dnf / yum / zypper to install your development package.
* For debian based distribution, locate your debian subdirectory (e.g. indimail-mta/indimail-mta-x/debian, libqmail/debian). In this directory you will find files with `.dsc` extension. Look at the line Build-Depends. Use `apt-get install package` to install the package. If your debian distribution has few libraries different than the default, you will find a `.dsc` filename with a name corresponding to your distribution. (e.g. indimail-mta-x-Debain_10.dsc)

## Download / clone libqmail

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ cd /usr/local/src/libqmail
$ ./default.configure
$ make
$ sudo make install-strip
```

## Download indimail-mta

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-mta.git
cd /usr/local/src/indimail-mta/libdkim-x
./default.configure
make
sudo make install-strip
cd /usr/local/src/indimail-mta/libsrs2-x
./default.configure
make
sudo make install-strip
```

To configure indimail-mta, you need to configure conf-prefix, conf-qmail, conf-sysconfdir, conf-shared and conf-libexec. Defaults are given in the table below. You can also use the script default.configure to set the below values.

config file|value
-----------|------
conf-prefix|/usr
conf-qmail|/var/indimail
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail
conf-libexec|/usr/libexec/indimail


```
cd /usr/local/src/indimail-mta/indimail-mta-x
./default-configure
make
sudo make install-strip
```

## Download optional components

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
```

### Build nssd

Optional component. Required only if you require a Name Service Switch to authenticate from a MySQL db (e.g. for authenticated SMTP)

```
cd /usr/local/src/indimail-virtualdomans/nssd-x
./default.configure
make
sudo make install-strip
```

### Build pam-multi

Optional. Required only if you require PAM authentication for authenticated SMTP or extra PAM other than /etc/shadow authentication for IMAP / POP3

```
cd /usr/local/src/indimail-virtualdomans/pam-multi-x
./default.configure
make
sudo make install-strip
```

### Build courier-imap

Optional. Required only if you want IMAP, POP3 to retrieve your mails

```
cd /usr/local/src/indimail-virtualdomans/courier-imap-x
./default.configure
make
sudo make install-strip
```

### Build fetchmail

Optional. Required only if you want fetchmail to retrieve your mails

```
cd /usr/local/src/indimail-virtualdomans/fetchmail-x
./default.configure
make
sudo make install-strip
```

### Build altermime

Optional. Required only if you want altermime to add content to your emails before delivery. e.g. adding disclaimers

```
cd /usr/local/src/indimail-virtualdomans/altermime-x
./default.configure
make
sudo make install-strip
```

### Build ripmime

Optional. Required only if you want extract attachments from your emails

```
cd /usr/local/src/indimail-virtualdomans/ripmime-x
./default.configure
make
sudo make install-strip
```

### Build mpack

Optional. Required only if you want to pack a zip file and attach it to your email.

```
cd /usr/local/src/indimail-virtualdomans/mpack-x
./default.configure
make
sudo make install-strip
```

### Build flash

Optional. Required only if you want a configurable ncurses based menu system to configure a system for administering emails using a dumb terminal

```
cd /usr/local/src/indimail-virtualdomans/flash-x
./default.configure
make
sudo make install-strip
```

### Build fortune

Optional. Required only if you want fortune cookies to be sent out in your outgoing emails.

```
cd /usr/local/src/indimail-virtualdomans/fortune-x
./default.configure
make
sudo make install-strip
```

# Setup & Configuration

You are here because you decided to do a complete source installation. If you use source installation method, you need to setup various configuration and services. You can configure indimail-mta using /usr/sbin/svctool. svctool is a general purpose utility to configure indimail-mta services.

You can also run the script `create_services` which invokes svctool to setup few default services to start a fully configured system. `create_services` will also put a systemd unit file `svscan.service` in `/lib/systemd/system`.

```
$ cd /usr/local/src/indimail-mta-x
$ sudo sh ./create_services --servicedir=/services --qbase=/var/indimail/queue
$ sudo service indimail start
```

# Binary Packages Build

If you need to have indimail-mta on multiple machines, you can build binary packages once and install the same package on multiple machines. The other big advantage of using a binary build is that the binary installation will give you fully functional, configured system using your hostname for defaults. You can always change these configuration files in /etc/indimail to cater to your requirements later. With a binary build, you don't need to run the `create_services` command.
You can also download pre-built binary packages from [openSUSE Build Service](https://build.opensuse.org/), described in the chapter '# Binary Builds on openSUSE Build Service`.

The steps for doing a binary build are

## Clone git repository

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ git clone https://github.com/mbhangui/indimail-mta.git
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
```

## Build libqmail, libqmail-dev package

```
$ cd /usr/local/src/libqmail
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-mta package

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-auth package

Optional. Required only if you want extra authentication methods using NSS or PAM.

```
$ cd /usr/local/src/indimail-virtualdomains/indimail-auth
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-access package

Optional. You require this if you want IMAP/POP3 or fetchmail to retrieve your mails

```
$ cd /usr/local/src/indimail-virtualdomains/indimail-access
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-utils package

Optional. Required only if you want utilities like altermime, ripmime, flash menu, mpack and fortune

```
$ cd /usr/local/src/indimail-virtualdomains/indimail-utils
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-spamfilter package

Optional. Required only if you want to use bogofilter to filter SPAM mails

```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-x
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Install Packages

Installing and configuration is much simplied when you use the Binary Packages Build. The pre, post instlation scripts do all the hard work for you. For each of the packages built above you can follow the steps below, depending on your linux distribution. The location of the RPM file will be ~/rpmbuild/RPMS/x86_64 for rpm based distributions and ~/stage for debian/ubuntu distributions.

** For RPM based distributions **

`sudo rpm -ivh rpm_file`

** For Debian based distributions **

`sudo dpkg -i debian_file`

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
4. [indimail-utils](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-utils) (Multiple utilities that can work with indimail-mta - altermime, ripmime, mpack, fortune and flash - customizable menu based admin interface)

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

# Docker / Podman Repository

The [docker repository](https://hub.docker.com/r/cprogrammer/indimail) can be used to pull docker/podman images
for indimail.

For latest details refer to [README](https://github.com/mbhangui/docker/blob/master/README.md)

# SUPPORT INFORMATION #

## IRC
IndiMail has an IRC channel ##indimail and ##indimail-mta

## Mailing list

There are four Mailing Lists for IndiMail

1. indimail-support  - You can subscribe for Support [here](https://lists.sourceforge.net/lists/listinfo/indimail-support). You can mail [indimail-support](mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here](https://lists.sourceforge.net/lists/listinfo/indimail-devel). You can mail [indimail-devel](mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe [here](http://groups.google.com/group/indimail)
4. Archive at [Google Groups](http://groups.google.com/group/indimail). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker](http://sourceforge.net/tracker/?group_id=230686) for IndiMail (Bugs, Feature Requests, Patches, Support Requests)
