#
# $Log: qmailconfig.sh,v $
# Revision 1.6  2011-04-23 09:35:25+05:30  Cprogrammer
# use hostname command from /var/indimail/sbin/hostname
#
# Revision 1.5  2009-11-26 18:13:43+05:30  Cprogrammer
# fixed syntax error
# renamed config.sh to qmailconfig.sh
#
#
# $Id: qmailconfig.sh,v 1.6 2011-04-23 09:35:25+05:30 Cprogrammer Exp mbhangui $
#
if [ -x ./hostname ] ; then
	HOSTNAME_CMD=./hostname
elif [ -x QMAIL/sbin/hostname ] ; then
	HOSTNAME_CMD=QMAIL/sbin/hostname
else
	HOSTNAME_CMD=hostname
fi
$HOSTNAME_CMD | tr '[A-Z]' '[a-z]' |
(
if read host
then
	echo Your hostname is "$host".
	(
	if [ -x QMAIL/bin/dnsfq ] ; then
		QMAIL/bin/dnsfq "$host" | tr '[A-Z]' '[a-z]'
	else
		./dnsfq "$host" | tr '[A-Z]' '[a-z]'
	fi
	) |
	(
	if read fqdn
	then
		echo Your host\'s fully qualified name in DNS is "$fqdn".
		echo Putting "$fqdn" into control/me...
		echo "$fqdn" > QMAIL/control/me
		chmod 644 QMAIL/control/me
		(
			echo "$fqdn" | sed 's/^\([^\.]*\)\.\([^\.]*\)\./\2\./' |
			(
				read ddom
				echo Putting "$ddom" into control/defaultdomain...
				echo "$ddom" > QMAIL/control/defaultdomain
				chmod 644 QMAIL/control/defaultdomain
			)
		)
		(
			echo "$fqdn" | sed 's/^.*\.\([^\.]*\)\.\([^\.]*\)$/\1.\2/' |
			(
				read pdom
				echo Putting "$pdom" into control/plusdomain...
				echo "$pdom" > QMAIL/control/plusdomain
				chmod 644 QMAIL/control/plusdomain
			)
		)
		echo ' '
		echo Checking local IP addresses:
		: > QMAIL/control/locals
		chmod 644 QMAIL/control/locals
		(
			if [ -x QMAIL/bin/dnsip ] ; then
				QMAIL/bin/dnsip "$fqdn"
			else
				./dnsip "$fqdn"
			fi
			if [ -x QMAIL/bbin/ipmeprint ] ; then
				QMAIL/bin/ipmeprint | awk '{print $3}'
			else
				./ipmeprint | awk '{print $3}'
			fi
		) | sort -u |
		(
			while read localip
			do
				echo "$localip: " | tr -d '\012'
				(
				if [ -x QMAIL/bin/dnsptr ] ; then
					QMAIL/bin/dnsptr "$localip" 2>/dev/null
				else
					./dnsptr "$localip" 2>/dev/null
				fi
				) | 
				(
					if read local
					then
						echo Adding "$local" to control/locals...
						echo "$local" >> QMAIL/control/locals
					else
						echo PTR lookup failed. I assume this address has no DNS name.
					fi
				)
			done
		)
		echo ' '
		echo If there are any other domain names that point to you,
		echo you will have to add them to QMAIL/control/locals.
		echo You don\'t have to worry about aliases, i.e., domains with CNAME records.
		echo ' '
		echo Copying QMAIL/control/locals to QMAIL/control/rcpthosts...
		cp QMAIL/control/locals QMAIL/control/rcpthosts
		chmod 644 QMAIL/control/rcpthosts
		echo 'Now qmail will refuse to accept SMTP messages except to those hosts.'
		echo 'Make sure to change rcpthosts if you add hosts to locals or virtualdomains!'
	else
		echo Sorry, I couldn\'t find your host\'s canonical name in DNS.
		echo You will have to set up control/me yourself.
	fi
	)
else
	echo Sorry, I couldn\'t find your hostname.
	echo You will have to set up control/me yourself.
fi
)
