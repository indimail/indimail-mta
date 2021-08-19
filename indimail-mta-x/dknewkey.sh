#
# $Log: dknewkey.sh,v $
# Revision 1.7  2021-08-19 19:54:39+05:30  Cprogrammer
# added options to print, remove, generate DKIM keys
#
# Revision 1.6  2019-02-15 17:11:18+05:30  Cprogrammer
# fixed checking of argument count
#
# Revision 1.5  2017-03-09 16:38:24+05:30  Cprogrammer
# use full path of openssl
#
# Revision 1.4  2010-05-16 19:59:48+05:30  Cprogrammer
# fix for Mac OS X
#
# Revision 1.3  2004-11-02 20:48:31+05:30  Cprogrammer
# fixed error when dknewkey was called without arguments
#
# Revision 1.2  2004-10-21 21:54:25+05:30  Cprogrammer
# create public key file
#
# Revision 1.1  2004-10-20 20:40:56+05:30  Cprogrammer
# Initial revision
#
#

usage()
{
	echo "Usage: dknewkey [options] keyfile"
	echo "options"
	echo "       [-p | --print]          : print DKIM public keys"
	echo "       [-r | --remove]         : remove DKIM keys"
	echo "       [-d | --domain domain]  : domain name"
	echo "       [-b | --bits   size]    : DKIM private key size"
	echo "       [-f]                    : force DKIM private key creation"
	exit $1
}

# This function comes from Tatsuya Yokota
# https://github.com/kotaroman/domainkey
function split_str()
{
	local STR="$1"
	local START=0
	local STR_COUNT=0
	local LINE=""
	local SPLIT=255

	while true
	do
		START=$STR_COUNT
		LINE=${STR:$START:$SPLIT}
		if [ ${#LINE} -eq 0 ]; then
			break
		fi
		if [ $START -ne 0 ]; then
			if [ ${#LINE} -ne $SPLIT ]; then
				printf "\t\"${LINE}\""
			else
				printf "\t\"${LINE}\"\n"
			fi
		else
			printf "\"${LINE}\"\n"
		fi
		STR_COUNT=$(expr $STR_COUNT + $SPLIT)
	done
}

print_key()
{
	if [ $bits -lt 2048 ] ; then
		printf "%s._domainkey.%s. IN TXT (\"v=DKIM1; k=rsa; t=y; p=%s\")\n" "$1" "$2" "$3" > $1.pub
	else
		printf "%s._domainkey.%s. IN TXT (" "$1" "$2"
		split_str "v=DKIM1; k=rsa; t=y; p=$3"
		printf ")\n"
	fi
}

controldir=@qsysconfdir@/control
options=$(getopt -a -n dknewkey -o "prfd:b:" -l print,remove,force,domain:,bits: -- "$@")
if [ $? != 0 ]; then
	usage 100
fi

do_print=0
remove=0
force=0
bits=1024
domain=""
eval set -- "$options"
while :
do
	case "$1" in
	-f | --force)
	force=1
	shift 1
	;;
	-p | --print)
	do_print=1
	shift 1
	;;
	-r | --remove)
	remove=1
	shift 1
	;;
	-d | --domain)
	domain="$2"
	shift 2
	;;
	-s | --selector)
	selector="$2"
	shift 2
	;;
	-b | --bits)
	bits="$2"
	shift 2
	;;
	--) # end of options
	shift
	break
	;;
	*)
	echo "Unexpected option: $1 - this should not happen."
	usage 100
	;;
	esac
done
if [ $do_print -eq 0 -a $# -ne 1 ] ; then
	usage 100
else
	selector=$1
fi
if [ -z "$domain" ] ; then
	dir=$controldir/domainkeys
	if [ -f $controldir/defaultdomain ] ; then
		domain=$(cat $controldir/defaultdomain)
	else
		domain=$([ -n "$HOSTNAME" ] && echo "$HOSTNAME" || uname -n)
	fi
else
	dir=$controldir/domainkeys/$domain
fi
if [ ! -d $dir ] ; then
	if (! mkdir -p $dir || ! chown indimail:qmail $dir || ! chmod 755 $dir) ; then
		exit 1
	fi
fi
cd $dir
if [ $? -ne 0 ] ; then
	exit 2
fi
if [ $do_print -eq 1 ] ; then
	if [ -n "$selector" ] ; then
		if [ -f $selector.pub ] ; then
			echo
			echo "DKIM TXT record for $domain with selector=$selector file $dir/$selector.pub"
			cat $selector.pub
			echo "------------------------------------------------------"
			exit 0
		else
			echo "DKIM TXT record for $domain with selector=$selector does not exist"
			exit 2
		fi
	else
		for i in $(find . -name '*'.pub -print)
		do
			selector=$(basename $i | cut -d. -f1)
			t=$(echo $i | cut -c3-)
			echo
			echo "DKIM TXT record for $domain with selector=$selector file $dir/$t"
			cat $i
			echo "------------------------------------------------------"
		done
		exit 0
	fi
elif [ $remove -eq 1 ] ; then
	echo "Removing DKIM Keys $selector, $selector.pub"
	/bin/rm -f $selector $selector.pub
	files=$(ls)
	cd ..
	if [ -d $domain -a -z "$files" ] ; then
		echo "Removing empty directory $domain"
		rmdir --ignore-fail-on-non-empty $dir
	fi
	exit 0
else
	if [ -f $selector -a $force -eq 0 ] ; then
		echo "DKIM private key $selector exists. Skipping private key generation" 1>&2
		skip_private_key=1
	else
		skip_private_key=0
	fi
	err=$(mktemp -t dkkeyXXXXXXXXXX)
	if [ $? -ne 0 ] ; then
		echo "Unable to create temp files" 1>&2
		exit 1
	fi
	exec 3>&2 # save stderr in fd 3
	exec 2>$err
	exec 0<$err
	/bin/rm -f $err
	if [ $skip_priv_key -ne 1 ] ; then
		echo "Generating DKIM private key selector=$selector keysize=$bits in $dir"
		/usr/bin/openssl genrsa -out $selector $bits
		if [ $? -ne 0 ] ; then
			/bin/cat
			exit 1
		fi
	fi
	pubkey=$(/usr/bin/openssl rsa -in $selector -pubout -outform PEM | grep -v '^--' | tr -d '\n')
	if [ $? -ne 0 ] ; then
		/bin/cat
		exit 1
	fi
	exec 2>&3 # restore stderr
	echo "Generating DKIM public key selector=$selector keysize=$bits in $dir"
	print_key "$selector" "$domain" "$pubkey" > $selector.pub
	if [ $? -ne 0 ] ; then
		exit 1
	fi
	if ( ! chown indimail:qmail $selector $selector.pub || ! chmod 640 $selector || ! chmod 644 $selector.pub) ; then
		exit 1
	fi
	if [ -f $selector ] ; then
		echo "DKIM TXT record for $domain with selector=$selector in $dir"
		cat $selector.pub
		echo "------------------------------------------------------"
		exit 0
	else
		echo "DKIM TXT record for $domain with selector=$selector does not exist in $dir"
		exit 2
	fi
fi
