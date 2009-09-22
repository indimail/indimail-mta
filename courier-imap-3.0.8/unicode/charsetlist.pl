# Copyright 2000-2003 Double Precision, Inc.
# See COPYING for distribution information.
#
# $Id: charsetlist.pl,v 1.2 2003/03/07 00:47:31 mrsam Exp $

my @chset;
my @structname;

print "#include \"unicode_config.h\"\n";
print "#include \"unicode.h\"\n";
while (<>)
{
	chomp;

my ($chset, $structname)=split(/[ \t]+/);

	push @chset, $chset;
	push @structname, $structname;
}

for ($i=0; $i <= $#structname; $i++)
{
	printf("extern const struct unicode_info $structname[$i];\n");
}

print "const struct unicode_chsetlist unicode_chsetlist[] = {\n";

for ($i=0; $i <= $#structname; $i++)
{
	printf("{\"$chset[$i]\", &$structname[$i]},\n");
}

printf("{0,0}};\n");
