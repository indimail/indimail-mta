/*
** Copyright 2011-2014 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"unicode.h"

extern "C" {

	static int iconv_trampoline(const char *str, size_t cnt, void *arg)
	{
		return reinterpret_cast<unicode::iconvert *>(arg)
			->converted(str, cnt);
	}

	int unicode::linebreak_trampoline(int value, void *ptr)
	{
		return (*reinterpret_cast<unicode::linebreak_callback_base *>
			(ptr)).callback(value);
	}

	int unicode::linebreakc_trampoline(int value, unicode_char ch, void *ptr)
	{
		return (*reinterpret_cast<unicode::linebreakc_callback_base *>
			(ptr)).callback(value, ch);
	}

	int unicode::wordbreak_trampoline(int value, void *ptr)
	{
		return (*reinterpret_cast<unicode::wordbreak_callback_base *>
			(ptr)).callback(value != 0);
	}

}

const char unicode::ucs_4[]=
#if WORDS_BIGENDIAN
	"UCS-4BE"
#else
	"UCS-4LE"
#endif
	;

const char unicode::ucs_2[]=
#if WORDS_BIGENDIAN
	"UCS-2BE"
#else
	"UCS-2LE"
#endif
	;

const char unicode::utf_8[]="utf-8";

const char unicode::iso_8859_1[]="iso-8859-1";

size_t unicode_wcwidth(const std::vector<unicode_char> &uc)
{
	size_t w=0;

	for (std::vector<unicode_char>::const_iterator
		     b(uc.begin()), e(uc.end()); b != e; ++b)
		w += unicode_wcwidth(*b);
	return w;
}

unicode::iconvert::iconvert() : handle(NULL)
{
}

unicode::iconvert::~iconvert()
{
	end();
}

int unicode::iconvert::converted(const char *, size_t)
{
	return 0;
}

bool unicode::iconvert::begin(const std::string &src_chset,
			   const std::string &dst_chset)
{
	end();

	if ((handle=unicode_convert_init(src_chset.c_str(),
					   dst_chset.c_str(),
					   &iconv_trampoline,
					   this)) == NULL)
		return false;
	return true;
}

bool unicode::iconvert::end(bool *errflag)
{
	int errptr;

	int rc;

	if (!handle)
		return true;

	rc=unicode_convert_deinit(handle, &errptr);
	handle=NULL;

	if (errflag)
		*errflag=errptr != 0;
	return rc == 0;
}

bool unicode::iconvert::operator()(const char *str, size_t cnt)
{
	if (!handle)
		return false;

	return (unicode_convert(handle, str, cnt) == 0);
}

bool unicode::iconvert::operator()(const unicode_char *str, size_t cnt)
{
	if (!handle)
		return false;

	return (unicode_convert_uc(handle, str, cnt) == 0);
}

std::string unicode::iconvert::convert(const std::string &text,
				    const std::string &charset,
				    const std::string &dstcharset,
				    bool &errflag)
{
	std::string buf;
	int errptr;

	char *p=unicode_convert_tobuf(text.c_str(),
					charset.c_str(),
					dstcharset.c_str(),
					&errptr);

	errflag= errptr != 0;

	try {
		buf=p;
		free(p);
	} catch (...) {
		free(p);
		throw;
	}

	return buf;
}


std::string unicode::iconvert::convert(const std::vector<unicode_char> &uc,
				    const std::string &dstcharset,
				    bool &errflag)
{
	std::string buf;

	char *c;
	size_t csize;
	int err;

	if (unicode_convert_fromu_tobuf(&uc[0], uc.size(),
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

bool unicode::iconvert::convert(const std::string &text,
			     const std::string &charset,
			     std::vector<unicode_char> &uc)
{
	int err;

	unicode_char *ucbuf;
	size_t ucsize;

	if (unicode_convert_tou_tobuf(text.c_str(),
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

int unicode::iconvert::tou::converted(const unicode_char *, size_t)
{
	return 0;
}

bool unicode::iconvert::tou::begin(const std::string &chset)
{
	return iconvert::begin(chset, unicode_u_ucs4_native);
}

int unicode::iconvert::tou::converted(const char *ptr, size_t cnt)
{
	return converted(reinterpret_cast<const unicode_char *>(ptr),
			 cnt/sizeof(unicode_char));
}

std::pair<std::vector<unicode_char>, bool>
unicode::iconvert::tou::convert(const std::string &str,
				const std::string &chset)
{
	std::pair<std::vector<unicode_char>, bool> ret;

	ret.second=convert(str.begin(), str.end(), chset, ret.first);
	return ret;
}

bool unicode::iconvert::fromu::begin(const std::string &chset)
{
	return iconvert::begin(unicode_u_ucs4_native, chset);
}

std::pair<std::string, bool>
unicode::iconvert::fromu::convert(const std::vector<unicode_char> &ubuf,
				  const std::string &chset)
{
	std::pair<std::string, bool> ret;

	convert(ubuf.begin(), ubuf.end(), chset,
		ret.first, ret.second);

	return ret;
}

std::string unicode::iconvert::convert_tocase(const std::string &text,
					   const std::string &charset,
					   bool &err,
					   unicode_char (*first_char_func)(unicode_char),
					   unicode_char (*char_func)(unicode_char))
{
	err=false;
	std::string s;

	char *p=unicode_convert_tocase(text.c_str(),
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

unicode::linebreak_callback_base::linebreak_callback_base()
	: handle(NULL), opts(0)
{
}


void unicode::linebreak_callback_base::set_opts(int optsArg)
{
	opts=optsArg;

	if (handle)
		unicode_lb_set_opts(handle, opts);
}

unicode::linebreak_callback_base::~linebreak_callback_base()
{
	finish();
}

int unicode::linebreak_callback_base::callback(int ignore)
{
	return 0;
}

unicode::linebreak_callback_base
&unicode::linebreak_callback_base::operator<<(unicode_char uc)
{
	if (!handle)
	{
		handle=unicode_lb_init(linebreak_trampoline,
				       reinterpret_cast<void *>
				       (static_cast<linebreak_callback_base *>
					(this)));
		set_opts(opts);
	}

	if (handle)
		if (unicode_lb_next(handle, uc))
			finish();
	return *this;
}

void unicode::linebreak_callback_base::finish()
{
	if (handle)
		unicode_lb_end(handle);
	handle=NULL;
}


unicode::linebreak_callback_save_buf::linebreak_callback_save_buf()
{
}

unicode::linebreak_callback_save_buf::~linebreak_callback_save_buf()
{
}

int unicode::linebreak_callback_save_buf::callback(int value)
{
	lb_buf.push_back(value);
	return 0;
}

unicode::linebreakc_callback_base::linebreakc_callback_base()
	: handle(NULL), opts(0)
{
}

unicode::linebreakc_callback_base::~linebreakc_callback_base()
{
	finish();
}

int unicode::linebreakc_callback_base::callback(int dummy1, unicode_char dummy2)
{
	return 0;
}

void unicode::linebreakc_callback_base::set_opts(int optsArg)
{
	opts=optsArg;

	if (handle)
		unicode_lbc_set_opts(handle, opts);
}

unicode::linebreakc_callback_base
&unicode::linebreakc_callback_base::operator<<(unicode_char uc)
{
	if (handle == NULL)
	{
		handle=unicode_lbc_init(linebreakc_trampoline,
					reinterpret_cast<void *>
					(static_cast<linebreakc_callback_base *>
					 (this)));
		set_opts(opts);
	}

	if (handle)
		if (unicode_lbc_next(handle, uc))
			finish();
	return *this;
}

void unicode::linebreakc_callback_base::finish()
{
	if (handle)
		unicode_lbc_end(handle);
	handle=NULL;
}


unicode::linebreakc_callback_save_buf::linebreakc_callback_save_buf()
{
}

unicode::linebreakc_callback_save_buf::~linebreakc_callback_save_buf()
{
}

int unicode::linebreakc_callback_save_buf::callback(int c, unicode_char ch)
{
	lb_buf.push_back(std::make_pair(c, ch));
	return 0;
}

unicode::wordbreak_callback_base::wordbreak_callback_base()
	: handle(NULL)
{
}

unicode::wordbreak_callback_base::~wordbreak_callback_base()
{
	finish();
}

int unicode::wordbreak_callback_base::callback(bool ignore)
{
	return 0;
}

unicode::wordbreak_callback_base
&unicode::wordbreak_callback_base::operator<<(unicode_char uc)
{
	if (!handle)
	{
		handle=unicode_wb_init(wordbreak_trampoline,
				       reinterpret_cast<void *>
				       (static_cast<wordbreak_callback_base *>
					(this)));
	}

	if (handle)
		if (unicode_wb_next(handle, uc))
			finish();
	return *this;
}

void unicode::wordbreak_callback_base::finish()
{
	if (handle)
		unicode_wb_end(handle);
	handle=NULL;
}

/* -------------------------------------------- */

unicode::wordbreakscan::wordbreakscan() : handle(NULL)
{
}

unicode::wordbreakscan::~wordbreakscan()
{
	finish();
}

bool unicode::wordbreakscan::operator<<(unicode_char uc)
{
	if (!handle)
		handle=unicode_wbscan_init();

	if (handle)
		return unicode_wbscan_next(handle, uc) != 0;

	return false;
}

size_t unicode::wordbreakscan::finish()
{
	size_t n=0;

	if (handle)
	{
		n=unicode_wbscan_end(handle);
		handle=NULL;
	}
	return n;
}
