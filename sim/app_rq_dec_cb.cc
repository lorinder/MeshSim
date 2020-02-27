
#include "app_rq_dec_cb.h"

using std::fclose;
using std::fprintf;

AppRqDecCb::AppRqDecCb(FILE* fp_out)
  : fp(fp_out),
    has_cached(false)
{
}

static void print_decinfo(FILE* fp, const ns3::RqDecoder::DecodeInfo& I)
{
	std::fprintf(fp, "%d %d %d %d\n",
		I.sbid,
		(int)I.success,
		I.n_rcv,
		I.n_src_rcv);
}

AppRqDecCb::~AppRqDecCb()
{
	/* Flush out cache if necessary */
	if (has_cached) {
		print_decinfo(fp, C);
		has_cached = false;
	}

	fclose(fp);
}

void AppRqDecCb::rqDecCb(const ns3::RqDecoder::DecodeInfo& I)
{
	/* Check if we need to print a cached input */
	if (has_cached && C.sbid != I.sbid) {
		print_decinfo(fp, C);
		has_cached = false;
	}

	/* Add new reception to cache */
	C = I;
	has_cached = true;
}
