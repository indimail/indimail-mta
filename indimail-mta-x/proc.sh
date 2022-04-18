#!/bin/sh

# Using splogger to send the log through syslog.
# Using procmail to deliver messages to /var/spool/mail/$USER by default.

exec env - PATH="PREFIX/bin:$PATH" \
qscheduler '|preline procmail' splogger qmail
