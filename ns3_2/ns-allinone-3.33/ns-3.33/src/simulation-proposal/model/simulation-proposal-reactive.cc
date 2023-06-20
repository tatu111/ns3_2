/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// ns3 - On/Off Data Source Application class
// George F. Riley, Georgia Tech, Spring 2007
// Adapted from ApplicationOnOff in GTNetS.

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"

//追加
#include "ns3/olsr-routing-protocol.h"
#include <random>
#include "ns3/ipv4-list-routing.h"
#include "ns3/simulation-proposal.h"
#include "simulation-proposal-reactive.h"
#include "ns3/simulation-header.h"
#include "ns3/seq-ts-size-header.h"

#define task 500000000
//

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimulationProposalReactive");

NS_OBJECT_ENSURE_REGISTERED (SimulationProposalReactive);

TypeId
SimulationProposalReactive::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimulationProposalReactive")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<SimulationProposalReactive> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&SimulationProposalReactive::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&SimulationProposalReactive::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&SimulationProposalReactive::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("Local",
                   "The Address on which to bind the socket. If not set, it is generated automatically.",
                   AddressValue (),
                   MakeAddressAccessor (&SimulationProposalReactive::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=6.0]"),
                   MakePointerAccessor (&SimulationProposalReactive::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=3.0]"),
                   MakePointerAccessor (&SimulationProposalReactive::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (1000000),
                   MakeUintegerAccessor (&SimulationProposalReactive::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use. This should be "
                   "a subclass of ns3::SocketFactory",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&SimulationProposalReactive::m_tid),
                   // This should check for SocketFactory as a parent
                   MakeTypeIdChecker ())
    .AddAttribute ("EnableSeqTsSizeHeader",
                   "Enable use of SeqTsSizeHeader for sequence number and timestamp",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SimulationProposalReactive::m_enableSeqTsSizeHeader),
                   MakeBooleanChecker ())
//    .AddTraceSource ("Tx", "A new packet is created and is sent",
//                     MakeTraceSourceAccessor (&SimulationProposal::m_txTrace),
//                     "ns3::Packet::TracedCallback")
//    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
//                     MakeTraceSourceAccessor (&SimulationProposal::m_txTraceWithAddresses),
//                     "ns3::Packet::TwoAddressTracedCallback")
//    .AddTraceSource ("TxWithSeqTsSize", "A new packet is created with SeqTsSizeHeader",
//                     MakeTraceSourceAccessor (&SimulationProposal::m_txTraceWithSeqTsSize),
//                     "ns3::PacketSink::SeqTsSizeCallback")
//	.AddTraceSource ("Rx",
//	                "A packet has been received",
//					  MakeTraceSourceAccessor (&SimulationProposal::m_rxTrace),
//					  "ns3::Packet::AddressTracedCallback")
//	.AddTraceSource ("RxWithAddresses", "A packet has been received",
//			         MakeTraceSourceAccessor (&SimulationProposal::m_rxTraceWithAddresses),
//					 "ns3::Packet::TwoAddressTracedCallback")
//    .AddTraceSource ("RxWithSeqTsSize",
//    		         "A packet with SeqTsSize header has been received",
//					  MakeTraceSourceAccessor (&SimulationProposal::m_rxTraceWithSeqTsSize),
//					  "ns3::PacketSink::SeqTsSizeCallback")
  ;
  return tid;
}


SimulationProposalReactive::SimulationProposalReactive ()
  : m_socket (0),
	m_socket_local (0),
	m_recvSocket (0),
    m_connected (false),
    m_residualBits (0),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0),
    m_unsentPacket (0)
{
  NS_LOG_FUNCTION (this);
  m_totalRx = 0;
}

SimulationProposalReactive::~SimulationProposalReactive()
{
  NS_LOG_FUNCTION (this);
}

void
SimulationProposalReactive::SetMaxBytes (uint64_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}

Ptr<Socket>
SimulationProposalReactive::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

int64_t
SimulationProposalReactive::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  return 2;
}


Ptr<Socket>
SimulationProposalReactive::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

std::list<Ptr<Socket> >
SimulationProposalReactive::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}


void
SimulationProposalReactive::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  m_socket = 0;
  m_socket_local = 0;
  m_unsentPacket = 0;
  m_socketList.clear ();
  // chain up
  Application::DoDispose ();
}

// Application Methods
void SimulationProposalReactive::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);


  Ptr<Node> node = GetNode();
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();


