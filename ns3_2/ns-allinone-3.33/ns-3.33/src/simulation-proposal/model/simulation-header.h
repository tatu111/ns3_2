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




  // Inherited
  virtual TypeId GetInstanceTypeId (void) const override;
  virtual void Print (std::ostream &os) const override;
  virtual uint32_t GetSerializedSize (void) const override;
  virtual void Serialize (Buffer::Iterator start) const override;
  virtual uint32_t Deserialize (Buffer::Iterator start) override;


  struct Proactive
  {
	uint32_t m_proactive_resource;
	void Serialize(Buffer::Iterator start) const;

	uint32_t GetSerializedSize (void) const;

	uint32_t Deserialize (Buffer::Iterator start);


	void SetResource (uint32_t resource_sum);


	uint32_t GetResource (void) const;
  };

  struct Core
  {

	void Serialize(Buffer::Iterator start) const;

	uint32_t GetSerializedSize (void) const;

	uint32_t Deserialize (Buffer::Iterator start);


  };

  struct Flooding
  {

	  void Serialize(Buffer::Iterator start) const;

	  uint32_t GetSerializedSize (void) const;

	  uint32_t Deserialize (Buffer::Iterator start);

  };

  struct FloodingReturn
  {
	  void Serialize(Buffer::Iterator start) const;

	  uint32_t GetSerializedSize (void) const;

	  uint32_t Deserialize (Buffer::Iterator start);
  };

  struct Notice
  {
	  void Serialize(Buffer::Iterator start) const;

	  uint32_t GetSerializedSize (void) const;

	  uint32_t Deserialize (Buffer::Iterator start);
  };



  struct
    {
      Proactive proactive;      //!< MID message (optional).
      Core core;  //!< HELLO message (optional).
      Flooding flooding;        //!< TC message (optional).
      FloodingReturn floodingreturn;      //!< HNA message (optional).
      Notice notice;//NOTICE message
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
    void GetFlooding ()
    {
      if (m_messageType == 0)
        {
          m_messageType = FLOODING_MESSAGE;
        }
      else
        {
          NS_ASSERT (m_messageType == FLOODING_MESSAGE);
        }
//      return m_message.flooding;
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
    const void GetFlooding () const
    {
      NS_ASSERT (m_messageType == FLOODING_MESSAGE);
//      return m_message.flooding;
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







private:
  uint64_t m_size {0}; //!< The 'size' information that the header is carrying
  uint32_t m_resource;
  MessageType m_messageType;
  uint8_t m_hopCount;
  Ipv4Address m_destinationAddress;
};

} // namespace ns3

#endif /* SEQ_TS_SIZE_HEADER2 */
