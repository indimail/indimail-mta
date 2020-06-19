# Binary Packages Build

## Clone git repository

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ git clone https://github.com/mbhangui/indimail-mta.git
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
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

Installing and configuration is much simplied when you use the Binary Packages Build. The pre, post instlation scripts do all the hard work for you.

**For RPM based distributions**

`$ sudo rpm -ivh rpm_file`

**For Debian based distributions**

`$ sudo dpkg -i debian_file`
