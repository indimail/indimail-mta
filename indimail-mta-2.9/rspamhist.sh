#
# $Log: rspamhist.sh,v $
# Revision 1.1  2004-02-06 22:59:32+05:30  Cprogrammer
# Initial revision
#
#
# @400000004020701c169c4abc bogofilter: pid 1639, X-Bogosity: No, spamicity=0.000000, cutoff=5.11e-01, ham_cutoff=5.00e-01
count=10
hist_length=30
awk -v hist_length=$hist_length -v count=$count '
  /bogofilter:/ {
	if (num = index($7, "="))
		spamicity = substr($7, num + 1, length($7) - 11)
	for (i=1;i < (1 + count);i++)
	{
		spam[i] += 0
		if (spamicity <= i/count)
		{
			spam[i] += 1
			break
		}
	}
	if (spamicity > 1)
		spam[count] += 1
  }
  END {
  	for (ind in spam) {
		total += spam[ind]
	}
  	for (i = 1;i < (1 + count);i++) {
		hashcount = (spam[i] * hist_length)/total
		printf("%0.2f ", i/count)
		for (j = 0;j < hashcount;j++)
			printf("x");
		for (;j < hist_length;j++)
			printf(".");
		printf(" %d\n", spam[i]);
	}
	printf("Total ");
	for (j = 0;j < hist_length;j++)
		printf("#");
	printf(" %d\n", total);
  }
'
