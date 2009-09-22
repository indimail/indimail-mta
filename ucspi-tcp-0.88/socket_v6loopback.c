/*
 * $Log: socket_v6loopback.c,v $
 * Revision 1.2  2005-06-10 12:19:31+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.1  2005-06-10 09:04:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
const unsigned char V6loopback[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
#endif
