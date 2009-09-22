#!/usr/bin/perl

require "cjkcompat.pl";

my $revhash=1050;

open (SET, "gunzip -cd <Unihan-3.2.0.txt.gz |") || die "gb2312.txt: $!\n";
while (<SET>)
{
        chomp;
        s/\#.*//;

	next unless /^U\+(....)\s+kIRG_GSource\s+0\-(....)/;

	($code, $unicode)=("0x$2", "0x$1");

        next unless $code ne "";

        eval "\$code=$code;";
        eval "\$unicode=$unicode;";

        die if $code < 0 || $code > 65535;

	$code |= 0x8080;

	$codeh= int($code/256);
	$codel= $code % 256;

	&add($codeh,$codel,$unicode);
}
close SET;

#  Unihan-3.2 does not make mention of GB 2312-80 non-hanzi.
#  So manually add a converting map...
#  cf. ftp://ftp.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/GB/GB2312.TXT 

&add(0xA1,0xA1,0x3000); # IDEOGRAPHIC SPACE
&add(0xA1,0xA2,0x3001); # IDEOGRAPHIC COMMA
&add(0xA1,0xA3,0x3002); # IDEOGRAPHIC FULL STOP
&add(0xA1,0xA4,0x30FB); # KATAKANA MIDDLE DOT
&add(0xA1,0xA5,0x02C9); # MODIFIER LETTER MACRON (Mandarin Chinese first tone)
&add(0xA1,0xA6,0x02C7); # CARON (Mandarin Chinese third tone)
&add(0xA1,0xA7,0x00A8); # DIAERESIS
&add(0xA1,0xA8,0x3003); # DITTO MARK
&add(0xA1,0xA9,0x3005); # IDEOGRAPHIC ITERATION MARK
&add(0xA1,0xAA,0x2015); # HORIZONTAL BAR
&add(0xA1,0xAB,0xFF5E); # FULLWIDTH TILDE
&add(0xA1,0xAC,0x2016); # DOUBLE VERTICAL LINE
&add(0xA1,0xAD,0x2026); # HORIZONTAL ELLIPSIS
&add(0xA1,0xAE,0x2018); # LEFT SINGLE QUOTATION MARK
&add(0xA1,0xAF,0x2019); # RIGHT SINGLE QUOTATION MARK
&add(0xA1,0xB0,0x201C); # LEFT DOUBLE QUOTATION MARK
&add(0xA1,0xB1,0x201D); # RIGHT DOUBLE QUOTATION MARK
&add(0xA1,0xB2,0x3014); # LEFT TORTOISE SHELL BRACKET
&add(0xA1,0xB3,0x3015); # RIGHT TORTOISE SHELL BRACKET
&add(0xA1,0xB4,0x3008); # LEFT ANGLE BRACKET
&add(0xA1,0xB5,0x3009); # RIGHT ANGLE BRACKET
&add(0xA1,0xB6,0x300A); # LEFT DOUBLE ANGLE BRACKET
&add(0xA1,0xB7,0x300B); # RIGHT DOUBLE ANGLE BRACKET
&add(0xA1,0xB8,0x300C); # LEFT CORNER BRACKET
&add(0xA1,0xB9,0x300D); # RIGHT CORNER BRACKET
&add(0xA1,0xBA,0x300E); # LEFT WHITE CORNER BRACKET
&add(0xA1,0xBB,0x300F); # RIGHT WHITE CORNER BRACKET
&add(0xA1,0xBC,0x3016); # LEFT WHITE LENTICULAR BRACKET
&add(0xA1,0xBD,0x3017); # RIGHT WHITE LENTICULAR BRACKET
&add(0xA1,0xBE,0x3010); # LEFT BLACK LENTICULAR BRACKET
&add(0xA1,0xBF,0x3011); # RIGHT BLACK LENTICULAR BRACKET
&add(0xA1,0xC0,0x00B1); # PLUS-MINUS SIGN
&add(0xA1,0xC1,0x00D7); # MULTIPLICATION SIGN
&add(0xA1,0xC2,0x00F7); # DIVISION SIGN
&add(0xA1,0xC3,0x2236); # RATIO
&add(0xA1,0xC4,0x2227); # LOGICAL AND
&add(0xA1,0xC5,0x2228); # LOGICAL OR
&add(0xA1,0xC6,0x2211); # N-ARY SUMMATION
&add(0xA1,0xC7,0x220F); # N-ARY PRODUCT
&add(0xA1,0xC8,0x222A); # UNION
&add(0xA1,0xC9,0x2229); # INTERSECTION
&add(0xA1,0xCA,0x2208); # ELEMENT OF
&add(0xA1,0xCB,0x2237); # PROPORTION
&add(0xA1,0xCC,0x221A); # SQUARE ROOT
&add(0xA1,0xCD,0x22A5); # UP TACK
&add(0xA1,0xCE,0x2225); # PARALLEL TO
&add(0xA1,0xCF,0x2220); # ANGLE
&add(0xA1,0xD0,0x2312); # ARC
&add(0xA1,0xD1,0x2299); # CIRCLED DOT OPERATOR
&add(0xA1,0xD2,0x222B); # INTEGRAL
&add(0xA1,0xD3,0x222E); # CONTOUR INTEGRAL
&add(0xA1,0xD4,0x2261); # IDENTICAL TO
&add(0xA1,0xD5,0x224C); # ALL EQUAL TO
&add(0xA1,0xD6,0x2248); # ALMOST EQUAL TO
&add(0xA1,0xD7,0x223D); # REVERSED TILDE
&add(0xA1,0xD8,0x221D); # PROPORTIONAL TO
&add(0xA1,0xD9,0x2260); # NOT EQUAL TO
&add(0xA1,0xDA,0x226E); # NOT LESS-THAN
&add(0xA1,0xDB,0x226F); # NOT GREATER-THAN
&add(0xA1,0xDC,0x2264); # LESS-THAN OR EQUAL TO
&add(0xA1,0xDD,0x2265); # GREATER-THAN OR EQUAL TO
&add(0xA1,0xDE,0x221E); # INFINITY
&add(0xA1,0xDF,0x2235); # BECAUSE
&add(0xA1,0xE0,0x2234); # THEREFORE
&add(0xA1,0xE1,0x2642); # MALE SIGN
&add(0xA1,0xE2,0x2640); # FEMALE SIGN
&add(0xA1,0xE3,0x00B0); # DEGREE SIGN
&add(0xA1,0xE4,0x2032); # PRIME
&add(0xA1,0xE5,0x2033); # DOUBLE PRIME
&add(0xA1,0xE6,0x2103); # DEGREE CELSIUS
&add(0xA1,0xE7,0xFF04); # FULLWIDTH DOLLAR SIGN
&add(0xA1,0xE8,0x00A4); # CURRENCY SIGN
&add(0xA1,0xE9,0xFFE0); # FULLWIDTH CENT SIGN
&add(0xA1,0xEA,0xFFE1); # FULLWIDTH POUND SIGN
&add(0xA1,0xEB,0x2030); # PER MILLE SIGN
&add(0xA1,0xEC,0x00A7); # SECTION SIGN
&add(0xA1,0xED,0x2116); # NUMERO SIGN
&add(0xA1,0xEE,0x2606); # WHITE STAR
&add(0xA1,0xEF,0x2605); # BLACK STAR
&add(0xA1,0xF0,0x25CB); # WHITE CIRCLE
&add(0xA1,0xF1,0x25CF); # BLACK CIRCLE
&add(0xA1,0xF2,0x25CE); # BULLSEYE
&add(0xA1,0xF3,0x25C7); # WHITE DIAMOND
&add(0xA1,0xF4,0x25C6); # BLACK DIAMOND
&add(0xA1,0xF5,0x25A1); # WHITE SQUARE
&add(0xA1,0xF6,0x25A0); # BLACK SQUARE
&add(0xA1,0xF7,0x25B3); # WHITE UP-POINTING TRIANGLE
&add(0xA1,0xF8,0x25B2); # BLACK UP-POINTING TRIANGLE
&add(0xA1,0xF9,0x203B); # REFERENCE MARK
&add(0xA1,0xFA,0x2192); # RIGHTWARDS ARROW
&add(0xA1,0xFB,0x2190); # LEFTWARDS ARROW
&add(0xA1,0xFC,0x2191); # UPWARDS ARROW
&add(0xA1,0xFD,0x2193); # DOWNWARDS ARROW
&add(0xA1,0xFE,0x3013); # GETA MARK
# DIGIT/NUMBER FULL STOP
foreach ((0xB1..0xC4)) {
	&add(0xA2,$_,0x2488+$_-0xB1);
}
# PARENTHESIZED DIGIT/NUMBER
foreach ((0xC5..0xD8)) {
	&add(0xA2,$_,0x2474+$_-0xC5);
}
# CIRCLED DIGIT/NUMBER
foreach ((0xD9..0xE2)) {
	&add(0xA2,$_,0x2460+$_-0xD9);
}
# PARENTHESIZED IDEOGRAPH
foreach ((0xE5..0xEE)) {
	&add(0xA2,$_,0x3220+$_-0xE5);
}
# ROMAN NUMERAL
foreach ((0xF1..0xFC)) {
	&add(0xA2,$_,0x2160+$_-0xF1);
}
# Fullwidth forms of BASIC LATIN
foreach ((0xA1..0xA3,0xA5..0xFD)) {
	&add(0xA3,$_,0xFF01+$_-0xA1);
}
&add(0xA3,0xA4,0xFFE5); # FULLWIDTH YEN SIGN
&add(0xA3,0xFE,0xFFE3); # FULLWIDTH MACRON
# HIRAGANA
foreach ((0xA1..0xF3)) {
	&add(0xA4,$_,0x3041+$_-0xA1);
}
# KATAKANA
foreach ((0xA1..0xF6)) {
	&add(0xA5,$_,0x30A1+$_-0xA1);
}
&add(0xA6,0xA1,0x0391); # GREEK CAPITAL LETTER ALPHA
&add(0xA6,0xA2,0x0392); # GREEK CAPITAL LETTER BETA
&add(0xA6,0xA3,0x0393); # GREEK CAPITAL LETTER GAMMA
&add(0xA6,0xA4,0x0394); # GREEK CAPITAL LETTER DELTA
&add(0xA6,0xA5,0x0395); # GREEK CAPITAL LETTER EPSILON
&add(0xA6,0xA6,0x0396); # GREEK CAPITAL LETTER ZETA
&add(0xA6,0xA7,0x0397); # GREEK CAPITAL LETTER ETA
&add(0xA6,0xA8,0x0398); # GREEK CAPITAL LETTER THETA
&add(0xA6,0xA9,0x0399); # GREEK CAPITAL LETTER IOTA
&add(0xA6,0xAA,0x039A); # GREEK CAPITAL LETTER KAPPA
&add(0xA6,0xAB,0x039B); # GREEK CAPITAL LETTER LAMDA
&add(0xA6,0xAC,0x039C); # GREEK CAPITAL LETTER MU
&add(0xA6,0xAD,0x039D); # GREEK CAPITAL LETTER NU
&add(0xA6,0xAE,0x039E); # GREEK CAPITAL LETTER XI
&add(0xA6,0xAF,0x039F); # GREEK CAPITAL LETTER OMICRON
&add(0xA6,0xB0,0x03A0); # GREEK CAPITAL LETTER PI
&add(0xA6,0xB1,0x03A1); # GREEK CAPITAL LETTER RHO
&add(0xA6,0xB2,0x03A3); # GREEK CAPITAL LETTER SIGMA
&add(0xA6,0xB3,0x03A4); # GREEK CAPITAL LETTER TAU
&add(0xA6,0xB4,0x03A5); # GREEK CAPITAL LETTER UPSILON
&add(0xA6,0xB5,0x03A6); # GREEK CAPITAL LETTER PHI
&add(0xA6,0xB6,0x03A7); # GREEK CAPITAL LETTER CHI
&add(0xA6,0xB7,0x03A8); # GREEK CAPITAL LETTER PSI
&add(0xA6,0xB8,0x03A9); # GREEK CAPITAL LETTER OMEGA
&add(0xA6,0xC1,0x03B1); # GREEK SMALL LETTER ALPHA
&add(0xA6,0xC2,0x03B2); # GREEK SMALL LETTER BETA
&add(0xA6,0xC3,0x03B3); # GREEK SMALL LETTER GAMMA
&add(0xA6,0xC4,0x03B4); # GREEK SMALL LETTER DELTA
&add(0xA6,0xC5,0x03B5); # GREEK SMALL LETTER EPSILON
&add(0xA6,0xC6,0x03B6); # GREEK SMALL LETTER ZETA
&add(0xA6,0xC7,0x03B7); # GREEK SMALL LETTER ETA
&add(0xA6,0xC8,0x03B8); # GREEK SMALL LETTER THETA
&add(0xA6,0xC9,0x03B9); # GREEK SMALL LETTER IOTA
&add(0xA6,0xCA,0x03BA); # GREEK SMALL LETTER KAPPA
&add(0xA6,0xCB,0x03BB); # GREEK SMALL LETTER LAMDA
&add(0xA6,0xCC,0x03BC); # GREEK SMALL LETTER MU
&add(0xA6,0xCD,0x03BD); # GREEK SMALL LETTER NU
&add(0xA6,0xCE,0x03BE); # GREEK SMALL LETTER XI
&add(0xA6,0xCF,0x03BF); # GREEK SMALL LETTER OMICRON
&add(0xA6,0xD0,0x03C0); # GREEK SMALL LETTER PI
&add(0xA6,0xD1,0x03C1); # GREEK SMALL LETTER RHO
&add(0xA6,0xD2,0x03C3); # GREEK SMALL LETTER SIGMA
&add(0xA6,0xD3,0x03C4); # GREEK SMALL LETTER TAU
&add(0xA6,0xD4,0x03C5); # GREEK SMALL LETTER UPSILON
&add(0xA6,0xD5,0x03C6); # GREEK SMALL LETTER PHI
&add(0xA6,0xD6,0x03C7); # GREEK SMALL LETTER CHI
&add(0xA6,0xD7,0x03C8); # GREEK SMALL LETTER PSI
&add(0xA6,0xD8,0x03C9); # GREEK SMALL LETTER OMEGA
&add(0xA7,0xA1,0x0410); # CYRILLIC CAPITAL LETTER A
&add(0xA7,0xA2,0x0411); # CYRILLIC CAPITAL LETTER BE
&add(0xA7,0xA3,0x0412); # CYRILLIC CAPITAL LETTER VE
&add(0xA7,0xA4,0x0413); # CYRILLIC CAPITAL LETTER GHE
&add(0xA7,0xA5,0x0414); # CYRILLIC CAPITAL LETTER DE
&add(0xA7,0xA6,0x0415); # CYRILLIC CAPITAL LETTER IE
&add(0xA7,0xA7,0x0401); # CYRILLIC CAPITAL LETTER IO
&add(0xA7,0xA8,0x0416); # CYRILLIC CAPITAL LETTER ZHE
&add(0xA7,0xA9,0x0417); # CYRILLIC CAPITAL LETTER ZE
&add(0xA7,0xAA,0x0418); # CYRILLIC CAPITAL LETTER I
&add(0xA7,0xAB,0x0419); # CYRILLIC CAPITAL LETTER SHORT I
&add(0xA7,0xAC,0x041A); # CYRILLIC CAPITAL LETTER KA
&add(0xA7,0xAD,0x041B); # CYRILLIC CAPITAL LETTER EL
&add(0xA7,0xAE,0x041C); # CYRILLIC CAPITAL LETTER EM
&add(0xA7,0xAF,0x041D); # CYRILLIC CAPITAL LETTER EN
&add(0xA7,0xB0,0x041E); # CYRILLIC CAPITAL LETTER O
&add(0xA7,0xB1,0x041F); # CYRILLIC CAPITAL LETTER PE
&add(0xA7,0xB2,0x0420); # CYRILLIC CAPITAL LETTER ER
&add(0xA7,0xB3,0x0421); # CYRILLIC CAPITAL LETTER ES
&add(0xA7,0xB4,0x0422); # CYRILLIC CAPITAL LETTER TE
&add(0xA7,0xB5,0x0423); # CYRILLIC CAPITAL LETTER U
&add(0xA7,0xB6,0x0424); # CYRILLIC CAPITAL LETTER EF
&add(0xA7,0xB7,0x0425); # CYRILLIC CAPITAL LETTER HA
&add(0xA7,0xB8,0x0426); # CYRILLIC CAPITAL LETTER TSE
&add(0xA7,0xB9,0x0427); # CYRILLIC CAPITAL LETTER CHE
&add(0xA7,0xBA,0x0428); # CYRILLIC CAPITAL LETTER SHA
&add(0xA7,0xBB,0x0429); # CYRILLIC CAPITAL LETTER SHCHA
&add(0xA7,0xBC,0x042A); # CYRILLIC CAPITAL LETTER HARD SIGN
&add(0xA7,0xBD,0x042B); # CYRILLIC CAPITAL LETTER YERU
&add(0xA7,0xBE,0x042C); # CYRILLIC CAPITAL LETTER SOFT SIGN
&add(0xA7,0xBF,0x042D); # CYRILLIC CAPITAL LETTER E
&add(0xA7,0xC0,0x042E); # CYRILLIC CAPITAL LETTER YU
&add(0xA7,0xC1,0x042F); # CYRILLIC CAPITAL LETTER YA
&add(0xA7,0xD1,0x0430); # CYRILLIC SMALL LETTER A
&add(0xA7,0xD2,0x0431); # CYRILLIC SMALL LETTER BE
&add(0xA7,0xD3,0x0432); # CYRILLIC SMALL LETTER VE
&add(0xA7,0xD4,0x0433); # CYRILLIC SMALL LETTER GHE
&add(0xA7,0xD5,0x0434); # CYRILLIC SMALL LETTER DE
&add(0xA7,0xD6,0x0435); # CYRILLIC SMALL LETTER IE
&add(0xA7,0xD7,0x0451); # CYRILLIC SMALL LETTER IO
&add(0xA7,0xD8,0x0436); # CYRILLIC SMALL LETTER ZHE
&add(0xA7,0xD9,0x0437); # CYRILLIC SMALL LETTER ZE
&add(0xA7,0xDA,0x0438); # CYRILLIC SMALL LETTER I
&add(0xA7,0xDB,0x0439); # CYRILLIC SMALL LETTER SHORT I
&add(0xA7,0xDC,0x043A); # CYRILLIC SMALL LETTER KA
&add(0xA7,0xDD,0x043B); # CYRILLIC SMALL LETTER EL
&add(0xA7,0xDE,0x043C); # CYRILLIC SMALL LETTER EM
&add(0xA7,0xDF,0x043D); # CYRILLIC SMALL LETTER EN
&add(0xA7,0xE0,0x043E); # CYRILLIC SMALL LETTER O
&add(0xA7,0xE1,0x043F); # CYRILLIC SMALL LETTER PE
&add(0xA7,0xE2,0x0440); # CYRILLIC SMALL LETTER ER
&add(0xA7,0xE3,0x0441); # CYRILLIC SMALL LETTER ES
&add(0xA7,0xE4,0x0442); # CYRILLIC SMALL LETTER TE
&add(0xA7,0xE5,0x0443); # CYRILLIC SMALL LETTER U
&add(0xA7,0xE6,0x0444); # CYRILLIC SMALL LETTER EF
&add(0xA7,0xE7,0x0445); # CYRILLIC SMALL LETTER HA
&add(0xA7,0xE8,0x0446); # CYRILLIC SMALL LETTER TSE
&add(0xA7,0xE9,0x0447); # CYRILLIC SMALL LETTER CHE
&add(0xA7,0xEA,0x0448); # CYRILLIC SMALL LETTER SHA
&add(0xA7,0xEB,0x0449); # CYRILLIC SMALL LETTER SHCHA
&add(0xA7,0xEC,0x044A); # CYRILLIC SMALL LETTER HARD SIGN
&add(0xA7,0xED,0x044B); # CYRILLIC SMALL LETTER YERU
&add(0xA7,0xEE,0x044C); # CYRILLIC SMALL LETTER SOFT SIGN
&add(0xA7,0xEF,0x044D); # CYRILLIC SMALL LETTER E
&add(0xA7,0xF0,0x044E); # CYRILLIC SMALL LETTER YU
&add(0xA7,0xF1,0x044F); # CYRILLIC SMALL LETTER YA
&add(0xA8,0xA1,0x0101); # LATIN SMALL LETTER A WITH MACRON
&add(0xA8,0xA2,0x00E1); # LATIN SMALL LETTER A WITH ACUTE
&add(0xA8,0xA3,0x01CE); # LATIN SMALL LETTER A WITH CARON
&add(0xA8,0xA4,0x00E0); # LATIN SMALL LETTER A WITH GRAVE
&add(0xA8,0xA5,0x0113); # LATIN SMALL LETTER E WITH MACRON
&add(0xA8,0xA6,0x00E9); # LATIN SMALL LETTER E WITH ACUTE
&add(0xA8,0xA7,0x011B); # LATIN SMALL LETTER E WITH CARON
&add(0xA8,0xA8,0x00E8); # LATIN SMALL LETTER E WITH GRAVE
&add(0xA8,0xA9,0x012B); # LATIN SMALL LETTER I WITH MACRON
&add(0xA8,0xAA,0x00ED); # LATIN SMALL LETTER I WITH ACUTE
&add(0xA8,0xAB,0x01D0); # LATIN SMALL LETTER I WITH CARON
&add(0xA8,0xAC,0x00EC); # LATIN SMALL LETTER I WITH GRAVE
&add(0xA8,0xAD,0x014D); # LATIN SMALL LETTER O WITH MACRON
&add(0xA8,0xAE,0x00F3); # LATIN SMALL LETTER O WITH ACUTE
&add(0xA8,0xAF,0x01D2); # LATIN SMALL LETTER O WITH CARON
&add(0xA8,0xB0,0x00F2); # LATIN SMALL LETTER O WITH GRAVE
&add(0xA8,0xB1,0x016B); # LATIN SMALL LETTER U WITH MACRON
&add(0xA8,0xB2,0x00FA); # LATIN SMALL LETTER U WITH ACUTE
&add(0xA8,0xB3,0x01D4); # LATIN SMALL LETTER U WITH CARON
&add(0xA8,0xB4,0x00F9); # LATIN SMALL LETTER U WITH GRAVE
&add(0xA8,0xB5,0x01D6); # LATIN SMALL LETTER U WITH DIAERESIS AND MACRON
&add(0xA8,0xB6,0x01D8); # LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE
&add(0xA8,0xB7,0x01DA); # LATIN SMALL LETTER U WITH DIAERESIS AND CARON
&add(0xA8,0xB8,0x01DC); # LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE
&add(0xA8,0xB9,0x00FC); # LATIN SMALL LETTER U WITH DIAERESIS
&add(0xA8,0xBA,0x00EA); # LATIN SMALL LETTER E WITH CIRCUMFLEX
# BOPOMOFO
foreach ((0xC5..0xE9)) {
	&add(0xA8,$_,0x3105+$_-0xC5);
}
# BOX DRAWINGS
foreach ((0xA4..0xEF)) {
	&add(0xA9,$_,0x2500+$_-0xA4);
}

