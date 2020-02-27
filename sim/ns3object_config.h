#ifndef NS3OBJECT_CONFIG_H
#define NS3OBJECT_CONFIG_H

#include <string>
#include <vector>

struct ns3objectConfig {
	std::string type_name;
	std::vector< std::string > attribute_assignments;
};

bool readNs3objectConfig(ns3objectConfig*cfg,
			std::vector<std::string>::const_iterator begin_range,
			std::vector<std::string>::const_iterator end_range);

bool readNs3objectConfig(ns3objectConfig* cfg,
			const std::vector<std::string>& tokens);

#endif /* NS3OBJECT_CONFIG_H */
