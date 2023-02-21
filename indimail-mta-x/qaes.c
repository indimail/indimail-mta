/*
 * AES encryption/decryption demo program using OpenSSL EVP apis
 * gcc -Wall openssl_aes.c -lcrypto
 *
 * this is public domain code.
 *
 * Saju Pillai (saju.pillai@gmail.com)
 *
 * $Log: qaes.c,v $
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2017-08-08 23:56:21+05:30  Cprogrammer
 * openssl 1.1.0 port
 *
 * Revision 1.4  2016-01-02 17:45:51+05:30  Cprogrammer
 * fixed usage and reformatted error strings
 *
 * Revision 1.3  2014-01-29 14:01:58+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.2  2013-12-05 18:07:36+05:30  Cprogrammer
 * salt can now be specified on command line
 *
 * Revision 1.1  2013-12-05 17:41:46+05:30  Cprogrammer
 * Initial revision
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <substdio.h>
#include <fmt.h>
#include <str.h>
#include <stralloc.h>
#include <base64.h>
#include <getln.h>
#include <scan.h>
#include <sgetopt.h>
#include <error.h>
#include <strerr.h>
#include <noreturn.h>

#define READ_ERR  1
#define WRITE_ERR 2
#define MEM_ERR   3
#define USAGE_ERR 4
#define AES_BLOCK_SIZE 512

static char     ssinbuf[1024];
static char     ssoutbuf[512];
static char     sserrbuf[512];
static char    *usage = "usage: qaes -k key [ -i -d -e -s salt ]\n";
static char     strnum[FMT_ULONG];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));

void
logerr(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(char *s)
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

no_return void
my_error(char *s1, char *s2, int exit_val)
{
	logerr(s1);
	logerr(": ");
	if (s2) {
		logerr(s2);
		logerr(": ");
	}
	logerr(error_str(errno));
	logerrf("\n");
	_exit(exit_val);
}

void
my_puts(char *s)
{
	if (substdio_puts(&ssout, s) == -1)
		my_error("qaes: write", 0, WRITE_ERR);
}

/* Iterative function to reverse digits of num*/
int reversDigits(int num)
{
	int             rev_num = 0;

	while(num > 0) {
		rev_num = rev_num*10 + num%10;
		num = num/10;
	}
	return rev_num;
}

/*-
 * Create an 256 bit key and IV using the supplied key_data. salt can be added for taste.
 * Fills in the encryption and decryption ctx objects and returns 0 on success
 */
int
aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx)
{
	int             i, nrounds = 5;
	unsigned char   key[32], iv[32];

	/*
	 * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
	 * nrounds is the number of times the we hash the material. More rounds are more secure but
	 * slower.
	 */
	i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, key_data, key_data_len, nrounds, key, iv);
	if (i != 32) {
		logerr("Key size is ");
		strnum[fmt_ulong(strnum, i)] = 0;
		logerr(strnum);
		logerr(" bits - should be 256 bits\n");
		return -1;
	}
	EVP_CIPHER_CTX_init(e_ctx);
	EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
	EVP_CIPHER_CTX_init(d_ctx);
	EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);
	return 0;
}

/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (unsigned char[])
 */
unsigned char  *
aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len)
{
	/*
	 * max ciphertext len for a n bytes of plaintext
	 * is n + AES_BLOCK_SIZE -1 bytes
	 */
	int             c_len = *len + AES_BLOCK_SIZE, f_len = 0;
	unsigned char  *ciphertext = malloc(c_len);

	/*
	 * allows reusing of 'e' for multiple encryption cycles
	 */
	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

	/*
	 * update ciphertext, c_len is filled with the length of ciphertext generated,
	 * *len is the size of plaintext in bytes
	 */
	EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);

	/*
	 * update ciphertext with the final remaining bytes
	 */
	EVP_EncryptFinal_ex(e, ciphertext + c_len, &f_len);
	*len = c_len + f_len;
	return ciphertext;
}

/*
 * Decrypt *len bytes of ciphertext
 */
unsigned char  *
aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len)
{
/*
 * because we have padding ON, we must allocate an extra cipher block size of memory
 */
	int             p_len = *len, f_len = 0;
	unsigned char  *plaintext = malloc(p_len + AES_BLOCK_SIZE);

	EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
	EVP_DecryptFinal_ex(e, plaintext + p_len, &f_len);
	*len = p_len + f_len;
	return plaintext;
}

