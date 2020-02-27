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

#ifndef RQ_DECODER_H
#define RQ_DECODER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"

#include "proxy-base.h"

namespace ns3 {

/**
 * \ingroup applications
 * \defgroup rqdecoder RqDecoder
 */

/**
 * \ingroup rqdecoder
 *
 * \brief This application simulates an RQ decoder.  It receives
 * encoded UDP packet, conceptually decodes source blocks and transmits
 * the results as TCP stream.
 */
class RqDecoder : public ProxyBase
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  RqDecoder ();

  virtual ~RqDecoder ();

  /// Describes the result of a decoding operation.
  struct DecodeInfo {
    unsigned int sbid;  //!< source block ID
    bool success;       //!< decoding success
    int n_rcv;          //!< # packets recevied
    int n_src_rcv;      //!< # src packets recvd

    /// Describes decode Information on a per-slot basis.
    struct SlotDecodeInfo {
      int n_src_rcv; //!< # src packets received
      int n_contig_src_rcv; //!< # contiguous src packets received
    } slotinfo[TX_SLOT_COUNT];

    int n_slots;	//!< # of slots.
  };

  typedef void (*RqDecoderCallback)(const DecodeInfo& I);

protected:
  virtual void StartApplication (void);

  void PopulateDecodeInfo(DecodeInfo* I);

  /**
   * \brief Handle a packet received by the application
   * \param socket the receiving socket
   */
  virtual void HandleRead (int slotID, Ptr<Socket> socket);

  /**
   *\brief Sends traffic, if appropriate
   */
  virtual void TryToSend(int slotID);

  /**
   *\brief Sender callback
   */
  virtual void HandleSend(int slotID, Ptr<Socket>, uint32_t sz);

  // RQ configuration parameters
  int             m_rqkval;        //!< Source block size in packets
  int             m_rqnval;        //!< Encoded block size in packets
  uint32_t        m_rqtval;        //!< Packet size

  // RQ state
  int		  m_sbid;	   //!< Current source block ID
  int		  m_sb_nrcv;	   //!< # syms received for SB
  int             m_sb_nsrcrcv;    //!< same, for src syms
  int		  m_nslots;	   //!< Count of slots available
  struct SlotState {
    uint32_t      m_nbuffered;     //!< # bytes buffered for stream
    int           m_nsymbcontig;   //!< # contiguous symbs for stream rcvd
    int		  m_nsymb;         //!< # symbs for that stream in SB
  } m_slotStates[TX_SLOT_COUNT];

  // RQ decoder event traces
  TracedCallback< const DecodeInfo& > m_rqDecodingTrace;
};

} // namespace ns3

#endif /* RQ_DECODER_H */

