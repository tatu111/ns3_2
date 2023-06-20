/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2009 INRIA
 *   Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
 *                      (added timestamp and size fields)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SIMULATION_HEADER_H
#define SIMULATION_HEADER_H

#include <ns3/seq-ts-header.h>
#include "ns3/ipv4-address.h"

namespace ns3 {
/**
 * \ingroup applications
 * \brief Header with a sequence, a timestamp, and a "size" attribute
 *
 * This header adds a size attribute to the sequence number and timestamp
 * of class \c SeqTsHeader.  The size attribute can be used to track
 * application data units for stream-based sockets such as TCP.
 *
 * \sa ns3::SeqTsHeader
 */
class SimulationHeader : public SeqTsHeader
{
public:

	  enum MessageType
	  {
	    PROACTIVE_MESSAGE = 1,
	    CORE_MESSAGE    = 2,
	    FLOODING_MESSAGE   = 3,
	    FLOODINGRETURN_MESSAGE   = 4,
		NOTICE_MESSAGE = 5,
		CORECOMPUTING_MESSAGE = 6,
		COMPUTING_MESSAGE = 7,
		NORMAL_MESSAGE = 8,
		STOPCOMPUTING_MESSAGE = 9,
	  };




  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief constructor
   */
  SimulationHeader ();

  /**
   * \brief Set the size information that the header will carry
   * \param size the size
   */
  void SetSize (uint64_t size);

//  void SetResource (uint32_t resource_sum);

  void SetMessageType (MessageType messageType)
  {
    m_messageType = messageType;
  }
  /**
   * Get the message type.
   * \return The message type.
   */
  MessageType GetMessageType () const
  {
    return m_messageType;
  }


  void SetHopCount (uint8_t hopCount)
  {
    m_hopCount = hopCount;
  }
  /**
   * Get the hop count.
   * \return The hop count.
   */
  uint8_t GetHopCount () const
  {
    return m_hopCount;
  }




  void SetDestinationAddress (Ipv4Address destinationAddress)
  {
	  m_destinationAddress = destinationAddress;
  }


  Ipv4Address GetDestinationAddress () const
  {
	  return m_destinationAddress;
  }



  /**
   * \brief Get the size information that the header is carrying
   * \return the size
   */
  uint64_t GetSize (void) const;


private:
  uint64_t m_size {0}; //!< The 'size' information that the header is carrying
  uint32_t m_resource;
  MessageType m_messageType;
  uint8_t m_hopCount;
  Ipv4Address m_destinationAddress;
//  uint8_t m_timeToLive;             //!< The time to live.


public:


  // Inherited
  virtual TypeId GetInstanceTypeId (void) const override;
  virtual void Print (std::ostream &os) const override;
  virtual uint32_t GetSerializedSize (void) const override;
  virtual void Serialize (Buffer::Iterator start) const override;
  virtual uint32_t Deserialize (Buffer::Iterator start) override;


  struct Proactive
  {
	uint32_t m_proactive_resource;
	std::vector<Ipv4Address> core_candidate_surrounding;
	void Serialize(Buffer::Iterator start) const;

	uint32_t GetSerializedSize (void) const;

	uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);


	void SetResource (uint32_t resource_sum);

	uint32_t GetResource (void) const;

	void SetAddr (std::vector<Ipv4Address> addr);

	std::vector<Ipv4Address> GetAddr (void) const;
  };

  struct Core
  {

	void Serialize(Buffer::Iterator start) const;

	uint32_t GetSerializedSize (void) const;

	uint32_t Deserialize (Buffer::Iterator start);


  };

  struct Flooding
  {
	  uint8_t m_timeToLive = 2;

	  void SetTimeToLive (uint8_t timeToLive);
//	  {
//	    m_timeToLive = timeToLive;
//	  }
	  /**
	   * Get the time to live.
	   * \return The time to live.
	   */
	  uint8_t GetTimeToLive () const;
//	  {
//	    return m_timeToLive;
//	  }