sub add {
local($codeh,$codel,$unicode) = @_;

	my $code = $codeh*256+$codel;
	my $unicodehash= int($unicode % $revhash);

	die if $codeh < 0xA1 || $codeh > 0xF7;
	die if $codel < 0xA1 || $codel > 0xFE;

	if (! defined $fwd{$codeh})
	{
	    my %dummy;

	    $fwd{$codeh}= \%dummy;
	}

	$fwd{$codeh}{$codel}=$unicode;

	if (! defined $rev[$unicodehash])
	{
	    my @dummy;

	    $rev[$unicodehash]= \@dummy;
	}

	my $r=$rev[$unicodehash];

	push @$r, "$unicode $code";

	$revmap{$unicode} = $code;
}

# Add maps for CJK compatibility ideographs of Unicode.
&add_cjkcompat(%compat_ksx1001);
&add_cjkcompat(%compat_big5);
&add_cjkcompat(%compat_ibm32);
&add_cjkcompat(%compat_jisx0213);
&add_cjkcompat(%compat_cns11643);

sub add_cjkcompat {
local(%compat) = @_;
	foreach (keys %compat) {
		if (defined $revmap{$compat{$_}}) {
			my $unicodehash = int($_ % $revhash);
			if (! defined $rev[$unicodehash])
			{
				my @dummy;
				$rev[$unicodehash]= \@dummy;
			}
			my $r=$rev[$unicodehash];
			push @$r, "$_ $revmap{$compat{$_}}";
		}
	}
}



