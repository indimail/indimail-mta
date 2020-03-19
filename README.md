# indimail-mta
qmail fork with DKIM, SRS2 &amp; IndiMail virtual domains

To install you need to do the following

# Setup conf- files

edit the files conf-qmail, conf-prefix, conf-sysconfdir, conf-shared, conf-libexec
(see the bottom of this page)

# Compiling/Linking

# Download / clone libqmail
```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail-0.1.git
$ cd /usr/local/src/libqmail-0.1
$ ./default.configure
$ make
$ sudo make install-strip
```
   
# Download indimail-mta
```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-mta.git
cd /usr/local/src/indimail-mta/libdkim-1.5
./default.configure
make
sudo make install-strip
cd /usr/local/src/indimail-mta/libsrs2-1.0.18
./default.configure
make
sudo make install-strip
cd /usr/local/src/indimail-mta/indimail-mta-2.8 (replace 2.8 with the version number)
```

edit conf-files (see the note on directory structure at the bottom)

```
make
sudo make install-strip
```

# Setup & Configuration
You can configure indimail-mta using /usr/sbin/svctool. svctool is a general purpose utility to configure indimail-mta services.

You can also run the script create_services which invokes svctool to setup few default services to start a full fledged messaging server create_services will also put a systemd unit file indimail.service in /lib/systemd/system

```
$ cd /usr/local/src/indimail-mta-2.7
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

Stable Releases       - http://download.opensuse.org/repositories/home:/indimail/

Experimental Releases - http://download.opensuse.org/repositories/home:/mbhangui/

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories are at

https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail-mta

&

https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail-mta

```
Currently, the list of supported distributions for IndiMail is

    * SUSE
          o openSUSE_Leap_15.0
          o openSUSE_Leap_15.1
          o openSUSE_Leap_15.2
		  o openSUSE_Tumbleweed
          o SUSE Linux Enterprise 15
          o SUSE Linux Enterprise 15 SP1

    * Red Hat
          o Fedora 30
          o Fedora 31
          o Red Hat Enterprise Linux 6
          o Red Hat Enterprise Linux 7
          o CentOS 6
          o CentOS 7

    * Debian
          o Debian 7.0
          o Debian 8.0
          o Debian 9.0
          o Ubuntu 16.04
          o Ubuntu 17.04
          o Ubuntu 18.04
          o Ubuntu 19.04
          o Ubuntu 19.10
```

# Docker / Podman Repository
The docker repository for indimail-mta is at

https://hub.docker.com/r/cprogrammer/indimail-mta

for Docker
```
indimail-mta - docker pull cprogrammer/indimail-mta:tag
```
or

for Podman
```
indimail-mta - podman pull cprogrammer/indimail-mta:tag
```

where tag is one of

xenial   for ubuntu 16.04

bionic   for ubuntu 18.04

disco    for ubuntu 19.04

centos7  for centos7

debian8  for debian8

debian9  for debian9

debian10 for debian10

fc31     for fc31

fc30     for fc30
