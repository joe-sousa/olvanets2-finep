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

/*
 * SUE0 (0,0)                      SUE1 (230,0)
 * |                                |
 * eNB0 (0,10)                     eNB1 (230,10)
 * |                                |
 * UE (0,20) ---------------------> UE (230,20)
 *
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
//NodeContainer providers;
NodeContainer edgeNodes;
NodeContainer enbNodes;
NodeContainer ueNodes;
std::map < uint32_t ,Ipv4Address>	edgeEnb; // map para associar um enb com seu SUE (super user equipment)

void ReceivePacket (Ptr<Socket> socket) //recebe o pacote em unicast
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
	//Config::SetDefault ("ns3::MmWaveEnbPhy::DoSetBandwidth", 1000000, 1000000);
	//Config::SetDefault ("ns3::MmWaveEnbMac::DoConfigureMac", UintegerValue (1000000), UintegerValue (1000000));
	//Config::SetDefault ("ns3::MmWaveUePhy::DoSetDlBandwidth", UintegerValue (1000000)
	//lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth));
	//lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth)););
	//default ns3::mmWaveEnbNetDevice::DlBandwidth "25"

	CommandLine cmd;
	cmd.Parse (argc, argv);

	Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
	ptr_mmWave->Initialize();
	ptr_mmWave->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
	Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
	ptr_mmWave->SetEpcHelper (epcHelper);
	ptr_mmWave->SetHarqEnabled (harqEnabled);

	edgeNodes.Create (2);
	enbNodes.Create (2);
	ueNodes.Create (1);

	MobilityHelper edgeMobility;
	Ptr<ListPositionAllocator> edgePositionAlloc = CreateObject<ListPositionAllocator> ();
	edgePositionAlloc->Add (Vector (0.0, 0.0, 0.0)); //SUE0
	edgePositionAlloc->Add (Vector (230.0, 0.0, 0.0)); //SUE1
	edgeMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	edgeMobility.SetPositionAllocator(edgePositionAlloc);
	edgeMobility.Install (edgeNodes);

	MobilityHelper enbmobility;
	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
	enbPositionAlloc->Add (Vector (0.0, 10.0, 0.0)); //eNB0
	enbPositionAlloc->Add (Vector (230.0, 10.0, 0.0)); //eNB1
	enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	enbmobility.SetPositionAllocator(enbPositionAlloc);
	enbmobility.Install (enbNodes);

	MobilityHelper uemobility;
	Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
	uePositionAlloc->Add (Vector (0.0, 20.0, 0.0));
	uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	uemobility.SetPositionAllocator(uePositionAlloc);
	uemobility.Install (ueNodes);

	NetDeviceContainer edgeNetDev = ptr_mmWave->InstallUeDevice (edgeNodes);
	NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
	NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);

	//instala a pilha da internet nos EUs que agem como servidores de borda
	InternetStackHelper internet;
	internet.Install (edgeNodes);
	Ipv4InterfaceContainer edgeIpIface;
	edgeIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (edgeNetDev));

	// Install the IP stack on the UEs
	internet.Install (ueNodes);
	Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
	//ueIpIface = ipv4.Assign (ueNetDev);

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	// Set the default gateway for the SUEs and associate with your eNB
	for(uint32_t i=0; i<edgeNodes.GetN(); i++){
		Ptr<Node> edgeNode = edgeNodes.Get (i);
		Ptr<Ipv4StaticRouting> edgeNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (edgeNode->GetObject<Ipv4> ());
		edgeNodeStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

		uint32_t idEnb = enbNodes.Get(i)->GetId();
		edgeEnb[idEnb] = edgeIpIface.GetAddress (i);
	}

	// Set the default gateway for the UE
	Ptr<Node> ueNode = ueNodes.Get (0);
	Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
	ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

	ptr_mmWave->AttachToClosestEnb (edgeNetDev, enbNetDev);
	ptr_mmWave->AttachToClosestEnb (ueNetDev, enbNetDev);
	ptr_mmWave->EnableTraces();

	// Activate a data radio bearer
	//enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
	//EpsBearer bearer (q);
	//ptr_mmWave->ActivateDataRadioBearer (ueNetDev, bearer);

	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), 80);

	//como pegar o enb associado ao ue?
	Ptr<MmWaveUeNetDevice> teste = ueNetDev.Get(0)->GetObject<MmWaveUeNetDevice>();
	Ptr<MmWaveEnbNetDevice> teste2 = teste->GetTargetEnb();
	uint32_t teste3 = teste2->GetNode()->GetId();
	std::cerr << "Nó:" << teste3 << "[DEBUG] enb; tempo: => "<< Simulator::Now().GetSeconds() << std::endl;
	//pegar endereço do SUE associado a este eNB
	//edgeEnb[teste3]

	//busca em broadcast por SUEs
	/*Ptr<Socket> clientRequest = Socket::CreateSocket (ueNodes.Get (0), tid); //cria socket UDP do cliente para o broadcast
	InetSocketAddress port = InetSocketAddress (Ipv4Address::GetAny (), 80); //usado para receber as respostas das solicitações em broadcast
	InetSocketAddress broadcast = InetSocketAddress (Ipv4Address("255.255.255.255"), 80); //usado para enviar as solicitações em broadcast
	clientRequest->SetAllowBroadcast(true); //configura se transmissões de datagramas broadcast são permitidos
	clientRequest->Connect (broadcast); //Inicia uma conexão com o hospedeiro remoto (broadcast)
	clientRequest->Bind(port); //Aloca um endpoint para este socket para escutar respostas das solicitações em broadcast
	clientRequest->SetRecvCallback(MakeCallback(&clientRecRepPkt)); //Notifica quando novos dados estão disponíveis, recebe respostas e escolhe substitutos

	//fazer os SUEs escutarem as solicitações
	for (uint32_t x = 0; x < edgeNodes.GetN(); x++) { //percorre todos os SUEs
		Ptr<Socket> sink = Socket::CreateSocket (edgeNodes.Get (x), tid); //cria socket UDP nos SUEs para escutar solicitações do cliente
		sink->Bind(local); //aloca um endpoint para este socket para escutar solicitações do cliente
		sink->SetRecvCallback(MakeCallback(&serverListenForRequests)); //recebe solicitações e responde para o cliente
	}
	*/

	//recepção no SUE0
	//Ptr<Socket> recvSink = Socket::CreateSocket (edgeNodes.Get (0), tid);
	//recvSink->Bind (local);
	//recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	//recepção no SUE1
	//Ptr<Socket> recvSink = Socket::CreateSocket (edgeNodes.Get (1), tid);
	//recvSink->Bind (local);
	//recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	//recepção no UE
	//Ptr<Socket> recvSink = Socket::CreateSocket (ueNodes.Get (0), tid);
	//recvSink->Bind (local);
	//recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

	//envio em unicast do UE
	//Ptr<Socket> source = Socket::CreateSocket (ueNodes.Get (0), tid);
	//InetSocketAddress remote = InetSocketAddress (edgeIpIface.GetAddress (1), 80);
	//source->Connect (remote);

	//envio em unicast do SUE0
	//Ptr<Socket> source = Socket::CreateSocket (edgeNodes.Get (0), tid);
	//InetSocketAddress remote = InetSocketAddress (ueIpIface.GetAddress (0), 80);
	//source->Connect (remote);

	//envio em unicast do SUE1
	//Ptr<Socket> source = Socket::CreateSocket (edgeNodes.Get (1), tid);
	//InetSocketAddress remote = InetSocketAddress (ueIpIface.GetAddress (0), 80);
	//source->Connect (remote);

	AnimationInterface anim ("mmwave-alisson.xml2");

	//Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
	//		Seconds (0.1), &GenerateTraffic,
	//		source, packetSize, numPackets, interPacketInterval);

	//Simulator::Schedule(Seconds(0.1),clientSendPkt,clientRequest); //cliente envia pacote em broadcast para descoberta

	//vai tentar enviar para o SUE associado ao eNB associado

	//envio em unicast do UE
	Ptr<Socket> source = Socket::CreateSocket (ueNodes.Get (0), tid);
	InetSocketAddress remote = InetSocketAddress (edgeEnb[teste3], 80);
	source->Connect (remote);

	//criar socket para recepção em todos os SUEs
	for(uint32_t i =0; i < edgeNodes.GetN(); i++){
		Ptr<Socket> recvSink = Socket::CreateSocket (edgeNodes.Get (i), tid);
		recvSink->Bind (local);
		recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
	}

	//escalonando o envio
	Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
			Seconds (0.1), &GenerateTraffic,
			source, packetSize, numPackets, interPacketInterval);

	Simulator::Stop (Seconds (0.5));
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
