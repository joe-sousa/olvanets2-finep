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
 * Testar data rate com TCP de grandes arquivos
 *
 * SUE0 (0,0)
 * |
 * eNB0 (0,10)
 * |
 * UE (0,20)
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
#include "ns3/rng-seed-manager.h"

using namespace ns3;
using namespace mmwave;

//uint32_t packetSize = 1000;     //1000; // bytes
uint32_t packetSize = 2000;
uint32_t numPackets = 1;
double interval = 1.0; // seconds
Time interPacketInterval = Seconds (interval);
//NodeContainer providers;
NodeContainer edgeNodes;
NodeContainer enbNodes;
NodeContainer ueNodes;
NodeContainer remoteHostContainer;
std::map < uint32_t ,Ipv4Address>	edgeEnb; // map para associar um enb com seu SUE (super user equipment)
Ptr<Socket> client_side;
Ptr<Socket> server_side;
std::map < Ptr<Socket> ,uint32_t>	server_tcp_recv; // map para saber o quanto de dados o servidor recebeu de cada socket
std::map < Ptr<Socket> ,uint32_t>	server_tcp_send; // map para saber o quanto de dados o servidor enviou para cada socket
std::map < Ptr<Socket> ,uint32_t>	client_tcp_send; // map para saber o quanto de dados o cliente enviou para cada socket
std::map < Ptr<Socket> ,uint32_t>	client_tcp_recv; // map para saber o quanto de dados o cliente recebeu de cada socket
double datarate = 4000000000.0; //1 Gbps
Ipv4Address remoteHostAddr;
Ipv4InterfaceContainer ueIpIface;
Ipv4InterfaceContainer edgeIpIface;
unsigned run = 0;

//função para enviar dados maiores que o buffer do socket TCP
void ClientWriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
	//while (client_tcp_send[localSocket] < packetSize && localSocket->GetTxAvailable () > 0)
	if (client_tcp_send[localSocket] < packetSize && localSocket->GetTxAvailable () > 0)
	{
		uint32_t tcpsegment = 1500;
		uint32_t left = packetSize - client_tcp_send[localSocket];
		uint32_t toSend = std::min (left, localSocket->GetTxAvailable ());
		toSend = std::min (toSend, tcpsegment);
		Ptr<Packet> p = Create<Packet> (toSend);
		//NS_LOG_DEBUG ("Source send data=\"" << GetString (p) << "\"");
		int sent = localSocket->Send (p, 0);
		//NS_TEST_EXPECT_MSG_EQ ((sent != -1), true, "Error during send ?");
		if(sent < 0)
		{
			// we will be called again when new tx space becomes available.
			return;
		}
		client_tcp_send[localSocket] += sent;
	}
	//else if (client_tcp_send[localSocket] >= packetSize ) {
	if (client_tcp_send[localSocket] >= packetSize ) {
		return;
	}
	Time tNext (Seconds (1400 * 8 / datarate));
	Simulator::Schedule (tNext, &ClientWriteUntilBufferFull, localSocket, txSpace);


	//if(tcp_send[localSocket] == packetSize) {
	//	  tcp_send[localSocket] = 0; //zera depois de transmitir tudo
	//}
	//localSocket->Close ();
}

//função para enviar dados maiores que o buffer do socket TCP
void ServerWriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
	//while (server_tcp_send[localSocket] < packetSize && localSocket->GetTxAvailable () > 0)
	if (server_tcp_send[localSocket] < packetSize && localSocket->GetTxAvailable () > 0)
	{
		uint32_t tcpsegment = 1500;
		uint32_t left = packetSize - server_tcp_send[localSocket];
		uint32_t toSend = std::min (left, localSocket->GetTxAvailable ());
		toSend = std::min (toSend, tcpsegment);
		Ptr<Packet> p = Create<Packet> (toSend);
		//NS_LOG_DEBUG ("Source send data=\"" << GetString (p) << "\"");
		int sent = localSocket->Send (p, 0);
		//NS_TEST_EXPECT_MSG_EQ ((sent != -1), true, "Error during send ?");
		if(sent < 0)
		{
			// we will be called again when new tx space becomes available.
			return;
		}
		server_tcp_send[localSocket] += sent;
	}
	if(server_tcp_send[localSocket] == packetSize) {
	//if(server_tcp_send[localSocket] >= packetSize) { //temporário
		//	  tcp_send[localSocket] = 0; //zera depois de transmitir tudo

		//localSocket->Close ();
		return;
	}
	Time tNext (Seconds (1400 * 8 / datarate));
	Simulator::Schedule (tNext, &ServerWriteUntilBufferFull, localSocket, txSpace);
}

