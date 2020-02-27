#include <cstdio>
#include <cctype>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>

#include "io_utils.h"
#include "routing_config.h"

using namespace std;

bool loadRoutingConfig(routingConfig* cfg,
			const string& config_file_name)
{
	fstream fp(config_file_name);
	if (!fp) {
		cerr << "Error:  Could not open routing config file \""
		  << config_file_name << "\"\n";
		return false;
	}

	/* Read line by line */
	uint32_t routes_on = 0;
	vector<string> tokens;
	while (getconfiglinetokenized(fp, tokens)) {
		if (tokens[0] == "proto") {
			std::string r = boost::algorithm::to_lower_copy(tokens[1]);
			if (r == "olsr") {
				cfg->proto = routingConfig::ROUTING_OLSR;
			} else if (r == "aodv") {
				cfg->proto = routingConfig::ROUTING_AODV;
			} else if (r == "none") {
				cfg->proto = routingConfig::ROUTING_NONE;
			} else {
				cerr << "Error:  Unknown routing protocol \""
				     << r << "\"\n";
				return false;
			}
		} else if (tokens[0] == "routes_on") {
			if (!read_ip_addr(routes_on, tokens[1]))
				return false;
		} else {
			if (tokens.size() < 2 && tokens.size() > 4) {
				cerr << "Error:  Wrong number of tokens on route spec\n";
				return false;
			}
			routingConfig::rtbl_entry ent;
			if (!read_ip_network_addr(ent.target, ent.target_mask, tokens[0]))
				return false;
			if (!read_ip_addr(ent.via_if_ip, tokens[1]))
				return false;

			ent.gateway = 0;
			ent.metric = 0;
			if (tokens.size() >= 3) {
				if (!read_ip_addr(ent.gateway, tokens[2])) {
					return false;
				}
			}
			if (tokens.size() >= 4) {
				ent.metric = stoi(tokens[3]);
			}

			// Add entry into data structure
			cfg->tables[routes_on].push_back(ent);
		}
	}
	return true;
}
