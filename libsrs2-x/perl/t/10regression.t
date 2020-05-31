use strict;
use warnings;
use blib;

use Test::More tests => 7;
use Mail::SRS 0.30;

BEGIN { use_ok('Mail::SRS_XS'); }

my $srsx = new Mail::SRS_XS({
				Secret	=> "secret",
					});
ok(defined $srsx);
ok(UNIVERSAL::isa($srsx, 'Mail::SRS_XS'));

my $srsp = new Mail::SRS(
				Secret	=> "secret",
					);

my $s0x = $srsx->forward('usera@hosta.com', 'userb@hostb.com');
my $s0p = $srsp->forward('usera@hosta.com', 'userb@hostb.com');
is($s0x, $s0p, 'SRS0 forwarding matches Perl implementation');

print "$s0x\n";

my $s1x = $srsx->forward($s0x, 'userc@hostc.com');
my $s1p = $srsp->forward($s0p, 'userc@hostc.com');
is($s1x, $s1p, 'SRS1 forwarding matches Perl implementation');

my $s1rx = $srsx->reverse($s1x);
my $s1rp = $srsp->reverse($s1p);
is($s1rx, $s1rp, 'SRS1 reversal matches Perl implementation');

my $s0rx = $srsx->reverse($s1rx);
my $s0rp = $srsp->reverse($s1rp);
is($s0rx, $s0rp, 'SRS0 reversal matches Perl implementation');
