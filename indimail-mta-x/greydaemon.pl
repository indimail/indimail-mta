#!/usr/bin/perl
# $Header: /home/johnl/hack/RCS/greydaemon,v 1.8 2004/05/28 19:31:36 johnl Exp $

# greydaemon [ flags ] ipaddr savefile

# -u username run as username
# -w whitelist of IP ranges
# -s context file save interval time
# -t timeout for known IPs in days
# -g grey resend window, in hours
# -m min resend accept time, in minutes

# Copyright (c) 2009, John R. Levine, Taughannock Networks
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
#  * Neither the name of Taughannock Networks nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.

require 5.002;
use strict;
use IO::Socket;
use Getopt::Std;

sub newranges();
sub addrange( $ );
sub endranges();
sub checkrange( $ );

use vars qw($opt_d $opt_s $opt_g $opt_m $opt_t $opt_u $opt_w %ips %grey);
getopts("ds:g:m:t:u:w:");

$| = 1;

my ($ipaddr, $port, $savefile, $server, $rmt, $msg, $omsg, $now);

$ipaddr = shift or die "Need IP address to serve on";

$savefile = shift or die "Need name of context save file";
my $savetime = ($opt_s || 10)*60; # how long between saves
my $lastsave;

my $timeout = ($opt_t || 7)*3600*24;
my $maxgrey = ($opt_g || 12)*3600;
my $mingrey = ($opt_m || 5)*60;


if($ipaddr =~ /(.*):(.*)/) {
    $ipaddr = $1;
    $port = $2;
} else {
    $port = 1999;
}

$server = IO::Socket::INET->new(LocalAddr => $ipaddr, LocalPort => $port, Proto => "udp")
    or die "socket failed $ipaddr:$port $!";

if(defined($opt_u)) {
    my ($name, $pwd, $uid, $gid) = getpwnam $opt_u or die "can't find $opt_u";
    $( = $) = $gid;
    $< = $> = $uid;
    print "Now running as user $opt_u ($uid, $gid)\n";
}

if($opt_w) {
    open(WHITE, "<$opt_w") or die "? cannot open $opt_w";
    newranges();
    while(<WHITE>) {
		s/#.*//;
		next if /^\s*$/;
		chomp;
		addrange($_);
    }
    close WHITE;
    endranges();
}

$lastsave = $now = time;

if(open(SAVE, "<$savefile")) {
    my $tl = $now - $timeout;
    my $gtl = $now - $maxgrey;
    
    print "load $savefile\n" if $opt_d;
    while(<SAVE>) {
		if(/^I (\d+) ([0-9.]+)/) {
	    	print "  ip $1 = $2\n" if $opt_d and $1 > $tl;
	    	$ips{$2} = $1 if $1 > $tl;
		} elsif(/^G (\d+) ([0-9.]+) (\S+) (\S+)/) {
	    	$grey{"$2 $3 $4"} = $1 if $1 > $gtl;
	    	print "  grey $1 = $2 $3 $4\n" if $opt_d and $1 > $gtl;
		} else {
	    	print "? Strange save entry $_";
		}
    }
    close SAVE;
}

print "started greydaemon on $ipaddr:$port\n";

