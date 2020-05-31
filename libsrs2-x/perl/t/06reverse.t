use strict;
use warnings;
use blib;

use Test::More tests => 6;

BEGIN { use_ok('Mail::SRS_XS'); }

my $srs = new Mail::SRS_XS({
				Secret	=> "foo",
					});
ok(defined $srs);
ok(UNIVERSAL::isa($srs, 'Mail::SRS_XS'));

my $addr = 'usera@hosta.com';

my $fwd1 = $srs->forward($addr, 'userb@hostb.com');
print STDERR "fwd1 = $fwd1\n";
my $rev1 = $srs->reverse($fwd1);
print STDERR "rev1 = $rev1\n";
is($rev1, $addr, 'Reversal works at a first level');

my $fwd2 = $srs->forward($fwd1, 'userc@hostc.com');
print STDERR "fwd2 = $fwd2\n";
my $rev2 = $srs->reverse($fwd2);
print STDERR "rev2 = $rev2\n";
is($rev2, $fwd1, 'Reversal works at a second level');

my $fwd3 = $srs->forward($fwd2, 'userd@hostd.com');
print STDERR "fwd3 = $fwd3\n";
my $rev3 = $srs->reverse($fwd3);
print STDERR "rev3 = $rev3\n";
is($rev3, $fwd1, 'Reversal works at a third level');
