#include <algorithm>
#include <cassert>

#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

#include "rq-header.h"
#include "rq-decoder.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RqDecoder");

NS_OBJECT_ENSURE_REGISTERED (RqDecoder);

TypeId
RqDecoder::GetTypeId (void)
{
  static bool configured = false;
  static TypeId tid;
  if (!configured) {
    tid = TypeId ("ns3::RqDecoder")
      .SetParent<ProxyBase> ()
      .SetGroupName ("Applications")
      .AddConstructor<RqDecoder> ()
      .AddAttribute ("Tval", "The size of packets sent by RQ encoder",
                     UintegerValue (1024),
                     MakeUintegerAccessor (&RqDecoder::m_rqtval),
                     MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("Kval", "The K value (source block size) in number of packets",
                     UintegerValue (100),
                     MakeUintegerAccessor (&RqDecoder::m_rqkval),
                     MakeUintegerChecker<uint32_t> (1))
      .AddAttribute ("Nval", "The N value (encoded block size) in number of packets",
                     UintegerValue (120),
                     MakeUintegerAccessor (&RqDecoder::m_rqnval),
                     MakeUintegerChecker<uint32_t> (1))
      .AddTraceSource ("RqDecodingEvent", "An Rq Decoding event occurred",
                     MakeTraceSourceAccessor(&RqDecoder::m_rqDecodingTrace),
                     "ns3::RqDecoder::RqDecoderCallback")
    ;
    ProxyBase::SetTidDefaultProtocols(&tid,
                     UdpSocketFactory::GetTypeId(),
                     TcpSocketFactory::GetTypeId());
    configured = true;
  }
  return tid;
}

RqDecoder::RqDecoder ()
  : m_rqkval(100),
    m_rqnval(120),
    m_rqtval(1024),
    m_sbid(-1),
    m_sb_nrcv(0),
    m_sb_nsrcrcv(0),
    m_nslots(0), // Need to figure this out when app is started.
    m_slotStates{0}
{
  NS_LOG_FUNCTION (this);
}

RqDecoder::~RqDecoder()
{
  NS_LOG_FUNCTION (this);
}

// Application Methods
void RqDecoder::StartApplication (void)
{
  ProxyBase::StartApplication();

  // Compute m_nslots
  for (m_nslots = 0; m_nslots < TX_SLOT_COUNT; ++m_nslots) {
    if (m_txSlots[m_nslots].addr.IsInvalid())
      break;
  }
}

void RqDecoder::PopulateDecodeInfo(DecodeInfo* I)
{
  I->sbid = m_sbid;
  I->success = (m_sb_nrcv >= m_rqkval);
  I->n_rcv = m_sb_nrcv;
  I->n_src_rcv = m_sb_nsrcrcv;
  for (int i = 0; i < m_nslots; ++i) {
    I->slotinfo[i].n_src_rcv = m_slotStates[i].m_nsymb;
    I->slotinfo[i].n_contig_src_rcv = m_slotStates[i].m_nsymbcontig;
  }
  I->n_slots = m_nslots;
}

void RqDecoder::HandleRead (int slotID, Ptr<Socket> socket)
{
  NS_ASSERT(slotID == 0);

  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      RqHeader rq_hdr;
      if (packet->PeekHeader(rq_hdr) < RqHeader::GetMinSerializedSize()
          || !rq_hdr.IsValid())
      {
        NS_ABORT_MSG("Malformed packet received in RqDecoder.");
      }

      /* Trace */
      Address localAddress;
      socket->GetSockName (localAddress);
      m_rxTrace (packet, from);
      m_rxTraceWithAddresses (packet, from, localAddress);

      /* Decoding logic */
      const int pkt_id = (int)rq_hdr.GetSeqno();
      NS_LOG_INFO("Got packet with sequence number " << pkt_id);

      const int sbid = pkt_id / m_rqnval;
      const int esi = pkt_id % m_rqnval;

      /* Move to subsequent source block? */
      if (sbid > m_sbid) {
          /* Write DecodeInfo trace
           *
           * If we were successful in decoding, a trace call was
           * already issued for the same SB previously.  We
           * send another success message here, because only now we
           * know now how many symbols total were received for
           * this source block. (On the other hand, the trace call
           * here will be missing for the last source block.)
           */
          if (m_sbid >= 0)
            {
              DecodeInfo I;
              PopulateDecodeInfo(&I);
              m_rqDecodingTrace(I);
            }

          /* Update RQ source block id */
          m_sbid = sbid;
          m_sb_nrcv = 0;
          m_sb_nsrcrcv = 0;
          for (int i = 0; i < m_nslots; ++i) {
            m_slotStates[i].m_nsymbcontig = 0;
            m_slotStates[i].m_nsymb = 0;
          }
      } else if (sbid < m_sbid) {
          /* Packet is for already processed sb */
          NS_LOG_INFO ("Got packet for already processed source "
            "block.");
          continue;
      }

      /* Update all the slot states' nsymb counters
       *
       * Once we have >= k packets for the SB, we'll know exactly
       * how many packets for each stream were included in this SB,
       * since in that case, we have at least one packet with
       * ESI >= k - 1, all of which have the maximum and final
       * iseq values for the source block.  The same guarantee
       * can't be made for < k packets.
       */
      const std::vector<int>& iseq = rq_hdr.GetIseqData();
      NS_ASSERT(m_nslots == (int)iseq.size());
      for (int i = 0; i < m_nslots; ++i) {
        if (iseq[i] > m_slotStates[i].m_nsymb)
          m_slotStates[i].m_nsymb = iseq[i];
      }

      /* Update slot state for the stream of the packet */
      const int streamid = rq_hdr.GetIseqStreamID ();
      if (streamid != -1
        && m_slotStates[streamid].m_nsymbcontig + 1
           == m_slotStates[streamid].m_nsymb)
      {
        /* Conceptually, when data is received in order for the
         * stream, just send it out on the corresponding socket
         * immediately.  We can't do that if the data is not
         * contiguous.
         */
        ++m_slotStates[streamid].m_nsymbcontig;
        m_slotStates[streamid].m_nbuffered += m_rqtval;
      }

      /* Update code level counters */
      ++m_sb_nrcv;
      if (esi < m_rqkval) {
          ++m_sb_nsrcrcv;
      }

      /* Check if we have enough to decode now */
      if (m_sb_nrcv == m_rqkval) {
          /* Conceptually, decode and send what we held back
           * on each stream
           */
          for (int i = 0; i < m_nslots; ++i) {
            SlotState* st = &m_slotStates[i];
            st->m_nbuffered
              += (st->m_nsymb - st->m_nsymbcontig) * m_rqtval;
          }

          /* Write DecodeInfo trace */
          DecodeInfo I;
          PopulateDecodeInfo(&I);
          m_rqDecodingTrace(I);
      }
    }

  /* Try to send out some while we're at it */
  for (int i = 0; i < m_nslots; ++i)
    TryToSend(i);
}

void RqDecoder::TryToSend(int slotID)
{
  TxSlot* sl = &m_txSlots[slotID];
  SlotState* ss = &m_slotStates[slotID];

  /* Check if anything can be sent */
  if (!sl->connected)
    return;
  if (ss->m_nbuffered == 0)
    return;

  const uint32_t txavail = sl->sock->GetTxAvailable();
  if (txavail == 0)
    return;

  /* Send */
  const uint32_t amount = std::min(txavail, ss->m_nbuffered);
  Ptr<Packet> pkt = Create<Packet> (amount);
  const uint32_t sent = sl->sock->Send (pkt);
  NS_ASSERT(sent == amount);

  /* Update accounting */
  ss->m_nbuffered -= sent;
  sl->total += sent;
  m_txTrace(pkt);
  // XXX figure out addresses m_txTraceWithAddresses(pkt, a1, a2);
}

void RqDecoder::HandleSend(int slotID,
                        Ptr<Socket> sock,
                        uint32_t max_size)
{
  TryToSend(slotID);
}

} // Namespace ns3

// vim:set et:sts=2:sw=2
