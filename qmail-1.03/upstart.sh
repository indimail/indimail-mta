# $Log: upstart.sh,v $
# Revision 1.3  2008-09-08 15:24:34+05:30  Cprogrammer
# use the shorter notation for denoting multiple run levels
#
# Revision 1.2  2008-07-25 16:56:09+05:30  Cprogrammer
# added respawn
#
# Revision 1.1  2008-06-24 22:23:35+05:30  Cprogrammer
# Initial revision
#
# Start deamontools watcher
# console output, owner, none
# to start - initctl emit qmailstart
# to stop  - initctl emit qmailstop
# start svscan
# stop  svscan
# $Id: upstart.sh,v 1.3 2008-09-08 15:24:34+05:30 Cprogrammer Stab mbhangui $

start on runlevel [345]
start on qmailstart

stop on runlevel [0126]
stop on runlevel r
stop on qmailstop

console none
respawn
script
	exec QMAIL/bin/svscanboot /service /service1
end script

