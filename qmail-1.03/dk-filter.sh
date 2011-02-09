#
# $Log: dk-filter.sh,v $
# Revision 1.12  2010-05-04 08:37:42+05:30  Cprogrammer
# do DK signing before DKIM signing to prevent DK_SYNTAX error
#
# Revision 1.11  2009-12-10 19:25:13+05:30  Cprogrammer
# added RCS id
#
# Revision 1.10  2009-12-10 16:41:14+05:30  Cprogrammer
# continue of message gives DK_SYNTAX_ERR
#
# Revision 1.9  2009-05-04 10:30:32+05:30  Cprogrammer
# fixed argument expected error
#
# Revision 1.8  2009-04-21 20:42:44+05:30  Cprogrammer
# added check for dktest, dkim executables
#
# Revision 1.7  2009-04-20 10:06:58+05:30  Cprogrammer
# added DKSIGNOPTS
#
# Revision 1.6  2009-04-19 13:38:24+05:30  Cprogrammer
# added full set of dkim options
# replaced indimail/bin/echo with echo 1>&2
#
# Revision 1.5  2009-04-06 16:37:50+05:30  Cprogrammer
# added SIGN_PRACTICE
# use ietf standard insted of allman so that Yahoo verification does not fail
#
# Revision 1.4  2009-04-03 14:39:00+05:30  Cprogrammer
# added return status
#
# Revision 1.3  2009-04-03 08:55:29+05:30  Cprogrammer
# print error messages to stderr
#
# Revision 1.2  2009-04-02 20:36:25+05:30  Cprogrammer
# added -h option to dktest
# added -x - option to dkim
#
# Revision 1.1  2009-04-02 14:52:27+05:30  Cprogrammer
# Initial revision
#
# $Id: dk-filter.sh,v 1.12 2010-05-04 08:37:42+05:30 Cprogrammer Stab mbhangui $
#
if [ -z "$QMAILREMOTE" -a -z "$QMAILLOCAL" ]; then
	echo "dk-filter should be run by spawn-filter" 1>&2
	exit 1
fi
if [ -z "$DKSIGN" -a -z "$DKIMSIGN" -a -z "$DKVERIFY" -a -z "$DKIMVERIFY" ] ; then
	echo "Must provide at least one of DKSIGN, DKIMSIGN, DKVERIFY, DKIMVERIFY" 1>&2
	exit 1
fi
dksign=0
dkimsign=0
dkverify=0
dkimverify=0
if [ ! -z $DKSIGN ] ; then
	if [ ! -f QMAILHOME/bin/dktest ] ; then
		echo "QMAILHOME/bin/dktest: No such file or directory" 1>&2
		exit 1
	fi
	dksign=1
	if [ ! " $_SENDER" = " " ] ; then
		domain=`echo $_SENDER | cut -d@ -f2`
		# replace '%' in filename with domain
		dkkeyfn=`echo $DKSIGN | sed s{%{$domain{g`
	else
		dkkeyfn=$DKSIGN
	fi
	dkselector=`basename $dkkeyfn`
fi
if [ ! -z $DKIMSIGN ] ; then
	if [ ! -f QMAILHOME/bin/dkim ] ; then
		echo "QMAILHOME/bin/dkim: No such file or directory" 1>&2
		exit 1
	fi
	dkimsign=1
	if [ ! " $_SENDER" = " " ] ; then
		# replace '%' in filename with domain
		domain=`echo $_SENDER | cut -d@ -f2`
		dkimkeyfn=`echo $DKIMSIGN | sed s{%{$domain{g`
	else
		dkimkeyfn=$DKIMSIGN
	fi
	dkimselector=`basename $dkimkeyfn`
fi
if [ ! -z $DKVERIFY ] ; then
	if [ ! -f QMAILHOME/bin/dktest ] ; then
		echo "QMAILHOME/bin/dktest: No such file or directory" 1>&2
		exit 1
	fi
	dkverify=1
fi
if [ ! -z $DKIMVERIFY ] ; then
	if [ ! -f QMAILHOME/bin/dkim ] ; then
		echo "QMAILHOME/bin/dkim: No such file or directory" 1>&2
		exit 1
	fi
	dkimverify=1