int
main(int argc, char **argv)
{
	stralloc        user = { 0 }, userout = { 0 }, temp = { 0 };
	char           *plaintext;
	unsigned char  *key_data = 0, *ciphertext;
	unsigned int    salt[] = { 12345, 54321 };
	int             key_data_len = 0, opt, len, encode = 1, match,
					ignore_newline = 0, error_count = 0;
	/*
	 * "opaque" encryption, decryption ctx structures that libcrypto uses to record
	 * status of enc/dec operations
	 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_CIPHER_CTX  *en, *de;
#else
	EVP_CIPHER_CTX  en, de;
#endif

	while ((opt = getopt(argc, argv, "k:s:ide")) != opteof) {
		switch (opt)
		{
		case 'k':
			/*
			 * 8 bytes to salt the key_data during key generation. This is an example of
			 * compiled in salt. We just read the bit pattern created by these two 4 byte
			 * integers on the stack as 64 bits of contiguous salt material -
			 * ofcourse this only works if sizeof(int) >= 4
			 *
			 * the key_data is read from the argument list
			 */
			key_data = (unsigned char *) optarg;
			key_data_len = str_len(optarg);
			break;
		case 's':
			scan_int(optarg, (int *) &salt[0]);
			salt[1] = reversDigits(salt[0]);
			break;
		case 'd':
			encode = 0;
			break;
		case 'e': /*- default */
			encode = 1;
			break;
		case 'i':
			ignore_newline = 1;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	if (!key_data) {
		logerrf("encryption key not specified\n");
		strerr_die1x(100, usage);
	}
	/*
	 * gen key and iv. init the cipher ctx object
	 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	if (!(en = EVP_CIPHER_CTX_new())) {
		logerrf("Couldn't initialize AES cipher\n");
		return -1;
	}
	if (!(de = EVP_CIPHER_CTX_new())) {
		logerrf("Couldn't initialize AES cipher\n");
		return -1;
	}
	if (aes_init(key_data, key_data_len, (unsigned char *) &salt, en, de)) {
#else
	if (aes_init(key_data, key_data_len, (unsigned char *) &salt, &en, &de)) {
#endif
		logerrf("Couldn't initialize AES cipher\n");
		return -1;
	}
	/*
	 * encrypt and decrypt each input string
	 */
	for (opt = -1;;) {
		if (getln(&ssin, &user, &match, '\n') == -1)
			my_error("qaes: read", 0, READ_ERR);
		if (!match && user.len == 0)
			break;
		if (match && ignore_newline)
			user.len--; /*- remove new line */
		if (encode) {
			len = user.len;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
			ciphertext = aes_encrypt(en, (unsigned char *) user.s, &len);
#else
			ciphertext = aes_encrypt(&en, (unsigned char *) user.s, &len);
#endif
			if (len) {
				if (!stralloc_copyb(&temp, (char *) ciphertext, len))
					my_error("qaes: out of memory", 0, MEM_ERR);
				if ((opt = b64encode(&temp, &userout))) {
					logerrf("qaes: base64 encode failed");
					len = 0;
				} else {
					if (substdio_bput(&ssout, userout.s, userout.len) == -1)
						my_error("qaes: write", 0, WRITE_ERR);
					if (substdio_bput(&ssout, "\n", 1))
						my_error("qaes: write", 0, WRITE_ERR);
					if (substdio_flush(&ssout) == -1)
						my_error("qaes: write", 0, WRITE_ERR);
				}
				free(ciphertext);
			}
		} else {
			if ((opt = b64decode((const unsigned char *) user.s, user.len, &userout))) {
				logerrf("qaes: base64 decode failed\n");
				len = 0;
			} else {
				len = userout.len;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
				plaintext = (char *) aes_decrypt(de, (unsigned char *) userout.s, &len);
#else
				plaintext = (char *) aes_decrypt(&de, (unsigned char *) userout.s, &len);
#endif
			}
			if (len) {
				if (substdio_bput(&ssout, plaintext, len) == -1)
					my_error("qaes: write", 0, WRITE_ERR);
				if (substdio_bput(&ssout, "\n", 1))
					my_error("qaes: write", 0, WRITE_ERR);
				if (substdio_flush(&ssout) == -1)
					my_error("qaes: write", 0, WRITE_ERR);
				free(plaintext);
			}
		}
		if (!len) {
			error_count++;
			if (substdio_bput(&ssout, user.s, user.len) == -1)
				my_error("qaes: write", 0, WRITE_ERR);
			if (substdio_bput(&ssout, "qaes: unable to ", 16))
				my_error("qaes: write", 0, WRITE_ERR);
			if (substdio_bput(&ssout, encode ? "encrypt\n" : "decrypt\n", 8))
				my_error("qaes: write", 0, WRITE_ERR);
		}
	}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_CIPHER_CTX_cleanup(en);
	EVP_CIPHER_CTX_cleanup(de);
#else
	EVP_CIPHER_CTX_cleanup(&en);
	EVP_CIPHER_CTX_cleanup(&de);
#endif
	if (substdio_flush(&ssout) == -1) {
		my_error("qaes: write", 0, WRITE_ERR);
	}
	return error_count;
}

void
getversion_qaes_c()
{
	static char    *x = "$Id: qaes.c,v 1.6 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
