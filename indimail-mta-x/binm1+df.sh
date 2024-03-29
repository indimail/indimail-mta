#!/bin/sh

# Using splogger to send the log through syslog.
# Using dot-forward to support sendmail-style ~/.forward files.
# Using binmail to deliver messages to /var/spool/mail/$USER by default.
# Using BSD 4.4 binmail interface: /usr/libexec/mail.local -r

exec env - PATH="PREFIX/bin:$PATH" \
qscheduler '|dot-forward .forward
|preline -f /usr/libexec/mail.local -r "${SENDER:-MAILER-DAEMON}" -d "$USER"' \
splogger qmail