mainloop: while($rmt = $server->recv($msg, 2048)) {       # 2048 = MAXGREYPKTSIZE in grey.h
    my ($rport, $raddr) = sockaddr_in($server->peername);
    my ($addr, $resp, $ip, $from, $to, @args);

    $addr = inet_ntoa($raddr);
    $now = time;
    my $dmsg = $msg;
    $dmsg =~ s/\0/ /g;
    @args = split /\0/,$msg;
    if((shift @args) =~ m{I(.*)}) {
		$ip = $1;
    } else {
		print "$addr:$rport bad req no I $dmsg\n";
		next;
    }
   
	# whitelisted ip
    if(checkrange($ip)) {
		print "$addr:$rport white $dmsg\n";
		$resp = "\1\1";
    } elsif(defined $ips{$ip} && $ips{$ip} > ($now - $timeout)) { # not older than timeout days
		$ips{$ip} = $now;
		print "$addr:$rport ok ip $dmsg\n";
		$resp = "\1\2";
    } else {
		if((shift @args) =~ /F(.*)/) {
	    	$from = $1;
	    	$from =~ s/ //g;	# no spaces allowed
	    	$from = "." if $from eq "";
		} else {
	    	print "$addr:$rport bad req no F $dmsg\n";
	    	next mainloop;
		}
		$resp = "\1\3";
		for $to (@args) {
	    	unless($to =~ s/^T//) {
				print "$addr:$rport bad req no T $dmsg\n";
				next mainloop;
	    	}
	    	$to =~ s/ //g;	# no spaces allowed
	    	if($from eq "." and $to =~ /-.*=/) {
				$resp = "\1\4";	# bounce
				print "$addr:$rport bounce $dmsg\n";
				last;
	    	}
	    	my $gt = $grey{lc "$ip $from $to"};
	    	if($gt) {
				if(($now - $gt) < $mingrey) {
		    		$resp = "\0\2"; # too new
		    		print "$addr:$rport retry too soon $dmsg\n";
				} elsif(($now - $gt) < $maxgrey) {
		    		$ips{$ip} = $now;
		    		print "$addr:$rport retry ok $dmsg\n";
				} else {
		    		$grey{lc "$ip $from $to"} = $now;
		    		$resp = "\0\3";
		    		print "$addr:$rport stale grey $dmsg\n";
				}
	    	} else {
				$grey{lc "$ip $from $to"} = $now;
				$resp = "\0\1";
				print "$addr:$rport new grey $dmsg\n";
	    	}
		}
    }
    $server->send($resp);
    if($opt_d or ($now - $lastsave) > $savetime) {
		my $tl = $now - $timeout;
		my $gtl = $now - $maxgrey;
		my ($t, $k);
		print "save status to $savefile\n";
		open(SAVE, ">$savefile.new") or die "create $savefile.new";
		while(($ip, $t) = each %ips) {
	    	print SAVE "I $t $ip\n" if $t > $tl;
		}
		while(($k, $t) = each %grey) {
	    	print SAVE "G $t $k\n" if $t > $gtl;
		}
		close SAVE;
		unlink $savefile;
		rename "$savefile.new",$savefile;
		$lastsave = $now;
    }
	# main loop
}

################################################################
# a testable set of IP ranges
my (%ranges, @masks, %masks);

sub newranges() {
    %ranges = ();
    %masks = @masks = ();
}

sub addrange($) {
    my ($range) = @_;
    my ($addr, $mask);

    print "addrange $range " if $opt_d;

    if($range =~ m{(\d[0-9.]+)/(\d+)}) {
		$addr = $1; $mask = $2;
    } elsif($range =~ m{(\d+\.\d+\.\d+\.\d+)}) {
		$addr = $1; $mask = 32;
    } elsif($range =~ m{(\d+\.\d+\.\d+)}) {
		$addr = $1; $mask = 24;
    } elsif($range =~ m{(\d+\.\d+)}) {
		$addr = $1; $mask = 16;
    } elsif($range =~ m{(\d+)}) {
		$addr = $1; $mask = 8;
    } else { 
		die "? bad range $range";
    }
    my $naddr = unpack "N", pack "CCCC", split /\./, $addr;
    printf " = %08x / %d\n", $naddr, $mask if $opt_d;
    $ranges{$naddr} = $mask;
    $masks{$mask} = 1;
}

sub endranges() {
    @masks = sort { $b <=> $a } keys %masks; # largest to smallest mask
    print "masks " . join(" ", @masks) . "\n" if $opt_d;
}

sub checkrange($) {
    my ($range) = @_;
    my ($i);
    my $nrange = unpack "N", pack "CCCC", split /\./, $range;

    foreach $i (@masks) {
		my $m = (2**32) - 2**(32-$i);
		my $mv = $nrange & $m;
		return 1 if defined($ranges{$mv}) and $ranges{$mv} <= $i;
    }
    0;
}
