#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

/**	Get the next line in a config file.
 *
 *	This is a version of std::getline that skips over lines starting
 *	with the comment char '#', and which trims the result.
 */
std::istream& getconfigline(std::istream& stream, std::string& line);

void tokenize(std::vector<std::string>& vec_ret, const std::string& line);

/**	Get the next config file line split into tokens */
std::istream& getconfiglinetokenized(std::istream& stream,
				std::vector<std::string>& vec_ret);

/** Parse a string as IP address
 *
 *  Returned IP address is in host order.
 */
bool read_ip_addr(uint32_t& host_ret,
		const std::string& addr);

bool read_ip_network_addr(uint32_t& network_ret,
		uint32_t& netmask_ret,
		const std::string& addr);

#endif /* IO_UTILS_H */
