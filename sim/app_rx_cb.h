#ifndef APP_RX_CB_H
#define APP_RX_CB_H

#include "ns3_all.h"

#include <cstdio>

/** State structure for the rx trace callback */
class AppRxCb {
public:
	AppRxCb(FILE* rx_byte_fp,
		FILE* pl_fp,
		int pl_window_size);
	~AppRxCb();

	void rxCb(ns3::Ptr<const ns3::Packet> packet,
		const ns3::Address& address);
private:
	/*** byte count related members ***/

	void LogAndUpdateByteCounts(ns3::Ptr<const ns3::Packet> packet);

	/* The FILE pointer to write for the RX byte count */
	FILE* rx_byte_fp;

	/* Number of bytes received so far */
	long int rx_byte_count;

	/*** packet loss measure related members ***/

	void LogAndUpdateLosses(ns3::Ptr<const ns3::Packet> packet);

	/* The FILE pointer to write the packet loss data,
	 * or NULL if the stream doesn't capture packet loss
	 * data.
	 */
	FILE* pl_fp;

	/* Data structure note:  Received sequence numbers are inserted
	 * into a heap, and the heap is kept at constant size N by popping
	 * elements whenever the heap reaches the limit.  For the popped
	 * elements, we then check whether they skip sequence numbers.
	 * The effect is that if packets are received out of order by
	 * less than N packets, they count as received, otherwise, they
	 * count as lost.
	 */

	/* Maximum heap size */
	int n_pl_heap_max_sz;

	/* Current heap size */
	int n_pl_heap_size;

	/* The heap itself; contains the sequence numbers _negated_.
	 * The reason is that this results in the heap order that we
	 * want.
	 */
	int64_t* pl_heap;

	/* Last sequence number popped. */
	int64_t pl_largest_popped;

};

#endif /* APP_RX_CB_H */
