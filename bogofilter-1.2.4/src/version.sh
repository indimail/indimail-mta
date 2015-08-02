# no shebang line here, we want explicit $(SHELL) from make!

#	version.sh
#
#	create version.c to define BOGOFILTER_VERSION
#
#	if VERSION in config.h does not contain 'svn', use its value.
#	if it does contain 'svn', append a date to it.
#
#	We'll first try to find CVS/Entries files and use the most
#	current date from the files. To parse, we need Perl 5 and
#	the HTTP::Date module.
#
#	If that fails, either because HTTP::Date is missing, Perl is
#	missing or we're building outside the CVS working directory,
#	we'll fall back to use the current date (GMT time zone) with
#	hour precision.
#
#	On FreeBSD, HTTP::Date is part of the p5-libwww port.

VERSION=$(grep define.VERSION config.h | awk '{print $3}' | tr -d '"')

SUFFIX=$(echo $VERSION | egrep "\.svn$")

srcdir=$1
shift

set -e

if [ ! -z "$SUFFIX" ]; then
    set +e
    DATE=$(svn info --xml "$srcdir" | grep date | sed 's,</\?date>,,g' | perl -MHTTP::Date -e '
    $date = str2time(<>);
    $date=HTTP::Date::time2isoz($date);
    $date=~tr/ :Z-/_/d;
    print $date, "\n";
    ') || DATE=
    if [ "x$DATE" = "x" ] ; then
       DATE=$(env TZ=GMT date "+build_date_%Y%m%d_%Hh")
    fi
    VERSION="$VERSION.$DATE"
fi

echo "#include \"globals.h\""
echo "const char * const version = \"$VERSION\";"