//begin implementation of sending "Application"
void StartFlow (Ptr<Socket> localSocket, Ipv4Address servAddress, uint16_t servPort, std::string upOrDownload) {
  //NS_LOG_LOGIC ("Starting flow at time " <<  Simulator::Now ().GetSeconds ());
  //localSocket->Connect (InetSocketAddress (servAddress, servPort)); //connect

  // tell the tcp implementation to call WriteUntilBufferFull again
  // if we blocked and new tx buffer space becomes available
  if(upOrDownload == "upload"){
	  localSocket->SetSendCallback (MakeCallback (&ClientWriteUntilBufferFull));
	  ClientWriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
  } else if(upOrDownload == "download"){
	  localSocket->SetSendCallback (MakeCallback (&ServerWriteUntilBufferFull));
	  ServerWriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
  }
}

//Envia os resultados para o cliente depois de fazer o processamento
void serverSendAfter(Ptr<Socket> socket, Ipv4Address ipv4From, uint16_t portFrom) {
	std::cout << "[SERVIDOR] Enviando as tarefas..." << std::endl;
	//@socket->Send(Create<Packet> (packetSize),0);
	StartFlow(socket,ipv4From, portFrom,"download");
	//socket->Close();
}

//Recebe os dados do cliente, processa (depois de sleepTime) e chama SendAfter para enviar os resultados
void serverHandler(Ptr<Socket> socket) {
	while(socket->GetRxAvailable() > 0){
		Address from;
		Ptr<Packet> pack2 = socket->RecvFrom (from);
		//std::cout << "[SERVIDOR] Pacote recebido " << std::endl;
		uint32_t packet2 = pack2->GetSize();
		server_tcp_recv[socket] += packet2;  //@TODO: garantir para zerar depois esse map
		Ipv4Address ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();  //ipv4 do cliente
		uint16_t portFrom = InetSocketAddress::ConvertFrom(from).GetPort();; //porta do cliente

		if (server_tcp_recv[socket]>=packetSize) {
			std::cout << "[SERVIDOR] Tarefa recebida\n";
			std::cout << "[SERVIDOR] Processando..." << std::endl;
			std::cerr << "[DEBUG] Tempo antes de processar => "<< Simulator::Now().GetSeconds () << std::endl;
			Simulator::Schedule(Seconds(0.0),&serverSendAfter,socket, ipv4From, portFrom); //processa (num tempo sleepTime) e chama o sendAfter p enviar os resultados
		}
	}

}

// Aceita a conexão
void serverAccept(Ptr<Socket> socket,const ns3::Address& from)
{
	std::cout<<"[SERVIDOR] Conexão aceita"<< std::endl;
	socket->SetRecvCallback (MakeCallback (&serverHandler));
}

//Cria o socket servidor e coloca-o para escutar e depois responder
void serverSide(uint32_t index) {
	TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
	InetSocketAddress listen = InetSocketAddress (Ipv4Address::GetAny (), 55555);
	std::cout << "[SERVIDOR] Servidor " << index << " ativo!" << std::endl;
	//server_side = Socket::CreateSocket(remoteHostContainer.Get(0),tid);
	server_side = Socket::CreateSocket(edgeNodes.Get(0),tid);
	server_side->Bind(listen);
	server_side->Listen();
	server_side->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>,const Address &> (),MakeCallback(&serverAccept));
}

//recebe resultados dos offloadings e imprime no arquivo de resultados
void clientHandler(Ptr<Socket> socket) {
	//FILE *fp = fopen("/tmp/results", "a+");  //arquivo com resultados   "results/resultados.tr"
	//@FILE *fp = fopen("results/results.tr", "a+");  //arquivo com resultados
	while(socket->GetRxAvailable() > 0){
		Address from;
		Ptr<Packet> pack = socket->RecvFrom (from);
		//std::cout << "[CLIENTE] Pacote recebido " << std::endl;
		uint32_t packet = pack->GetSize();
		client_tcp_recv[socket] += packet;

		if (client_tcp_recv[socket]>=packetSize) {
			std::cout << "[CLIENTE] Resultados recebidos!\n";
			std::cerr << "[DEBUG] Tempo depois do processamento => "<< Simulator::Now().GetSeconds () << std::endl;
		}
	}
}

