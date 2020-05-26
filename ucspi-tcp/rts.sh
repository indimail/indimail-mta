#
# $Log: rts.sh,v $
# Revision 1.1  2003-12-31 19:57:58+05:30  Cprogrammer
# Initial revision
#
#
env - PATH="`pwd`:$PATH" sh rts.tests 2>&1 | cat -v
