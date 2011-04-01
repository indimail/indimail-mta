#include	"unicode_config.h"
#include	"unicode.h"

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

		std::vector<unicode_char> ubuf;
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

				unicode_char uc;

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

		typedef std::vector<unicode_char>::const_iterator ubuf_iter;
		typedef mail::linebreak_iter<ubuf_iter> lb_iter;

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
			exit(1);
		}
	}
}

static void testlinebreakc()
{
	static unicode_char str[]={'$', '(', 0x0300, 0x0301, 0x0302, 0x0303,
				   0x0304, 0x0305, 0x0306, 0x0307, '1', '.',
				   '2', ' ', 'A'};

	typedef std::vector<std::pair<int, unicode_char> > linebreakvec_t;

	linebreakvec_t linebreakvec;

	std::copy(mail::linebreakc_iter<unicode_char *>(str,
							str + sizeof(str)
							/sizeof(str[0])),
		  mail::linebreakc_iter<unicode_char *>(),
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

	std::vector<unicode_char> uc;

	mail::iconvert::tou
		::convert(convteststr, "utf-8", uc);

	std::vector<unicode_char>::iterator e(uc.end()),
		b(std::find_if(uc.begin(), e,
			       std::not1(std::bind2nd(std::equal_to<unicode_char>
						      (),
						      unicode_char('0')))));

	if (b == e || *b++ != 0x30A2 || b != e)
	{
		std::cerr << "mail::iconvert::tou::convert failed"
			  << std::endl;
		exit(1);
	}

	if (mail::iconvert::fromu::convert(uc, "utf-8") != convteststr)
	{
		std::cerr << "mail::iconvert::fromu::convert failed"
			  << std::endl;
		exit(1);
	}

	return 0;
}
