#! /usr/bin/perl
#
# Compile list of HTML 4.0/XHTML entities.

my %ent=("amp" => 38, "lt" => 60, "gt" => 62);

foreach ("xhtml-lat1.ent", "xhtml-special.ent", "xhtml-symbol.ent")
{
    open(F, "<$_") or die "$_: $!\n";

    my $l;

    while (defined($l=<F>))
    {
	chomp $l;
	next unless $l =~ m/^<!ENTITY\s+([^\s]+)\s+"&#(\d+);">/;

	$ent{$1}=$2;
    }
}

print "static const char n[]={\n";

my $prev="\t";

foreach (sort keys %ent)
{
    my $n=$_;

    my $str="";

    print $prev;
    $prev="";

    foreach (0..length($n)-1)
    {
	$str .= $prev . ord(substr($n, $_, 1));
	$prev=", ";
    }

    print $str;

    $prev=",";

    $prev .= (" " x (40 - length($str)))
	if (length($str) < 40);

    $prev .= " /* $n */\n\t";
}

substr($prev, 0, 1)=" ";

$prev =~ s/\t//;

print "$prev};\n\nstatic const struct i ii[]={";

$prev="\n\t";

my $n=0;

foreach (sort keys %ent)
{
    print $prev . "{$n, " . length($_) . ", $ent{$_}" . "}";

    $n += length($_);
    $prev=",\n\t";
}

print "\n};\n";
