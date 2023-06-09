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


uint64_t
SimulationHeader::GetSize (void) const
{
  return m_size;
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
	    case CORECOMPUTING_MESSAGE:
	    	size += m_message.corecomputing.GetSerializedSize();
	    	break;
	    case COMPUTING_MESSAGE:
	    	size += m_message.computing.GetSerializedSize();
	    	break;
	    case NORMAL_MESSAGE:
	    	size += m_message.normal.GetSerializedSize();
	    	break;
	    case STOPCOMPUTING_MESSAGE:
	    	size += m_message.stopcomputing.GetSerializedSize();
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
  i.WriteHtonU32(m_destinationAddress.Get());
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
      case CORECOMPUTING_MESSAGE:
    	  m_message.corecomputing.Serialize(i);
    	  break;
      case COMPUTING_MESSAGE:
    	  m_message.computing.Serialize(i);
    	  break;
      case NORMAL_MESSAGE:
     	  m_message.normal.Serialize(i);
     	  break;
      case STOPCOMPUTING_MESSAGE:
    	  m_message.stopcomputing.Serialize(i);
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
  m_destinationAddress = Ipv4Address (i.ReadNtohU32 ());
  SeqTsHeader::Deserialize (i);

  switch (m_messageType)
  	    {
  	    case PROACTIVE_MESSAGE:
  	      size += m_message.proactive.Deserialize (i, m_size - 16);
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
  	      size += m_message.notice.Deserialize (i, m_size - 16);
  	      break;
  	    case CORECOMPUTING_MESSAGE:
  	      size += m_message.corecomputing.Deserialize (i, m_size - 16);
  	      break;
  	    case COMPUTING_MESSAGE:
  	      size += m_message.computing.Deserialize (i);
  	      break;
  	    case NORMAL_MESSAGE:
  	    	size += m_message.normal.Deserialize (i);
  	    	break;
  	    case STOPCOMPUTING_MESSAGE:
  	    	size += m_message.stopcomputing.Deserialize (i, m_size -16);
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
	i.WriteHtonU16(6 + core_candidate_surrounding.size() * 4);
	for(std::vector<Ipv4Address>::const_iterator iter = core_candidate_surrounding.begin ();
			iter != core_candidate_surrounding.end (); iter++)
	{
		i.WriteU32(iter->Get());
	}
}

uint32_t
SimulationHeader::Proactive::GetSerializedSize (void) const
{
	uint32_t size = 6;
	size += 4 * this->core_candidate_surrounding.size();
	return size;
}

uint32_t
SimulationHeader::Proactive::Deserialize(Buffer::Iterator start, uint32_t messageSize)
{

	Buffer::Iterator i = start;
	m_proactive_resource = i.ReadNtohU32();
	uint16_t size = messageSize;
	uint16_t lmSize = i.ReadNtohU16 ();
	for (int n = (lmSize - 6) / 4; n; --n)
	{
		Ipv4Address addr = Ipv4Address (i.ReadU32());
		core_candidate_surrounding.push_back(addr);
	}
	size -= lmSize;
	//
	//	}

	return messageSize;
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


void
SimulationHeader::Proactive::SetAddr (std::vector<Ipv4Address> addr)
{
	core_candidate_surrounding = addr;
}

std::vector<Ipv4Address>
SimulationHeader::Proactive::GetAddr (void) const
{
	return core_candidate_surrounding;
}

uint32_t
SimulationHeader::Core::GetSerializedSize (void) const
{
	uint32_t size = 4;
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



void
SimulationHeader::Flooding::SetTimeToLive (uint8_t timeToLive)
{
	m_timeToLive = timeToLive;
}

uint8_t
SimulationHeader::Flooding::GetTimeToLive () const
{
	return m_timeToLive;
}

uint32_t
SimulationHeader::Flooding::GetSerializedSize (void) const
{
	uint32_t size = 8;
	return size;
}

void
SimulationHeader::Flooding::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
	Buffer::Iterator i = start;
	i.WriteHtonU32(m_timeToLive);

}

uint32_t
SimulationHeader::Flooding::Deserialize(Buffer::Iterator start)
{

	Buffer::Iterator i = start;
	m_timeToLive = i.ReadNtohU32();

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
	Buffer::Iterator i = start;
	i.WriteHtonU32(m_floodingreturn_resource);
}

uint32_t
SimulationHeader::FloodingReturn::Deserialize(Buffer::Iterator start)
{

	Buffer::Iterator i = start;
	m_floodingreturn_resource = i.ReadNtohU32();
	return GetSerializedSize ();
}

void
SimulationHeader::FloodingReturn::SetResource (uint32_t resource_sum)
{
	m_floodingreturn_resource = resource_sum;
}

uint32_t
SimulationHeader::FloodingReturn::GetResource (void) const
{
  return m_floodingreturn_resource;
}


uint32_t
SimulationHeader::Notice::GetSerializedSize (void) const
{
	uint32_t size = 2;

	size += 8 * this->m_resource_floodingreturn.size();

	return size;
}


void
SimulationHeader::Notice::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
	Buffer::Iterator i = start;
//	std::cout<<m_resource_floodingreturn.size()<<std::endl;
	i.WriteHtonU16(2 + m_resource_floodingreturn.size() * 8);
	for(std::map<Ipv4Address, uint32_t>::const_iterator iter = m_resource_floodingreturn.begin ();
		       iter != m_resource_floodingreturn.end (); iter++)
	{
		i.WriteU32(iter->first.Get());
		i.WriteU32(iter->second);
	}
}

uint32_t
SimulationHeader::Notice::Deserialize(Buffer::Iterator start, uint32_t messageSize)
{
	NS_LOG_FUNCTION (this << &start);
	Buffer::Iterator i = start;

	uint16_t size = messageSize;
	uint16_t lmSize = i.ReadNtohU16 ();
//	std::cout<<size<<std::endl;
//	size -= 2;
//	while (size)
//	{
//		uint16_t lmSize = i.ReadNtohU16 ();
	for (int n = (lmSize - 2) / 8; n; --n)
	{
		Ipv4Address addr = Ipv4Address (i.ReadU32());
		uint32_t resource = i.ReadU32();
		m_resource_floodingreturn.insert (std::make_pair(addr, resource));
	}
	size -= lmSize;
//
//	}

	return messageSize;
}

void
SimulationHeader::Notice::SetResource (std::map<Ipv4Address, uint32_t> resource)
{
	m_resource_floodingreturn = resource;
}

std::map<Ipv4Address, uint32_t>
SimulationHeader::Notice::GetResource (void) const
{
  return m_resource_floodingreturn;
}



uint32_t
SimulationHeader::CoreComputing::GetSerializedSize (void) const
{
	uint32_t size = 2;

	size += 8 * this->core_computing_addr.size();
	return size;
}

void
SimulationHeader::CoreComputing::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
	Buffer::Iterator i = start;
	i.WriteHtonU16(2 + core_computing_addr.size() * 8);
	for(std::map<Ipv4Address, uint32_t>::const_iterator iter = core_computing_addr.begin ();
			iter != core_computing_addr.end (); iter++)
	{
		i.WriteU32(iter->first.Get());
		i.WriteU32(iter->second);
	}
}

