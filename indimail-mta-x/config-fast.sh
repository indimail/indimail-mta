myecho()
{
	if [ $verbose -eq 0 ] ; then
		return 0
	fi
	echo $*
}
create_config()
{
fqdn=$1

if [ $verbose -eq 2 ] ; then
	echo "Your fully qualified domain name is $fqdn"
fi
if [ ! -f $CONTROLDIR/me -o $force -eq 1 ] ; then
	myecho Putting "$fqdn" into $CONTROLDIR/me...
	echo "$fqdn" > "$DESTDIR"$CONTROLDIR/me
	chmod 644 "$DESTDIR"$CONTROLDIR/me
fi
echo $fqdn | grep "\." > /dev/null
if [ $? -eq 0 ] ; then
	if [ ! -f $CONTROLDIR/defaulthost -o $force -eq 1 ] ; then
		# qmail-inject
		myecho Putting "$fqdn" into $CONTROLDIR/defaulthost...
		echo $fqdn > "$DESTDIR"$CONTROLDIR/defaulthost
		chmod 644 "$DESTDIR"$CONTROLDIR/defaulthost
	fi
	if [ ! -f $CONTROLDIR/envnoathost -o $force -eq 1 ] ; then
		# qmail-send
		myecho Putting "$fqdn" into $CONTROLDIR/envnoathost...
		echo $fqdn > "$DESTDIR"$CONTROLDIR/envnoathost
		chmod 644 "$DESTDIR"$CONTROLDIR/envnoathost
	fi
else
	echo "config-fast: no dots in $fqdn. Skipping defaulthost, envnoathost" 1>&2
fi

echo "$fqdn" | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./' |
(
	read ddom
	echo $ddom | grep "\." > /dev/null
	if [ $? -eq 0 ] ; then
		if [ ! -f $CONTROLDIR/defaultdomain -o $force -eq 1 ] ; then
			# dot-forward, inewaliases, newinclude, qmail-inject
			myecho Putting "$ddom" into $CONTROLDIR/defaultdomain...
			echo "$ddom" > "$DESTDIR"$CONTROLDIR/defaultdomain
			chmod 644 "$DESTDIR"$CONTROLDIR/defaultdomain
		fi
	else
		echo "config-fast: no dots in $ddom. Skipping defaultdomain" 1>&2
	fi
)

echo "$fqdn" | sed 's/^.*\.\([^\.]*\)\.\([^\.]*\)$/\1.\2/' |
(
	read pdom
	echo $pdom | grep "\." > /dev/null
	if [ $? -eq 0 ] ; then
		if [ ! -f $CONTROLDIR/plusdomain -o $force -eq 1 ] ; then
			# inewaliases, newinclude, qmail-inject
			myecho Putting "$pdom" into $CONTROLDIR/plusdomain...
			echo "$pdom" > "$DESTDIR"$CONTROLDIR/plusdomain
			chmod 644 "$DESTDIR"$CONTROLDIR/plusdomain
		fi
	else
		echo "config-fast.sh: no dots in $pdom. Skipping plusdomain" 1>&2
	fi
)

grep -w $fqdn "$DESTDIR"/$CONTROLDIR/virtualdomains >/dev/null 2>&1
if [ $? -ne 0 ] ; then
	# qmail-send, qmail-smtpd
	myecho Putting "$fqdn" into $CONTROLDIR/locals...
	echo "$fqdn" >> "$DESTDIR"$CONTROLDIR/locals
	sort -u "$DESTDIR"$CONTROLDIR/locals -o "$DESTDIR"/$CONTROLDIR/locals
	chmod 644 "$DESTDIR"$CONTROLDIR/locals
fi

# qmail-smtpd
grep -w $fqdn "$DESTDIR"/$CONTROLDIR/rcpthosts >/dev/null 2>&1
if [ $? -ne 0 ] ; then
	myecho Putting "$fqdn" into $CONTROLDIR/rcpthosts...
	echo "$fqdn" >> "$DESTDIR"$CONTROLDIR/rcpthosts
	sort -u "$DESTDIR"$CONTROLDIR/rcpthosts -o "$DESTDIR"/$CONTROLDIR/rcpthosts
	chmod 644 "$DESTDIR"$CONTROLDIR/rcpthosts
	if [ $verbose -ne 0 ] ; then
		echo "Now qmail will refuse to accept SMTP messages except to the following..."
		/bin/cat "$DESTDIR"$CONTROLDIR/rcpthosts
	fi
fi
if [ $verbose -eq 2 ] ; then
	echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
fi
}

if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR=@controldir@
fi
slash=`echo $CONTROLDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
	CONTROLDIR=SYSCONFDIR/control
fi
force=0
verbose=1
while test $# -gt 0; do
    case "$1" in
    	-*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'`
    	;;
    	*) optarg=
    	;;
    esac

    case "$1" in
    	--destdir=*)
    	destdir=$optarg
    	;;
		--verbose)
		verbose=2
    	;;
		--quiet)
		verbose=0
		;;
    	--force=*)
    	force=1
    	;;
		*)
		break
		;;
    esac
    shift
done
create_config $1
