/*
*  Alisson Barbosa, Janeiro/2021. E-mail: alisson@ufc.br
*  This code realizes the packets exchange between vehicles (V2V) and the edge (V2I), simulating
*  the send of subtasks to be executed in the surrogate vehicles and in the edge of the network.
*
*  This code is protected by copyright and intellectual property right laws.
*  Do not use or distribute without the author's authorization.
*  This is true even for modified versions or snippets of this code.
*
*/

#include "scratch/olvanets/globals.h"

/*
***	Variáveis Globais
*/
typedef std::pair<Ptr<Node>,double> pair; //novo tipo para ajudar na ordenação dos providers; node / distância para o cliente

std::ofstream os; //fluxo para o arquivo de log

FILE *fp; //arquivo dos resultados finais

std::chrono::high_resolution_clock::time_point startElapsedTime; //para medir o tempo que roda o algoritmo
std::chrono::high_resolution_clock::time_point endElapsedTime; //para medir o tempo que roda o algoritmo

bool rcvdFirstServerReply = false;
bool alreadyDecided; //se já decidiu como distribuir as tarefas
bool enbNotFound = true; //inicialmente o enb n foi encontrado
bool KEY = true;
bool alreadySent = false; //verifica se o cliente já fez o offloading uma vez
bool alreadySentToSort = false; //verifica se o cliente chamou a ordenação dos candidatos a substitutos
bool alreadyStartedCheckConnectivity = false; //apenas p o hvc: verifica se já iniciou a verificação de conectividade
bool hvcTimeout = false; //estouro de tempo do hvc para executar localmente as tarefas q faltaram

int imgSize;
int tasksOnlyLocal=0; //número de tarefas executadas localmente desde o início
int tasksOffloadedSuc=0; //número de tarefas executadas remotamente com sucesso
int tasksRecovered=0; //número de tarefas offloadadas recuperadas para executar localmente
int tasksCounted=0; //número final de tarefas contadas que é a soma das 3 variáveis anteriores
int nReplies=0; //número de respostas após 0.5s de espera
int numberOfEnergyViolations=0; //número de violações nas restrições de energia

uint32_t initAction;
uint32_t clientId; //id do cliente
uint32_t numberOfNodes;
uint32_t packetSize;
uint32_t resultPacketSize = 1000; //pacote de retorno é apenas uma string
uint32_t workload; //especifica qual workload será executado
uint32_t numberOfSurrogates;
uint32_t numberOfRecoveries = 0; //conta a quantidade de recuperações de falhas
uint32_t run; //usado para ir pegando partes subsequentes da semente (seed)
uint32_t numberOfEdgeServers; //número de servidores de borda e eNBs
uint32_t idxOfServerAccept = 0;
uint32_t idxFromMoreClosestEnb; //índice do enb mais próximo do cliente no initAction
uint32_t idxOfProvidersActionedInServer = 0; //índice dos providers já acionados no servidor
uint32_t idxOfProvidersActionedInClient = 0; //índice dos providers já acionados no cliente
uint32_t numberOfTasks; //número de imagens a ser processadas
uint32_t attempts = 0; //número de tentativas
uint32_t OffloadSuccess=0;
uint32_t foodSources; //qtd de soluções ou food sources do algoritmo ABC
uint32_t numberOfCycles; //número de ciclos do algoritmo ABC

double imgTime;
double trange; //alcance de transmissão WAVE
double txpower; //potência de transmissão WAVE
double sleepTime; // tempo de processamento
double onlyLocalTime; //tempo a ser batido... unicamente local
double variation; //variação do offloadingTime em relação ao onlyLocalTime
double localTime; //o local fica com algum processamento... esse é o tempo desse processamento
double timeOfFirstServerReply;
double datarateMmWave = 3000000000.0; //3 Gbps
double datarateWave = 27000000; //27 Mbps
double startTime; //tempo inicial da simulação
double finishTime; //tempo final da simulação
double tempoIni;
double wAvgCpuReq; //média de cpu required das tasks do workload
double procDeadline; //time limit/deadline para processar tasks
double wSumCpuReq=0.0; //soma de cpu required de todas as tasks do workload
double replyTime = 0.0; //tempo para o servidor responder à solicitação, p evitar concorrência no canal
double elapsedTime; //para medir o tempo que roda o algoritmo

std::string algorithm;
std::string pknownRoutes; //porcentagem de rotas conhecidas
std::string tracePath; //caminho do trace de mobilidade
std::string traceFile; //arquivo de trace de mobilidade
std::string scenario; //urban ou highway
std::string density; //low, medium ou high
std::string cellcoverage; //tipo de cobertura das torres celulares 5G
std::string logFile; //arquivo para gerar os logs

