use strict;
use warnings;
use blib;

use Test::More tests => 2;

BEGIN { use_ok('Mail::SRS_XS'); }

my $srs = new Mail::SRS_XS({
				Secret	=> "foo",
					});

my $new = $srs->forward('usera@hosta.com', 'userb@hostb.com');
like($new, qr/^SRS/, 'Forwarding works');
