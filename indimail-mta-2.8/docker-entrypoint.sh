#
# $Id: docker-entrypoint.sh,v 1.1 2019-12-08 18:07:36+05:30 Cprogrammer Exp mbhangui $
#
# $Log: docker-entrypoint.sh,v $
# Revision 1.1  2019-12-08 18:07:36+05:30  Cprogrammer
# Initial revision
#
set -e

case "$1" in
indimail)
	if [ -f /service/mysql.3306/down -a -d /var/indimail/mysqldb/data/indimail ] ; then
		/bin/rm -f /service/mysql.3306/down
	fi
   	exec PREFIX/sbin/svscan /service
;;
indimail-mta|svscan)
   	exec PREFIX/sbin/svscan /service
;;
*)
	exec "$@"
;;
esac