//  Ptr<Ipv4RoutingProtocol> ipv4_proto = ipv4->GetRoutingProtocol ();
//  Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (ipv4_proto);
//  if (list)
//  {
//	  int16_t priority;
	  Ptr<Application> app;
	  Ptr<SimulationProposal> propo;
	  for (uint32_t i = 0; i < node->GetNApplications(); i++)
	  {
		  app = node->GetApplication(i);
         propo = DynamicCast<SimulationProposal> (app);
         //NS_ASSERT (listOlsr);


         if (propo)
           {
        	 std::cout<<"ok"<<std::endl;


        	 //listen
        	 // Create the socket if not already
        	 if(node->GetId()==0){
        		 if (!m_socket)
        		 {
        			 m_socket = Socket::CreateSocket (GetNode (), m_tid);

                   Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal ();

                   std::cout<<addr<<std::endl;

                   InetSocketAddress address = InetSocketAddress (addr, 49153);

                   m_local = address;


//                   m_socket->Bind(address);

					 if (m_socket->Bind (address) == -1)
					 {
						 NS_FATAL_ERROR ("Failed to bind socket");
					 }

					 m_socket->Listen ();

					 if (m_recvSocket == 0)
					 {
						 m_recvSocket = Socket::CreateSocket (node,
								 UdpSocketFactory::GetTypeId ());
						 m_recvSocket->SetAllowBroadcast (true);
						 InetSocketAddress inetAddr (Ipv4Address::GetAny (), 49153);
						 m_recvSocket->SetRecvCallback (MakeCallback (&SimulationProposalReactive::HandleRead, this));
						 if (m_recvSocket->Bind (inetAddr))
						 {
							 NS_FATAL_ERROR ("Failed to bind() OLSR socket");
						 }
						 m_recvSocket->SetRecvPktInfo (true);
						 m_recvSocket->ShutdownSend ();
					 }


//              	 //自分で作成
//               SimulationProposal simulation;
//               sink_core_candidate_reactive = simulation.GetSinkCoreCandidate();
//               uint32_t max_core_candidate = 0;
//
//               for(std::map<Ipv4Address, uint32_t>::const_iterator i = sink_core_candidate_reactive.begin();
//                  i != sink_core_candidate_reactive.end();i++)
//                 {
//                    if(max_core_candidate < i->second)
//                    {
//                    	max_core_candidate = i->second;
//                    }
//                 }
//
//               for(std::map<Ipv4Address, uint32_t>::const_iterator i = sink_core_candidate_reactive.begin();
//                  i != sink_core_candidate_reactive.end();i++)
//                 {
//            	   	   if((max_core_candidate = i->second))
//            	   	   {
//            	   		  Ptr<Packet> packet;
//            	   		  SimulationHeader header;
//            	   		  header.SetSeq (m_seq++);
//            	   		  header.SetSize (m_pktSize);
//            	   		  SimulationHeader::Core &core = header.GetCore();
//
//            	   		  core.SetDestinationAddress(i->first);
//
//            	   		  NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
//            	   		  packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
//            	   		  // Trace before adding header, for consistency with PacketSink
//            	   		  packet->AddHeader (header);
//
//            	   		  m_socket->Connect (i->first);
////            	   		  m_peer = InetSocketAddress (i->first, 49290);
////            	   		  m_socket->SendTo(packet, 0, i->first);
//
//                       }
//                 }

					 if (addressUtils::IsMulticast (address))
					 {
						 Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
						 if (udpSocket)
						 {
							 // equivalent to setsockopt (MCAST_JOIN_GROUP)
							 udpSocket->MulticastJoinGroup (0, address);
						 }
						 else
						 {
							 NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
						 }
					 }

					 m_socket->SetRecvCallback (MakeCallback (&SimulationProposalReactive::HandleRead, this));
					 m_socket->SetAcceptCallback (
							 MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
							 MakeCallback (&SimulationProposalReactive::HandleAccept, this));
					 m_socket->SetCloseCallbacks (
							 MakeCallback (&SimulationProposalReactive::HandlePeerClose, this),
							 MakeCallback (&SimulationProposalReactive::HandlePeerError, this));

					 m_socket->SetConnectCallback (
							 MakeCallback (&SimulationProposalReactive::ConnectionSucceeded, this),
							 MakeCallback (&SimulationProposalReactive::ConnectionFailed, this));
        		 }

       	       ScheduleStartEvent ();

        	 }
        	 else
        	 {
        		 if(!m_socket)
        		 {

        			 Ipv4Address addr = ipv4->GetAddress (1, 0).GetLocal ();
        			 InetSocketAddress address = InetSocketAddress (addr, 49153);

        			 m_peer = address;

        			 m_socket = Socket::CreateSocket (node, m_tid);

//
//        			 int ret = -1;
//
//        			 if (! m_peer.IsInvalid())
//        			 {
//        				 ret = m_socket->Bind (m_peer);
//        			 }
//        			 else
//        			 {
//        				 if (Inet6SocketAddress::IsMatchingType (m_peer))
//        				 {
//        					 ret = m_socket->Bind6 ();
//        				 }
//        				 else if (InetSocketAddress::IsMatchingType (m_peer) ||
//        						 PacketSocketAddress::IsMatchingType (m_peer))
//        				 {
//        					 ret = m_socket->Bind ();
//        				 }
//        			 }
//
//        			 if (ret == -1)
//        			 {
//        				 NS_FATAL_ERROR ("Failed to bind socket");
//        			 }

        			 if (m_socket->Bind (m_peer) == -1)
        			 {
        				 NS_FATAL_ERROR ("Failed to bind socket");
        			 }

        			 if (addressUtils::IsMulticast (address))
        			 {
        				 Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
        				 if (udpSocket)
        				 {
        					 // equivalent to setsockopt (MCAST_JOIN_GROUP)
        					 udpSocket->MulticastJoinGroup (0, address);
        				 }
        				 else
        				 {
        					 NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
        				 }
        			 }
        			 m_socket->SetAllowBroadcast (true);

        			 m_socket->Listen ();
        			 if (m_recvSocket == 0)
        			 {
        				 m_recvSocket = Socket::CreateSocket (node,
        						 UdpSocketFactory::GetTypeId ());
        				 m_recvSocket->SetAllowBroadcast (true);
        				 InetSocketAddress inetAddr (Ipv4Address::GetAny (), 49153);
        				 m_recvSocket->SetRecvCallback (MakeCallback (&SimulationProposalReactive::HandleRead, this));
        				 if (m_recvSocket->Bind (inetAddr))
        				 {
        					 NS_FATAL_ERROR ("Failed to bind() OLSR socket");
        				 }
        				 m_recvSocket->SetRecvPktInfo (true);
        				 m_recvSocket->ShutdownSend ();
        			 }


        			 m_socket->SetRecvCallback (MakeCallback (&SimulationProposalReactive::HandleRead, this));
        			 m_socket->SetAcceptCallback (
        					 MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
							 MakeCallback (&SimulationProposalReactive::HandleAccept, this));
        			 m_socket->SetCloseCallbacks (
        					 MakeCallback (&SimulationProposalReactive::HandlePeerClose, this),
							 MakeCallback (&SimulationProposalReactive::HandlePeerError, this));
        		 }
        	 }



//                   	 if (!m_socket_local)
//                   		 {
//                   		 m_socket_local = Socket::CreateSocket (node, m_tid);
//                   		 int ret = -1;
//
//                   		 if (! m_local.IsInvalid())
//                   			 {
//                   			 NS_ABORT_MSG_IF ((Inet6SocketAddress::IsMatchingType (m_peer) && InetSocketAddress::IsMatchingType (m_local)) ||
//                   								   (InetSocketAddress::IsMatchingType (m_peer) && Inet6SocketAddress::IsMatchingType (m_local)),
//                   								   "Incompatible peer and local address IP version");
//                   			 ret = m_socket_local->Bind (m_local);
//                   			 }
//                   		 else
//                   			 {
//                   			 if (Inet6SocketAddress::IsMatchingType (m_peer))
//                   					{
//                   				 ret = m_socket_local->Bind6 ();
//                   					}
//                   			 else if (InetSocketAddress::IsMatchingType (m_peer) ||
//                   						   PacketSocketAddress::IsMatchingType (m_peer))
//                   					{
//                   				 ret = m_socket_local->Bind ();
//                   					}
//                   			 }
//
//                   			  if (ret == -1)
//                   				    {
//                   				  NS_FATAL_ERROR ("Failed to bind socket");
//                   				    }
//
//                   			  m_socket_local->Connect (m_peer);
////                   			  m_socket_local->SetAllowBroadcast (true);
//                   			  m_socket_local->ShutdownRecv ();
//
//                   			  m_socket_local->SetConnectCallback (
//                   				MakeCallback (&SimulationProposalReactive::ConnectionSucceeded, this),
//                   				MakeCallback (&SimulationProposalReactive::ConnectionFailed, this));
//                   		 }

//                   	 m_cbrRateFailSafe = m_cbrRate;
//
//                   	 // Insure no pending event
//                   	 CancelEvents ();
//                   	 // If we are not yet connected, there is nothing to do here
//                   	 // The ConnectionComplete upcall will start timers at that time
//                   	 //if (!m_connected) return;
//                   	 ScheduleStartEvent ();
//
//        			 }
//        		 }

           }
	  }
