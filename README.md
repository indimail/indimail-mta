# indimail-mta

qmail fork with DKIM, SRS2 &amp; [IndiMail virtual domains](https://github.com/mbhangui/indimail-virtualdomains)

To install you need to do the following

# Source Compiling/Linking

The steps below give instructions on building from source. If you need to deploy indimail-mta on multiple hosts, it is better to create a set of RPM / Deb binary packages. Once generated, the same package can be deployed on multiple hosts. To generate RPM packages for all components refer to [CREATE-Packages.md] (CREATE-Packages.md)

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

[Stable Releases] (http://download.opensuse.org/repositories/home:/indimail/)

[Experimental Releases] (http://download.opensuse.org/repositories/home:/mbhangui/)

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories, depending on whether you want to install a stable or an experimental release, are

* [Stable] (https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail)
* [Experimental] (https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail-mta)

NOTE: Once you have setup your DNF / YUM / apt-get repo, you an also decide to install the additional software

1. indimail-auth (nssd - providing name service switch and pam-multi to provide multiple pam auth methods)
2. indimail-utils (Multiple utility that can work with indimail-mta - altermime, ripmime, mpack, fortune and flash - customizable menu based admin interface)
3. indimail-spamfilter - SPAM filter capabillity using bogofilter - https://bogofilter.sourceforge.io
4. indimail-saccess - IMAP/POP3 & fetchmail for mail retreival

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

The docker repository for indimail-mta is [here] (https://hub.docker.com/r/cprogrammer/indimail-mta)

for Docker

```
docker pull cprogrammer/indimail-mta:tag
docker run image_id indimail
```
or

for Podman

```
podman pull cprogrammer/indimail-mta:tag
podman run image_id indimail
```
where image_id is the IMAGE ID of the docker / podman container obtained by running the **docker images** or the **podman images** command and tag is one of

tag|OS Distribution
----|----------------------
xenial|Ubuntu 16.04
bionic|Ubuntu 18.04
disco|Ubuntu 19.04
focal|Ubuntu 20.04
centos7|CentOS 7
debian8|Debian 8
debian9|Debian 9
debian10|Debian10
fc31|Fedora Core 31
fc32|Fedora Core 32
Tumbleweed|openSUSE Tumbleweed
Leap15.2|openSUSE Leap 15.2
