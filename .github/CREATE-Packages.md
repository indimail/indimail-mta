# Binary Packages Build

For this to work you need few things to be installed on your system. Check your system manual on how to install them.

* RPM based systems - rpm-build, gcc, g++, autoconf, automake, libtool, aclocal, rpmdevtools
* Debian based systems - build-essentials, cdbs, debhelper, gnupg2
* Arch Linux - base-devel

## Clone git repository

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ git clone https://github.com/mbhangui/indimail-mta.git
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
```

## Build libqmail package

Common library used by indimail, indimail-mta, ezmlm-idx, tinydnssec packages

```
$ cd /usr/local/src/libqmail
$ ./create_rpm     # for RPM
$ ./create_debian  # for deb
$ ./create_archpkg # for zst (Arch Linux)
```

## Build libdkim package

```
$ cd /usr/local/src/indimail-mta/libdkim-x
$ ./create_rpm     # for RPM
$ ./create_debian  # for deb
$ ./create_archpkg # for zst (Arch Linux)
```

## Build libsrs2 package

```
$ cd /usr/local/src/indimail-mta/libsrs2-x
$ ./create_rpm     # for RPM
$ ./create_debian  # for deb
$ ./create_archpkg # for zst (Arch Linux)
```

## Build indimail-mta package

Essential component required for providng MTA functions

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ ./create_rpm     # for RPM
$ ./create_debian  # for deb
$ ./create_archpkg # for zst (Arch Linux)
```

## Build indimail-auth package

Optional component. Required only if you require a Name Service Switch & extra PAM modules for authentication

```
$ cd /usr/local/src/indimail-virtualdomains/indimail-auth
$ ./create_rpm     # for RPM
$ ./create_debian  # for deb
$ ./create_archpkg # for zst (Arch Linux)
```

## Build indimail-access package

Optional. You require this if you want IMAP/POP3 or fetchmail to retrieve your mails

```
$ cd /usr/local/src/indimail-virtualdomains/indimail-access
$ ./create_rpm     # for RPM
$ ./create_debian  # for deb
$ ./create_archpkg # for zst (Arch Linux)
```

## Build indimail-utils package

Optional. Required only if you want utilities like altermime, ripmime, flash menu, mpack and fortune

```
$ cd /usr/local/src/indimail-virtualdomains/indimail-utils
$ ./create_rpm     # for RPM
$ ./create_debian  # for deb
$ ./create_archpkg # for zst (Arch Linux)
```

## Build indimail-spamfilter package

Optional. Required only if you want to use bogofilter to filter SPAM mails

```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-x
$ ./create_rpm     # for RPM
$ ./create_debian  # for deb
$ ./create_archpkg # for zst (Arch Linux)
```

## Install Packages

Installing and configuration is much simplied when you use the Binary Packages Build. The pre, post instlation scripts do all the hard work for you.

**For RPM based distributions**

```
$ sudo rpm -ivh file.rpm
```

**For Debian based distributions**

```
$ sudo dpkg -i file.deb
```

**For Arch Linux**

```
$ sudo pacman -U file.zst
```
