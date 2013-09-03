package Mail::SRS_XS;

use strict;
eval { require warnings; };
use vars qw($VERSION @ISA @EXPORT_OK %EXPORT_TAGS);
use Exporter;

require DynaLoader;

$VERSION = "0.01";
@ISA = qw(DynaLoader Exporter);
@EXPORT_OK = ();
%EXPORT_TAGS = (
	all	=> \@EXPORT_OK,
		);

bootstrap Mail::SRS_XS;

=head1 NAME

Mail::SRS_XS - An XS implementation of Mail::SRS

=head1 DESCRIPTION

This is an interface to the C libsrs for the purpose of checking
compatability with the existing Mail::SRS implementation. It is not
primarily used for standalone use as an SRS implementation - you
should be using the original Mail::SRS for that; it should not be
noticeably slower.

This package also may not implement all the features of the full
Mail::SRS, although the intent is to perform regression testing.

=head1 SUPPORT

Mail the author at <cpan@anarres.org>

=head1 AUTHOR

	Shevek
	CPAN ID: SHEVEK
	cpan@anarres.org
	http://www.anarres.org/projects/

=head1 COPYRIGHT

Copyright (c) 2002 Shevek. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=head1 SEE ALSO

Mail::SRS, perl(1).

=cut

1;
