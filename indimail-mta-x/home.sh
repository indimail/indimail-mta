#!/bin/sh

# Using splogger to send the log through syslog.
# Using qmail-local to deliver messages to ~/Mailbox by default.

exec env - PATH="PREFIX/sbin:$PATH" \
qscheduler ./Mailbox splogger qmail
