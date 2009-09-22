#!/usr/bin/perl
# usage:
#   % iso2022jp.pl > iso2022jp.h

require "cjkcompat.pl";


$perline = 8;
$unihan = "Unihan-3.2.0.txt.gz";

die "${unihan}: File not found.\n" if (!(-f $unihan));
open (SET, "gunzip -cd < ${unihan} |") or die "${unihan}: $!\n";

$ln=0; $ls = "";
while (<SET>)
{
  $line++;
  $ls = $_;
  chomp;
  s/\#.*//;
  next unless /^U\+(.....?)\s+kIRG_JSource\s+([01])\-(....)/;

  my $jcode = hex $3;
  my $jlevel = $2+0;
  my $ucode = hex $1;
  
  my $jcodeh = int ($jcode / 256);
  my $jcodel = $jcode % 256;

  if ($jcode < 0 || $jcode > 65535 || $ucode == 0 || 
      $jcodeh < 0x21 || $jcodeh > 0x7e || $jcodel < 0x21 || $jcodel > 0x7e) {
    print "$0: Out of JIS/Unicode code range\n";
    print sprintf("line %d: JIS 0x%04X, U+%04X\n", $line, $jcode, $ucode);
    print "> $ls";
    die;
  }

  if ($jlevel == 0) {
    $j2u{$jcode} = $ucode;
  } elsif ($jlevel == 1) {
    $jisx0212_j2u{$jcode} = $ucode;
  }
}

close(SET);

##
## Map for JIS X 0208:1983/1990/1997
##

# NOTE: Some row-cells of JIS X 0208:1997 and JIS X 0212 are duplicated
# with US-ASCII (identical to ISO 646 IRV) and GL of JIS X 0201 (ISO 646
# Japanese version).
# They are mapped to Fullwidth Form (U+FFxx) so that round-trip compatibility
# will be kept.

# DIGIT and LETTER of BASIC LATIN
foreach $jcode ((0x2330..0x2339,0x2341..0x235a,0x2361..0x237a)) {
  $ucode = $jcode + 0xdbe0;
  $j2u{$jcode} = $ucode;
}
# HIRAGANA
# Note: 0x2474-0x2476 are assigned by JIS X 0213:2000,
# but are found in some vendor codepages for JIS X 0208. 
for ($jcode = 0x2421; $jcode <= 0x2476; $jcode++) {
  $ucode = $jcode + 0xc20;
  $j2u{$jcode} = $ucode;
}
# KATAKANA
for ($jcode = 0x2521; $jcode <= 0x2576; $jcode++) {
  $ucode = $jcode + 0xb80;
  $j2u{$jcode} = $ucode;
}

#  Unicode-3.2 does not make mention of JIS X 0208 marks
#  (there is no clear definitions), so manually add a
#  converting map...
#  cf. JIS X 0208:1997

