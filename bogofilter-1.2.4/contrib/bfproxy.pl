#!/usr/bin/perl -Tw
require 5.006_001;
use strict;
use sigtrap;

=head1 NAME

bfproxy - performs bogofilter functions via email

=cut

my $version = "0.3.5";

################################################
############### Copyleft Notice ################
################################################

# Copyright © 2003 Order amid Chaos, Inc.
# Author: Tom Anderson 
# neo+bfproxy@orderamidchaos.com
# 
# This program is open-source software; you can redistribute it 
# and/or modify it under the terms of the GNU General Public 
# License, v2, as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public 
# License along with this program; if not, write to: 
#
# Free Software Foundation 
# 59 Temple Place, Suite 330
# Boston, MA 02111-1307  USA
#
# http://www.gnu.org/

#################################################
################# Documentation #################
#################################################

# use "perldoc bfproxy" or "bfproxy -h" to read this

=head1 SYNOPSIS

=head2 Command line usage:

B<bfproxy> [I<options>] < [I<rfc822_email>]

=head2 Procmail usage (recommended):

Add to ~/.procmailrc the following recipe.  This recipe needs to 
be BEFORE the filtering recipe to prevent reclassifying all of 
the forwarded messages again.

  :0fw
  * ^To: $USER\+bfproxyID
  | $HOME/.bogofilter/bfproxy

Where I<$HOME> is your home directory, if not set in the environment, 
I<$USER> is your login name, if not set, and "I<bfproxyID>" is any 
string by which you choose to identify mail that should be 
processed by bfproxy.  Essentially, you are matching your regular 
email address with "I<+bfproxyID>" after it. Eg:

  joe.shmoe+mybfproxystring@somedomain.com

Joe's procmail recipe would then be:

  :0fw
  * ^To: joe.shmoe\+mybfproxystring
  | $HOME/.bogofilter/bfproxy

Don't use the default "I<bfproxyID>" though, and keep yours 
confidential so that people can't send bogofilter commands on your 
behalf.  The from address would have to be spoofed, and you'd get a 
report anyway, but why tempt fate?  Using your own unique 
"I<bfproxyID>" helps prevent automated attacks by spammers on 
bogofilter users.

When entering the bogofilter recipe after the bfproxy recipe, you 
should add the C<E> flag so that bogofilter does not filter the 
results of running bfproxy.  Eg:

  :0Efw
  | bogofilter -uep

It's not necessary, but suggested.  If you use the C<-u> option with 
bogofilter, these bfproxy-trained emails would be trained again
otherwise, which is generally not what you want.  You would also 
risk having the results thrown in with your spam, which would also
be undesireable.  Therefore, it's best to use the C<E> flag in the
bogofilter recipe.

Just remember to make sure that the bogofilter recipe is AFTER the 
bfproxy recipe to prevent screwing up your database by double-
classifying those attached emails.

=head2 Email usage:

When using an MDA such as Procmail to handle running bfproxy, as
intended, you'll need to send yourself emails with the special
"I<bfproxyID>" included, followed by any options. Eg:

[I<USER>]+[I<bfproxyID>]-[I<options>]@[I<DOMAIN>]

The email you send to this address should contain any emails you 
want to correct as attachments.  Using your email client's forward-
as-attachment functionality should suffice.  All such attached
emails will be processed according to address-line options.

=head1 OPTIONS:

=head2 Address-line options:

=over 4

=item B<C>

all emails with an original bogosity of I<spam> or I<ham> will
be unregistered and then re-registered as the opposite bogosity 
(C<-Sn>, C<-Ns>).  This is useful if the emails were originally 
auto registered with the C<-u> flag or if a user error occurred.

=item B<R>

all emails with an original bogosity of I<spam> or I<ham> will
be registered as the opposite bogosity (C<-n>, C<-s>).

=item B<N>

unregister all attached emails from the ham database

=item B<S>

unregister all attached emails from the spam database

=item B<n>

register all attached emails as ham

=item B<s>

register all attached emails as spam

=item B<x>

Repeatedly register emails that are within the "unsure" zone
of spamicity values (between ham_cutoff and spam_cutoff) until 
they are correctly classified or until I<rmax> has been reached.

=item B<v>