//  }




}

void SimulationProposalReactive::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();

  //listen
  while(!m_socketList.empty ()) //these are accepted sockets, close them
      {
        Ptr<Socket> acceptedSocket = m_socketList.front ();
        m_socketList.pop_front ();
        acceptedSocket->Close ();
      }
    if (m_socket)
      {
        m_socket->Close ();
        m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      }

  //send
  if(m_socket_local != 0)
    {
      m_socket_local->Close ();
    }
  else
    {
      NS_LOG_WARN ("simulation_proposal found null socket to close in StopApplication");
    }
}

void SimulationProposalReactive::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  if (m_sendEvent.IsRunning () && m_cbrRateFailSafe == m_cbrRate )
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
      int64x64_t bits = delta.To (Time::S) * m_cbrRate.GetBitRate ();
      m_residualBits += bits.GetHigh ();
    }
  m_cbrRateFailSafe = m_cbrRate;
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_startStopEvent);
  // Canceling events may cause discontinuity in sequence number if the
  // SeqTsSizeHeader is header, and m_unsentPacket is true
  if (m_unsentPacket)
    {
      NS_LOG_DEBUG ("Discarding cached packet upon CancelEvents ()");
    }
  m_unsentPacket = 0;
}

// Event handlers
void SimulationProposalReactive::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();

  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
//  ScheduleStopEvent ();
}

void SimulationProposalReactive::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}

// Private helpers
void SimulationProposalReactive::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
//      NS_ABORT_MSG_IF (m_residualBits > m_pktSize * 8, "Calculation to compute next send time will overflow");
//      uint32_t bits = m_pktSize * 8 - m_residualBits;
//      NS_LOG_LOGIC ("bits = " << bits);
//      Time nextTime (Seconds (bits /
//                              static_cast<double>(m_cbrRate.GetBitRate ()))); // Time till next packet
//      NS_LOG_LOGIC ("nextTime = " << nextTime.As (Time::S));
//      m_sendEvent = Simulator::Schedule (nextTime,
//                                         &SimulationProposalReactive::SendPacket, this);


	  if(send_count<1){

		  m_residualBits = 0;
		  NS_ABORT_MSG_IF (m_residualBits > m_pktSize * 8, "Calculation to compute next send time will overflow");
		  uint32_t bits = m_pktSize * 8 - m_residualBits;
		  NS_LOG_LOGIC ("bits = " << bits);
		  Time nextTime (Seconds (bits /
				  static_cast<double>(m_cbrRate.GetBitRate ()))); // Time till next packet
		  NS_LOG_LOGIC ("nextTime = " << nextTime.As (Time::S));
		  m_sendEvent = Simulator::Schedule (nextTime,
				  &SimulationProposalReactive::SendPacket, this);
		  send_count += 1;
	  	  }
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}

void SimulationProposalReactive::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  send_count = 0;


  //タスクの要求の発生
  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());

  std::exponential_distribution<> dist(0.1);


  // 指数分布で乱数を生成する
  Time task_interval = Seconds(dist(engine));
  std::cout<<task_interval<<std::endl;


//  Time offInterval = Seconds (m_offTime->GetValue ());
//  NS_LOG_LOGIC ("start at " << offInterval.As (Time::S));
  m_startStopEvent = Simulator::Schedule (task_interval, &SimulationProposalReactive::StartSending, this);
}


void SimulationProposalReactive::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval.As (Time::S));
  m_startStopEvent = Simulator::Schedule (onInterval, &SimulationProposalReactive::StopSending, this);
}


