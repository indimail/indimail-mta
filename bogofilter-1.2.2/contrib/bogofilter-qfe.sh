#!/bin/sh
#This a qmail specific bogofilter frontend script which allows the use of a centralized bogofilter
#running on an smtp mail server
#Copyright (C) Gyepi Sam <gyepi@praxis-sw.com> 2002
 
#Change this!
domain='example.com'

#Change this if you want.
sender='postmaster'
TMPDIR='/tmp'

opt="X"

case "$EXT2" in
	register-spam)		# register as spam
		opt="-s"
	;;

	register-nonspam)	# register as non-spam
		opt="-n"
	;;

	spam)			# unregister as ham
				# register as spam
		opt="-Ns"
	;;

	nonspam)		# unregister as spam
				# register as ham
		opt="-Sn"
	;;
esac

if [ "${opt}" = "X" ]; then
 
 tmpfile=$(mktemp "$TMPDIR/bogofilter-fe-$$.XXXXXX")

 cat - > $tmpfile

 #Quite a few  MUAs use Resent-* headers
 recipient=$(formail -x Resent-From < $tmpfile|grep -i $domain| tr '\n' ',')

 #but some don't
 if [ "${recipient}X" = "X" ]; then
	recipient=$SENDER
	echo "bogofilter-qfe: defaulting error response to $SENDER"
 fi

 #may need to weed out specific recipients here

 subject=$(formail -x Subject < $tmpfile)

 rm -f $tmpfile

 if [ -z "$recipient" ]; then
   exit 0
 fi

 /usr/qmail/bin/qmail-inject -f$sender@$domain $recipient<<EOF
From: "bogofilter frontend" <$sender@$domain>
To:$recipient
Subject: Re:$subject

I was unable to understand the message you sent to $RECIPIENT.

To correct a bogofilter classification:

Send mis-classified spam to bogofilter-spam@$domain
Send mis-classified nonspam to bogofilter-nonspam@$domain
 
To register new messages:

Send spam to bogofilter-register-spam@$domain
Send nonspam to bogofilter-register-nonspam@$domain

In either case, be sure to 'bounce' or 'resend' the message rather than forwarding it.

EOF

else

 exec /usr/bin/formail -c | \
 /bin/sed "/^Resent/d; /^Delivered-To/d; /^Received.*$domain/d; /^X-Bogosity/d" | \
 /usr/bin/bogofilter -d /home/bogofilter $opt

fi
