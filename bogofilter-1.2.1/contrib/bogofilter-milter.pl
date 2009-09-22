#!/usr/bin/perl

# $Id: bogofilter-milter.pl 6722 2008-04-30 22:12:08Z relson $
#
# bogofilter-milter.pl - a Sendmail::Milter Perl script for filtering
# mail using individual users' bogofilter databases.
#
# (additional information below the coypright statement)

# Copyright 2003, 2005, 2007 Jonathan Kamens
# <jik@kamens.brookline.ma.us>.  Please send me bug reports,
# suggestions, criticisms, compliments, or any other feedback you have
# about this script!
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version. 
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details. 

# You will need the following non-standard Perl modules installed to
# use this script: Sendmail::Milter, Mail::Alias, Proc::Daemon,
# IO::Stringy, Socket, Net::CIDR.  Before using this script, search
# for CONFIGURABLE SETTINGS and configure them appropriately for your
# site.
#
# Inserts "X-Bogosity: Spam, tests=bogofilter" into messages that
# appear to be spam (or "Ham" into ones that don't).  If the message is
# rejected, you usually won't see the "Spam", but see below about
# training mode.
#
# Save this script somewhere, launch it as root (by running it in the
# background or invoking it with "--daemon" in which case it will
# background itself), and reconfigure your sendmail installation to
# call it as an external filter (probably by calling INPUT_MAIL_FILTER
# in your sendmail.mc file).  Running this script as root should be
# safe because it changes its effective UID and GID whenever
# performing operations on individual users' files (if you find a
# security problem, please let me know!).
#
# NOTE: You will want to take steps to ensure that this script is
# started before sendmail whenever your machine boots, e.g., by
# creating an appropriate script in /etc/rc.d/init.d with appropriate
# links to it in /etc/rc.d/rc?.d, because once you configure sendmail
# to talk to a particular milter, it may refuse to deliver email if
# that milter isn't running when the email comes in.
#
# For additional information about libmilter and integrating this or
# any other libmilter filter into your sendmail installation, see the
# file README.libmilter that ships with sendmail and/or the section
# entitled "ADDING NEW MAIL FILTERS" in the README file that ships
# with the M4 sendmail CF templates.
#
# You may need to restart this script to get it to notice changes in
# mail aliases.

# This script logs various informational, warning and error messages
# to the "mail" facility.

# Thanks to Tom Anderson <neo+bogofilter-milter@orderamidchaos.com>
# for the IP whitelisting changes and for several other useful
# suggestions and bug fixes.

# BEGIN CONFIGURABLE SETTINGS

# If this string appears in the Subject of a message (case
# insensitive), the message won't be filtered.
my $magic_string = '[no-bogofilter]';

# Set the syslog facility you wish to log messages to.
my $log_facility = 'LOG_MAIL';

# These settings control exactly what error sendmail sends back to the
# sender if a message is rejected.  You can leave them as-is, or
# customize them as desired.
my $rcode = 550; # three-digit RFC 821 SMTP reply
my $xcode = "5.7.1"; # extended RFC 2034 reply code
my $reject_message = "Your message looks like spam.\n" .
    "If it isn't, resend it with $magic_string " .
    "in the Subject line.";

# Whitelist any IP addresses or ranges from this filter.
# For example:
#my(@whitelist) = ("127.0.0.1", "10.127.0.1-10.127.0.9", "192.168.0.0/16");
my(@ip_whitelist) = ();

# If you want to whitelist any addresses which have authenticated
# via poprelayd (i.e. remote workstations of users on your server)
# set $dbfile to your popip.db location, else set it to undef.
# For example:
#my $ip_whitelist_db = "/etc/mail/popip.db";
my $ip_whitelist_db = undef;

# The largest message to keep in memory rather than writing to a
# temporary file.
my $MAX_INCORE_MSG_LENGTH = 1000000;

my $pid_file = '/var/run/bogofilter-milter.pid';

# Whatever path you specify for $socket needs to match the socket
# specified in the sendmail.cf file (with "local:" in front of it
# there, but not here).
my $socket = '/var/run/bogofilter-milter.sock';

# The following two settings give more granular control over whether
# bogofilter is used for any particular user and what configuration
# settings are used when it is.
# - If $bogofilter_cf is set, then the script will look for a file
# with that name in the user's home directory.  If it finds it, then
# bogofilter will be called with "-c $HOME/$bogofilter_cf" so that the
# specified configuration file is used rather than the default,
# .bogofilter.cf.
# - If $require_cf is true, then the specified configuration file
# *must* exist for bogofilter to be used for this user.  In other
# words, rather than only looking for the .bogofilter subdirectory of
# the user's home directory, the script will look for both the
# .bogofilter subdirectory *and* the config file.
# - Note that $require_cf is ignored if $bogofilter_cf is unset.
my $bogofilter_cf = undef;
my $require_cf = undef;

