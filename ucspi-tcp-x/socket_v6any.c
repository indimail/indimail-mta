/*
 * $Log: socket_v6any.c,v $
 * Revision 1.2  2005-06-10 12:19:26+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2005-06-10 08:29:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
unsigned char V6any[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