$j2u{0x2121} = 0x3000;
$j2u{0x2122} = 0x3001;
$j2u{0x2123} = 0x3002;
$j2u{0x2124} = 0xff0c;
$j2u{0x2125} = 0xff0e;
$j2u{0x2126} = 0x30fb;
$j2u{0x2127} = 0xff1a;
$j2u{0x2128} = 0xff1b;
$j2u{0x2129} = 0xff1f;
$j2u{0x212a} = 0xff01;
$j2u{0x212b} = 0x309b;
$j2u{0x212c} = 0x309c;
$j2u{0x212d} = 0x00b4;
$j2u{0x212e} = 0xff40;
$j2u{0x212f} = 0x00a8;
$j2u{0x2130} = 0xff3e;
$j2u{0x2131} = 0xffe3; # OVERLINE vs. JIS X 0201 GL -> FULLWIDTH MACRON
$j2u{0x2132} = 0xff3f;
$j2u{0x2133} = 0x30fd;
$j2u{0x2134} = 0x30fe;
$j2u{0x2135} = 0x309d;
$j2u{0x2136} = 0x309e;
$j2u{0x2137} = 0x3003;
$j2u{0x2138} = 0x4edd;
$j2u{0x2139} = 0x3005;
$j2u{0x213a} = 0x3006;
$j2u{0x213b} = 0x3007;
$j2u{0x213c} = 0x30fc;
$j2u{0x213d} = 0x2014; # HOLIZONTAL BAR (JIS X 0208:1990) -> EM DASH (1997)
$j2u{0x213e} = 0x2010;
$j2u{0x213f} = 0xff0f;
$j2u{0x2140} = 0xff3c; # REVERSE SOLIDUS vs. ASCII -> FULLWIDTH REVERSE SOLIDUS
$j2u{0x2141} = 0x301c;
$j2u{0x2142} = 0x2016;
$j2u{0x2143} = 0xff5c;
$j2u{0x2144} = 0x2026;
$j2u{0x2145} = 0x2025;
$j2u{0x2146} = 0x2018;
$j2u{0x2147} = 0x2019;
$j2u{0x2148} = 0x201c;
$j2u{0x2149} = 0x201d;
$j2u{0x214a} = 0xff08;
$j2u{0x214b} = 0xff09;
$j2u{0x214c} = 0x3014;
$j2u{0x214d} = 0x3015;
$j2u{0x214e} = 0xff3b;
$j2u{0x214f} = 0xff3d;
$j2u{0x2150} = 0xff5b;
$j2u{0x2151} = 0xff5d;
$j2u{0x2152} = 0x3008;
$j2u{0x2153} = 0x3009;
$j2u{0x2154} = 0x300a;
$j2u{0x2155} = 0x300b;
$j2u{0x2156} = 0x300c;
$j2u{0x2157} = 0x300d;
$j2u{0x2158} = 0x300e;
$j2u{0x2159} = 0x300f;
$j2u{0x215a} = 0x3010;
$j2u{0x215b} = 0x3011;
$j2u{0x215c} = 0xff0b;
$j2u{0x215d} = 0x2212;
$j2u{0x215e} = 0x00b1;
$j2u{0x215f} = 0x00d7;
$j2u{0x2160} = 0x00f7;
$j2u{0x2161} = 0xff1d;
$j2u{0x2162} = 0x2260;
$j2u{0x2163} = 0xff1c;
$j2u{0x2164} = 0xff1e;
$j2u{0x2165} = 0x2266;
$j2u{0x2166} = 0x2267;
$j2u{0x2167} = 0x221e;
$j2u{0x2168} = 0x2234;
$j2u{0x2169} = 0x2642;
$j2u{0x216a} = 0x2640;
$j2u{0x216b} = 0x00b0;
$j2u{0x216c} = 0x2032;
$j2u{0x216d} = 0x2033;
$j2u{0x216e} = 0x2103;
$j2u{0x216f} = 0xffe5; # YEN SIGN vs. JIS X 0201 GL -> FULLWIDTH YEN SIGN
$j2u{0x2170} = 0xff04;
$j2u{0x2171} = 0x00a2;
$j2u{0x2172} = 0x00a3;
$j2u{0x2173} = 0xff05;
$j2u{0x2174} = 0xff03;
$j2u{0x2175} = 0xff06;
$j2u{0x2176} = 0xff0a;
$j2u{0x2177} = 0xff20;
$j2u{0x2178} = 0x00a7;
$j2u{0x2179} = 0x2606;
$j2u{0x217a} = 0x2605;
$j2u{0x217b} = 0x25cb;
$j2u{0x217c} = 0x25cf;
$j2u{0x217d} = 0x25ce;
$j2u{0x217e} = 0x25c7;
$j2u{0x2221} = 0x25c6;
$j2u{0x2222} = 0x25a1;
$j2u{0x2223} = 0x25a0;
$j2u{0x2224} = 0x25b3;
$j2u{0x2225} = 0x25b2;
$j2u{0x2226} = 0x25bd;
$j2u{0x2227} = 0x25bc;
$j2u{0x2228} = 0x203b;
$j2u{0x2229} = 0x3012;
$j2u{0x222a} = 0x2192;
$j2u{0x222b} = 0x2190;
$j2u{0x222c} = 0x2191;
$j2u{0x222d} = 0x2193;
$j2u{0x222e} = 0x3013;
$j2u{0x223a} = 0x2208;
$j2u{0x223b} = 0x220b;
$j2u{0x223c} = 0x2286;
$j2u{0x223d} = 0x2287;
$j2u{0x223e} = 0x2282;
$j2u{0x223f} = 0x2283;
$j2u{0x2240} = 0x222a;
$j2u{0x2241} = 0x2229;
$j2u{0x224a} = 0x2227;
$j2u{0x224b} = 0x2228;
$j2u{0x224c} = 0x00ac;
$j2u{0x224d} = 0x21d2;
$j2u{0x224e} = 0x21d4;
$j2u{0x224f} = 0x2200;
$j2u{0x2250} = 0x2203;
$j2u{0x225c} = 0x2220;
$j2u{0x225d} = 0x22a5;
$j2u{0x225e} = 0x2312;
$j2u{0x225f} = 0x2202;
$j2u{0x2260} = 0x2207;
$j2u{0x2261} = 0x2261;
$j2u{0x2262} = 0x2252;
$j2u{0x2263} = 0x226a;
$j2u{0x2264} = 0x226b;
$j2u{0x2265} = 0x221a;
$j2u{0x2266} = 0x223d;
$j2u{0x2267} = 0x221d;
$j2u{0x2268} = 0x2235;
$j2u{0x2269} = 0x222b;
$j2u{0x226a} = 0x222c;
$j2u{0x2272} = 0x212b;
$j2u{0x2273} = 0x2030;
$j2u{0x2274} = 0x266f;
$j2u{0x2275} = 0x266d;
$j2u{0x2276} = 0x266a;
$j2u{0x2277} = 0x2020;
$j2u{0x2278} = 0x2021;
$j2u{0x2279} = 0x00b6;
$j2u{0x227e} = 0x25ef;
$j2u{0x2621} = 0x0391;
$j2u{0x2622} = 0x0392;
$j2u{0x2623} = 0x0393;
$j2u{0x2624} = 0x0394;
$j2u{0x2625} = 0x0395;
$j2u{0x2626} = 0x0396;
$j2u{0x2627} = 0x0397;
$j2u{0x2628} = 0x0398;
$j2u{0x2629} = 0x0399;
$j2u{0x262a} = 0x039a;
$j2u{0x262b} = 0x039b;
$j2u{0x262c} = 0x039c;
$j2u{0x262d} = 0x039d;
$j2u{0x262e} = 0x039e;
$j2u{0x262f} = 0x039f;
$j2u{0x2630} = 0x03a0;
$j2u{0x2631} = 0x03a1;
$j2u{0x2632} = 0x03a3;
$j2u{0x2633} = 0x03a4;
$j2u{0x2634} = 0x03a5;
$j2u{0x2635} = 0x03a6;
$j2u{0x2636} = 0x03a7;
$j2u{0x2637} = 0x03a8;
$j2u{0x2638} = 0x03a9;
$j2u{0x2641} = 0x03b1;
$j2u{0x2642} = 0x03b2;
$j2u{0x2643} = 0x03b3;
$j2u{0x2644} = 0x03b4;
$j2u{0x2645} = 0x03b5;
$j2u{0x2646} = 0x03b6;
$j2u{0x2647} = 0x03b7;
$j2u{0x2648} = 0x03b8;
$j2u{0x2649} = 0x03b9;
$j2u{0x264a} = 0x03ba;
$j2u{0x264b} = 0x03bb;
$j2u{0x264c} = 0x03bc;
$j2u{0x264d} = 0x03bd;
$j2u{0x264e} = 0x03be;
$j2u{0x264f} = 0x03bf;
$j2u{0x2650} = 0x03c0;
$j2u{0x2651} = 0x03c1;
$j2u{0x2652} = 0x03c3;
$j2u{0x2653} = 0x03c4;
$j2u{0x2654} = 0x03c5;
$j2u{0x2655} = 0x03c6;
$j2u{0x2656} = 0x03c7;
$j2u{0x2657} = 0x03c8;
$j2u{0x2658} = 0x03c9;
$j2u{0x2721} = 0x0410;
$j2u{0x2722} = 0x0411;
$j2u{0x2723} = 0x0412;
$j2u{0x2724} = 0x0413;
$j2u{0x2725} = 0x0414;
$j2u{0x2726} = 0x0415;
$j2u{0x2727} = 0x0401;
$j2u{0x2728} = 0x0416;
$j2u{0x2729} = 0x0417;
$j2u{0x272a} = 0x0418;
$j2u{0x272b} = 0x0419;
$j2u{0x272c} = 0x041a;
$j2u{0x272d} = 0x041b;
$j2u{0x272e} = 0x041c;
$j2u{0x272f} = 0x041d;
$j2u{0x2730} = 0x041e;
$j2u{0x2731} = 0x041f;
$j2u{0x2732} = 0x0420;
$j2u{0x2733} = 0x0421;
$j2u{0x2734} = 0x0422;
$j2u{0x2735} = 0x0423;
$j2u{0x2736} = 0x0424;
$j2u{0x2737} = 0x0425;
$j2u{0x2738} = 0x0426;
$j2u{0x2739} = 0x0427;
$j2u{0x273a} = 0x0428;
$j2u{0x273b} = 0x0429;
$j2u{0x273c} = 0x042a;
$j2u{0x273d} = 0x042b;
$j2u{0x273e} = 0x042c;
$j2u{0x273f} = 0x042d;
$j2u{0x2740} = 0x042e;
$j2u{0x2741} = 0x042f;
$j2u{0x2751} = 0x0430;
$j2u{0x2752} = 0x0431;
$j2u{0x2753} = 0x0432;
$j2u{0x2754} = 0x0433;
$j2u{0x2755} = 0x0434;
$j2u{0x2756} = 0x0435;
$j2u{0x2757} = 0x0451;
$j2u{0x2758} = 0x0436;
$j2u{0x2759} = 0x0437;
$j2u{0x275a} = 0x0438;
$j2u{0x275b} = 0x0439;
$j2u{0x275c} = 0x043a;
$j2u{0x275d} = 0x043b;
$j2u{0x275e} = 0x043c;
$j2u{0x275f} = 0x043d;
$j2u{0x2760} = 0x043e;
$j2u{0x2761} = 0x043f;
$j2u{0x2762} = 0x0440;
$j2u{0x2763} = 0x0441;
$j2u{0x2764} = 0x0442;
$j2u{0x2765} = 0x0443;
$j2u{0x2766} = 0x0444;
$j2u{0x2767} = 0x0445;
$j2u{0x2768} = 0x0446;
$j2u{0x2769} = 0x0447;
$j2u{0x276a} = 0x0448;
$j2u{0x276b} = 0x0449;
$j2u{0x276c} = 0x044a;
$j2u{0x276d} = 0x044b;
$j2u{0x276e} = 0x044c;
$j2u{0x276f} = 0x044d;
$j2u{0x2770} = 0x044e;
$j2u{0x2771} = 0x044f;
$j2u{0x2821} = 0x2500;
$j2u{0x2822} = 0x2502;
$j2u{0x2823} = 0x250c;
$j2u{0x2824} = 0x2510;
$j2u{0x2825} = 0x2518;
$j2u{0x2826} = 0x2514;
$j2u{0x2827} = 0x251c;
$j2u{0x2828} = 0x252c;
$j2u{0x2829} = 0x2524;
$j2u{0x282a} = 0x2534;
$j2u{0x282b} = 0x253c;
$j2u{0x282c} = 0x2501;
$j2u{0x282d} = 0x2503;
$j2u{0x282e} = 0x250f;
$j2u{0x282f} = 0x2513;
$j2u{0x2830} = 0x251b;
$j2u{0x2831} = 0x2517;
$j2u{0x2832} = 0x2523;
$j2u{0x2833} = 0x2533;
$j2u{0x2834} = 0x252b;
$j2u{0x2835} = 0x253b;
$j2u{0x2836} = 0x254b;
$j2u{0x2837} = 0x2520;
$j2u{0x2838} = 0x252f;
$j2u{0x2839} = 0x2528;
$j2u{0x283a} = 0x2537;
$j2u{0x283b} = 0x253f;
$j2u{0x283c} = 0x251d;
$j2u{0x283d} = 0x2530;
$j2u{0x283e} = 0x2525;
$j2u{0x283f} = 0x2538;
$j2u{0x2840} = 0x2542;

