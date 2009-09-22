#!/usr/bin/perl
#
# This is a filter which attempts to clean up MIME messages with HTML
# and binary attachments.  It strips the HTML and binary sections and
# flattens all multipart sections into one text/plain section.  We
# keep the MIME encoding type for the plain section.
#
# Note:  This script probably won't work properly for email with a lot
# of international characters.  It has only been tested on mailing
# lists with english content.
#
# More information and the latest version can be found at
# http://www.phred.org/~alex/stripmime.html
#
# Change History:  
# 1.0 - first public version 
# 1.1 - HTML conversion - Mar 11 2001
#
# This code is Copyright 2000-2001 Alex Wetmore. 
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met: 
#
# 1.  Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer. 
#
# 2.  Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following
# disclaimer in the documentation and/or other materials provided with
# the distribution. 
#
# 3.  All advertising materials mentioning features or use of this
# software must display the following acknowledgement:  This product
# includes software developed by Alex Wetmore. 
#
# 4.  The name of Alex Wetmore may not be used to endorse or promote
# products derived from this software without specific prior written
# permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

my $szLine = "";
my $fInHeaders = 1;
my $fMime = 0;

my %rgJunkHeaders = (
	"content-type"				=> 1,
	"content-transfer-encoding"	=> 1,
);

my $szHeaders = "";
my $szRemoved = "";
my $fForward = 0;

# This is not at all complete.  I just did the easy ones.
my %hHTMLCharConversion = (
	"nbsp"		=> " ",
	"gt"		=> ">",
	"lt"		=> "<",
	"amp"		=> "&",
	"and"		=> "&&",
	"asymp"		=> "=~",
	"brvbar"	=> "|",
	"bull"		=> "*",
	"cong"		=> "=~",
	"copy"		=> "(c)",
	"crarr"		=> "<cr>",
	"equiv"		=> "==",
	"ge"		=> "<=",
	"lang"		=> "<",
	"lsquo"		=> "\'",
	"mdash"		=> "--",
	"minus"		=> "-",
	"reg"		=> "(tm)",
	"sim"		=> "~",
	"thinsp"	=> " ",
	"shy"		=> "-",
	"trade"		=> "(tm)",
	"times"		=> "*",
);

# here I am mostly picking ones which have a large effect on layout
my %hHTMLTagConversion = (
	"/div"		=> "\n",
	"/p"		=> "\n",
	"hr"		=> "\n------------------------------------------\n",
	"br"		=> "\n",
	"li"		=> " * ",
);

