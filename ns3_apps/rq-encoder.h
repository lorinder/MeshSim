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

#ifndef RQ_ENCODER_H
#define RQ_ENCODER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"

#include "proxy-base.h"

namespace ns3 {

/**
 * \ingroup applications
 * \defgroup rqencoder RqEncoder
 */

/**
 * \ingroup packetsink
 *
 * \brief
 * This application simulates an RQ encoder. It receives a TCP stream
 * and then sends a corresponding UDP stream with simulated RQ encoding.
 */
class RqEncoder : public ProxyBase
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  RqEncoder ();

  virtual ~RqEncoder ();

protected:
  virtual void StartApplication();

  /**
   *\brief Sends traffic packet, if appropriate.
   */
  void SendPacket();

  // RQ configuration parameters
  uint32_t        m_rqkval;        //!< Source block size in packets
  uint32_t        m_rqnval;        //!< Encoded block size in packets
  uint32_t        m_rqtval;        //!< Packet size

  // Data receiving state
  int		  m_nRxSlots;      //!< The number of RX slots used
  int             m_nextRxSlot;	   //!< Next slot to probe (round robin)

  // Data sending state
  EventId         m_sendEvent;
  DataRate	  m_sendingRate;   //!< Data TX rate

  // RQ state
  uint32_t        m_sbid;          //!< Current source block ID (sending)
  uint32_t        m_esi;           //!< Next symbol ID to send
  uint32_t        m_symbcounts[RX_SLOT_COUNT]; //!< Symbol counts
};

} // namespace ns3

#endif /* RQ_ENCODER_H */

