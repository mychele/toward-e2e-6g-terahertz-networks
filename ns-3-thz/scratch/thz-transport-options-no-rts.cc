/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 University at Buffalo, the State University of New York
 * (http://ubnano.tech/)
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
 * Author: Qing Xia <qingxia@buffalo.edu>
 *         Zahed Hossain <zahedhos@buffalo.edu>
 *         Josep Miquel Jornet <jmjornet@buffalo.edu>
 * Modified by: Michele Polese <michele.polese@gmail.com>
 */
#include <vector>
#include <iostream>
#include <cmath>
#include "ns3/antenna-module.h"
#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/thz-dir-antenna.h"
#include "ns3/thz-phy-macro.h"
#include "ns3/thz-mac-macro.h"
#include "ns3/thz-channel.h"
#include "ns3/thz-spectrum-waveform.h"
#include "ns3/thz-mac-macro-helper.h"
#include "ns3/thz-phy-macro-helper.h"
#include "ns3/thz-directional-antenna-helper.h"
#include "ns3/thz-udp-server.h"
#include "ns3/thz-udp-client.h"
#include "ns3/thz-udp-trace-client.h"
#include "ns3/thz-udp-client-server-helper.h"
#include "ns3/traffic-generator.h"
#include "ns3/traffic-generator-helper.h"

using namespace ns3;

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}


static void
RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () / 2 << "\t" << newRtt.GetSeconds () / 2 << std::endl;
}

static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
{
  if (packet->GetSize () > 0)
  {
    SeqTsHeader seqTs;
    uint32_t hSize = packet->PeekHeader (seqTs);
    if(hSize > 0)
    {
      *stream->GetStream () << Simulator::Now ().GetSeconds () 
        << "\t" << packet->GetSize() 
        << "\t" << Simulator::Now () - seqTs.GetTs ()
        << std::endl;
    }
    else
    {
      *stream->GetStream () << Simulator::Now ().GetSeconds () 
        << "\t" << packet->GetSize() 
        << "\t" << -1
        << std::endl;
    }
  }
  else
  {
    *stream->GetStream () << Simulator::Now ().GetSeconds () 
      << "\t" << packet->GetSize() 
      << "\t" << -1
      << std::endl;
  }
}

static void
Traces(uint32_t serverId, uint32_t ueId, std::string pathVersion, std::string finalPart)
{
  AsciiTraceHelper asciiTraceHelper;

  std::ostringstream pathCW;
  pathCW<<"/NodeList/"<< serverId <<"/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow";

  std::ostringstream fileCW;
  fileCW << pathVersion << "TCP-cwnd-change"  << ueId+1 << "_" << finalPart;

  std::ostringstream pathRTT;
  pathRTT << "/NodeList/"<< serverId <<"/$ns3::TcpL4Protocol/SocketList/0/RTT";

  std::ostringstream fileRTT;
  fileRTT << pathVersion << "latency-"  << ueId+1 << finalPart;

  // std::ostringstream pathRCWnd;
  // pathRCWnd<<"/NodeList/"<< serverId <<"/$ns3::TcpL4Protocol/SocketList/0/RWND";

  // std::ostringstream fileRCWnd;
  // fileRCWnd<<pathVersion << "TCP-rwnd-change"  << ueId+1 << "_" << finalPart;

  // std::ostringstream pathACK;
  // pathACK<<"/NodeList/"<< serverId <<"/$ns3::TcpL4Protocol/SocketList/0/Rx";

  // std::ostringstream fileACK;
  // fileACK<<pathVersion << "TCP-ack"  << ueId+1 << "_" << finalPart;

  // std::ostringstream pathACKtx;
  // pathACKtx<<"/NodeList/"<< ueId <<"/$ns3::TcpL4Protocol/SocketList/0/Tx";

  // std::ostringstream fileACKtx;
  // fileACKtx<<pathVersion << "TCP-ack-tx"  << ueId+1 << "_" << finalPart;

  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream (fileCW.str ().c_str ());
  Config::ConnectWithoutContext (pathCW.str ().c_str (), MakeBoundCallback(&CwndChange, stream1));

  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream (fileRTT.str ().c_str ());
  Config::ConnectWithoutContext (pathRTT.str ().c_str (), MakeBoundCallback(&RttChange, stream2));

  // Ptr<OutputStreamWrapper> stream4 = asciiTraceHelper.CreateFileStream (fileRCWnd.str ().c_str ());
  // Config::ConnectWithoutContext (pathRCWnd.str ().c_str (), MakeBoundCallback(&CwndChange, stream4));

  // Ptr<OutputStreamWrapper> stream5 = asciiTraceHelper.CreateFileStream (fileACK.str ().c_str ());
  // Config::ConnectWithoutContext (pathACK.str ().c_str (), MakeBoundCallback(&TxRx, stream5));

  // Ptr<OutputStreamWrapper> stream6 = asciiTraceHelper.CreateFileStream (fileACKtx.str ().c_str ());
  // Config::ConnectWithoutContext (pathACKtx.str ().c_str (), MakeBoundCallback(&TxRx, stream6));
}

