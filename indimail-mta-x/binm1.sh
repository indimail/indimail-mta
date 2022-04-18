#!/bin/sh

# Using splogger to send the log through syslog.
# Using binmail to deliver messages to /var/spool/mail/$USER by default.
# Using BSD 4.4 binmail interface: /usr/libexec/mail.local -r

exec PREFIX/sbin/qscheduler \
'|preline -f /usr/libexec/mail.local -r "${SENDER:-MAILER-DAEMON}" -d "$USER"' \
splogger qmail
