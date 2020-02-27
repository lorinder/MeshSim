#include <cassert>

#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/double.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/packet-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"

#include "rq-header.h"
#include "rq-encoder.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RqEncoder");

NS_OBJECT_ENSURE_REGISTERED (RqEncoder);

TypeId
RqEncoder::GetTypeId (void)
{
  static bool configured = false;
  static TypeId tid;
  if (!configured) {
    tid = TypeId ("ns3::RqEncoder")
      .SetParent<ProxyBase> ()
      .SetGroupName("Applications")
      .AddConstructor<RqEncoder> ()
      .AddAttribute ("Tval", "The size of packets sent by RQ encoder",
                     UintegerValue (1024),
                     MakeUintegerAccessor (&RqEncoder::m_rqtval),
                     MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("Kval",
                     "The K value (source block size) in number of packets",
                     UintegerValue (100),
                     MakeUintegerAccessor (&RqEncoder::m_rqkval),
                     MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("Nval",
                     "The N value (encoded block size) in number of packets",
                     UintegerValue (120),
                     MakeUintegerAccessor (&RqEncoder::m_rqnval),
                     MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("DataRate",
                     "The sending data rate",
                     DataRateValue (DataRate ("5Mbps")),
                     MakeDataRateAccessor (&RqEncoder::m_sendingRate),
                     MakeDataRateChecker ())
    ;
    ProxyBase::SetTidDefaultProtocols(&tid,
                     TcpSocketFactory::GetTypeId(),
                     UdpSocketFactory::GetTypeId());
    configured = true;
  }
  return tid;
}

RqEncoder::RqEncoder ()
  : m_rqkval(100),
    m_rqnval(120),
    m_rqtval(1024),
    m_nRxSlots(0),
    m_nextRxSlot(0),
    m_sendingRate(DataRate("5Mbps")),
    m_sbid(0),
    m_esi(0),
    m_symbcounts{0}
{
  NS_LOG_FUNCTION (this);
}

RqEncoder::~RqEncoder()
{
  NS_LOG_FUNCTION (this);
}

void RqEncoder::StartApplication()
{
  ProxyBase::StartApplication();

  // Compute m_nRxSlots
  for (m_nRxSlots = 0;
    m_nRxSlots < RX_SLOT_COUNT && !m_rxSlots[m_nRxSlots].addr.IsInvalid();
    ++m_nRxSlots)
  {
  }
  if (m_nRxSlots == 0) {
    NS_FATAL_ERROR("At least one RxSlot needs to be set for the RqEncoder app.");
  }

  // Get the packet sending going
  SendPacket();
}

void RqEncoder::SendPacket()
{
  // Schedule next transmission
  Time nextTime = Seconds(m_rqtval * 8.0 /
                          double(m_sendingRate.GetBitRate()));
  m_sendEvent = Simulator::Schedule(nextTime,
                          &RqEncoder::SendPacket, this);

  // Check if we can send data
  if (m_txSlots[0].sock->GetTxAvailable()
        < m_rqtval + RqHeader::GetMinSerializedSize() + m_nRxSlots * 4)
  {
    /* Don't have the buffer space available to send,
     * so skip this time slot
     */
    return;
  }

  // Retrieve source data.
  Ptr<Packet> pkt;
  int streamid = -1;
  if (m_esi < m_rqkval) {
    const int end = m_nextRxSlot;
    do {
      // Advance
      const int slot = m_nextRxSlot;
      if (++m_nextRxSlot == m_nRxSlots)
        m_nextRxSlot = 0;

      // Check whether we can read from this socket.
      if (m_rxSlots[slot].accepted_socks.empty())
        continue;
      Ptr<Socket> sock = m_rxSlots[slot].accepted_socks.front();
      if (sock->GetRxAvailable() < m_rqtval)
        continue;

      // Read from the socket
      Address from, localAddr;
      sock->GetSockName(localAddr);
      pkt = sock->Recv(m_rqtval, 0);
      m_rxSlots[slot].total += pkt->GetSize();
      m_rxTrace(pkt, from);
      m_rxTraceWithAddresses(pkt, from, localAddr);
      streamid = slot;

      // Send out a dummy packet to socket
      // to work around ns3 TCP flow control bugs
      sock->Send (Create<Packet> (1));

      // Update received symbol counts
      m_symbcounts[slot]++;

      // We created the payload, now we can send it.
      goto packet_created;

    } while (m_nextRxSlot != end);

    // We get here if none of the sockets were readable. In such a case
    // there is nothing to send out at this point.
    return;
  } else {
    // create repair packet
    pkt = Create<Packet> (m_rqtval);
  }

packet_created:
  // Add the RQ header to the payload
  RqHeader hdr(m_rqnval * (uint64_t)m_sbid + m_esi,
                    streamid,
                    m_nRxSlots,
                    m_symbcounts);
  pkt->AddHeader(hdr);

  // Send.
  const int ret = m_txSlots[0].sock->Send(pkt);
  assert(ret == (int)pkt->GetSize());
  m_txSlots[0].total += pkt->GetSize();
  m_txTrace(pkt);
  m_txTraceWithAddresses(pkt,
        m_rxSlots[0].addr,
        m_txSlots[0].addr);

  // Move to next ESI and possibly next source block
  if (++m_esi == m_rqnval) {
    m_esi = 0;
    ++m_sbid;
    std::fill_n(m_symbcounts, RX_SLOT_COUNT, 0);
  }
}

} // Namespace ns3

// vim:set et:sts=2:sw=2
