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
#include "ns3/rng-seed-manager.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/ns2-mobility-helper.h"

using namespace ns3;
using namespace mmwave;

//uint32_t packetSize = 1000;     //1000; // bytes
uint32_t packetSize = 2000;
uint32_t numPackets = 1;
double interval = 1.0; // seconds
Time interPacketInterval = Seconds (interval);
NodeContainer edgeNodes;
NodeContainer enbNodes;
NodeContainer carNodes;
NodeContainer clientNodes;
NodeContainer remoteHostContainer;
std::map < uint32_t ,Ipv4Address>	ipEdgeEnb; // map para associar um enb com seu SUE (super user equipment)
std::map < uint32_t ,uint32_t>	idEdgeEnb; // map para associar um enb com seu SUE (super user equipment)
Ptr<Socket> client_side;
Ptr<Socket> server_side;
double datarate = 4000000000.0;
Ipv4Address remoteHostAddr;
Ipv4InterfaceContainer clientIpIface;
Ipv4InterfaceContainer clientIpIfaceWave;
Ipv4InterfaceContainer edgeIpIface;
Ipv4InterfaceContainer carIpIface;
unsigned run = 0;
bool rcvdFirstServerReply = false;
double timeOfFirstServerReply;

// Função para receber solicitações dos clientes e responder
void serverListenForRequests(Ptr<Socket> socket) {
	//@TODO: O que prova que recebeu pacote?
	Address from;
	Ptr<Packet> packet= socket->RecvFrom(from);
	Ipv4Address ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();

	uint8_t *buff = new uint8_t[packet->GetSize()];
	packet->CopyData(buff,packet->GetSize());
	std::string data = std::string((char*)buff);

	std::cout << "[SERVIDOR] Mensagem recebida: "<< data << " em "<< Simulator::Now().GetSeconds () << " segundos" << std::endl;
	std::cout << "[SERVIDOR] Solicitação recebida de: "<< ipv4From << std::endl;
	std::cout << "[SERVIDOR] Enviando a resposta..." << std::endl;
	socket->Connect(InetSocketAddress(ipv4From,80));
	socket->Send(Create<Packet> (60)); //envia uma resposta
	socket->Close();
}

// Envia pacotes em unicast para o edge node associado ao enb
void clientRequestUc(Ptr<Socket> socket) {
	std::cout << "Cliente enviando mensagem em unicast para o eNB em "<< Simulator::Now().GetSeconds () << " segundos" << std::endl;
	//std::cerr << "[DEBUG] => "<< Simulator::Now().GetSeconds () << std::endl;
	std::stringstream msgx;
	msgx << "Alo, Alisson";
	Ptr<Packet> pkt = Create<Packet>((uint8_t*) msgx.str().c_str(), 256);
	socket->Send(pkt); //envia pacote pelo socket
}

// Recebe os pacotes de resposta e faz a escolha dos substitutos
void clientRecRepPkt(Ptr<Socket> socket) {
	Ipv4Address ipv4From;
	Address from;
	Ptr<Packet> packet= socket->RecvFrom(from);
	ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();
	std::cout << "[CLIENTE] Recebeu resposta de: "<< ipv4From << " em " << Simulator::Now().GetSeconds () << " segundos" << std::endl;
}

void buildTcl() {

	FILE *arq;
	FILE *arqWrite;
  	char a[100];
  	char b[100];
  	char d[100];
  	char e[100];
  	char f[100];
  	char g[100];
  	char h[100];
  	float c;
  	int init_action = 50;
  	int index = init_action-1;

  	// Abre um arquivo TEXTO para LEITURA
 	arq = fopen("/home/alisson/ns-3.29/mobilityTraces/urban-low.tcl", "rt");
 	arqWrite = fopen("/home/alisson/ns-3.29/mobilityTraces/teste-current.tcl", "wt");
  	if (arq == NULL)  // Se houve erro na abertura
  	{
     	printf("Problemas na abertura do arquivo\n");

  	}

  	while (!feof(arq))
  	{
      if(fscanf(arq, "%s %s %f %s %s %s %s %s", a, b, &c, d,e,f,g,h)){
      	if (c >= init_action-1 && strcmp (a, "$ns_") == 0){
      		fprintf(arqWrite,"%s %s %f %s %s %s %s %s\n", a, b, c-index, d,e,f,g,h);
      	}
      }
    }

  	fclose(arq);
  	fclose(arqWrite);
}

