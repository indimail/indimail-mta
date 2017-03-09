#!/bin/sh

# Using splogger to send the log through syslog.
# Using dot-forward to support sendmail-style ~/.forward files.
# Using procmail to deliver messages to /var/spool/mail/$USER by default.

exec env - PATH="PREFIX/bin:$PATH" \
qmail-daemon '|dot-forward .forward
|preline procmail' splogger qmail
