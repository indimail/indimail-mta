#! /usr/bin/env perl

# unsort.pl - print words of a short file in random order
# processes from stdin to stdout

# Copyright (C) 2004 by Matthias Andree
# This file is under the GNU General Public License, v2

my @a = <>;
chomp @a;
@a = split ' ', join(' ', @a);

srand(time ^ ($$ + ($$ << 15)));
while(scalar @a) {
    my $pos = int rand scalar @a;
    print $a[$pos], "\n";
    @a = (@a[0..($pos-1)],@a[($pos+1)..$#a]);
}