NS_LOG_COMPONENT_DEFINE ("ThzTransport");

int main (int argc, char* argv[])
{
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);

  // LogComponentEnable("THzSpectrumValueFactory", LOG_LEVEL_INFO);
  // LogComponentEnable("THzSpectrumPropagationLoss", LOG_LEVEL_INFO);
  // LogComponentEnable("THzDirectionalAntenna", LOG_LEVEL_LOGIC);
  // LogComponentEnable("THzNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable("THzMacMacro", LOG_LEVEL_FUNCTION);
  LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);
  LogComponentEnable("TcpCongestionOps", LOG_LEVEL_LOGIC);
  // LogComponentEnable("THzChannel", LOG_LEVEL_INFO);
  // LogComponentEnable ("THzUdpClient", LOG_LEVEL_INFO);
  // LogComponentEnable ("THzUdpServer", LOG_LEVEL_INFO);

  uint32_t packetSize = 15000;
  double ueDist = 10;
  std::string transport = "tcp";
  double simTime = 10; // seconds
  double interPacketInterval = 10; // microseconds
  bool additionalTraces = true;
  bool rtsOn = 0;
  CommandLine cmd;
  cmd.AddValue ("packetSize", "size of the packets", packetSize);
  cmd.AddValue ("ueDist", "Distance between Enb and Ue [m]", ueDist);
  cmd.AddValue ("transport", "tcp or udp", transport);
  cmd.AddValue ("ipi", "interPacketInterval", interPacketInterval);
  cmd.AddValue ("rts", "enable rts", rtsOn);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpCubic::GetTypeId ()));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (131072*400));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (131072*400));
  // Config::SetDefault ("ns3::CoDelQueueDisc::Mode", EnumValue (ns3::CoDelQueueDisc::QueueDiscMode::QUEUE_DISC_MODE_PACKETS));
  // Config::SetDefault ("ns3::CoDelQueueDisc::MaxPackets", UintegerValue (50000));

  int node_num = 1;
  uint8_t SNodes = 1;
  uint8_t CNodes = node_num;
  NodeContainer Servernodes;
  Servernodes.Create (SNodes);
  NodeContainer Clientnodes;
  Clientnodes.Create (CNodes);
  std::printf ("node_num = %d\n", Clientnodes.GetN ());
  NodeContainer nodes;
  nodes.Add (Servernodes);
  nodes.Add (Clientnodes);

  //---------------------------------MOBILITY-------------------------------------//
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (Servernodes);

  Ptr<ListPositionAllocator> positionAllocUe = CreateObject<ListPositionAllocator> ();
  positionAllocUe->Add (Vector (ueDist, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAllocUe);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (Clientnodes);

  //------------------------------------CONNECT ALL-------------------------------------------//

  Ptr<THzChannel> thzChan = CreateObject<THzChannel> ();
  THzMacMacroHelper thzMac = THzMacMacroHelper::Default ();

  std::printf ("rts on? %d\n", rtsOn);
  if (rtsOn == true)
    {
      thzMac.Set ("EnableRts",StringValue ("1"));
    }
  else
    {
      thzMac.Set ("EnableRts",StringValue ("0"));
    }

  Config::SetDefault ("ns3::THzPhyMacro::CsPowerTh", DoubleValue(-120));
  Config::SetDefault ("ns3::THzPhyMacro::TxPower", DoubleValue(-10));
  Config::SetDefault ("ns3::THzPhyMacro::SinrTh", DoubleValue(0));
  Config::SetDefault ("ns3::THzDirectionalAntenna::TurningSpeed", DoubleValue (91032.04));
  Config::SetDefault ("ns3::THzDirectionalAntenna::MaxGain", DoubleValue (17.27));
  Config::SetDefault ("ns3::THzDirectionalAntenna::BeamWidth", DoubleValue (27.69));

  Config::SetDefault ("ns3::THzSpectrumValueFactory::TotalBandWidth", DoubleValue (7.476812e10));
  Config::SetDefault ("ns3::THzSpectrumValueFactory::NumSample", DoubleValue (1));

  THzPhyMacroHelper thzPhy = THzPhyMacroHelper::Default ();
  THzDirectionalAntennaHelper thzDirAntenna = THzDirectionalAntennaHelper::Default ();
  THzHelper thz;
  NetDeviceContainer devices = thz.Install (nodes, thzChan, thzPhy, thzMac, thzDirAntenna);

  //------------------SETUP NETWORK LAYER-------------------//
  InternetStackHelper internet;
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iface = ipv4.Assign (devices);


  //-----------------------------------PopulateArpCache---------------------------------------------//
  Ptr<ArpCache> arp = CreateObject<ArpCache> ();
  arp->SetAliveTimeout (Seconds (3600)); 
  for (uint16_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<Ipv4L3Protocol> ip = nodes.Get (i)->GetObject<Ipv4L3Protocol> ();
      NS_ASSERT (ip != 0);
      int ninter = (int) ip->GetNInterfaces ();
      for (int j = 0; j < ninter; j++)
        {
          Ptr<Ipv4Interface> ipIface = ip->GetInterface (j);
          NS_ASSERT (ipIface != 0);
          Ptr<NetDevice> device = ipIface->GetDevice ();
          NS_ASSERT (device != 0);
          Mac48Address addr = Mac48Address::ConvertFrom (device->GetAddress ());
          for (uint32_t k = 0; k < ipIface->GetNAddresses (); k++)
            {
              Ipv4Address ipAddr = ipIface->GetAddress (k).GetLocal ();
              if (ipAddr == Ipv4Address::GetLoopback ())
                {
                  continue;
                }
              ArpCache::Entry * entry = arp->Add (ipAddr);
              
              Ipv4Header ipHeader;
              Ptr<Packet> packet = Create<Packet> ();
              packet->AddHeader (ipHeader);
              
              entry->MarkWaitReply (ArpCache::Ipv4PayloadHeaderPair (packet, ipHeader));
              entry->MarkAlive (addr);

            }
        }
    }
  for (uint16_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<Ipv4L3Protocol> ip = nodes.Get (i)->GetObject<Ipv4L3Protocol> ();
      NS_ASSERT (ip != 0);
      int ninter = (int) ip->GetNInterfaces ();
      for (int j = 0; j < ninter; j++)
        {
          Ptr<Ipv4Interface> ipIface = ip->GetInterface (j);
          ipIface->SetArpCache (arp);
        }
    }

  //-----------------------------------End of ARP table-----------------------------------------------//

  ApplicationContainer sourceApps;
  ApplicationContainer sinkApps;
  uint16_t sinkPort = 20000;

  std::string path = "";


  for (uint16_t i = 0; i < Clientnodes.GetN (); i++)
  {
    Ptr<Node> clientNode = Clientnodes.Get (i);
    uint32_t serverId = Servernodes.Get (0)->GetId();
    uint32_t clientId = clientNode->GetId();

    NS_LOG_UNCOND("clientId " << clientId << " serverId " << serverId);

    if (transport == "tcp")
    {
      // sink on "server"
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      sinkApps.Add (packetSinkHelper.Install (Servernodes));

      // BulkSend on "client"
      BulkSendHelper ftp ("ns3::TcpSocketFactory",
                               InetSocketAddress (iface.GetAddress (0), sinkPort)); // address of the server
      sourceApps.Add (ftp.Install (clientNode));

      // enable additional traces
      if (additionalTraces)
      {
        Simulator::Schedule (Seconds (0.1001+0.01*i), &Traces, clientId, clientId, 
            path, ".txt");
      }
      
    }
    else if (transport == "udp")
    {
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      sinkApps.Add (packetSinkHelper.Install (Servernodes));

      // UdpClient on client
      UdpClientHelper dlClient (iface.GetAddress (0), sinkPort);
      dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds (interPacketInterval)));
      dlClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
      dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      sourceApps.Add (dlClient.Install (clientNode));
    }
    else
    {
      NS_FATAL_ERROR("transport not recognized");
    }

    std::ostringstream fileName;
    fileName << path << "rx-data"  << clientId + 1 << ".txt";

    AsciiTraceHelper asciiTraceHelper;

    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (fileName.str ().c_str ());
    sinkApps.Get(i)->TraceConnectWithoutContext("Rx", MakeBoundCallback (&Rx, stream));

    sourceApps.Get(i)->SetStartTime(Seconds (0.1+0.01*i)); 
    sourceApps.Get(i)->SetStopTime (Seconds (simTime));

    sinkPort++;
  }
  
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (simTime));

  // THzUdpServerHelper Server (9);
  // ApplicationContainer Apps = Server.Install (Servernodes);
  // Apps.Start (Seconds (0.0));
  // Apps.Stop (Seconds (10.0));


  // THzUdpClientHelper Client (iface.GetAddress (0), 9);
  // Client.SetAttribute ("PacketSize", UintegerValue (15000));
  // Client.SetAttribute ("Mean", DoubleValue (22));
  // Apps = Client.Install (Clientnodes);
  // Apps.Start (Seconds (0.0));
  // Apps.Stop (Seconds (10.0));

  Simulator::Stop (Seconds (simTime + 0.000001));
  ConfigStore config;
  config.ConfigureDefaults ();
  config.ConfigureAttributes ();
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
