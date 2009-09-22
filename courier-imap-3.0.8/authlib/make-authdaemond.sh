#
# This script is called by the make-authdaemond target in the Makefile.
# It's job is to read authdaemond.versions.final, which lists the authdaemond
# permutations that have been configured.  We recursively invoke make,
# to build those authdaemonds.
#
# $Id: make-authdaemond.sh,v 1.4 2003/10/25 03:57:37 mrsam Exp $
#
# Copyright 2000 Double Precision, Inc.  See COPYING for
# distribution information
#

MAKE="$1"
EXEEXT="$2"

authdaemond_build() {
authdaemond="$1"
modules="$2"
dependencies="$3"

    $MAKE AUTHDAEMOND_TARGET=$authdaemond$EXEEXT \
			AUTHDAEMONDLIST_O="$authdaemond.o" \
			AUTHDAEMONDLIST_C="$authdaemond.c" \
			AUTHDAEMOND_MODS="$modules" \
			AUTHDAEMOND_DEPS="$dependencies" \
			$authdaemond$EXEEXT
}

. ./authdaemond.versions.final