# If a file with this name exists in the user's .bogofilter directory,
# then that user's mail will be filtered in training mode.  This means
# that the message will be filtered and registered as spam or non-spam
# and the appropriate X-Bogosity header will be inserted, but it'll be
# delivered even if bogofilter thinks it's spam.  This allows the user
# to detect false positives or false negatives and feed them back into
# bogofilter to train it.  To disable this functionality set
# $training_file to undef.
my $training_file = 'training';

# If a file or link with this name exists in the user's .bogofilter
# directory, then copies of rejected messages will be saved in this
# file in mbox format, using flock locking.  To disable rejected
# message archiving, set $archive_mbox to undef.
my $archive_mbox = 'archive';

# If $cyrus_deliver is set to an existing executable, then it is
# assumed to be a Cyrus IMAP "deliver" program.  If the $archive_mbox
# for a particular user is a symlink pointing at a nonexistent file
# whose name starts with "cyrus:", then everything after the "cyrus:"
# is assumed to be the name of a Cyrus IMAP folder within the user's
# mailbox to which to deliver the spam message instead of saving it
# into an mbox format file.
my $cyrus_deliver = '/usr/lib/cyrus-imapd/deliver';

# If you would like to use a shared bogofilter database for everyone,
# rather than separate per-user databases, then create a user on your
# system to be used as a home for the shared database, and set
# $database_user to that user's username.
# 
# If you set $database_user, then all the logic described above for
# deciding whether to run bogofilter, whether to run in training mode
# or real mode, and whether to archive spam still applies, so make
# sure you configure $database_user's account properly.
# 
# If you set $database_user, then $aliases_file, $sendmail_canon,
# $sendmail_prog, $recipient_cache_expire, and
# $recipient_cache_check_interval do NOT apply and are ignored.
my $database_user = undef;

# Mail::Alias is used to expand SMTP recipient addresses into local
# mailboxes to determine if any of them have bogofilter databases.  If
# someone sends E-mail to a mailing list or alias whose expansion
# contains one or more local users with bogofilter databases, then one
# of those users' database (which one in particular is not defined)
# will be used to filter the message.  To disable this functionality
# and remove the dependency on Mail::Alias, comment out the "use
# Mail::Alias;" line and set $aliases_file to undef in the
# configuration section.  With this functionality disabled, mail will
# only be filtered if it is sent directly to a user in the passwd
# file.  On the other hand, with this functionality enabled, one
# person's bogofilter database can cause a message to be filtered for
# everyone on a local mailing list.
my $aliases_file = '/etc/aliases';

# If you want the milter to ask sendmail to canonicalize recipient
# addresses before trying to alias-expand them, then set
# $sendmail_canon to true and $sendmail_prog to the path of the
# sendmail binary to invoke.  This is necessary, e.g., if you use a
# virtual user table for some recipients that do sendmail filtering.
# You may also wish to examine the sendmail_canon subroutine below,
# because it may not be right for your particular sendmail
# configuration.  Search for CHECKTHIS in the function.
my $sendmail_canon = 1;
my $sendmail_prog = '/usr/sbin/sendmail';

# @discard_control is an array of anonymous arrays.  Each sub-array
# contains a pair of entries, a control pattern and an action, either
# "discard" or "reject".  The action corresponding to the first
# matching control pattern determines what happens to the messages.
# If @discard_control is empty or none of its control patterns match,
# the default action is "reject".  The following control patterns are
# valid:

# "addr:a.b.c.d"       matches if the sending host has the indicated IP address
# "netblock:a.b.c.d/e" matches if the sending host is in the indicated netblock
# "host:fqdn"          matches if the IP address of the sending host resolves
#                      to the indicated host name
# "domain:fqdn"        matches if the IP address of the sending host resolves
#                      to a host name in the indicated domain
# "mx"                 matches if one of the MX servers for the recipient's
#                      domain resolves to the IP address of the sending host
# "*"                  always matches

# The default @discard_control setting discards messages from MX
# servers to prevent this script from contributing to spam "blowback",
# which occurs when a spammer forges someone's real email address as
# the return address on spam, and then that person has to deal with
# tons of bounce messages from sites that reject the spam.
my(@discard_control) =
    (
     ["mx" => "discard"],
     ["*"  => "reject"],
     );

