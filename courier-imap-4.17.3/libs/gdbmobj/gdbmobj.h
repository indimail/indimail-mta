#ifndef	gdbmobj_h
#define	gdbmobj_h

/*
** Copyright 1998 - 2007 Double Precision, Inc.  See COPYING for
** distribution information.
*/


#include	"config.h"
#include	<gdbm.h>

#include	<stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

struct gdbmobj {
	GDBM_FILE	dbf;
	int		has_dbf;
	char	*prev_key;
	size_t	prev_key_len;
	} ;

void gdbmobj_init(struct gdbmobj *);

int gdbmobj_open(struct gdbmobj *, const char *, const char *);
void gdbmobj_close(struct gdbmobj *);

#define	gdbmobj_isopen(p)	(!!(p)->has_dbf)

char	*gdbmobj_fetch(struct gdbmobj *, const char *, size_t, size_t *, const char *);
int	gdbmobj_exists(struct gdbmobj *, const char *, size_t);
int	gdbmobj_delete(struct gdbmobj *, const char *, size_t);
int	gdbmobj_store(struct gdbmobj *, const char *, size_t, const char *,
		size_t, const char *);

char	*gdbmobj_firstkey(struct gdbmobj *, size_t *, char **, size_t *);
char	*gdbmobj_nextkey(struct gdbmobj *, size_t *, char **, size_t *);

#ifdef	__cplusplus
}

#include <string>

class GdbmObj {

	struct gdbmobj obj;

	GdbmObj(const GdbmObj &);			/* Undefined */
	GdbmObj	&operator=(const GdbmObj &);		/* Undefined */

public:
	GdbmObj()	{ gdbmobj_init(&obj); }
	~GdbmObj()	{ gdbmobj_close(&obj); }
	int	Open(std::string filename, const char *mode)
	{
		return ( gdbmobj_open(&obj, filename.c_str(), mode));
	}

	int	IsOpen() { return (gdbmobj_isopen(&obj)); }
	void	Close() { gdbmobj_close(&obj); }

	std::string Fetch(std::string key, std::string mode)
	{
		size_t l;
		char *p=Fetch(key.c_str(), key.size(), l, mode.c_str());

		if (!p) return "";

		std::string v(p, p+l);

		free(p);
		return v;
	}

	bool	Exists(std::string key)
	{
		return !!Exists(key.c_str(), key.size());
	}

	bool	Delete(std::string key)
	{
		return !!Delete(key.c_str(), key.size());
	}

	int	Store(std::string key, std::string val, std::string mode)
	{
		return Store(key.c_str(), key.size(),
			     val.c_str(), val.size(), mode.c_str());
	}

	std::string FetchFirstKeyVal(std::string &valRet)
	{
		char *key;
		size_t keyLen;
		char *val;
		size_t valLen;

		key=FetchFirstKeyVal(keyLen, val, valLen);

		if (!key)
			return "";

		std::string r(key, key+keyLen);

		valRet=std::string(val, val+valLen);
		free(val);
		return r;
	}

	std::string FetchNextKeyVal(std::string &valRet)
	{
		char *key;
		size_t keyLen;
		char *val;
		size_t valLen;

		key=FetchNextKeyVal(keyLen, val, valLen);

		if (!key)
			return "";

		std::string r(key, key+keyLen);

		valRet=std::string(val, val+valLen);
		free(val);
		return r;
	}


	char	*Fetch(const char *key, size_t keylen,
			size_t &valuelen, const char *mode)
		{
			return (gdbmobj_fetch(&obj, key, keylen,
				&valuelen, mode));
		}

	int	Exists(const char *key, size_t keylen)
		{
			return (gdbmobj_exists(&obj, key, keylen));
		}

	int	Delete(const char *key, size_t keylen)
		{
			return (gdbmobj_delete(&obj, key, keylen));
		}

	int	Store(const char *key, size_t keylen,
			const char *value, size_t valuelen,
			const char *mode)
		{
			return (gdbmobj_store(&obj, key, keylen, value,
				valuelen, mode));
		}

	char	*FetchFirstKeyVal(size_t &keylen, char *&val, size_t &vallen)
		{
			return (gdbmobj_firstkey(&obj, &keylen, &val, &vallen));
		}
	char	*FetchNextKeyVal(size_t &keylen, char *&val, size_t &vallen)
		{
			return (gdbmobj_nextkey(&obj, &keylen, &val, &vallen));
		}
} ;

#endif

#endif
