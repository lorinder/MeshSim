/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */

#ifndef PROXY_BASE_H
#define PROXY_BASE_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/attribute-accessor-helper.h"

namespace ns3 {

class Address;
class Socket;
class Packet;

/**
 * \ingroup applications
 * \defgroup proxy Proxy
 *
 * Simulates a proxy application, receiving traffic, and forwarding it
 * elsewhere with potential modifications such as encoding, timing, etc.
 */
class ProxyBase : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  ProxyBase ();

  virtual ~ProxyBase ();

  /**
   * \return the total bytes received in this encoder app
   */
  uint64_t GetTotalRx () const;

  /**
   * \return the total bytes sent by the encoder
   */
  uint64_t GetTotalTx () const;

  enum {
    RX_SLOT_COUNT = 8,
    TX_SLOT_COUNT = 8
  };

protected:
  // Utility functions for subclassing
  static void SetTidDefaultProtocols(TypeId* tid,
				const TypeId& rx,
				const TypeId& tx);

  // Destruction
  virtual void DoDispose (void);

  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  /* Static method dispatchers.
   * Those are here so we can use MakeBoundCallback with class
   * member functions.  NS-3 doesn't have built-in support for this,
   * unfortunately.
   */
  template<void (ProxyBase::*METHOD)(int, Ptr<Socket>)>
    static void DispatchMethod(ProxyBase* instance,
				int slotID,
				Ptr<Socket> socket)
  {
    (instance->*METHOD) (slotID, socket);
  }
  template<void (ProxyBase::*METHOD)(int, Ptr<Socket>, const Address&)>
    static void DispatchMethod(ProxyBase* instance,
				int slotID,
				Ptr<Socket> socket,
				const Address& addr)
  {
    (instance->*METHOD) (slotID, socket, addr);
  }
  template<void (ProxyBase::*METHOD)(int, Ptr<Socket>, uint32_t)>
    static void DispatchMethod(ProxyBase* instance,
				int slotID,
				Ptr<Socket> socket,
				uint32_t sz)
  {
    (instance->*METHOD) (slotID, socket, sz);
  }

  /**
   * \brief Handle a packet received by the application
   * \param socket the receiving socket
   */
  virtual void HandleRead (int slotID, Ptr<Socket> socket);

  /**
   * \brief Handle an incoming connection
   * \param socket the incoming connection socket
   * \param from the address the connection is from
   */
  virtual void HandleAccept (int slotID,
		     Ptr<Socket> socket,
		     const Address& from);

  /**
   * \brief Handle an connection close
   * \param socket the connected socket
   */
  virtual void HandlePeerClose (int slotID,
		    Ptr<Socket> socket);

  /**
   * \brief Handle an connection error
   * \param socket the connected socket
   */
  virtual void HandlePeerError (int slotID, Ptr<Socket> socket);

  /**
   *\brief Sender callback
   */
  virtual void HandleSend(int slotID, Ptr<Socket>, uint32_t sz);

  void HandleBackRead (Ptr<Socket> socket);

  // Receive state

  struct RxSlot {
    Address addr;		              //!< Local address to bind to

    // In the case of TCP, each socket accept returns a new socket, so the
    // listening socket is stored separately from the accepted sockets
    Ptr<Socket> listen_sock;		      //!< Listening socket
    std::list< Ptr<Socket> > accepted_socks;  //!< the accepted sockets

    TypeId          tid;                      //!< Protocol TypeId
    uint64_t        total;                    //!< Total bytes received
  };

  RxSlot          m_rxSlots[RX_SLOT_COUNT];
  int             m_nextRxSlot;

  // Transmit state
  struct TxSlot {
    Address         addr;                     //!< Peer address
    Ptr<Socket>     sock;                     //!< Associated socket to send on
    TypeId          tid;                      //!< Protocol TypeId
    bool            connected;                //!< True if connected
    uint64_t        total;                    //!< Total bytes sent
  };

  TxSlot          m_txSlots[TX_SLOT_COUNT];

  /// Traced Callback: received packets, source address.
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

  /// Callback for tracing the packet Rx events, includes source and destination addresses
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;

    /// Traced Callback: transmitted packets.
  TracedCallback<Ptr<const Packet> > m_txTrace;

  /// Callbacks for tracing the packet Tx events, includes source and destination addresses
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses;

  /**
   * \brief Handle a Connection Succeed event
   * \param socket the connected socket
   */
  void ConnectionSucceeded (int slotID, Ptr<Socket> socket);

  /**
   * \brief Handle a Connection Failed event
   * \param socket the not connected socket
   */
  void ConnectionFailed (int slotID, Ptr<Socket> socket);

  /* Special purpose attribute accessors.
   *
   * These are necessary because we can't make the conventional ones
   * work with arrays.
   */
  class RxTxAddrAttributeAccessor
    : public AccessorHelper<ProxyBase, AddressValue>
  {
  public:
    RxTxAddrAttributeAccessor(bool isTx, int index);
    bool DoSet (ProxyBase *object, const AddressValue* v) const;
    bool DoGet (const ProxyBase* object, AddressValue* v) const;
    bool HasGetter() const { return true; }
    bool HasSetter() const { return true; }
  private:
    bool isTx;
    int index;
  };
  friend class RxTxAddrAttributeAccessor;

  static Ptr<const AttributeAccessor>
    MakeRxTxAddrAttributeAccessor(bool isTx, int index);

  class RxTxProtoAttributeAccessor
    : public AccessorHelper<ProxyBase, TypeIdValue>
  {
  public:
    RxTxProtoAttributeAccessor(bool isTx, int index);
    bool DoSet (ProxyBase *object, const TypeIdValue* v) const;
    bool DoGet (const ProxyBase* object, TypeIdValue* v) const;
    bool HasGetter() const { return true; }
    bool HasSetter() const { return true; }
  private:
    bool isTx;
    int index;
  };
  friend class RxTxProtoAttributeAccessor;

  static Ptr<const AttributeAccessor>
    MakeRxTxProtoAttributeAccessor(bool isTx, int index);
};

} // namespace ns3

#endif /* PROXY_BASE_H */

