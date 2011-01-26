/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: unicodecpp.C,v 1.4 2011/01/19 03:55:48 mrsam Exp $
*/

#include	"unicode_config.h"
#include	"unicode.h"

extern "C" {

	static int trampoline(const char *str, size_t cnt, void *arg)
	{
		return reinterpret_cast<mail::iconvert *>(arg)
			->converted(str, cnt);
	}
}

mail::iconvert::iconvert() : handle(NULL)
{
}

mail::iconvert::~iconvert()
{
	end();
}

bool mail::iconvert::begin(const std::string &src_chset,
			   const std::string &dst_chset)
{
	end();

	if ((handle=libmail_u_convert_init(src_chset.c_str(),
					   dst_chset.c_str(),
					   &trampoline,
					   this)) == NULL)
		return false;
	return true;
}

bool mail::iconvert::end(int *errptr)
{
	int rc;

	if (!handle)
		return true;

	rc=libmail_u_convert_deinit(handle, errptr);
	handle=NULL;
	return rc == 0;
}

bool mail::iconvert::operator()(const char *str, size_t cnt)
{
	if (!handle)
		return false;

	return (libmail_u_convert(handle, str, cnt) == 0);
}

int mail::iconvert::converted(const char *str, size_t cnt)
{
	return 0;
}


std::string mail::iconvert::convert(const std::string &text,
				    const std::string &charset,
				    const std::string &dstcharset,
				    bool &errflag)
{
	std::string buf;
	int flag;

	char *p=libmail_u_convert_tobuf(text.c_str(),
					charset.c_str(),
					dstcharset.c_str(),
					&flag);

	errflag= flag != 0;

	try {
		buf=p;
		free(p);
	} catch (...) {
		free(p);
		throw;
	}

	return buf;
}


std::string mail::iconvert::convert(const std::vector<unicode_char> &uc,
				    const std::string &dstcharset,
				    bool &errflag)
{
	std::string buf;

	char *c;
	size_t csize;
	int err;

	if (libmail_u_convert_fromu_tobuf(&uc[0], uc.size(),
					  dstcharset.c_str(), &c, &csize,
					  &err))
	{
		err=1;
	}
	else
	{
		if (csize)
			--csize; // Trailing NULL
		try {
			buf.append(c, c+csize);
			free(c);
		} catch (...)
		{
			free(c);
			throw;
		}
	}

	errflag= err != 0;

	return buf;
}

bool mail::iconvert::convert(const std::string &text,
			     const std::string &charset,
			     std::vector<unicode_char> &uc)
{
	int err;

	unicode_char *ucbuf;
	size_t ucsize;

	if (libmail_u_convert_tou_tobuf(text.c_str(),
					text.size(),
					charset.c_str(),
					&ucbuf,
					&ucsize,
					&err))
		return false;

	try {
		uc.clear();
		uc.reserve(ucsize);
		uc.insert(uc.end(), ucbuf, ucbuf+ucsize);
		free(ucbuf);
	} catch (...)
	{
		free(ucbuf);
		throw;
	}

	return err == 0;
}


std::string mail::iconvert::convert_tocase(const std::string &text,
					   const std::string &charset,
					   bool &err,
					   unicode_char (*first_char_func)(unicode_char),
					   unicode_char (*char_func)(unicode_char))
{
	err=false;
	std::string s;

	char *p=libmail_u_convert_tocase(text.c_str(),
					 charset.c_str(),
					 first_char_func,
					 char_func);

	if (!p)
	{
		err=true;
		return s;
	}

	try {
		s=p;
		free(p);
	} catch (...) {
		free(p);
		throw;
	}
	return s;
}
