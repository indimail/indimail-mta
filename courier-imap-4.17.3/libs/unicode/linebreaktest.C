#include	"unicode_config.h"
#include	"courier-unicode.h"

#include	<iostream>
#include	<fstream>
#include	<sstream>
#include	<iomanip>
#include	<algorithm>
#include	<functional>
#include	<cstdlib>
#include	<list>
#include	<vector>

static void testsuite()
{
	std::string buf;
	int linenum=0;

	int flag=0;

	std::ifstream fp("LineBreakTest.txt");

	if (!fp.is_open())
		exit(1);

	while (1)
	{
		buf.clear();

		if (std::getline(fp, buf).eof() && buf.empty())
			break;

		++linenum;

		buf.erase(std::find(buf.begin(), buf.end(), '#'), buf.end());

		if (buf.empty())
			continue;

		std::list<std::string> words;

		for (std::string::iterator b=buf.begin(), e=buf.end(); b != e;)
		{
			if (isspace(*b))
			{
				++b;
				continue;
			}

			std::string::iterator p=b;

			while (b != e)
			{
				if (isspace(*b))
					break;
				++b;
			}

			words.push_back(std::string(p, b));
		}

		std::u32string ubuf;
		std::vector<int> status;

		while (1)
		{
			if (!words.empty() && words.front().size() > 1)
			{
				int expected=UNICODE_LB_MANDATORY;

				std::string s=words.front();

				words.pop_front();

				if ((unsigned char)s[0] ==
				    (unsigned char)0xc3)
					switch ( (unsigned char)s[1] ) {
					case (unsigned char)0x97:
						expected=UNICODE_LB_NONE;
						break;
					case (unsigned char)0xb7:
						expected=UNICODE_LB_ALLOWED;
						break;
					}

				if (words.empty())
					break;

				status.push_back(expected);

				std::istringstream i(words.front());

				uint32_t uc;

				i >> std::hex >> uc;

				words.pop_front();

				if (!i.fail())
				{
					ubuf.push_back(uc);
					continue;
				}
			}

			std::cerr << "Parse error, line " << linenum
				  << ": " << buf << std::endl;
			exit(1);
		}

		std::vector<int> computed_status;

		typedef std::u32string::const_iterator ubuf_iter;
		typedef unicode::linebreak_iter<ubuf_iter> lb_iter;

		std::copy(lb_iter(ubuf.begin(), ubuf.end()), lb_iter(),
			  std::back_insert_iterator<std::vector<int> >
			  (computed_status));

		std::replace(computed_status.begin(),
			     computed_status.end(),
			     UNICODE_LB_MANDATORY,
			     UNICODE_LB_ALLOWED);

		if (computed_status != status)
		{
			std::cerr << "Regression, line " << linenum
				  << ": " << buf << std::endl;
			flag=1;
		}
	}

	if (flag)
	{
		exit(1);
	}
}

static void testlinebreakc()
{
	static char32_t str[]={'$', '(', 0x0300, 0x0301, 0x0302, 0x0303,
				   0x0304, 0x0305, 0x0306, 0x0307, '1', '.',
				   '2', ' ', 'A'};

	typedef std::vector<std::pair<int, char32_t> > linebreakvec_t;

	linebreakvec_t linebreakvec;

	std::copy(unicode::linebreakc_iter<char32_t *>(str,
							str + sizeof(str)
							/sizeof(str[0])),
		  unicode::linebreakc_iter<char32_t *>(),
		  std::back_insert_iterator<linebreakvec_t>
		  (linebreakvec));

	if (linebreakvec.size() == sizeof(str)/sizeof(str[0]))
	{
		size_t i;

		for (i=0; i<sizeof(str)/sizeof(str[0]); ++i)
		{
			if (str[i] != linebreakvec[i].second ||
			    linebreakvec[i].first !=
			    (i < sizeof(str)/sizeof(str[0])-1
			     ? UNICODE_LB_NONE:UNICODE_LB_ALLOWED))
				break;
		}

		if (i == sizeof(str)/sizeof(str[0]))
			return;
	}
	std::cerr << "Line break test 1 failed" << std::endl;
}

int main(int argc, char **argv)
{
	testsuite();
	testlinebreakc();

	std::string convteststr="0000000000000000000000000000000\xe3\x82\xa2";

	std::pair<std::u32string, bool> uc;

	uc=unicode::iconvert::tou::convert(convteststr, "utf-8");

	if (uc.second)
	{
		std::cerr << "Valid UTF-8 string is invalid" << std::endl;
		exit(1);
	}

	std::u32string::iterator e(uc.first.end()),
		b(std::find_if(uc.first.begin(), e,
			       std::not1(std::bind2nd(std::equal_to<char32_t>
						      (),
						      char32_t('0')))));

	if (b == e || *b++ != 0x30A2 || b != e)
	{
		std::cerr << "unicode::iconvert::tou::convert failed"
			  << std::endl;
		exit(1);
	}

	std::pair<std::string, bool>
		ret=unicode::iconvert::fromu::convert(uc.first, "utf-8");

	if (ret.first != convteststr || ret.second)
	{
		std::cerr << "unicode::iconvert::fromu::convert failed (1)"
			  << std::endl;
		exit(1);
	}

	uc.first.clear();
	uc.first.push_back(0x30A2);

	if (!unicode::iconvert::fromu::convert(uc.first, "iso-8859-1")
	    .second)
	{
		std::cerr << "unicode::iconvert::fromu::convert failed (2)"
			  << std::endl;
		exit(1);
	}

	uc.first[0]=160;

	if (unicode::iconvert::fromu::convert(uc.first, "iso-8859-1")
	    .second)
	{
		std::cerr << "unicode::iconvert::fromu::convert failed (3)"
			  << std::endl;
		exit(1);
	}


	uc=unicode::iconvert::tou::convert("\xE3", "utf-8");

	if (!uc.second)
	{
		std::cerr << "Invalid UTF-8 string is valid" << std::endl;
		exit(1);
	}
	return 0;
}
