# ucspi-tcp - A collection of tools for building TCP client-server applications

This is a fork of [ucsp-tcp](http://cr.yp.to/ucspi-tcp.html). tcpserver and tcpclient are easy-to-use command-line tools for building TCP client-server applications. This version of ucspi-tcp is part of the [indimail-mta](https://github.com/mbhangui/indimail-mta), but can be used independently.

Few of the changes are

1. [IPv6](https://www.fefe.de/ucspi/) (using fefe's patch)
2. Andre Opperman's SSL/TLS patch for tcpserver
3. TLS/SSL support in tcpserver, tcpclient
4. Option -a to specify CA certificates. Taken from this patch [here](http://malete.org/tar/ucspi-tcp-ssl-20040113.patch.gz)
5. transparent TLS/SSL proxy with STARTTLS support for SMTP/POP3 servers using dotls
6. ucspi-tcp/ip6\_fmt.c: bug fix by Erwin Hoffman
7. GREETDELAY from Erwin Hoffman's [ucspi-tcp6](http://www.fehcom.de/ipnet/ucspi-tcp6.html)
8. Jens Wehrenbrecht's IPv4 CIDR extension
9. Li Minh Bui's IPv6 support for compactified IPv6 addresses and CIDR notation support.
10. Ability to load dynamic libraries to add functionality. tcpserver can load entire qmail-smtpd as a pluggable module.
11. MySQL support for tcprules. Uses dlopen to load MySQL at runtime.
12. tcpserver plugins. This version can load entire qmail-smtpd, rblsmtpd as a plugin in memory. Avoid fork/exec and repeated opening of qmail control files for every connection. uses dlopen/dlmopen at runtime.
13. Linked with [libqmail](https://github.com/mbhangui/libqmail) to avoid duplication of functions and ease of maintenance. This also implies use of substdio interface instead of buffer interface for standard input / output

## Build ucsp-tcp

To configure the build for ucspi-tcp, you need to configure conf-prefix, conf-sysconfdir, conf-shared, conf-libexec and conf-servicedir. Defaults are given in the table below. If you are ok with the defaults, you can run the script default.configure to set the below values.

**Linux**

config file|value
-----------|------
conf-prefix|/usr
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail
conf-libexec|/usr/libexec/indimail
conf-servicedir|/service

**FreeBSD** / **Darwin**

config file|value
-----------|------
conf-prefix|/usr/local
conf-sysconfdir|/usr/local/etc/indimail
conf-shared|/usr/local/share/indimail
conf-libexec|/usr/local/libexec/indimail
conf-servicedir|/usr/local/libexec/indimail/service

The build below depends on several Makefiles. For the build to operate without errors, you need to run default.configure the first time and everytime after you do a `make clean`. If you don't run default.configure, you can run replace `make` with `./qmake`

```
$ ./default.configure
$ cd /usr/local/src/indimail-mta/ucspi-tcp-x
$ make or ./qmake
$ sudo make install or sudo ./qmake install
```

(check version in indimail-mta/ucspi-tcp-x/conf-version)

For more details look at [README](https://github.com/mbhangui/indimail-mta/blob/master/README.md)
