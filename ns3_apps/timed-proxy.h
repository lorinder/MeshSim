#ifndef TIMED_PROXY_H
#define TIMED_PROXY_H

#include <deque>

#include "ns3/data-rate.h"

#include "proxy-base.h"

namespace ns3 {

class TimedProxy : public ProxyBase
{
public:
  static TypeId GetTypeId (void);

  TimedProxy ();
  virtual ~TimedProxy ();

protected:
  virtual void HandleRead(int slotID, Ptr<Socket> sock);

  void SendNextPacket();

  // Compute the next possible transmission opportunity.
  Time GetNextTxTime();

  // The current send event
  EventId m_sendEv;

  // Time when current transmission ends
  Time m_txEndTime;

  // Packet buffer
  // Contains packets that have been received but not yet forwarded.
  std::deque< Ptr<Packet> > m_packetStore;

  // Sending rate while active.
  DataRate m_cbRate;

  // Width of time slices in milliseconds
  uint32_t m_nSliceWidthMS;

  // Tapering off time of slice
  uint32_t m_nSliceTaperMS;

  // Count of the number of slices per cycle
  uint32_t m_nSlices;

  // Bitmap of which slices are used to send
  uint32_t m_ActiveMap;
};

} // namespace ns3

#endif /* TIMED_PROXY_H */

// vim:sw=2:sts=2:et
