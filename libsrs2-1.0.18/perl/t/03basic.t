use strict;
use warnings;
use blib;

use Test::More tests => 9;

BEGIN { use_ok('Mail::SRS_XS'); }

my $srs = new Mail::SRS_XS({
				Secret	=> "foo",
					});

my $secret = $srs->get_secret(0);
is($secret, 'foo', 'Secret was stored OK');

my $t = $srs->timestamp_create(time());
ok(defined $t, 'Created a timestamp');
ok(length $t == 2, 'Timestamp is 2 characters');
ok($srs->timestamp_check($t), 'Timestamp verifies');
my $notlong = 60 * 60 * 24 * 3;
my $ages = 60 * 60 * 24 * 50;
ok($srs->timestamp_check($srs->timestamp_create(time() - $notlong)),
				'Past timestamp is OK');
ok(! $srs->timestamp_check($srs->timestamp_create(time() - $ages)),
				'Antique timestamp fails');
ok(! $srs->timestamp_check($srs->timestamp_create(time() + $notlong)),
				'Future timestamp fails');
ok(! $srs->timestamp_check($srs->timestamp_create(time() + $ages)),
				'Future timestamp fails');
