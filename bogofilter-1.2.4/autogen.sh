#!/bin/sh
#
# Run this to generate all the initial makefiles, etc.
#
# $Id: autogen.sh 3443 2003-10-29 15:12:06Z m-a $

DIE=0

PROGNAME="bogofilter"
(autoconf --version) < /dev/null | head -1 > autogen.log 2>&1 || {
  echo
  echo "**Error**: You must have 'autoconf' installed to compile $PROGNAME."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

(automake --version) < /dev/null | head -1 >> autogen.log 2>&1 || {
  echo
  echo "**Error**: You must have 'automake' installed to compile $PROGNAME."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

(flex --version) < /dev/null | head -1 >> autogen.log 2>&1 || {
  echo
  echo "**Error**: You must have 'flex' installed to compile $PROGNAME."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

if [ "$DIE" -eq 1 ]; then
  exit 1
fi

if [ -z "$*" ]; then
  echo "**Warning**:  Running 'configure' with no arguments."
  echo "If you wish to pass any to it, specify them on the"
  echo "'$0' command line."
  echo
fi

echo "Running 'autoreconf -fis'"
autoreconf -fis
echo ""

echo "Running './configure --quiet $*'"
./configure --quiet $*
echo ""

echo "Running 'make -s check'"
make -s check
echo ""
