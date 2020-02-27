#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "ns3_utils.h"

using namespace ns3;
using namespace std;

bool SetWifiStandardByString(WifiPhyStandard* target,
			const string stdString)
{
	if (stdString == "a" || stdString == "802.11a")
		*target = WIFI_PHY_STANDARD_80211a;
	else if (stdString == "b" || stdString == "802.11b")
		*target = WIFI_PHY_STANDARD_80211b;
	else if (stdString == "g" || stdString == "802.11g")
		*target = WIFI_PHY_STANDARD_80211g;
	else if (stdString == "n2.4" || stdString == "802.11n2.4")
		*target = WIFI_PHY_STANDARD_80211n_2_4GHZ;
	else if (stdString == "n5" || stdString == "802.11n5")
		*target = WIFI_PHY_STANDARD_80211n_5GHZ;
	else if (stdString == "ac" || stdString == "802.11ac")
		*target = WIFI_PHY_STANDARD_80211ac;
	else if (stdString == "ax2.4" || stdString == "802.11ax2.4")
		*target = WIFI_PHY_STANDARD_80211ax_2_4GHZ;
	else if (stdString == "ax5" || stdString == "802.11ax5")
		*target = WIFI_PHY_STANDARD_80211ax_5GHZ;
	else {
		cerr << "Error:  Wifi standard \"" << stdString
		     << "\" unknown.\n";
		return false;
	}
	return true;
}

bool ParseAttributeAssignmentSpec(pair<string, string>& kv,
			     const string& assignmentSpec)
{
	string::const_iterator
	  x = find(assignmentSpec.begin(), assignmentSpec.end(), '=');
	if (x == assignmentSpec.end()) {
		cerr << "Error:  Malformed attribute assignment spec \""
		     << assignmentSpec << "\".\n";
		return false;
	}

	kv.first = string(assignmentSpec.begin(), x);
	kv.second = string(x + 1, assignmentSpec.end());
	return true;
}

bool SetAttributeByAssignmentSpec(Ptr<Object> object,
			     const string& assignmentSpec)
{
	pair<string, string> kv;
	if (!ParseAttributeAssignmentSpec(kv, assignmentSpec)) {
		/* Error message already printed */
		return false;
	}

	object->SetAttribute(kv.first, StringValue(kv.second));
	return true;
}

bool configureWifiChannel(YansWifiChannelHelper* helper,
				 const wifiConfig& cfg)
{
	if (cfg.default_channel) {
		*helper = YansWifiChannelHelper::Default();
		return true;
	}

	// Set delay model
	if (cfg.delay_model.type_name == "") {
		helper->SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	} else {
		if (!setObjectFromConfig(helper,
		     &YansWifiChannelHelper::SetPropagationDelay,
		     cfg.delay_model))
		{
			return false;
		}
	}

	// Apply propagation loss models
	for (int i = 0; i < int(cfg.loss_model.size()); ++i) {
		if (cfg.loss_model[i].type_name == "@matrix") {
			// We handle the pseudo-ns3 loss model called
			// "@matrix" separately to create loss models.
			cerr << "Error:  @matrix loss not implemented.\n";
			return false;
		} else {
			if (!setObjectFromConfig(helper,
			     &YansWifiChannelHelper::AddPropagationLoss,
			     cfg.loss_model[i]))
			{
				return false;
			}
		}
	}
	return true;
}

bool configureWifiPhy(ns3::YansWifiPhyHelper* phy,
		      const wifiConfig& cfg)
{
	for (const auto& a : cfg.phy_attribs) {
		if (!SetFactoryAttributeByAssignmentSpec(phy, a)) {
			return false;
		}
	}

	return true;
}
