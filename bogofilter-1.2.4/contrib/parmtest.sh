#!/bin/sh

#	parmtest.sh - a parameter testing script.
#
#	This script can be used to test a bogofilter option
#	to see whether the option makes bogofilter is more
#	effective or less effective.
#
#	It is presently set to test 'tag_header_lines', but
#	by creating different config files can be used to test
#	other config file options.
#
#	For each test named in variable TESTS, a corresponding 
#	config file, e.g. $test.cf, is needed.  Bogofilter is
#	trained using $test.cf to create wordlists for the test.
#	The newly created wordlists are saved in directory $test.d.
#	This allows a test to be run using the same parameters
#	used to build the wordlists.
#
#	For each mbox file used in testing, an output file is
#	created that gives the result of testing each message
#	in the mbox.  The result lines include:
#
#		S/H/U - the spam/ham/unsure classification.
#		spamicity - 0.0 to 1.0
#		message number - 0000, 0001, ...
#		subject line - "Subject: This is a test"
#
#	A summary line is generated for each file giving
#	test name, filename, S count, H count, and U count, 
#	for example:
#
#		default.spam.mbx.out 1603    5  136
#		default.good.mbx.out    3 4934  108

BIN=~/bin
BOGOUTIL="$BIN/bogoutil"
BOGOFILTER="$BIN/bogofilter"

#
TEST_DIR="./test.bogofilter"
BOGOFILTER_DIR="$TEST_DIR"
export BOGOFILTER_DIR

#specify files for creating the test wordlists
SPAM_TRAIN=$(ls spam_train*mbx)
GOOD_TRAIN=$(ls good_train*mbx)

#specify files for testing
SPAM_TEST=$(ls spam_test*mbx)
GOOD_TEST=$(ls good_test*mbx)

#specify names of tests to run
TESTS="default tag_header_lines"

if [ -z "$1" ] ; then

# create standard (base) config file

    cat <<EOF > base.cf
algorithm = fisher
ham_cutoff = 0.10
spam_cutoff = 0.95
terse_format = %1.1c %f
header_format = %1.1c
spamicity_tags = S, H, U
spamicity_formats = %0.6f %0.6f %0.6f
EOF

    # copy base config file for each test
    for test in $TESTS ; do
	cat base.cf > $test.cf
    done

    # customize config files
    cat <<EOF >> default.cf
tag_header_lines=no
EOF

    cat <<EOF >> tag_header_lines.cf
tag_header_lines=yes
EOF

    [ ! -d $TEST_DIR ] && mkdir $TEST_DIR

    date

    for test in $TESTS ; do

	echo $test

	# train (build wordlists for testing)
	if [ ! -d $test.d ] ; then
	    mkdir $test.d
	    rm -f $TEST_DIR/*.db
	    if [ ! -f spamlist.db ] ; then
		for m in $SPAM_TRAIN ; do
		    echo $m
		    $BOGOFILTER -v -c $test.cf -s < $m
		done
		cp -p $TEST_DIR/spamlist.db $test.d
	    fi
	    if [ ! -f goodlist.db ] ; then
		for m in $GOOD_TRAIN ; do
		    echo $m
		    $BOGOFILTER -v -c $test.cfd -n < $m
		done
		cp -p $TEST_DIR/goodlist.db $test.d
	    fi
	    $BOGOUTIL -w $test.d .MSG_COUNT
	fi

	# copy wordlists for use
	cp -pf $test.d/*.db $TEST_DIR

	# test (score accuracy)
	cfg="$test.cf"
	list="$SPAM_TEST $GOOD_TEST"

	for mbox in $list ; do
	    b=$(basename $mbox)
	    f="$test.$b.out"
	    cat /dev/null > $f
	    FILENO=0000 formail -s $0 $cfg $f < $mbox
	    s=$(grep "^S" $f | wc -l)
	    h=$(grep "^H" $f | wc -l)
	    u=$(grep "^U" $f | wc -l)
	    printf "%-20s %4d %4d %4d\n" $(basename $f) $s $h $u
	done

	echo ""
    done
else
    cfg="$1"
    out="$2"
    cat > $$.tmp
    result=$($BOGOFILTER -t -v -c $cfg < $$.tmp | head -1)
    echo $result  $FILENO  $(grep ^Subject: $$.tmp | head -1) >> $out
    rm -f $$.tmp
fi
