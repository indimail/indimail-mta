/*
 * $Log: socket_v4mappedprefix.c,v $
 * Revision 1.2  2005-06-10 12:19:23+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2005-06-10 09:04:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
unsigned char V4mappedprefix[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };
#endif
