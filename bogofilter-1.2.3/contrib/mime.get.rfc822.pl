#!/usr/bin/perl -w

# $Id: mime.get.rfc822.pl 6288 2005-10-26 08:56:08Z m-a $ #

=head1 NAME

mime.get.rfc822 - splits out message/rfc822 parts from a MIME message

=head1 SYNOPSIS

Usage:
    mime.get.rfc822 <message

=head1 DESCRIPTION

Trivial script to print out all message/rfc822 parts of a MIME message.

Originally from an idea on the bogofilter mailing list as one way to allow
people to easily submit things to the spamlist without having their own
addresses added when forwarding spam to an account.  In such a case messages
should be piped to this before being piped to bogofilter -s

The output is forced into an mbox format for easy parsing by bogofilter.

=head1 AUTHOR

Simon Huggins <huggie@earth.li>

=cut


# Copyright(C) Simon Huggins 2003 <huggie@earth.li>
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc., 59
# Temple Place, Suite 330, Boston, MA 02111-1307  USA


# Yes, it is silly having the license boilerplate take up more space than
# the code but it does remove all doubt.

use strict;
use MIME::Parser;

my $parser = new MIME::Parser;
$parser->extract_nested_messages(0);
$parser->output_to_core(1);		 # No temporary files
my $entity = $parser->parse(\*STDIN);

my $found=0;
# Loop recursing deeper until we find a message/rfc822 part.
my @check_parts = $entity->parts;

while (!$found) {
	foreach my $subent (@check_parts) {
		if ($subent->effective_type eq "message/rfc822") {
			print "From invalid\@example.com Mon May 19 18:00:00 2003\n";
			my $body = $subent->stringify_body;
			$body =~ s/^From />From /mg;
			$body =~ s/\n*$/\n\n/;
			print $body;
			$found++;
		}
	}
	if (!$found) {
		@check_parts = map { $_->parts; } @check_parts;
		last if ! @check_parts;
	}
}

