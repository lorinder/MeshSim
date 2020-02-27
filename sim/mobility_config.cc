#include <fstream>
#include <string>

#include "ns3_all.h"
#include "ns3_utils.h"
#include "io_utils.h"
#include "mobility_config.h"

using namespace ns3;
using namespace std;

bool configureMobilityHelper(MobilityHelper* helper,
			     const string& config_file_name)
{
	/* Open */
	fstream fp(config_file_name);
	if (!fp) {
		cerr << "Error:  Could not open mobility file \""
		  << config_file_name << "\"" << endl;
		return false;
	}

	/* Get the allocator spec */
	vector<string> pos_alloc_spec;
	if (!getconfiglinetokenized(fp, pos_alloc_spec)) {
		cerr << "Error:  Could not read position allocator spec "
		  "in file \"" << config_file_name << "\"" << endl;
		return false;
	}

	/* Construct PositionAllocator */
	Ptr<PositionAllocator> pos_alloc;
	{
		ObjectFactory f;
		f.SetTypeId(pos_alloc_spec[0]);
		pos_alloc = f.Create<PositionAllocator> ();
	}

	/* Set all the attributes */
	for (int i = 1; i < int(pos_alloc_spec.size()); ++i) {
		if (!SetAttributeByAssignmentSpec(pos_alloc, pos_alloc_spec[i])) {
			/* Error already printed */
			return false;
		}
	}

	/* Read positions */
	string line;
	while (getconfigline(fp, line)) {
		vector<string> tok;
		tokenize(tok, line);
		if (tok.size() != 2 && tok.size() != 3) {
			cerr << "Error:  Positions-vector not readable.\n";
			return false;
		}
		Vector3D v;
		if (tok.size() == 2) {
			v = Vector3D{stod(tok[0]), stod(tok[1]), 0};
		} else {
			v = Vector3D{stod(tok[0]), stod(tok[1]), stod(tok[2])};
		}
		DynamicCast<ListPositionAllocator>(pos_alloc)->Add(v);
	}

	/* Assign the position allocator */
	helper->SetPositionAllocator(pos_alloc);

	/* Constant position mobility */
	helper->SetMobilityModel("ns3::ConstantPositionMobilityModel");
	return true;
}