std::vector <uint32_t> sizesOfPackets; //serve p, p ex, enviar apenas uma imagem (tam1), enviar duas imagens (tam2) ...

std::vector <double> sleepTimes; //serve p, p ex, saber quanto tempo de processamento para 1 imagem, 2 imagens ...
std::vector <double> enbXPositions; //posições no eixo X dos eNBs
std::vector <double> enbYPositions; //posições no eixo Y dos eNBs
std::vector <double> m_txSafetyRanges; //vetor de ranges usado para a aplicação BSM

std::vector <std::vector <double>> edgeMatrixCpuSizeTime; //matriz de tamanho da imagem, tempo, cpu de um edge server
std::vector <std::vector <double>> carMatrixCpuSizeTime; //matriz de tamanho da imagem, tempo, cpu de um carro
std::vector <std::vector <double>> workloadMatrix; //(posição do vetor interno é o id da task; vector[0]: task size, vector[1]: cpu required)
std::vector <std::vector <double>> backupWorkloadMatrix; //(posição do vetor interno é o id da task; vector[0]: task size, vector[1]: cpu required)
std::vector <std::vector <double>> scheduleMatrix; //matriz: task destiny (pos do vetor interno:taskId; vec[0]: tasksize, vec[1]: cpu required)

std::vector <pair> sortCarProviders; // vetor de pares usado para ordenar os possíveis carros servidores
std::vector <pair> sortAuxProviders; //GCF2. vetor de pares usado para ordenar os possíveis servidores

std::vector <Ptr<Node>> providers;

std::map <uint32_t ,uint32_t> edgeNodeCPUQueue; //(id do edge node, tempo na fila no edge node)
std::map <uint32_t ,uint32_t> carNodeCPUQueue; //(id do carro, tempo na fila no carro)
std::map <uint32_t ,uint32_t> carNodeKnownRoutes; //(id do carro, 1 se a rota for conhecida e 0 se n for conhecida)
std::map <uint32_t ,uint32_t> idEdgeEnb; // map para associar um enb com seu SUE (super user equipment) (edge node)

std::map <uint32_t ,double> edgeNodeCPUCap; //(id do edge node, capacidade de processamento da CPU do edge node)
std::map <uint32_t ,double> edgeCpuConsumption; //(id do edge server, coeficiente de consumo de energia da CPU em Wh)
std::map <uint32_t ,double> carNodeCPUCap; //(id do carro, porcentagem de uso da CPU do carro)
std::map <uint32_t ,double> carCpuConsumption; //(id do carro, coeficiente de consumo de energia da CPU em Wh)
std::map <uint32_t ,double> carMinimalEnergy; //(id do carro, energia mínima do carro em Wh para topar o offloading)
std::map <uint32_t ,double> carEnergyLevel; //(id do carro, nível de energia atual do carro em Wh)

std::map <uint32_t ,Ipv4Address> ipEdgeEnb; // map para associar um enb com seu SUE (super user equipment) (edge node)

std::map <std::string ,double> hashSleepTimes; //string das características / sleepTime

std::map <Ptr<Node> ,bool> auxProvidersKr; //map auxiliar; node / se known route é conhecido

std::map <Ptr<Node> ,uint32_t> auxProvidersCarQueue; //map auxiliar para ajudar na ordenação dos providers; node-car / CPU queue do candidato
std::map <Ptr<Node> ,uint32_t> auxProvidersEdgeQueue; //map auxiliar para ajudar na ordenação dos providers; node-edge / CPU queue do candidato

std::map <Ptr<Node> ,double> auxProvidersDist; //map auxiliar para ajudar na ordenação dos providers; node / distância para o cliente
std::map <Ptr<Node> ,double> auxProvidersQueue; //map auxiliar para ajudar na ordenação dos providers; node / CPU queue do candidato
std::map <Ptr<Node> ,double> auxProvidersCap; //map auxiliar para ajudar na ordenação dos providers; node / CPU capacity do candidato
std::map <Ptr<Node> ,double> auxProvidersLlt; //map auxiliar; node / link lifetime para o cliente
std::map <Ptr<Node> ,double> auxProvidersCpuConsumption; //map auxiliar para ajudar na ordenação dos providers; node / CPU consumption
std::map <Ptr<Node> ,double> auxProvidersMinimalEnergy; //map auxiliar para ajudar na ordenação dos providers; node / bateria mínima necessária
std::map <Ptr<Node> ,double> auxProvidersEnergyLevel; //map auxiliar para ajudar na ordenação dos providers; node / energia atual da bateria
std::map <Ptr<Node> ,double> auxProvidersCarDist; //map auxiliar para ajudar na ordenação dos providers; node / distância para o cliente

