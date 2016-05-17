fqdn="$1"
if [ $# -eq 2 ] ; then
	DESTDIR=$2
fi
echo Your fully qualified host name is "$fqdn".

if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR=@controldir@
fi
slash=`echo $CONTROLDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
	cd QMAIL
fi
echo Putting "$fqdn" into "$DESTDIR"/$CONTROLDIR/me...
echo "$fqdn" > "$DESTDIR"/$CONTROLDIR/me
chmod 644 "$DESTDIR"/$CONTROLDIR/me
(
	echo "$fqdn" | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./' |
	(
		read ddom
		echo Putting "$ddom" into $CONTROLDIR/defaultdomain...
		echo "$ddom" > "$DESTDIR"/$CONTROLDIR/defaultdomain
		chmod 644 "$DESTDIR"/$CONTROLDIR/defaultdomain
	)
)

(
	echo "$fqdn" | sed 's/^.*\.\([^\.]*\)\.\([^\.]*\)$/\1.\2/' |
	(
		read pdom
		echo Putting "$pdom" into "$DESTDIR"/$CONTROLDIR/plusdomain...
		echo "$pdom" > "$DESTDIR"/$CONTROLDIR/plusdomain
		chmod 644 "$DESTDIR"/$CONTROLDIR/plusdomain
	)
)

grep $fqdn "$DESTDIR"/$CONTROLDIR/virtualdomains > /dev/null 2>&1
if [ $? -ne 0 ] ; then
	echo Putting "$fqdn" into "$DESTDIR"/$CONTROLDIR/locals...
	echo "$fqdn" >> "$DESTDIR"/$CONTROLDIR/locals
	sort -u "$DESTDIR"/$CONTROLDIR/locals -o "$DESTDIR"/$CONTROLDIR/locals
	chmod 644 "$DESTDIR"/$CONTROLDIR/locals
fi

echo Putting "$fqdn" into "$DESTDIR"/$CONTROLDIR/rcpthosts...
echo "$fqdn" >> "$DESTDIR"/$CONTROLDIR/rcpthosts
sort -u "$DESTDIR"/$CONTROLDIR/rcpthosts -o "$DESTDIR"/$CONTROLDIR/rcpthosts
chmod 644 "$DESTDIR"/$CONTROLDIR/rcpthosts
echo "Now qmail will refuse to accept SMTP messages except to $fqdn."
echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