# 73 row-cells below are assigned as "Compatibility characters for
# national implementations" by JIS X 0213:2000,
# but are found in some vendor codepages for JIS X 0208.
# NOTE: U+2116 NUMERO SIGN is duplicated with JIS X 0212.  
$j2u{0x2d21} = 0x2460; # CIRCLED DIGIT ONE 
$j2u{0x2d22} = 0x2461; # CIRCLED DIGIT TWO 
$j2u{0x2d23} = 0x2462; # CIRCLED DIGIT THREE 
$j2u{0x2d24} = 0x2463; # CIRCLED DIGIT FOUR 
$j2u{0x2d25} = 0x2464; # CIRCLED DIGIT FIVE 
$j2u{0x2d26} = 0x2465; # CIRCLED DIGIT SIX 
$j2u{0x2d27} = 0x2466; # CIRCLED DIGIT SEVEN 
$j2u{0x2d28} = 0x2467; # CIRCLED DIGIT EIGHT 
$j2u{0x2d29} = 0x2468; # CIRCLED DIGIT NINE 
$j2u{0x2d2a} = 0x2469; # CIRCLED NUMBER TEN 
$j2u{0x2d2b} = 0x246a; # CIRCLED NUMBER ELEVEN 
$j2u{0x2d2c} = 0x246b; # CIRCLED NUMBER TWELVE 
$j2u{0x2d2d} = 0x246c; # CIRCLED NUMBER THIRTEEN 
$j2u{0x2d2e} = 0x246d; # CIRCLED NUMBER FOURTEEN 
$j2u{0x2d2f} = 0x246e; # CIRCLED NUMBER FIFTEEN 
$j2u{0x2d30} = 0x246f; # CIRCLED NUMBER SIXTEEN 
$j2u{0x2d31} = 0x2470; # CIRCLED NUMBER SEVENTEEN 
$j2u{0x2d32} = 0x2471; # CIRCLED NUMBER EIGHTEEN 
$j2u{0x2d33} = 0x2472; # CIRCLED NUMBER NINETEEN 
$j2u{0x2d34} = 0x2473; # CIRCLED NUMBER TWENTY 
$j2u{0x2d35} = 0x2160; # ROMAN NUMERAL ONE 
$j2u{0x2d36} = 0x2161; # ROMAN NUMERAL TWO 
$j2u{0x2d37} = 0x2162; # ROMAN NUMERAL THREE 
$j2u{0x2d38} = 0x2163; # ROMAN NUMERAL FOUR 
$j2u{0x2d39} = 0x2164; # ROMAN NUMERAL FIVE 
$j2u{0x2d3a} = 0x2165; # ROMAN NUMERAL SIX 
$j2u{0x2d3b} = 0x2166; # ROMAN NUMERAL SEVEN 
$j2u{0x2d3c} = 0x2167; # ROMAN NUMERAL EIGHT 
$j2u{0x2d3d} = 0x2168; # ROMAN NUMERAL NINE 
$j2u{0x2d3e} = 0x2169; # ROMAN NUMERAL TEN 
#$j2u{0x2d3f} = 0x216a; # ROMAN NUMERAL ELEVEN 
$j2u{0x2d40} = 0x3349; # SQUARE MIRI 
$j2u{0x2d41} = 0x3314; # SQUARE KIRO 
$j2u{0x2d42} = 0x3322; # SQUARE SENTI 
$j2u{0x2d43} = 0x334d; # SQUARE MEETORU 
$j2u{0x2d44} = 0x3318; # SQUARE GURAMU 
$j2u{0x2d45} = 0x3327; # SQUARE TON 
$j2u{0x2d46} = 0x3303; # SQUARE AARU 
$j2u{0x2d47} = 0x3336; # SQUARE HEKUTAARU 
$j2u{0x2d48} = 0x3351; # SQUARE RITTORU 
$j2u{0x2d49} = 0x3357; # SQUARE WATTO 
$j2u{0x2d4a} = 0x330d; # SQUARE KARORII 
$j2u{0x2d4b} = 0x3326; # SQUARE DORU 
$j2u{0x2d4c} = 0x3323; # SQUARE SENTO 
$j2u{0x2d4d} = 0x332b; # SQUARE PAASENTO 
$j2u{0x2d4e} = 0x334a; # SQUARE MIRIBAARU 
$j2u{0x2d4f} = 0x333b; # SQUARE PEEZI 
$j2u{0x2d50} = 0x339c; # SQUARE MM 
$j2u{0x2d51} = 0x339d; # SQUARE CM 
$j2u{0x2d52} = 0x339e; # SQUARE KM 
$j2u{0x2d53} = 0x338e; # SQUARE MG 
$j2u{0x2d54} = 0x338f; # SQUARE KG 
$j2u{0x2d55} = 0x33c4; # SQUARE CC 
$j2u{0x2d56} = 0x33a1; # SQUARE M SQUARED 
#$j2u{0x2d57} = 0x216b; # ROMAN NUMERAL TWELVE 
$j2u{0x2d5f} = 0x337b; # SQUARE ERA NAME HEISEI 
$j2u{0x2d60} = 0x301d; # REVERSED DOUBLE PRIME QUATATION MARK 
$j2u{0x2d61} = 0x301f; # LOW DOUBLE PRIME QUATATION MARK 
$j2u{0x2d62} = 0x2116; # NUMERO SIGN 
$j2u{0x2d63} = 0x33cd; # SQUARE KK 
$j2u{0x2d64} = 0x2121; # TELEPHONE SIGN 
$j2u{0x2d65} = 0x32a4; # CIRCLED IDEOGRAPH HIGH 
$j2u{0x2d66} = 0x32a5; # CIRCLED IDEOGRAPH CENTRE 
$j2u{0x2d67} = 0x32a6; # CIRCLED IDEOGRAPH LOW 
$j2u{0x2d68} = 0x32a7; # CIRCLED IDEOGRAPH LEFT 
$j2u{0x2d69} = 0x32a8; # CIRCLED IDEOGRAPH RIGHT 
$j2u{0x2d6a} = 0x3231; # PARENTHESIZED IDEOGRAPH STOCK 
$j2u{0x2d6b} = 0x3232; # PARENTHESIZED IDEOGRAPH HAVE 
$j2u{0x2d6c} = 0x3239; # PARENTHESIZED IDEOGRAPH REPRESENT 
$j2u{0x2d6d} = 0x337e; # SQUARE ERA NAME MEIZI 
$j2u{0x2d6e} = 0x337d; # SQUARE ERA NAME TAISYOU 
$j2u{0x2d6f} = 0x337c; # SQUARE ERA NAME SYOUWA 
$j2u{0x2d73} = 0x222e; # CONTOUR INTEGRAL 
$j2u{0x2d78} = 0x221f; # RIGHT ANGLE 
$j2u{0x2d79} = 0x22bf; # RIGHT TRIANGLE 
#$j2u{0x2d7d} = 0x2756; # BLACK DIAMOND MINUS WHITE X 
#$j2u{0x2d7e} = 0x261e; # WHITE RIGHT POINTING INDEX 


##
## Map (upper-)compatible with JIS C 6226:1978
##

# 26 pairs of row-cells in JIS X 0208:1997 are swapped.
# cf. JIS X 0208:1997 Annex 2.

foreach (keys %j2u) {
  $jisx0208_1978_j2u{$_} = $j2u{$_};
}
&swap_1978(0x3033, 0x724D);
&swap_1978(0x3229, 0x7274);
&swap_1978(0x3342, 0x695a);
&swap_1978(0x3349, 0x5978);
&swap_1978(0x3376, 0x635e);
&swap_1978(0x3443, 0x5e75);
&swap_1978(0x3452, 0x6b5d);
&swap_1978(0x375b, 0x7074);
&swap_1978(0x395c, 0x6268);
&swap_1978(0x3c49, 0x6922);
&swap_1978(0x3F59, 0x7057);
&swap_1978(0x4128, 0x6c4d);
&swap_1978(0x445B, 0x5464);
&swap_1978(0x4557, 0x626a);
&swap_1978(0x456e, 0x5b6d);
&swap_1978(0x4573, 0x5e39);
&swap_1978(0x4676, 0x6d6e);
&swap_1978(0x4768, 0x6a24);
&swap_1978(0x4930, 0x5B58);
&swap_1978(0x4b79, 0x5056);
&swap_1978(0x4c79, 0x692e);
&swap_1978(0x4F36, 0x6446);
&swap_1978(0x3646, 0x7421);
&swap_1978(0x4B6A, 0x7422);
&swap_1978(0x4D5A, 0x7423);
&swap_1978(0x6076, 0x7424);

