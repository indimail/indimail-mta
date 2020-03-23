#!/bin/sh
# cdbgetm.sh
# cdbget for multiple records/key
# $Log: cdbgetm.sh,v $
# Revision 1.3  2018-01-31 12:01:37+05:30  Cprogrammer
# moved cdbget to sbin
#
# Revision 1.2  2017-03-09 16:37:57+05:30  Cprogrammer
# FHS changes
#
# Revision 1.1  2010-04-30 14:53:07+05:30  Cprogrammer
# Initial revision
#
# 
# $Id: cdbgetm.sh,v 1.3 2018-01-31 12:01:37+05:30 Cprogrammer Exp mbhangui $
skip=0
while PREFIX/sbin/cdbget ${1} ${skip} ${@} ; do
    echo ""
    skip=$((${skip} + 1))
done
