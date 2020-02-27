#include <algorithm>
#include <cassert>

#include "rq-header.h"

#include "app_rx_cb.h"

using namespace std;
using namespace ns3;

AppRxCb::AppRxCb(FILE* rx_byte_fp_,
		FILE* pl_fp_,
		int pl_window_size)
 : rx_byte_fp(rx_byte_fp_),
   rx_byte_count(0),
   pl_fp(pl_fp_),
   n_pl_heap_max_sz(pl_window_size),
   n_pl_heap_size(0),
   pl_heap(new int64_t[pl_window_size]),
   pl_largest_popped(-1)
{
}

AppRxCb::~AppRxCb()
{
	if (rx_byte_fp)
		fclose(rx_byte_fp);
	if (pl_fp) {
		if (pl_largest_popped > -1) {
			fprintf(pl_fp, "largest_seqno_processed %ld\n",
			  (long int)pl_largest_popped);
		}
		fclose(pl_fp);
	}

	delete[] pl_heap;
}

void AppRxCb::rxCb(Ptr<const Packet> packet,
			const Address &address)
{
	LogAndUpdateByteCounts(packet);
	LogAndUpdateLosses(packet);
}

void AppRxCb::LogAndUpdateByteCounts(Ptr<const Packet> packet)
{
	rx_byte_count += packet->GetSize();
	const uint64_t time_us = Simulator::Now().GetMicroSeconds();
	fprintf(rx_byte_fp, "%ld %ld\n",
		(long int)time_us, (long int)rx_byte_count);
}

void AppRxCb::LogAndUpdateLosses(Ptr<const Packet> packet)
{
	if (pl_fp == NULL)
		return;

	/* Check the header */
	RqHeader rq_hdr;
	if (packet->GetSize() < RqHeader::GetMinSerializedSize())
		return;
	packet->PeekHeader(rq_hdr);
	if (!rq_hdr.IsValid())
		return;

	/* Pop if we need to */
	assert(n_pl_heap_size <= n_pl_heap_max_sz);
	if (n_pl_heap_size == n_pl_heap_max_sz) {
		pop_heap(pl_heap, pl_heap + n_pl_heap_size);
		const int64_t v = -pl_heap[--n_pl_heap_size];

		/* Checks for reordering and duplicates */
		if (v < pl_largest_popped) {
			/* Reordered beyond the window size */
			fprintf(pl_fp, "large_reorder %ld %ld\n",
			  v, pl_largest_popped);
		} else if (v == pl_largest_popped) {
			/* Duplicate packets */
			fprintf(pl_fp, "dup %ld\n", (long int)v);
		} else if (v > pl_largest_popped + 1) {
			/* Lost range */
			fprintf(pl_fp, "lost_range %ld %ld\n",
			  pl_largest_popped + 1, v - 1);
		}

		/* Update pop */
		if (v > pl_largest_popped)
			pl_largest_popped = v;
	}

	/* Push new element to heap */
	const uint64_t seqno = rq_hdr.GetSeqno();
	pl_heap[n_pl_heap_size++] = -seqno;
	push_heap(pl_heap, pl_heap + n_pl_heap_size);
}
