#
# $Log: cdbmake-12.sh,v $
# Revision 1.2  2017-03-09 16:38:00+05:30  Cprogrammer
# FHS changes
#
# Revision 1.1  2008-09-16 08:22:40+05:30  Cprogrammer
# Initial revision
#
#
awk '
	/^[^#]/ {
		print "+" length($1) "," length($2) ":" $1 "->" $2
	}
	END {
		print ""
	}
' | PREFIX/bin/cdbmake "$@"
