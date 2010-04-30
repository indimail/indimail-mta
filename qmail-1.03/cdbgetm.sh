#!/bin/sh
# cdbgetm.sh
# cdbget for multiple records/key
# $Log: cdbgetm.sh,v $
# Revision 1.1  2010-04-30 14:53:07+05:30  Cprogrammer
# Initial revision
#
# 
# $Id: cdbgetm.sh,v 1.1 2010-04-30 14:53:07+05:30 Cprogrammer Exp mbhangui $
skip=0
while QMAIL/bin/cdbget ${1} ${skip} ${@} ; do
    echo ""
    skip=$((${skip} + 1))
done
