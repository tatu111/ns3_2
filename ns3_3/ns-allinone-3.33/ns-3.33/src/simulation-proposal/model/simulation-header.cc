/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
 * Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
 *                    (added timestamp and size fields)
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
 */

#include "simulation-header.h"

#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimulationHeader");

NS_OBJECT_ENSURE_REGISTERED (SimulationHeader);

SimulationHeader::SimulationHeader ()
  : m_messageType (SimulationHeader::MessageType (0))
{
  NS_LOG_FUNCTION (this);
}

TypeId
SimulationHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimulationHeader")
    .SetParent<SeqTsHeader> ()
    .SetGroupName ("Applications")
    .AddConstructor<SimulationHeader> ()
  ;
  return tid;
}

TypeId
SimulationHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
SimulationHeader::SetSize (uint64_t size)
{
  m_size = size;
}

void
SimulationHeader::SetResource (uint32_t resource_sum)
{
  m_resource = resource_sum;
}

uint64_t
SimulationHeader::GetSize (void) const
{
  return m_size;
}

uint32_t
SimulationHeader::GetResource (void) const
{
  return m_resource;
}

void
SimulationHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(size=" << m_size << ") AND ";
  SeqTsHeader::Print (os);
}

uint32_t
SimulationHeader::GetSerializedSize (void) const
{
	uint32_t size = 16;
	switch (m_messageType)
	    {
	    case PROACTIVE_MESSAGE:
	      size += m_message.proactive.GetSerializedSize ();
	      break;
	    case CORE_MESSAGE:
	      size += m_message.core.GetSerializedSize ();
	      break;
	    case FLOODING_MESSAGE:
	      size += m_message.flooding.GetSerializedSize ();
	      break;
	    case FLOODINGRETURN_MESSAGE:
	      size += m_message.floodingreturn.GetSerializedSize ();
	      break;
	    case NOTICE_MESSAGE:
	      size += m_message.notice.GetSerializedSize();
	      break;
	    default:
	      NS_ASSERT (false);
	    }
  return SeqTsHeader::GetSerializedSize () + size;
}

void
SimulationHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteU8 (m_messageType);
  i.WriteU8 (m_hopCount);
  i.WriteHtonU64 (m_size);
  SeqTsHeader::Serialize (i);

  switch (m_messageType)
      {
      case PROACTIVE_MESSAGE:
        m_message.proactive.Serialize (i);
        break;
      case CORE_MESSAGE:
        m_message.core.Serialize (i);
        break;
      case FLOODING_MESSAGE:
        m_message.flooding.Serialize (i);
        break;
      case FLOODINGRETURN_MESSAGE:
        m_message.floodingreturn.Serialize (i);
        break;
      case NOTICE_MESSAGE:
        m_message.notice.Serialize(i);
        break;
      default:
        NS_ASSERT (false);
      }


}

uint32_t
SimulationHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  uint32_t size = 16;
  Buffer::Iterator i = start;
  m_messageType  = (MessageType) i.ReadU8 ();
  m_hopCount  = i.ReadU8 ();
  m_size = i.ReadNtohU64 ();
  SeqTsHeader::Deserialize (i);

  switch (m_messageType)
  	    {
  	    case PROACTIVE_MESSAGE:
  	      size += m_message.proactive.Deserialize (i);
  	      break;
  	    case CORE_MESSAGE:
  	      size += m_message.core.Deserialize (i);
  	      break;
  	    case FLOODING_MESSAGE:
  	      size += m_message.flooding.Deserialize (i);
  	      break;
  	    case FLOODINGRETURN_MESSAGE:
  	      size += m_message.floodingreturn.Deserialize (i);
  	      break;
  	    case NOTICE_MESSAGE:
  	      size += m_message.notice.Deserialize (i);
  	      break;
  	    default:
  	      NS_ASSERT (false);
  	    }

  return size;
}


void
SimulationHeader::Proactive::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
	Buffer::Iterator i = start;
	i.WriteHtonU32(m_proactive_resource);

}

uint32_t
SimulationHeader::Proactive::GetSerializedSize (void) const
{
	uint32_t size = 8;
	return size;
}

uint32_t
SimulationHeader::Proactive::Deserialize(Buffer::Iterator start)
{

	Buffer::Iterator i = start;
	m_proactive_resource = i.ReadNtohU32();

	return GetSerializedSize ();
}

void
SimulationHeader::Proactive::SetResource (uint32_t resource_sum)
{
	m_proactive_resource = resource_sum;
}

uint32_t
SimulationHeader::Proactive::GetResource (void) const
{
  return m_proactive_resource;
}

uint32_t
SimulationHeader::Core::GetSerializedSize (void) const
{
	uint32_t size = 16;
	return size;
}

void
SimulationHeader::Core::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
//	Buffer::Iterator i = start;

}

uint32_t
SimulationHeader::Core::Deserialize(Buffer::Iterator start)
{

//	Buffer::Iterator i = start;

	return GetSerializedSize ();
}




uint32_t
SimulationHeader::Flooding::GetSerializedSize (void) const
{
	uint32_t size = 16;
	return size;
}

void
SimulationHeader::Flooding::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
//	Buffer::Iterator i = start;

}

uint32_t
SimulationHeader::Flooding::Deserialize(Buffer::Iterator start)
{

//	Buffer::Iterator i = start;

	return GetSerializedSize ();
}

uint32_t
SimulationHeader::FloodingReturn::GetSerializedSize (void) const
{
	uint32_t size = 16;
	return size;
}


void
SimulationHeader::FloodingReturn::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
//	Buffer::Iterator i = start;

}

uint32_t
SimulationHeader::FloodingReturn::Deserialize(Buffer::Iterator start)
{

//	Buffer::Iterator i = start;

	return GetSerializedSize ();
}

uint32_t
SimulationHeader::Notice::GetSerializedSize (void) const
{
	uint32_t size = 16;
	return size;
}


void
SimulationHeader::Notice::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
//	Buffer::Iterator i = start;

}

uint32_t
SimulationHeader::Notice::Deserialize(Buffer::Iterator start)
{
//
//	Buffer::Iterator i = start;

	return GetSerializedSize ();
}



} // namespace ns3
