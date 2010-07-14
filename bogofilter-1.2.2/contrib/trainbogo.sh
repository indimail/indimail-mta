#!/bin/sh

# Train bogofilter from a ham and spam corpus
#
# Copyright 2003 by Trevor Harrison (trevor-trainbogo@harrison.org)
#
# This file is released under the GPL. See http://www.gnu.org/licenses/gpl.txt

# $Id: trainbogo.sh 4431 2004-05-30 20:25:16Z m-a $ #

# Note:  this script has not yet had bogofilter maintainer review.
# Security concerned people should not run it if in doubt about its security.

usage()
{
    echo "USAGE:"
    echo
    echo "  trainbogo.sh [options]"
    echo
    echo "OPTIONS:"
    echo
    echo "  Required arguments:"
    echo "  -H hamdir             points to directory with all your ham"
    echo "  -S spamdir            points to directory will all your spam"
    echo
    echo "  Optional arguments:"
    echo "  -s statdir            directory where stat and tmp files are created."
    echo "                        default is ./stats.tmp"
    echo "  -b pathtobogofilter   points to the bogofilter executable,"
    echo "                        with any bogofilter options you need."
    echo "                        ex. -b \"/usr/local/bin/bogofilter -d /etc/bogodb\""
    echo "  -f                    force rebuild of ham and spam directory index.  Will"
    echo "                        cause msgs to be sorted into new order unless"
    echo "                        -p and -t are used."
    echo "  -c                    cleanup statdir when done. (default is not to)"
    echo "  -p rndseed            specify the pid.timestamp used to randomize the msgs."
    echo "                        ex. -p 5432.1049498805"
    echo "  -m                    don't test or train bogofilter, just show cached stats."
    echo "  -n                    don't train bogofilter, just test."
    echo "  -q                    don't show stats or dots. (quiet)"
    echo "  -h                    show help."
    echo
}

help()
{
    echo "trainbogo.sh"
    echo
    echo "  Train bogofilter from a qmail maildir type ham and spam corpus"
    echo
    echo "    This script relies on you having seperated your qmail maildir messages into"
    echo "    ham and spam directories.  This script randomizes the message order, and"
    echo "    then feeds each message in turn into bogofilter, noting if bogofilter"
    echo "    correctly identified the message as ham or spam. If mis-identified, it"
    echo "    trains bogofilter with that message, and then re-tests to see if bogofilter"
    echo "    correctly identifies the message."
    echo
    echo "    When I've used this script on my ham/spam collection, it takes about 4"
    echo "    consecutive executions to get my wordlists to a 0 false positive state."
    echo "    Just because this script reports 0 failed trainings doesn't mean that you"
    echo "    are ready to go.  Run the script a second time to make sure.  You should"
    echo "    keep running the script until you get 0 misdetections and, of course, 0"
    echo "    retrain failed's."
    echo
    echo "  While running, trainbogo.sh will write some dots and dashes to the screen."
    echo
    echo "    . = successfully categorized the message."
    echo "    - = failed to categorized the message, and training was turned off (-n)."
    echo "    + = successfully categorized the message after being retrained."
    echo "    f = failed to categorize the message after training."
    echo
    echo "  The results of the testing can be found in the statsdir.  Log files have"
    echo "  the filename of each message that match the logfile name:"
    echo
    echo "      trainbogo.log.[0,1].[success,fail]"
    echo "         0 = spam message log"
    echo "         1 = ham message log"
    echo "         success/fail = were/weren't correctly categorized."
    echo
    usage
}

verbose()
{
    [ -n "${verbose}" ] && echo $@
}

normal()
{
    [ -z "${quiet}" ] && echo $@
}

normaln()
{
    [ -z "${quiet}" ] && printf "%s" "$*"
}

cleanup()
{
    verbose "Performing cleanup"

    [ -z "${log}" ] || [ -z "${list}" ] || [ "${docleanup}" != "y" ] && return

    rm -f	${log}.[01].success ${log}.[01].fail \
	${log}.[01].train.success ${log}.[01].train.fail \
	${list}

    [ "${madestatsdir}" = "y" ] && [ -n "${statsdir}" ] && rmdir --ignore-fail-on-non-empty "${statsdir}"
}

dofilelist=
dotrain=y
dotest=y
docleanup=
verbose=
quiet=
statsdir="${PWD}/stats.tmp/"
origstatsdir="${statsdir}"
bf=bogofilter

while getopts "H:S:s:b:p:fcmnqvh" optname; do

    case "${optname}" in
	
	"H")	hamdir="$OPTARG" ;;
	"S")	spamdir="$OPTARG" ;;
	"s")	statsdir="$OPTARG" ;;
	"b")	bf="$OPTARG";;
	"f")	dofilelist=y ;;
	"c")	docleanup=y ;;
	"p")	rndseed=$OPTARG ;;
	"m")	dotest= ; dotrain= ;;
	"n")	dotrain= ;;
	"q")	quiet=y ;;
	"v")	verbose=y ;;
	"h")	help; exit ;;
    esac

done

# Check for required options
[ -z "${hamdir}" ] || [ ! -d "${hamdir}" ] && echo "Missing or bad -H option" && usage && exit
[ -z "${spamdir}" ] || [ ! -d "${spamdir}" ] && echo "Missing or bad -S option" && usage && exit
[ -z "${statsdir}" ] && echo "Bad statsdir option" && usage && exit

