#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "mesh_sim.h"
#include "mobility_config.h"
#include "ns3_all.h"
#include "ns3_utils.h"
#include "wifi_config.h"

using namespace ns3;
using namespace std;
namespace filesys = boost::filesystem;

bool MeshSim::ProcessCommandLineArgs(int argc, char** argv)
{
	CommandLine cmd;

	/* Set up arguments */
	cmd.AddValue("simDuration",
		"Duration of the simulation (in sec)", simDuration);

	cmd.AddValue("meshSize",
		"Number of mesh nodes", meshSize);
	cmd.AddValue("staSize",
		"Number of STA nodes", staSize);

	cmd.AddValue("enablePcap",
		"Generate PCAP output files", enablePcap);

	cmd.AddValue("promiscuousMode",
		"Promiscuous mode for PCAPs", promiscuousMode);
	cmd.AddValue("useRadioTap",
		"Enable RadioTap headers in PCAP files", useRadioTap);

	cmd.AddValue("cwmin",
		     "Contention window minimum", cwmin);
	/* Parse */
	cmd.Parse(argc, argv);

	/* Now process the non-option (i.e., positional) args
	 *
	 * We support two non-option args:
	 *  1) configuration directory
	 *  2) output directory.
	 */
	const int nopts = (int)cmd.GetNExtraNonOptions();
	if (nopts > 2) {
		cerr << "Error:  Found extraneous command line arguments\n";
		return false;
	}

	if (nopts >= 2) {
		outDir = cmd.GetExtraNonOption(1);
	}
	if (nopts >= 1) {
		configDir = cmd.GetExtraNonOption(0);
	}

	// Check is outDir exists and is a directory
	filesys::path outPath(outDir);
	if (!filesys::exists(outPath) || !filesys::is_directory(outPath)) {
	    cerr << "Error: out directory does not exists or is not a directory\n";
	    return false;
	}
	return true;
}

bool MeshSim::LoadConfig()
{
	/* Load the mobility configurations */
	if (!configureMobilityHelper(&meshMobilityHelper,
				 configDir + "/mesh_mobility.txt"))
	{
		/* Error already printed */
		return false;
	}
	if (!configureMobilityHelper(&staMobilityHelper,
				 configDir + "/sta_mobility.txt"))
	{
		/* Error already printed */
		return false;
	}
	if (!loadAppsConfig(&appsCfg, configDir + "/apps.txt")) {
		/* Error already printed */
		return false;
	}
	if (!loadRoutingConfig(&routing, configDir + "/routing.txt")) {
		/* Error already printed */
		return false;
	}
	if (!loadWifiConfig(&meshWifiConfig,
				configDir + "/mesh_wifi.txt"))
	{
		/* Error already printed */
		return false;
	}
	if (!loadWifiConfig(&apStaWifiConfig,
				configDir + "/apsta_wifi.txt"))
	{
		/* Error already printed */
		return false;
	}

	return true;
}

bool MeshSim::CreateSim()
{
	if (!CreateChannels())
		return false;

	if (!CreateMesh())
		return false;
	CreateStas();
	CreateBackhaul();
	CreateWiredStas();

	CreateInterfaces();
	if (!InstallApps())
		return false;

	PreparePcap();

	return true;
}

bool MeshSim::Run()
{
	// Flow monitor
	Ptr<FlowMonitor> flowMonitor;
	FlowMonitorHelper flowHelper;
	flowMonitor = flowHelper.InstallAll();

	Simulator::Stop(Seconds(simDuration));
	Simulator::Run();

	flowMonitor->SerializeToXmlFile(outDir + "/flowdata.xml", true, true);

	Simulator::Destroy();
	return true;
}

/*********/