sub swap_1978 {
local($x, $y) = @_;
  ($jisx0208_1978_j2u{$x}, $jisx0208_1978_j2u{$y}) =
    ($jisx0208_1978_j2u{$y}, $jisx0208_1978_j2u{$x});
  $j2u_1978{int($x/256)} = 1;
  $j2u_1978{int($y/256)} = 1;
  $u2j_1978{int($jisx0208_1978_j2u{$x}/256)} = 1;
  $u2j_1978{int($jisx0208_1978_j2u{$y}/256)} = 1;
}


##
## Map for JIS X 0212:1990 ("Supplementary Kanzi")
##

#  Unicode-3.2 does not make mention of JIS X 0212 symbols, marks,
#  alphabets with diacritial mark etc. So manually add converting map...
# cf. ftp://ftp.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/JIS0212.TXT

$jisx0212_j2u{0x222f} = 0x02d8;	# BREVE
$jisx0212_j2u{0x2230} = 0x02c7;	# CARON (Mandarin Chinese third tone)
$jisx0212_j2u{0x2231} = 0x00b8;	# CEDILLA
$jisx0212_j2u{0x2232} = 0x02d9;	# DOT ABOVE (Mandarin Chinese light tone)
$jisx0212_j2u{0x2233} = 0x02dd;	# DOUBLE ACUTE ACCENT
$jisx0212_j2u{0x2234} = 0x00af;	# MACRON
$jisx0212_j2u{0x2235} = 0x02db;	# OGONEK
$jisx0212_j2u{0x2236} = 0x02da;	# RING ABOVE
$jisx0212_j2u{0x2237} = 0xff5e;	# TILDE vs. ASCII -> FULLWIDTH TILDE
$jisx0212_j2u{0x2238} = 0x0384;	# GREEK TONOS
$jisx0212_j2u{0x2239} = 0x0385;	# GREEK DIALYTIKA TONOS
$jisx0212_j2u{0x2242} = 0x00a1;	# INVERTED EXCLAMATION MARK
$jisx0212_j2u{0x2243} = 0x00a6;	# BROKEN BAR
$jisx0212_j2u{0x2244} = 0x00bf;	# INVERTED QUESTION MARK
$jisx0212_j2u{0x226b} = 0x00ba;	# MASCULINE ORDINAL INDICATOR
$jisx0212_j2u{0x226c} = 0x00aa;	# FEMININE ORDINAL INDICATOR
$jisx0212_j2u{0x226d} = 0x00a9;	# COPYRIGHT SIGN
$jisx0212_j2u{0x226e} = 0x00ae;	# REGISTERED SIGN
$jisx0212_j2u{0x226f} = 0x2122;	# TRADE MARK SIGN
$jisx0212_j2u{0x2270} = 0x00a4;	# CURRENCY SIGN
$jisx0212_j2u{0x2271} = 0x2116;	# NUMERO SIGN
$jisx0212_j2u{0x2661} = 0x0386;	# GREEK CAPITAL LETTER ALPHA WITH TONOS
$jisx0212_j2u{0x2662} = 0x0388;	# GREEK CAPITAL LETTER EPSILON WITH TONOS
$jisx0212_j2u{0x2663} = 0x0389;	# GREEK CAPITAL LETTER ETA WITH TONOS
$jisx0212_j2u{0x2664} = 0x038a;	# GREEK CAPITAL LETTER IOTA WITH TONOS
$jisx0212_j2u{0x2665} = 0x03aa;	# GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
$jisx0212_j2u{0x2667} = 0x038c;	# GREEK CAPITAL LETTER OMICRON WITH TONOS
$jisx0212_j2u{0x2669} = 0x038e;	# GREEK CAPITAL LETTER UPSILON WITH TONOS
$jisx0212_j2u{0x266a} = 0x03ab;	# GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
$jisx0212_j2u{0x266c} = 0x038f;	# GREEK CAPITAL LETTER OMEGA WITH TONOS
$jisx0212_j2u{0x2671} = 0x03ac;	# GREEK SMALL LETTER ALPHA WITH TONOS
$jisx0212_j2u{0x2672} = 0x03ad;	# GREEK SMALL LETTER EPSILON WITH TONOS
$jisx0212_j2u{0x2673} = 0x03ae;	# GREEK SMALL LETTER ETA WITH TONOS
$jisx0212_j2u{0x2674} = 0x03af;	# GREEK SMALL LETTER IOTA WITH TONOS
$jisx0212_j2u{0x2675} = 0x03ca;	# GREEK SMALL LETTER IOTA WITH DIALYTIKA
$jisx0212_j2u{0x2676} = 0x0390;	# GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
$jisx0212_j2u{0x2677} = 0x03cc;	# GREEK SMALL LETTER OMICRON WITH TONOS
$jisx0212_j2u{0x2678} = 0x03c2;	# GREEK SMALL LETTER FINAL SIGMA
$jisx0212_j2u{0x2679} = 0x03cd;	# GREEK SMALL LETTER UPSILON WITH TONOS
$jisx0212_j2u{0x267a} = 0x03cb;	# GREEK SMALL LETTER UPSILON WITH DIALYTIKA
$jisx0212_j2u{0x267b} = 0x03b0;	# GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
$jisx0212_j2u{0x267c} = 0x03ce;	# GREEK SMALL LETTER OMEGA WITH TONOS
$jisx0212_j2u{0x2742} = 0x0402;	# CYRILLIC CAPITAL LETTER DJE
$jisx0212_j2u{0x2743} = 0x0403;	# CYRILLIC CAPITAL LETTER GJE
$jisx0212_j2u{0x2744} = 0x0404;	# CYRILLIC CAPITAL LETTER UKRAINIAN IE
$jisx0212_j2u{0x2745} = 0x0405;	# CYRILLIC CAPITAL LETTER DZE
$jisx0212_j2u{0x2746} = 0x0406;	# CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
$jisx0212_j2u{0x2747} = 0x0407;	# CYRILLIC CAPITAL LETTER YI
$jisx0212_j2u{0x2748} = 0x0408;	# CYRILLIC CAPITAL LETTER JE
$jisx0212_j2u{0x2749} = 0x0409;	# CYRILLIC CAPITAL LETTER LJE
$jisx0212_j2u{0x274a} = 0x040a;	# CYRILLIC CAPITAL LETTER NJE
$jisx0212_j2u{0x274b} = 0x040b;	# CYRILLIC CAPITAL LETTER TSHE
$jisx0212_j2u{0x274c} = 0x040c;	# CYRILLIC CAPITAL LETTER KJE
$jisx0212_j2u{0x274d} = 0x040e;	# CYRILLIC CAPITAL LETTER SHORT U
$jisx0212_j2u{0x274e} = 0x040f;	# CYRILLIC CAPITAL LETTER DZHE
$jisx0212_j2u{0x2772} = 0x0452;	# CYRILLIC SMALL LETTER DJE
$jisx0212_j2u{0x2773} = 0x0453;	# CYRILLIC SMALL LETTER GJE
$jisx0212_j2u{0x2774} = 0x0454;	# CYRILLIC SMALL LETTER UKRAINIAN IE
$jisx0212_j2u{0x2775} = 0x0455;	# CYRILLIC SMALL LETTER DZE
$jisx0212_j2u{0x2776} = 0x0456;	# CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
$jisx0212_j2u{0x2777} = 0x0457;	# CYRILLIC SMALL LETTER YI
$jisx0212_j2u{0x2778} = 0x0458;	# CYRILLIC SMALL LETTER JE
$jisx0212_j2u{0x2779} = 0x0459;	# CYRILLIC SMALL LETTER LJE
$jisx0212_j2u{0x277a} = 0x045a;	# CYRILLIC SMALL LETTER NJE
$jisx0212_j2u{0x277b} = 0x045b;	# CYRILLIC SMALL LETTER TSHE
$jisx0212_j2u{0x277c} = 0x045c;	# CYRILLIC SMALL LETTER KJE
$jisx0212_j2u{0x277d} = 0x045e;	# CYRILLIC SMALL LETTER SHORT U
$jisx0212_j2u{0x277e} = 0x045f;	# CYRILLIC SMALL LETTER DZHE
$jisx0212_j2u{0x2921} = 0x00c6;	# LATIN CAPITAL LIGATURE AE
$jisx0212_j2u{0x2922} = 0x0110;	# LATIN CAPITAL LETTER D WITH STROKE
$jisx0212_j2u{0x2924} = 0x0126;	# LATIN CAPITAL LETTER H WITH STROKE
$jisx0212_j2u{0x2926} = 0x0132;	# LATIN CAPITAL LIGATURE IJ
$jisx0212_j2u{0x2928} = 0x0141;	# LATIN CAPITAL LETTER L WITH STROKE
$jisx0212_j2u{0x2929} = 0x013f;	# LATIN CAPITAL LETTER L WITH MIDDLE DOT
$jisx0212_j2u{0x292b} = 0x014a;	# LATIN CAPITAL LETTER ENG
$jisx0212_j2u{0x292c} = 0x00d8;	# LATIN CAPITAL LETTER O WITH STROKE
$jisx0212_j2u{0x292d} = 0x0152;	# LATIN CAPITAL LIGATURE OE
$jisx0212_j2u{0x292f} = 0x0166;	# LATIN CAPITAL LETTER T WITH STROKE
$jisx0212_j2u{0x2930} = 0x00de;	# LATIN CAPITAL LETTER THORN
$jisx0212_j2u{0x2941} = 0x00e6;	# LATIN SMALL LIGATURE AE
$jisx0212_j2u{0x2942} = 0x0111;	# LATIN SMALL LETTER D WITH STROKE
$jisx0212_j2u{0x2943} = 0x00f0;	# LATIN SMALL LETTER ETH
$jisx0212_j2u{0x2944} = 0x0127;	# LATIN SMALL LETTER H WITH STROKE
$jisx0212_j2u{0x2945} = 0x0131;	# LATIN SMALL LETTER DOTLESS I
$jisx0212_j2u{0x2946} = 0x0133;	# LATIN SMALL LIGATURE IJ
$jisx0212_j2u{0x2947} = 0x0138;	# LATIN SMALL LETTER KRA
$jisx0212_j2u{0x2948} = 0x0142;	# LATIN SMALL LETTER L WITH STROKE
$jisx0212_j2u{0x2949} = 0x0140;	# LATIN SMALL LETTER L WITH MIDDLE DOT
$jisx0212_j2u{0x294a} = 0x0149;	# LATIN SMALL LETTER N PRECEDED BY APOSTROPHE
$jisx0212_j2u{0x294b} = 0x014b;	# LATIN SMALL LETTER ENG
$jisx0212_j2u{0x294c} = 0x00f8;	# LATIN SMALL LETTER O WITH STROKE
$jisx0212_j2u{0x294d} = 0x0153;	# LATIN SMALL LIGATURE OE
$jisx0212_j2u{0x294e} = 0x00df;	# LATIN SMALL LETTER SHARP S
$jisx0212_j2u{0x294f} = 0x0167;	# LATIN SMALL LETTER T WITH STROKE
$jisx0212_j2u{0x2950} = 0x00fe;	# LATIN SMALL LETTER THORN
$jisx0212_j2u{0x2a21} = 0x00c1;	# LATIN CAPITAL LETTER A WITH ACUTE
$jisx0212_j2u{0x2a22} = 0x00c0;	# LATIN CAPITAL LETTER A WITH GRAVE
$jisx0212_j2u{0x2a23} = 0x00c4;	# LATIN CAPITAL LETTER A WITH DIAERESIS
$jisx0212_j2u{0x2a24} = 0x00c2;	# LATIN CAPITAL LETTER A WITH CIRCUMFLEX
$jisx0212_j2u{0x2a25} = 0x0102;	# LATIN CAPITAL LETTER A WITH BREVE
$jisx0212_j2u{0x2a26} = 0x01cd;	# LATIN CAPITAL LETTER A WITH CARON
$jisx0212_j2u{0x2a27} = 0x0100;	# LATIN CAPITAL LETTER A WITH MACRON
$jisx0212_j2u{0x2a28} = 0x0104;	# LATIN CAPITAL LETTER A WITH OGONEK
$jisx0212_j2u{0x2a29} = 0x00c5;	# LATIN CAPITAL LETTER A WITH RING ABOVE
$jisx0212_j2u{0x2a2a} = 0x00c3;	# LATIN CAPITAL LETTER A WITH TILDE
$jisx0212_j2u{0x2a2b} = 0x0106;	# LATIN CAPITAL LETTER C WITH ACUTE
$jisx0212_j2u{0x2a2c} = 0x0108;	# LATIN CAPITAL LETTER C WITH CIRCUMFLEX
$jisx0212_j2u{0x2a2d} = 0x010c;	# LATIN CAPITAL LETTER C WITH CARON
$jisx0212_j2u{0x2a2e} = 0x00c7;	# LATIN CAPITAL LETTER C WITH CEDILLA
$jisx0212_j2u{0x2a2f} = 0x010a;	# LATIN CAPITAL LETTER C WITH DOT ABOVE
$jisx0212_j2u{0x2a30} = 0x010e;	# LATIN CAPITAL LETTER D WITH CARON
$jisx0212_j2u{0x2a31} = 0x00c9;	# LATIN CAPITAL LETTER E WITH ACUTE
$jisx0212_j2u{0x2a32} = 0x00c8;	# LATIN CAPITAL LETTER E WITH GRAVE
$jisx0212_j2u{0x2a33} = 0x00cb;	# LATIN CAPITAL LETTER E WITH DIAERESIS
$jisx0212_j2u{0x2a34} = 0x00ca;	# LATIN CAPITAL LETTER E WITH CIRCUMFLEX
$jisx0212_j2u{0x2a35} = 0x011a;	# LATIN CAPITAL LETTER E WITH CARON
$jisx0212_j2u{0x2a36} = 0x0116;	# LATIN CAPITAL LETTER E WITH DOT ABOVE
$jisx0212_j2u{0x2a37} = 0x0112;	# LATIN CAPITAL LETTER E WITH MACRON
$jisx0212_j2u{0x2a38} = 0x0118;	# LATIN CAPITAL LETTER E WITH OGONEK
$jisx0212_j2u{0x2a3a} = 0x011c;	# LATIN CAPITAL LETTER G WITH CIRCUMFLEX
$jisx0212_j2u{0x2a3b} = 0x011e;	# LATIN CAPITAL LETTER G WITH BREVE
$jisx0212_j2u{0x2a3c} = 0x0122;	# LATIN CAPITAL LETTER G WITH CEDILLA
$jisx0212_j2u{0x2a3d} = 0x0120;	# LATIN CAPITAL LETTER G WITH DOT ABOVE
$jisx0212_j2u{0x2a3e} = 0x0124;	# LATIN CAPITAL LETTER H WITH CIRCUMFLEX
$jisx0212_j2u{0x2a3f} = 0x00cd;	# LATIN CAPITAL LETTER I WITH ACUTE
$jisx0212_j2u{0x2a40} = 0x00cc;	# LATIN CAPITAL LETTER I WITH GRAVE
$jisx0212_j2u{0x2a41} = 0x00cf;	# LATIN CAPITAL LETTER I WITH DIAERESIS
$jisx0212_j2u{0x2a42} = 0x00ce;	# LATIN CAPITAL LETTER I WITH CIRCUMFLEX
$jisx0212_j2u{0x2a43} = 0x01cf;	# LATIN CAPITAL LETTER I WITH CARON
$jisx0212_j2u{0x2a44} = 0x0130;	# LATIN CAPITAL LETTER I WITH DOT ABOVE
$jisx0212_j2u{0x2a45} = 0x012a;	# LATIN CAPITAL LETTER I WITH MACRON
$jisx0212_j2u{0x2a46} = 0x012e;	# LATIN CAPITAL LETTER I WITH OGONEK
$jisx0212_j2u{0x2a47} = 0x0128;	# LATIN CAPITAL LETTER I WITH TILDE
$jisx0212_j2u{0x2a48} = 0x0134;	# LATIN CAPITAL LETTER J WITH CIRCUMFLEX
$jisx0212_j2u{0x2a49} = 0x0136;	# LATIN CAPITAL LETTER K WITH CEDILLA
$jisx0212_j2u{0x2a4a} = 0x0139;	# LATIN CAPITAL LETTER L WITH ACUTE
$jisx0212_j2u{0x2a4b} = 0x013d;	# LATIN CAPITAL LETTER L WITH CARON
$jisx0212_j2u{0x2a4c} = 0x013b;	# LATIN CAPITAL LETTER L WITH CEDILLA
$jisx0212_j2u{0x2a4d} = 0x0143;	# LATIN CAPITAL LETTER N WITH ACUTE
$jisx0212_j2u{0x2a4e} = 0x0147;	# LATIN CAPITAL LETTER N WITH CARON
$jisx0212_j2u{0x2a4f} = 0x0145;	# LATIN CAPITAL LETTER N WITH CEDILLA
$jisx0212_j2u{0x2a50} = 0x00d1;	# LATIN CAPITAL LETTER N WITH TILDE
$jisx0212_j2u{0x2a51} = 0x00d3;	# LATIN CAPITAL LETTER O WITH ACUTE
$jisx0212_j2u{0x2a52} = 0x00d2;	# LATIN CAPITAL LETTER O WITH GRAVE
$jisx0212_j2u{0x2a53} = 0x00d6;	# LATIN CAPITAL LETTER O WITH DIAERESIS
$jisx0212_j2u{0x2a54} = 0x00d4;	# LATIN CAPITAL LETTER O WITH CIRCUMFLEX
$jisx0212_j2u{0x2a55} = 0x01d1;	# LATIN CAPITAL LETTER O WITH CARON
$jisx0212_j2u{0x2a56} = 0x0150;	# LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
$jisx0212_j2u{0x2a57} = 0x014c;	# LATIN CAPITAL LETTER O WITH MACRON
$jisx0212_j2u{0x2a58} = 0x00d5;	# LATIN CAPITAL LETTER O WITH TILDE
$jisx0212_j2u{0x2a59} = 0x0154;	# LATIN CAPITAL LETTER R WITH ACUTE
$jisx0212_j2u{0x2a5a} = 0x0158;	# LATIN CAPITAL LETTER R WITH CARON
$jisx0212_j2u{0x2a5b} = 0x0156;	# LATIN CAPITAL LETTER R WITH CEDILLA
$jisx0212_j2u{0x2a5c} = 0x015a;	# LATIN CAPITAL LETTER S WITH ACUTE
$jisx0212_j2u{0x2a5d} = 0x015c;	# LATIN CAPITAL LETTER S WITH CIRCUMFLEX
$jisx0212_j2u{0x2a5e} = 0x0160;	# LATIN CAPITAL LETTER S WITH CARON
$jisx0212_j2u{0x2a5f} = 0x015e;	# LATIN CAPITAL LETTER S WITH CEDILLA
$jisx0212_j2u{0x2a60} = 0x0164;	# LATIN CAPITAL LETTER T WITH CARON
$jisx0212_j2u{0x2a61} = 0x0162;	# LATIN CAPITAL LETTER T WITH CEDILLA
$jisx0212_j2u{0x2a62} = 0x00da;	# LATIN CAPITAL LETTER U WITH ACUTE
$jisx0212_j2u{0x2a63} = 0x00d9;	# LATIN CAPITAL LETTER U WITH GRAVE
$jisx0212_j2u{0x2a64} = 0x00dc;	# LATIN CAPITAL LETTER U WITH DIAERESIS
$jisx0212_j2u{0x2a65} = 0x00db;	# LATIN CAPITAL LETTER U WITH CIRCUMFLEX
$jisx0212_j2u{0x2a66} = 0x016c;	# LATIN CAPITAL LETTER U WITH BREVE
$jisx0212_j2u{0x2a67} = 0x01d3;	# LATIN CAPITAL LETTER U WITH CARON
$jisx0212_j2u{0x2a68} = 0x0170;	# LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
$jisx0212_j2u{0x2a69} = 0x016a;	# LATIN CAPITAL LETTER U WITH MACRON
$jisx0212_j2u{0x2a6a} = 0x0172;	# LATIN CAPITAL LETTER U WITH OGONEK
$jisx0212_j2u{0x2a6b} = 0x016e;	# LATIN CAPITAL LETTER U WITH RING ABOVE
$jisx0212_j2u{0x2a6c} = 0x0168;	# LATIN CAPITAL LETTER U WITH TILDE
$jisx0212_j2u{0x2a6d} = 0x01d7;	# LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE
$jisx0212_j2u{0x2a6e} = 0x01db;	# LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE
$jisx0212_j2u{0x2a6f} = 0x01d9;	# LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON
$jisx0212_j2u{0x2a70} = 0x01d5;	# LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON
$jisx0212_j2u{0x2a71} = 0x0174;	# LATIN CAPITAL LETTER W WITH CIRCUMFLEX
$jisx0212_j2u{0x2a72} = 0x00dd;	# LATIN CAPITAL LETTER Y WITH ACUTE
$jisx0212_j2u{0x2a73} = 0x0178;	# LATIN CAPITAL LETTER Y WITH DIAERESIS
$jisx0212_j2u{0x2a74} = 0x0176;	# LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
$jisx0212_j2u{0x2a75} = 0x0179;	# LATIN CAPITAL LETTER Z WITH ACUTE
$jisx0212_j2u{0x2a76} = 0x017d;	# LATIN CAPITAL LETTER Z WITH CARON
$jisx0212_j2u{0x2a77} = 0x017b;	# LATIN CAPITAL LETTER Z WITH DOT ABOVE
$jisx0212_j2u{0x2b21} = 0x00e1;	# LATIN SMALL LETTER A WITH ACUTE
$jisx0212_j2u{0x2b22} = 0x00e0;	# LATIN SMALL LETTER A WITH GRAVE
$jisx0212_j2u{0x2b23} = 0x00e4;	# LATIN SMALL LETTER A WITH DIAERESIS
$jisx0212_j2u{0x2b24} = 0x00e2;	# LATIN SMALL LETTER A WITH CIRCUMFLEX
$jisx0212_j2u{0x2b25} = 0x0103;	# LATIN SMALL LETTER A WITH BREVE
$jisx0212_j2u{0x2b26} = 0x01ce;	# LATIN SMALL LETTER A WITH CARON
$jisx0212_j2u{0x2b27} = 0x0101;	# LATIN SMALL LETTER A WITH MACRON
$jisx0212_j2u{0x2b28} = 0x0105;	# LATIN SMALL LETTER A WITH OGONEK
$jisx0212_j2u{0x2b29} = 0x00e5;	# LATIN SMALL LETTER A WITH RING ABOVE
$jisx0212_j2u{0x2b2a} = 0x00e3;	# LATIN SMALL LETTER A WITH TILDE
$jisx0212_j2u{0x2b2b} = 0x0107;	# LATIN SMALL LETTER C WITH ACUTE
$jisx0212_j2u{0x2b2c} = 0x0109;	# LATIN SMALL LETTER C WITH CIRCUMFLEX
$jisx0212_j2u{0x2b2d} = 0x010d;	# LATIN SMALL LETTER C WITH CARON
$jisx0212_j2u{0x2b2e} = 0x00e7;	# LATIN SMALL LETTER C WITH CEDILLA
$jisx0212_j2u{0x2b2f} = 0x010b;	# LATIN SMALL LETTER C WITH DOT ABOVE
$jisx0212_j2u{0x2b30} = 0x010f;	# LATIN SMALL LETTER D WITH CARON
$jisx0212_j2u{0x2b31} = 0x00e9;	# LATIN SMALL LETTER E WITH ACUTE
$jisx0212_j2u{0x2b32} = 0x00e8;	# LATIN SMALL LETTER E WITH GRAVE
$jisx0212_j2u{0x2b33} = 0x00eb;	# LATIN SMALL LETTER E WITH DIAERESIS
$jisx0212_j2u{0x2b34} = 0x00ea;	# LATIN SMALL LETTER E WITH CIRCUMFLEX
$jisx0212_j2u{0x2b35} = 0x011b;	# LATIN SMALL LETTER E WITH CARON
$jisx0212_j2u{0x2b36} = 0x0117;	# LATIN SMALL LETTER E WITH DOT ABOVE
$jisx0212_j2u{0x2b37} = 0x0113;	# LATIN SMALL LETTER E WITH MACRON
$jisx0212_j2u{0x2b38} = 0x0119;	# LATIN SMALL LETTER E WITH OGONEK
$jisx0212_j2u{0x2b39} = 0x01f5;	# LATIN SMALL LETTER G WITH ACUTE
$jisx0212_j2u{0x2b3a} = 0x011d;	# LATIN SMALL LETTER G WITH CIRCUMFLEX
$jisx0212_j2u{0x2b3b} = 0x011f;	# LATIN SMALL LETTER G WITH BREVE
$jisx0212_j2u{0x2b3d} = 0x0121;	# LATIN SMALL LETTER G WITH DOT ABOVE
$jisx0212_j2u{0x2b3e} = 0x0125;	# LATIN SMALL LETTER H WITH CIRCUMFLEX
$jisx0212_j2u{0x2b3f} = 0x00ed;	# LATIN SMALL LETTER I WITH ACUTE
$jisx0212_j2u{0x2b40} = 0x00ec;	# LATIN SMALL LETTER I WITH GRAVE
$jisx0212_j2u{0x2b41} = 0x00ef;	# LATIN SMALL LETTER I WITH DIAERESIS
$jisx0212_j2u{0x2b42} = 0x00ee;	# LATIN SMALL LETTER I WITH CIRCUMFLEX
$jisx0212_j2u{0x2b43} = 0x01d0;	# LATIN SMALL LETTER I WITH CARON
$jisx0212_j2u{0x2b45} = 0x012b;	# LATIN SMALL LETTER I WITH MACRON
$jisx0212_j2u{0x2b46} = 0x012f;	# LATIN SMALL LETTER I WITH OGONEK
$jisx0212_j2u{0x2b47} = 0x0129;	# LATIN SMALL LETTER I WITH TILDE
$jisx0212_j2u{0x2b48} = 0x0135;	# LATIN SMALL LETTER J WITH CIRCUMFLEX
$jisx0212_j2u{0x2b49} = 0x0137;	# LATIN SMALL LETTER K WITH CEDILLA
$jisx0212_j2u{0x2b4a} = 0x013a;	# LATIN SMALL LETTER L WITH ACUTE
$jisx0212_j2u{0x2b4b} = 0x013e;	# LATIN SMALL LETTER L WITH CARON
$jisx0212_j2u{0x2b4c} = 0x013c;	# LATIN SMALL LETTER L WITH CEDILLA
$jisx0212_j2u{0x2b4d} = 0x0144;	# LATIN SMALL LETTER N WITH ACUTE
$jisx0212_j2u{0x2b4e} = 0x0148;	# LATIN SMALL LETTER N WITH CARON
$jisx0212_j2u{0x2b4f} = 0x0146;	# LATIN SMALL LETTER N WITH CEDILLA
$jisx0212_j2u{0x2b50} = 0x00f1;	# LATIN SMALL LETTER N WITH TILDE
$jisx0212_j2u{0x2b51} = 0x00f3;	# LATIN SMALL LETTER O WITH ACUTE
$jisx0212_j2u{0x2b52} = 0x00f2;	# LATIN SMALL LETTER O WITH GRAVE
$jisx0212_j2u{0x2b53} = 0x00f6;	# LATIN SMALL LETTER O WITH DIAERESIS
$jisx0212_j2u{0x2b54} = 0x00f4;	# LATIN SMALL LETTER O WITH CIRCUMFLEX
$jisx0212_j2u{0x2b55} = 0x01d2;	# LATIN SMALL LETTER O WITH CARON
$jisx0212_j2u{0x2b56} = 0x0151;	# LATIN SMALL LETTER O WITH DOUBLE ACUTE
$jisx0212_j2u{0x2b57} = 0x014d;	# LATIN SMALL LETTER O WITH MACRON
$jisx0212_j2u{0x2b58} = 0x00f5;	# LATIN SMALL LETTER O WITH TILDE
$jisx0212_j2u{0x2b59} = 0x0155;	# LATIN SMALL LETTER R WITH ACUTE
$jisx0212_j2u{0x2b5a} = 0x0159;	# LATIN SMALL LETTER R WITH CARON
$jisx0212_j2u{0x2b5b} = 0x0157;	# LATIN SMALL LETTER R WITH CEDILLA
$jisx0212_j2u{0x2b5c} = 0x015b;	# LATIN SMALL LETTER S WITH ACUTE
$jisx0212_j2u{0x2b5d} = 0x015d;	# LATIN SMALL LETTER S WITH CIRCUMFLEX
$jisx0212_j2u{0x2b5e} = 0x0161;	# LATIN SMALL LETTER S WITH CARON
$jisx0212_j2u{0x2b5f} = 0x015f;	# LATIN SMALL LETTER S WITH CEDILLA
$jisx0212_j2u{0x2b60} = 0x0165;	# LATIN SMALL LETTER T WITH CARON
$jisx0212_j2u{0x2b61} = 0x0163;	# LATIN SMALL LETTER T WITH CEDILLA
$jisx0212_j2u{0x2b62} = 0x00fa;	# LATIN SMALL LETTER U WITH ACUTE
$jisx0212_j2u{0x2b63} = 0x00f9;	# LATIN SMALL LETTER U WITH GRAVE
$jisx0212_j2u{0x2b64} = 0x00fc;	# LATIN SMALL LETTER U WITH DIAERESIS
$jisx0212_j2u{0x2b65} = 0x00fb;	# LATIN SMALL LETTER U WITH CIRCUMFLEX
$jisx0212_j2u{0x2b66} = 0x016d;	# LATIN SMALL LETTER U WITH BREVE
$jisx0212_j2u{0x2b67} = 0x01d4;	# LATIN SMALL LETTER U WITH CARON
$jisx0212_j2u{0x2b68} = 0x0171;	# LATIN SMALL LETTER U WITH DOUBLE ACUTE
$jisx0212_j2u{0x2b69} = 0x016b;	# LATIN SMALL LETTER U WITH MACRON
$jisx0212_j2u{0x2b6a} = 0x0173;	# LATIN SMALL LETTER U WITH OGONEK
$jisx0212_j2u{0x2b6b} = 0x016f;	# LATIN SMALL LETTER U WITH RING ABOVE
$jisx0212_j2u{0x2b6c} = 0x0169;	# LATIN SMALL LETTER U WITH TILDE
$jisx0212_j2u{0x2b6d} = 0x01d8;	# LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE
$jisx0212_j2u{0x2b6e} = 0x01dc;	# LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE
$jisx0212_j2u{0x2b6f} = 0x01da;	# LATIN SMALL LETTER U WITH DIAERESIS AND CARON
$jisx0212_j2u{0x2b70} = 0x01d6;	# LATIN SMALL LETTER U WITH DIAERESIS AND MACRON
$jisx0212_j2u{0x2b71} = 0x0175;	# LATIN SMALL LETTER W WITH CIRCUMFLEX
$jisx0212_j2u{0x2b72} = 0x00fd;	# LATIN SMALL LETTER Y WITH ACUTE
$jisx0212_j2u{0x2b73} = 0x00ff;	# LATIN SMALL LETTER Y WITH DIAERESIS
$jisx0212_j2u{0x2b74} = 0x0177;	# LATIN SMALL LETTER Y WITH CIRCUMFLEX
$jisx0212_j2u{0x2b75} = 0x017a;	# LATIN SMALL LETTER Z WITH ACUTE
$jisx0212_j2u{0x2b76} = 0x017e;	# LATIN SMALL LETTER Z WITH CARON
$jisx0212_j2u{0x2b77} = 0x017c;	# LATIN SMALL LETTER Z WITH DOT ABOVE

