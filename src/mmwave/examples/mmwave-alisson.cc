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
 *   Author: Alisson Barbosa <alisson@ufc.br>
 *
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/point-to-point-helper.h"
#include <ns3/buildings-helper.h>
#include "ns3/global-route-manager.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"

using namespace ns3;
using namespace mmwave;

void ReceivePacket (Ptr<Socket> socket)
{
	//Address from;
	//socket->GetPeerName(from);
	//std::cout << "TESTE"<< from << std::endl;

	while (socket->Recv ())
	{
		std::cerr << "Nó:" << socket->GetNode()->GetId() << " [DEBUG] Tempo depois de receber => "<< Simulator::Now().GetSeconds() << std::endl;
		NS_LOG_UNCOND ("Received one packet!");
	}
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
		uint32_t pktCount, Time pktInterval )
{
	if (pktCount > 0)
	{
		std::cerr << "Nó:" << socket->GetNode()->GetId() << "[DEBUG] Tempo antes de enviar => "<< Simulator::Now().GetSeconds() << std::endl;
		socket->Send (Create<Packet> (pktSize));
		Simulator::Schedule (pktInterval, &GenerateTraffic,
				socket, pktSize,pktCount - 1, pktInterval);
	}
	else
	{
		socket->Close ();
	}
}