# You can configure how long addresses will stay in the cache of
# addresses that have been been expanded against the virtual user
# table (if $sendmail_canon is set above), then expanded against the
# aliases file (if $aliases_file is set above), then checked to see if
# they represent users who are doing filtering.  You would want cache
# entries to time out if you get a lot of spam dictionary attacks
# against your mail server, when the spammers try tons of invalid
# addresses on the off chance that one of them might be valid, because
# in that case your cache will grow without bound and the bogofilter
# milter process will get really large.  Set this to 0 to disable
# cache expiration, or to the number of seconds after which cache
# entries should expire.
my $recipient_cache_expire = 24 * 60 * 60; # one day
# How often to expire entries from the cache.
my $recipient_cache_check_interval = 60 * 60; # one hour

# You may wish to remove this restriction, by setting this variable to
# 0, if your site gets a lot of mail, but I haven't tested the script
# to make sure it functions correctly with multiple interpreters.
my $milter_interpreters = 1;

# END CONFIGURABLE SETTINGS

require 5.008_000; # for User::pwent

use strict;
use warnings;
use DB_File;
use Data::Dumper;
use English '-no_match_vars';
use Fcntl qw(:flock :seek);
use File::Basename;
use File::Temp qw(tempfile);
use Getopt::Long;
use IO::Scalar;
use IPC::Open2;
use Mail::Alias;
use Net::CIDR;
use Net::DNS;
use POSIX;
use Proc::Daemon;
use Sendmail::Milter;
use Socket;
use Sys::Syslog qw(:DEFAULT :macros setlogsock);
use User::pwent;

$Data::Dumper::Indent = 0;

# Used to cache the results of alias expansions and checks for
# filtered recipients.
my %cached_recipients;

my $whoami = basename $0;
my $usage = "Usage: $whoami [--daemon] [--debug]\n";
my($run_as_daemon, $get_help, $debug);

my %my_milter_callbacks =
(
 'envrcpt' => \&my_rcpt_callback,
 'header'  => \&my_header_callback,
 'eoh'     => \&my_eoh_callback,
 'body'    => \&my_body_callback,
 'eom'     => \&my_eom_callback,
 'abort'   => \&my_abort_callback,
 'close'   => \&my_close_callback,
 );

$my_milter_callbacks{'connect'} = \&my_connect_callback
    if (@ip_whitelist || $ip_whitelist_db || @discard_control);

dia $usage if (! GetOptions('daemon' => \$run_as_daemon,
			    'debug' => \$debug,
			    'help|h|?' => \$get_help));
if ($get_help) {
    print $usage;
    exit;
}

if ($run_as_daemon) {
    Proc::Daemon::Init;
}

my $magic_string_re = $magic_string;
$magic_string_re =~ s/(\W)/\\$1/g;

# convert whitelist into CIDR notation
{
    my(@whitelist_cidr);

    foreach my $IP (@ip_whitelist) {
	if (not eval {@whitelist_cidr = 
			  Net::CIDR::cidradd($IP, @whitelist_cidr)}) {
	    &die("Error processing whitelist: \"$IP\" is not a valid IP ",
		 "address or range.");
	}
    }
    @ip_whitelist = @whitelist_cidr;
}

# open popip database for reading
my %ip_whitelist_db;

&opendb_read if ($ip_whitelist_db);

setlogsock('unix');
openlog($whoami, 'pid', $log_facility);
if (! $debug) {
    # I'd really like to to this, but it doesn't work wit Sys::Syslog
    # 0.13 in Perl 5.8.8.
    # setlogmask(&LOG_UPTO(LOG_INFO));
    eval "
	no warnings 'redefine';
	sub debuglog {
	}
    ";
}
    
if ($database_user) {
    $aliases_file = $sendmail_canon = $sendmail_prog =
	$recipient_cache_expire = $recipient_cache_check_interval = undef;
    syslog("info", "Using shared bogofilter database under %s's account",
	   $database_user);
}

if (! (open(PIDFILE, '+<', $pid_file) ||
       open(PIDFILE, '+>', $pid_file))) {
    &die("open($pid_file): $!\n");
}

seek(PIDFILE, 0, SEEK_SET);

if (! flock(PIDFILE, LOCK_EX|LOCK_NB)) {
    &die("flock($pid_file): $!\n");
}
if (! (print(PIDFILE "$$\n"))) {
    &die("writing to $pid_file: $!\n");
}
# Flush the PID
seek(PIDFILE, 0, SEEK_SET);

unlink($socket);
Sendmail::Milter::setconn("local:$socket");
Sendmail::Milter::register("bogofilter-milter",
			   \%my_milter_callbacks, SMFI_CURR_ACTS);

Sendmail::Milter::main($milter_interpreters);

&closedb;

