#ifndef PROGRESS_REPORT_H
#define PROGRESS_REPORT_H

#include <iostream>

#include "ns3_all.h"

class ProgressReport {
public:
	ProgressReport();

private:
	void Update(void);

	ns3::EventId		event;
	ns3::SystemWallClockMs	walltime;

	/** Update interval, in simulation time */
	double			updateInterval = 1.0;
};

#endif /* PROGRESS_REPORT_H */
