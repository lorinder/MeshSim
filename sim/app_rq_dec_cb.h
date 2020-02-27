#include "ns3_all.h"

#include "rq-decoder.h"

#include <cstdio>

class AppRqDecCb {
public:
	AppRqDecCb(FILE* fp_out);
	~AppRqDecCb();

	void rqDecCb(const ns3::RqDecoder::DecodeInfo& I);

private:
	FILE* fp;

	bool has_cached;
	ns3::RqDecoder::DecodeInfo C;
};