# 12 row-cells of JIS X 0212:1990 below are unified to JIS X 0208:1997
# by JIS X 0213:2000.
# cf. http://wakaba-web.hp.infoseek.co.jp/0212-0213/jisx0212-0213.ja.html
&unify_jisx0212(0x3031, 0x213a);
&unify_jisx0212(0x3063, 0x4322);
&unify_jisx0212(0x3742, 0x4333);
&unify_jisx0212(0x3c77, 0x5740);
&unify_jisx0212(0x3d58, 0x5765);
&unify_jisx0212(0x4039, 0x5954);
&unify_jisx0212(0x4147, 0x327e);
&unify_jisx0212(0x4323, 0x3341);
&unify_jisx0212(0x4344, 0x5b4f);
&unify_jisx0212(0x4b51, 0x6824);
&unify_jisx0212(0x4d77, 0x4169);
&unify_jisx0212(0x545a, 0x3752);

sub unify_jisx0212 {
local ($x, $y) = @_;
  ($jisx0212_unified{$x}, $jisx0212_j2u{$x}) = ($jisx0212_j2u{$x}, $j2u{$y});
}


##
## Make reversal maps.
##

foreach (keys %j2u) {
  $u2j{$j2u{$_}} = $_;
}
foreach (keys %jisx0208_1978_j2u) {
  $jisx0208_1978_u2j{$jisx0208_1978_j2u{$_}} = $_;
}
foreach (keys %jisx0212_j2u) {
  $jisx0212_u2j{$jisx0212_j2u{$_}} = $_;
}

