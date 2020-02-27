#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <string>
#include <vector>

#include "ns3object_config.h"

struct wifiConfig {
	// Wifi hardware configuration
	std::string wifi_standard;
	ns3objectConfig station_manager;

	// Wifi PHY configuration
	std::vector< std::string > phy_attribs;

	// Wifi channel configuration
	bool default_channel;
	ns3objectConfig delay_model;
	// As a special hack, the loss_model can be of the pseudo model
	// called "@matrix", which we will then handle specially to
	// create a suitable matrix loss model.
	std::vector<ns3objectConfig> loss_model;
};

bool loadWifiConfig(wifiConfig* cfg,
	const std::string& config_file_name);

#endif /* WIFI_CONFIG_H */