print '
/*
** Copyright 2000-2001 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: gb2312.pl,v 1.4 2004/02/08 04:59:15 mrsam Exp $
** Non-hanzi support by Hatuka*nezumi - IKEDA Soji <nezumi@jca.apc.org>
*/

#include "unicode.h"
';

foreach (sort keys %fwd)
{
    my $h=$_;
    my $l;

    printf ("static const unicode_char gb2312_%02x[94]={", $h);

    for ($l=0xA1; $l < 0xFF; $l++)
    {
	print "\n" if ($l % 16) == 0;
	printf ("%d", $fwd{$h}{$l});
	print "," unless $l >= 0xFE;
    }
    print "};\n";

}

print "static const unsigned gb2312_revhash_size=$revhash;
static const unicode_char gb2312_revtable_uc[]={\n";

my $index=0;

for ($i=0; $i<$revhash; $i++)
{
    my $a= $rev[$i];

    $revindex[$i]=$index;

    my $v;

    my @aa=@$a;

    while (defined ($v=shift @aa))
    {
	print "," if $index > 0;
	print "\n" if $index && ($index % 16) == 0;

	$v =~ s/ .*//;
	print $v;
	++$index;
    }
}

print "};\nstatic const unsigned gb2312_revtable_octets[]={\n";

$maxl=0;
$index=0;
for ($i=0; $i<$revhash; $i++)
{
    my $a= $rev[$i];

    my $v;

    my @aa=@$a;

    $maxl=$#aa if $#aa > $maxl;
    while (defined ($v=shift @aa))
    {
	print "," if $index > 0;
	print "\n" if $index && ($index % 16) == 0;

	$v =~ s/.* //;
	print $v;
	++$index;
    }
}

print "};\nstatic const unsigned gb2312_revtable_index[]={\n";

for ($i=0; $i<$revhash; $i++)
{
    print "," if $i > 0;
    print "\n" if $i && ($i % 16) == 0;
    print $revindex[$i];
}

print "};\n";
