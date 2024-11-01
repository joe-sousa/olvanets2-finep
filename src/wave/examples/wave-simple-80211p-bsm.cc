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
#include "ns3/wave-bsm-helper.h"
#include <iostream>

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleBsm");

double m_TotalSimTime = 300.01;
std::vector <double> m_txSafetyRanges;

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
			wifiPhy.Set("TxPowerStart",  DoubleValue (3.3));
			wifiPhy.Set("TxPowerEnd",  DoubleValue (3.3));
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
	//std::string phyMode ("OfdmRate6MbpsBW10MHz");
	uint32_t packetSize = 1000; // bytes
	uint32_t numPackets = 3;
	double interval = 1.0; // seconds
	bool verbose = false;

	CommandLine cmd;

	//cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
	cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
	cmd.AddValue ("numPackets", "number of packets generated", numPackets);
	cmd.AddValue ("interval", "interval (seconds) between packets", interval);
	cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
	cmd.Parse (argc, argv);
	// Convert to time object
	Time interPacketInterval = Seconds (interval);


	NodeContainer c;
	c.Create (5);

	//begin-alisson
	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
			"Frequency", DoubleValue (5.9e9),
			"HeightAboveZ", DoubleValue (1.5));
	//	  wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
	//wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange",DoubleValue(600));

	//double trange = 251.0; //alcance de transmissão
	//ConfigureTransmissionRangeTRG(trange, wifiPhy);

	std::string phyMode ("OfdmRate27MbpsBW10MHz"); //padrão do WAVE é 6Mbps
	wifiPhy.Set ("TxGain", DoubleValue(2.0) );
	wifiPhy.Set ("RxGain", DoubleValue (2.0) );
	wifiPhy.Set ("TxPowerLevels", UintegerValue(1));
	wifiPhy.Set("EnergyDetectionThreshold",DoubleValue(-95.0));
	//trange = 251.0;
	//ConfigureTransmissionRangeTRG(trange); //configura a camada física para garantir o alcance
	wifiPhy.Set ("TxPowerStart", DoubleValue(16.7));
	wifiPhy.Set ("TxPowerEnd", DoubleValue(16.7));
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


	// Tracing
	wifiPhy.EnablePcap ("wave-simple-80211p-bsm", devices);

	double distance = 200.0;
	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
	positionAlloc->Add (Vector (0.0, 0.0, 0.0));
	positionAlloc->Add (Vector (0.0, distance, 0.0));
	positionAlloc->Add (Vector (10.0, 10.0, 0.0));
	positionAlloc->Add (Vector (20.0, 20.0, 0.0));
	positionAlloc->Add (Vector (30.0, 30.0, 0.0));
	mobility.SetPositionAllocator (positionAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	//mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel");
	mobility.Install (c);

	InternetStackHelper internet;
	internet.Install (c);

	Ipv4AddressHelper ipv4;
	NS_LOG_INFO ("Assign IP Addresses.");
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i = ipv4.Assign (devices);

	//configura a aplicação de mensagens beacons (BSM - Basic Safety Messages)
	WaveBsmHelper waveBsm;
	m_txSafetyRanges.push_back(251.0);
	WaveBsmHelper::GetNodesMoving().resize(5,1);
	waveBsm.Install (i,
					   Seconds (m_TotalSimTime),
					   200,
					   Seconds (0.1),
					   // GPS accuracy (i.e, clock drift), in number of ns
					   40,
					   m_txSafetyRanges,
					   0,
					   // tx max delay before transmit, in ms
					   MilliSeconds (10));


	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), 80);

	//recepção no nó 0
	Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
	recvSink->Bind (local);
	recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

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
	Ptr<Socket> source = Socket::CreateSocket (c.Get (1), tid);
	InetSocketAddress remote = InetSocketAddress (i.GetAddress (0), 80);
	source->Connect (remote);



	// Animacao gerada para analise
	//AnimationInterface anim ("wave-simple-80211p-bsm.xml");

	std::cerr << "[DEBUG] Distância entre os nós => "<< distance << std::endl;

	Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
			Seconds (1.0), &GenerateTraffic,
			source, packetSize, numPackets, interPacketInterval);

	Simulator::Stop (Seconds (m_TotalSimTime));
	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}
