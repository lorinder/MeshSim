#include <cstdio>

#include "progress_report.h"

using namespace std;
using namespace ns3;

ProgressReport::ProgressReport()
{
	Simulator::Schedule(Seconds(updateInterval),
		&ProgressReport::Update,
		this);
	walltime.Start();
}

void ProgressReport::Update(void)
{
	double sim_elapsed = Simulator::Now().GetSeconds();
	double wall_elapsed = walltime.GetElapsedReal() / 1000.0;

	/* Print status line */
	printf("+++ Sim time elapsed %6.2f   Wall time elapsed %6.2f  "
	       "(slowdown %5.2fx)\n",
	       sim_elapsed, wall_elapsed, wall_elapsed/sim_elapsed);

	/* Schedule next event */
	Simulator::Schedule(Seconds(1),
		&ProgressReport::Update,
		this);
}
