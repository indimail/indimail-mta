#! /bin/bash
#
# $Id: scramble.sh 6306 2005-11-01 00:30:59Z relson $ #
#
#  scramble -- split up files into entries assuming each entry
#                starts with a given separator
#              optionally, classify the entries
#              produce, on stdout, a random list of entries and
#                their locations and classes, or a stream of
#                entries in random order, with classifiers if given
#  needs:    POSIX compliant sh, basename rm grep awk wc perl dd
#  usage:    see function usage() starting on line 14 of this file
#  version:  0.12 (Greg Louis <glouis@dynamicro.on.ca>)

# Note: on Solaris, use /usr/xpg4/bin/sh -- /bin/sh will not work.

pid=$$

mbox=1		# mbox mode
mdir=2		# maildir mode

usage() {
    iam=$(basename $0)
    echo "Usage: $iam separator [-l] [-c classID] filename/directory [...]"
    echo "       Files contain entries, each of which begins with"
    echo "       text matching the separator.  Entries are listed or"
    echo "       output in random order."
    echo "Parameters:"
    echo "       separator is a regex used by grep that matches the"
    echo "       start of each entry."
    echo "       -l indicates that the output is to be a list of"
    echo "       entries.  If this option is not given, the output"
    echo "       consists of the entries themselves."
    echo "       classID is text used to identify the class of a"
    echo "       given entry.  If no classID values are specified,"
    echo "       this field consists of a single . character.  If"
    echo "       classID values are given and the -l option is not"
    echo "       used, each entry in the output stream is preceded"
    echo "       by a single line of the form %%-CLASS-ClassID-%%."
    echo "       No one file may contain entries of more than one"
    echo "       class."
    echo "       File and directory names may not contain blanks."
#   rm -f list.$pid
    exit 1
}

doit()
{
# the first param is the separator
    test "x$1" = "x" && usage
    if [ "$1" != "-d" ] ; then
	sep="$1"
	mode="$mbox"
    else
	mode="$mdir"
    fi
    shift

    stream=1
    classID="."

    if [ "$mode" = "$mbox" ] ; then	# mbox
	create_mbox_entries $*
    else				# maildir
	create_maildir_entries $*
    fi

    output | perl \
	-e' srand ( time() ^ ($$ + ($$ << 15)) );' \
	-e' foreach $key (<>) {' \
	-e'     $shuf{$key} = rand;' \
	-e' }' \
	-e' foreach $key (sort { $shuf{$b} <=> $shuf{$a} } keys %shuf ) {' \
	-e'     print $key;' \
	-e' }' >shuf.$pid

    cat shuf.$pid

# next line can be commented out for debugging
  rm list.$pid shuf.$pid
}

create_mbox_entries()
{
# get all the byte offsets in all the files, in one list
    while [ ${#*} -gt 0 ]; do
	if [ "$1" = "-l" ]; then
	    stream=0
	    shift
	    continue
	fi
	if [ "$1" = "-c" ]; then
	    classID=$2
	    shift 2
	    continue
	fi
	file=$1 ; shift
	if [ ! -r $file ]; then echo "$file not found"; usage; fi
	grep -a -b '^From ' $file | \
	    awk "BEGIN {FS=\":\"} {print \"$classID $file \"\$1}" >>list.$pid
	wc -c $file | awk "{print \"$classID $file \"\$1}" >>list.$pid
    done
}

create_maildir_entries()
{
# get all the byte offsets in all the files, in one list
    while [ ${#*} -gt 0 ]; do
	if [ "$1" = "-l" ]; then
	    stream=0
	    shift
	    continue
	fi
	if [ "$1" = "-c" ]; then
	    classID=$2
	    shift 2
	    continue
	fi
	file=$1 ; shift
	if [ ! -r $file ]; then echo "$file not found"; usage; fi
	for fnam in $file/* ; do echo $classID $fnam >>list.$pid ; done
    done
}

output_mbox_entries()
{
    file=""
    {
	while read classID fnam offset; do
	    if [ "$fnam" = "$file" ]; then
		let length=$offset-$oldoff
		if [ $length -gt 0 ] ; then
		    echo "$classID $fnam $oldoff $length"
		fi
		oldoff=$offset
	    else
		file=$fnam
		oldoff=0
	    fi
	done
    } < list.$pid
}

output_maildir_entries()
{
    cat list.$pid
}

output()
{
    if [ "$mode" = "$mbox" ] ; then	# mbox
	output_mbox_entries
    else				# maildir
	output_maildir_entries
    fi
}

doit "$@"
