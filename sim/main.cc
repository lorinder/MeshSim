#include <cstdlib>
#include <iostream>

#include "mesh_sim.h"
#include "ns3_all.h"
#include "progress_report.h"

using namespace std;

int main(int argc, char** argv)
{
	/* Command line processing */
	MeshSim sim;
	if (!sim.ProcessCommandLineArgs(argc, argv)) {
		/* Error already printed */
		return EXIT_FAILURE;
	}

	/* Configuration store */
	ns3::ConfigStore config;
	config.ConfigureDefaults();

	/* Load configuration files */
	if (!sim.LoadConfig()) {
		/* Error already printed */
		return EXIT_FAILURE;
	}

	/* Create the topology */
	cout << "Creating the Simulation setup...\n";
	if (!sim.CreateSim())
		return EXIT_FAILURE;

	/* Configure the the attributes */
	config.ConfigureAttributes();

	/* Run the sim */
	cout << "Running the Simulation.\n";
	ProgressReport pr;
	sim.Run();

	return 0;
}
