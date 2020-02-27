#ifndef ROUTING_CONFIG_H
#define ROUTING_CONFIG_H

#include <cstdint>

#include <unordered_map>
#include <vector>
#include <string>

struct routingConfig {
	enum RoutingProtocol {
		ROUTING_NONE,	//< Don't use any routing other than baseline static
		ROUTING_OLSR,	//< Optimized Link State Routing
		ROUTING_AODV,	//< Ad-Hoc On-Demand Distance Vector
	};

	/** The dynamic routing protocol */
	RoutingProtocol proto = ROUTING_OLSR;

	/** Static routing info */
	struct rtbl_entry {
		uint32_t target;
		uint32_t target_mask;

		uint32_t via_if_ip;
		uint32_t gateway;

		int metric;
	};

	/** The entire set of routing tables */
	std::unordered_map< uint32_t, std::vector<rtbl_entry> > tables;
};

bool loadRoutingConfig(routingConfig* cfg,
			const std::string& config_file_name);

#endif /* ROUTING_CONFIG_H */