void SimulationProposalReactive::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  std::cout<<"send"<<std::endl;

  Ptr<Packet> packet;
  if (m_unsentPacket)
    {
      packet = m_unsentPacket;
    }
  else if (m_enableSeqTsSizeHeader)
    {
//      Address from, to;
//      m_socket_local->GetSockName (from);
//      m_socket_local->GetPeerName (to);
//      SeqTsSizeHeader header;
//      header.SetSeq (m_seq++);
//      header.SetSize (m_pktSize);
//      NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
//      packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
//      // Trace before adding header, for consistency with PacketSink
//      m_txTraceWithSeqTsSize (packet, from, to, header);
//      packet->AddHeader (header);
	  //自分で作成
	  Ptr<Node> node = GetNode();
	  Ptr<Application> app;
	  Ptr<SimulationProposal> propo;
	  for (uint32_t i = 0; i < node->GetNApplications(); i++)
	  {
		  app = node->GetApplication(i);
		  propo = DynamicCast<SimulationProposal> (app);
		  //NS_ASSERT (listOlsr);


		  if (propo)
		  {
			  sink_core_candidate_reactive = propo->GetSinkCoreCandidate();
			  std::cout<<sink_core_candidate_reactive.size()<<std::endl;
			  uint32_t max_core_candidate = 0;

			  for(std::map<Ipv4Address, uint32_t>::const_iterator i = sink_core_candidate_reactive.begin();
					  i != sink_core_candidate_reactive.end();i++)
			  {
				  if(max_core_candidate < i->second)
				  {
					  max_core_candidate = i->second;
				  }
			  }

			  for(std::map<Ipv4Address, uint32_t>::const_iterator i = sink_core_candidate_reactive.begin();
					  i != sink_core_candidate_reactive.end();i++)
			  {
				  if((max_core_candidate == i->second))
				  {
//					  Address address = i->first.ConvertTo();
					  InetSocketAddress address = InetSocketAddress (i->first, 49153);

					  m_socket->Connect (address);

					  Ptr<Packet> packet;
					  Address from, to;
					  std::cout<<m_socket->Connect (address)<<std::endl;

					  m_socket->GetSockName (from);
					  m_socket->GetPeerName (to);

					  SimulationHeader header;
					  header.SetSeq (m_seq++);
					  header.SetSize (m_pktSize);
					  header.GetCore();

					  header.SetDestinationAddress(i->first);

					  NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
					  packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
					  // Trace before adding header, for consistency with PacketSink
					  m_txTraceWithSeqTsSize2 (packet, from, to, header);
					  packet->AddHeader (header);


					  m_socket->SendTo(packet, 0, address);

//					  break;

				  }
			  }
		  }
	  }

    }
  else
  {
	  packet = Create<Packet> (m_pktSize);
  }

//  int actual = m_socket->Send (packet);
//  if ((unsigned) actual == m_pktSize)
//    {
//      m_txTrace (packet);
//      m_totBytes += m_pktSize;
//      m_unsentPacket = 0;
//      Address localAddress;
//      m_socket->GetSockName (localAddress);
//      if (InetSocketAddress::IsMatchingType (m_peer))
//        {
//          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S)
//                       << " on-off application sent "
//                       <<  packet->GetSize () << " bytes to "
//                       << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
//                       << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
//                       << " total Tx " << m_totBytes << " bytes");
//          m_txTraceWithAddresses (packet, localAddress, InetSocketAddress::ConvertFrom (m_peer));
//        }
//      else if (Inet6SocketAddress::IsMatchingType (m_peer))
//        {
//          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S)
//                       << " on-off application sent "
//                       <<  packet->GetSize () << " bytes to "
//                       << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
//                       << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
//                       << " total Tx " << m_totBytes << " bytes");
//          m_txTraceWithAddresses (packet, localAddress, Inet6SocketAddress::ConvertFrom(m_peer));
//        }
//    }
//  else
//    {
//      NS_LOG_DEBUG ("Unable to send packet; actual " << actual << " size " << m_pktSize << "; caching for later attempt");
//      m_unsentPacket = packet;
//    }

  m_residualBits = 0;
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();

}


void SimulationProposalReactive::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void SimulationProposalReactive::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_FATAL_ERROR ("Can't connect");
}

void SimulationProposalReactive::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0)
        { //EOF
          break;
        }
      m_totalRx += packet->GetSize ();
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S)
                       << " packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S)
                       << " packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom(from).GetIpv6 ()
                       << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      socket->GetSockName (localAddress);
      m_rxTrace (packet, from);
      m_rxTraceWithAddresses (packet, from, localAddress);

      if (m_enableSeqTsSizeHeader)
        {
          PacketReceived (packet, from, localAddress, socket, packet);
        }
    }
}

