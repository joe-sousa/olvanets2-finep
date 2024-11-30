/*
*  Alisson Barbosa, Abril/2020. E-mail: alisson@ufc.br
*  This code realizes the packets exchange between vehicles (V2V) and the edge (V2I), simulating
*  the send of subtasks to be executed in the surrogate vehicles and in the edge of the network.
*
*  This code is protected by copyright and intellectual property right laws.
*  Do not use or distribute without the author's authorization.
*  This is true even for modified versions or snippets of this code.
*
*/

#ifndef OLVANETS_H_
#define OLVANETS_H_

#include "ns3/internet-stack-helper.h"
#include "scratch/olvanets/globals.h"
#include "scratch/olvanets/gcf2.h"
#include "scratch/olvanets/gtt.h"
#include "scratch/olvanets/hvc.h"
#include "scratch/olvanets/random2.h"
#include "scratch/olvanets/abc.h"
#include "scratch/olvanets/mdo.h"
#include <ctime>

using namespace ns3;

//retorna o decimal representando o octeto requerido
std::string getDecimalFromOctet(Ipv4Address curIp, uint32_t octet);
//template para dividir uma string em substrings
std::vector<std::string> split2(const std::string &s, char delim);
//função para dividir uma string em substrings
std::vector<std::string> split(const std::string &s, char delim);
//função para cortar o arquivo de mobilidade e atualizá-lo p a simulação ficar mais rápida
//deixa apenas 2 segundos antes de initAction
void buildTcl();
void readLinesFromFile(std::string filename, std::string goal);
void getEnergyValues ();
//retorna IP do nó, dependendo da interface
Ipv4Address getIPFromServer(Ptr<Node> curNode);
//calcula sleepTime
double calcSleepTime(double cpuCap, double cpuQueue, double howMuchProc);
// Retorna a velocidade de um nó no eixo x
double GetSpeed(Ptr<const MobilityModel> model1);
// Pega a distância de um nó para o outro
double GetDistance(Ptr<const MobilityModel> model1,Ptr<const MobilityModel> model2);
// Pega o ângulo para descobrir a direção
int getAngle(Ptr<const MobilityModel> model1);
// Verifica se os dois nós estão indo na mesma direção
bool sameDirection(Ptr<const MobilityModel> model1,Ptr<const MobilityModel> model2);
// função do Alisson - calcula o tempo de vida do enlace
//double linkEstimatedLifeTime(Vector my_velocity ,Vector  neighbour_velocity,
//		Vector my_position, Vector neighbour_position, double range=200.0){
double linkEstimatedLifeTime(Ptr<MobilityModel> model1, Ptr<MobilityModel> model2, double range);
// Pega o tempo de início do processo
void GetTempoIni();
void checkOffloadingSuccess(bool checkInFinish);
//função para enviar dados maiores que o buffer do socket TCP
void ClientWriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace);
//função para enviar dados maiores que o buffer do socket TCP
void ServerWriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace);
//begin implementation of sending "Application"
void StartFlow (Ptr<Socket> localSocket, Ipv4Address servAddress, uint16_t servPort, std::string upOrDownload);
//Envia os resultados para o cliente depois de fazer o processamento
void serverSendAfter(Ptr<Socket> socket, Ipv4Address ipv4From, uint16_t portFrom);
//Recebe os dados do cliente, processa (depois de sleepTime) e chama SendAfter para enviar os resultados
void serverHandler(Ptr<Socket> socket);
// Aceita a conexão
void serverAccept(Ptr<Socket> socket,const ns3::Address& from);
//Cria o socket servidor e coloca-o para escutar e depois responder
void serverSide();
// Função para receber solicitações dos clientes e responder
void serverListenForRequests(Ptr<Socket> socket);
// Função para responder solicitações dos clientes
void serverSendReply(Ptr<Socket> replySocket, InetSocketAddress dst);
//recebe resultados dos offloadings e imprime no arquivo de resultados
void clientHandler(Ptr<Socket> socket);
//Cria socket para se comunicar com o servidor e envia o pacote
void clientSide();
// Cliente faz solicitação
void clientRequest(Ptr<Socket> socket);
//função para verificar periodicamente se as distâncias com os providers estão ok
//se não, aloca para executar local
void checkConnectivity();
//calcula packetsize, localTime, numberOfSurrogates
void tasksToLocal(uint32_t clientId);
void random2Decision(Ipv4Address ipv4From, uint32_t iface);
//localTime, numberOfSurrogates
void tasksToLocalHvc(uint32_t clientId);
//função que, qdo estoura o tempo, executa o resto das tarefas desalocadas localmente
void hvcTimeoutToFind();
void hvcDecisionNoMmWave();
void hvcDecision(Ipv4Address ipv4From, uint32_t iface);
// Recebe os pacotes de resposta e faz a escolha dos substitutos - todos com capacidade são favoráveis ao offloading
void clientRecRepPkt(Ptr<Socket> socket);
void initializeWave ();
void initializeMmWave ();
void idClient ();
std::vector<double> clientPosition ();
void createStaticMobility ();
void getEnbAndEdgePoints ();
void configureTimeAndNumberOfNodes();



#endif /* OLVANETS_H_ */