int
main (int argc, char *argv[])
{

	Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024 * 1024));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::ResourceBlockNum", UintegerValue(1));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue(72));
	bool rlcAmEnabled = true;
	Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue (1024 * 1024));
	Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue(rlcAmEnabled));
	Config::SetDefault ("ns3::LteRlcAm::PollRetransmitTimer", TimeValue(MilliSeconds(0.4)));
	Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue(MilliSeconds(1.0)));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::TbDecodeLatency", UintegerValue(2.0));
	Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (Seconds(0)));
	Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1apLinkDelay", TimeValue (Seconds(0)));
	Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::CqiTimerThreshold", UintegerValue(100000));

	//Config::SetDefault ("ns3::MmWaveHelper::HarqEnabled", BooleanValue(true));
	//Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue(true));
	//Config::SetDefault ("ns3::MmWaveFlexTtiMaxWeightMacScheduler::HarqEnabled", BooleanValue(true));
	//Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue(true));

	//Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHighSpeed::GetTypeId ()));
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1500)); // seta o tamanho do segmento TCP
	Config::SetDefault ("ns3::TcpSocket::TcpNoDelay", BooleanValue (true));
	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (131072*50));
	Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (131072*50));
	//Config::SetDefault ("ns3::MmWavePropagationLossModel::ChannelStates", StringValue ("n"));
	Config::SetDefault ("ns3::MmWavePropagationLossModel::FixedLossTst", BooleanValue (false));
	Config::SetDefault ("ns3::MmWavePropagationLossModel::LossFixedDb", DoubleValue (100.0));

	//configurando periodicidade das mensagens 5G
	//Config::SetDefault ("ns3::MmWavePhyMacCommon::SubframePeriod", DoubleValue(10000000.0));
	//Config::SetDefault ("ns3::MmWaveBeamforming::LongTermUpdatePeriod", TimeValue (Seconds (20.0)));
	Config::SetDefault ("ns3::MmWaveEnbPhy::UpdateSinrEstimatePeriod", IntegerValue (25600));

	CommandLine cmd;
	cmd.Parse (argc, argv);

	RngSeedManager::SetSeed (1234);
	RngSeedManager::SetRun (run);
	//SeedManager::SetSeed (m_seed);

	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults();

	clientNodes.Create (50);
	NodeContainer client;
	client.Add(clientNodes.Get(0));

	buildTcl();

	MobilityHelper mobility; // Habilitando a mobilidade do ns2 que utiliza os traces gerados pelo SUMO.
	//Ns2MobilityHelper ns2 = Ns2MobilityHelper ("/home/alisson/ns-3.29/mobilityTraces/urban-low.tcl");
	//Ns2MobilityHelper ns2 = Ns2MobilityHelper ("/home/alisson/ns-3.29/mobilityTraces/teste2.tcl");
	Ns2MobilityHelper ns2 = Ns2MobilityHelper ("/home/alisson/ns-3.29/mobilityTraces/teste-current.tcl");
	ns2.Install (); // instala a mobilidade por traces

	edgeNodes.Create (1);
	enbNodes.Create (1);

	MobilityHelper edgeMobility;
	Ptr<ListPositionAllocator> edgePositionAlloc = CreateObject<ListPositionAllocator> ();
	edgePositionAlloc->Add (Vector (3384.51 + 10.0, 1965.47, 0.0)); //SUE0
	//edgePositionAlloc->Add (Vector (0.0, 0.0, 0.0)); //SUE0
	edgeMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	edgeMobility.SetPositionAllocator(edgePositionAlloc);
	edgeMobility.Install (edgeNodes);

	MobilityHelper enbmobility;
	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
	enbPositionAlloc->Add (Vector (3384.51, 1965.47, 0.0)); //eNB0
	//enbPositionAlloc->Add (Vector (10.0, 0.0, 0.0)); //eNB0
	enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	enbmobility.SetPositionAllocator(enbPositionAlloc);
	enbmobility.Install (enbNodes);

	/*MobilityHelper clientmobility;
	Ptr<ListPositionAllocator> clientPositionAlloc = CreateObject<ListPositionAllocator> ();
	clientPositionAlloc->Add (Vector (3582.79, 2005.23, 0.0));
	//clientPositionAlloc->Add (Vector (50.0, 50.0, 0.0));
	clientmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	clientmobility.SetPositionAllocator(clientPositionAlloc);
	clientmobility.Install (clientNodes);*/

	Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
	//ptr_mmWave->Initialize();
	ptr_mmWave->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
	Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
	ptr_mmWave->SetEpcHelper (epcHelper);
	//ptr_mmWave->SetHarqEnabled (true);

	//instala primeiro a parte do mmwave
	NetDeviceContainer edgeNetDev = ptr_mmWave->InstallUeDevice (edgeNodes);
	NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
	NetDeviceContainer clientNetDevMmWave = ptr_mmWave->InstallUeDevice (client);

	//instala a pilha da internet nos EUs que agem como servidores de borda
	InternetStackHelper internet;
	internet.Install (edgeNodes);
	edgeIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (edgeNetDev));

	// Install the IP stack on the clients
	internet.Install (client);
	clientIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (clientNetDevMmWave));

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	// Set the default gateway for the SUEs and associate with your eNB
	for(uint32_t i=0; i<edgeNodes.GetN(); i++){
		Ptr<Node> edgeNode = edgeNodes.Get (i);
		Ptr<Ipv4StaticRouting> edgeNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (edgeNode->GetObject<Ipv4> ());
		edgeNodeStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

		uint32_t idEnb = enbNodes.Get(i)->GetId();
		ipEdgeEnb[idEnb] = edgeIpIface.GetAddress (i);
		idEdgeEnb[idEnb] = i;
	}

	// Set the default gateway for the UE
	Ptr<Node> clientNode = client.Get (0);
	Ptr<Ipv4StaticRouting> clientStaticRouting = ipv4RoutingHelper.GetStaticRouting (clientNode->GetObject<Ipv4> ());
	clientStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

	ptr_mmWave->AttachToClosestEnb (edgeNetDev, enbNetDev);
	ptr_mmWave->AttachToClosestEnb (clientNetDevMmWave, enbNetDev);
	ptr_mmWave->EnableTraces();

	//como pegar o enb associado ao ue?
	Ptr<MmWaveUeNetDevice> teste = clientNetDevMmWave.Get(0)->GetObject<MmWaveUeNetDevice>();
	Ptr<MmWaveEnbNetDevice> targetEnb = teste->GetTargetEnb();
	uint32_t idTargetEnb = targetEnb->GetNode()->GetId();
	std::cerr << "Nó:" << idTargetEnb << "[DEBUG] enb; tempo: => "<< Simulator::Now().GetSeconds() << std::endl;

	TypeId tidudp = TypeId::LookupByName ("ns3::UdpSocketFactory"); //pega o tipo da API para criar instâncias de sockets UDP
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80); //usado para escutar solicitações do cliente

	//hello em unicast para o eNB	
	Ptr<Socket> dstSocket = Socket::CreateSocket (clientNodes.Get (0), tidudp);
	InetSocketAddress port = InetSocketAddress (Ipv4Address::GetAny (), 80); //usado para receber as respostas das solicitações em broadcast
	InetSocketAddress remoteEdge = InetSocketAddress (ipEdgeEnb[idTargetEnb], 80);
	dstSocket->Connect (remoteEdge);
	dstSocket->Bind(port); //Aloca um endpoint para este socket para escutar respostas das solicitações em broadcast
	dstSocket->SetRecvCallback (MakeCallback (&clientRecRepPkt));

	//p servidor da edge receber solicitações em unicast e responder
	Ptr<Socket> sink2 = Socket::CreateSocket (edgeNodes.Get (0), tidudp); //cria socket UDP nos edgeNodes para escutar solicitações do cliente
	sink2->Bind(local); //aloca um endpoint para este socket para escutar solicitações do cliente
	sink2->SetRecvCallback(MakeCallback(&serverListenForRequests)); //recebe solicitações e responde para o cliente

	//p2ph.EnablePcapAll("mmwave-alisson3");
	AnimationInterface anim ("mmwave-alisson7.xml");
	anim.SetMaxPktsPerTraceFile(50000000); //50 milhões

	Simulator::Schedule(Seconds(1),clientRequestUc,dstSocket); //cliente envia pacote em unicast para o enb

	Simulator::Stop (Seconds (3));
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
