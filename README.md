# indimail-mta

qmail fork with DKIM, SRS2 &amp; [IndiMail Virtual Domains](https://github.com/mbhangui/indimail-virtualdomains)

indimail-mta is the default MTA installed when you install [IndiMail Virtual Domains](https://github.com/mbhangui/indimail-virtualdomains).
Refer to this detailed [README](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/README-indimail.md) for Indimail Virtual Domains.
Refer to this detailed [TUTORIAL](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/indimail.md) for  detailed tutorial on working with IndiMail.

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

edit the files conf-qmail, conf-prefix, conf-sysconfdir, conf-shared, conf-libexec in /usr/local/src/indimail-mta/indimail-mta-x directory.
(see the note on directory structure at the bottom)

```
cd /usr/local/src/indimail-mta/indimail-mta-x
make
sudo make install-strip
```

# Binary Packages build

If you need to have indimail-mta on multiple machines, you can build binary packages once and install the same package on multiple machines.
The steps for doing this are

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

# Setup & Configuration

You can configure indimail-mta using /usr/sbin/svctool. svctool is a general purpose utility to configure indimail-mta services.

You can also run the script create_services which invokes svctool to setup few default services to start a full fledged messaging server create_services will also put a systemd unit file indimail.service in /lib/systemd/system

```
$ cd /usr/local/src/indimail-mta-2.x
$ sudo sh ./create_services --servicedir=/services --qbase=/var/indimail/queue
$ sudo service indimail start
```

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

# Some settings

```
conf-shared       - /usr/share/indimail
conf-prefix       - /usr
conf-sysconfdir   - /etc/indimail
conf-libexec      - /usr/libexec/indimail
conf-qmail        - /var/indimail
```

# Binary Builds

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