sub my_connect_callback {
    my $ctx = shift;		# milter context object
    my $hostname = shift;       # The connection's host name.
    my $sockaddr_in = shift;    # AF_INET portion of the host address,
				# from getpeername(2) syscall
    my $hash = $ctx->getpriv();

    my ($port, $ipaddr) = Socket::unpack_sockaddr_in($sockaddr_in) or
	&die("Could not unpack socket address: $!");
    $ipaddr = Socket::inet_ntoa($ipaddr); # translates into standard IPv4 addr

    &debuglog("my_connect_callback: entering with hostname=$hostname, ",
	      "ipaddr=$ipaddr, port=$port");

    # check if the connecting server is listed in the whitelist
    if (@ip_whitelist) {
        if (eval {Net::CIDR::cidrlookup($ipaddr, @ip_whitelist)}) {
          syslog('info', '%s', "$ipaddr is whitelisted, so this email is " .
		 "being accepted unfiltered.");
          $ctx -> setpriv(undef);
          return SMFIS_ACCEPT;
        }
        else {
	    &debuglog("$ipaddr is not in the whitelist");
	}
    }

    # check if connecting server is listed in the popip database
    if ($ip_whitelist_db) {
	if ($ip_whitelist_db{$ipaddr}) {
	    syslog('info', '%s', "$ipaddr is authenticated via poprelayd, " .
		   "so this email is being accepted unfiltered.");
	    $ctx -> setpriv(undef);
	    return SMFIS_ACCEPT;
	}
	else {
	    &debuglog("$ipaddr is not in the popip database");
	}
    }

    $hash->{'ipaddr'} = $ipaddr;
    $ctx->setpriv($hash);
    &debuglog("my_connect_callback: return CONTINUE with hash");
    return SMFIS_CONTINUE;
}

sub my_rcpt_callback {
    my $ctx = shift;
    my $envrcpt = shift;
    my $hash = $ctx->getpriv();

    &debuglog("my_rcpt_callback: entering with " . Data::Dumper->Dump([$hash], [qw(hash)]));

    if ($hash->{'rcpt'}) {
	# We've already encountered a recipient who is filtering this message.
	$ctx->setpriv($hash);
	&debuglog("my_rcpt_callback: return CONTINUE with old hash");
	return SMFIS_CONTINUE;
    }
    my $rcpt = $ctx->getsymval('{rcpt_addr}');

    &debuglog("my_rcpt_callback: rcpt_addr: $rcpt");

    if (&filtered_dir($rcpt)) {
	$hash->{'rcpt'} = $rcpt;
	$hash->{'envrcpt'} = $envrcpt;
	$ctx->setpriv($hash);
	&debuglog("my_rcpt_callback: return CONTINUE with hash");
	return SMFIS_CONTINUE;
    }
    else {
	$ctx->setpriv(undef);
	&debuglog("my_rcpt_callback: return CONTINUE with undef");
	return SMFIS_CONTINUE;
    }
}

sub my_header_callback {
    my($ctx, $field, $value) = @_;
    my($hash) = $ctx->getpriv();

    &debuglog("my_header_callback: entering with " . Data::Dumper->Dump([$hash, $field, $value], [qw(hash field value)]));

    if (! $hash) {
	&debuglog("my_header_callback: return ACCEPT with no hash");
	return SMFIS_ACCEPT;
    }

    if (($field =~ /^subject$/i) && ($value =~ /$magic_string_re/oi)) {
	$ctx->setpriv(undef);
	&debuglog("my_header_callback: returning ACCEPT for magic subject");
	return SMFIS_ACCEPT;
    }

    $hash = &add_to_message($hash, "$field: $value\n");

    $ctx->setpriv($hash);

    &debuglog("my_header_callback: returning CONTINUE with hash");
    return SMFIS_CONTINUE;
}

sub my_eoh_callback {
    my($ctx) = @_;
    my($hash) = $ctx->getpriv();

    # If $hash is undefined here, it means that the sender sent no
    # message header at all, so the block of code in
    # my_header_callback for checking if $hash is undefined never got
    # called.  This means the message is almost certainly spam, but
    # it's not our job to determine that if none of the recipients are
    # using bogofilter.
    if (! $hash) {
	&debuglog("my_eoh_callback: return ACCEPT with no hash (message had empty header)");
	return SMFIS_ACCEPT;
    }


    &debuglog("my_eoh_callback: entering with " . Data::Dumper->Dump([$hash], [qw(hash)]));

    $hash = &add_to_message($hash, "\n");

    $ctx->setpriv($hash);

    &debuglog("my_eoh_callback: returning CONTINUE with hash");
    return SMFIS_CONTINUE;
}

