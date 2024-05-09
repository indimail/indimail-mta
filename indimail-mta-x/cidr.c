/*
 * $Log: cidr.c,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2018-05-07 17:06:36+05:30  Cprogrammer
 * removed compilation warnings
 *
 * Revision 1.1  2013-09-03 17:05:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define IP_BINARY_LENGTH 32+1	/* 32 bits ipv4 address +1 for null */
#define IP_HEX_LENGTH    10
#define MAX_CIDR_MASK    32
#define MAX_CIDR_LEN     18+1	/*255.255.255.255/32 */

void
printNotation(char *cidrNotation)
{
	printf("%s\n", cidrNotation);
}

/*
 *
 * ipToBin - convert an ipv4 address to binary representation
 *           and pads zeros to the beginning of the string if
 *           the length is not 32
 *           (Important for ranges like 10.10.0.1 - 20.20.20.20 )
 *
 * ip   - ipv4 address on host order
 * pOut - Buffer to store binary.
 *
 * RETURNS: OK or ERROR
 */

int
ipToBin(uint32_t ip, char *pOut)
{
	char            hex[IP_HEX_LENGTH];
	int             i;
	int             result = 0;
	int             len;
	char            pTmp[2];
	int             tmp;
	/*- XXX: Could use bit operations instead but was easier to debug */
	char            binMap[16][5] = {
		"0000", "0001", "0010", "0011", "0100",
		"0101", "0110", "0111", "1000", "1001",
		"1010", "1011", "1100", "1101", "1110", "1111",
	};
	pTmp[1] = 0x0;
	memset(hex, 0x0, sizeof (hex));
	len = sprintf(hex, "%x", ip);

	for (i = 0; i < len; i++) {
		/*- Ugly but to use strtol , we need the last byte as null */
		pTmp[0] = hex[i];
		errno = 0;
		tmp = strtol(pTmp, 0x0, 16);
		/*- Should not happen */
		if (errno != 0) {
			memset(pOut, '0', IP_BINARY_LENGTH - 1);
#if 0
			DEBUG("strtol failed for hex 0x%s\n", pTmp);
#endif
			return -1;
		}
		result += sprintf(pOut + result, "%s", binMap[tmp]);
	}
#if 0
	DEBUG("bits %u printed for ip address for hex len %u\n", result, len);
#endif
	/*- if length is not 32 , pad the start with zeros */
	if (result < IP_BINARY_LENGTH - 1) {
		char            pSwap[IP_BINARY_LENGTH + 1];
		strncpy(pSwap, pOut, IP_BINARY_LENGTH);
		memset(pOut, '0', IP_BINARY_LENGTH + 1);
		strncpy(pOut + IP_BINARY_LENGTH - 1 - result, pSwap, result + 3);
#if 0
		DEBUG("corrected length to 32\n");
#endif
	} else
	if (result > IP_BINARY_LENGTH - 1)
		return -1;
	return 0;
}

/*-
 * rangeToCidr - convert an ip Range to CIDR, and call 'callback' to handle
 * the value.
 *
 * from     - IP Range start address
 * to       - IP Range end address
 * callback - Callback function to handle cidr.
 * RETURNS: OK or ERROR
 */

void
rangeToCidr(uint32_t from, uint32_t to, void (callback) (char *cidrNotation))
{
	int             cidrStart = 0;
	int             cidrEnd = MAX_CIDR_MASK - 1;
	long            newfrom;
	long            mask;
	char            fromIp[IP_BINARY_LENGTH];
	char            toIp[IP_BINARY_LENGTH];
	struct in_addr  addr;
	char            cidrNotation[MAX_CIDR_LEN];

	memset(fromIp, 0x0, sizeof (fromIp));
	memset(toIp, 0x0, sizeof (toIp));
	if (ipToBin(from, fromIp) != 0)
		return;
	if (ipToBin(to, toIp) != 0)
		return;
	if (from < to) {
		/*
		 * Compare the from and to address ranges to get the first
		 * point of difference
		 */
		while (fromIp[cidrStart] == toIp[cidrStart])
			cidrStart++;
		cidrStart = 32 - cidrStart - 1;
		/*
		 * Starting from the found point of difference make all bits on the
		 * right side zero
		 */
		newfrom = from >> (cidrStart + 1) << (cidrStart + 1);
		/*
		 * Starting from the end iterate reverse direction to find
		 * cidrEnd
		 */
		while (fromIp[cidrEnd] == '0' && toIp[cidrEnd] == '1')
			cidrEnd--;
		cidrEnd = MAX_CIDR_MASK - 1 - cidrEnd;
		if (cidrEnd <= cidrStart) {
		/*
		 * Make all the bit-shifted bits equal to 1, for
		 * iteration # 1.
		 */
			mask = pow(2, cidrStart) - 1;
			rangeToCidr(from, newfrom | mask, callback);
			rangeToCidr(newfrom | 1 << cidrStart, to, callback);
		} else {
			addr.s_addr = htonl(newfrom);
			sprintf(cidrNotation, "%s/%d", inet_ntoa(addr), MAX_CIDR_MASK - cidrEnd);
			if (callback != NULL)
				callback(cidrNotation);
		}
	} else {
		addr.s_addr = htonl(from);
		sprintf(cidrNotation, "%s/%d", inet_ntoa(addr), MAX_CIDR_MASK);
		if (callback != NULL)
			callback(cidrNotation);
	}
}

int
main(int argc, char **argv)
{
	long            fromIp, toIp;
	struct in_addr  addr;

	if (argc != 3) {
		printf("Usage: %s <from> <to>\n", argv[0]);
		return (0);
	}
	/*
	 * All operation on host order
	 */
	if (inet_aton(argv[1], &addr) == 0)
		goto error;
	fromIp = ntohl(addr.s_addr);
	if (inet_aton(argv[2], &addr) == 0)
		goto error;
	toIp = ntohl(addr.s_addr);
	rangeToCidr(fromIp, toIp, printNotation);
	return 0;
error:
	printf("Invalid Argument\n");
	return -EINVAL;
}

void
getversion_cidr_c()
{
	const char     *x = "$Id: cidr.c,v 1.3 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";
	if (x)
		x++;
}
