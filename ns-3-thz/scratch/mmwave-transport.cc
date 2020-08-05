/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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
*
*   Author: Marco Miozzo <marco.miozzo@cttc.es>
*           Nicola Baldo  <nbaldo@cttc.es>
*
*   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
*                         Sourjya Dutta <sdutta@nyu.edu>
*                         Russell Ford <russell.ford@nyu.edu>
*                         Menglei Zhang <menglei@nyu.edu>
*/


#include "ns3/mmwave-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;
using namespace mmwave;


static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}


static void
RttChange (Ptr<OutputStreamWrapper> stream, Time oldRtt, Time newRtt)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldRtt.GetSeconds () << "\t" << newRtt.GetSeconds () << std::endl;
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


NS_LOG_COMPONENT_DEFINE ("MmWaveTransport");
int
main (int argc, char *argv[])
{
  uint16_t numEnb = 1;
  uint16_t numUe = 1;

  uint32_t packetSize = 1500;
  double ueDist = 10;
  std::string transport = "tcp";
  double simTime = 10; // seconds
  double interPacketInterval = 10; // microseconds
  bool additionalTraces = true;

  CommandLine cmd;
  cmd.AddValue ("packetSize", "size of the packets", packetSize);
  cmd.AddValue ("ueDist", "Distance between Enb and Ue [m]", ueDist);
  cmd.AddValue ("transport", "tcp or udp", transport);
  cmd.AddValue ("ipi", "interPacketInterval", interPacketInterval);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteRlcAm::PollRetransmitTimer", TimeValue (MilliSeconds (4.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReorderingTimer", TimeValue (MilliSeconds (2.0)));
  Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue (MilliSeconds (1.0)));
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MilliSeconds (4.0)));
  Config::SetDefault ("ns3::LteRlcAm::MaxTxBufferSize", UintegerValue (20 * 1024 * 1024));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
  Config::SetDefault ("ns3::LteEnbRrc::FirstSibTime", UintegerValue (2));

  // 400 MHz bandwidth
  Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue(29));
  
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (131072*400));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (131072*400));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpCubic::GetTypeId ()));

  Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> ();
  mmwaveHelper->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
  Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmwaveHelper->SetEpcHelper (epcHelper);
  mmwaveHelper->SetHarqEnabled (true);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2000));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.001)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  // Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (numEnb);
  ueNodes.Create (numUe);

  // Install Mobility Model
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 10.5));
  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNodes);

  MobilityHelper uemobility;
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  uePositionAlloc->Add (Vector (ueDist, 0.0, 1.7));
  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator (uePositionAlloc);
  uemobility.Install (ueNodes);

  // Install mmWave Devices to the nodes
  NetDeviceContainer enbmmWaveDevs = mmwaveHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer uemmWaveDevs = mmwaveHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (uemmWaveDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  mmwaveHelper->AttachToClosestEnb (uemmWaveDevs, enbmmWaveDevs);


  // Install and start applications on UEs and remote host
  ApplicationContainer sourceApps;
  ApplicationContainer sinkApps;
  uint16_t sinkPort = 20000;

  std::string path = "";

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> clientNode = remoteHost;
    uint32_t serverId = ueNodes.Get (u)->GetId();
    uint32_t clientId = clientNode->GetId();

    NS_LOG_UNCOND("clientId " << clientId << " serverId " << serverId);

    if (transport == "tcp")
    {
      // sink on "server"
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      sinkApps.Add (packetSinkHelper.Install (ueNodes.Get (u)));

      // BulkSend on "client"
      BulkSendHelper ftp ("ns3::TcpSocketFactory",
                               InetSocketAddress (ueIpIface.GetAddress (u), sinkPort)); // address of the server
      sourceApps.Add (ftp.Install (clientNode));

      // enable additional traces
      if (additionalTraces)
      {
        Simulator::Schedule (Seconds (0.1001+0.01*u), &Traces, clientId, clientId, 
            path, ".txt");
      }
      
    }
    else if (transport == "udp")
    {
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      sinkApps.Add (packetSinkHelper.Install (ueNodes.Get (u)));

      // UdpClient on client
      UdpClientHelper dlClient (ueIpIface.GetAddress (u), sinkPort);
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
    sinkApps.Get(u)->TraceConnectWithoutContext("Rx", MakeBoundCallback (&Rx, stream));

    sourceApps.Get(u)->SetStartTime(Seconds (0.1+0.01*u)); 
    sourceApps.Get(u)->SetStopTime (Seconds (simTime));

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