sub my_body_callback {
    my($ctx, $body, $len) = @_;
    my($hash) = $ctx->getpriv();

    &debuglog("my_body_callback: entering with " . Data::Dumper->Dump([$hash, $len], [qw(hash len)]));

    $hash = &add_to_message($hash, $body);

    $ctx->setpriv($hash);

    &debuglog("my_body_callback: returning CONTINUE with hash");
    return SMFIS_CONTINUE;
}

sub add_to_message {
    my($hash, $text) = @_;
    return $hash if (! $text);

    if (! $hash->{'fh'}) {
	$hash->{'msg'} = '' if (! $hash->{'msg'});
	$hash->{'msg'} .= $text;

	if (length($hash->{'msg'}) <= $MAX_INCORE_MSG_LENGTH) {
	    return $hash;
	}

	($hash->{'fh'}, $hash->{'fn'}) = tempfile();

	if (! $hash->{'fn'}) {
	    &die("error creating temporary file");
	}

	&debuglog("switching to temporary file " . $hash->{'fn'});

	$text = $hash->{'msg'};
	delete $hash->{'msg'};
    }

    if (! print({$hash->{'fh'} } $text)) {
	&die("error writing to temporary file " . $hash->{'fn'});
    }

    return $hash;
}

sub message_read_handle {
    my($hash) = @_;

    if ($hash->{'fn'}) {
	if (! seek($hash->{'fh'}, 0, SEEK_SET)) {
	    &die("couldn't seek in " . $hash->{'fn'} . ": $!");
	}
	return $hash->{'fh'};
    }
    else {
	return new IO::Scalar \$hash->{'msg'};
    }
}

    
sub my_eom_callback {
    my $ctx = shift;
    my $hash = $ctx->getpriv();
    my $fh;
    local($_);

    &debuglog("my_eom_callback: entering with " . Data::Dumper->Dump([$hash], [qw(hash)]));

    my $dir = &filtered_dir($hash->{'rcpt'});

    if (! $dir) {
	syslog('err', '%s', "my_eom_callback called for non-filtered recipient; " . Data::Dumper->Dump([$hash], [qw(hash)]));
	$ctx->setpriv(undef);
	&debuglog("my_eom_callback: returning ACCEPT with undef");
	return SMFIS_ACCEPT;
    }

    my $pid = open(BOGOFILTER, '|-');
    if (! defined($pid)) {
	&die("opening bogofilter: $!\n");
    }
    elsif (! $pid) {
	&die("couldn't restrict permissions") if
	    (! &restrict_permissions($hash->{'rcpt'}, 1));;
	my(@cmd) = ('bogofilter', '-u', '-d', $dir);
	if ($bogofilter_cf && -f "$dir/$bogofilter_cf") {
	    push(@cmd, '-c', "$dir/$bogofilter_cf");
	}
	exec(@cmd) || &die("exec(bogofilter): $!\n");
	# &die had better not return!
    }

    $fh = &message_read_handle($hash);
    if ($hash->{'fn'}) {
	# This is safe to do on Unix, since on Unix you can unlink an
	# open file and it'll stay around until the last open file
	# handle to it goes away.  If this script were to be used on
	# non-Unix operating systems, which is a big "if" that I'm not
	# sure could ever happen, then this unlink might be a problem
	# and would need to happen later.
	unlink $hash->{'fn'};
    }

    while (<$fh>) {
	s/\r\n$/\n/;
	print(BOGOFILTER $_) || &die("writing to bogofilter: $!\n");
    }

    if (close(BOGOFILTER)) {
	my($training);
	if ($training_file) {
	    if (&restrict_permissions($hash->{'rcpt'})) {
		$training = (-f "$dir/$training_file");
		&unrestrict_permissions;
	    }
	    else {
		syslog('warning', 'assuming training mode because ' .
		       'permissions could not be restricted');
		$training = 1;
	    }
	}
	$ctx->addheader('X-Bogosity', 'Spam, tests=bogofilter');
	my $from = $ctx->getsymval('{mail_addr}');
	my $which = &reject_or_discard($hash);
	my($verb) = ($which == SMFIS_REJECT) ? "reject" : "discard";
	syslog('info', '%s', ($training ? "would $verb" : "${verb}ing") . 
	       " likely spam from $from to " . $hash->{'rcpt'} . " based on $dir");
	if (! $training) {
	    my($archive, $link);

	    $archive = ($archive_mbox &&
			&restrict_permissions($hash->{'rcpt'}) &&
			(lstat($archive = "$dir/$archive_mbox"))) ?
			$archive : undef;

	    if ($cyrus_deliver && -f $cyrus_deliver && -X $cyrus_deliver &&
		-l $archive && ($link = readlink($archive)) &&
		$link =~ s/^cyrus:// && (! -f $archive)) {
		&unrestrict_permissions;
		my $user = &filtered_user($hash->{'rcpt'});
		if (! $user) {
		    &die("Couldn't determine username for IMAP delivery");
		}
		if (! seek($fh, 0, SEEK_SET)) {
		    &die("error rewinding message handle: $!");
		}
		my $pid = open(DELIVER, "|-");
		if (! defined($pid)) {
		    &die("Error forking to execute $cyrus_deliver: $!");
		}
		elsif (! $pid) {
		    exec($cyrus_deliver, '-a', $user, '-m',
			 "user.$user.$link") ||
			     &die("exec($cyrus_deliver): $!");
		}
		else {
		    local($/) = undef;
		    my $ret = 1;
		    $ret = $ret && print(DELIVER <$fh>);
		    $ret = $ret && close(DELIVER);
		    if (! $ret) {
			syslog('warning', '%s',
			       "$cyrus_deliver failed for user.$user.$link");
		    }
		    goto permissions_already_unrestricted;
		}
	    }
	    if ($archive) {
		# There is an annoying race condition here.  Suppose two spam
		# messages are delivered at the same time to a user whose
		# archive file is a symlink pointing at a nonexistent (yet)
		# file.  Milter process A tries to open with +< and fails.  IN
		# the meantime, process B also tries to open with +< and fails.
		# Then A opens witn +>, locks the file and starts writing to
		# it, and *then* B opens with +>, thus truncating whatever data
		# was written thus far by A.  I'm not sure what the best way is
		# to fix this race condition reliably, and it seems rare enough
		# that it isn't worth the effort.
		if (! (open(MBOX, '+<', $archive) ||
		       open(MBOX, '+>', $archive))) {
		    syslog('warning', '%s', "opening $archive for " .
			   "write: $!");
		    goto no_archive_open;
		}
		if (! flock(MBOX, LOCK_EX)) {
		    syslog('warning', '%s', "locking $archive: $!");
		    goto close_archive;
		}
		if (! seek(MBOX, 0, SEEK_END)) {
		    syslog('warning', '%s', 
			   "seek($archive, 0, SEEK_END): $!");
		    goto close_archive;
		}
		if (! seek($fh, 0, SEEK_SET)) {
		    &die("error rewinding message handle: $!");
		}

		if (! print(MBOX "From " . ($from || 'MAILER-DAEMON') .
			    "  " . localtime() . "\n")) {
		    syslog('warning', '%s', "write($archive): $!");
		    goto close_archive;
		}

		my($last_blank, $last_nl);

		while (<$fh>) {
		    s/\r\n/\n/;
		    s/^From />From /;
		    if (! print(MBOX $_)) {
			syslog('warning', '%s', "write($archive): $!");
			goto close_archive;
		    }

		    $last_nl = ($_ =~ /\n/);
		    $last_blank = ($_ eq "\n");
		}

		# Mbox format requires a blank line at the end
		if (! ($last_blank || print(MBOX ($last_nl ? "\n" : "\n\n")))) {
		    syslog('warning', '%s', "write($archive): $!");
		    goto close_archive;
		}

	      close_archive:
		if (! close(MBOX)) {
		    syslog('warning', '%s', "close($archive): $!");
		}
	    }
	  no_archive_open:
	    &unrestrict_permissions;
	  permissions_already_unrestricted:
	    $ctx->setreply($rcode, $xcode, $reject_message);
	    $ctx->setpriv(undef);
	    return $which;
	}
    }
    else {
	$ctx->addheader('X-Bogosity', 'Ham, tests=bogofilter');
    }

    $ctx->setpriv(undef);
    return SMFIS_CONTINUE;
}

