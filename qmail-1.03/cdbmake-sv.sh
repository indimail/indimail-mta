#
# $Log: cdbmake-sv.sh,v $
# Revision 1.1  2008-09-16 08:22:51+05:30  Cprogrammer
# Initial revision
#
#
awk '{
	if (split($0,x,"#")) {
		f = split(x[1],y)
		if (f >= 2) {
			if (split(y[2],z,"/") >= 2) {
				a = "@" z[1] "/" z[2]
				print "+" length(a) "," length(y[1]) ":" a "->" y[1]
				for (i = 1;i <= f;i += 1) {
					if (i != 2) {
						a = y[i] "/" z[2]
						print "+" length(a) "," length(z[1]) ":" a "->" z[1]
					}
				}
			}
		}
	}
}
END {
    print ""
}
' | QMAIL/bin/cdbmake "$@"
