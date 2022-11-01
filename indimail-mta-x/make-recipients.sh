#
# $Id: make-recipients.sh,v 1.1 2022-11-01 18:16:06+05:30 Cprogrammer Exp mbhangui $
#
LANG=C
cat SYSCONF/control/locals | while read me
do
	(
	if [ -d QMAIL/alias ] ; then
		ls QMAIL/alias/.qmail-*|cut -d- -f2-|\
			egrep -v "^Bham|^Bregister-ham|^Bregister-spam|^Bspam|^srs-default|^default" | \
			awk -F: '{print $1"@localhost"}'
	fi
	if [ -f SYSCONF/users/recipients ] ; then
		cat SYSCONF/users/recipients
	fi
	grep "/home/" /etc/passwd | awk -F: '{print $1"@localhost"}'
	grep "^=" QMAIL/users/assign | awk -F: '{print $1"@localhost"}' | cut -c2-
	)|sed -e s"{localhost{$me{" | sort -u
done

#
# $Log: make-recipients.sh,v $
# Revision 1.1  2022-11-01 18:16:06+05:30  Cprogrammer
# Initial revision
#
#
