# $Id: libauth.sh,v 1.2 2000/10/05 04:15:41 mrsam Exp $

SHELL="$1"
srcdir="$2"

tr ' ' '\012' | $SHELL ${srcdir}/libauth1.sh | $SHELL ${srcdir}/libauth2.sh \
	| $SHELL ${srcdir}/libauth1.sh