# Remove maps duplicated between JIS X 0208 extension and JIS X 0212.
delete $u2j{0x2116}; # NUMERO SIGN
delete $jisx0208_1978_u2j{0x2116}; # NUMERO SIGN

# Add JIS X 0212 maps unified with JIS X 0208:1997
foreach (keys %jisx0212_unified) {
  if (defined($jisx0212_u2j{$jisx0212_unified{$_}})) {
    die "Duplicated map: $_ : $jisx0212_unified{$_} :  $jisx0212_u2j{$jisx0212_unified{$_}}";
  } else {
    $jisx0212_u2j{$jisx0212_unified{$_}} = $_;
  }
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
    if (defined $u2j{$compat{$_}}) {
      if (defined $u2j{$_}) {
        warn sprintf("duplicated: %04X -> %04X / %04X", $_, $u2j{$_}, $u2j{$compat{$_}});
      }
      $u2j{$_} = $u2j{$compat{$_}};
    }
    if (defined $jisx0208_1978_u2j{$compat{$_}}) {
      if (defined $jisx0208_1978_u2j{$_}) {
        warn sprintf("duplicated: %04X -> %04X / %04X", $_, $jisx0208_1978_u2j{$_}, $jisx0208_1978_u2j{$compat{$_}});
      }
      $jisx0208_1978_u2j{$_} = $jisx0208_1978_u2j{$compat{$_}};
    }
    $u2j_1978{0xf9} = 1;
    $u2j_1978{0xfa} = 1;

    if (defined $jisx0212_u2j{$compat{$_}}) {
      if (defined $jisx0212_u2j{$_}) {
        warn sprintf("duplicated: %04X -> %04X / %04X", $_, $jisx0212_u2j{$_}, $jisx0212_u2j{$compat{$_}});
      }
      $jisx0212_u2j{$_} = $jisx0212_u2j{$compat{$_}};
    }
  }
}


