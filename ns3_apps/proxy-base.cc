#include <sstream>

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

#include "proxy-base.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ProxyBase");

NS_OBJECT_ENSURE_REGISTERED (ProxyBase);

TypeId
ProxyBase::GetTypeId (void)
{
  static bool configured = false;
  static TypeId tid;
  if (!configured) {
    tid = TypeId ("ns3::ProxyBase")
      .SetParent<Application> ()
      .SetGroupName("Applications");
    for (int i = 0; i < RX_SLOT_COUNT; ++i) {
      std::ostringstream addrname;
      addrname << "RxAddr" << i;
      tid.AddAttribute (addrname.str(),
                   "The Address on which to Bind the rx socket",
                   AddressValue (),
                   MakeRxTxAddrAttributeAccessor (false, i),
                   MakeAddressChecker ());
    }
    for (int i = 0; i < TX_SLOT_COUNT; ++i) {
      std::ostringstream addrname;
      addrname << "TxAddr" << i;
      tid.AddAttribute(addrname.str(),
                   "The address of the destination",
                   AddressValue (),
                   MakeRxTxAddrAttributeAccessor (true, i),
                   MakeAddressChecker ());
    }
    tid.AddTraceSource ("Rx",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&ProxyBase::m_rxTrace),
                     "ns3::Packet::AddressTracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&ProxyBase::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&ProxyBase::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&ProxyBase::m_txTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
    ;

    configured = true;
  }
  return tid;
}

ProxyBase::ProxyBase ()
{
  NS_LOG_FUNCTION (this);
  for (int i = 0; i < RX_SLOT_COUNT; ++i) {
    m_rxSlots[i].listen_sock = 0;
    m_rxSlots[i].total = 0;
  }
  for (int i = 0; i < TX_SLOT_COUNT; ++i) {
    m_txSlots[i].sock = 0;
    m_txSlots[i].connected = false;
    m_txSlots[i].total = 0;
  }
}

ProxyBase::~ProxyBase()
{
  NS_LOG_FUNCTION (this);
}

uint64_t ProxyBase::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);
  uint64_t tot = 0;
  for (int i = 0; i < RX_SLOT_COUNT; ++i) {
    tot += m_rxSlots[i].total;
  }
  return tot;
}

uint64_t ProxyBase::GetTotalTx () const
{
  NS_LOG_FUNCTION (this);
  uint64_t tot = 0;
  for (int i = 0; i < TX_SLOT_COUNT; ++i) {
    tot += m_txSlots[i].total;
  }
  return tot;
}

void ProxyBase::SetTidDefaultProtocols(TypeId* tid,
				const TypeId& rx,
				const TypeId& tx)
{
  for (int i = 0; i < RX_SLOT_COUNT; ++i) {
    std::ostringstream protoname;
    protoname << "ProtocolRx" << i;
    tid->AddAttribute(protoname.str(),
                "The type ID for the protocol for the Rx socket",
                TypeIdValue(rx),
                MakeRxTxProtoAttributeAccessor(false, i),
                MakeTypeIdChecker ());
  }
  for (int i = 0; i < TX_SLOT_COUNT; ++i) {
    std::ostringstream protoname;
    protoname << "ProtocolTx" << i;
    tid->AddAttribute(protoname.str(),
                "The type ID for the protocol for the Tx socket",
                TypeIdValue(tx),
                MakeRxTxProtoAttributeAccessor(true, i),
                MakeTypeIdChecker ());
  }
}

void ProxyBase::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (int i = 0; i < RX_SLOT_COUNT; ++i) {
    m_rxSlots[i].listen_sock = 0;
    m_rxSlots[i].accepted_socks.clear();
    m_rxSlots[i].total = 0;
  }
  for (int i = 0; i < TX_SLOT_COUNT; ++i) {
    m_txSlots[i].sock = 0;
    m_txSlots[i].connected = 0;
    m_txSlots[i].total = 0;
  }

  // chain up
  Application::DoDispose ();
}

// Application Methods
void ProxyBase::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Create the RX listen sockets and set up the callbacks.
  for (int i = 0; i < RX_SLOT_COUNT; ++i) {
    RxSlot* sl = &m_rxSlots[i];
    if (sl->addr.IsInvalid())
      continue;
    if (!sl->listen_sock)
      {
        sl->listen_sock = Socket::CreateSocket (GetNode (), sl->tid);
        if (sl->listen_sock->Bind (sl->addr) == -1)
          {
            NS_FATAL_ERROR ("Failed to bind socket");
          }
        sl->listen_sock->Listen ();
        if (addressUtils::IsMulticast (sl->addr))
          {
            Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (sl->listen_sock);
            if (udpSocket)
              {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup (0, sl->addr);
              }
            else
              {
                NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
              }
          }
      }

      sl->listen_sock->SetRecvCallback (
        MakeBoundCallback (&DispatchMethod<&ProxyBase::HandleRead>,
                                this, i));
      sl->listen_sock->SetAcceptCallback (
        MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
        MakeBoundCallback (&DispatchMethod<&ProxyBase::HandleAccept>,
                                this, i));
      sl->listen_sock->SetCloseCallbacks (
        MakeBoundCallback (&DispatchMethod<&ProxyBase::HandlePeerClose>,
                                this, i),
        MakeBoundCallback (&DispatchMethod<&ProxyBase::HandlePeerError>,
                                this, i));
  }

  // Create the TX sockets and set up the callbacks
  for (int i = 0; i < TX_SLOT_COUNT; ++i) {
    TxSlot* sl = &m_txSlots[i];
    if (sl->addr.IsInvalid())
      continue;
    if (!sl->sock)
      {
        sl->sock = Socket::CreateSocket (GetNode (), sl->tid);
        if (Inet6SocketAddress::IsMatchingType (sl->addr))
          {
            if (sl->sock->Bind6 () == -1)
              {
                NS_FATAL_ERROR ("Failed to bind socket");
              }
          }
        else if (InetSocketAddress::IsMatchingType (sl->addr) ||
                 PacketSocketAddress::IsMatchingType (sl->addr))
          {
            if (sl->sock->Bind () == -1)
              {
                NS_FATAL_ERROR ("Failed to bind socket");
              }
          }
        sl->sock->Connect (sl->addr);
        sl->sock->SetAllowBroadcast (true);

        sl->sock->SetConnectCallback (
          MakeBoundCallback (
            &DispatchMethod<&ProxyBase::ConnectionSucceeded>,
            this, i),
          MakeBoundCallback (
            &DispatchMethod<&ProxyBase::ConnectionFailed>,
            this, i));
        sl->sock->SetSendCallback (
          MakeBoundCallback (
            &DispatchMethod<&ProxyBase::HandleSend>,
            this, i));
        sl->sock->SetRecvCallback (
          MakeCallback(&ProxyBase::HandleBackRead, this));
      }
  }

}