# make the stats dir if its missing, but only if its the default stats dir and not user specified
[ "${statsdir}" = "${origstatsdir}" ] && [ ! -d "${statsdir}" ] && mkdir "${statsdir}" && madestatsdir=y
[ ! -d "${statsdir}" ] && echo "Missing statsdir (-s option)" && exit

# check for bogofilter
bfbin=$(which ${bf%% *})
[ $? -ne 0 ] && echo "Missing bogofilter, not in path? (${bf})" && exit
[ ! -x "${bfbin}" ] && echo "Missing or bad bogofilter binary! (${bf})" && exit

list="${statsdir}/trainbogo.filenames.txt"
log="${statsdir}/trainbogo.log"

# Init log files
if [ ! -f "${log}.0.success" ] || [ -n "${dotest}" ] || [ -n "${dotrain}" ] ; then
    verbose "init log files"
    >"${log}.0.success"
    >"${log}.1.success"
    >"${log}.0.fail"
    >"${log}.1.fail"
    >"${log}.0.train.success"
    >"${log}.0.train.fail"
    >"${log}.1.train.success"
    >"${log}.1.train.fail"
fi

# First make a randomly sorted list of all the ham and spam files (if needed)
if [ ! -f "${list}" ] || [ -n "${dofilelist}" ]; then
    # MD5 all the spam and ham

    [ -z "${rndseed}" ] && rndseed="$$.$(date +%s)"

    normal "MD5'ing ham and spam corpus, rndseed used: ${rndseed}"

    >"${list}"

    for i in "${hamdir}"/* "${spamdir}"/*
      do
      [ ! -f "${i}" ] && continue
      md5=$(printf "%s" "${rndseed}${i}" | md5sum | sed "s/  -//")
      echo "${md5}  ${i}" >> "${list}"
    done

    [ $(wc -l < "${list}") -eq 0 ] && echo "No files to work on!!!" && exit

    # This randomizes the file names by sorting on the md5 hash
    normal "Randomizing ham and spam"
    sort "${list}" > "${list}.tmp"
    mv -f "${list}.tmp" "${list}"

    # Drop the hash
    sed "s/^.\{32\}  \(.*\)/\1/" < "${list}" > "${list}.tmp"
    mv -f "${list}.tmp" "${list}"

    # Put expected bogofilter error levels in front of each filename
    # Using @'s for sed's rule delimiter because ${hamdir} can have /'s.
    # Hopefully there won't be any @'s in the ham/spam dir name.
    sed "s@^${hamdir}\(.*\)@1 ${hamdir}\\1@g; s@^${spamdir}\(.*\)@0 ${spamdir}\\1@g" < "${list}" > "${list}.tmp"
    mv -f "${list}.tmp" "${list}"
fi

# Read each filename from the filelist and test and train bogofilter.
if [ -n "${dotest}" ] || [ -n "${dotrain}" ]; then
    normal "Training bogofilter"
    (while read spamstatus fname
	do
	normaln  "${lastdot}"
	bogotest=$(${bf} -v < "${fname}")
	ret=$?
	if [ ${spamstatus} -eq ${ret} ]; then	# bogofilter detected this message correctly
	    echo "${fname}" >> "${log}.${spamstatus}.success"
	    lastdot="."
	    continue
	fi

	# Bogofilter failed to detect the msg correctly
	echo "${fname}" >> "${log}.${spamstatus}.fail"
	lastdot="-"

	[ -z "${dotrain}" ] && continue

	# Set the bogofilter option for training
	if [ ${spamstatus} -eq 0 ]; then
	    bfopt="-s"
	else
	    bfopt="-n"
	fi

	# Train bogofilter
	${bf} ${bfopt} < "${fname}"

	# Test again
	bogotest=$(${bf} -v < "${fname}")
	ret=$?

	# Did it train successfully?
	if [ ${spamstatus} -eq ${ret} ]; then
	    testresult="success"
	    lastdot="+"
	else
	    testresult="fail"
	    lastdot="f"
	fi
	
	# Log train result
	echo "${fname}" >> "${log}.${spamstatus}.train.${testresult}"
	done) < ${list}
fi

echo
echo

if [ -z "${quiet}" ]; then

    total_msg=$(wc -l < "${list}")

    total_ham_msg=$(ls "${hamdir}" | wc -l)
    total_ham_success=$(wc -l < "${log}.1.success")
    total_ham_fail=$(wc -l < "${log}.1.fail")
    total_ham_train_fail=$(wc -l < "${log}.1.train.fail")

    total_spam_msg=$(ls "${spamdir}" | wc -l)
    total_spam_success=$(wc -l < "${log}.0.success")
    total_spam_fail=$(wc -l < "${log}.0.fail")
    total_spam_train_fail=$(wc -l < "${log}.0.train.fail")

    echo "Total   messages: ${total_msg}"
    echo
    echo "Total        ham: ${total_ham_msg}"
    echo "Misdetected  ham: ${total_ham_fail}"
    [ -n "${dotrain}" ] && echo "    retrain fail: ${total_ham_train_fail}"
    echo
    echo "Total       spam: ${total_spam_msg}"
    echo "Misdetected spam: ${total_spam_fail}"
    [ -n "${dotrain}" ] && echo "    retrain fail: ${total_spam_train_fail}"
    echo
fi

normal "Done"

cleanup

# done
