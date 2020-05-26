/*
 * $Log: trycpp.c,v $
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
int
main()
{
#ifdef NeXT
	printf("nextstep\n");
	exit(0);
#endif
	printf("unknown\n");
	exit(0);
}
