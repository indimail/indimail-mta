#!/usr/bin/perl

#       If you are using qmail as your MTA
#       stick this file in /usr/local/bin/do-spf.pl and chmod +x it.
#       and then for each .qmail file which needs SPF processing
#       just add  |/usr/local/bin/do-spl.pl 
#       to the top of the .qmail file -- normally before the Maildir
#       line.
#
#       By default, this rejects nothing, but adds the Received-SPF
#       or X-SPF-Guess header. Rejecting is pretty useless as you
#       know that the sender is forged!
#
#       Philip Gladstone
#

use strict;

use Mail::SPF::Query;

my $sender = $ENV{SENDER};
my $src;
my $helo;

my $hardfail = 0;       # Don't reject DENY messages

my @msg;

while (<>) {
    push @msg, $_;
    if (/^Received-SPF/i || /^X-SPF-Guess/i) {
        # I ought to check that it is my record 
        # but since it appears before the first received, it probably is mine
        # Signify that delivery is to continue
        exit 0;
    }
    if (!$sender && /^Return-path: <(.*)>/i) {
        $sender = $1;
    }
    if (!$src && /^Received:\s*from\s+(\S+)\s.*\(\[([.\d]+)\]\)/i) {
        my $from = $1;
        $src = $2;
        ($helo) = /\(HELO (\S+)\)/;
        $helo ||= $from;
        last;
    }
}

if (!$src) {
    # This is probably a local delivery
    exit 0;
}

my $query = Mail::SPF::Query->new(ip => $src, sender => $sender, helo => $helo);

my ($result, $comment)       = $query->result();
my $hdr = "Received-SPF";

if    ($result eq "pass")     { } # domain is not forged
elsif ($result eq "fail")     { 
    if ($hardfail) {
        print "SPF failure: $comment\nFor information on SPF, see http://spf.pobox.com/";
        # Fail the delivery
        exit 100; 
    }
} elsif ($result eq "softfail") {  } # domain may be forged
else {       # domain has not implemented SPF
    $hdr = "X-SPF-Guess";
    ($result, $comment)       = $query->best_guess();
}

$result ||= 'unknown';

($ENV{QMAILSUSER}, $ENV{QMAILSHOST}) = $sender =~ /(.*)@([^@]+)/;

#open (MAIL, "|qmail-inject $ENV{RECIPIENT}");
open (MAIL, "|-") || exec 'qmail-inject', $ENV{RECIPIENT};

print MAIL "$hdr: $result ($comment)\n";
print MAIL @msg;
while (<>) {
    print MAIL;
}
close(MAIL) || exit 0;

# Delivery OK, but don't continue in .qmail file
exit 99;