void
SimulationProposalReactive::PacketReceived (const Ptr<Packet> &p, const Address &from,
                            const Address &localAddress, Ptr<Socket> socket, Ptr<Packet> packet)
{
  SimulationHeader header;
  Ptr<Packet> buffer;

  Ptr<Node> node = GetNode();
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

//  Ptr<Packet> receivedPacket;
//  Address sourceAddress;
//  receivedPacket = socket->RecvFrom (from);

  std::cout<<"receive"<<std::endl;

  auto itBuffer = m_buffer.find (from);
  if (itBuffer == m_buffer.end ())
    {
      itBuffer = m_buffer.insert (std::make_pair (from, Create<Packet> (0))).first;
    }

  buffer = itBuffer->second;
  buffer->AddAtEnd (p);
  buffer->PeekHeader (header);

  NS_ABORT_IF (header.GetSize () == 0);


//	  NS_LOG_DEBUG ("Removing packet of size " << header.GetSize () << " from buffer of size " << buffer->GetSize ());
//	  Ptr<Packet> complete = buffer->CreateFragment (0, static_cast<uint32_t> (header.GetSize ()));
//	  buffer->RemoveAtStart (static_cast<uint32_t> (header.GetSize ()));

  p->RemoveHeader (header);


  if(header.GetMessageType() == SimulationHeader::CORE_MESSAGE)
  {
	  //coreノードの処理をコーディングした関数
	  std::cout<<"core"<<std::endl;
	  while (buffer->GetSize () >= header.GetSize ())
	  {
		  buffer->RemoveAtStart (static_cast<uint32_t> (header.GetSize ()));
		  if (buffer->GetSize () > header.GetSerializedSize ())
		  {
			  buffer->PeekHeader (header);
		  }
		  else
		  {
			  break;
		  }
	  }
	  m_floodingEvent = Simulator::Schedule (flooding_interval, &SimulationProposalReactive::CoreFlooding, this);
  }

      //floodingメッセージを受け取った際のアクション
  if(header.GetMessageType() == SimulationHeader::FLOODING_MESSAGE)
  {
	  std::cout<<"flooding"<<std::endl;
//		  std::cout<<node->GetId()<<std::endl;

	  RespondFlooding(header);

	  SimulationHeader::Flooding &flooding = header.GetFlooding();

	  if(flooding_flag == 0){
		  flooding_flag = 1;
		  if(flooding.GetTimeToLive() > 1)
		  {
//				  std::cout<<int(flooding.GetSerializedSize())<<std::endl;
//				  std::cout<<int(flooding.GetTimeToLive())<<std::endl;

			  NodeFlooding(header);
//				  RespondFlooding(header);
		  }
	  }
	  else if(flooding_flag == 1 || flooding.GetTimeToLive () == 1)
	  {
		  while (buffer->GetSize () >= header.GetSize ())
		  {
			  buffer->RemoveAtStart (static_cast<uint32_t> (header.GetSize ()));
			  if (buffer->GetSize () > header.GetSerializedSize ())
			  {
				  buffer->PeekHeader (header);
			  }
			  else
			  {
				  break;
			  }
		  }
	  }
  }

  if(header.GetMessageType() == SimulationHeader::FLOODINGRETURN_MESSAGE)
  {
	  std::cout<<"floodingreturn"<<std::endl;

	  Ipv4Address addr = header.GetDestinationAddress();
	  SimulationHeader::FloodingReturn &floodingreturn = header.GetFloodingReturn();
	  m_resource_flooding.insert(std::make_pair(addr, floodingreturn.GetResource()));
	  std::cout<<m_resource_flooding.size()<<std::endl;


	  if(floodingreturn_flag == 0){

		  m_floodingreturnEvent = Simulator::Schedule (floodingreturn_interval, &SimulationProposalReactive::RespondFloodingReturn, this);
//		  RespondFloodingReturn(header);

	  }
	  floodingreturn_flag = 1;

  }
  if(header.GetMessageType() == SimulationHeader::NOTICE_MESSAGE)
  {
	  std::cout<<"notice"<<std::endl;

	  SimulationHeader::Notice &notice = header.GetNotice();
	  m_resource_flooding = notice.GetResource();
	  std::cout<<m_resource_flooding.size()<<std::endl;

	  std::vector<uint32_t> resource_tempo;
	  for(std::map<Ipv4Address, uint32_t>::const_iterator iter = m_resource_flooding.begin ();
			  iter != m_resource_flooding.end (); iter++)
	  {
		  resource_tempo.push_back(iter->second);
	  }
	  for(int i = 0; i<int(resource_tempo.size())-1; i++)
	  {
		  for(int j = int(resource_tempo.size())-1; j>i; j--)
		  {
			  if(resource_tempo[i] < resource_tempo[j])
			  {
				  uint32_t temp = resource_tempo[i];
				  resource_tempo[i] = resource_tempo[j];
				  resource_tempo[j] = temp;
			  }
		  }
		  std::cout<<resource_tempo[i]<<std::endl;
//		  for(std::map<Ipv4Address, uint32_t>::const_iterator iter = m_resource_flooding.begin ();
//		  	 			  iter != m_resource_flooding.end (); iter++)
//		  {
//			  if(resource_tempo[i] == iter->second)
//			  {
//				  m_sort_resource_flooding[iter->first] = iter->second;
//			  }
//		  }
	  }



//	  for(std::map<Ipv4Address, uint32_t>::const_iterator iter = m_sort_resource_flooding.begin ();
//	 		  	 			  iter != m_sort_resource_flooding.end (); iter++)
//	  {
//		  std::cout<<iter->second<<std::endl;
//	  }

	  uint32_t sum_computing = 0;
	  computing_resource_sum = 0;
	  for(int i = 0; i<int(resource_tempo.size())-1; i++)
	  {
		  if(sum_computing < 50)
		  {
			  sum_computing += resource_tempo[i];
			  for(std::map<Ipv4Address, uint32_t>::const_iterator iter = m_resource_flooding.begin ();
					  iter != m_resource_flooding.end (); iter++)
			  {
				if(iter->second == resource_tempo[i])
				{
					computing_node_address.push_back(iter->first);
//					computing_resource_sum += iter->second;
				}
			  }
		  }
		  else if(sum_computing >= 50)
		  {
			  break;
		  }
	  }

	  sum_computing = 0;
	  computing_resource_each.clear();
	  for(int i = 0; i<int(resource_tempo.size())-1; i++)
	  {
		  if(sum_computing < 50)
		  {
			  sum_computing += resource_tempo[i];
			  for(std::map<Ipv4Address, uint32_t>::const_iterator iter = m_resource_flooding.begin ();
	  					  iter != m_resource_flooding.end (); iter++)
			  {
				  if(iter->second == resource_tempo[i])
				  {
					  computing_resource_each.insert(std::make_pair(iter->first, iter->second));
				  }
	  		  }
		  }
		  else if(sum_computing >= 50)
		  {
			  break;
		  }
	  }
	  Ptr<Application> app;
	  Ptr<SimulationProposal> propo;
	  for (uint32_t i = 0; i < node->GetNApplications(); i++)
	  {
		  app = node->GetApplication(i);
		  propo = DynamicCast<SimulationProposal> (app);

		  if (propo)
		  {
			  for(std::map<Ipv4Address, std::vector<Ipv4Address>>::const_iterator i = propo->sink_candidate_addr.begin ();
  					  i != propo->sink_candidate_addr.end (); i++)
			  {
				  for(std::vector<Ipv4Address>::const_iterator i2 =i->second.begin();
						  i2 != i->second.end(); i2++)
				  {
					  for(std::vector<Ipv4Address>::const_iterator i3 = computing_node_address.begin();
							  i3 != computing_node_address.end(); i3++)
					  {
						  if(*i2 == *i3)
						  {
							  propo->sink_core_candidate[i->first] -= m_resource_flooding[*i3];
						  }
					  }
				  }
			  }
		  }
	  }


//	  std::cout<<computing_resource_each.size()<<std::endl;



//	  m_floodingreturnEvent = Simulator::Schedule (floodingreturn_interval, &SimulationProposalReactive::CoreComputing, this);
	  CoreComputing();


  }

  if(header.GetMessageType() == SimulationHeader::CORECOMPUTING_MESSAGE)
  {
	  std::cout<<"corecomputing"<<std::endl;
	  SimulationHeader::CoreComputing &corecomputing = header.GetCoreComputing();
	  std::cout<<corecomputing.GetAddr().size()<<std::endl;

	  computing_resource_each = corecomputing.GetAddr();
	  DistributeComputing();
  }

  if(header.GetMessageType() == SimulationHeader::COMPUTING_MESSAGE)
  {
  	  std::cout<<"computing"<<std::endl;
//  	  SimulationHeader::Computing &computing = header.GetComputing();

  	  //計算機資源の増減を表現
  	  core_tempo = header.GetDestinationAddress();
  	  Computing(header);


//  	  std::cout<<header.GetDestinationAddress()<<std::endl;


  }
  if(header.GetMessageType() == SimulationHeader::NORMAL_MESSAGE)
  {
	  std::cout<<"normal"<<std::endl;
	  count_computing += 1;
	  std::cout<<computing_resource_each.size()<<std::endl;
	  if(count_computing == computing_resource_each.size())
	  {
		  std::cout<<"normal"<<std::endl;
		  FinishComputing();
	  }
  }



	if(header.GetMessageType() == SimulationHeader::STOPCOMPUTING_MESSAGE)
	{
		std::cout<<"stop"<<std::endl;
		SimulationHeader::StopComputing &stopcomputing = header.GetStopComputing();
		computing_resource_each3 = stopcomputing.GetAddr();
		Ptr<Node> node = GetNode();
		Ptr<Application> app;
		Ptr<SimulationProposal> propo;
		for (uint32_t i = 0; i < node->GetNApplications(); i++)
		{
			app = node->GetApplication(i);
			propo = DynamicCast<SimulationProposal> (app);

			if (propo)
			{
				for(std::map<Ipv4Address, std::vector<Ipv4Address>>::const_iterator i = propo->sink_candidate_addr.begin ();
						i != propo->sink_candidate_addr.end (); i++)
				{
					for(std::vector<Ipv4Address>::const_iterator i2 =i->second.begin();
							i2 != i->second.end(); i2++)
					{
						for(std::map<Ipv4Address, uint32_t>::const_iterator i3 = computing_resource_each3.begin();
								i3 != computing_resource_each3.end(); i3++)
						{
							if(*i2 == i3->first)
							{
								propo->sink_core_candidate[i->first] += computing_resource_each3[i3->first];
							}
						}
					}
				}
			}
		}
	}


//      m_rxTraceWithSeqTsSize2 (complete, from, localAddress, header);



}