fi
cat > /tmp/dk.$$
if [ $dkimsign -eq 1 ] ; then
	# DKIMSIGNOPTIONS="-z 1 -b 2 -x - -y $dkimselector -s $dkimkeyfn"
	set -- `getopt lqthb:c:d:i:x:z:y:s: $DKIMSIGNOPTIONS`
	bopt=0
	xopt=0
	zopt=0
	yopt=0
	sopt=0
	dkimopts="QMAILHOME/bin/dkim"
	while [ $1 != -- ]
	do
		case $1 in
		-l)
		dkimopts="$dkimopts -l"
		;;
		-q)
		dkimopts="$dkimopts -q"
		;;
		-t)
		dkimopts="$dkimopts -t"
		;;
		-h)
		dkimopts="$dkimopts -h"
		;;

		-b)
		bopt=1
		dkimopts="$dkimopts -b $2"
		shift
		;;

		-c)
		dkimopts="$dkimopts -c $2"
		shift
		;;

		-i)
		dkimopts="$dkimopts -i $2"
		shift
		;;

		-x)
		xopt=1
		dkimopts="$dkimopts -x $2"
		shift
		;;

		-z)
		zopt=1
		dkimopts="$dkimopts -z $2"
		shift
		;;

		-y)
		yopt=1
		dkimopts="$dkimopts -y $2"
		shift
		;;

		-s)
		sopt=1
		dkimopts="$dkimopts -s $2"
		shift
		;;
		esac
		shift   # next flag
	done
	if [ $zopt -eq 0 ] ; then
		dkimopts="$dkimopts -z 1"
	fi
	if [ $bopt -eq 0 ] ; then
		dkimopts="$dkimopts -b 2"
	fi
	if [ $xopt -eq 0 ] ; then
		dkimopts="$dkimopts -x -"
	fi
	if [ $yopt -eq 0 ] ; then
		dkimopts="$dkimopts -y $dkimselector"
	fi
	if [ $sopt -eq 0 ] ; then
		dkimopts="$dkimopts -s $dkimkeyfn"
	fi
	exec 0</tmp/dk.$$
	eval $dkimopts
	if [ $? -ne 0 ] ; then
		/bin/rm -f /tmp/dk.$$
		exit 1
	fi
fi
if [ $dksign -eq 1 ] ; then
	#dktest: [-f] [-b advice_length] [-c nofws|simple] [-v|-s selector] [-h] [-t#] [-r] [-T][-d dnsrecord]
	# DKSIGNOPTIONS="-z 1 -b 2 -x - -y $dkimselector -s $dkimkeyfn"
	set -- `getopt hrb:c:s: $DKSIGNOPTIONS`
	dkopts="QMAILHOME/bin/dktest"
	sopt=0
	while [ $1 != -- ]
	do
		case $1 in
		-h)
		dkopts="$dkopts -h"
		;;
		-r)
		dkopts="$dkopts -r"
		;;

		-b)
		dkopts="$dkopts -b $2"
		shift
		;;

		-c)
		dkopts="$dkopts -c $2"
		shift
		;;

		-s)
		sopt=1
		dkopts="$dkopts -s $2"
		shift
		;;
		esac
		shift   # next flag
	done
	if [ $sopt -eq 0 ] ; then
		dkopts="$dkopts -s $dkkeyfn"
	fi
	exec 0</tmp/dk.$$
	#QMAILHOME/bin/dktest -h -s $dkkeyfn
	eval $dkopts
	exit_val=$?
	if [ $exit_val -ne 0 ] ; then
		if [ $exit_val -eq 6 ] ; then
			exec 0</tmp/dk.$$
			/bin/rm -f /tmp/dk.$$
			cat
			exit $?
		fi
		/bin/rm -f /tmp/dk.$$
		exit $exit_val
	fi
fi
if [ $dkimverify -eq 1 ] ; then
	practice=$SIGN_PRACTICE
	if [ " $practice" = " " ] ; then
		practice=0
	elif [ " $practice" = " ssp" ] ; then
		practice=1
	elif [ " $practice" = " adsp" ] ; then
		practice=2
	fi
	exec 0</tmp/dk.$$
	QMAILHOME/bin/dkim -p $practice -v
	ret=$?
	case $ret in
		14)
		/bin/rm -f /tmp/dk.$$
		exit 100
		;;
		88)
		/bin/rm -f /tmp/dk.$$
		exit 111
		;;
	esac
	if [ $ret -lt 0 ] ; then
		/bin/rm -f /tmp/dk.$$
		exit 1
	fi
fi
if [ $dkverify -eq 1 ] ; then
	exec 0</tmp/dk.$$
	QMAILHOME/bin/dktest -v
	if [ $? -ne 0 ] ; then
		/bin/rm -f /tmp/dk.$$
		exit 1
	fi
fi
exec 0</tmp/dk.$$
/bin/rm -f /tmp/dk.$$
cat
exit $?
