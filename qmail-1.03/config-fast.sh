fqdn="$1"
if [ $# -eq 2 ] ; then
	DESTDIR=$2
fi
echo Your fully qualified host name is "$fqdn".

echo Putting "$fqdn" into "$DESTDIR"QMAIL/control/me...
echo "$fqdn" > "$DESTDIR"QMAIL/control/me
chmod 644 "$DESTDIR"QMAIL/control/me
(
	echo "$fqdn" | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./' |
	(
		read ddom
		echo Putting "$ddom" into control/defaultdomain...
		echo "$ddom" > "$DESTDIR"QMAIL/control/defaultdomain
		chmod 644 "$DESTDIR"QMAIL/control/defaultdomain
	)
)

(
	echo "$fqdn" | sed 's/^.*\.\([^\.]*\)\.\([^\.]*\)$/\1.\2/' |
	(
		read pdom
		echo Putting "$pdom" into "$DESTDIR"QMAIL/control/plusdomain...
		echo "$pdom" > "$DESTDIR"QMAIL/control/plusdomain
		chmod 644 "$DESTDIR"QMAIL/control/plusdomain
	)
)

grep $fqdn "$DESTDIR"QMAIL/control/virtualdomains > /dev/null
if [ $? -ne 0 ] ; then
	echo Putting "$fqdn" into "$DESTDIR"QMAIL/control/locals...
	echo "$fqdn" >> "$DESTDIR"QMAIL/control/locals
	sort -u "$DESTDIR"QMAIL/control/locals -o "$DESTDIR"QMAIL/control/locals
	chmod 644 "$DESTDIR"QMAIL/control/locals
fi

echo Putting "$fqdn" into "$DESTDIR"QMAIL/control/rcpthosts...
echo "$fqdn" >> "$DESTDIR"QMAIL/control/rcpthosts
sort -u "$DESTDIR"QMAIL/control/rcpthosts -o "$DESTDIR"QMAIL/control/rcpthosts
chmod 644 "$DESTDIR"QMAIL/control/rcpthosts
echo "Now qmail will refuse to accept SMTP messages except to $fqdn."
echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
