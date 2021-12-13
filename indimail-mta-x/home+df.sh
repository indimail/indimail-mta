#!/bin/sh

# Using splogger to send the log through syslog.
# Using dot-forward to support sendmail-style ~/.forward files.
# Using qmail-local to deliver messages to ~/Mailbox by default.

exec env - PATH="PREFIX/sbin:PREFIX/bin$PATH" \
qscheduler '|dot-forward .forward
./Mailbox' splogger qmail