#
# all of the work is done in this function
#
# arguments:
#   boundary for this level.  if "" then we are at the top level
# 
sub StripMime {
	my $szBoundary = shift;
	my $szContentType = shift;
	my $szRemoveDepth = shift;
	my $fTopLevel = ($szBoundary eq "" ? 1 : 0);
	my $szNewBoundary = "";
	my $szContentTransferEncodingHeader = "";
	my $szContentTypeHeader = "";
	my $fQuotedPrintable = 0;
	my $cConsecQuotedLines = 0;
	my $fConvertHTML = 0;
	my $fInTag = 0;
	
	while (<>) {
		chomp;
		my $szThisLine = $_;

		if (!$fTopLevel && $szThisLine =~ /^--\Q$szBoundary\E($|--$)/) {
			#
			# we hit a boundary in a multipart section
			#
			if ($1 eq "--") {
				#
				# this is the end of the multipart section, return
				# to our caller
				#
				return;
			}
			# default to plaintext in all sections unless told otherwise
			$szContentType = "text/plain";
			# we are in headers again
			$fInHeaders = 1;
		} elsif ($fInHeaders && $szThisLine eq "") { 
			#
			# we hit the end of the headers.  
			#
			$fInHeaders = 0;

			# if there was a multipart section with a new boundary marker
			# then we need to recurse into it and clean it up
			if ($szNewBoundary ne "") {
				$szRemoved .= "$szRemoveDepth$szContentType\n";
				if ($fTopLevel) {
					$szHeaders = $szHeaders . "X-StripMime: Non-text section removed by stripmime\n";
				}
				StripMime($szNewBoundary, $szContentType, $szRemoveDepth . "  ");
				$szNewBoundary = "";
			} else {
				if (($szContentType eq "text/plain" || $fConvertHTML) && $szHeaders ne "") {
					print $szHeaders;
					print $szContentTransferEncodingHeader;
					print $szContentTypeHeader;
					print "\n";
					$szHeaders = "";
				}
				if ($szContentType ne "text/plain" || $szRemoved ne "") {
					if ($szContentType eq "text/plain") {
						$szRemoved .= "$szRemoveDepth$szContentType (text body -- kept)\n";
					} elsif ($szContentType eq "text/html" && $fConvertHTML) {
						$szRemoved .= "$szRemoveDepth$szContentType (html body -- converted)\n";
					} else {
						$szRemoved .= "$szRemoveDepth$szContentType\n";
					}
				}
			}
		} elsif ($fInHeaders) {
			#
			# we are processing headers
			#

			my $szHeaderName = "";

			# check for a header continuation
			if ($szThisLine =~ /^\s(.*)$/) {
				$szLine .= $1;
			} else {
				$szLine = $szThisLine;
			}

			# get the name of this header
			if ($szLine =~ /^([\w-]+):/) { 
				$szHeaderName = lc($1); 
			}

			# get the content type
			if ($szLine =~ /^Content-type:\s*([^;]*)(;|$)/i) { 
				$szContentType = lc($1);
				$szContentTypeHeader = $szLine . "\n";

				if ($szContentType eq "text/html" && $fTopLevel) {
					$fConvertHTML = 1;
					$szContentTypeHeader = "Content-Type: text/plain\n";
				}
			}

			# see if this message is forwarded
			if ($szThisLine =~ /^subject:(\s+fw:|.*\(fwd\)\s*$)/i) {
				$fForward = 1;
			}

			# if the content type is multipart then get the boundary code
			if ($szLine =~ /^Content-type:\s*multipart\/.*boundary=(\"([^\"]+)\"|([\w+\'\(\)\+,\-.\/:=?]+))/i) {
				$szNewBoundary = ($2 eq "" ? $3 : $2);
			}

			# get the content transfer encoding.  if it is quoted-printable
			# then we will clean it up a bit when working on the body.
			if ($szLine =~ /^Content-transfer-encoding:\s+(.*)$/i) {
				$szContentTransferEncodingHeader = $szLine . "\n";
				if ($1 =~ /quoted-printable/i) {
					$fQuotedPrintable = 0;
				}
			}

			# print this header if it is at the top-level and not one
			# of our junk headers
			if ($fTopLevel && !(exists $rgJunkHeaders{$szHeaderName})) {
				$szHeaders = $szHeaders . "$szThisLine\n";
			}
		} elsif ($szContentType eq "text/plain" | $fConvertHTML) {
			# go through and work on the normal body
			# if the line ends with = then strip it
			if ($fQuotedPrintable) {
				# remove trailing =
				$szThisLine =~ s/=$//;

				# if a normal character is quoted then unquote it
				# we need to process left to right (instead of
				# reprocessing what we've processed) so that we
				# don't convert something "=3D20" to " " (it
				# should be "=20"
				my $szRemainder = $szThisLine;
				$szThisLine = "";

				while ($szRemainder =~ /=([0-9A-F]{2})/) {
					my $i = hex $1;
					my $ch = "=$1";
					if ($i >= 32 && $i <= 127) {
						$ch = chr($i);
					} 
					$szThisLine .= "$`$ch";
					$szRemainder = $';
				}
				$szThisLine .= $szRemainder;
			}

			# we have a very simple parser for HTML.  we go through
			# each line, removing tags until none are left.  when
			# we find tags we look them up in %hHTMLTagConversion
			# and convert them into ascii if they are in that
			# table.  The goal here is to get nice looking ASCII
			# text with a minimum of real work
			if ($fConvertHTML) {
				my $szBeforeTag;
				my $szInTag;
				my $iStartTag;
				my $iEndTag;
				my $szAfterTag;
				my $szTagConversion = "";
				my $szTag;
				if ($fInTag) {
					if (($iEndTag = index($szThisLine, ">")) != -1) {
						$szThisLine = substr $szThisLine, $iEndTag+1;
					} else {
						$szThisLine = "";
					}
				}
				while (($iStartTag = index($szThisLine, "<")) != -1) {
					$szBeforeTag = substr $szThisLine, 0, $iStartTag;
					$szInTag = substr $szThisLine, $iStartTag+1;
					if (($iEndTag = index($szInTag, ">")) != -1) {
						# we have the end of the tag on this line
						$szTag = substr $szInTag, 0, $iEndTag;
						$szAfterTag = substr $szInTag, $iEndTag+1;
					} else {
						# the tag extends to another line
						$szTag = $szInTag;
						$fInTag = 1;
						$szAfterTag = "";
					}
					$szTag = lc $szTag;	
					if (exists $hHTMLTagConversion{$szTag}) {
						$szTagConversion = $hHTMLTagConversion{$szTag};
					}
					$szThisLine = "$szBeforeTag$szTagConversion$szAfterTag";
				}

				# now that we have removed all tags we go through and
				# convert special characters back to their ascii
				# equivelents
				while ($szThisLine =~ /&(\w*);/) {
					my $szBefore = $`;
					my $szAscii;
					my $szEntity = $1;
					my $szAfter = $';
					if ($szEntity =~ /^#(\n\n\n)/) {
						if ($1 < 255) {
							# convert a numeric code
							$szAscii = chr $1;
						} else {
							$szAscii = "{$szEntity}";
						}
					} elsif (exists $hHTMLCharConversion{$szEntity}) {
						$szAscii = $hHTMLCharConversion{$szEntity};
					} else {
						# don't know how to convert
						$szAscii = "{$szEntity}";
					}
					$szThisLine = "$szBefore$szAscii$szAfter";
				}
			}

#			if (length($szThisLine) < 77) {
				print "$szThisLine\n";
#			} else {
#				# wrap long lines using this format
#format =
#^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#$szThisLine
#.
#				# figure out the quote characters at the front of
#				# the line.  if they exist then we will put them in
#				# front of wrapped lines to make the quoting look
#				# right
#				$szThisLine =~ /([>\s]+)/;
#				do {
#					write;
#					$szThisLine = $1 . $szThisLine;
#				} while ($szThisLine ne $1);
#			}
		}
	}

	if ($fTopLevel) {
		if ($szHeaders ne "" && $szContentType ne "text/plain") {
			# if there was no text/plain part then we can end up here.
			# not much that we can do, so we add a special header saying
			# that the content was HTML only and then put a helpful blurb
			# in the body
			print $szHeaders;
			print "X-StripMime-Failure: no text/plain\n";
			print "\n";
			print "--- StripMime Report -- processed MIME parts ---\n";
			print "$szRemoved";
			print "--- StripMime Errors ---\n";
			print "A message with no text/plain section was received.\n";
			print "The entire body of the message was removed.  Please\n";
			print "resend the email using plaintext formatting\n";
			print "---\n";
		} elsif ($szRemoved ne "") {
			# if we removed some stuff then let the world know
			print "\n";
			print "--- StripMime Report -- processed MIME parts ---\n";
			print "$szRemoved";
			print "---\n";
		}
	}
}

# start things going at the top level
StripMime("", "text/plain", "");