bool MeshSim::CreateChannels()
{
	// Setup meshPhy helper
	meshPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper meshChannelHelper;
	if (!configureWifiChannel(&meshChannelHelper, meshWifiConfig))
		return false;
	meshPhy.SetChannel(meshChannelHelper.Create());

	// Setup staPhy helper
	staPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper staChannelHelper;
	if (!configureWifiChannel(&staChannelHelper, apStaWifiConfig))
		return false;
	staPhy.SetChannel(staChannelHelper.Create());

	// Configure AP<->STA wifi
	if (!configureWifiStdAndRateControl(&apStaWifi, apStaWifiConfig))
		return false;
	if (!configureWifiPhy(&staPhy, apStaWifiConfig))
		return false;

	// Create Point-to-point channel helper
	backhaulP2pHelper.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
	backhaulP2pHelper.SetChannelAttribute("Delay", StringValue("1ms"));

	wiredStaHelper.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
	wiredStaHelper.SetChannelAttribute("Delay", StringValue("0ms"));

	return true;
}

bool MeshSim::CreateMesh()
{
	// Create the Mesh nodes themselves
	meshNodes.Create(meshSize);

	// Set node placement
	meshMobilityHelper.Install(meshNodes);

	// Set up mesh
	MeshHelper meshHelper = MeshHelper::Default();

	if (!configureWifiStdAndRateControl(&meshHelper, meshWifiConfig))
		return false;
	if (!configureWifiPhy(&meshPhy, meshWifiConfig))
		return false;
	meshHelper.SetStackInstaller("ns3::Dot11sStack"); // XXX

	if (cwmin == 9999)
	    meshHelper.SetMacType("RandomStart", TimeValue(Seconds(0.1)));
	else
	    meshHelper.SetMacType("RandomStart", TimeValue(Seconds(0.1)), "CwMin", UintegerValue(cwmin)); // XXX

	meshHelper.SetSpreadInterfaceChannels(MeshHelper::SPREAD_CHANNELS); // XXX
	//meshHelper.SetNumberOfInterfaces(1); // XXX

	meshDevices = meshHelper.Install(meshPhy, meshNodes);

	// Now add the AP devices onto the Mesh nodes.
	WifiMacHelper mac;
	Ssid ssid = Ssid("ns-3-ssid");
	mac.SetType("ns3::ApWifiMac",
		    "Ssid", SsidValue(ssid));
	apDevices = apStaWifi.Install(staPhy, mac, meshNodes);

	return true;
}

void MeshSim::CreateStas()
{
	// Create the nodes
	staNodes.Create(staSize);

	// Set node placement
	staMobilityHelper.Install(staNodes);

	// Create the network devices on the nodes
	WifiMacHelper mac;
	Ssid ssid = Ssid("ns-3-ssid");
	mac.SetType("ns3::StaWifiMac",
		    "Ssid", SsidValue(ssid),
		    "ActiveProbing", BooleanValue(false));
	staDevices = apStaWifi.Install(staPhy, mac, staNodes);
}

void MeshSim::CreateWiredStas()
{
	wiredStaNodes.Create(staSize);
	for (int i = 0; i < (int)wiredStaNodes.GetN(); ++i) {
		Ptr<Node> staNode = staNodes.Get(i);
		Ptr<Node> wiredStaNode = wiredStaNodes.Get(i);
		NetDeviceContainer
		  devs = wiredStaHelper.Install(staNode, wiredStaNode);
		sta2wDevices.Add(devs.Get(0));
		wiredStaDevices.Add(devs.Get(1));
	}
}

void MeshSim::CreateBackhaul()
{
	// Create 1 backhaul node
	backhaulNodes.Create(1);

	// Install network devices
	backhaulP2pDevices
	  = backhaulP2pHelper.Install(backhaulNodes.Get(0), meshNodes.Get(0));
}

static void addInterfacesToMap(
		unordered_map< uint32_t, Ptr<NetDevice> >* addr2netdev_map,
		const Ipv4InterfaceContainer& interfaces)
{
	for (int i = 0; i < (int)interfaces.GetN(); ++i) {
		/* Find the corresponding interface */
		auto entry = interfaces.Get(i);
		Ptr<Ipv4> ipv4 = entry.first;
		uint32_t index = entry.second;
		Ptr<Ipv4Interface> iface =
		  ipv4->GetObject<Ipv4L3Protocol>()->GetInterface(index);

		/* Get the netdevice */
		Ptr<NetDevice> dev = iface->GetDevice();

		/* Get all the addresses */
		for (int j = 0; j < (int)iface->GetNAddresses(); ++j) {
			/* Get address */
			auto ifaddr = iface->GetAddress(j);
			uint32_t addr = ifaddr.GetLocal().Get();

			/* Add into map */
			(*addr2netdev_map)[addr] = dev;
		}
	}
}