void SimulationProposalReactive::CoreFlooding ()
{
  NS_LOG_FUNCTION (this);

  Ptr<Node> node = GetNode();
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();


  flooding_flag = 1;
  if(!m_socket_local)
  {
	  m_socket_local = Socket::CreateSocket (node, m_tid);
	  m_socket_local->SetAllowBroadcast (true);

	  m_socket_local->SetIpTtl (255);
	  m_socket_local->SetRecvPktInfo (true);
	  InetSocketAddress inetAddr (ipv4->GetAddress (1, 0).GetLocal (), 49154);
	  if (m_socket_local->Bind (inetAddr))
	  {
		  NS_FATAL_ERROR ("Failed to bind() socket");
	  }
  }

  Ipv4InterfaceAddress iface = ipv4->GetAddress (1, 0);
  Ipv4Address sourceaddress = iface.GetLocal();

  uint8_t h = 2;
  Address from, to;
  m_socket_local->GetSockName (from);
  m_socket_local->GetPeerName (to);

  Ptr<Packet> packet;
  SimulationHeader header;
  header.SetSeq (m_seq++);
  header.SetSize (m_pktSize);
  header.SetDestinationAddress(sourceaddress);

//  header.GetFlooding();
  SimulationHeader::Flooding &flooding = header.GetFlooding();
  flooding.SetTimeToLive(h);

//  std::cout<<int(flooding.GetTimeToLive())<<std::endl;

  NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
  packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
  // Trace before adding header, for consistency with PacketSink
  m_txTraceWithSeqTsSize2 (packet, from, to, header);

  packet->AddHeader (header);

  Ptr<Packet> pkt = packet->Copy ();


  Ipv4Address bcast = iface.GetLocal ().GetSubnetDirectedBroadcast (iface.GetMask ());
//  std::cout<<bcast<<std::endl;
  m_socket_local->SendTo (pkt, 0, InetSocketAddress (bcast, 49153));

}

void SimulationProposalReactive::NodeFlooding (SimulationHeader header)
{
  NS_LOG_FUNCTION (this);

  Ptr<Node> node = GetNode();
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

  SimulationHeader::Flooding &flooding = header.GetFlooding();
  uint8_t ttl = flooding.GetTimeToLive() - 1;
  if(!m_socket_local)
  {
	  m_socket_local = Socket::CreateSocket (node, m_tid);
	  m_socket_local->SetAllowBroadcast (true);

	  m_socket_local->SetIpTtl (255);
	  m_socket_local->SetRecvPktInfo (true);
	  InetSocketAddress inetAddr (ipv4->GetAddress (1, 0).GetLocal (), 49154);
	  if (m_socket_local->Bind (inetAddr))
	  {
		  NS_FATAL_ERROR ("Failed to bind() socket");
	  }
  }


  Ptr<Packet> packet;
//  SimulationHeader header;
  header.SetSeq (m_seq++);
  header.SetSize (m_pktSize);

  //  header.GetFlooding();
//    SimulationHeader::Flooding &flooding = header.GetFlooding();
//    uint8_t ttl = flooding.GetTimeToLive() - 1;
    flooding.SetTimeToLive(ttl);


  NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
  packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
  // Trace before adding header, for consistency with PacketSink
  packet->AddHeader (header);

  Ptr<Packet> pkt = packet->Copy ();


  Ipv4InterfaceAddress iface = ipv4->GetAddress (1, 0);
  Ipv4Address bcast = iface.GetLocal ().GetSubnetDirectedBroadcast (iface.GetMask ());
//  std::cout<<bcast<<std::endl;
  m_socket_local->SendTo (pkt, 0, InetSocketAddress (bcast, 49153));

}

