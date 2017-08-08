#! /usr/bin/perl

# USAGE: perl mkeastasianwidth.pl > charwidth.c

use IO::File;

my $fh=new IO::File "<EastAsianWidth.txt";

my $pb=-1;
my $pe=-1;

print "static const char32_t unicode_wcwidth_tab[][2]={\n";

sub full($$) {
    my $b=hex(shift);
    my $e=hex(shift);

    if ($b == $pe+1)
    {
	$pe=$e;
	return;
    }

    printf ("{0x%04x, 0x%04x},\n", $pb, $pe) unless $pb < 0;

    $pb=$b;
    $pe=$e;
}


while (defined($_=<$fh>))
{
    chomp;
    s/#.*//;

    my @w=split(/;/);

    grep {s/^\s*//; s/\s*$//; } @w;

    next unless $w[1] eq "F" || $w[1] eq "W";

    if ($w[0] =~ /(.*)\.\.(.*)/)
    {
	full($1, $2);
    }
    else
    {
	full($w[0], $w[0]);
    }
}

printf ("{0x%04x, 0x%04x}\n};\n", $pb, $pe);
