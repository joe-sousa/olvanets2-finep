/*
*  Alisson Barbosa, Janeiro/2020. E-mail: alisson@ufc.br
*  This code realizes the packets exchange between vehicles (V2V) and the edge (V2I), simulating
*  the send of subtasks to be executed in the surrogate vehicles and in the edge of the network.
*
*  This code is protected by copyright and intellectual property right laws.
*  Do not use or distribute without the author's authorization.
*  This is true even for modified versions or snippets of this code.
*
*/

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "ns3/internet-stack-helper.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include <chrono>

using namespace ns3;
using namespace mmwave;

#define GOOD_DISTANCE 150.0
#define PI 3.14

/*
** @TODO:
** WARNING: IS NOT A GOOD PRACTICE DECLARE ALL VARIABLES AS GLOBALS. THE IDEAL IS TO CREATE CLASSES/OBJECTS WITH VARIABLES AND
** 			GETTERS AND SETTERS.
*/
typedef std::pair<Ptr<Node>,double> pair; //novo tipo para ajudar na ordenação dos providers; node / distância para o cliente

extern std::ofstream os; //fluxo para o arquivo de log

extern FILE *fp; //arquivo dos resultados finais

extern std::chrono::high_resolution_clock::time_point startElapsedTime; //para medir o tempo que roda o algoritmo
extern std::chrono::high_resolution_clock::time_point endElapsedTime; //para medir o tempo que roda o algoritmo

extern bool KEY;
extern bool alreadySent; //verifica se o cliente já fez o offloading uma vez
extern bool alreadySentToSort; //verifica se o cliente chamou a ordenação dos candidatos a substitutos
extern bool alreadyStartedCheckConnectivity; //apenas p o hvc: verifica se já iniciou a verificação de conectividade
extern bool rcvdFirstServerReply;
extern bool alreadyDecided; //se já decidiu como distribuir as tarefas
extern bool enbNotFound; //inicialmente o enb n foi encontrado
extern bool hvcTimeout; //estouro de tempo do hvc para executar localmente as tarefas q faltaram

extern int imgSize;
extern int tasksOnlyLocal; //número de tarefas executadas localmente desde o início
extern int tasksOffloadedSuc; //número de tarefas executadas remotamente com sucesso
extern int tasksRecovered; //número de tarefas offloadadas recuperadas para executar localmente
extern int tasksCounted; //número final de tarefas contadas que é a soma das 3 variáveis anteriores
extern int nReplies; //número de respostas após 0.5s de espera
extern int numberOfEnergyViolations; //número de violações nas restrições de energia

extern uint32_t initAction;
extern uint32_t clientId; //id do cliente
extern uint32_t numberOfNodes;
extern uint32_t packetSize;
extern uint32_t resultPacketSize; //pacote de retorno é apenas uma string
extern uint32_t workload; //especifica qual workload será executado
extern uint32_t numberOfSurrogates;
extern uint32_t numberOfRecoveries; //conta a quantidade de recuperações de falhas
extern uint32_t run; //usado para ir pegando partes subsequentes da semente (seed)
extern uint32_t numberOfEdgeServers; //número de servidores de borda e eNBs
extern uint32_t numberOfParkedVehicles; //número de veículos estacionados, desligados, mas funcionando CPU e 802.11p
extern uint32_t idxOfServerAccept;
extern uint32_t idxFromMoreClosestEnb; //índice do enb mais próximo do cliente no initAction
extern uint32_t idxOfProvidersActionedInServer; //índice dos providers já acionados no servidor
extern uint32_t idxOfProvidersActionedInClient; //índice dos providers já acionados no cliente
extern uint32_t numberOfTasks; //número de imagens a ser processadas
extern uint32_t attempts; //número de tentativas
extern uint32_t OffloadSuccess;
extern uint32_t foodSources; //qtd de soluções ou food sources do algoritmo ABC
extern uint32_t numberOfCycles; //número de ciclos do algoritmo ABC

extern double imgTime;
extern double trange; //alcance de transmissão
extern double txpower; //potência de transmissão
extern double sleepTime; // tempo de processamento
extern double onlyLocalTime; //tempo a ser batido... unicamente local
extern double variation; //variação do offloadingTime em relação ao onlyLocalTime
extern double localTime; //o local fica com algum processamento... esse é o tempo desse processamento
extern double startTime; //tempo inicial da simulação
extern double finishTime; //tempo final da simulação
extern double timeOfFirstServerReply;
extern double datarateMmWave; //3 Gbps
extern double datarateWave; //27 Mbps
extern double tempoIni;
extern double wAvgCpuReq; //média de cpu required das tasks do workload
extern double procDeadline; //time limit/deadline para processar tasks
extern double wSumCpuReq; //soma de cpu required de todas as tasks do workload
extern double replyTime; //tempo para o servidor responder à solicitação, p evitar concorrência no canal
extern double elapsedTime; //para medir o tempo que roda o algoritmo