void SimulationProposalReactive::RespondFlooding (SimulationHeader header)
{
	//自分で作成
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

	Ptr<Application> app;
	Ptr<SimulationProposal> propo;

	Ptr<Ipv4RoutingProtocol> ipv4_proto = ipv4->GetRoutingProtocol ();
	Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (ipv4_proto);
	if (list)
	{
		int16_t priority;
		Ptr<Ipv4RoutingProtocol> listProto;
		Ptr<olsr::RoutingProtocol> protocol;
		for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
		{
			listProto = list->GetRoutingProtocol (i, priority);
			protocol = DynamicCast<olsr::RoutingProtocol> (listProto);
			if (protocol){
	//					  Address address = i->first.ConvertTo();
				InetSocketAddress address = InetSocketAddress (header.GetDestinationAddress(), 49153);

				if(header.GetDestinationAddress() == ipv4->GetAddress(1, 0).GetLocal())
				{
					break;
				}
				m_socket->Connect (address);

				Ptr<Packet> packet;
				Address from, to;

				m_socket->GetSockName (from);
				m_socket->GetPeerName (to);

				Ipv4Address addr = ipv4->GetAddress(1, 0).GetLocal();

				SimulationHeader header;
				header.SetSeq (m_seq++);
				header.SetSize (m_pktSize);
				header.SetDestinationAddress(addr);
				SimulationHeader::FloodingReturn &floodingreturn = header.GetFloodingReturn();
				uint8_t resource = protocol->m_resource;
				uint32_t resource_return = static_cast<uint32_t>(resource);
				floodingreturn.SetResource(resource_return);

				NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
				packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
				// Trace before adding header, for consistency with PacketSink
				m_txTraceWithSeqTsSize2 (packet, from, to, header);
				packet->AddHeader (header);

				m_socket->SendTo(packet, 0, address);
			}

		}
	}
}


void SimulationProposalReactive::RespondFloodingReturn ()
{
	//自分で作成
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

	Ptr<Application> app;
	Ptr<SimulationProposal> propo;

	for (uint32_t i = 0; i < node->GetNApplications(); i++)
	{
		app = node->GetApplication(i);
		propo = DynamicCast<SimulationProposal> (app);
	         //NS_ASSERT (listOlsr);


		if (propo)
		{
	//					  Address address = i->first.ConvertTo();
//			m_peer = propo->m_peer;

			InetSocketAddress address = InetSocketAddress ("10.1.1.1", 49153);

			m_socket->Connect (address);

			Ptr<Packet> packet;
			Address from, to;

			m_socket->GetSockName (from);
			m_socket->GetPeerName (to);

//			std::cout<<m_socket->GetSockName (from)<<std::endl;

			Ipv4Address addr = ipv4->GetAddress(1, 0).GetLocal();

			SimulationHeader header;
			header.SetSeq (m_seq++);
			header.SetSize (m_pktSize);
			header.SetDestinationAddress(addr);
			SimulationHeader::Notice &notice = header.GetNotice();

			notice.SetResource(m_resource_flooding);
			m_resource_flooding = notice.m_resource_floodingreturn;
//			std::cout<<m_resource_flooding.size()<<std::endl;

			NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
			packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
			// Trace before adding header, for consistency with PacketSink
			m_txTraceWithSeqTsSize2 (packet, from, to, header);
			packet->AddHeader (header);

			m_socket->SendTo(packet, 0, address);

		}
	}
}

void SimulationProposalReactive::CoreComputing()
{
	//自分で作成
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

	Ptr<Application> app;
	Ptr<SimulationProposal> propo;

	for (uint32_t i = 0; i < node->GetNApplications(); i++)
	{
		app = node->GetApplication(i);
		propo = DynamicCast<SimulationProposal> (app);
		         //NS_ASSERT (listOlsr);

		if (propo)
		{
			uint32_t max_core_candidate = 0;

			for(std::map<Ipv4Address, uint32_t>::const_iterator i = sink_core_candidate_reactive.begin();
					i != sink_core_candidate_reactive.end();i++)
			{
				if(max_core_candidate < i->second)
				{
					max_core_candidate = i->second;
				}
			}

			for(std::map<Ipv4Address, uint32_t>::const_iterator i = sink_core_candidate_reactive.begin();
					i != sink_core_candidate_reactive.end();i++)
			{
				if((max_core_candidate == i->second))
				{
//					  Address address = i->first.ConvertTo();
					InetSocketAddress address = InetSocketAddress (i->first, 49153);

					m_socket->Connect (address);

					Ptr<Packet> packet;
					Address from, to;
//					std::cout<<m_socket->Connect (address)<<std::endl;

					m_socket->GetSockName (from);
					m_socket->GetPeerName (to);

					SimulationHeader header;
					header.SetSeq (m_seq++);
					header.SetSize (m_pktSize);
					SimulationHeader::CoreComputing &corecomputing = header.GetCoreComputing();


					corecomputing.SetAddr(computing_resource_each);

					NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
					packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
					// Trace before adding header, for consistency with PacketSink
					m_txTraceWithSeqTsSize2 (packet, from, to, header);
					packet->AddHeader (header);

					m_socket->SendTo(packet, 0, address);

//					  break;


				}
			}
		}
	}
}


