#!/usr/bin/perl -Tw
require 5.006_001;
use strict;

=head1 NAME

Stripsearch - where the spam's body gets violated...

=cut

my $version = "1.0.5";

################################################
############### Copyleft Notice ################
################################################

# Copyright © 2005 Order amid Chaos, Inc.
# Author: Tom Anderson 
# neo+stripsearch@orderamidchaos.com
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

# use "perldoc stripsearch" or "stripsearch -h" to read this

=head1 SYNOPSIS

=head2 Command line usage:

B<stripsearch> [I<options>] < [I<rfc822_email>]

=head2 Procmail usage (recommended):

Add to ~/.procmailrc the following recipe, where I<$HOME> 
is your home directory, if not set in the environment:

  :0
  {
        :0 fbw
        | $HOME/.bogofilter/stripsearch
  
        # filter through bogofilter, tagging as spam 
	# or not and updating the word lists
        :0 fw
        | bogofilter -uep
  }

=head2 Command line options:

=over 4

=item B<h>

display this help file

=item B<b>

display benchmarking info

=back


=head1 REQUIRES

=over 4

=item *

Perl 5.6.1

=item *

MIME::QuotedPrint

=item *

Benchmark

=back


=head1 DESCRIPTION

Stripsearch investigates the body of your emails for evidence of 
spamvertized URLs by looking them up in a Realtime BlockList (RBL) 
such as surbl.org or spamhaus.org.  Any matching URLs are then
replaced by the token SPAM-ADDRESS as a hook for statistical 
filters, and a link to look up the domain in a list checker
such as the one at rulesemporium.com.  This serves the 
double purpose of making it less likely to click on a phishing 
scam or illegitimate unsubscribe link, and also making it more 
likely that the email will be flagged as spam.  This should 
especially help to classify those spams which consist of just a 
linked image and no text, or a phishing scam posing as a
company with which you have regular correspondance such as, 
perhaps, eBay, PayPal, or your bank.

If a domain is not listed in a URIBL, a further test is
performed on HTML emails.  The HREF link is compared to the 
content enclosed by the tag, and if there is a domain in the
content, and it does not match the domain of the link, then the
token SCAM-ADDRESS is added to indicate that the address shown
is not the address to which the link connects.  There may be
occasions where this is proper, but in most cases, this 
technique is used for phishing or fraud.

Stripsearch does not remove any information from emails.  It
only adds flags to them.  If an address is flagged 
inappropriately, little harm is done.  These tokens alone 
shouldn't cause a ham message to be misclassified by a
statistical filter as there should be many other hammy tokens 
to offset it.  However, it is quite effective at making spams 
that are otherwise right on the border, or having very few 
overall tokens, classify correctly as spam.


=head1 FAQ

=head2 Ask a question

Ye may receive an answer here if it is asked frequently


=head1 BUGS

=over 4

=item *

Please report any.

=back


=head1 TODO

=over 4

=item *

Detect MIME headers and only parse text or html parts

=item *

Suggestions welcome.

=back


=head1 SEE ALSO

=over 4

=item *

L<procmail>

=item *

L<bogofilter>

=back


=head1 AUTHOR

Tom Anderson <neo+stripsearch@orderamidchaos.com>

=cut

#################################################
############### User Variables  #################
#################################################

# please edit according to your setup

# default path
our $path = "/bin:/usr/bin:/usr/local/bin";

# default shell
our $shell = "/bin/sh";

# seconds before we bail waiting on input
our $timeout = 30;

# maximum lines per email to parse
# this may break large attachments so set it high
our $maxlines = 100000;

# server to use for URIBL lookups (separate multiple servers by a comma or semi-colon)
our $uribl_server = "multi.surbl.org;sbl-xbl.spamhaus.org";

# URL to lookup spamvertized URL (use [DOMAIN] for domain holder in lookup URLs)
our $lookup = "http://www.rulesemporium.com/cgi-bin/uribl.cgi?domain0=[DOMAIN]&bl0=0";

# of course, modify the first line of this file,
# the shebang, to point to your perl interpreter

# do not edit below this line unless you really
# know what you're doing

#################################################
############## Include Libraries ################
#################################################

#use MIME::QuotedPrint;
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

# options flags
our $options = "";

# define the control-linefeed syntax for this system
our $CRLF = 
"\n";
#($^O=~/VMS/i)? 	"\n": 		# VMS
#("\t" ne "\011")? 	"\r\n": 	# EBCDIC
#			"\015\012"; 	# others 

our $lastline = "";

################################################
##################### Main #####################
################################################

# process options
if (defined @ARGV && $ARGV[0] =~ /h/) 
{
	my $stripsearch = $1 if $0 =~ /^([\w\/.\-~]*)$/;
	system("perldoc $stripsearch"); exit(0);
}
if (defined @ARGV && $ARGV[0] =~ /b/) { $options .= "b"; }	# output benchmarking info

# start timing the process
my $start_time = new Benchmark if $options =~ /b/;

# get STDIN and process the email
eval 
{
	# set an alarm so that we don't hang on an empty STDIN
	local $SIG{ALRM} = sub { die "timeout" };
	alarm $timeout;

	# do the search
	my $body = disrobe();
	
	# cancel timeout if we got this far
	alarm 0;

	print $body;
};

