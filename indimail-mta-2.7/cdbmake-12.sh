#
# $Log: cdbmake-12.sh,v $
# Revision 1.3  2018-01-31 12:02:29+05:30  Cprogrammer
# moved cdbmake to sbin
#
# Revision 1.2  2017-03-09 16:38:00+05:30  Cprogrammer
# FHS changes
#
# Revision 1.1  2008-09-16 08:22:40+05:30  Cprogrammer
# Initial revision
#
#
awk -c '
	/^[^#]/ {
		print "+" length($1) "," length($2) ":" $1 "->" $2
	}
	END {
		print ""
	}
' | PREFIX/sbin/cdbmake "$@"