void MeshSim::CreateInterfaces()
{
	// Install an internet stack on all the devices.
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add (staticRouting, 0);

	/* Configure dynamic routing */
	switch (routing.proto) {
	case routingConfig::ROUTING_NONE:
		/* No extra routing */
		break;
	case routingConfig::ROUTING_OLSR: {
		OlsrHelper olsrRouting;
		list.Add (olsrRouting, 10);
		break;
	}
	case routingConfig::ROUTING_AODV: {
		AodvHelper aodvRouting;
		list.Add (aodvRouting, 10);
		break;
	}
	}
	InternetStackHelper istackHelper;
	istackHelper.SetRoutingHelper(list);

	istackHelper.Install(backhaulNodes);
	istackHelper.Install(meshNodes);
	istackHelper.Install(staNodes);
	istackHelper.Install(wiredStaNodes);

	// Create the Network interfaces, and assign addresses
	Ipv4AddressHelper addrHelper;
	addrHelper.SetBase("10.1.1.0", "255.255.255.0");
	backhaulP2pInterfaces = addrHelper.Assign(backhaulP2pDevices);

	addrHelper.SetBase("10.1.2.0", "255.255.255.0");
	meshInterfaces = addrHelper.Assign(meshDevices);

	addrHelper.SetBase("10.1.3.0", "255.255.255.0");
	apInterfaces = addrHelper.Assign(apDevices);
	addrHelper.SetBase("10.1.3.0", "255.255.255.0", "0.0.0.101");
	staInterfaces = addrHelper.Assign(staDevices);

	addrHelper.SetBase("10.1.4.0", "255.255.255.0", "0.0.0.101");
	sta2wInterfaces = addrHelper.Assign(sta2wDevices);

	addrHelper.SetBase("10.1.4.0", "255.255.255.0", "0.0.0.1");
	wiredStaInterfaces = addrHelper.Assign(wiredStaDevices);

	// Configure static routing
	unordered_map< uint32_t, Ptr<NetDevice> > addr2netdev;
	addInterfacesToMap(&addr2netdev, backhaulP2pInterfaces);
	addInterfacesToMap(&addr2netdev, meshInterfaces);
	addInterfacesToMap(&addr2netdev, apInterfaces);
	addInterfacesToMap(&addr2netdev, staInterfaces);
	addInterfacesToMap(&addr2netdev, sta2wInterfaces);
	addInterfacesToMap(&addr2netdev, wiredStaInterfaces);

	Ipv4StaticRoutingHelper sr_helper;
	for(auto& a: routing.tables) {
		// Find the ipv4
		Ptr<Node> node = addr2netdev[a.first]->GetNode();
		Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
		Ptr<Ipv4StaticRouting> sr = sr_helper.GetStaticRouting(ipv4);

#define route_args(x)	((x) >> 24) & 0xff, ((x) >> 16) & 0xff, ((x) >> 8) & 0xff, (x) & 0xff
		fprintf(stderr, "Static routes on node %d.%d.%d.%d:\n", route_args(a.first));

		for (const auto& ent: a.second) {
			uint32_t if_index =
			  ipv4->GetInterfaceForAddress(
					Ipv4Address(ent.via_if_ip));
			if (ent.gateway == 0) {
				sr->AddNetworkRouteTo(Ipv4Address(ent.target),
						Ipv4Mask(ent.target_mask),
						if_index,
						ent.metric);
				fprintf(stderr, "               "
				  "%d.%d.%d.%d mask %d.%d.%d.%d via %d\n",
				  route_args(ent.target), route_args(ent.target_mask),
				  (int)if_index);
			} else {
				sr->AddNetworkRouteTo(Ipv4Address(ent.target),
						Ipv4Mask(ent.target_mask),
						Ipv4Address(ent.gateway),
						if_index,
						ent.metric);
				fprintf(stderr, "               "
				  "%d.%d.%d.%d mask %d.%d.%d.%d via %d gateway "
				  "%d.%d.%d.%d\n",
				  route_args(ent.target),
				  route_args(ent.target_mask),
				  (int)if_index,
				  route_args(ent.gateway));
			}
		}
#undef route_args
		Ptr<OutputStreamWrapper>
		  streamwrapper = Create<OutputStreamWrapper>(&cout);

		sr->PrintRoutingTable(streamwrapper, Time::S);
	}
}

