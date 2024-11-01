/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
 * Copyright (c) 2013 Dalian University of Technology
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
 * Author: Junling Bu <linlinjavaer@gmail.com>
 *
 */
/**
 * This example shows basic construction of an 802.11p node.  Two nodes
 * are constructed with 802.11p devices, and by default, one node sends a single
 * packet to another node (the number of packets and interval between
 * them can be configured by command-line arguments).  The example shows
 * typical usage of the helper classes for this mode of WiFi (where "OCB" refers
 * to "Outside the Context of a BSS")."
 */

#include "ns3/vector.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/udp-socket.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/netanim-module.h"
#include <iostream>

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleOcb");

/*
 * In WAVE module, there is no net device class named like "Wifi80211pNetDevice",
 * instead, we need to use Wifi80211pHelper to create an object of
 * WifiNetDevice class.
 *
 * usage:
 *  NodeContainer nodes;
 *  NetDeviceContainer devices;
 *  nodes.Create (2);
 *  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
 *  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
 *  wifiPhy.SetChannel (wifiChannel.Create ());
 *  NqosWaveMacHelper wifi80211pMac = NqosWave80211pMacHelper::Default();
 *  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
 *  devices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodes);
 *
 * The reason of not providing a 802.11p class is that most of modeling
 * 802.11p standard has been done in wifi module, so we only need a high
 * MAC class that enables OCB mode.
 */

void ConfigureTransmissionRangeTRG(double trange, YansWifiPhyHelper &wifiPhy) {
	if(trange == 200.0){
		wifiPhy.Set("TxPowerStart",  DoubleValue (0.4));
		wifiPhy.Set("TxPowerEnd",  DoubleValue (0.4));
	}
	if(trange == 249.0){
		wifiPhy.Set("TxPowerStart",  DoubleValue (2.3));
		wifiPhy.Set("TxPowerEnd",  DoubleValue (2.3));
	}
	if(trange == 251.0){
			wifiPhy.Set("TxPowerStart",  DoubleValue (2.4));
			wifiPhy.Set("TxPowerEnd",  DoubleValue (2.4));
	}
	else { //para os parâmetros atuais, não estava conseguindo aplicar a fórmulado do TRG, então achei uma parecida a partir de 200m (testando)
		double pt = 0.4 + ((trange - 200.0)/2.578)*0.1;
		wifiPhy.Set("TxPowerStart",  DoubleValue (pt));
		wifiPhy.Set("TxPowerEnd",  DoubleValue (pt));
	}
}

