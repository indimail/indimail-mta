#!/bin/sh
# cdbgetm.sh
# cdbget for multiple records/key
# $Log: $
# 
# $Id: $
skip=0
while QMAIL/bin/cdbget ${1} ${skip} ${@} ; do
    echo ""
    skip=$((${skip} + 1))
done
