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

The steps below give instructions on building from source. If you need to deploy indimail-mta on multiple hosts, it is better to create a set of RPM / Deb binary packages. Once generated, the same package can be deployed on multiple hosts. To generate RPM packages for all components refer to [CREATE-Packages.md](CREATE-Packages.md)

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

```
conf-prefix       - this is where bin, sbin go
conf-shared       - this is where boot, doc go (conf-prefix/share/indimail)
conf-sysconfdir   - this is where etc, control, users go
conf-libexec      - this is where private scripts/executables go
conf-qmail        - domains, alias, queue, autoturn, qscanq, symlinks
                    for control, users, bin and sbin
```

You can have the old non-fhs behaviour by having /var/indimail in the
above 4 files. In addition to the above, indimail uses the hardcoded
directory /usr/lib/indimail in build scripts

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
