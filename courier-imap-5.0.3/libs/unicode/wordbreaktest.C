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

class collect_wordbreakflags : public unicode::wordbreak_callback_base {

public:

	std::vector<bool> flags;

	template<typename iter_type> void operator()(iter_type b, iter_type e)
	{
		unicode::wordbreak_callback_base::operator()(b, e);
	}

	using unicode::wordbreak_callback_base::operator<<;

private:
	int callback(bool flag)
	{
		flags.push_back(flag);
		return 0;
	}
};

static void testsuite()
{
	std::string buf;
	int linenum=0;

	std::ifstream fp("WordBreakTest.txt");

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
		std::vector<bool> status;

		while (1)
		{
			if (!words.empty() && words.front().size() > 1)
			{
				bool flag=false;
				std::string s=words.front();

				words.pop_front();

				if ((unsigned char)s[0] ==
				    (unsigned char)0xc3 &&
				    (unsigned char)s[1] == (unsigned char)0xb7)
					flag=true;

				if (words.empty())
					break;

				status.push_back(flag);

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

		if (linenum == 24)
		{
			linenum=24;
		}
		collect_wordbreakflags flags;

		flags(ubuf.begin(), ubuf.end());
		flags.finish();

		if (status != flags.flags)
		{
			std::cerr << "Regression, line " << linenum
				  << ": " << buf << std::endl;
			exit(1);
		}
	}
}

int main(int argc, char **argv)
{
	testsuite();
	return 0;
}