print <<_HEADER_;
#ifndef _ISO2022JP_HDR_
#define _ISO2022JP_HDR_
/*
 * iso-2022-jp support by Norihisa Washitake <nori\@washitake.com>
 * JIS X 0208:1997 update and JIS X 0212:1990 support
 *  by Hatuka*nezumi - IKEDA Soji <nezumi\@jca.apc.org>
 *  $Id: iso2022jp.pl,v 1.7 2004/02/03 02:00:00 mrsam Exp $
 *
 */

#if (JIS_DEBUG > 0) && defined(JIS_BUILD_APP)
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

/* Definitions from unicode.h */
typedef wchar_t unicode_char;
struct unicode_info {
  const char *chset;
  unicode_char *(*c2u)(const char *, int *);
  char *(*u2c)(const unicode_char *, int *);
  char *(*toupper_func)(const char *, int *);
  char *(*tolower_func)(const char *, int *);
  char *(*totitle_func)(const char *, int *);
};
#else
#include "unicode.h"
#endif /* JIS_BUILD_APP */

/*
 * Some characters are unique in ISO-2022-JP character set,
 * so define them specially.
 */

#define JIS_CHAR_ESC    0x1B
#define JIS_CHAR_SO     0x0E
#define JIS_CHAR_SI     0x0F