void clientSide(uint32_t index) {
	std::cerr << "\t\t[DEBUG] Iniciando conexão com "<< index << std::endl;

	TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
	client_side = Socket::CreateSocket(ueNodes.Get(0),tid);

	//Ptr<TcpSocketBase> tcpSock = client_side->GetObject<TcpSocketBase> ();
	//tcpSock->SetAttribute("TcpNoDelay", BooleanValue (true));
	//tcpSock->SetAttribute("DelAckMaxCount", UintegerValue (0));
	//SetDelAckMaxCount=0 para não gerar o ack delayed

	//client_side->SetAttribute("ns3::TcpSocket::TcpNoDelay", BooleanValue (true));
	//client_side->SetAttribute("ns3::TcpSocket::m_noDelay", BooleanValue (true));
	//client_side->SetAttribute("TcpSocketBase::SetTcpNoDelay", BooleanValue (true));
	//m_noDelay

	//Ipv4Address serverIP = remoteHostAddr;
	//InetSocketAddress end = InetSocketAddress (serverIP, 55555);
	//Ipv4Address serverIP = edgeEnb[index];

	Ipv4Address serverIP = edgeIpIface.GetAddress(0);
	InetSocketAddress end = InetSocketAddress (serverIP, 55555);
	uint32_t status = client_side->Connect(end);
	std::cout << "[SISTEMA] Status da conexão => "<< status << std::endl;

	std::cout << "[CLIENTE] Enviando tarefas..." << std::endl;
	std::cerr << "Nó:" << "[DEBUG] Tempo antes de enviar => "<< Simulator::Now().GetSeconds() << std::endl;
	StartFlow(client_side,serverIP, 55555,"upload");
	//@client_side->Send(Create<Packet> (packetSize),0); // Envia o pedaco da matriz
	client_side->SetRecvCallback(MakeCallback(&clientHandler)); //espera para receber o resultado
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

	CommandLine cmd;
	cmd.Parse (argc, argv);

	RngSeedManager::SetSeed (1234);
	RngSeedManager::SetRun (run);
	//SeedManager::SetSeed (m_seed);

	Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
	//ptr_mmWave->Initialize();
	ptr_mmWave->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
	Ptr<MmWavePointToPointEpcHelper>  epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
	ptr_mmWave->SetEpcHelper (epcHelper);
	//ptr_mmWave->SetHarqEnabled (true);

	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults();

	/*Ptr<Node> pgw = epcHelper->GetPgwNode ();

	// Create a single RemoteHost
	remoteHostContainer.Create (1);
	Ptr<Node> remoteHost = remoteHostContainer.Get (0);
	InternetStackHelper internet;
	internet.Install (remoteHostContainer);

	// Create the Internet
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000000001)));
	NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
	Ipv4AddressHelper ipv4h;
	ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
	// interface 0 is localhost, 1 is the p2p device
	remoteHostAddr = internetIpIfaces.GetAddress (1);
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
	remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);*/

	edgeNodes.Create (1);
	enbNodes.Create (1);
	ueNodes.Create (1);

	MobilityHelper edgeMobility;
	Ptr<ListPositionAllocator> edgePositionAlloc = CreateObject<ListPositionAllocator> ();
	edgePositionAlloc->Add (Vector (0.0, 0.0, 0.0)); //SUE0
	edgeMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	edgeMobility.SetPositionAllocator(edgePositionAlloc);
	edgeMobility.Install (edgeNodes);

	MobilityHelper enbmobility;
	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
	enbPositionAlloc->Add (Vector (0.0, 10.0, 3.0)); //eNB0
	enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	enbmobility.SetPositionAllocator(enbPositionAlloc);
	enbmobility.Install (enbNodes);

	MobilityHelper uemobility;
	Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
	uePositionAlloc->Add (Vector (0.0, 20.0, 3.0));
	uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	uemobility.SetPositionAllocator(uePositionAlloc);
	uemobility.Install (ueNodes);

	NetDeviceContainer edgeNetDev = ptr_mmWave->InstallUeDevice (edgeNodes);
	NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
	NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);

	//instala a pilha da internet nos EUs que agem como servidores de borda
	InternetStackHelper internet;
	internet.Install (edgeNodes);
	edgeIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (edgeNetDev));

	// Install the IP stack on the UEs
	internet.Install (ueNodes);
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

	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	//InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), 80);

	//como pegar o enb associado ao ue?
	Ptr<MmWaveUeNetDevice> teste = ueNetDev.Get(0)->GetObject<MmWaveUeNetDevice>();
	Ptr<MmWaveEnbNetDevice> teste2 = teste->GetTargetEnb();
	uint32_t teste3 = teste2->GetNode()->GetId();
	std::cerr << "Nó:" << teste3 << "[DEBUG] enb; tempo: => "<< Simulator::Now().GetSeconds() << std::endl;

	//p2ph.EnablePcapAll("mmwave-alisson3");
	//AnimationInterface anim ("mmwave-alisson.xml3");
	//anim.SetMaxPktsPerTraceFile(50000000); //50 milhões

	Simulator::Schedule(Seconds(0.0),serverSide,teste3);
	Simulator::Schedule(Seconds(0.1),clientSide,teste3);

	Simulator::Stop (Seconds (2));
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
