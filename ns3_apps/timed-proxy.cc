#include <algorithm>
#include <cassert>
#include <iostream>

#include "ns3/log.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"

#include "timed-proxy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TimedProxy");
NS_OBJECT_ENSURE_REGISTERED (TimedProxy);

TypeId
TimedProxy::GetTypeId (void)
{
  static bool configured = false;
  static TypeId tid;
  if (!configured) {
    tid = TypeId("ns3::TimedProxy")
      .SetParent<ProxyBase> ()
      .SetGroupName ("Applications")
      .AddConstructor<TimedProxy> ()
      .AddAttribute ("DataRate",
                    "The sending rate while active.",
                    DataRateValue (DataRate ("5Mbps")),
                    MakeDataRateAccessor (&TimedProxy::m_cbRate),
                    MakeDataRateChecker ())
      .AddAttribute ("SliceWidthMS",
                    "Width of a time slice in MS.",
                    UintegerValue (20),
                    MakeUintegerAccessor (&TimedProxy::m_nSliceWidthMS),
                    MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("SliceTaperMS",
                    "Amount of the time in slice where sending is off.",
                    UintegerValue (2),
                    MakeUintegerAccessor (&TimedProxy::m_nSliceTaperMS),
                    MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("nSlices",
                    "Number of slices per cycle.",
                    UintegerValue (1),
                    MakeUintegerAccessor (&TimedProxy::m_nSlices),
                    MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("ActiveMap",
                    "Bitmap of active slices in this cycle.",
                    UintegerValue (0xffff),
                    MakeUintegerAccessor (&TimedProxy::m_ActiveMap),
                    MakeUintegerChecker<uint32_t> ())
    ;
    ProxyBase::SetTidDefaultProtocols(&tid,
                    UdpSocketFactory::GetTypeId(),
                    UdpSocketFactory::GetTypeId());
    configured = true;
  }
  return tid;
}

TimedProxy::TimedProxy ()
  : m_txEndTime(Time::From(0, Time::MS)),
    m_nSliceWidthMS(20),
    m_nSliceTaperMS(2),
    m_nSlices(1),
    m_ActiveMap(0xffff)
{
}

TimedProxy::~TimedProxy ()
{
}

void TimedProxy::HandleRead(int slotID, Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  socket->GetSockName(localAddress);
  while ((packet = socket->RecvFrom (from)))
    {
      m_rxSlots[slotID].total += (int)packet->GetSize();

      m_rxTrace(packet, from);
      m_rxTraceWithAddresses(packet, from, localAddress);

      if (m_packetStore.empty())
        {
          Time txtime = GetNextTxTime();
          m_sendEv = Simulator::Schedule(txtime - Simulator::Now(),
                        &TimedProxy::SendNextPacket, this);
        }
      m_packetStore.push_back(packet);
    }
}

void TimedProxy::SendNextPacket()
{
  NS_ASSERT(m_sendEv.IsExpired());
  NS_ASSERT(!m_packetStore.empty());

  // Pop the packet
  Ptr<Packet> packet = m_packetStore.front();
  m_packetStore.pop_front();

  // Send it
  m_txSlots[0].sock->Send(packet);
  m_txSlots[0].total += (int)packet->GetSize();

  // Schedule next transmission
  if (!m_packetStore.empty())
    {
      Time nextTime = GetNextTxTime();
      m_sendEv = Simulator::Schedule(nextTime - Simulator::Now(),
                    &TimedProxy::SendNextPacket, this);

      // Update TX end time
      Time duration = Seconds(packet->GetSize() * 8.0 / double(m_cbRate.GetBitRate()));
      m_txEndTime = nextTime + duration;
    }
}

Time TimedProxy::GetNextTxTime()
{
  Time min_t = std::max(m_txEndTime, Simulator::Now());
  int64_t min = min_t.GetMilliSeconds();
  int64_t slot = min / m_nSliceWidthMS;
  const int64_t
    slot_taper = (min + m_nSliceTaperMS) / m_nSliceWidthMS;

  // Check if we can send out at time min.
  if (((1u << (slot % m_nSlices)) & m_ActiveMap) && slot == slot_taper)
    {
      return min_t;
    }

  // Otherwise, need to find the slot time of the next eligible slot
  do
    {
      ++slot;
    }
  while (!((1u << (slot % m_nSlices)) & m_ActiveMap));

  return Time::From(slot * m_nSliceWidthMS, Time::MS);
}

} // Namespace ns3

// vim:sw=2:sts=2:et
