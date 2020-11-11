# daemontools - collection of tools for managing UNIX services

This is a significant rewrite of [daemontools](https://cr.yp.to/daemontools.html) written by djb. This version of daaemontools is part of the [indimail-mta](https://github.com/mbhangui/indimail-mta), but can be used independently.

1. run shutdown script on svc -d, svc -r
2. run alert scripts when services go down abnormally
3. systemctl unit file for linux, rc for FreeBSD, launchd unit file for OSX
4. Use of /run, /var/run to allow readonly service directory
5. Logging of svscan output
6. Private namespace for filesystems mounted by services under svscan
7. Configurable scan interval
8. Run initialiation commands on svscan startup
9. Addtional actions in svc command (-r, -1, -2)
10. svps command to pretty print service status
11. Ability for a service to wait for another service
12. Additional information in svstat (wait status, supervise pid)
13. rpm/debian packages along with create\_rpm, create\_debian scripts
14. Return the status of supervise/service in svstat itself.
15. man pages for all commands
16. docker-entrypoint to svscan
17. ability to become session leader if SETSID env variable is set
18. create .svscan.pid file
19. Linked with [libqmail](https://github.com/mbhangui/libqmail) to avoid duplication of functions and ease of maintenance. This also implies use of substdio interface instead of buffer interface for standard input / output

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

For more details look at [README](https://github.com/mbhangui/indimail-mta/blob/master/README.md)
