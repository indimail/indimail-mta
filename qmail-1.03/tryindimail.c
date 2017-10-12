#ifdef ENABLE_INDIMAIL
int
main()
{
	vclose(0);
	return(0);
}
#else
int
dummy()
{
		return(1);
}
#endif
