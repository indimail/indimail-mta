#
# $Log: qmailconfig.sh,v $
# Revision 1.10  2017-03-09 16:39:20+05:30  Cprogrammer
# FHS changes
#
# Revision 1.9  2016-06-17 17:26:18+05:30  Cprogrammer
# FHS compliance
#
# Revision 1.8  2016-06-05 13:20:18+05:30  Cprogrammer
# use PREFIX instead of QMAIL for binary prefix
#
# Revision 1.7  2016-05-17 23:11:42+05:30  Cprogrammer
# fix for configurable control directory
#
# Revision 1.6  2011-04-23 09:35:25+05:30  Cprogrammer
# use hostname command from /var/indimail/sbin/hostname
#
# Revision 1.5  2009-11-26 18:13:43+05:30  Cprogrammer
# fixed syntax error
# renamed config.sh to qmailconfig.sh
#
#
# $Id: qmailconfig.sh,v 1.10 2017-03-09 16:39:20+05:30 Cprogrammer Exp mbhangui $
#
if [ -x ./hostname ] ; then
	HOSTNAME_CMD=./hostname
elif [ -x PREFIX/sbin/hostname ] ; then
	HOSTNAME_CMD=PREFIX/sbin/hostname
else
	HOSTNAME_CMD=hostname
fi
if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR=@controldir@
fi
slash=`echo $CONTROLDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
	cd SYSCONFDIR
fi

$HOSTNAME_CMD | tr '[A-Z]' '[a-z]' |
(
if read host
then
	echo Your hostname is "$host".
	(
	if [ -x LIBEXEC/dnsfq ] ; then
		LIBEXEC/dnsfq "$host" | tr '[A-Z]' '[a-z]'
	else
		./dnsfq "$host" | tr '[A-Z]' '[a-z]'
	fi
	) |
	(
	if read fqdn
	then
		echo Your host\'s fully qualified name in DNS is "$fqdn".
		echo Putting "$fqdn" into $CONTROLDIR/me...
		echo "$fqdn" > $CONTROLDIR/me
		chmod 644 $CONTROLDIR/me
		(
			echo "$fqdn" | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./' |
			(
				read ddom
				echo Putting "$ddom" into $CONTROLDIR/defaultdomain...
				echo "$ddom" > $CONTROLDIR/defaultdomain
				chmod 644 $CONTROLDIR/defaultdomain
			)
		)
		(
			echo "$fqdn" | sed 's/^.*\.\([^\.]*\)\.\([^\.]*\)$/\1.\2/' |
			(
				read pdom
				echo Putting "$pdom" into $CONTROLDIR/plusdomain...
				echo "$pdom" > $CONTROLDIR/plusdomain
				chmod 644 $CONTROLDIR/plusdomain
			)
		)
		echo ' '
		echo Checking local IP addresses:
		: > $CONTROLDIR/locals
		chmod 644 $CONTROLDIR/locals
		(
			if [ -x LIBEXEC/dnsip ] ; then
				LIBEXEC/dnsip "$fqdn"
			else
				./dnsip "$fqdn"
			fi
			if [ -x LIBEXEC/ipmeprint ] ; then
				LIBEXEC/ipmeprint | awk '{print $3}'
			else
				./ipmeprint | awk '{print $3}'
			fi
		) | sort -u |
		(
			while read localip
			do
				echo "$localip: " | tr -d '\012'
				(
				if [ -x LIBEXEC/dnsptr ] ; then
					LIBEXEC/dnsptr "$localip" 2>/dev/null
				else
					./dnsptr "$localip" 2>/dev/null
				fi
				) | 
				(
					if read local
					then
						echo Adding "$local" to $CONTROLDIR/locals...
						echo "$local" >> $CONTROLDIR/locals
					else
						echo PTR lookup failed. I assume this address has no DNS name.
					fi
				)
			done
		)
		echo ' '
		echo If there are any other domain names that point to you,
		echo you will have to add them to $CONTROLDIR/locals.
		echo You don\'t have to worry about aliases, i.e., domains with CNAME records.
		echo ' '
		echo Copying $CONTROLDIR/locals to $CONTROLDIR/rcpthosts...
		cp $CONTROLDIR/locals $CONTROLDIR/rcpthosts
		chmod 644 $CONTROLDIR/rcpthosts
		echo 'Now qmail will refuse to accept SMTP messages except to those hosts.'
		echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
	else
		echo Sorry, I couldn\'t find your host\'s canonical name in DNS.
		echo You will have to set up $CONTROLDIR/me yourself.
	fi
	)
else
	echo Sorry, I couldn\'t find your hostname.
	echo You will have to set up $CONTROLDIR/me yourself.
fi
)