/* ISOREG #1/#3: US-ASCII (identical to ISO 646 IRV) */
#define JIS_TYPE_ASCII      0x0
/* ISOREG #14: JIS X 0201:1976/1997 GL (ISO 646 Japanese version) */
#define JIS_TYPE_ROMAN      0x1
/* ISOREG #13: JIS X 0201:1976/1997 GR ("Halfwidth katakana") */
#define JIS_TYPE_7BITKANA   0x2
#define JIS_TYPE_8BITKANA   0x3
/* ISOREG #87/#168: JIS X 0208:1983/1990/1997 */
#define JIS_TYPE_KANJI      0x4
#define JIS_TYPE_JISX0208   0x4
/* ISOREG #42: JIS C 6226:1978 ("78JIS" or "Old JIS") */
#define JIS_TYPE_JISX0208_1978      0x5
/* ISOREG #159: JIS X 0212:1990 ("Supplementary kanzi") */
#define JIS_TYPE_JISX0212   0x6
/* Unknown state */
#define JIS_TYPE_BINARY     0xF

struct jischar_t {
  int type;
  unsigned int value;
};

_HEADER_


# first, j2u. next, u2j.
&j2u_map('jisx0208', 'JIS X 0208:1997', 0, %j2u);
print "#define jis2uni_tbls jisx0208_to_uni_tbls\n\n";
&u2j_map('jisx0208', 'JIS X 0208:1997', 0, %u2j);
print "#define uni2jis_tbls uni_to_jisx0208_tbls\n\n";
&j2u_map('jisx0208_1978', 'JIS C 6226:1978', 1, %jisx0208_1978_j2u);
&u2j_map('jisx0208_1978', 'JIS C 6226:1978', 1, %jisx0208_1978_u2j);
&j2u_map('jisx0212', 'JIS X 0212:1990', 0, %jisx0212_j2u);
&u2j_map('jisx0212', 'JIS X 0212:1990', 0, %jisx0212_u2j);

sub j2u_map {
 local ($name, $setname, $compat_1978, %j2u) = @_;
 local %j2uout;

 print "/* map: $setname to Unicode */\n";
 for ($hb=0x21; $hb<0x7f; $hb++) {
  $items = 0;
  for ($lb=0x21; $lb<0x7f; $lb++) {
    $items++ if ($j2u{$hb*256 + $lb} > 0);
  }
  if ($items > 0) {
    $items = 0;
    if (!$compat_1978 || ($compat_1978 && $j2u_1978{$hb})) { 
     printf "static const unicode_char ${name}_to_uni_tbl_%02x[] = {", $hb;
     for ($lb = 0x21; $lb < 0x7f; $lb++) {
      $real = $hb*256 + $lb;
      print ", " if ($items > 0);
      print "\n  " if ($items % $perline == 0);
      $j2u{$real} = 0x003f if ($j2u{$real} == 0);
      printf("0x%04X", $j2u{$real});
      $items++;
     }
     print "\n};\n";
    }
    $j2uout{$hb} = 1;
  }
 }

 print "const unicode_char * ${name}_to_uni_tbls[] = {\n";
 for ($hb=0x21; $hb<0x7f; $hb++) {
  print (($hb > 0x21) ? ",\n  " : "  ");
  if ($j2uout{$hb} > 0) {
   if (!$compat_1978 || ($compat_1978 && $j2u_1978{$hb})) {
    printf "${name}_to_uni_tbl_%02x", $hb;
   } elsif ($compat_1978) {
    printf "jisx0208_to_uni_tbl_%02x", $hb;
   }
  }else {
    print "NULL";
  }
 }

 print "\n};\n";
 print "\n\n";
}

sub u2j_map {
 local ($name, $setname, $compat_1978, %u2j) = @_;
 local %u2jout;

 print "/* map : Unicode to $setname */\n";
 for ($hb=0x00; $hb<=0xff; $hb++) {
    $items = 0;
    for ($lb=0x0; $lb<=0xff; $lb++) {
        $items++ if ($u2j{$hb*256 + $lb} > 0);
    }
    if ($items > 0) {
        $items = 0;
        if (!$compat_1978 || ($compat_1978 && $u2j_1978{$hb})) {
         printf "static const unsigned uni_to_${name}_tbl_%02x[] = {", $hb;
         for ($lb = 0x00; $lb <= 0xff; $lb++) {
            $real = $hb * 256 + $lb;
            print ", " if ($items > 0);
            print "\n  " if ($items % $perline == 0);
            $u2j{$real} = 0x003f if ($u2j{$real} == 0);
            printf("0x%04X", $u2j{$real});
            $items++;
         }
         print "\n};\n";
        }
        $u2jout{$hb} = 1;
    }
 }

 print "const unsigned * uni_to_${name}_tbls[] = {\n";
 for ($hb = 0x00; $hb <= 0xff; $hb++) {
    print (($hb > 0x00) ? ",\n  " : "  ");
    if ($u2jout{$hb} > 0) {
      if (!$compat_1978 || ($compat_1978 && $u2j_1978{$hb})) {
        printf "uni_to_${name}_tbl_%02x", $hb;
      } elsif ($compat_1978) {
        printf "uni_to_jisx0208_tbl_%02x", $hb
      }
    } else {
        print "NULL";
    }
 }
 print "\n};\n";
}

print "#endif /* _ISO2022JP_HDR_ */\n";

__END__

