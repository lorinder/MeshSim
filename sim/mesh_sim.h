#ifndef MESH_SIM_H
#define MESH_SIM_H

#include <string>

#include "apps_config.h"
#include "apps_manager.h"
#include "routing_config.h"
#include "wifi_config.h"

#include "ns3_all.h"

class MeshSim {
public:

	/**	Process command line arguments */
	bool ProcessCommandLineArgs(int argc, char** argv);

	/**	Load the simulation configuration */
	bool LoadConfig();

	/**	Create the entire sim */
	bool CreateSim();

	/**	Run the simulation.
	 */
	bool Run();
private:
	/** \defgroup SimDirSettings Simulation directories
	 */

	std::string configDir = "conf";
	std::string outDir = "out";

	/** \defgroup SimCfgSettings Simulation configuration settings
	 *  @{
	 */

	/** Total duration of the simulation in seconds */
	double simDuration = 60;

	/** The number of mesh nodes in the system */
	int meshSize = 9;

	/** The number of STAs in the system */
	int staSize = 9;

	/** The cwmin setting for the mesh congestion window */
	int cwmin = 9999;

	/** Mobility helpers to set up mesh node and STA locations */
	ns3::MobilityHelper meshMobilityHelper;
	ns3::MobilityHelper staMobilityHelper;

	/** Configuration of applications */
	AppsCompleteConfig appsCfg;

	/** Configuration of the routing */
	routingConfig routing;

	/** Wifi setup for intra-mesh communication */
	wifiConfig meshWifiConfig;

	/** Wifi setup for AP<->STA communication */
	wifiConfig apStaWifiConfig;

	/** Whether to produce pcap output files */
	bool enablePcap = false;

	/** Whether to use promiscuous mode for PCAPs */
	bool promiscuousMode = false;

	/** Flags related to PCAP generation */
	bool useRadioTap = false;

	/* @} */

	AppsManager appsMgr;

	ns3::Ipv4InterfaceContainer backhaulP2pInterfaces;

	ns3::Ipv4InterfaceContainer meshInterfaces;
	ns3::Ipv4InterfaceContainer apInterfaces;
	ns3::Ipv4InterfaceContainer staInterfaces;
	ns3::Ipv4InterfaceContainer sta2wInterfaces;
	ns3::Ipv4InterfaceContainer wiredStaInterfaces;

	ns3::NetDeviceContainer backhaulP2pDevices;

	ns3::NodeContainer backhaulNodes;
	ns3::PointToPointHelper backhaulP2pHelper;

	ns3::PointToPointHelper wiredStaHelper;

	/** PhyHelper for the mesh */
	ns3::YansWifiPhyHelper meshPhy;

	/** PhyHelper for STA <-> AP communication */
	ns3::YansWifiPhyHelper staPhy;

	/** The Wifi device helper */
	ns3::WifiHelper apStaWifi;

	/** List of Mesh nodes */
	ns3::NodeContainer meshNodes;

	/** Mesh network devices
	 *
	 *  Those are the network interfaces for intra-mesh
	 *  communication.
	 */
	ns3::NetDeviceContainer meshDevices;

	/** AP network devices
	 *
	 *  These devices reside on the mesh nodes and provide the AP
	 *  functionality of the nodes.
	 */
	ns3::NetDeviceContainer apDevices;

	/** List of UE nodes */
	ns3::NodeContainer staNodes;

	/** UE network devices */
	ns3::NetDeviceContainer	staDevices;

	/** For the wired devices. */
	ns3::NodeContainer wiredStaNodes;

	/** STA netdevice to wiredSta */
	ns3::NetDeviceContainer sta2wDevices;

	/** WiredSta netdevices */
	ns3::NetDeviceContainer wiredStaDevices;

	/** List of wired STA nodes */

	/**	\defgroup SimSetup Methods to set up the sim
	 *	@{
	 */

	/**	Create the data channels.
	 */
	bool CreateChannels();

	/**	Create the Mesh nodes and devices.
	 */
	bool CreateMesh();

	/**	Create the station nodes with their devices.
	 */
	void CreateStas();

	/**	Create wired STA devices.
	 */
	void CreateWiredStas();

	/**	Create the backhaul nodes and devices.
	 */
	void CreateBackhaul();

	/**	Create all network interfaces and setup the routing.
	 */
	void CreateInterfaces();

	/**	Install traffic generating apps. */
	bool InstallApps();

	/**	Enable Pcap output (if applicable) */
	void PreparePcap();

	/**	@} */
};

#endif /* MESH_SIM_H */