int
main (int argc, char *argv[])
{

	uint32_t packetSize = 1000;     //1000; // bytes
	uint32_t numPackets = 1;
	double interval = 1.0; // seconds
	Time interPacketInterval = Seconds (interval);
	bool harqEnabled = true;
	bool rlcAmEnabled = false;
	bool fixedTti = false;
	unsigned symPerSf = 24;
	double sfPeriod = 100.0;
	bool smallScale = true;
	double speed = 3;

	Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue(rlcAmEnabled));
	Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue(harqEnabled));
	Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue(harqEnabled));
	Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::CqiTimerThreshold", UintegerValue(1000));
	Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue(harqEnabled));
	Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::FixedTti", BooleanValue(fixedTti));
	Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::SymPerSlot", UintegerValue(6));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::ResourceBlockNum", UintegerValue(1));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue(72));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::SymbolsPerSubframe", UintegerValue(symPerSf));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::SubframePeriod", DoubleValue(sfPeriod));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::TbDecodeLatency", UintegerValue(200.0));
	Config::SetDefault ("ns3::MmWaveBeamforming::LongTermUpdatePeriod", TimeValue (MilliSeconds (100.0)));
	Config::SetDefault ("ns3::LteEnbRrc::SystemInformationPeriodicity", TimeValue (MilliSeconds (5.0)));
	//Config::SetDefault ("ns3::MmWavePropagationLossModel::ChannelStates", StringValue ("n"));
	Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue(MicroSeconds(100.0)));
	Config::SetDefault ("ns3::LteRlcUmLowLat::ReportBufferStatusTimer", TimeValue(MicroSeconds(100.0)));
	Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
	Config::SetDefault ("ns3::LteEnbRrc::FirstSibTime", UintegerValue (2));
	Config::SetDefault ("ns3::MmWaveBeamforming::SmallScaleFading", BooleanValue (smallScale));
	Config::SetDefault ("ns3::MmWaveBeamforming::FixSpeed", BooleanValue (true));
	Config::SetDefault ("ns3::MmWaveBeamforming::UeSpeed", DoubleValue (speed));
	//Config::SetDefault ("ns3::MmWaveUePhy::DoSetDlBandwidth", UintegerValue (10.0));

	CommandLine cmd;
	cmd.Parse (argc, argv);

	Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
	ptr_mmWave->Initialize();
	ptr_mmWave->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
	Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
	ptr_mmWave->SetEpcHelper (epcHelper);
	ptr_mmWave->SetHarqEnabled (harqEnabled);

	//Ptr<MmWaveUePhy> teste = CreateObject<MmWaveUePhy>();
	//teste->DoSetDlBandwidth()

	Ptr<Node> pgw = epcHelper->GetPgwNode ();

	//Install the mobility of pgw
	Ptr<ListPositionAllocator> pgwPositionAlloc = CreateObject<ListPositionAllocator> ();
	pgwPositionAlloc->Add (Vector (20.0, 10.0, 0.0));
	MobilityHelper pgwmobility;
	pgwmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	pgwmobility.SetPositionAllocator(pgwPositionAlloc);
	pgwmobility.Install (pgw);

	// Create a single RemoteHost
	NodeContainer remoteHostContainer;
	remoteHostContainer.Create (1);
	Ptr<Node> remoteHost = remoteHostContainer.Get (0);
	InternetStackHelper internet;
	internet.Install (remoteHostContainer);

	// Install the mobility of remote host
	Ptr<ListPositionAllocator> rhPositionAlloc = CreateObject<ListPositionAllocator> ();
	rhPositionAlloc->Add (Vector (0.0, 0.0, 0.0));
	MobilityHelper rhmobility;
	rhmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	rhmobility.SetPositionAllocator(rhPositionAlloc);
	rhmobility.Install (remoteHostContainer);

	// Create the Internet
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.0)));
	NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
	Ipv4AddressHelper ipv4h;
	ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
	// interface 0 is localhost, 1 is the p2p device
	//Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
	remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

	NodeContainer enbNodes;
	NodeContainer ueNodes;
	enbNodes.Create (1);
	ueNodes.Create (1);

	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
	enbPositionAlloc->Add (Vector (40.0, 0.0, 0.0));

	MobilityHelper enbmobility;
	enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	enbmobility.SetPositionAllocator(enbPositionAlloc);
	enbmobility.Install (enbNodes);
	//BuildingsHelper::Install (enbNodes);

	MobilityHelper uemobility;
	Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
	uePositionAlloc->Add (Vector (0.0, 60.0, 0.0));

	uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	uemobility.SetPositionAllocator(uePositionAlloc);
	uemobility.Install (ueNodes);
	//BuildingsHelper::Install (ueNodes);

	NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
	NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);

	//instala a pilha da internet e atribui IP ao eNB
	//InternetStackHelper internet;
	//internet.Install (enbNodes);
	//Ipv4AddressHelper ipv4;
	//ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	//Ipv4InterfaceContainer i = ipv4.Assign (enbNetDev);

	// Install the IP stack on the UEs
	internet.Install (ueNodes);
	Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
	//ueIpIface = ipv4.Assign (ueNetDev);

	Ptr<Node> ueNode = ueNodes.Get (0);
	// Set the default gateway for the UE
	//Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
	ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

	ptr_mmWave->AttachToClosestEnb (ueNetDev, enbNetDev);
	ptr_mmWave->EnableTraces();

	// Activate a data radio bearer
	//enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
	//EpsBearer bearer (q);
	//ptr_mmWave->ActivateDataRadioBearer (ueNetDev, bearer);

	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), 80);

	//recepção no remote host
	//Ptr<Socket> recvSink = Socket::CreateSocket (remoteHostContainer.Get (0), tid);
	//recvSink->Bind (local);
	//recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	//recepção no UE
	Ptr<Socket> recvSink = Socket::CreateSocket (ueNodes.Get (0), tid);
	recvSink->Bind (local);
	recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	//envio em unicast do UE
	//Ptr<Socket> source = Socket::CreateSocket (ueNodes.Get (0), tid);
	//InetSocketAddress remote = InetSocketAddress (remoteHostAddr, 80);
	//source->Connect (remote);

	//envio em unicast do remote host
	Ptr<Socket> source = Socket::CreateSocket (remoteHostContainer.Get (0), tid);
	InetSocketAddress remote = InetSocketAddress (ueIpIface.GetAddress (0), 80);
	source->Connect (remote);

	//envio em unicast do enb
	//Ptr<Socket> source = Socket::CreateSocket (enbNodes.Get (0), tid);
	//InetSocketAddress remote = InetSocketAddress (ueIpIface.GetAddress (0), 80);
	//source->Connect (remote);

	AnimationInterface anim ("mmwave-alisson.xml");

	Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
			Seconds (0.1), &GenerateTraffic,
			source, packetSize, numPackets, interPacketInterval);

	Simulator::Stop (Seconds (1));
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
