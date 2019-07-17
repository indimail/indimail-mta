# indimail-mta
qmail fork with DKIM, SRS2 &amp; IndiMail virtual domains

To install you need to do the following

# Setup conf- files

edit the files conf-qmail, conf-prefix, conf-sysconfdir, conf-shared, conf-libexec

# Compiling/Linking

cd libdkim-1.5

./default.configure

make

sudo make install-strip


cd libsrs2-1.0.18

./default.configure

make

sudo make install-strip


cd indimail-mta

edit conf-fiies (see the note on directory structure at the bottom)

make

sudo make install-strip


# Setup & Configuration
You can configure indimail-mta using /usr/sbin/svctool. svctool is a general purpose utility to configure indimail-mta services.

You can also run the script create_services which invokes svctool to setup few default services to start a full fledged messaging server create_services will also put a systemd unit file indimail.service in /lib/systemd/system


$ cd /usr/local/src/indimail-mta-2.7

$ sudo sh ./create_services --servicedir=/services --qbase=/var/indimail/queue

$ sudo service indimail start

Some Notes on directory structure
=================================
indimail-mta has files in standard unix directories. You can change
the locationsby editing the following files in indimail-mta source
directory

conf-prefix       - this is where bin, sbin go

conf-shared       - this is where boot, doc go (conf-prefix/share/indimail)

conf-sysconfdir   - this is where etc, control, users go

conf-libexec      - this is where private scripts/executables go

conf-qmail        - domains, alias, queue, autoturn, qscanq, symlinks
                    for control, users, bin and sbin

You can have the old non-fhs behaviour by having /var/indimail in the
above 4 files. In addition to the above, indimail uses the hardcoded
directory /usr/lib/indimail in build scripts

/usr/lib/indimail - plugins, modules (architecture-dependent files)

# Some settings

conf-shared       - /usr/share/indimail

conf-prefix       - /usr

conf-sysconfdir   - /etc/indimail

conf-libexec      - /usr/libexec/indimail

conf-qmail        - /var/indimail
