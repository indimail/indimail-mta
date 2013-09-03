use strict;
use warnings;
use blib;

use Test::More tests => 4;

BEGIN { use_ok('Mail::SRS_XS', ':all'); }

ok(defined SRS_ERRTYPE_CONFIG, 'SRS_ERRTYPE_CONFIG defined');
ok(defined SRS_SUCCESS, 'SRS_SUCCESS defined');
ok(SRS_EHASHINVALID, 'SRS_EHASHINVALID defined and nonzero');
