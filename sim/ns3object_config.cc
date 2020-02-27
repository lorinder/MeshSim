#include <iostream>
#include <iterator>

#include "ns3object_config.h"

using namespace std;

bool readNs3objectConfig(ns3objectConfig* cfg,
			vector<string>::const_iterator begin_range,
			vector<string>::const_iterator end_range)
{
	if (begin_range == end_range) {
		cerr << "Error:  Empty object configuration statement.\n";
		return false;
	}

	// Read type name
	cfg->type_name = *begin_range;

	// Read attribute specs
	copy(begin_range + 1,
		end_range,
		back_inserter(cfg->attribute_assignments));
	return true;
}

bool readNs3objectConfig(ns3objectConfig* cfg,
			const vector<string>& tokens)
{
	return readNs3objectConfig(cfg, tokens.begin(), tokens.end());
}
