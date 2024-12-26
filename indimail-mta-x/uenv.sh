#
# $Id: uenv.sh,v 1.1 2024-12-27 01:04:35+05:30 Cprogrammer Exp mbhangui $
#

do_clear=0
while getopts c:i:q: opt
do
	case $opt in
		c)
		do_clear=1
		shift
		;;
		?)
		echo "USAGE: $0 [-c] child" 1>&2
		exit 1
		;;
	esac
done

if [ $# -lt 1 ] ; then
	echo "USAGE: $0 [-c] child" 1>&2
	exit 111
fi
if [ -n "$SKIP_LOCAL_ENVIRONMENT" -o -z "$HOME" ] ; then
	exec $*
fi
home=$HOME
if [ ! -d $HOME/.defaultqueue ] ; then
	exec $*
fi
if [ -s $HOME/.defaultqueue/QUEUE_BASE ] ; then
	if [ $do_clear -eq 1 ] ; then
		exec @prefix@/bin/envdir -c $HOME/.defaultqueue env HOME=$home $*
	else
		exec @prefix@/bin/envdir $HOME/.defaultqueue $*
	fi
else
	if [ $do_clear -eq 1 ] ; then
		exec @prefix@/bin/envdir -c $HOME/.defaultqueue envdir @controldir@/defaultqueue env HOME=$home $*
	else
		exec @prefix@/bin/envdir $HOME/.defaultqueue envdir @controldir@/defaultqueue $*
	fi
fi
