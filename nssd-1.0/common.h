#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <unistd.h>
#include <syslog.h>
#include <stdio.h>

#ifndef HAVE_STRERROR_R
#define strerror_r(n,b,s) strerror(n)
#endif

#define INTERFACE_VERSION 3

#define XFREE(n) { if (n) { free (n); (n) = '\0'; } }

#define RDS (sizeof (struct response_data))

#define NSVS_MAX(x,y) ( (x) > (y) ? (x) : (y) )

typedef enum {
	GETPWBYNAME,
	GETPWBYUID,
	GETPW,
	GETGRBYNAME,
	GETGRBYGID,
	GETGR,
	GETGRMEMSBYGID,
	GETGRGIDSBYMEM,
	GETSPBYNAME,
	GETSP,
	NUM_REQUEST_TYPES
} request_types;

typedef struct {
	int32_t         version;	/* Version number of the daemon interface */
	int32_t         type;		/* enum request_types */
	int32_t         key_len;	/* Key length */
} request_header_t;

/*
 * ANY CHANGES TO THESE REQUIRES INTERFACE_VERSION CHANGE 
 */
/*
 * Password 
 */
typedef enum {
	ROW_PW_NAME,				/* String */
	ROW_PW_PASSWD,				/* String */
	ROW_PW_UID,					/* Number */
	ROW_PW_GID,					/* Number */
#if defined (sun)
	ROW_PW_AGE,					/* String */
	ROW_PW_COMMENT,				/* String */
#endif
#if defined(__FreeBSD__)
	ROW_PW_CHANGE,				/* Number */
	ROW_PW_CLASS,				/* String */
#endif
	ROW_PW_GECOS,				/* String */
	ROW_PW_DIR,					/* String */
	ROW_PW_SHELL,				/* String */
#if defined(__FreeBSD__)
	ROW_PW_EXPIRE,				/* Number */
#endif
	NUM_PW_ELEMENTS
} pw_rows;

/*
 * ANY CHANGES TO THESE REQUIRES INTERFACE_VERSION CHANGE 
 */
/*
 * Shadow 
 */
typedef enum {
	ROW_SP_NAMP,
	ROW_SP_PWDP,
	ROW_SP_LSTCHG,
	ROW_SP_MIN,
	ROW_SP_MAX,
	ROW_SP_WARN,
	ROW_SP_INACT,
	ROW_SP_EXPIRE,
	ROW_SP_FLAG,
	NUM_SP_ELEMENTS
} sp_rows;

/*
 * ANY CHANGES TO THESE REQUIRES INTERFACE_VERSION CHANGE 
 */
/*
 * Group 
 */
typedef enum {
	ROW_GR_NAME,
	ROW_GR_PASSWD,
	ROW_GR_GID,
	ROW_GR_MEM,
	NUM_GR_ELEMENTS
} gr_rows;

/*
 * ANY CHANGES TO THIS REQUIRES INTERFACE_VERSION CHANGE 
 */
#define MAX_ELEMENTS NSVS_MAX (NUM_PW_ELEMENTS, NSVS_MAX (NUM_SP_ELEMENTS, \
                                                          NUM_GR_ELEMENTS))

/*
 * ANY CHANGES TO THESE REQUIRES INTERFACE_VERSION CHANGE 
 */
typedef struct {
	int32_t         version;
	int32_t         count;		/* Number of entries found */
	int32_t         response_size;	/* Size of response following this header */
} response_header_t;

/*
 * ANY CHANGES TO THESE REQUIRES INTERFACE_VERSION CHANGE 
 */
typedef struct {
	int32_t         version;
	int32_t         record_size;	/* Size of this record (hdr + data) */
	int32_t         offsets[MAX_ELEMENTS];	/* byte location of each string */
} data_header_t;

/*
 * ANY CHANGES TO THESE REQUIRES INTERFACE_VERSION CHANGE 
 */
struct response_data {
	data_header_t   header;
	char            strdata[0];
};

/*
 * Persistent/static *ent data 
 */
typedef struct {
	response_header_t response_header;
	struct response_data *data;
	struct response_data *dp;
	int32_t         cur_rec;
} ent_t;

/*
 * util/util.c 
 */
ssize_t         write_wt(int fd, const void *buf, size_t count, int timeout);
ssize_t         read_wt(int fd, void *buf, size_t count, int timeout);
void            getEnvConfigStr(char **, char *, char *);
