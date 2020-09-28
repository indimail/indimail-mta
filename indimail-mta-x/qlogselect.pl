#!PREFIX/bin/perl
use Time::Local;

$start_time = 0;
$end_time = 0;
$from_pattern = undef;
$add_status = 0;
$strip_status = 0;

sub conv_date {
    local($_) = shift;
    my $time;
    if(/^(\d+)-(\d+)-(\d+)$/) {
	$time = timelocal(0, 0, 0, $3, $2-1, $1);
    } elsif(/^\d+$/) {
	$time = $_;
    } else {
	print STDERR "Invalid time: '$_'\n";
	exit 1;
    }
    $time;
}

while($arg = shift @ARGV) {
    if($arg eq 'start') {
	$start_time = &conv_date(shift @ARGV);
    } elsif($arg eq 'end') {
	$end_time = &conv_date(shift @ARGV);
    } elsif($arg eq 'from') {
	$from_pattern = shift @ARGV;
    } elsif($arg eq 'stripstatus') {
	$strip_status = 1;
    } elsif($arg eq 'addstatus') {
	$add_status = 1;
    } else {
	print STDERR "Invalid argument '$arg'\n";
	exit 1;
    }
}

if($start_time) {
    while(<>) {
	next unless /^(\d+\.\d+) /o;
	last if $1 >= $start_time;
    }
}

$_ = <> unless $_;

while(1) {
    $line = $_;
    chomp;
    unless(s/^(\d+\.\d+) +//o) {
	print STDERR "Bad line: '$_'\n";
	next;
    }
    my $time = $1;
    last if $end_time && $time > $end_time;
    my $msg;
    my $delivery;
    if(/^status: /o) {
	print $line unless $strip_status;
    } elsif(/^starting delivery (\d+): msg (\d+) to (local|remote)/o) {
	$delivery = $1;
	$msg = $2;
	$lr = $3;
	if($use_msg{$msg}) {
	    $use_delivery{$delivery} = $lr;
	    print $line;
	    $concurrency{$lr}++;
	    print "$time status: status: local $concurrency{local}/? remote $concurrency{remote}/?\n" if $add_status;
	}
    } elsif(/^delivery (\d+): /o) {
	$delivery = $1;
	$lr = $use_delivery{$delivery};
	if($lr) {
	    print $line;
	    $concurrency{$lr}--;
	    print "$time status: status: local $concurrency{local}/? remote $concurrency{remote}/?\n" if $add_status;
	}
	delete $use_delivery{$delivery};
    } elsif(/^new msg (\d+)$/o) {
	$msg = $1;
	$new_msg{$msg} = $line;
    } elsif(/^info msg (\d+): bytes \d+ from <(.*)> qp \d+ uid \d+$/o){
	$msg = $1;
	my $from = $2;
	if(!$from_pattern || $from =~ /^$from_pattern$/oi) {
	    $use_msg{$msg} = 1;
	    print $new_msg{$msg}, $line;
	}
    } elsif(/^end msg (\d+)$/o) {
	$msg = $1;
	print $line if $use_msg{$msg};
	delete $use_msg{$msg};
    } elsif(/^bounce msg (\d+) qp (\d+)$/o) {
	$msg = $1;
	print $line if $use_msg{$msg};
    }
} continue {
    $_ = <>;
    last unless $_;
}