void ProxyBase::ConnectionSucceeded (int slotID, Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_txSlots[slotID].connected = true;
}

void ProxyBase::ConnectionFailed (int slotID, Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void ProxyBase::StopApplication ()     // Called at time specified by Stop
{
  // Clean up receive sockets
  NS_LOG_FUNCTION (this);
  for (int i = 0; i < RX_SLOT_COUNT; ++i) {
    // close the accepted sockets
    while(!m_rxSlots[i].accepted_socks.empty ())
      {
        Ptr<Socket> sock = m_rxSlots[i].accepted_socks.back ();
        m_rxSlots[i].accepted_socks.pop_back ();
        sock->Close ();
      }
    if (m_rxSlots[i].listen_sock)
      {
        m_rxSlots[i].listen_sock->Close ();
        m_rxSlots[i].listen_sock
          ->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      }
  }

  // Clean up transmit sockets
  for (int i = 0; i < TX_SLOT_COUNT; ++i) {
    if(m_txSlots[i].sock != 0)
      {
        m_txSlots[i].sock->Close ();
      }
  }
}

void ProxyBase::HandleRead (int slotID, Ptr<Socket> socket)
{
}

void ProxyBase::HandleAccept (int slotID, Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeBoundCallback (
    &DispatchMethod<&ProxyBase::HandleRead>, this, slotID));

  // Find the slot it belongs to, and add it to list.
  m_rxSlots[slotID].accepted_socks.push_back(s);
}

void ProxyBase::HandlePeerClose (int slotID, Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void ProxyBase::HandlePeerError (int slotID, Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void ProxyBase::HandleSend(int slotID, Ptr<Socket>, uint32_t sz)
{
}

void ProxyBase::HandleBackRead(Ptr<Socket> sock)
{
        // Read and discard.
        sock->Recv();
}

/* Implementation of the special purpose attribute accessors */

Ptr<const AttributeAccessor>
  ProxyBase::MakeRxTxAddrAttributeAccessor(bool isTx, int index)
{
  RxTxAddrAttributeAccessor*
    acc = new RxTxAddrAttributeAccessor(isTx, index);
  return Ptr<const AttributeAccessor>(acc);
}

ProxyBase::RxTxAddrAttributeAccessor::RxTxAddrAttributeAccessor(
                bool isTx_,
                int index_)
  : isTx(isTx_), index(index_)
{
}

bool ProxyBase::RxTxAddrAttributeAccessor::DoSet(
                ProxyBase* object, const AddressValue* v) const
{
  if (isTx) {
    object->m_txSlots[index].addr = v->Get();
  } else {
    object->m_rxSlots[index].addr = v->Get();
  }
  return true;
}

bool ProxyBase::RxTxAddrAttributeAccessor::DoGet(
                const ProxyBase* object, AddressValue* v) const
{
  if (isTx) {
    v->Set(object->m_txSlots[index].addr);
  } else {
    v->Set(object->m_rxSlots[index].addr);
  }
  return true;
}

Ptr<const AttributeAccessor>
  ProxyBase::MakeRxTxProtoAttributeAccessor(bool isTx, int index)
{
  RxTxProtoAttributeAccessor*
    acc = new RxTxProtoAttributeAccessor(isTx, index);
  return Ptr<const AttributeAccessor>(acc);
}

ProxyBase::RxTxProtoAttributeAccessor::RxTxProtoAttributeAccessor(
                bool isTx_,
                int index_)
  : isTx(isTx_), index(index_)
{
}

bool ProxyBase::RxTxProtoAttributeAccessor::DoSet(
                ProxyBase* object, const TypeIdValue* v) const
{
  if (isTx) {
    object->m_txSlots[index].tid = v->Get();
  } else {
    object->m_rxSlots[index].tid = v->Get();
  }
  return true;
}

bool ProxyBase::RxTxProtoAttributeAccessor::DoGet(
                const ProxyBase* object, TypeIdValue* v) const
{
  if (isTx) {
    v->Set(object->m_txSlots[index].tid);
  } else {
    v->Set(object->m_rxSlots[index].tid);
  }
  return true;
}

} // Namespace ns3

// vim:set et:sts=2:sw=2