Verbose output.  Show per-email registration information in 
addition to the subject lines and summary.

=item B<b>

Show benchmarking information in the output.

=item B<c>

Display bogofilter configuration info in the output.

=back


If no address-line command is provided, it defaults to C<C> for 
"correction" mode.  If you choose to use C<C> or C<R> and wish to
register unsures at the same time, you can add a secondary command
after the C<C> or C<R> to register unsures as either spam (C<s>) 
or ham (C<n>).  Eg:

    joe.shmoe+mybfproxystring-Rs@somedomain.com
    
The result of the above command string will be that all emails
previously classified as spams will be registered as hams, all
emails previously classified as hams will be registered as spams, 
and all emails previously classified as unsures will be registered 
as spams.  If a secondary command is not used with C<C> or C<R>, 
then unsures are not registered.

=head2 Command-line options:

=over 4

=item B<h>

display this help file

=back

=head1 REQUIRES

=over 4

=item *

Perl 5.6.1

=item *

bogofilter

=back

=head1 DESCRIPTION

Bfproxy accepts an email on stdin (generally from procmail or some
other MDA) containing one or more forwarded-as-attachment emails.  
It will extract the attached emails and remove [SPAM] indicators 
from the subject line.  Then, it'll determine the original 
classification of each email from its X-Bogosity field, and pass 
the original emails to bogofilter with the appropriate flags as
determined by address-line options.  

The attachments containing the emails should be message/rfc822 
format, which is how most email clients will do 
forwarding-by-attachment by default.  If they are not in this 
format, then it is quite likely that important header information 
is being omitted, and bfproxy will ignore such attachments.  
Moreover, bfproxy will only recognize attachments in encapsulating 
MIME types multipart/mixed or multipart/digest.  Quoted-text 
attachments will not work, nor will inline attachments which do not 
conform to the previous requirements (rfc822 and mixed or digest).  
Just about any MUA should be able to meet these requirements for 
forwarding emails.  If there is one that uses a slightly different, 
but possibly acceptable format, please send a bug report to 
neo+bfproxy@orderamidchaos.com.

Also, bfproxy will not parse attached emails multiple MIME levels.
If an attachment has an attachment, then the whole thing will get
parsed as a single email rather than accounting for the headers in
the attached email.  Do not nest corrections within each other.

