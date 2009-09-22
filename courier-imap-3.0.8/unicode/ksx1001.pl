#! /usr/bin/perl

# USAGE: perl ksx1001.pl > ksx1001.h

# Requires CP949.TXT, found on:
#   http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP949.TXT"

require "cjkcompat.pl";

$cp949 = 'CP949.TXT';
$perline = 8;

die "${cp949}: File not found.\n" if (!(-f $cp949));
open (SET, $cp949) or die "${cp949}: $!\n";

while (<SET>) {
	chomp;

	s/\#.*//;

	next unless /^0x([0-9A-F]{2,4})\s+0x([0-9A-F]{4})\s*$/;

	my ($code, $ucode) = (hex($1), hex($2));

	if ($code > 0x8000) {
		print STDERR "Warning: duplicated: $code->$k2u{$code},$ucode\n"
			if defined $k2u{$code};
		$k2u{$code} = $ucode;
	}
}

close SET;

# make reversal map.

foreach (keys %k2u) {
	print STDERR "Warning: duplicated: $u2k{$k2u{$_}},$_<-$k2u{$_}\n"
		if defined $u2k{$k2u{$_}};
	$c1 = $_ >> 8;
	$c2 = $_ & 0x00FF;
	if ($c1 >= 0xA1 && $c2 >= 0xA1) {
		$u2k_ksx1001{$k2u{$_}} = $_;
	} else {
		$u2k_cp949{$k2u{$_}} = $_;
	}
}

# Add maps for CJK compatibility ideographs of Unicode.
####&add_cjkcompat(%compat_ksx1001);
&add_cjkcompat(%compat_big5);
&add_cjkcompat(%compat_ibm32);
&add_cjkcompat(%compat_jisx0213);
&add_cjkcompat(%compat_cns11643);

sub add_cjkcompat {
local(%compat) = @_;
  foreach (keys %compat) {
    if (defined $u2k_ksx1001{$compat{$_}}) {
      $u2k_ksx1001{$_} = $u2k_ksx1001{$compat{$_}};
    }
    if (defined $u2k_cp949{$compat{$_}}) {
      $u2k_cp949{$_} = $u2k_cp949{$compat{$_}};
    }
  }
}


print <<"EOF";
#ifndef _KSX1001_HDR_
#define _KSX1001_HDR_
/*
 * KS X 1001 and CP949 (UHC) support
 *  by Hatuka*nezumi - IKEDA Soji <nezumi\@jca.apc.org>
 * $Id: ksx1001.pl,v 1.1 2004/02/03 02:00:00 mrsam Exp $
 *
 */

#include "unicode.h"

#define KS_CHAR_SO	0x0E
#define KS_CHAR_SI	0x0F
#define KS_CHAR_ESC	0x1B

/* ISOREG #1/#3: US-ASCII (identical to ISO 646 IRV) */
#define KS_STATE_ASCII		0x0
/* ISOREG #149: KS X 1001:1992 Wansung */
#define KS_STATE_KSX1001	0x4
/* Unknown state */
#define KS_STATE_BINARY		0xF

EOF

print "/* map: CP949 to Unicode */\n";
for ($hb = 0x81; $hb <= 0xFE; $hb++) {
	$items = 0;
	for ($lb = 0x41; $lb <= 0xFE; $lb++) {
		$items++ if $k2u{$hb*256 + $lb};
	}
	if ($items) {
		$items = 0;
		printf "static const unicode_char cp949_to_uni_tbl_%02x[] = {", $hb;
		for ($lb = 0x41; $lb <= 0xFE; $lb++) {
			$code = $hb*256 + $lb;
			print ", " if ($items > 0);
			print "\n  " if ($items % $perline == 0);
			$k2u{$code} = 0xFFFD unless $k2u{$code};
			printf("0x%04X", $k2u{$code});
			$items++;
		}
		print "\n};\n";
		$k2uout{$hb} = 1;
	}
}

print "const unicode_char * cp949_to_uni_tbls[] = {\n";
for ($hb=0x81; $hb <= 0xFE; $hb++) {
	print (($hb > 0x81) ? ",\n  " : "  ");
	if ($k2uout{$hb}) {
		printf "cp949_to_uni_tbl_%02x", $hb;
	} else {
		print "NULL";
	}
}

print "\n};\n";
print "\n\n";


%u2kout = ();
print "/* map: Unicode to KS X 1001 */\n";
for ($hb = 0x00; $hb <= 0xFF; $hb++) {
	$items = 0;
	for ($lb = 0x00; $lb <= 0xFF; $lb++) {
		$items++ if $u2k_ksx1001{$hb*256 + $lb};
	}
	if ($items) {
		$items = 0;
		printf "static const unicode_char uni_to_ksx1001_tbl_%02x[] = {", $hb;
		for ($lb = 0x00; $lb <= 0xFF; $lb++) {
			$code = $hb*256 + $lb;
			print ", " if ($items > 0);
			print "\n  " if ($items % $perline == 0);
			$u2k_ksx1001{$code} = 0x003F unless $u2k_ksx1001{$code};
			printf("0x%04X", $u2k_ksx1001{$code});
			$items++;
		}
		print "\n};\n";
		$u2kout{$hb} = 1;
	}
}

print "const unicode_char * uni_to_ksx1001_tbls[] = {\n";
for ($hb=0x00; $hb <= 0xFF; $hb++) {
	print (($hb > 0x00) ? ",\n  " : "  ");
	if ($u2kout{$hb}) {
		printf "uni_to_ksx1001_tbl_%02x", $hb;
	} else {
		print "NULL";
	}
}

print "\n};\n";
print "\n\n";

%u2kout = ();
print "/* map: Unicode to CP949 extension */\n";
for ($hb = 0x00; $hb <= 0xFF; $hb++) {
	$items = 0;
	for ($lb = 0x00; $lb <= 0xFF; $lb++) {
		$items++ if $u2k_cp949{$hb*256 + $lb};
	}
	if ($items) {
		$items = 0;
		printf "static const unicode_char uni_to_cp949_tbl_%02x[] = {", $hb;
		for ($lb = 0x00; $lb <= 0xFF; $lb++) {
			$code = $hb*256 + $lb;
			print ", " if ($items > 0);
			print "\n  " if ($items % $perline == 0);
			$u2k_cp949{$code} = 0x003F unless $u2k_cp949{$code};
			printf("0x%04X", $u2k_cp949{$code});
			$items++;
		}
		print "\n};\n";
		$u2kout{$hb} = 1;
	}
}

print "const unicode_char * uni_to_cp949_tbls[] = {\n";
for ($hb=0x00; $hb <= 0xFF; $hb++) {
	print (($hb > 0x00) ? ",\n  " : "  ");
	if ($u2kout{$hb}) {
		printf "uni_to_cp949_tbl_%02x", $hb;
	} else {
		print "NULL";
	}
}

print "\n};\n";
print "\n\n";


print "#endif /* _KSX1001_HDR_ */\n";
