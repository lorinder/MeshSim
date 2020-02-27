#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>

#include "wifi_config.h"
#include "io_utils.h"

using namespace std;

bool loadWifiConfig(wifiConfig* cfg,
	const string& config_file_name)
{
	fstream fp(config_file_name);
	if (!fp) {
		cerr << "Error:  Could not open Wifi config file \""
		  << config_file_name << "\"\n";
		return false;
	}

	vector<string> tokens;
	cfg->default_channel = true;
	while (getconfiglinetokenized(fp, tokens)) {
		if (tokens[0] == "standard") {
			if (tokens.size() != 2) {
				cerr << "Error:  Malformed wifi standard "
				  "line.  Expect \"standard\" <std>.\n";
				return false;
			}
			cfg->wifi_standard = tokens[1];
		} else if (tokens[0] == "ratecontrol") {
			if (!readNs3objectConfig(&cfg->station_manager,
				tokens.begin() + 1, tokens.end()))
			{
				return false;
			}
		} else if (tokens[0] == "phy_attribs") {
			std::copy(tokens.begin() + 1,
					tokens.end(),
					back_inserter(cfg->phy_attribs));
		} else if (tokens[0] == "channel") {
			cfg->default_channel = false;
			if (tokens.size() < 2) {
				cerr << "Error:  Empty channel statement.\n";
				return false;
			}
			if (tokens[1] == "delay") {
				if (!readNs3objectConfig(&cfg->delay_model,
				   tokens.begin() + 2, tokens.end()))
					return false;
			} else if (tokens[1] == "add_loss") {
				ns3objectConfig loss;
				if (!readNs3objectConfig(&loss, tokens.begin() + 2,
				  tokens.end()))
					return false;
				cfg->loss_model.push_back(loss);
			} else {
				cerr << "Error:  Unknown \"" << tokens[1] << "\" "
				     "directive after \"channel\".\n";
				return false;
			}
		} else {
			cerr << "Error:  Wifi config expects a directive each row.\n"
			     << "        Valid directives: \"standard\", "
			        "\"ratecontrol\", \"channel\"\n"
			     << "        Got: \"" << tokens[0] << "\"\n";
			return false;
		}
	}

	return true;
}