std::map <Ptr<Node> , Ptr<Socket>> serverRcvRequestSocket; //map para vincular o socket UDP q recebe requests do cliente

std::map <Ipv4Address ,double> expectedResultTime; //IP do server q deve receber o upload do cliente / tempo esperado p receber os resultados

std::map <Ipv4Address ,Ptr<Socket>> ServerIP_ClientSocket; //IP do server q deve receber o upload do cliente / socket do cliente ao provider
std::map <Ptr<Socket>, Ipv4Address> ClientSocket_ServerIP; //socket do cliente ao provider / IP do server q deve receber o upload do cliente

std::map <Ptr<Socket> ,bool> clientRcvdResult; //socket ligando client ao provider / informa se recebeu o resultado de cada provider

std::map <Ptr<Socket> ,uint32_t> auxProvidersNumberOfTasksToProc; //socket ligando client ao provider / qtd de tasks q provider deve receber
std::map <Ptr<Socket> ,uint32_t> auxProvidersPktToRcv; //socket ligando client ao provider / tamanho do pkt que o provider deve receber
std::map <Ptr<Socket> ,uint32_t> server_tcp_recv; // map para saber o quanto de dados o servidor recebeu de cada socket
std::map <Ptr<Socket> ,uint32_t> client_tcp_recv; // map para saber o quanto de dados o cliente recebeu de cada socket
std::map <Ptr<Socket> ,uint32_t> client_tcp_send; // map para saber o quanto de dados o cliente enviou para cada socket
std::map <Ptr<Socket> ,uint32_t> server_tcp_send; // map para saber o quanto de dados o servidor enviou para cada socket

std::map <Ptr<Socket> ,double> auxProvidersTaskToProc; //socket ligando client ao provider / cpu cycles required
std::map <Ptr<Socket> ,double> clientExpectedResultTime; //socket ligando client ao provider / tempo esperado para receber os resultados do socket

Ptr<MmWaveHelper> ptr_mmWave;
Ptr<MmWavePointToPointEpcHelper>  epcHelper;

InternetStackHelper internet; //para instalar a pilha de internet tcp/ip

NetDeviceContainer edgeNetDev; //container de interfaces mmWAVE dos edge servers
NetDeviceContainer enbNetDev; //container de interfaces mmWAVE dos enbs
NetDeviceContainer clientNetDevMmWave; //container de interfaces mmWAVE do carro cliente
NetDeviceContainer devices; //container de interfaces WAVE dos carros

NodeContainer c; // Container principal dos carros
NodeContainer edgeNodes; //container dos computadores de borda
NodeContainer enbNodes; //container dos enbs
NodeContainer surrogates; // Container dos surrogates
NodeContainer clients; // Container para o cliente
NodeContainer bestProviders;

Ipv4InterfaceContainer ips;
Ipv4InterfaceContainer edgeIpIface;
Ipv4InterfaceContainer clientIpIface;

Ptr<Socket> mmwReqUncSock; //socket da solicitação em unicast via mmwave
Ptr<Socket> wReqBcSock; //socket da solicitação em broadcast via wave
Ptr<Socket> client_side; //@TODO: na verdade, o cliente possui vários sockets
Ptr<Socket> server_side; //@TODO: na verdade, existem vários sockets de servidor

//uint32_t numberOfParkedVehicles; //número de veículos estacionados, desligados, mas funcionando CPU e 802.11p
//std::vector <double> parkedVehiclesXPositions; //posições no eixo X dos veículos estacionados (desligados)
//std::vector <double> parkedVehiclesYPositions; //posições no eixo Y dos veículos estacionados (desligados)
//std::map< Ptr<Node> , uint32_t> auxProvidersCarCpu; //map auxiliar para ajudar na ordenação dos providers; node-car / CPU busy do candidato
//std::map< Ptr<Node> , uint32_t> auxProvidersEdgeCpu; //map auxiliar para ajudar na ordenação dos providers; node-edge / CPU busy do candidato
//std::map< Ptr<Socket> , uint32_t> auxProvidersDatarate; //socket ligando client ao provider / datarate do socket (depende da interface)
//std::map < uint32_t ,uint32_t>   edgeNodeCPUBusy; //(id do edge node, porcentagem de uso da CPU do edge node)
//std::map < uint32_t ,uint32_t>   carNodeCPUBusy; //(id do carro, porcentagem de uso da CPU do carro)
//Ptr<Node> auxProviders[4]; //array auxiliar para ajudar na ordenação dos providers
//NodeContainer cParkedVehicles; // Container dos carros estacionados
