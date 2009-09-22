# Copyright 2000-2006 Double Precision, Inc.
# See COPYING for distribution information.
#
# $Id: mkiso8859.pl,v 1.7 2006/03/25 14:24:43 mrsam Exp $
#
# Generate iso-8859* unicode tables

my $set=shift;

open (U, "UnicodeData.txt") || die "$!\n";

while (<U>)
{
	chomp;

my @fields= split /;/;

my ($code, $uc, $lc, $tc);

	$code="0x$fields[0]";
	eval "\$code=$code;";

	$uc=$fields[12];
	if ($uc ne "")
	{
		eval "\$uc=0x$uc;";
		$UC{$code}=$uc;
	}

	$lc=$fields[13];
	if ($lc ne "")
	{
		eval "\$lc=0x$lc;";
		$LC{$code}=$lc;
	}

	$tc=$fields[14];
	if ($tc ne "")
	{
		eval "\$tc=0x$tc;";
		$TC{$code}=$tc;
	}
}

close(U);

my @fwd;

my $rev;

open (SET, $set) || die "$set: $!\n";
while (<SET>)
{
	chomp;
	s/\#.*//;

my ($code, $unicode)=split /[ \t]+/;

	next unless $code ne "";

	eval "\$code=$code;";
	eval "\$unicode=$unicode;";

	die if $code < 0 || $code > 255;

	$fwd[$code]=$unicode;
	$rev{$unicode}=$code;
}
close(SET);

my $fwdname=shift;

print '
/*
** Copyright 2000-2006 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: mkiso8859.pl,v 1.7 2006/03/25 14:24:43 mrsam Exp $
*/

#include "unicode.h"
';


print "static const unicode_char $fwdname [128]={\n";

for ($i=0; $i<128; $i++)
{
    die "Unicode map not US-ASCII: $i\n" unless $fwd[$i] == $i;
}

for ($i=128; $i<256; $i++)
{
my $n=$fwd[$i];

	$n=0 unless $n;

	print "$n";
	print "," if $i < 255;
	print "\n" if ($i % 16) == 15;
}

print "};\n";

my $ucname=shift;

print "static const char $ucname [256]={\n";

for ($i=0; $i<256; $i++)
{
my $unicode=$fwd[$i];

	$unicode=$UC{$unicode} && $rev{$UC{$unicode}} ? $rev{$UC{$unicode}}:$i;

	printf("(char)0x%02x", $unicode);
	print "," if $i < 255;
	print "\n" if ($i % 8) == 7;
}

print "};\n";

my $lcname=shift;

print "static const char $lcname [256]={\n";

for ($i=0; $i<256; $i++)
{
my $unicode=$fwd[$i];

	$unicode=$LC{$unicode} && $rev{$LC{$unicode}} ? $rev{$LC{$unicode}}:$i;

	printf("(char)0x%02x", $unicode);
	print "," if $i < 255;
	print "\n" if ($i % 8) == 7;
}

print "};\n";

my $tcname=shift;

print "static const char $tcname [256]={\n";

for ($i=0; $i<256; $i++)
{
my $unicode=$fwd[$i];

	$unicode=$TC{$unicode} && $rev{$TC{$unicode}} ? $rev{$TC{$unicode}}:$i;

	printf("(char)0x%02x", $unicode);
	print "," if $i < 255;
	print "\n" if ($i % 8) == 7;
}

my $structname=shift;
my $chsetname=shift;

print "};


static unicode_char *c2u(const struct unicode_info *u, const char *cp, int *ip)
{
	return (unicode_iso8859_c2u(cp, ip, $fwdname));
}

static char *u2c(const struct unicode_info *u, const unicode_char *cp, int *ip)
{
	return ("
.
($set eq "windows-874.txt" ? "unicode_windows874_u2c":"unicode_iso8859_u2c")
. "(cp, ip, $fwdname));
}

static char *toupper_func(const struct unicode_info *u, const char *cp, int *ip)
{
	return (unicode_iso8859_convert(cp, ip, $ucname));
}

static char *tolower_func(const struct unicode_info *u, const char *cp, int *ip)
{
	return (unicode_iso8859_convert(cp, ip, $lcname));
}

static char *totitle_func(const struct unicode_info *u, const char *cp, int *ip)
{
	return (unicode_iso8859_convert(cp, ip, $tcname));
}

const struct unicode_info $structname = {
	\"$chsetname\"," . ($set =~ /^8859*/ || $set =~ /^windows-12/ ? "
	UNICODE_USASCII |
	UNICODE_HEADER_QUOPRI | UNICODE_BODY_QUOPRI," :
 $set eq "windows-874.txt" ? "
	UNICODE_USASCII |
	UNICODE_HEADER_BASE64 | UNICODE_BODY_BASE64," :"
	UNICODE_USASCII,") . "
	c2u,
	u2c,
	toupper_func,
	tolower_func,
	totitle_func};
";
