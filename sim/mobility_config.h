#ifndef MOBILITY_CONFIG_H
#define MOBILITY_CONFIG_H

#include <string>

#include <ns3/mobility-helper.h>

bool configureMobilityHelper(ns3::MobilityHelper* helper,
			     const std::string& config_file_name);

#endif /* MOBILITY_CONFIG_H */