This scheme uses "subaddressing" to direct the MDA to run the 
email through bfproxy without requiring a new user or alias on the 
system.  Moreover, the output of the operation will arrive in the 
user's mailbox without having to resend a second email.  This also
allows bfproxy to run with the permissions of the user whose 
database is to be altered by the command, and access his/her 
environment, including the appropriate config files and db files.  
See RFC3598 (http://www.ietf.org/) for details on "subaddressing".  
It is supported by common MTA's such as Sendmail.

=head1 FAQ

=head2 Why isn't it working?

If you get an error the first time you try it, such as bfproxy 
saying it received zero emails for correction, then view the source 
of the email you sent and make sure the attachments use 
message/rfc822 format, and that the attachments are contained in a 
MIME part of multipart/mixed or multipart/digest.  If not, check your 
email client's options to see if you can get it into that format.  If 
you get any other errors/problems, please submit a bug report.

=head2 Why call it 'bfproxy' instead of 'bogoproxy', etc.?

Because "bogo" refers to bogons which are the elementary particle
of bogosity which bogofilter filters.  We're not proxying bogons
but commands to bogofilter.  Therefore, despite the lyrical 
synergy that "bogoproxy" would have, it simply wouldn't make any
logical sense, whereas the less elegant bfproxy is more 
descriptive of its actual function... proxying "bf" commands.

=head2 What happens to X-headers from my client/proxy/etc?

X-headers are left as-is now, unlike initial bfproxy behavior.  The 
reason is that bogofilter's Bayesian algorithm will essentially make
those X-headers irrelevant to classifications since an equal number
will come from spams as from hams.  Therefore emails will end up
being classified based on other tokens instead.  And this way, 
X-headers from senders' clients/proxies/servers/etc may be useful in
classifying.  Furthermore, this reduces the number of ad hoc rules
that bfproxy applies and would need to be maintained.  Bogofilter
already strips the X-Bogosity header, so it is unnecessary to do so
in bfproxy.

=head1 BUGS

=over 4

=item Please report any.

=back

=head1 TODO

=over 4

=item Add more verbosity options

=item Use bogofilter -Q to obtain non-standard tags, formats, etc.

=item Suggestions welcome.

=back

=head1 SEE ALSO

L<bogofilter>

=head1 AUTHOR

Tom Anderson <neo+bfproxy@orderamidchaos.com>

=cut

#################################################
############### User Variables  #################
#################################################

# please edit according to your setup

# default path
our $path = "/bin:/usr/bin:/usr/local/bin";

# default shell
our $shell = "/bin/sh";

# address-line command string (after the "+", before the "@")
our $command = "bfproxyID";

# maximum number of recursions for train-to-exhaustion
our $rmax = 10;

# delivery username; leave blank unless your MTA is unable
# to deliver to individual virtual host users but instead
# delivers to a single shared virtual host user
our $mailbox = "";  # most setups will not need this

# of course, modify the first line of this file, 
# the shebang, to point to your perl interpreter

# do not edit below this line unless you really
# know what you're doing

#################################################
############## Include Libraries ################
#################################################

use IO::Select;
use IPC::Open2;
use Fcntl qw(:flock);
use Benchmark;

#################################################
############## Default Globals ##################
#################################################

$> = $<; # set effective user ID to real UID
$) = $(; # set effective group ID to real GID

# Make %ENV safer
delete @ENV{qw(IFS CDPATH ENV BASH_ENV PATH SHELL)};

# Set the environment explicitely
$ENV{PATH} = $path;
$ENV{SHELL} = $shell;

################################################
##################### Main #####################
################################################

# print help info
if (defined @ARGV && $ARGV[0] =~ /^-+h.*/) 
{
	my $bfproxy = $1 if $0 =~ /^([\w\/.\-~]*)$/;
	system("perldoc $bfproxy"); exit(0);
}

# determine bogofilter settings
our ($robx, $robs, $min_dev, $ham_cutoff, $spam_cutoff) = get_config();

# extract the header
my ($header, $boundary, $from, $options) = extract_header($command, $version);

# run bogofilter on the extracted emails and gather the results
my $results = process_emails($boundary, $from, $options);

# output the new email containing the results
print "$header\n$results\n";

################################################
############## Get Configuration  ##############
################################################

sub get_config
{	
	my ($robx, $robs, $min_dev, $ham_cutoff, $spam_cutoff) = (0,0,0,0,0);

	open (CONF, "bogofilter -Q |") or error("warn", "Could not get bogofilter settings");
	
	while (<CONF>) 
	{
		my $line = $_;
		
		$robx = ($1+0) if $line =~ /robx\s*?=\s*?([0-9\.]+?)\s/i;
		$robs = ($1+0) if $line =~ /robs\s*?=\s*?([0-9\.]+?)\s/i;
		$min_dev = ($1+0) if $line =~ /min_dev\s*?=\s*?([0-9\.]+?)\s/i;
		$ham_cutoff = ($1+0) if $line =~ /ham_cutoff\s*?=\s*?([0-9\.]+?)\s/i;
		$spam_cutoff = ($1+0) if $line =~ /spam_cutoff\s*?=\s*?([0-9\.]+?)\s/i;
	}
	close CONF;
	
	return $robx, $robs, $min_dev, $ham_cutoff, $spam_cutoff;
}

################################################
############### Extract Header  ################
################################################

sub extract_header
{
	my $command = shift;
	my $version = shift;
	my $header = "";
	my $boundary = "";
	my $multipart = 0;
	my $from = "";
	my $to = "";

	while (<STDIN>)
	{
		my $line = $_;
		
		# record the boundary of the multipart/mixed or digest MIME message
		$multipart = 1 if $line =~ /Content-Type: multipart\/(?:mixed|digest);/i;
		$boundary = $1 if $multipart && $line =~ /boundary="(.*?)"/i;

		# replace the from, subject, and content-type lines in the headers
		$from = $1 if $line =~ s/^From:\s(.*)$/From: Bfproxy v$version/i;
		$line =~ s/^Subject:\s.*$/Subject: training results/i;
		$line =~ s/^Content-type:\s.*$/Content-Type: text\/plain/i;
		$to = $1 if $line =~ /^To:\s(.*)$/i;
		
		$header .= $line; # if $line =~ /^\w*?:\s|^From\s/i; # unless $line =~ /^X-[^\s]*?:/i;
		
		# we're done with the header when we've found a blank line
		last if $line !~ /[^\s]/i;		
	}
	
	# extract the "user" portion of the "from" address
	my $user = $1 if $from =~ /.*?((?:\w|-|\.)+?)\@.*$/i;
	
	# parse any address-line flags
	my $options = ($to =~ /.*?$user\+$command\-(.*?)\@.*$/i)? $1 : "";

	return $header, $boundary, $from, $options;
}

################################################
############### Process Emails  ################
################################################

sub process_emails
{
	# 1) Read the email from STDIN
	# 2) Discard the enveloping email and process only the "message/rfc822" MIME parts
	#    representing the forwarded-as-attachment incorrectly classified emails
	# 3) Send emails to bogofilter for registration
	# 4) Output the results

	my $boundary=shift||""; # multipart boundary
	my $from = shift || "";	# address of sender
	my $options=shift ||"";	# address-line flags

	my @count = (0,0,0,0);	# count of emails (found, processed, lines, words)
	my $locked = 0;		# lock the boundary at the shallowest level where rfc822 content found
	my %email;		# hash to temporarily hold a single email
	my $found = 0;		# indicator of when to record an email
					# 0: no rfc822 messages found yet
					# 1: we've got a message/rfc822 header
					# 2: we're out of the header part
					
	# start timing the process
	my $start_time = new Benchmark if $options =~ /b/;

	# begin generating output
	my $results .= "Bfproxy has processed the following emails with option $options:\n\n";

	while (<STDIN>)
	{
		my $line = $_;
		$count[2]++;
	
		if ($found < 2) # try to find any new attached emails
		{
			# record the boundary of the multipart/mixed or digest MIME message
			unless ($locked) { $boundary = $1 if $line =~ /Content-Type: multipart\/(?:mixed|digest); boundary="(.*?)"/i; }
			
			# we've found a new attached email if the content-type is message/rfc822 and we have a boundary
			if ($line =~ /Content-Type: message\/rfc822/i && $boundary)
			{
				$found++;
				$locked = 1; # lock the boundary on first attachment
			}

			# we're ready to start recording if we're out of the attachment header (found a blank line)
			$found++ if ($line !~ /[^\s]/i && $found == 1);
		}
		else # if we're inside an attached email, record it
		{
			if ($line !~ /\Q$boundary\E/) 
			{
				# strip [SPAM] token from subject
				$line =~ s/^Subject: (.*?)\[(?:SPAM|UNSURE)\]/Subject: $1/i;				
			
				# escape From lines in body
				$line =~ s/^(From) />$1 /i;
			
				# record properties of this email
				$email{'spam'} = 'U' if $line =~ /^X-Bogosity: (?:Unsure)/i;
				$email{'spam'} = 'S' if $line =~ /^X-Bogosity: (?:Yes|Spam)/i;
				$email{'spam'} = 'H' if $line =~ /^X-Bogosity: (?:No|Ham|Clean)/i;
				$email{'spamicity'} = $1 if $line =~ /spamicity=((?:\d|\.)*?), /;
				$email{'content'} .= "$line" unless $line =~ /^X-[^\s]*?:/i;
				$email{'return-path'} = $1 if $line =~ /^Return-Path: <?(.*?)(?=>|\n)/i;
				$email{'from'} = $1 if !defined $email{'from'} && $line =~ /^From: (?:.*?<)?(.*?)(?=>|\n)/i;
				$email{'subject'} = $1 if $line =~ /^Subject: (.{0, 40})(.*?)/i; 
				$email{'subject'} .= "..." if $line =~ /^Subject: .{40}.+?/i;
			}
			else # the email is finished once we've come to a boundary -- send it to bogofilter
			{
				$count[0]++;
				$email{'subject'} = "No Subject" unless $email{'subject'};
				$email{'spam'} = 'U' unless defined $email{'spam'};
				$email{'spamicity'} = "None" unless $email{'spamicity'};	
				
				my $training = train(\%email, $options, 0, \@count);

				# per-email output
				$results .= "subject: $email{'subject'}\n";				
				$results .= "original spamicity: $email{'spamicity'}\n" . $training if $options =~ /v/;
				
				# close this email and advance to the next
				undef %email;
				$found = 0; 
			}
		}
	}

	$results .= "\n";
	$results .= "$count[0] emails found, containing $count[2] lines total.\n";
	$results .= "$count[3] words from $count[1] emails were registered.\n\n";
	
	if ($options =~ /b/)
	{
		# calculate total running time
		my $end_time = new Benchmark;  
		my $td = timediff($end_time, $start_time);
		my $cpu = $td->[1]+$td->[2]+$td->[3]+$td->[4];
		my $wall = $td->[0];
		my $per_line = $cpu / $count[2]; $per_line = int(($per_line*1000) + .5 * ($per_line <=> 0)) / 1000;
		my $per_mail = $cpu / $count[0]; $per_mail = int(($per_mail*1000) + .5 * ($per_mail <=> 0)) / 1000;
	
		$results .= "Total running time was $wall wallclock secs, $cpu CPU secs.\n";
		$results .= "$per_line CPU secs/line, $per_mail CPU secs/email.\n";
		$results .= "Bfproxy required ".$td->[1]." usr + ".$td->[2]." sys = ".($td->[1]+$td->[2])." CPU secs.\n";
		$results .= "Bogofilter required ".$td->[3]." usr + ".$td->[4]." sys = ".($td->[3]+$td->[4])." CPU secs.\n\n";
	}
	
	if ($options =~ /c/)
	{
		$results .= "robx=$robx, robs=$robs, min_dev=$min_dev, spam_cutoff=$spam_cutoff, ham_cutoff=$ham_cutoff, rmax=$rmax\n";
	}
	
	return $results;
}

################################################
############### Perform Training ###############
################################################

sub train 
{
	my $email = shift or error("warn", "No email provided");
	my $options = shift || "C";
	my $r = shift || 0; # recursion counter for exhaustive training
	my $count = shift || (0,0,0,0);
	my $results = "";
		
	if ($options =~ /[CR]/) # if action is corrective, check the bogosity to decide what to do
	{
		if ($email->{'spam'} == 'S') # email was classified as SPAM
		{
			my $flag = ($options =~ /C/)? "Sn" : "n";
			$results .= "user classification: ham\ncommand: bogofilter -${\$flag}\n" unless $r;
		
			my ($status, $words, $output) = bogofilter("-${\$flag}evD 2>&1", $email->{'content'});
			unless ($status || $r) { $count->[1]++; $count->[3]+=$words; $results .= "words: $words\n"; }
			elsif ($status) {$results .= "could not process this email: $status\n";}
		}
		elsif ($email->{'spam'} == 'H') # email was classified as HAM
		{    
			my $flag = ($options =~ /C/)? "Ns" : "s";
			$results .= "user classification: spam\ncommand: bogofilter -${\$flag}\n" unless $r;
				
			my ($status, $words, $output) = bogofilter("-${\$flag}evD 2>&1", $email->{'content'});
			unless ($status || $r) { $count->[1]++; $count->[3]+=$words; $results .= "words: $words\n"; }
			elsif ($status) {$results .= "could not process this email: $status\n";}
		}
		elsif ($email->{'spam'} == 'U') # email was classified as UNSURE
		{
			my $new_options = $options; $new_options =~ s/[CR]//gs;
			$results .= train($email, $new_options, $r, $count); 
			return $results;
		}
	}
	elsif ($options =~ /[NSns]/) # if action is direct, just do it
	{
		my $flag = $options; $flag =~ s/[^NSns]//gs;
		
		my $class = ($flag =~ /s/)? "spam" : ($flag =~ /n/)? "ham" : "none";
		$results .= "user classification: $class\ncommand: bogofilter -${\$flag}\n" unless $r;
			
		my ($status, $words, $output) = bogofilter("-${\$flag}evD 2>&1", $email->{'content'});
		unless ($status || $r) { $count->[1]++; $count->[3]+=$words; $results .= "words: $words\n"; }
		elsif ($status) { $results .= "could not process this email: $status\n"; }
	}
	else { $results .= "could not process this email: no option passed in for this condition\n"; }

	# check new spamicity				
	my ($status, $words, $output) = bogofilter("-te 2>&1", $email->{'content'});
	unless ($status) { $output =~ s/^.*?((?:\d|\.)*?)$/$1/;	$results .= "new spamicity: $output"; }
	else {$results .= "could not classify this email: $status\n";}
				
	# train-to-exhaustion
	$options =~ s/[^nsx]//gis; # only do registration on subsequent recursions
	$results .= train($email, $options, $r+1, $count) if $options =~ /x/ && $r < $rmax && (($options =~ /s/ && $output < $spam_cutoff) || ($options =~ /n/ && $output > $ham_cutoff));
	
	$results .= "\n" unless $r;
	
	return $results;
}

################################################
################ Run Bogofilter ################
################################################

sub bogofilter
{
	my $options = shift || "";
	my $email = shift || "";
	my $status = 0;
	my $output = "";
	my $words = "";

	# for environments that don't split virtual host users into system users
	$options = "-d $ENV{HOME}/.bogofilter/$mailbox " . $options if $mailbox;

	# trap broken pipes
	$SIG{PIPE} = \&sig_trap;

	# fork pipe to bogofilter
	my $pid = open2(\*R, \*W, "bogofilter $options") or error("die", "Could not open pipe to bogofilter: $!");
		
	# lock filehandle
	assert_dominance (\*W, LOCK_EX);

	# create filehandle references
	my $R = *R{IO};	my $W = *W{IO};
	
	# create select object and add handles
	my $sel = IO::Select->new($R, $W) or error("die", "Cannot create IO::Select object: $!");
	
	# set autoflush
	select((select(W), $| = 1)[0]);

	# select strings
	my $ex = join(', ', $sel->has_exception(0));
	my $cw = join(', ', $sel->can_write(0));
	my $w  = "$W";
	
	# send email to bogofilter
	unless ($ex =~ /\Q$w\E/) 	# unless there was an exception on this filehandle
	{
		if ($cw =~ /\Q$w\E/) 	# if this filehandle is writeable
		{
			syswrite W, $email or error("die", "Cannot write to pipe: bogofilter $options: $email: $!");
		} 
		else {error("warn", "Filehandle $w is not in ready list: $cw: $!");}
	} 
	else {error("warn", "Filehandle $w had an exception: $!");}
	
	# close write filehandle to flush the buffer and read from the process outputs
	if (close W)
	{
		$status = $? >> 8;  # exit status
		sysread R, $output, 32 or error("warn", "Cannot read from pipe: $!");
		$words = $1 if $output =~ /^#\s*?(\d*?) words.*/;
	} 
	else {error("warn", "Could not flush output to bogofilter: $!");}

	# close read filehandle
	unless (close R) {error("warn", "Could not close input from bogofilter: $!");}

	# terminate child processes
	waitpid $pid, 0;

	return $status, $words, $output;
}

################################################
################ Error Handling ################
################################################

sub error
{
	my ($action, $msg) = @_;

	die $msg if $action eq "die";
	warn $msg unless $action eq "die";
	# add other actions if you like
}

sub sig_trap
{
	my $sig = shift;
	my ($action, $more) = ("warn", "");

	sig: 
	{
		$action = "warn", last sig if $sig =~ /ALRM/;
		$action = "warn", last sig if $sig =~ /PIPE/;
		$action = "warn", last sig if $sig =~ /CHLD/;
		$action = "die" , last sig if $sig =~ /INT/;
		$action = "die" , last sig if $sig =~ /HUP/;
		$action = "warn";
	}
	
	my $waitedpid = wait;
	$more = "; Reaped pid $waitedpid, exited with status " . ($? >> 8) if $waitedpid;

	$SIG{$sig} = \&sig_trap;

	error ($action, "Trapped signal SIG$sig$more");
}

################################################
################# File Locking #################
################################################

sub assert_dominance 
{
	my ($handle, $type) = @_;

	# assert yourself
	unless (flock ($handle, $type)) 
	{
		# get impatient
		sleep 3;

		# reassert yourself or give up
		unless (flock ($handle, $type)) { error ("die", "File lock error: $!"); }
	}
	
	seek $handle, 0, 2;
}
