#include <iostream>

#include <boost/algorithm/string.hpp>

#include "io_utils.h"

using namespace std;
using namespace boost::algorithm;

istream& getconfigline(istream& stream, string& line_ret)
{
	while (getline(stream, line_ret)) {
		trim(line_ret);
		if (line_ret.empty() || line_ret[0] == '#')
			continue;
		/* Found a line, done. */
		break;
	}

	return stream;
}

void tokenize(vector<string>& vec_ret, const string& line)
{
	vec_ret.clear();
	split(vec_ret, line, is_space(), token_compress_on);
}

istream& getconfiglinetokenized(istream& stream,
				vector<string>& vec_ret)
{
	string line;
	if (!getconfigline(stream, line))
		return stream;
	tokenize(vec_ret, line);
	return stream;
}

bool read_ip_addr(uint32_t& host_ret, const string& addr)
{
	uint32_t mask;
	if (!read_ip_network_addr(host_ret, mask, addr))
		return false;
	if (mask != (uint32_t)-1) {
		cerr << "Error:  Expected host address, but got network "
			"address: \"" << addr << "\"\n";
		return false;
	}
	return true;
}

bool read_ip_network_addr(uint32_t& network_ret,
			uint32_t& netmask_ret,
			const string& addr)
{
	unsigned int a,b,c,d,e=32;
	char dummy;
#define inrange(x) (0 <= (x) && (x) < 256)
	int n = sscanf(addr.c_str(), "%d.%d.%d.%d/%d%c",
			&a, &b, &c, &d, &e, &dummy);
	if ((n != 4 && n != 5) ||
	    !inrange(a) || !inrange(b) || !inrange(c) || !inrange(d)
	    || e < 0 || e > 32)

	{
		cerr << "Error:  Can't parse IP address \"" << addr << "\"\n";
		return false;
	}
#undef inrange

	network_ret = (a << 24) + (b << 16) + (c << 8) + d;
	if (n == 4) {
		netmask_ret = (uint32_t)-1;
	} else {
		if (e == 0)
			/* Need to special case the e==0 case, since
			 * 1<<32 will overflow.
			 */
			netmask_ret = 0;
		else
			netmask_ret = ~((1u << (32 - e)) - 1);
	}
	return true;
}

