# $Log: multirotate.sh,v $
# Revision 1.3  2020-10-08 22:55:00+05:30  Cprogrammer
# use variables from Makefile
#
# Revision 1.2  2017-03-09 16:39:02+05:30  Cprogrammer
# FHS changes
#
# Revision 1.1  2002-10-04 15:24:14+05:30  Cprogrammer
# Initial revision
#

#
# $Id: multirotate.sh,v 1.3 2020-10-08 22:55:00+05:30 Cprogrammer Exp mbhangui $
#

exec PREFIX/bin/svc -a @servicedir@/*/log 2>&1
