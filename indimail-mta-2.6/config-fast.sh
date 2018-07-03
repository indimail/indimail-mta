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
	CONTROLDIR=SYSCONFDIR/control
fi
echo Putting "$fqdn" into $CONTROLDIR/me...
echo "$fqdn" > "$DESTDIR"/$CONTROLDIR/me
chmod 644 "$DESTDIR"/$CONTROLDIR/me
echo $fqdn | grep "\." > /dev/null
if [ $? -eq 0 ] ; then
	# qmail-inject
	echo Putting "$fqdn" into $CONTROLDIR/defaulthost...
	echo $fqdn > "$DESTDIR"/$CONTROLDIR/defaulthost
	chmod 644 "$DESTDIR"/$CONTROLDIR/defaulthost
	# qmail-send
	echo Putting "$fqdn" into $CONTROLDIR/envnoathost...
	echo $fqdn > "$DESTDIR"/$CONTROLDIR/envnoathost
	chmod 644 "$DESTDIR"/$CONTROLDIR/envnoathost
else
	echo "config-fast.sh: no dots in $fqdn. Skipping defaulthost, envnoathost" 1>&2
fi

echo "$fqdn" | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./' |
(
	read ddom
	echo $ddom | grep "\." > /dev/null
	if [ $? -eq 0 ] ; then
		# dot-forward, inewaliases, newinclude, qmail-inject
		echo Putting "$ddom" into $CONTROLDIR/defaultdomain...
		echo "$ddom" > "$DESTDIR"/$CONTROLDIR/defaultdomain
		chmod 644 "$DESTDIR"/$CONTROLDIR/defaultdomain
	else
		echo "config-fast.sh: no dots in $ddom. Skipping defaultdomain" 1>&2
	fi
)

echo "$fqdn" | sed 's/^.*\.\([^\.]*\)\.\([^\.]*\)$/\1.\2/' |
(
	read pdom
	echo $pdom | grep "\." > /dev/null
	if [ $? -eq 0 ] ; then
		# inewaliases, newinclude, qmail-inject
		echo Putting "$pdom" into $CONTROLDIR/plusdomain...
		echo "$pdom" > "$DESTDIR"/$CONTROLDIR/plusdomain
		chmod 644 "$DESTDIR"/$CONTROLDIR/plusdomain
	else
		echo "config-fast.sh: no dots in $pdom. Skipping plusdomain" 1>&2
	fi
)

grep $fqdn "$DESTDIR"/$CONTROLDIR/virtualdomains > /dev/null 2>&1
if [ $? -ne 0 ] ; then
	# qmail-send, qmail-smtpd
	echo Putting "$fqdn" into $CONTROLDIR/locals...
	echo "$fqdn" >> "$DESTDIR"/$CONTROLDIR/locals
	sort -u "$DESTDIR"/$CONTROLDIR/locals -o "$DESTDIR"/$CONTROLDIR/locals
	chmod 644 "$DESTDIR"/$CONTROLDIR/locals
fi

# qmail-smtpd
echo Putting "$fqdn" into /$CONTROLDIR/rcpthosts...
echo "$fqdn" >> "$DESTDIR"/$CONTROLDIR/rcpthosts
sort -u "$DESTDIR"/$CONTROLDIR/rcpthosts -o "$DESTDIR"/$CONTROLDIR/rcpthosts
chmod 644 "$DESTDIR"/$CONTROLDIR/rcpthosts
echo "Now qmail will refuse to accept SMTP messages except to the following..."
/bin/cat "$DESTDIR"/$CONTROLDIR/rcpthosts
echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
