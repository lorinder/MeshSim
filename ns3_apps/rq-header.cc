#include <algorithm>

#include "rq-header.h"
#include "ns3/log.h"

#define MAGIC     0x5271480d

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("RqHeader");
NS_OBJECT_ENSURE_REGISTERED (RqHeader);

RqHeader::RqHeader (uint64_t seqno)
    : m_valid(true),
      m_seqno(0)
{
    m_valid = false;
    m_seqno = 0;
    m_iseq_streamid = 0;
}

RqHeader::RqHeader (uint64_t seqno,
                int iseq_streamid,
                int iseq_count,
                const uint32_t* iseq_data
                )
    : m_valid(true),
      m_seqno(seqno),
      m_iseq_streamid(iseq_streamid)
{
    std::copy(iseq_data,
            iseq_data + iseq_count,
            std::back_inserter(m_iseq_data));
}

bool RqHeader::IsValid (void) const
{
    return m_valid;
}

uint64_t RqHeader::GetSeqno (void) const
{
    return m_seqno;
}

int RqHeader::GetIseqStreamID (void) const
{
    return m_iseq_streamid;
}

const std::vector<int>& RqHeader::GetIseqData (void) const
{
    return m_iseq_data;
}

void RqHeader::Print (std::ostream &os) const
{
    os << "RqHeader"
       << " valid " << m_valid
       << " seqno " << m_seqno
       << " iseq_streamid " << m_iseq_streamid
       << " iseq_data";
    for (int i = 0; i < (int)m_iseq_data.size(); ++i) {
        os << ' ' << m_iseq_data[i];
    }
    os << '\n';
}

uint32_t RqHeader::GetSerializedSize (void) const
{
    return GetMinSerializedSize() + 4 * (int)m_iseq_data.size();
}

/** The smallest possible serialized size */
uint32_t RqHeader::GetMinSerializedSize (void)
{
    return 4 + 8 + 4 + 4; /* Magic + sequence number +
                             iseq_streamid + iseq_data_count */
}

void
RqHeader::Serialize (Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    i.WriteHtonU32 (MAGIC);
    i.WriteHtonU64 (m_seqno);
    i.WriteHtonU32 (m_iseq_streamid);
    i.WriteHtonU32 (int(m_iseq_data.size()));
    for (int j = 0; j < (int)m_iseq_data.size(); ++j) {
        i.WriteHtonU32 (m_iseq_data[j]);
    }
}

uint32_t RqHeader::Deserialize (Buffer::Iterator start)
{
    m_iseq_data.clear();
    m_seqno = 0;

    Buffer::Iterator i = start;
    uint32_t magic = i.ReadNtohU32();
    m_valid = (magic == MAGIC);
    if (!m_valid) {
        /* Invalid, don't try to decode the rest */
        return 4;
    }
    m_seqno = i.ReadNtohU64 ();
    m_iseq_streamid = i.ReadNtohU32 ();

    int iseq_sz = i.ReadNtohU32 ();
    for (int j = 0; j < iseq_sz; ++j) {
        m_iseq_data.push_back (i.ReadNtohU32 ());
    }

    return GetSerializedSize ();
}

TypeId RqHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::RqHeader")
        .SetParent<Header> ()
        .SetGroupName("Network")
        ;
    return tid;
}

TypeId RqHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}


} // namespace ns3

// vim:sts=4:ts=8:sw=4:et