void SimulationProposalReactive::DistributeComputing()
{
	//自分で作成
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

	Ptr<Application> app;
	Ptr<SimulationProposal> propo;

	for (uint32_t i = 0; i < node->GetNApplications(); i++)
	{
		app = node->GetApplication(i);
		propo = DynamicCast<SimulationProposal> (app);
		         //NS_ASSERT (listOlsr);

		if (propo)
		{
			computing_resource_each2 = computing_resource_each;
			for(std::map<Ipv4Address, uint32_t>::const_iterator i = computing_resource_each.begin();
								i != computing_resource_each.end();i++)
			{
				computing_resource_sum += i->second;
			}

			for(std::map<Ipv4Address, uint32_t>::const_iterator i = computing_resource_each.begin();
											i != computing_resource_each.end();i++)
			{
				computing_resource_each[i->first] = computing_resource_each[i->first]/computing_resource_sum;
			}
			for(std::map<Ipv4Address, uint32_t>::const_iterator i = computing_resource_each.begin();
					i != computing_resource_each.end();i++)
			{
//					  Address address = i->first.ConvertTo();
				InetSocketAddress address = InetSocketAddress (i->first, 49153);

				m_socket->Connect (address);

				Ipv4InterfaceAddress iface = ipv4->GetAddress (1, 0);
				Ipv4Address sourceaddress = iface.GetLocal();

				Ptr<Packet> packet;
				Address from, to;
//					std::cout<<m_socket->Connect (address)<<std::endl;

				m_socket->GetSockName (from);
				m_socket->GetPeerName (to);

				SimulationHeader header;
				header.SetSeq (m_seq++);
				header.SetSize (m_pktSize);
				header.SetDestinationAddress(sourceaddress);
				SimulationHeader::Computing &computing = header.GetComputing();
				computing.SetComputingResource(i->second);

				NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
				packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
				// Trace before adding header, for consistency with PacketSink
				m_txTraceWithSeqTsSize2 (packet, from, to, header);
				packet->AddHeader (header);

				m_socket->SendTo(packet, 0, address);

			}
		}
	}
}


void SimulationProposalReactive::Computing (SimulationHeader header)
{
	//自分で作成
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

	SimulationHeader::Computing &computing = header.GetComputing();
	uint32_t calcu = computing.GetComputingResource();
	Ptr<Application> app;
	Ptr<SimulationProposal> propo;

	Ptr<Ipv4RoutingProtocol> ipv4_proto = ipv4->GetRoutingProtocol ();
	Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (ipv4_proto);
	if (list)
	{
		int16_t priority;
		Ptr<Ipv4RoutingProtocol> listProto;
		Ptr<olsr::RoutingProtocol> protocol;
		for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
		{
			listProto = list->GetRoutingProtocol (i, priority);
			protocol = DynamicCast<olsr::RoutingProtocol> (listProto);
			if (protocol)
			{
				std::cout<<protocol->m_resource_num<<std::endl;
				computing_interval = Seconds((task * calcu) / protocol->m_resource_num);
				resource_tempo = protocol->m_resource_num;
				protocol->m_resource_num = 0;
				m_computingEvent = Simulator::Schedule (computing_interval, &SimulationProposalReactive::StopComputing, this);
			}

		}
	}
}

void SimulationProposalReactive::StopComputing ()
{
	//自分で作成
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

	Ptr<Application> app;
	Ptr<SimulationProposal> propo;

	Ptr<Ipv4RoutingProtocol> ipv4_proto = ipv4->GetRoutingProtocol ();
	Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (ipv4_proto);
	if (list)
	{
		int16_t priority;
		Ptr<Ipv4RoutingProtocol> listProto;
		Ptr<olsr::RoutingProtocol> protocol;
		for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
		{
			listProto = list->GetRoutingProtocol (i, priority);
			protocol = DynamicCast<olsr::RoutingProtocol> (listProto);
			if (protocol)
			{
				std::cout<<protocol->m_resource_num<<std::endl;
				protocol->m_resource_num = resource_tempo;
			}
		}
	}
	RespondComputing(core_tempo);
}

void SimulationProposalReactive::RespondComputing (Ipv4Address addr)
{
	//自分で作成
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

	Ptr<Application> app;
	Ptr<SimulationProposal> propo;

	for (uint32_t i = 0; i < node->GetNApplications(); i++)
	{
		app = node->GetApplication(i);
		propo = DynamicCast<SimulationProposal> (app);
			         //NS_ASSERT (listOlsr);

		if (propo)
		{
			InetSocketAddress address = InetSocketAddress (addr, 49153);

			Ptr<Packet> packet;
			Address from, to;
			m_socket->GetSockName (from);
			m_socket->GetPeerName (to);

			m_socket->Connect (address);
			SimulationHeader header;
			header.SetSeq (m_seq++);
			header.SetSize (m_pktSize);
			header.GetNormal();

			NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
			packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
	  // Trace before adding header, for consistency with PacketSink
//  			  m_txTraceWithSeqTsSize2 (packet, from, to, header);

			packet->AddHeader (header);

			m_socket->SendTo(packet, 0, address);
		}
	}

}



void SimulationProposalReactive::FinishComputing()
{
	//自分で作成
	Ptr<Node> node = GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

	Ptr<Application> app;
	Ptr<SimulationProposal> propo;

	for (uint32_t i = 0; i < node->GetNApplications(); i++)
	{
		app = node->GetApplication(i);
		propo = DynamicCast<SimulationProposal> (app);
		         //NS_ASSERT (listOlsr);

		if (propo)
		{
			InetSocketAddress address = InetSocketAddress ("10.1.1.1", 49153);

			m_socket->Connect (address);

			Ptr<Packet> packet;
			Address from, to;
//					std::cout<<m_socket->Connect (address)<<std::endl;

			m_socket->GetSockName (from);
			m_socket->GetPeerName (to);

			std::cout<<computing_resource_each.size()<<std::endl;
			SimulationHeader header;
			header.SetSeq (m_seq++);
			header.SetSize (m_pktSize);
			SimulationHeader::StopComputing &stopcomputing = header.GetStopComputing();
//
////					std::cout<<computing_resource_each.size()<<std::endl;
			stopcomputing.SetAddr(computing_resource_each2);
			NS_ABORT_IF (m_pktSize < header.GetSerializedSize ());
			packet = Create<Packet> (m_pktSize - header.GetSerializedSize ());
			// Trace before adding header, for consistency with PacketSink
			m_txTraceWithSeqTsSize2 (packet, from, to, header);
			packet->AddHeader (header);

			m_socket->SendTo(packet, 0, address);

		}
	}
}


void SimulationProposalReactive::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void SimulationProposalReactive::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void SimulationProposalReactive::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&SimulationProposalReactive::HandleRead, this));
  m_socketList.push_back (s);
}





} // Namespace ns3
