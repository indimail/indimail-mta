# Copyright 2000-2004 Double Precision, Inc.
# See COPYING for distribution information.
#
#
# Generate unicode upper/lower/titlecase translations.

print '/*
** Copyright 2000-2004 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include "courier-unicode.h"

';

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
		$FLAG{$code}=1;
	}

	$lc=$fields[13];
	if ($lc ne "")
	{
		eval "\$lc=0x$lc;";
		$LC{$code}=$lc;
		$FLAG{$code}=1;
	}

	$tc=$fields[14];
	if ($tc ne "")
	{
		eval "\$tc=0x$tc;";
		$TC{$code}=$tc;
		$FLAG{$code}=1;
	}
}

close(U);

my $tabsize=2048;

grep ($bucket[ $_ % $tabsize ] .= "$_\n", keys %FLAG);

my $maxcnt=0;

for ($i=0; $i < $tabsize; $i++)
{
    my $cnt=0;

    grep ( ++$cnt, split (/\n/, $bucket[$i]));

    $maxcnt=$cnt if $cnt > $maxcnt;
}

print "const unsigned unicode_case_hash=$tabsize;\n";
print "/* unicode_case_maxbucket="
    . ($maxcnt+2) . "*/\n";

print "const char32_t unicode_case_tab[][4]={\n";

my $idx=0;

for ($i=0; $i<$tabsize; $i++)
{
    $offset[$i]=$idx;

    grep {
	my $j=$_;
	my $u=$UC{$j}+0;
	my $l=$LC{$j}+0;
	my $t=$TC{$j}+0;

	if ($u || $l || $t)
	{
	    $u=$j unless $u;
	    $l=$j unless $l;
	    $t=$u unless $t;

	    printf("{0x%04x,0x%04x,0x%04x,0x%04x},",$j,$u,$l,$t);
	    print "\n" if ($idx % 4) == 3;
	    ++$idx;
	}
    } split(/\n/, $bucket[$i]);
}
print "{0,0,0,0}};

const unsigned unicode_case_offset[$tabsize]={
";

for ($i=0; $i<$tabsize;$i++)
{
    printf("%4d", $offset[$i]);
    print "," if $i < $tabsize-1;

    print "\n" if ($i % 16) == 15;
}
print "};\n";