bool MeshSim::InstallApps()
{
	/* Compute the map ip_addr -> node */
	unordered_map< uint32_t, Ptr<NetDevice> > addr2netdev;
	addInterfacesToMap(&addr2netdev, backhaulP2pInterfaces);
	addInterfacesToMap(&addr2netdev, meshInterfaces);
	addInterfacesToMap(&addr2netdev, apInterfaces);
	addInterfacesToMap(&addr2netdev, staInterfaces);
	addInterfacesToMap(&addr2netdev, sta2wInterfaces);
	addInterfacesToMap(&addr2netdev, wiredStaInterfaces);

	/* Run over all the apps and install them */
	appsMgr.setOutDir(outDir);
	if (!appsMgr.createApps(appsCfg, addr2netdev)) {
		/* Error already printed */
		return false;
	}
	return true;
}

void MeshSim::PreparePcap()
{
	if (!enablePcap)
		return;

	if (useRadioTap) {
		staPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
		meshPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
	}

	/* Create Pcaps for every IP address */
	struct {
		const char* name_prefix;
		PcapHelperForDevice* pcaphelper;
		ns3::Ipv4InterfaceContainer* ifaces;
	} devtypelist[] = {
		{ "backhaul",	&backhaulP2pHelper,	&backhaulP2pInterfaces },
		{ "mesh",	&meshPhy,		&meshInterfaces },
		{ "ap",		&staPhy,		&apInterfaces },
		{ "sta",	&staPhy,		&staInterfaces },
		{ "sta2w",	&wiredStaHelper,	&sta2wInterfaces },
		{ "wiredsta",	&wiredStaHelper,	&wiredStaInterfaces },
	};

	for (auto& v: devtypelist) {
		unordered_map< uint32_t, Ptr<NetDevice> > addr2dev;
		addInterfacesToMap(&addr2dev, *v.ifaces);

		cout << "name: " << v.name_prefix << "   size: " << addr2dev.size()
		 << '\n';

		for (const auto& addr_dev_pair: addr2dev) {
			auto meshdev = DynamicCast<MeshPointDevice>(addr_dev_pair.second);
			ostringstream fn_prefix;
			fn_prefix << outDir << '/' << v.name_prefix << '-'
			 << ((addr_dev_pair.first >> 24) & 0xff) << '.'
			 << ((addr_dev_pair.first >> 16) & 0xff) << '.'
			 << ((addr_dev_pair.first >>  8) & 0xff) << '.'
			 << ((addr_dev_pair.first >>  0) & 0xff);
			if (!meshdev) {
				// Normal device
				ostringstream fn;
				fn << fn_prefix.str() << ".pcap";
				v.pcaphelper->EnablePcap(fn.str(),	// file name
						addr_dev_pair.second,	// net device
						promiscuousMode,	// promiscuous
						true);			// explicit fn
			} else {
				// Mesh device.
				//
				// We need to go and enumerate the
				// subdevices because the meshdevice
				// itself can't do Pcaps.
				vector< Ptr<NetDevice> >
				  ifaces = meshdev->GetInterfaces();
				int i = 0;
				for (auto j = ifaces.begin(); j != ifaces.end(); ++j)
				{
					ostringstream fn;
					fn << fn_prefix.str() << '-' << i << ".pcap";
					v.pcaphelper->EnablePcap(fn.str(),
							*j,
							promiscuousMode,
							true);
					++i;
				}
			}
		}
	}
}