# propagate errors
die if $@ && $@ !~ /timeout/i;

# print timeout message
if ($@ =~ /timeout/i) { error("die","Timed out... make sure to supply an email for processing.  Try 'stripsearch -h' for details.\n"); }

# calculate total running time
if ($options =~ /b/)
{
	my $end_time = new Benchmark;  
	my $td = timediff($end_time, $start_time);
	my $usr = $td->[1]+$td->[3]; my $sys = $td->[2]+$td->[4];
	my $cpu = $usr+$sys; my $wall = $td->[0];
	print "Total stripsearch running time was $wall wallclock secs; $usr usr + $sys sys = $cpu CPU secs.$CRLF";
}

exit(0);

################################################
############# Search Body for URIs  ############
################################################

sub disrobe
{
	my $body = "";
	my $count = 0;	

        while (<STDIN>) { $body .= $_; last if $count > $maxlines; }

	$body = inspect($body);

	return $body;
}		

################################################
############# Parse Lines for URIs  ############
################################################

sub inspect
{
	my $body = shift;
	
	# heuristics
	my $DOMAIN	= qr~[A-Za-z](?:\w|-|\.)+\.\w{2,4}~is;
	my $IP		= qr~(?:\d{1,3}\.){3}\d{1,3}~is;
	my $LUSER	= qr~(?:\w|-|\.)+?~is;
	my $EMAIL 	= qr~$LUSER\@(?:$DOMAIN|$IP)~is;
	my $LINK	= qr~<\s*a\s+[^>]*href\s*=\s*['"]?~is;
	my $PROTOCOL	= qr~(?:ht|f)tps?:\/\/~is;
	my $DIRS	= qr~\/(?:[^\/\\;:,{}\[\]()'"<>]*?\/)*~is;
	my $FILE	= qr~[^\/\\;:,{}\[\]()<>'"[:space:]]+~is;
	my $ENDTAG	= qr~['" ]?.*?>~is;
	my $ENDURL	= qr~[,.;:<>{}\[\]()[:space:]'"]~is;
	my $CONTENT	= qr~.*?~is;
	my $CLOSETAG	= qr~<\s*\/\s*a\s*>~is;

	# unencode quoted-printable
	#$body = decode_qp($body) if $body =~ /Content-Transfer-Encoding: quoted-printable/i;
	
	# tag HTML links
	$body =~ s/($LINK)($PROTOCOL)?($IP|$DOMAIN)($DIRS)?($FILE)?($ENDTAG)($CONTENT)($CLOSETAG)/tag_uri($1,$2,$3,$4,$5,$6,$7,$8)/egis;

	# tag bare URLs (variable-width look-behinds would be good here)
	$body =~ s/(?<!href=|ref =|ref= |ref=['"]|f = ['"]|ef = |ef =['"]|ef= ['"]|ESS: |....[A-Za-z\/\\?._\-])($PROTOCOL)?($IP|$DOMAIN)($DIRS)?($FILE)?(?=$ENDURL)/tag_uri("",$1,$2,$3,$4)/egis;

	# re-encode quoted-printable
	#$body = encode_qp($body) if $body =~ /Content-Transfer-Encoding: quoted-printable/i;

	return $body;
}

sub tag_uri
{
	my $link = shift || "";
	my $proto = shift || "";
	my $domain = shift || "";
	my $dirs = shift || "";
	my $file = shift || "";
	my $endtag = shift || "";
	my $content = shift || "";
	my $closetag = shift || "";

	# don't tag the lookup URL we added
	return $link.$proto.$domain.$dirs.$file.$endtag.$content.$closetag if $domain eq get_domain($lookup);

	# strip domain to the root
	my $root_domain = $domain;
	$root_domain =~ s/.*?\.?([A-Za-z](?:\w|-)+\.(?:[A-Za-z]{2}|com|edu|gov|net|mil|org|info|(?:com|edu|gov|net|mil|org|ac|co|sch|pro|int|or|gv|priv|id|oz|info|asn|csiro|telememo|conf|otc|belgie|dns|fgov|adm|adv|agr|am|arq|art|ato|bio|bmd|cim|cng|cnt|coop|ecn|eng|esp|etc|eti|far|fm|fnd|fot|fst|g12|ggf|imb|ind|inf|jor|lel|mat|med|mus|nom|not|ntr|odo|ppg|psc|psi|qsl|rec|slg|srv|tmp|trd|tur|tv|vet|zlg|ab|bc|mb|nb|nf|nl|ns|nt|nu|on|pe|qc|sk|yk|ah|bj|cq|gd|gs|gx|gz|hb|he|hi|hk|hl|hn|jl|js|ln|mo|nm|nx|qh|sc|sn|sh|sx|tj|tw|xj|xz|yn|zj|arts|firm|info|int|store|web|ed|fi|go|or|sa|art|ass|pol|k12|fin|pri|fie|eun|sci|biz|name|school|asso|barreau|prd|presse|tm|aeroport|assedic|avocat|avoues|cci|chambagri|chirurgiens-dentistes|experts-comptables|geometre-expert|gouv|greta|huissier-justice|medecin|notaires|pharmacien|port|veterinaire|pvt|sch|ltd|alderney|guernsey|sark|gob|idv|2000|erotika|jogasz|sex|video|agrar|film|konyvelo|shop|bolt|forum|lakas|suli|priv|casino|games|media|szex|sport|city|hotel|news|tozsde|erotica|ingatian|reklam|utazas|muni|idf|lkd\.co|nic|plc\.co|ernet|res|gen|jersey|ltd|ad|ne|gr|ed|lg|re|press|uu|alt|cul|unam|telecom|cri|geek|govt|iwi|mod|museum|sld|ngo|aid|agro|atm|auto|gmina|gsm|mail|miasta|media|pc|nieruchomosci|powiat|realestate|rel|sklep|sos|szkola|targi|tourism|travel|turystyka|fam|gon|gop|gos|plo|sec|www|pp|pub|parti|brand|fh|fhsk|fhv|red|edunet|ens|nat|intl|rnrt|rnu|rns|tourism|bbs|us|uk|ca|eu|es|fr|it|se|dk|be|de|at|au|idv|gove|me|plc|nhs|police|dni|fed|gub|tec|health|ch|bourse|law|ngo|tel|fax|mob|mobil|mobile|tlf)\.[A-Za-z]{2})$)/$1/is;

	# replace "[DOMAIN]" in redirect with actual domain found
	my $domain_lookup = $lookup;
	$domain_lookup =~ s/\[DOMAIN\]/$root_domain/;

	my $br = ($link)? "<br />\n" : "\n";

	# if it's spam, replace the URI with a flag
	return "$br $br SPAM-ADDRESS: $proto$domain$dirs$file $br $domain_lookup $br $br $content " if listed($root_domain) || listed(host($root_domain));

	# if the link and content domains don't match, replace the URI with a flag
	return "$br $br SCAM-ADDRESS: $proto$domain$dirs$file $br $content $br $br" if $content && get_domain($content) && (get_domain($content) !~ /(^|\.)$root_domain$/i);

	# otherwise pass it along normally
	return $link.$proto.$domain.$dirs.$file.$endtag.$content.$closetag;
}

sub listed
{
	my $target = shift;
	my $output = "";
	my $server = $uribl_server;

	# reverse IP order to conform to standard
       	$target = "$4.$3.$2.$1" if $target =~ /(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})/;

	while ( $server =~ s/^([^;,]+)(?:;|,|$)//is && $target && !$output)
	{
		my $lookup = $target . "." . $1;
	
		open (HOST, "host -t a $lookup 2>/dev/null |") or error("warn","Host lookup failed: $!");
		while (<HOST>) { $output = $1 if /has address ([0-9.]+)/; }
		close HOST;
	}

	return $output;
}

sub host
{
	my $target = shift;
	my $output = "";
	
	my $IP		= qr~(?:\d{1,3}\.){3}\d{1,3}~;	
	my $DOMAIN	= qr~[\w|-|\.]+\.\w{2,4}~;

	if ($target =~ s/($IP|$DOMAIN)/$1/)
	{
		open (HOST, "host $target 2>/dev/null |") or error("warn","Host lookup failed: $!");
		while (<HOST>) { $output = $1 if /$DOMAIN (?:domain name pointer|has address) ($IP|$DOMAIN)\.?/; }
		close HOST;
	}
	
	return $output;
}

sub get_domain
{
        my $text = shift;

        # heuristics
        my $DOMAIN      = qr~(?:\w|-|\.)+\.\w{2,4}~;
        my $IP          = qr~(?:\d{1,3}\.){3}\d{1,3}~;
        my $PROTOCOL    = qr~(?:ht|f)tp(?:s?):\/\/~;

        return ($text =~ /(?<!.src=|src =|src= |src=['"]|c = ['"]|rc = |rc =['"]|rc= ['"]|ESS: |....[A-Za-z\/\\?._\-])$PROTOCOL($IP|$DOMAIN)(?=$|[^A-Za-z0-9.\-_])/i)? $1 : "";
}


################################################
################ Error Handling ################
################################################

sub error
{
	my ($action,$msg) = @_;

	die $msg if $action eq "die";
	warn $msg unless $action eq "die";
	# add other actions if you like
}

sub sig_trap
{
	my $sig = shift;
	my ($action,$more) = ("warn","");

	sig: 
	{
        	$action = "die",  last sig if $sig =~ /ALRM/;
	        $action = "warn", last sig if $sig =~ /PIPE/;
	        $action = "warn", last sig if $sig =~ /CHLD/;
        	$action = "die" , last sig if $sig =~ /INT/;
	        $action = "die" , last sig if $sig =~ /HUP/;
        	$action = "warn";
	}
	
	my $waitedpid = wait;
	$more = "; Reaped pid $waitedpid, exited with status " . ($? >> 8) if $waitedpid;

	$SIG{$sig} = \&sig_trap;

	error ($action,"Trapped signal SIG$sig$more");
}

################################################
################################################
################################################
