#! /usr/bin/perl -w

# printmaildir.pl -- (C) 2003 by Matthias Andree

# This program reads a Maildir and prints in in UNIX mbox format on
# stdout. It is redistributable in accordance to the terms of the
# GNU GENERAL PUBLIC LICENSE V2

use strict;
use POSIX;

sub read_dir($ ) {
    my $dir = shift;
    my @list = ();
    my $d;
    foreach $d (qw/cur new/) {
	opendir(DIR, $dir . "/$d") || die "can't opendir $dir/$d: $!";
	push @list, grep { ! /^\./ && $_ =~ s|^|$dir/$d/|; } readdir(DIR);
	closedir DIR;
    }
    foreach (@list) {
	if (open F, $_) {
	    my $head = <F>;
	    my $last = $head;
	    if ($head !~ /^From /) {
		print "From unknown\@example.invalid  ", POSIX::ctime(time);
	    }
	    print $head;
	    while(<F>) {
		$last = $_;
		if (/^From /){
		    print ">$_";
		} else {
		    print $_;
		}
	    }
	    if ($last !~ /\n$/) {
		print "\n";
	    }
	   close F;
	} else { # open failed
	    warn "can't open $_: $!";
	}
    }
}

if (!@ARGV) { unshift @ARGV, "."; }

foreach(@ARGV) {
    read_dir $_;
}