sub my_abort_callback {
    my($ctx) = shift;
    my $hash = $ctx->getpriv();

    &debuglog("my_abort_callback: entering with " . Data::Dumper->Dump([$hash], [qw(hash)]));

    if ($hash->{'fn'}) {
	unlink $hash->{'fn'};
    }

    $ctx->setpriv(undef);
    &debuglog("my_abort_callback: returning CONTINUE with undef");
    return SMFIS_CONTINUE;
}

sub my_close_callback {
    my($ctx) = shift;
    my $hash = $ctx->getpriv();

    &debuglog("my_close_callback: entering with " . Data::Dumper->Dump([$hash], [qw(hash)]));

    if ($hash) {
	if ($hash->{'fn'}) {
	    unlink $hash->{'fn'};
	}
    }

    $ctx->setpriv(undef);
    &debuglog("my_close_callback: returning CONTINUE with undef");
    return SMFIS_CONTINUE;
}

sub filtered_dir {
    my($uid, $gid, $dir) = &expand_recipient($_[0]);
    $dir;
}

sub filtered_user {
    my($uid, $gid, $dir, $stamp, $user) = &expand_recipient($_[0]);
    $user;
}

sub restrict_permissions {
    my($rcpt) = shift;
    my($no_going_back) = shift;

    my($uid, $gid, $dir) = &expand_recipient($rcpt);
    if (! (defined($uid) && defined($gid))) {
	syslog('err', '%s', "internal error: couldn't determine UID and GID " .
	       "for $rcpt");
	return undef;
    }
    $EUID = $uid;
    $EGID = $gid;
    if ($no_going_back) {
	# When we're ready to exec an external program, i.e.,
	# bogofilter, we want to set the real UID and GID so that,
	# e.g., bogofilter will look in the correct home directory for
	# .bogofilter.cf.
	$UID = $uid;
	$GID = $gid;
    }
    1;
}

