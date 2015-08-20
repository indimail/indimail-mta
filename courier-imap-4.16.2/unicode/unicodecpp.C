/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"unicode.h"

extern "C" {

	static int iconv_trampoline(const char *str, size_t cnt, void *arg)
	{
		return reinterpret_cast<mail::iconvert *>(arg)
			->converted(str, cnt);
	}

	int mail::linebreak_trampoline(int value, void *ptr)
	{
		return (*reinterpret_cast<mail::linebreak_callback_base *>
			(ptr))(value);
	}

	int mail::linebreakc_trampoline(int value, unicode_char ch, void *ptr)
	{
		return (*reinterpret_cast<mail::linebreakc_callback_base *>
			(ptr))(value, ch);
	}

	int mail::wordbreak_trampoline(int value, void *ptr)
	{
		return (*reinterpret_cast<mail::wordbreak_callback_base *>
			(ptr))(value != 0);
	}

}

size_t unicode_wcwidth(const std::vector<unicode_char> &uc)
{
	size_t w=0;

	for (std::vector<unicode_char>::const_iterator
		     b(uc.begin()), e(uc.end()); b != e; ++b)
		w += unicode_wcwidth(*b);
	return w;
}

mail::iconvert::iconvert() : handle(NULL)
{
}

mail::iconvert::~iconvert()
{
	end();
}

int mail::iconvert::converted(const char *, size_t)
{
	return 0;
}

bool mail::iconvert::begin(const std::string &src_chset,
			   const std::string &dst_chset)
{
	end();

	if ((handle=libmail_u_convert_init(src_chset.c_str(),
					   dst_chset.c_str(),
					   &iconv_trampoline,
					   this)) == NULL)
		return false;
	return true;
}

bool mail::iconvert::end(bool *errflag)
{
	int errptr;

	int rc;

	if (!handle)
		return true;

	rc=libmail_u_convert_deinit(handle, &errptr);
	handle=NULL;

	if (errflag)
		*errflag=errptr != 0;
	return rc == 0;
}

bool mail::iconvert::operator()(const char *str, size_t cnt)
{
	if (!handle)
		return false;

	return (libmail_u_convert(handle, str, cnt) == 0);
}

bool mail::iconvert::operator()(const unicode_char *str, size_t cnt)
{
	if (!handle)
		return false;

	return (libmail_u_convert_uc(handle, str, cnt) == 0);
}

std::string mail::iconvert::convert(const std::string &text,
				    const std::string &charset,
				    const std::string &dstcharset,
				    bool &errflag)
{
	std::string buf;
	int errptr;

	char *p=libmail_u_convert_tobuf(text.c_str(),
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

int mail::iconvert::tou::converted(const unicode_char *, size_t)
{
	return 0;
}

bool mail::iconvert::tou::begin(const std::string &chset)
{
	return iconvert::begin(chset, libmail_u_ucs4_native);
}

int mail::iconvert::tou::converted(const char *ptr, size_t cnt)
{
	return converted(reinterpret_cast<const unicode_char *>(ptr),
			 cnt/sizeof(unicode_char));
}

void mail::iconvert::tou::convert(const std::string &str,
				  const std::string &chset,
				  std::vector<unicode_char> &out_buf)
{
	convert(str.begin(), str.end(), chset, out_buf);
}

bool mail::iconvert::fromu::begin(const std::string &chset)
{
	return iconvert::begin(libmail_u_ucs4_native, chset);
}

std::string mail::iconvert::fromu::convert(const std::vector<unicode_char>
					   &ubuf,
					   const std::string &chset)
{
	std::string s;

	convert(ubuf, chset, s);
	return s;
}

void mail::iconvert::fromu::convert(const std::vector<unicode_char> &ubuf,
				    const std::string &chset,
				    std::string &out_buf)
{
	convert(ubuf.begin(), ubuf.end(), chset, out_buf);
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

mail::linebreak_callback_base::linebreak_callback_base()
	: handle(NULL), opts(0)
{
}


void mail::linebreak_callback_base::set_opts(int optsArg)
{
	opts=optsArg;

	if (handle)
		unicode_lb_set_opts(handle, opts);
}

mail::linebreak_callback_base::~linebreak_callback_base()
{
	finish();
}

int mail::linebreak_callback_base::operator()(int)
{
	return 0;
}

mail::linebreak_callback_base
&mail::linebreak_callback_base::operator<<(unicode_char uc)
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

void mail::linebreak_callback_base::finish()
{
	if (handle)
		unicode_lb_end(handle);
	handle=NULL;
}


mail::linebreak_callback_save_buf::linebreak_callback_save_buf()
{
}

mail::linebreak_callback_save_buf::~linebreak_callback_save_buf()
{
}

int mail::linebreak_callback_save_buf::operator()(int value)
{
	lb_buf.push_back(value);
	return 0;
}

mail::linebreakc_callback_base::linebreakc_callback_base()
	: handle(NULL), opts(0)
{
}

mail::linebreakc_callback_base::~linebreakc_callback_base()
{
	finish();
}

int mail::linebreakc_callback_base::operator()(int, unicode_char)
{
	return 0;
}

void mail::linebreakc_callback_base::set_opts(int optsArg)
{
	opts=optsArg;

	if (handle)
		unicode_lbc_set_opts(handle, opts);
}

mail::linebreakc_callback_base
&mail::linebreakc_callback_base::operator<<(unicode_char uc)
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

void mail::linebreakc_callback_base::finish()
{
	if (handle)
		unicode_lbc_end(handle);
	handle=NULL;
}


mail::linebreakc_callback_save_buf::linebreakc_callback_save_buf()
{
}

mail::linebreakc_callback_save_buf::~linebreakc_callback_save_buf()
{
}

int mail::linebreakc_callback_save_buf::operator()(int c, unicode_char ch)
{
	lb_buf.push_back(std::make_pair(c, ch));
	return 0;
}

mail::wordbreak_callback_base::wordbreak_callback_base()
	: handle(NULL)
{
}

mail::wordbreak_callback_base::~wordbreak_callback_base()
{
	finish();
}

int mail::wordbreak_callback_base::operator()(bool)
{
	return 0;
}

mail::wordbreak_callback_base
&mail::wordbreak_callback_base::operator<<(unicode_char uc)
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

void mail::wordbreak_callback_base::finish()
{
	if (handle)
		unicode_wb_end(handle);
	handle=NULL;
}

/* -------------------------------------------- */

mail::wordbreakscan::wordbreakscan() : handle(NULL)
{
}

mail::wordbreakscan::~wordbreakscan()
{
	finish();
}

bool mail::wordbreakscan::operator<<(unicode_char uc)
{
	if (!handle)
		handle=unicode_wbscan_init();

	if (handle)
		return unicode_wbscan_next(handle, uc) != 0;

	return false;
}

size_t mail::wordbreakscan::finish()
{
	size_t n=0;

	if (handle)
	{
		n=unicode_wbscan_end(handle);
		handle=NULL;
	}
	return n;
}
