#
# $Log: qtop.sh,v $
# Revision 1.1  2022-04-16 13:02:52+05:30  Cprogrammer
# Initial revision
#
#
#
usage ()
{
	echo "Wrong Usage"
	echo "$0 [options]"
	echo "-t    display interval"
	echo "-q	queue count"
	echo "-c	command"
}

interval=5
qcount=5
command="tail -F @logdir@/deliver.25/current|tai64nlocal"
while getopts c:i:q: opt
do
	case $opt in
		c)
		command=$OPTARG
		;;
		i)
		interval=$OPTARG
		;;
		q)
		qcount=$OPTARG
		;;
		?)
		usage $0
		exit 1
		;;
	esac
done
if [ -f /usr/bin/tput ] ; then
	height=$(tput lines)
	width=$(tput cols)
elif [ -f /usr/bin/stty ] ; then
	height=$(stty size |awk '{print $1}')
	width=$(stty size |awk '{print $2}')
else
	height=25
	width=80
fi
qread_lines=$(expr $qcount + 5)
percent=$(echo $qread_lines $height|awk '{printf "%.0f\n", 100 - 100 * $1/$2}')
exec tmux new-session -s qread -d -x $width -y $height \
	"while true; do clear; date;@prefix@/bin/qmail-qread -i; sleep $interval; done" \; \
	split-window -p $percent -f -v "$command" \; \
	attach -t qread
