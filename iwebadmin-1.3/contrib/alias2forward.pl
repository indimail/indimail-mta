#!/usr/bin/perl

# Written July 17, 2003 by Tom Collins <tom@tomlogic.com>
# Distributed as part of QmailAdmin 1.0.25 and later
# <http://sourceforge.net/projects/qmailadmin/>

# VERSION HISTORY
# 17-Jul-03 1.0 Initial release.

print <<EOF;
WARNING: This program could potentially corrupt your .qmail-user files.  
PLEASE make a backup of ~vpopmail/domains in case you need to restore any 
files.  Also, PLEASE do a dry run first and review the changes that would 
be made on a "real" run of the program.

Use the alias2forward.pl program to go through your existing domains
and convert the aliases in .qmail-user files to forwards.  The newer
versions of QmailAdmin have switched to using forwards for local and
remote users, instead of Maildir delivery for local users.  This
allows for proper processing of the .qmail file in the user's directory.

Note that you must run this script as root or the vpopmail user.

EOF

my $QMAILDIR = '/var/qmail';
my ($dryrun, $domain);

print "We recommend that you do a dry run first and watch the output to\n";
print "ensure that all changes will be as expected.\n";
print "Would you like to do a dry run? ";
$dryrun = <>;
chop $dryrun;
exit unless $dryrun;
$dryrun = ($dryrun =~ /^y/i);

print "\nEnter domain to convert, or leave blank for all domains: ";
$convert = <>;
chop $convert;

while (! -r "$QMAILDIR/users/assign")
{
	print "Couldn't open $QMAILDIR/users/assign.  Please enter your\n";
	print "qmail path (usually /var/qmail): ";
	$QMAILDIR = <>;
	chop $QMAILDIR;
	exit unless $QMAILDIR;
}

my %processed;

if ($convert) { print "Looking up $convert in $QMAILDIR/users/assign\n"; }
else { print "Processing domains in $QMAILDIR/users/assign\n"; }

open (ASSIGN, "$QMAILDIR/users/assign") ||
	die "Error, can't read $QMAILDIR/users/assign.\n";
while ($entry = <ASSIGN>) {
	chop $entry;
	last if ($entry eq '.');   # assign file ends with "."
	($alias, $domain, $uid, $gid, $path, $junk) = split (/:/, $entry);

	# only process if we have a domain name and path (sanity check)
	next unless ($domain && $path);

	# only process selected domain
	next if ($convert && ($convert ne $domain));

	# strip leading + and trailing - from alias
	$alias =~ s/.(.*)./$1/;

	if ($processed{$domain}) {
		print "Skipping $alias (alias to $domain, already processed).\n";
		next;
	}

	print "Processing $domain...\n";
	$processed{$domain}++;
	opendir (DOM, $path);
	while ($fn = readdir (DOM))
	{
		# only process .qmail files
		next unless ($fn =~ /^\.qmail-/);

		# skip the .qmail-default file
		next if ($fn eq '.qmail-default');

		# skip symbolic links (ezmlm files)
		next if (-l "$path/$fn");

		print "  processing $fn\n";

		$dotqmail = '';
		$changed = 0;
		open (DOTQMAIL, "$path/$fn");
		while ($line = <DOTQMAIL>) {
			if ($line =~ /\/.*\/([^\/]{2,})\/(.\/)?([^\/]+)\/(Maildir|\.maildir)(\/)?$/) {
				chop $line;
				($user, $domain) = ($3, $1);
				print "    converting '$line' to '&$user\@$domain'\n";
				$dotqmail .= "&$user\@$domain\n";
				$changed++;
			} else {
				$dotqmail .= $line;
			}
		}
		close (DOTQMAIL);

		# rewrite qmail file if changed, should keep permissions
		next unless $changed;
		if ($dryrun) {
			print "    (saving of changes skipped due to dry run).\n";
			next;
		}
		print "    saving changes\n";
		open (DOTQMAIL, ">$path/$fn") ||
			die "Can't write to $path/$fn, make sure you're running as root or vpopmail.\n";
		print DOTQMAIL $dotqmail;
		close (DOTQMAIL);
	}
	closedir (DOM);
}
close (ASSIGN);