sub unrestrict_permissions {
    $EUID = $UID;
    $EGID = $GID;
}

my $recipient_cache_last_checked;

sub expand_recipient {
    my($rcpt) = @_;
    my($orig, @expanded);
    my $now = time;

    if ($recipient_cache_expire) {
	if (! defined($recipient_cache_last_checked)) {
	    $recipient_cache_last_checked = $now;
	}
	if ($now - $recipient_cache_last_checked >
	    $recipient_cache_check_interval) {
	    my $old = $now - $recipient_cache_expire;
	    my(@keys) = keys %cached_recipients;
	    my(@expired) = grep($cached_recipients{$_}->[3] <= $old,
				keys %cached_recipients);
	    &debuglog('expiring %d entries (out of %d) ' .
		      'from the recipient cache',
		      scalar @expired, scalar @keys);
	    map(delete $cached_recipients{$_}, @expired);
	    $recipient_cache_last_checked = $now;
	}
    }

    if ($database_user) {
	$rcpt = $database_user;
    }

    if (defined($cached_recipients{$rcpt})) {
	return(@{$cached_recipients{$rcpt}});
    }

    $rcpt = &sendmail_canon($orig = $rcpt);

    if ($rcpt =~ /\@/) {
	return(@{$cached_recipients{$orig}} = (undef, undef, undef, $now, undef));
    }

    if ($aliases_file) {
	my $aliases = Mail::Alias::Sendmail->new($aliases_file);
	@expanded = $aliases->expand($rcpt);
    }
    else {
	@expanded = ($rcpt);
    }

    if ((@expanded == 1) && ($expanded[0] eq $rcpt)) {
	my($dir, $pw);
	my $stripped = $rcpt;

	$stripped =~ s/\+.*//;
	$pw = getpwnam($stripped);
	@{$cached_recipients{$orig}} =
	    $pw ? ($pw->uid, $pw->gid, undef, $now, $stripped) :
	    (undef, undef, undef, $now, undef);
	if ($pw && $pw->dir && &restrict_permissions($orig) &&
	    -d ($dir = $pw->dir . "/.bogofilter") &&
	    ! ($bogofilter_cf && $require_cf && ! -f "$dir/$bogofilter_cf")) {
	    $cached_recipients{$orig}->[2] = $dir;
	}
	elsif ($database_user) {
	    syslog("warning", "Shared database user %s is not configured " .
		   "properly for bogofilter", $database_user);
	}
	&unrestrict_permissions;
	return(@{$cached_recipients{$orig}});
    }
    else {
	foreach my $addr (@expanded) {
	    my(@sub);
	    if (@sub = &expand_recipient($addr)) {
		return(@{$cached_recipients{$orig}} = @sub);
	    }
	}
	return(@{$cached_recipients{$orig}} = (undef, undef, undef, $now, undef));
    }
}

sub sendmail_canon {
    return $_[0] if (! $sendmail_canon);

    my($pid, $sendmail_reader, $sendmail_writer, $last);
    local($_);

    $pid = open2($sendmail_reader, $sendmail_writer, $sendmail_prog, '-bt') or &die("open2 for sendmail failed");
    print($sendmail_writer "3,0 $_[0]\n");
    close($sendmail_writer);
    while (<$sendmail_reader>) {
	# CHECKTHIS You should run "sendmail -bt" as root, give it the
	# input "3,0 addr" where "addr" is one of the addresses in
	# your virtual user table, and confirm that the last
	# "returns:" line that it returns matches the regexp here for
	# local addresses.
	if (/\s+returns: \$\# local \$\:\s+(.+)/) {
	    $last = $1;
	}
    }
    close($sendmail_reader);
    waitpid $pid, 0;

    if ($last) {
	return $last;
    }
    else {
	return $_[0];
    }
}

