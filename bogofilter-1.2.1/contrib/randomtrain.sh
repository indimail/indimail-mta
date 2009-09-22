#! /bin/bash
#
# $Id: randomtrain.sh 6306 2005-11-01 00:30:59Z relson $ #
#
#  randomtrain -- bogofilter messages from files in random order
#                 and train if the result is wrong or uncertain
#  needs:    POSIX compliant sh, basename rm grep awk wc perl dd bogofilter
#  usage:    see function usage() of this file
#
#  original author: 	Greg Louis <glouis@dynamicro.on.ca>
#  modified by: 	David Relson <relson@osagesoftware.com>

# Note: on Solaris, use /usr/xpg4/bin/sh -- /bin/sh will not work.

BOGOFILTER="bogofilter"

usage() {
    name=$(basename $0)
    echo "Usage: $name [-d bogodir] [-p pid] [-c cfg] [-]n|s filename [-]n|s filename [...]"
    echo "       Messages contained in the files are fed to bogofilter"
    echo "       in random order.  If bogofilter is wrong or uncertain"
    echo "       about whether a message is spam, that message is used"
    echo "       for training, with bogofilter's -s or -n option."
    echo ""
    echo "Parameters:"
    echo "       bogodir is where bogofilter's wordlists files are kept"
    echo "       (bogodir defaults to $HOME/.bogofilter)."
    echo "       n (or -n) indicates that the next file contains only nonspams."
    echo "       s (or -s) means it contains only spams."
    echo "       No one file may contain both spams and nonspams."
    echo "       Filenames may not contain blanks."
    echo ""
    echo "       c (or -c) indicates that the next file is the config file."
    echo "       p (or -p) indicates that a pid comes next (used for re-running a test)."
    echo "NB:    At least one spam and one nonspam file are needed!"
    exit 1
}

train_mbox() {
    # go through the list, extract the messages, eval with bogofilter
    # and train if bogofilter is wrong or uncertain
    cnt=0; nspam=0; ngood=0; rspam=0; rgood=0;
    echo " spam  reg   good  reg"
    while read expect fnam offset length; do
	let cnt=cnt+1
	dd if=$fnam bs=1 skip=$offset count=$length 2>/dev/null >msg.$pid
	result=$($BOGOFILTER -t -v -d $bogodir $cfg -I msg.$pid)
	got=$?	# 0=spam, 1=ham, 2=unsure, 3=err
	if [ "$expect" = "s" ]; 
	then let nspam=$nspam+1
	else let ngood=$ngood+1; fi
	if [ $got -eq 0 ]; then got="s"; elif [ $got -eq 1 ]; then got="n"; fi
	if [ "$got" != "$expect" ]; then
	    if [ "$expect" = "s" ]; 
	    then let rspam=$rspam+1
	    else let rgood=$rgood+1; fi
	    # comment out the next line for dry-run testing
	    $BOGOFILTER -$expect $cfg -d $bogodir -I msg.$pid
	fi
#	echo $expect $got $result, $nspam $rspam, $ngood $rgood
	n=$(expr \( $nspam + $ngood \) % 10)
	test $n -eq 0 && \
	    printf "\r%5d%5d  %5d%5d " $nspam $rspam $ngood $rgood
    done
    printf "\r%5d%5d  %5d%5d\n" $nspam $rspam $ngood $rgood
}

train_maildir() {
    # go through the list, extract the messages, eval with bogofilter
    # and train if bogofilter is wrong or uncertain
    cnt=0; nspam=0; ngood=0; rspam=0; rgood=0;
    echo " spam  reg   good  reg"
    while read expect fnam; do
	let cnt=cnt+1
	result=$($BOGOFILTER -t -v -d $bogodir $cfg -I $fnam)
	got=$?	# 0=spam, 1=ham, 2=unsure, 3=err
	if [ "$expect" = "s" ]; 
	then let nspam=$nspam+1
	else let ngood=$ngood+1; fi
	if [ $got -eq 0 ]; then got="s"; elif [ $got -eq 1 ]; then got="n"; fi
	if [ "$got" != "$expect" ]; then
	    if [ "$expect" = "s" ]; 
	    then let rspam=$rspam+1
	    else let rgood=$rgood+1; fi
	    # comment out the next line for dry-run testing
	    $BOGOFILTER -$expect $cfg -d $bogodir -I $fnam
	fi
#	echo $expect $got $result, $nspam $rspam, $ngood $rgood
	n=$(expr \( $nspam + $ngood \) % 10)
	test $n -eq 0 && \
	    printf "\r%5d%5d  %5d%5d " $nspam $rspam $ngood $rgood
    done
    printf "\r%5d%5d  %5d%5d\n" $nspam $rspam $ngood $rgood
}

# Execution begins here...

cnt=0;
cmd=""
cfg='-C'
pid=$$

mode=0		# unknown
mbox=1		# mbox mode
mdir=2		# maildir mode

while [ ${#*} -gt 1 ]; do
    indic=$1 ; shift
    case "$indic" in
	s|-s|n|-n)
	    path=$1 ; shift
	    cmd="$cmd -c $indic $path"
	    if [ ! -r $path ]; then echo "file '$path' not found"; usage; fi
	    let cnt=cnt+2
	    if [ $mode = 0 ] ; then
		if [ -f $path ]; then	# if mbox
		    mode=$mbox
		else			# else maildir
		    mode=$mdir
		fi
	    fi
	    ;;
	c|-c)
	    file=$1 ; shift
	    cfg="-c $file"
	    if [ ! -r $file ]; then echo "file '$file' not found"; usage; fi
	    ;;
	p|-p)
	    pid=$1; shift;
	    ;;
	d|-d)
	    [ -n "$bogodir" ] && usage
	    bogodir=$1 ; shift
	    if [ ! -d $bogodir ]; then echo "directory '$bogodir' not found"; usage; fi
	    [ ! -f $bogodir/spamlist.db ] && $BOGOFILTER -d $bogodir -s -C < /dev/null
	    [ ! -f $bogodir/goodlist.db ] && $BOGOFILTER -d $bogodir -n -C < /dev/null
	    ;;
	*)
	    usage
    esac
done

# if the first param isn't s or n, treat it as a directory
test -z "$bogodir" && bogodir="${HOME}/.bogofilter"

# check for an even number of s/n files >= 4
test $cnt -ge 4 || usage
let n=$cnt%2
test $n -eq 0 || usage

# params may be ok, here goes...

# create a shuffled list, with lengths
if [ ! -f scram.$pid ] ; then
    if [ "$mode" = "$mbox" ] ; then	# mbox
	scramble.sh "^From " -l $cmd > scram.$pid
    else				# maildir
	scramble.sh -d -l $cmd > scram.$pid
    fi
fi

if [ "$mode" = "$mbox" ] ; then	# mbox
    train_mbox <scram.$pid
else				# maildir
    train_maildir <scram.$pid
fi

if [ $pid = $$ ] ; then
# next line can be commented out for debugging
    rm -f scram.$pid msg.$pid cfg.$pid
fi
