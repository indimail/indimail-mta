
my $unicodes=shift;
my $testing_hook=shift;

my %unicodes;

grep ( $unicodes{lc($_)}=1, split (/,/, $unicodes));

while (<STDIN>)
{
	if ($unicodes eq "" || $unicodes eq "yes")
	{
		print;
		next;
	}

my ($chset, $structname)=split (/[ \t\r\n]/, $_);

	if ($unicodes eq "no")
	{
		next if $structname ne "unicode_ISO8859_1";
	}
	else
	{
		next unless $unicodes{lc($chset)}
			|| $structname eq "unicode_ISO8859_1";
	}
	print;
}

print "X-TEST\tunicode_XTEST\n" if $testing_hook eq "yes";
