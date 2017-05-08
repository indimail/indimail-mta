/*
 * $Log: ismaildup.c,v $
 * Revision 2.2  2017-03-13 14:03:02+05:30  Cprogrammer
 * use PREFIX for bin programs
 *
 * Revision 2.1  2011-06-30 20:38:13+05:30  Cprogrammer
 * duplicate email eliminator
 *
 */
#include "indimail.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#ifdef HAVE_OPENSSL_EVP_H
#include <openssl/evp.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: ismaildup.c,v 2.2 2017-03-13 14:03:02+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_SSL 
static int
duplicateMD5(char *fileName, char *md5buffer)
{
	FILE           *fp;
	char            tmpbuf[MAX_BUFF], tmpMD5[MAX_BUFF];
	char           *ptr;
	long            pos, savepos, interval;
#ifdef FILE_LOCKING
	int             fd;
#endif
	time_t          curTime, recTime;
	struct tm      *tmptr;

	curTime = time(0);
	if (!(tmptr = localtime(&curTime)))
	{
		fprintf(stderr, "duplicateMD5: localtime: %s\n", strerror(errno));
		return (-1);
	}
#ifdef FILE_LOCKING
	if ((fd = getDbLock(fileName, 1)) == -1)
	{
		fprintf(stderr, "duplicateMD5: getDbLock: %s\n", strerror(errno));
		return (-1);
	}
#endif
	if (access(fileName, F_OK))
		fp = fopen(fileName, "w");
	else
		fp = fopen(fileName, "r+");
	if (!fp)
	{
		perror(fileName);
		return (-1);
	}
	getEnvConfigStr(&ptr, "DUPLICATE_INTERVAL", "900");
	interval = atol(ptr);
	for (recTime = 0, savepos = -1;;)
	{
		pos = ftell(fp);
		if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
			break;
		if (sscanf(tmpbuf, "%ld %s", &recTime, tmpMD5) != 2)
			continue;
		if (savepos == -1 && curTime > recTime + interval)
			savepos = pos;
		if (!memcmp(tmpMD5, md5buffer, 32))
		{
			fclose(fp);
#ifdef FILE_LOCKING
			delDbLock(fd, fileName, 1);
#endif
			return(1);
		}
	}
	if (savepos != -1 && fseek(fp, savepos, SEEK_SET) == -1)
	{
		fclose(fp);
#ifdef FILE_LOCKING
		delDbLock(fd, fileName, 1);
#endif
		return (-1);
	}
	fprintf(fp, "%ld %s\n", curTime, md5buffer);
	fclose(fp);
#ifdef FILE_LOCKING
	delDbLock(fd, fileName, 1);
#endif
	return(0);
}

int
ismaildup(char *maildir)
{
	int             md_len, error, code, wait_status, n, pim[2];
	long unsigned   pid;
	char            buffer[MAX_BUFF], dupfile[MAX_BUFF];
	char          **argv;
	char           *ptr;
	char           *binqqargs[8];
	EVP_MD_CTX      mdctx;
	const EVP_MD   *md;
	unsigned char   md_value[EVP_MAX_MD_SIZE];

	if (pipe(pim) == -1)
		return (0);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		return (0);
	case 0:
		if (lseek(0, 0l, SEEK_SET) < 0)
		{
			fprintf(stderr, "ismaildup: lseek: %s\n", strerror(errno));
			_exit(111);
		}
		close(pim[0]);
		if (dup2(pim[1], 1) == -1)
			_exit(111);
		binqqargs[0] = PREFIX"/bin/822header";
		if ((ptr = getenv("ELIMINATE_DUPS_ARGS")) && !(argv = MakeArgs(ptr)))
		{
			fprintf(stderr, "ismaildup: MakeArgs: %s\n", strerror(errno));
			_exit(111);
		}
		if (ptr)
			execv(*binqqargs, argv);
		else
		{
			binqqargs[1] = "-X";
			binqqargs[2] = "received";
			binqqargs[3] = "-X";
			binqqargs[4] = "delivered-to";
			binqqargs[5] = "-X";
			binqqargs[6] = "x-delivered-to";
			binqqargs[7] = 0;
			execv(*binqqargs, binqqargs);
		}
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	close(pim[1]);
	OpenSSL_add_all_digests();
	if (!(md = EVP_get_digestbyname("md5")))
	{
		fprintf(stderr, "Unknown message digest md5");
		return(0);
	}
	EVP_MD_CTX_init(&mdctx);
	if (!EVP_DigestInit_ex(&mdctx, md, NULL))
	{
		fprintf(stderr, "Digest Initialization failure");
		EVP_MD_CTX_cleanup(&mdctx);
		return(0);
	}
	error = 0;
	for (;;)
	{
		if ((n = read(pim[0], buffer, sizeof(buffer))) == -1)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			close(pim[0]);
			error = 1;
			break;
		} else
		if (!n)
			break;
		/*- Calculate Checksum */
		if (!EVP_DigestUpdate(&mdctx, buffer, n))
		{
			error = 1;
			fprintf(stderr, "Digest Update failure");
			break;
		}
	}
	for (;;)
	{
		if ((pid = wait(&wait_status)) == -1)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			fprintf(stderr, "822header crashed. indimail bug");
			return(0);
		}
		break;
	}
	if (WIFSTOPPED(wait_status) || WIFSIGNALED(wait_status))
	{
		fprintf(stderr, "822header crashed.");
		return(0);
	} else
	if (WIFEXITED(wait_status))
	{
		if ((code = WEXITSTATUS(wait_status)))
		{
			fprintf(stderr, "822header failed code(%d)", code);
			return(0);
		}
	}
	if (!error)
	{
		EVP_DigestFinal_ex(&mdctx, md_value, (unsigned int *) &md_len);
		EVP_MD_CTX_cleanup(&mdctx);
		for (n = 0; n < md_len; n++)
			snprintf(buffer + (2 * n), 3, "%02x", md_value[n]);
		buffer[2 * n] = 0;
		snprintf(dupfile, sizeof(dupfile), "%s/dupmd5", maildir);
		return((n = duplicateMD5(dupfile, buffer)) < 0 ? 0 : n);
	}
	return(0);
}
#endif

#ifdef MAIN
int
main(int argc, char **argv)
{
	int             MsgSize;
#ifdef MAKE_SEEKABLE
	char           *str;
#endif

	if (argc != 3)
	{
		fprintf(stderr, "USAGE: ismaildup directory program\n");
		_exit (100);
	}
#ifdef MAKE_SEEKABLE
	if ((str = getenv("MAKE_SEEKABLE")) && *str != '0' && makeseekable(stdin))
	{
		fprintf(stderr, "makeseekable: %s\n", strerror(errno));
		_exit(111);
	}
#endif
	/*- if we don't know the message size then read it */
	if (!(MsgSize = get_message_size()))
	{
		fprintf(stderr, "Discarding 0 size message ");
		_exit(0);
	}
#ifdef HAVE_SSL 
	if (getenv("ELIMINATE_DUPS") && ismaildup(argv[1]))
	{
		fprintf(stderr, "discarding duplicate msg ");
		_exit (0);
	}
#endif
	if (lseek(0, 0, SEEK_SET) == -1)
	{
		fprintf(stderr, "lseek: %s\n", strerror(errno));
		_exit(111);
	}
	execv(argv[2], argv + 2);
	_exit(111);
}
#endif

void
getversion_ismaildup_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