sub opendb_read {
    tie(%ip_whitelist_db, "DB_File", $ip_whitelist_db, O_RDONLY, 0, $DB_HASH) or &die("Can't open $ip_whitelist_db: $!");
}

sub closedb {
    untie %ip_whitelist_db;
}

sub die {
    my(@msg) = @_;

    &closedb;
    syslog('err', '%s', "@msg");
    exit(1);
}

sub debuglog {
    syslog('debug', "DEBUG: " . join("", @_));
}

my(%mx_cache);

sub reject_or_discard {
    my($hash) = @_;
    my $hostname;

    foreach my $i (0..@discard_control-1) {
	my($pattern, $action) = @{$discard_control[$i]};
	my $ret;
	if ($action =~ /^reject$/i) {
	    $ret = SMFIS_REJECT;
	}
	elsif ($action =~ /^discard$/i) {
	    $ret = SMFIS_DISCARD;
	}
	else {
	    &die("Invalid action $action ",
		 "for discard control pttern $pattern\n");
	}
	if ($pattern =~ /^addr:(.*)$/i) {
	    my $addr = $1;
	    &die("Invalid IP address in discard control pattern $pattern\n")
		if ($addr !~ /^\d+\.\d+\.\d+\.\d+$/);
	    if ($hash->{'ipaddr'} eq $addr) {
		&debuglog("reject_or_discard: addr match $addr: $action");
		return $ret;
	    }
	}
	elsif ($pattern =~ /^netblock:(.*)$/i) {
	    my $netblock = $1;
	    &die("Invalid netblock in discard control pattern $pattern\n")
		if ($netblock !~ /^\d+\.\d+\.\d+\.\d+\/\d+$/);
	    if (Net::CIDR::cidrlookup($hash->{'ipaddr'}, $netblock)) {
		&debuglog("reject_or_discard: netblock match ",
			  "$hash->{ipaddr} in $netblock: $action");
		return $ret;
	    }
	}
	elsif ($pattern =~ /^host:(.*)$/i) {
	    my $match_host = lc $1;
	    $hostname = lc gethostbyaddr(inet_aton($hash->{ipaddr}), AF_INET)
		if (! $hostname);
	    if ($match_host eq $hostname) {
		&debuglog("reject_or_discard: ",
			  "host match $hostname for $hash->{ipaddr}: ",
			  "$action and cache");
		splice(@discard_control, $i, 0,
		       [ "addr:$hash->{ipaddr}", $action ]);
		return $ret;
	    }
	}
	elsif ($pattern =~ /^domain:(.*)$/i) {
	    my $match_domain = lc $1;
	    $hostname = lc gethostbyaddr(inet_aton($hash->{ipaddr}), AF_INET)
		if (! $hostname);
	    if ($match_domain eq $hostname or
		(substr($hostname, -1-length($match_domain)) eq
		 ".$match_domain")) {
		&debuglog("reject_or_discard: domain match ",
			  "$hostname for $hash->{ipaddr} in $match_domain: ",
			  "$action and cache");
		splice(@discard_control, $i, 0,
		       [ "addr:$hash->{ipaddr}", $action ]);
		return $ret;
	    }
	}
	elsif ($pattern =~ /^mx$/i) {
	    my $mx_domain = lc $hash->{'envrcpt'};
	    if (! $mx_domain) {
		&debuglog("reject_or_discard: no envrcpt\n");
		next;
	    }
	    $mx_domain =~ s/.*\@(.*[^\>])\>?/$1/;
	    my %mx_ips;
	    if ($mx_cache{$mx_domain} and
		# refetch MX records once per hour
		time - $mx_cache{$mx_domain}->[0] < 60 * 60) {
		%mx_ips = %{$mx_cache{$mx_domain}->[1]};
	    }
	    else {
		my %mx_ips;
		foreach my $mx (mx($mx_domain)) {
		    my($name, $aliases, $addrtype, $length, @addrs) =
			gethostbyname($mx->exchange);
		    foreach my $addr (@addrs) {
			$mx_ips{inet_ntoa($addr)} = 1;
		    }
		}
		$mx_cache{$mx_domain} = [time, \%mx_ips];
		&debuglog("reject_or_discard: cached MX IPs ",
			  join(" ", sort keys %mx_ips),
			  " for domain $mx_domain");
	    }
	    if ($mx_ips{$hash->{'ipaddr'}}) {
		&debuglog("reject_or_discard: MX addr match ",
			  "$hash->{ipaddr} for domain $mx_domain: $action");
		return $ret;
	    }
	}
	elsif ($pattern eq "*") {
	    return $ret;
	}
	else {
	    &die("Unrecognized discard control pattern: $pattern");
	}
    }
	
    return SMFIS_REJECT;
}
