#
# $Id: docker-entrypoint.sh,v 1.4 2020-04-29 11:31:56+05:30 Cprogrammer Exp mbhangui $
#
# $Log: docker-entrypoint.sh,v $
# Revision 1.4  2020-04-29 11:31:56+05:30  Cprogrammer
# removed deletion of mysql.3306/down
#
# Revision 1.3  2020-03-21 23:41:17+05:30  Cprogrammer
# use envdir to set env variables in /service/.svscan/variables
#
# Revision 1.2  2020-03-20 15:08:32+05:30  Cprogrammer
# display command being run during docker entrypoint
#
# Revision 1.1  2019-12-08 18:07:36+05:30  Cprogrammer
# Initial revision
#
set -e

case "$1" in
indimail|indimail-mta|svscan)
	if [ -d /service/.svscan/variables ] ; then
   		echo "docker-entrypoint: [$1] PREFIX/bin/envdir /service/.svscan/variables PREFIX/sbin/svscan /service"
   		exec PREFIX/bin/envdir /service/.svscan/variables PREFIX/sbin/svscan /service
	else
   		echo "docker-entrypoint: [$1] executing PREFIX/sbin/svscan /service"
   		exec PREFIX/sbin/svscan /service
	fi
;;
*)
	echo "docker-entrypoint: executing $@"
	exec "$@"
;;
esac
