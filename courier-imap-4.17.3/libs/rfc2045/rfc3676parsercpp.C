/*
** Copyright 2011 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"rfc3676parser.h"

extern "C" {

	int mail::tpp_trampoline_line_begin(size_t quote_level, void *arg)
	{
		reinterpret_cast<mail::textplainparser *>(arg)
			->line_begin(quote_level);

		return 0;
	}

	int mail::tpp_trampoline_line_contents(const char32_t *ptr,
					       size_t cnt, void *arg)
	{
		reinterpret_cast<mail::textplainparser *>(arg)
			->line_contents(ptr, cnt);
		return 0;
	}

	int mail::tpp_trampoline_line_flowed_notify(void *arg)
	{
		reinterpret_cast<mail::textplainparser *>(arg)
			->line_flowed_notify();

		return 0;
	}

	int mail::tpp_trampoline_line_end(void *arg)
	{
		reinterpret_cast<mail::textplainparser *>(arg)
			->line_end();
		return 0;
	}
}

mail::textplainparser::textplainparser() : handle(NULL)
{
}

mail::textplainparser::~textplainparser()
{
	end();
}

bool mail::textplainparser::begin(const std::string &charset,
				  bool flowed,
				  bool delsp)
{
	end();

	struct rfc3676_parser_info info=rfc3676_parser_info();

	info.charset=charset.c_str();
	info.isflowed=flowed == true;
	info.isdelsp=delsp == true;

	info.line_begin=&tpp_trampoline_line_begin;
	info.line_contents=&tpp_trampoline_line_contents;
	info.line_flowed_notify=&tpp_trampoline_line_flowed_notify;
	info.line_end=&tpp_trampoline_line_end;

	info.arg=reinterpret_cast<void *>(this);

	if ((handle=rfc3676parser_init(&info)) == NULL)
		return false;

	return true;
}

void mail::textplainparser::end(bool &unicode_errflag)
{
	int rc=0;

	if (handle)
	{
		rfc3676parser_deinit(handle, &rc);
		handle=NULL;
	}

	unicode_errflag=rc != 0;
}

void mail::textplainparser::line_begin(size_t quote_level)
{
	if (quote_level)
	{
		std::vector<char32_t> vec;

		vec.reserve(quote_level+1);
		vec.insert(vec.end(), quote_level, '>');
		vec.push_back(' ');
		line_contents(&vec[0], vec.size());
	}
}

void mail::textplainparser::line_contents(const char32_t *data,
					  size_t cnt)
{
}

void mail::textplainparser::line_flowed_notify()
{
}

void mail::textplainparser::line_end()
{
	char32_t nl='\n';

	line_contents(&nl, 1);
}
