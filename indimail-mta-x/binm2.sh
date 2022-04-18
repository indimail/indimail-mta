#!/bin/sh

# Using splogger to send the log through syslog.
# Using binmail to deliver messages to /var/spool/mail/$USER by default.
# Using SVR4 binmail interface: /bin/mail -r

exec env - PATH="PREFIX/bin:$PATH" \
qscheduler \
'|preline -f /bin/mail -r "${SENDER:-MAILER-DAEMON}" -d "$USER"' \
splogger qmail
