#! /usr/bin/perl
#
# Compile LineBreak.txt into C array declarations.
#
# The array's structure is [firstchar, lastchar, class], giving the
# linebreaking "class" for unicode character range firstchar-lastchar.
#
# The ranges are sorted in numerical order.
#
# An array gets generated for each block of 4096 unicode characters.
#
# Finally, two arrays get declared: a pointer to an array for each 4096
# unicode character block, and the number of elements in the array.
#
# The pointer is NULL for each block of 4096 unicode characters that is not
# defined in LineBreak.txt
#
# By definition, a unicode character that is not listed in the array is
# class XX.

use strict;
use warnings;
use mkcommon;

my %general_category;

open(UC, "<UnicodeData.txt") || die;

while (defined($_=<UC>))
{
    chomp;

    my @f=split(/;/);

    my $cp;

    eval "\$cp=0x$f[0]";

    $general_category{$cp}=$f[2];
}

my $obj=mkcommon->new;

open(F, "<LineBreak.txt") || die;

while (defined($_=<F>))
{
    chomp;

    next unless /^([0-9A-F]+)(\.\.([0-9A-F]+))?\;([^\s][^\s])\s*/;

    my $f=$1;
    my $l=$3;
    my $t=$4;

    $l=$f unless $l;

    eval "\$f=0x$f";
    eval "\$l=0x$l";

    next if $t eq "XX";

    $t="NS" if $t eq "CJ";

    if ($t eq "SA")
    {
	while ($f <= $l)
	{
	    die "Cannot find general_category for $f\n"
		unless exists $general_category{$f};

	    $obj->range($f, $f,
			$general_category{$f} eq "Mn" ||
			$general_category{$f} eq "Mc" ?
			"UNICODE_LB_CM":"UNICODE_LB_AL");
	    # LB1 rule
	    ++$f;
	}
    }
    else
    {
	$t="AL" if $t eq "AI" || $t eq "SG"; # LB1 rule

	$obj->range($f, $l, "UNICODE_LB_$t");
    }
}

$obj->output;
