#ifndef RQ_HEADER_H
#define RQ_HEADER_H

#include <stdint.h>
#include <string>
#include <vector>

#include "ns3/header.h"

namespace ns3 {
/**
 * \brief Packet header for RaptorQ
 */

class RqHeader : public Header
{
public:

    /**
     * \brief Constructor
     *
     * Creates a null header
     */
    RqHeader (uint64_t seqno = 0);
    RqHeader (uint64_t seqno,
            int iseq_streamid,
            int iseq_count,
            const uint32_t* iseq_data);

    bool IsValid (void) const;

    /**
     * \return the id for this RqHeader
     */
    uint64_t GetSeqno (void) const;

    int GetIseqStreamID (void) const;

    const std::vector<int>& GetIseqData (void) const;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);

    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

    static uint32_t GetMinSerializedSize (void);

private:
    bool     m_valid;
    uint64_t m_seqno;

    int32_t  m_iseq_streamid;
    std::vector<int> m_iseq_data;
};

}

#endif /* RQ HEADER */

// vim:sts=4:ts=8:sw=4:et