uint32_t
SimulationHeader::CoreComputing::Deserialize(Buffer::Iterator start, uint32_t messageSize)
{

	Buffer::Iterator i = start;
	uint16_t size = messageSize;
	uint16_t lmSize = i.ReadNtohU16 ();
	for (int n = (lmSize - 2) / 8; n; --n)
	{
		Ipv4Address addr = Ipv4Address (i.ReadU32());
		uint32_t resource = i.ReadU32();
		core_computing_addr.insert (std::make_pair(addr, resource));
	}
	size -= lmSize;
//
//	}

	return messageSize;
}

void
SimulationHeader::CoreComputing::SetAddr (std::map<Ipv4Address, uint32_t> addr)
{
	core_computing_addr = addr;
}

std::map<Ipv4Address, uint32_t>
SimulationHeader::CoreComputing::GetAddr (void) const
{
  return core_computing_addr;
}

uint32_t
SimulationHeader::Computing::GetSerializedSize (void) const
{
	uint32_t size = 4;
	return size;
}

void
SimulationHeader::Computing::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
	Buffer::Iterator i = start;
	i.WriteHtonU32(computing_resource);

}

uint32_t
SimulationHeader::Computing::Deserialize(Buffer::Iterator start)
{

	Buffer::Iterator i = start;
	computing_resource = i.ReadNtohU32();

	return GetSerializedSize ();
}


void
SimulationHeader::Computing::SetComputingResource (uint32_t computing)
{
	computing_resource = computing;
}

uint32_t
SimulationHeader::Computing::GetComputingResource (void) const
{
	return computing_resource;
}



uint32_t
SimulationHeader::Normal::GetSerializedSize (void) const
{
	uint32_t size = 4;
	return size;
}

void
SimulationHeader::Normal::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
//	Buffer::Iterator i = start;

}

uint32_t
SimulationHeader::Normal::Deserialize(Buffer::Iterator start)
{

//	Buffer::Iterator i = start;

	return GetSerializedSize ();
}
void
SimulationHeader::StopComputing::SetAddr (std::map<Ipv4Address, uint32_t> addr)
{
	core_computing_addr = addr;
}

std::map<Ipv4Address, uint32_t>
SimulationHeader::StopComputing::GetAddr (void) const
{
  return core_computing_addr;
}

uint32_t
SimulationHeader::StopComputing::GetSerializedSize (void) const
{
	uint32_t size = 2;
	size += 8 * this->core_computing_addr.size();
	return size;
}

void
SimulationHeader::StopComputing::Serialize(Buffer::Iterator start) const
{
	NS_LOG_FUNCTION (this << &start);
	Buffer::Iterator i = start;
	i.WriteHtonU16(2 + core_computing_addr.size() * 8);
	for(std::map<Ipv4Address, uint32_t>::const_iterator iter = core_computing_addr.begin ();
			iter != core_computing_addr.end (); iter++)
	{
		i.WriteU32(iter->first.Get());
		i.WriteU32(iter->second);
	}
}

uint32_t
SimulationHeader::StopComputing::Deserialize(Buffer::Iterator start, uint32_t messageSize)
{

	Buffer::Iterator i = start;
	uint16_t size = messageSize;
	uint16_t lmSize = i.ReadNtohU16 ();
	for (int n = (lmSize - 2) / 8; n; --n)
	{
		Ipv4Address addr = Ipv4Address (i.ReadU32());
		uint32_t resource = i.ReadU32();
		core_computing_addr.insert (std::make_pair(addr, resource));
	}
	size -= lmSize;
//
//	}

	return messageSize;
}



} // namespace ns3
