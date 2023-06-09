/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "olsr-helper-seed.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/double.h"

namespace ns3 {

OlsrHelperSeed::OlsrHelperSeed (double seed)
{
  m_agentFactory.SetTypeId ("ns3::olsr::RoutingProtocol");
  m_agentFactory.Set ("randomseed", DoubleValue(seed));
}

OlsrHelperSeed::OlsrHelperSeed (const OlsrHelperSeed &o)
  : m_agentFactory (o.m_agentFactory)
{
  m_interfaceExclusions = o.m_interfaceExclusions;
  std::list<std::pair<const OlsrHelperSeed *, int16_t> >::const_iterator i;
  for (i = o.m_list.begin (); i != o.m_list.end (); ++i)
   {
     m_list.push_back (std::make_pair (const_cast<const OlsrHelperSeed *> (i->first->Copy ()), i->second));
   }
}

OlsrHelperSeed*
OlsrHelperSeed::Copy (void) const
{
  return new OlsrHelperSeed (*this);
}

void
OlsrHelperSeed::ExcludeInterface (Ptr<Node> node, uint32_t interface)
{
  std::map< Ptr<Node>, std::set<uint32_t> >::iterator it = m_interfaceExclusions.find (node);

  if (it == m_interfaceExclusions.end ())
    {
      std::set<uint32_t> interfaces;
      interfaces.insert (interface);

      m_interfaceExclusions.insert (std::make_pair (node, std::set<uint32_t> (interfaces) ));
    }
  else
    {
      it->second.insert (interface);
    }
}

Ptr<Ipv4RoutingProtocol>
OlsrHelperSeed::Create (Ptr<Node> node) const
{
  Ptr<olsr::RoutingProtocol> agent = m_agentFactory.Create<olsr::RoutingProtocol> ();

  std::map<Ptr<Node>, std::set<uint32_t> >::const_iterator it = m_interfaceExclusions.find (node);

  if (it != m_interfaceExclusions.end ())
    {
      agent->SetInterfaceExclusions (it->second);
    }

  node->AggregateObject (agent);
  return agent;
}

void
OlsrHelperSeed::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

int64_t
OlsrHelperSeed::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
      Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
      NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
      Ptr<olsr::RoutingProtocol> olsr = DynamicCast<olsr::RoutingProtocol> (proto);
      if (olsr)
        {
          currentStream += olsr->AssignStreams (currentStream);
          continue;
        }
      // Olsr may also be in a list
      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
      if (list)
        {
          int16_t priority;
          Ptr<Ipv4RoutingProtocol> listProto;
          Ptr<olsr::RoutingProtocol> listOlsr;
          for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
              listProto = list->GetRoutingProtocol (i, priority);
              listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
              if (listOlsr)
                {
                  currentStream += listOlsr->AssignStreams (currentStream);
                  break;
                }
            }
        }
    }


  return (currentStream - stream);

}

//自分で作成
void
OlsrHelperSeed::PrintResourceAll (Time printInterval, Ptr<OutputStreamWrapper> stream)
{
  for (uint32_t i = 0; i < NodeList::GetNNodes (); i++)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Simulator::Schedule (printInterval, &OlsrHelperSeed::PrintResource, printInterval, node, stream);
    }
}

//自分で作成した
void
OlsrHelperSeed::PrintResource (Time printInterval, Ptr<Node> node, Ptr<OutputStreamWrapper> stream)
{
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> ipv4_proto = ipv4->GetRoutingProtocol ();
  Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (ipv4_proto);

  if (list)
          {
            int16_t priority;
            Ptr<Ipv4RoutingProtocol> listProto;
            Ptr<olsr::RoutingProtocol> listOlsr;
            for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
              {
                listProto = list->GetRoutingProtocol (i, priority);
                listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
                NS_ASSERT (listOlsr);
                if (listOlsr)
                  {
                	listOlsr->Resource_Print(stream, node);
                	break;
                  }
              }
          }

}



//std::map<Ptr<Node>, Address>
//OlsrHelper::GetThreshold (Ptr<Node> node){
//	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
//	Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
//	Ptr<olsr::RoutingProtocol> olsr = DynamicCast<olsr::RoutingProtocol> (proto);
//
//	return olsr->GetResourceThreshold();
//}



//uint32_t
//OlsrHelper::ResourceAll ()
//{
//  for (uint32_t i = 0; i < NodeList::GetNNodes (); i++)
//    {
//      Ptr<Node> node = NodeList::GetNode (i);
//      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
//      Ptr<Ipv4RoutingProtocol> ipv4_proto = ipv4->GetRoutingProtocol ();
//      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (ipv4_proto);
//
//      if (list)
//       {
//           int16_t priority;
//           Ptr<Ipv4RoutingProtocol> listProto;
//           Ptr<olsr::RoutingProtocol> listOlsr;
//           for (uint32_t j = 0; j < list->GetNRoutingProtocols (); j++)
//             {
//        	   listProto = list->GetRoutingProtocol (j, priority);
//        	   listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
//        	   NS_ASSERT (listOlsr);
//        	   if (listOlsr)
//        	   {
//        		   return listOlsr->Resource_sum();
//
//               }
//             }
//       }
//    }
//  return 0;
//}

//自分で作成した
//void
//OlsrHelper::Resource (Ptr<Node> node)
//{
//  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
//  Ptr<Ipv4RoutingProtocol> ipv4_proto = ipv4->GetRoutingProtocol ();
//  Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (ipv4_proto);
//
//  if (list)
//          {
//            int16_t priority;
//            Ptr<Ipv4RoutingProtocol> listProto;
//            Ptr<olsr::RoutingProtocol> listOlsr;
//            for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
//              {
//                listProto = list->GetRoutingProtocol (i, priority);
//                listOlsr = DynamicCast<olsr::RoutingProtocol> (listProto);
//                NS_ASSERT (listOlsr);
//                if (listOlsr)
//                  {
//                	listOlsr->Resource_sum(node);
//                	break;
//                  }
//              }
//          }
//}



//

} // namespace ns3
