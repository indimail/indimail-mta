#!/bin/sh

# Using splogger to send the log through syslog.
# Using dot-forward to support sendmail-style ~/.forward files.
# Using binmail to deliver messages to /var/spool/mail/$USER by default.
# Using V7 binmail interface: /bin/mail -f

exec env - PATH="PREFIX/bin:$PATH" \
qmail-daemon '|dot-forward .forward
|preline -f /bin/mail -f "${SENDER:-MAILER-DAEMON}" -d "$USER"' \
splogger qmail