void ReceivePacket (Ptr<Socket> socket)
{
	//Address from;
	//socket->GetPeerName(from);
	//std::cout << "TESTE"<< from << std::endl;

  while (socket->Recv ())
    {
	  std::cerr << "Nó:" << socket->GetNode()->GetId() << " [DEBUG] Tempo depois de receber => "<< Simulator::Now().GetNanoSeconds() << std::endl;
      NS_LOG_UNCOND ("Received one packet!");

      //multicast
      //Address from;
      //Ptr<Packet> packet= socket->RecvFrom(from);
      //Ipv4Address ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();

      //socket->GetAttribute("port", )
      //std::cout << "TESTE"<< ipv4From << std::endl;
      //ipv4From.IsMulticast ();

	  /*if (ipv4From.IsMulticast ())
      	{
            std::cerr << "TESTE"<< Simulator::Now().GetNanoSeconds () << std::endl;
      	}*/


    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
	  std::cerr << "[DEBUG] Tempo antes de enviar => "<< Simulator::Now().GetNanoSeconds () << std::endl;
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

int main (int argc, char *argv[])
{
	std::string phyMode ("OfdmRate6MbpsBW10MHz");
	uint32_t packetSize = 1000; // bytes
	uint32_t numPackets = 3;
	double interval = 1.0; // seconds
	bool verbose = false;

	CommandLine cmd;

	cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
	cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
	cmd.AddValue ("numPackets", "number of packets generated", numPackets);
	cmd.AddValue ("interval", "interval (seconds) between packets", interval);
	cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
	cmd.Parse (argc, argv);
	// Convert to time object
	Time interPacketInterval = Seconds (interval);


	NodeContainer c;
	c.Create (5);

	/*@
	  // The below set of helpers will help us to put together the wifi NICs we want
	  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
	  wifiPhy.SetChannel (channel);
	  // ns-3 supports generate a pcap trace
	  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
	  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
	  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
	  if (verbose)
		{
		  wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
		}

	  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
										  "DataMode",StringValue (phyMode),
										  "ControlMode",StringValue (phyMode));
	  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);
	 */

	// wifi 802.11a with constant 6Mbps (20MHz channel width)

	//begin-alisson
	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
			"Frequency", DoubleValue (5.9e9),
			"HeightAboveZ", DoubleValue (1.5));
	//	  wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
	//wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange",DoubleValue(600));
	double trange = 200.0; //alcance de transmissão
	//wifiPhy.Set("TxPowerStart",  DoubleValue (0.4));
	//wifiPhy.Set("TxPowerEnd",  DoubleValue (0.4));
	//wifiPhy.Set("TxGain", DoubleValue(1.0));
	//wifiPhy.Set("RxGain", DoubleValue(1.0));
	ConfigureTransmissionRangeTRG(trange, wifiPhy);
	//wifiPhy.Set("ChannelNumber",  UintegerValue (CCH));
	wifiPhy.SetChannel (wifiChannel.Create ());
	// ns-3 supports generate a pcap trace
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
	NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
	Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
	wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
					"DataMode",StringValue (phyMode),
					"ControlMode",StringValue (phyMode));
	NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);
	//end-alisson

	//begin-mateus
	/*
	// Set of helpers will help us to put together the wifi NICs we want
		YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
		YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
		wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
		//wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel", "Distance1", 4);
		wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
				"Distance1", DoubleValue(80.0), "Distance2", DoubleValue(200.0),
				"m0", DoubleValue(1.5), "m1", DoubleValue(0.75), "m2", DoubleValue(0.75));
		// wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange",DoubleValue(300.0));
		Ptr<YansWifiChannel> channel = wifiChannel.Create ();
		wifiPhy.SetChannel (channel);
		// ns-3 supports generate a pcap trace
		wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
		wifiPhy.Set ("TxGain", DoubleValue(5) );
		wifiPhy.Set ("RxGain", DoubleValue (5) );
		wifiPhy.Set ("TxPowerStart", DoubleValue(5));
		wifiPhy.Set ("TxPowerEnd", DoubleValue(5));
		// wifiPhy.Set ("TxPowerLevels", UintegerValue(1));
		wifiPhy.Set("EnergyDetectionThreshold",DoubleValue(-100.0));
		//MAC LAYER
		NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
		Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
		wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
				"DataMode",StringValue (phyMode),
				"ControlMode",StringValue (phyMode));
		NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);
		*/
		//end-mateus


	// Tracing
	wifiPhy.EnablePcap ("wave-simple-80211p", devices);

	double distance = 50.0;
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	positionAlloc->Add (Vector (0.0, distance, 0.0));
	positionAlloc->Add (Vector (10.0, 10.0, 0.0));
	positionAlloc->Add (Vector (20.0, 20.0, 0.0));
	positionAlloc->Add (Vector (30.0, 30.0, 0.0));
	mobility.SetPositionAllocator (positionAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (c);

	InternetStackHelper internet;
	internet.Install (c);

	Ipv4AddressHelper ipv4;
	NS_LOG_INFO ("Assign IP Addresses.");
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i = ipv4.Assign (devices);

	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), 80);

	//multicast
	Ipv4Address multicastSource ("10.1.1.2");
	Ipv4Address multicastGroup ("225.1.2.4");
	Ipv4StaticRoutingHelper multicast;
	uint16_t multicastPort = 9;

	//recepção no nó 0
	Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
	recvSink->Bind (local);
	recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	//recepção no nó 0 em multicast
	InetSocketAddress local_m = InetSocketAddress (Ipv4Address::GetAny(), multicastPort);
	Ptr<Socket> recvSinkMulticast = Socket::CreateSocket (c.Get (0), tid);
	recvSinkMulticast->Bind (local_m);
	recvSinkMulticast->Listen ();
	recvSinkMulticast->ShutdownSend ();
	if (addressUtils::IsMulticast(local_m))
	{
      std::cerr << "TESTE"<< Simulator::Now().GetNanoSeconds () << std::endl;
	  Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (recvSinkMulticast);
	  if (udpSocket)
		{
		  // equivalent to setsockopt (MCAST_JOIN_GROUP)
		  //udpSocket->MulticastJoinGroup (0, local_m);

		  udpSocket->MulticastJoinGroup (0, Ipv4Address("225.1.2.4"));
		}
	  //if(!(udpSocket->MulticastJoinGroup(0, Ipv4Address("225.1.2.4")))){

	  //}
	}

	recvSinkMulticast->SetRecvCallback (MakeCallback (&ReceivePacket));


	//recepção no nó 2
	Ptr<Socket> recvSink2 = Socket::CreateSocket (c.Get (2), tid);
	recvSink2->Bind (local);
	recvSink2->SetRecvCallback (MakeCallback (&ReceivePacket));

	//recepção no nó 3
	Ptr<Socket> recvSink3 = Socket::CreateSocket (c.Get (3), tid);
	recvSink3->Bind (local);
	recvSink3->SetRecvCallback (MakeCallback (&ReceivePacket));

	//recepção no nó 4
	Ptr<Socket> recvSink4 = Socket::CreateSocket (c.Get (4), tid);
	recvSink4->Bind (local);
	recvSink4->SetRecvCallback (MakeCallback (&ReceivePacket));

	//envio em broadcast do nó 1
	/*Ptr<Socket> source = Socket::CreateSocket (c.Get (1), tid);
	InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
	source->SetAllowBroadcast (true);
	source->Connect (remote);*/

	//envio em unicast do nó 1
	/*Ptr<Socket> source = Socket::CreateSocket (c.Get (1), tid);
	InetSocketAddress remote = InetSocketAddress (i.GetAddress (3, 0), 80);
	source->Connect (remote);*/

	//envio em multicast do nó 1
	// 2) Set up a default multicast route on the sender n0
	Ptr<Node> sender = c.Get (1);
	Ptr<NetDevice> senderIf = devices.Get (1);
	multicast.SetDefaultMulticastRoute (sender, senderIf);

	Ptr<Socket> source = Socket::CreateSocket (c.Get (1), tid);
	InetSocketAddress remote = InetSocketAddress (multicastGroup, multicastPort);
	source->Connect (remote);




	// Animacao gerada para analise
	AnimationInterface anim ("wave-simple-80211p.xml");

	std::cerr << "[DEBUG] Distância entre os nós => "<< distance << std::endl;

	Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
			Seconds (1.0), &GenerateTraffic,
			source, packetSize, numPackets, interPacketInterval);

	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}