extern std::string clientType; //tipo de cliente: qualquer um ou apenas moto
extern std::string algorithm;
extern std::string pknownRoutes; //porcentagem de rotas conhecidas
extern std::string tracePath; //caminho do trace de mobilidade
extern std::string traceFile; //arquivo de trace de mobilidade
extern std::string scenario; //urban ou highway
extern std::string density; //low, medium ou high
extern std::string cellcoverage; //tipo de cobertura das torres celulares 5G
extern std::string logFile; //arquivo para gerar os logs

extern std::vector <uint32_t> sizesOfPackets; //serve p, p ex, enviar apenas uma imagem (tam1), enviar duas imagens (tam2) ...

extern std::vector <double> sleepTimes; //serve p, p ex, saber quanto tempo de processamento para 1 imagem, 2 imagens ...
extern std::vector <double> enbXPositions; //posições no eixo X dos eNBs
extern std::vector <double> enbYPositions; //posições no eixo Y dos eNBs
extern std::vector <double> parkedVehiclesXPositions; //posições no eixo X dos veículos estacionados (desligados)
extern std::vector <double> parkedVehiclesYPositions; //posições no eixo Y dos veículos estacionados (desligados)
extern std::vector <double> m_txSafetyRanges; //vetor de ranges usado para a aplicação BSM

extern std::vector <std::vector <double>> edgeMatrixCpuSizeTime; //matriz de tamanho da imagem, tempo, cpu de um edge server
extern std::vector <std::vector <double>> carMatrixCpuSizeTime; //matriz de tamanho da imagem, tempo, cpu de um carro
extern std::vector <std::vector <double>> workloadMatrix; //(posição do vetor interno é o id da task; vector[0]: task size, vector[1]: cpu required)
extern std::vector <std::vector <double>> backupWorkloadMatrix; //(posição do vetor interno é o id da task; vector[0]: task size, vector[1]: cpu required)
extern std::vector <std::vector <double>> scheduleMatrix; //matriz: task destiny (pos do vetor interno:taskId; vec[0]: tasksize, vec[1]: cpu required)

extern std::vector <pair> sortCarProviders; // vetor de pares usado para ordenar os possíveis carros servidores
extern std::vector <pair> sortAuxProviders; //GCF2. vetor de pares usado para ordenar os possíveis servidores

extern std::vector <Ptr<Node>> providers;

extern std::map <uint32_t ,uint32_t> idEdgeEnb; // map para associar um enb com seu SUE (super user equipment) (edge node)
extern std::map <uint32_t ,uint32_t> edgeNodeCPUQueue; //(id do edge node, tempo na fila no edge node)
extern std::map <uint32_t ,uint32_t> carNodeCPUQueue; //(id do carro, tempo na fila no carro)
extern std::map <uint32_t ,uint32_t> carNodeKnownRoutes; //(id do carro, 1 se a rota for conhecida e 0 se n for conhecida)

extern std::map <uint32_t ,double> edgeNodeCPUCap; //(id do edge node, capacidade de processamento da CPU do edge node)
extern std::map <uint32_t ,double> edgeCpuConsumption; //(id do edge server, coeficiente de consumo de energia da CPU em Wh)
extern std::map <uint32_t ,double> carNodeCPUCap; //(id do carro, porcentagem de uso da CPU do carro)
extern std::map <uint32_t ,double> carCpuConsumption; //(id do carro, coeficiente de consumo de energia da CPU em Wh)
extern std::map <uint32_t ,double> carMinimalEnergy; //(id do carro, energia mínima do carro em Wh para topar o offloading)
extern std::map <uint32_t ,double> carEnergyLevel; //(id do carro, nível de energia atual do carro em Wh)

extern std::map <uint32_t ,Ipv4Address>	ipEdgeEnb; // map para associar um enb com seu SUE (super user equipment) (edge node)

extern std::map <std::string ,double> hashSleepTimes; //string das características / sleepTime

extern std::map <Ptr<Node> ,bool> auxProvidersKr; //map auxiliar; node / se known route é conhecido

extern std::map <Ptr<Node> ,uint32_t> auxProvidersCarQueue; //map auxiliar para ajudar na ordenação dos providers; node-car / CPU queue do candidato
extern std::map <Ptr<Node> ,uint32_t> auxProvidersEdgeQueue; //map auxiliar para ajudar na ordenação dos providers; node-edge / CPU queue do candidato

extern std::map <Ptr<Node> ,double> auxProvidersDist; //GCF2. map auxiliar para ajudar na ordenação dos providers; node / distância para o cliente
extern std::map <Ptr<Node> ,double> auxProvidersQueue; //GCF2. map auxiliar para ajudar na ordenação dos providers; node / CPU queue do candidato
extern std::map <Ptr<Node> ,double> auxProvidersCap; //GCF2. map auxiliar para ajudar na ordenação dos providers; node / CPU capacity do candidato
extern std::map <Ptr<Node> ,double> auxProvidersLlt; //map auxiliar; node / link lifetime para o cliente
extern std::map <Ptr<Node> ,double> auxProvidersCpuConsumption; //map auxiliar para ajudar na ordenação dos providers; node / CPU consumption
extern std::map <Ptr<Node> ,double> auxProvidersMinimalEnergy; //map auxiliar para ajudar na ordenação dos providers; node / bateria mínima necessária
extern std::map <Ptr<Node> ,double> auxProvidersEnergyLevel; //map auxiliar para ajudar na ordenação dos providers; node / energia atual da bateria
extern std::map <Ptr<Node> ,double> auxProvidersCarDist; //map auxiliar para ajudar na ordenação dos providers; node / distância para o cliente