	  void Serialize(Buffer::Iterator start) const;

	  uint32_t GetSerializedSize (void) const;

	  uint32_t Deserialize (Buffer::Iterator start);

  };

  struct FloodingReturn
  {
	  uint32_t m_floodingreturn_resource;
	  void Serialize(Buffer::Iterator start) const;

	  uint32_t GetSerializedSize (void) const;

	  uint32_t Deserialize (Buffer::Iterator start);

	  void SetResource (uint32_t resource_sum);

	  uint32_t GetResource (void) const;
  };

  struct Notice
  {
	  std::map<Ipv4Address, uint32_t> m_resource_floodingreturn;
	  void Serialize(Buffer::Iterator start) const;

	  uint32_t GetSerializedSize (void) const;

	  uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);

	  void SetResource (std::map<Ipv4Address, uint32_t> resource);

	  std::map<Ipv4Address, uint32_t> GetResource (void) const;
  };

  struct CoreComputing
  {
	  std::map<Ipv4Address, uint32_t> core_computing_addr;
	  void Serialize(Buffer::Iterator start) const;

	  uint32_t GetSerializedSize (void) const;

	  uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);

	  void SetAddr (std::map<Ipv4Address, uint32_t> addr);

	  std::map<Ipv4Address, uint32_t> GetAddr (void) const;

  };

  struct Computing
  {
	  uint32_t computing_resource;
	  void Serialize(Buffer::Iterator start) const;

	  uint32_t GetSerializedSize (void) const;

	  uint32_t Deserialize (Buffer::Iterator start);

	  void SetComputingResource (uint32_t computing);

	  uint32_t GetComputingResource (void) const;
  };

  struct Normal
  {

	void Serialize(Buffer::Iterator start) const;

	uint32_t GetSerializedSize (void) const;

	uint32_t Deserialize (Buffer::Iterator start);

  };

  struct StopComputing
    {
  	  std::map<Ipv4Address, uint32_t> core_computing_addr;
  	  void Serialize(Buffer::Iterator start) const;

  	  uint32_t GetSerializedSize (void) const;

  	  uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);

  	  void SetAddr (std::map<Ipv4Address, uint32_t> addr);

  	  std::map<Ipv4Address, uint32_t> GetAddr (void) const;

    };


  struct
    {
      Proactive proactive;      //!< MID message (optional).
      Core core;  //!< HELLO message (optional).
      Flooding flooding;        //!< TC message (optional).
      FloodingReturn floodingreturn;      //!< HNA message (optional).
      Notice notice;//NOTICE message
      CoreComputing corecomputing;//CoreComputing message
      Computing computing;//Computing message
      Normal normal;//normal message
      StopComputing stopcomputing;//StopComputing message
    } m_message; //!< The actual message being carried.

  public:
    /**
     * Set the message type to MID and return the message content.
     * \returns The MID message.
     */
    Proactive& GetProactive ()
    {
      if (m_messageType == 0)
        {
          m_messageType = PROACTIVE_MESSAGE;
        }
      else
        {
          NS_ASSERT (m_messageType == PROACTIVE_MESSAGE);
        }
      return m_message.proactive;
    }

    /**
     * Set the message type to HELLO and return the message content.
     * \returns The HELLO message.
     */
    void GetCore ()
    {
      if (m_messageType == 0)
        {
          m_messageType = CORE_MESSAGE;
        }
      else
        {
          NS_ASSERT (m_messageType == CORE_MESSAGE);
        }
//      return m_message.core;
    }

    /**
     * Set the message type to TC and return the message content.
     * \returns The TC message.
     */
    Flooding& GetFlooding ()
    {
      if (m_messageType == 0)
        {
          m_messageType = FLOODING_MESSAGE;
        }
      else
        {
          NS_ASSERT (m_messageType == FLOODING_MESSAGE);
        }
      return m_message.flooding;
    }

    /**
     * Set the message type to HNA and return the message content.
     * \returns The HNA message.
     */
    FloodingReturn& GetFloodingReturn ()
    {
      if (m_messageType == 0)
        {
          m_messageType = FLOODINGRETURN_MESSAGE;
        }
      else
        {
          NS_ASSERT (m_messageType == FLOODINGRETURN_MESSAGE);
        }
      return m_message.floodingreturn;
    }


    Notice& GetNotice()
    {
  	  if (m_messageType == 0)
  	        {
  	          m_messageType = NOTICE_MESSAGE;
  	        }
  	      else
  	        {
  	          NS_ASSERT (m_messageType == NOTICE_MESSAGE);
  	        }
  	      return m_message.notice;
    }

    CoreComputing& GetCoreComputing ()
    {
    	if (m_messageType == 0)
    	{
    		m_messageType = CORECOMPUTING_MESSAGE;
    	}
    	else
    	{
    		NS_ASSERT (m_messageType == CORECOMPUTING_MESSAGE);
    	}
    	return m_message.corecomputing;
    }

    Computing& GetComputing ()
    {
    	if (m_messageType == 0)
    	{
    		m_messageType = COMPUTING_MESSAGE;
    	}
    	else
    	{
    		NS_ASSERT (m_messageType == COMPUTING_MESSAGE);
    	}
    	return m_message.computing;
    }

    void GetNormal ()
        {
        	if (m_messageType == 0)
        	{
        		m_messageType = NORMAL_MESSAGE;
        	}
        	else
        	{
        		NS_ASSERT (m_messageType == NORMAL_MESSAGE);
        	}
    //    	return m_message.computing;
        }

    StopComputing& GetStopComputing ()
    {
    	if (m_messageType == 0)
    	{
    		m_messageType = STOPCOMPUTING_MESSAGE;
    	}
    	else
    	{
    		NS_ASSERT (m_messageType == STOPCOMPUTING_MESSAGE);
    	}
    	return m_message.stopcomputing;
    }

    /**
     * Get the MID message.
     * \returns The MID message.
     */
    const Proactive& GetProactive () const
    {
      NS_ASSERT (m_messageType == PROACTIVE_MESSAGE);
      return m_message.proactive;
    }

    /**
     * Get the HELLO message.
     * \returns The HELLO message.
     */
    const void GetCore () const
    {
      NS_ASSERT (m_messageType == CORE_MESSAGE);
//      return m_message.core;
    }

    /**
     * Get the TC message.
     * \returns The TC message.
     */
    const Flooding& GetFlooding () const
    {
      NS_ASSERT (m_messageType == FLOODING_MESSAGE);
      return m_message.flooding;
    }

    /**
     * Get the HNA message.
     * \returns The HNA message.
     */
    const FloodingReturn& GetFloodingReturn  () const
    {
      NS_ASSERT (m_messageType == FLOODINGRETURN_MESSAGE);
      return m_message.floodingreturn;
    }

    const Notice& GetNotice () const
      {
        NS_ASSERT (m_messageType == NOTICE_MESSAGE);
        return m_message.notice;
      }

    const CoreComputing& GetCoreComputing () const
      {
        NS_ASSERT (m_messageType == CORECOMPUTING_MESSAGE);
        return m_message.corecomputing;
      }

    const Computing& GetComputing () const
      {
        NS_ASSERT (m_messageType == COMPUTING_MESSAGE);
        return m_message.computing;
      }

    const void GetNormal () const
    {
    	NS_ASSERT (m_messageType == NORMAL_MESSAGE);
    //        return m_message.computing;
    }

    const StopComputing& GetStopComputing () const
      {
        NS_ASSERT (m_messageType == STOPCOMPUTING_MESSAGE);
        return m_message.stopcomputing;
      }

};

} // namespace ns3

#endif /* SEQ_TS_SIZE_HEADER2 */
