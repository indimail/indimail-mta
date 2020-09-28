#!PREFIX/bin/perl -w
#
# mailroute by Chris Garrigues <cwg@DeepEddy.Com>
#
# Reads an email message on stdin and pretty prints the contents of the
# recieved headers.
#
# When given an email message as it's argument will parse out the received
# headers and display the route it took to get to you and the times it
# arrived at each of the locations in your local timezone.
#
# It also tries to be clever in how it displays things:  (1) It only shows
# what changed in the date/time rather than the entire date/time each time. 
# (2) It breaks the line before the keywords "from", "by", and "with"
# unless they appear in comments.

use Time::Local;

# Global variable for date parsing
%mon = ('jan' => 0,
	'feb' => 1,
	'mar' => 2,
	'apr' => 3,
	'may' => 4,
	'jun' => 5,
	'jul' => 6,
	'aug' => 7,
	'sep' => 8,
	'oct' => 9,
	'nov' => 10,
	'dec' => 11);

# Initialize some variables to keep -w quiet
($owd, $om, $od, $ot, $oy) = ("", "", "", "", "");

# Read the headers into $_
($_ = "\n" . join("", <>)) =~ s/\n\n.*$//sg;

# Parse the contents of the received headers into an array
@rec = ();
while (/\nreceived:(.*?)(\n\S)/gis) {
    unshift(@rec, $1);
    $_ = "$2$'";
}

for (@rec) {
    s/\s+/ /gs;
    # Format is "information; date"
    ($line, $date) = /^\s*(.*?)\s*;\s*(.*?)$/;
    $date =~ s/\(.*\)//g;
    $date =~ s/\s+/ /gs;
    # Parse the sucker
    if ($date =~ /(\d+) (jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec) (\d\d\d\d) (\d+):(\d\d):(\d\d) ([+-]?\d+)/i) {
	# Coerce the date into something we can give to timegm
	$d = $1;
	($m = $2) =~ tr/A-Z/a-z/;
	$m = $mon{$m};
	$y = $3 - 1900;
	$h = $4;
	$mi = $5;
	$s = $6;
	($zh, $zm) = $7 =~ /^([-+]\d\d)(\d\d)$/;
	# hmm, a timezone like -0830 will break this...damn.
	($wd, $m, $d, $t, $y) =
	    split(' ',
		  localtime(timegm($s, $mi, $h, $d, $m, $y) -
			    ($zh*60*60 + $zm*60)));
	$d = " $d" if ($d < 10);
	# Insert line breaks
	$line =~ s/(by|with|from)/\n                         $1/g;
	# But take them back out if they're in a comment
	while ($line =~ s/\(([^()]*?)\s\s+?(.*?)\)/\($1 $2\)/gs) {};
	$line =~ s/\( /\(/g;
	$line =~ s/^\s*//s;
	# Figure out what parts of the date we want to display
	($pwd, $pm, $pd, $pt, $py) = ($wd, $m, $d, $t, $y);
	$pwd = "   " if ($wd eq $owd);
	$pm = "   " if ($m eq $om);
	$pd = "  " if ($d eq $od);
	$pt = "        " if ($t eq $ot);
	$py = "    " if ($y eq $oy);
	print "$pwd $pm $pd $py $pt $line\n";
	($owd, $om, $od, $ot, $oy) = ($wd, $m, $d, $t, $y);

    } else {
	# bail...
	print "$date $line\n";
    }
}