extern std::map <Ptr<Node> , Ptr<Socket>> serverRcvRequestSocket; //map para vincular o socket UDP q recebe requests do cliente

extern std::map <Ipv4Address ,double> expectedResultTime; //IP do server q deve receber o upload do cliente / tempo esperado p receber os resultados

extern std::map <Ipv4Address ,Ptr<Socket>> ServerIP_ClientSocket; //IP do server q deve receber o upload do cliente / socket do cliente ao provider
extern std::map <Ptr<Socket>, Ipv4Address> ClientSocket_ServerIP; //socket do cliente ao provider / IP do server q deve receber o upload do cliente

extern std::map <Ptr<Socket> ,bool> clientRcvdResult; //socket ligando client ao provider / informa se recebeu o resultado de cada provider

extern std::map <Ptr<Socket> ,uint32_t> auxProvidersNumberOfTasksToProc; //socket ligando client ao provider / qtd de tasks q provider deve receber
extern std::map <Ptr<Socket> ,uint32_t> auxProvidersPktToRcv; //socket ligando client ao provider / tamanho do pkt que o provider deve receber
extern std::map <Ptr<Socket> ,uint32_t>	server_tcp_recv; // map para saber o quanto de dados o servidor recebeu de cada socket
extern std::map <Ptr<Socket> ,uint32_t>	client_tcp_recv; // map para saber o quanto de dados o cliente recebeu de cada socket
extern std::map <Ptr<Socket> ,uint32_t>	client_tcp_send; // map para saber o quanto de dados o cliente enviou para cada socket
extern std::map <Ptr<Socket> ,uint32_t>	server_tcp_send; // map para saber o quanto de dados o servidor enviou para cada socket

extern std::map <Ptr<Socket> ,double> auxProvidersTaskToProc; //socket ligando client ao provider / cpu cycles required
extern std::map <Ptr<Socket> ,double> clientExpectedResultTime; //socket ligando client ao provider / tempo esperado para receber os resultados do socket

extern Ptr<MmWaveHelper> ptr_mmWave;
extern Ptr<MmWavePointToPointEpcHelper>  epcHelper;

extern InternetStackHelper internet; //para instalar a pilha de internet tcp/ip

extern NetDeviceContainer edgeNetDev; //container de interfaces mmWAVE dos edge servers
extern NetDeviceContainer enbNetDev; //container de interfaces mmWAVE dos enbs
extern NetDeviceContainer clientNetDevMmWave; //container de interfaces mmWAVE do carro cliente
extern NetDeviceContainer devices; //container de interfaces WAVE dos carros

extern NodeContainer c; // Container principal dos carros
extern NodeContainer edgeNodes; //container dos computadores de borda
extern NodeContainer enbNodes; //container dos enbs
extern NodeContainer surrogates; // Container dos surrogates
extern NodeContainer clients; // Container para o cliente
extern NodeContainer bestProviders;

extern Ipv4InterfaceContainer ips;
extern Ipv4InterfaceContainer edgeIpIface;
extern Ipv4InterfaceContainer clientIpIface;

extern Ptr<Socket> mmwReqUncSock; //socket da solicitação em unicast via mmwave
extern Ptr<Socket> wReqBcSock; //socket da solicitação em broadcast via wave
extern Ptr<Socket> client_side; //@TODO: na verdade, o cliente possui vários sockets
extern Ptr<Socket> server_side; //@TODO: na verdade, existem vários sockets de servidor

//Ptr<Node> auxProviders[4]; //array auxiliar para ajudar na ordenação dos providers
//std::map < uint32_t ,uint32_t>   edgeNodeCPUBusy; //(id do edge node, porcentagem de uso da CPU do edge node)
//std::map < uint32_t ,uint32_t>   carNodeCPUBusy; //(id do carro, porcentagem de uso da CPU do carro)
//std::map< Ptr<Socket> , uint32_t> auxProvidersDatarate; //socket ligando client ao provider / datarate do socket (depende da interface)
//std::map< Ptr<Node> , uint32_t> auxProvidersEdgeCpu; //map auxiliar para ajudar na ordenação dos providers; node-edge / CPU busy do candidato
//std::map< Ptr<Node> , uint32_t> auxProvidersCarCpu; //map auxiliar para ajudar na ordenação dos providers; node-car / CPU busy do candidato

#endif /* GLOBALS_H_ */
