#! /usr/bin/perl
#
# Compile GraphemeBreakProperty.txt into C array declarations.
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

open(F, "<GraphemeBreakProperty.txt") || die;

# Read LineBreak.txt
#
# Invoke doclass() passing the first unicode char, the last unicode char,
# and the linebreaking class for the given unicode char range.

my @breaklist;

while (defined($_=<F>))
{
    chomp;

    next unless /^([0-9A-F]+)(\.\.([0-9A-F]+))?\s*\;\s*([^\s]+)\s*/;

    my $f=$1;
    my $l=$3;
    my $t=$4;

    $l=$f unless $l;

    eval "\$f=0x$f";
    eval "\$l=0x$l";

    push @breaklist, [$f, $l, $t];
}


my $obj=mkcommon->new;

foreach (sort { $$a[0] <=> $$b[0] } @breaklist)
{
    $obj->range($$_[0], $$_[1], "UNICODE_GRAPHEMEBREAK_$$_[2]");
}

$obj->output;
