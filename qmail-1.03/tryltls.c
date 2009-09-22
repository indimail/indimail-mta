int
main()
{
#ifdef TLS
	return(0);
#else
	no_function_tls();
	return(1);
#endif
}

