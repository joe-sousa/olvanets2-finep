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
#include "ns3/ipv4-interface-container.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wall-clock-synchronizer.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/wave-bsm-helper.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/global-route-manager.h"
#include "scratch/olvanets/olvanets.h"

#include <iostream>
#include <fstream>
#include <unistd.h>
//#include <stdlib.h>

using namespace ns3;
using namespace mmwave;

NS_LOG_COMPONENT_DEFINE ("olvanets");


//garante que o alcance de transmissão seja configurado
/*void ConfigureTransmissionRangeTRG(double trange, YansWifiPhyHelper &wifiPhy) {
	//wifiPhy.Set("TxGain", DoubleValue(1.0));
	//wifiPhy.Set("RxGain", DoubleValue(1.0));
	//wifiPhy.Set("ChannelNumber",  UintegerValue (CCH));
	if(trange == 200.0){
		txpower = 0.4;
	}
	if(trange == 249.0){
		txpower = 2.3;
	}
	if(trange == 251.0){
			txpower = 2.4;
	}
	else { //para os parâmetros atuais, não estava conseguindo aplicar a fórmulado do TRG, então achei uma parecida a partir de 200m (testando)
		double pt = 0.4 + ((trange - 200.0)/2.578)*0.1;
		txpower = pt;
	}
}*/

std::string getDecimalFromOctet(Ipv4Address curIp, uint32_t octet){
	std::stringbuf sbuf;
	std::ostream sIp(&sbuf);
	curIp.Print(sIp);
	std::string stringIp = sbuf.str();
	std::vector<std::string> tempSubstr;
	tempSubstr = split(stringIp, '.'); //divide a string em substrings
	return tempSubstr[octet];
}

std::vector<std::string> split2(const std::string &s, char delim) {
    std::istringstream iss(s);
    std::string item;
    std::vector<std::string> resultVecStr;
    while (std::getline(iss, item, delim)) {
    	resultVecStr.push_back(item);
    }
    return resultVecStr;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    elems = split2(s, delim);
    return elems;
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
	float lastTimeGetted = 0.0;

	std::map < std::string , std::vector<std::string>>	initialNodePositions; //matriz para armazenar as posições iniciais de cada nó; (node, posx, posy)
	if(initAction >= 2){
		int index = initAction-2;

		// escrever apenas a parte de setdest
		std::string fnameComplete = tracePath + traceFile;
		std::string newFnameComplete = tracePath + "tempCurrent.tcl";
		arq = fopen(fnameComplete.c_str(), "rt");
		arqWrite = fopen(newFnameComplete.c_str(), "wt");
		if (arq == NULL){	printf("Problemas na abertura do arquivo\n");	}

		while (!feof(arq)) {
			if(fscanf(arq, "%s", a)){
				if(strcmp(a, "$ns_") == 0){
					if(fscanf(arq, "%s %f %s", b, &c, d)){
						lastTimeGetted = c;
						if (c >= index && strcmp (a, "$ns_") == 0){
							if(fscanf(arq, "%s %s %s %s",e,f,g,h))
								fprintf(arqWrite,"%s %s %.1f %s %s %s %s %s\n", a, b, c-index, d,e,f,g,h);
						}
						if (c < index && strcmp (a, "$ns_") == 0){ //para atualizar as posições iniciais
							if(fscanf(arq, "%s %s %s %s",e,f,g,h)){
								std::string cutD = d;
								std::string cutF = f;
								std::string cutG = g;
								std::string curnode = cutD.substr(cutD.find("(") + 1, (cutD.size()-8)); //o 8 é de "$node_(
								curnode = curnode.substr (0, curnode.length() - 1);

								//atualizando as posições iniciais
								if(initialNodePositions[curnode].size()>0){
									initialNodePositions[curnode].clear(); //
								}
								initialNodePositions[curnode].push_back(cutF);
								initialNodePositions[curnode].push_back(cutG);
							}
						}

					}
				}
				else if(strstr(a,"$node")){ //armazenar as posições iniciais
					if(fscanf(arq, "%s %s %s", b, d, e)){
						if (lastTimeGetted >= index){  //deixar imprimir apenas as posições iniciais setadas depois de index
							fprintf(arqWrite,"%s %s %s %s\n", a,b,d,e);
						}
						else { //armazenar as posições posições iniciais setadas antes de index
							//pegando o node
							std::string cutA = a;
							std::string curnode = cutA.substr(cutA.find("(") + 1, (cutA.size()-7)); //o 7 é de $node_(
							curnode = curnode.substr (0, curnode.length() - 1);

							//convertendo char em string
							std::string sd = d;
							std::string se = e;

							if(sd == "X_"){
								//double de = std::stod(se);
								initialNodePositions[curnode].push_back(se);
							}
							if(sd == "Y_"){
								//double de = std::stod(se);
								initialNodePositions[curnode].push_back(se);
							}
						}

					}
				}
			}
		}
		fclose(arq);
		fclose(arqWrite);


		//agora anexar as posições iniciais no começo de um novo arquivo
		fnameComplete = tracePath + "tempCurrent.tcl";
		newFnameComplete = tracePath + "current.tcl";
		arq = fopen(fnameComplete.c_str(), "rt");
		arqWrite = fopen(newFnameComplete.c_str(), "wt");
		if (arq == NULL) {	printf("Problemas na abertura do arquivo\n");	}

		//for(int i = 0; initialNodePositions.size(); i++){
		for(std::map< std::string,std::vector<std::string>>::iterator it = initialNodePositions.begin();
			it != initialNodePositions.end(); ++it){
				fprintf(arqWrite, "$node_(%s) set X_ %s\n", it->first.c_str(), it->second[0].c_str());
				fprintf(arqWrite, "$node_(%s) set Y_ %s\n", it->first.c_str(), it->second[1].c_str());
				fprintf(arqWrite, "$node_(%s) set Z_ 0\n", it->first.c_str());
		}


		while (!feof(arq))
		{
			if(fscanf(arq, "%s", a)){
				if(strcmp(a, "$ns_") == 0){
					if(fscanf(arq, "%s %f %s", b, &c, d)){
						if (strcmp (a, "$ns_") == 0){
							if(fscanf(arq, "%s %s %s %s",e,f,g,h))
								fprintf(arqWrite,"%s %s %.1f %s %s %s %s %s\n", a, b, c, d,e,f,g,h);
						}
					}
				}
				else if(strstr(a,"$node")){ //armazenar as posições iniciais
					if(fscanf(arq, "%s %s %s", b, d, e)){
						fprintf(arqWrite,"%s %s %s %s\n", a,b,d,e);
					}
				}
			}
		}
		fclose(arq);
		fclose(arqWrite);

	}
}

void readLinesFromFile(std::string filename, std::string goal){
	const char* fname = filename.c_str();
	fp = fopen(fname, "r");  //arquivo com os dados de ocupação de CPU dos edge nodes
	if(fp == NULL){
		perror("Perdoe-nos, não conseguimos abrir o arquivo");
		exit(1);
	}
	char *line = NULL;
	size_t len = 0;
	uint32_t numberOfLine = 0;
	if(goal=="workload"){
		std::vector<double> tempDouble;
		while(getline(&line, &len, fp) != -1){
			std::string temp(line); //id;task size;CPU Gigacycles required
			std::vector<std::string> tempSubstr;
			tempSubstr = split(temp, ';'); //divide a string em substrings
			tempDouble.push_back(std::stod(tempSubstr[0])); //pega o tamanho da task
			tempDouble.push_back(std::stod(tempSubstr[1])); //pega CPU Gigacycles required
			workloadMatrix.push_back(tempDouble); //posição do vetor interno é o id da task; vector[0]: size, vector[1]: cpu required
			tempDouble.clear(); //zera o vetor
			numberOfLine++;
		}
	}
	if(goal=="edgeCPU"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfEdgeServers)){
			//edgeNodeCPUBusy[numberOfLine] = atoi(line); //line é o uso de CPU; numberOfLine é o ID do edgeNode
			edgeNodeCPUCap[numberOfLine] = atof(line); //line é a capacidade de CPU; numberOfLine é o ID do edgeNode
			numberOfLine++;
		}
	}
	if(goal=="carCPU"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfNodes)){
			//carNodeCPUBusy[numberOfLine] = atoi(line); //line é o uso de CPU; numberOfLine é o ID do edgeNode
			carNodeCPUCap[numberOfLine] = atof(line); //line é a capacidade de CPU; numberOfLine é o ID do edgeNode
			numberOfLine++;
		}
	}
	if(goal=="edgeQueue"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfEdgeServers)){
			edgeNodeCPUQueue[numberOfLine] = atoi(line); //line é o tempo na fila do edge; numberOfLine é o ID do edgeNode
			numberOfLine++;
		}
	}
	if(goal=="carQueue"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfNodes)){
			carNodeCPUQueue[numberOfLine] = atoi(line); //line é o tempo na fila do carro; numberOfLine é o ID do edgeNode
			numberOfLine++;
		}
	}
	if(goal=="carKnownRoutes"){
			while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfNodes)){
				carNodeKnownRoutes[numberOfLine] = atoi(line); //line é o tempo na fila do carro; numberOfLine é o ID do edgeNode
				numberOfLine++;
			}
		}
	if(goal=="enb-xpoints"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfEdgeServers)){
			enbXPositions.push_back(atof(line));
			numberOfLine++;
		}
	}
	if(goal=="enb-ypoints"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfEdgeServers)){
			enbYPositions.push_back(atof(line));
			numberOfLine++;
		}
	}
	if(goal=="carCpuConsumption"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfNodes)){
			carCpuConsumption[numberOfLine] = atof(line); //line é o coeficiente de consumo de energia da CPU em Wh; numberOfLine é o ID do carro
			numberOfLine++;
		}
	}
	if(goal=="edgeCpuConsumption"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfEdgeServers)){
			edgeCpuConsumption[numberOfLine] = atof(line); //line é o coeficiente de consumo de energia da CPU em Wh; numberOfLine é o ID do carro
			numberOfLine++;
		}
	}
	if(goal=="carMinimalEnergy"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfNodes)){
			carMinimalEnergy[numberOfLine] = atof(line); //line é o coeficiente de consumo de energia da CPU em Wh; numberOfLine é o ID do carro
			numberOfLine++;
		}
	}
	if(goal=="carEnergyLevel"){
		while((getline(&line, &len, fp) != -1) && (numberOfLine < numberOfNodes)){
			carEnergyLevel[numberOfLine] = atof(line); //line é o coeficiente de consumo de energia da CPU em Wh; numberOfLine é o ID do carro
			numberOfLine++;
		}
	}
	fclose(fp);
	free(line);
}

void getEnergyValues () {
	readLinesFromFile("inputs/energy/carCpuConsumption.txt", "carCpuConsumption");
	readLinesFromFile("inputs/energy/edgeCpuConsumption.txt", "edgeCpuConsumption");
	//Abaixo apenas para os carros. Edge server tem minimal -999 e level 999999
	if(scenario == "highway"){
		readLinesFromFile("inputs/energy/minimal-highway.txt", "carMinimalEnergy");
		readLinesFromFile("inputs/energy/level-highway.txt", "carEnergyLevel");
	}
	else{
		readLinesFromFile("inputs/energy/minimal-urban-"+density+".txt", "carMinimalEnergy");
		readLinesFromFile("inputs/energy/level-urban-"+density+".txt", "carEnergyLevel");
	}
}

Ipv4Address getIPFromServer(Ptr<Node> curNode){

	/*uint32_t iface = 0;
	Ptr<NetDevice> curDevice = curNode->GetDevice(iface);
	TypeId tid1 = curDevice->GetInstanceTypeId();
	std::string tidName = tid1.GetName(); //pega o nome da interface de rede
	if(tidName == "ns3::LoopbackNetDevice"){ //tenta pegar uma interface q n seja loopback
		iface = 1;
		curDevice = curNode->GetDevice(iface);
		tid1 = curDevice->GetInstanceTypeId();
		tidName = tid1.GetName(); //pega o nome da interface de rede
	}*/
	Ipv4Address ipv4Return = curNode->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal();
	return ipv4Return;
}

Ptr<Node> getNodeFromIP(Ipv4Address ipv4){
	Ptr<Node> node;
	std::string firstOctet = getDecimalFromOctet(ipv4, 0);
	if(firstOctet == "7"){ //é da rede dos edge servers,
		for (size_t i = 0; i < edgeNodes.GetN(); i++) {
			if (ipv4 == edgeNodes.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //a borda só tem interface 5G
				node = edgeNodes.Get(i);
				break;
			}
		}
	}
	else {
		for (size_t i = 0; i < surrogates.GetN(); i++) {
			if (ipv4 == surrogates.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //se o surrogate escolhido respondeu
				node = surrogates.Get(i);
				break;
			}
		}
	}
	return node;
}

//calcula sleepTime
double calcSleepTime(double cpuCap, double cpuQueue, double howMuchProc){
	//tempo é o tempo de fila + o tempo de processamento das tarefas
	double calcTime = cpuQueue; //acrescenta o tempo de fila (das tarefas q já estavam antes)
	calcTime = calcTime + howMuchProc/cpuCap; //acrescenta os tempos de processamento de cada tarefa
	return calcTime;
}

// Retorna a velocidade de um nó no eixo x
double GetSpeed(Ptr<const MobilityModel> model1){
	return sqrt(pow(model1->GetVelocity().x, 2.0) + pow(model1->GetVelocity().y, 2.0));
}

// Pega a distância de um nó para o outro
/*double GetDistance(Ptr<const MobilityModel> model1,Ptr<const MobilityModel> model2){
	return model1->GetDistanceFrom (model2);
}*/

// Pega a distância de um nó para o outro
double GetDistance(Ptr<const MobilityModel> model1,Ptr<const MobilityModel> model2){
	double dist = sqrt(pow((model1->GetPosition().x - model2->GetPosition().x), 2.0) +
			pow((model1->GetPosition().y - model2->GetPosition().y), 2.0));
	return dist;
}

// Pega o ângulo para descobrir a direção
int getAngle(Ptr<const MobilityModel> model1){
	return atan2(model1->GetVelocity().y,model1->GetVelocity().x)*180/PI;
}

// Verifica se os dois nós estão indo na mesma direção
bool sameDirection(Ptr<const MobilityModel> model1,Ptr<const MobilityModel> model2){
	// Moving to the east
	if (getAngle(model1) == getAngle(model2)) {
		return true;
	}
	else{
		return false;
	}
}

// função do Alisson - calcula o tempo de vida do enlace
//double linkEstimatedLifeTime(Vector my_velocity ,Vector  neighbour_velocity,
//		Vector my_position, Vector neighbour_position, double range=200.0){
double linkEstimatedLifeTime(Ptr<MobilityModel> model1, Ptr<MobilityModel> model2, double range){

	Vector my_velocity = model1->GetVelocity();
	Vector neighbour_velocity = model2->GetVelocity();
	Vector my_position = model1->GetPosition();
	Vector neighbour_position = model2->GetPosition();
	double distance = GetDistance(model1,model2);
	double mySpeed = GetSpeed(model1);
	double surrogateSpeed = GetSpeed(model2);

	//std::cout << "[SISTEMA] Distância: "<< distance << ". Velocidade do cliente: " << mySpeed << ". Velocidade do servidor: " <<
	//		surrogateSpeed << std::endl;
	std::cout << "Dist: "<< distance << ". Client vel.: " << mySpeed << ". Server vel.: " << surrogateSpeed << ". ";
	os << "Dist: "<< distance << ". Client vel.: " << mySpeed << ". Server vel.: " << surrogateSpeed << ". ";
	// std::cout << "Angle (client): "<< getAngle(model1) << std::endl;
	// std::cout << "Angle (surrogate): "<< getAngle(model2) << std::endl;

	double let = 100.0; //só p tirar o warning
	double a, b, c, x_1, x_2, delta;
	//se os carros estão parados
	if ((my_velocity.x == 0.0) && (my_velocity.y == 0.0) && (neighbour_velocity.x == 0.0)
			&& (neighbour_velocity.y == 0.0)) {
		let = 100.0;
	} else { //pelo menos um dos carros está em movimento
		if (my_velocity.x - neighbour_velocity.x == 0.0) { neighbour_velocity.x = neighbour_velocity.x + 0.00001; }
		if (my_velocity.y - neighbour_velocity.y == 0.0) { neighbour_velocity.y = neighbour_velocity.y + 0.00001; }
		if (my_position.x - neighbour_position.x == 0.0) { neighbour_position.x = neighbour_position.x + 0.00001; }
		if (my_position.y - neighbour_position.y == 0.0) { neighbour_position.y = neighbour_position.y + 0.00001; }

		a = pow((my_velocity.x - neighbour_velocity.x), 2.0) +
				pow((my_velocity.y - neighbour_velocity.y), 2.0);
		b = 2*( (my_position.x - neighbour_position.x)*(my_velocity.x - neighbour_velocity.x)
				+(my_position.y - neighbour_position.y)*(my_velocity.y - neighbour_velocity.y)
		);
		c = pow( (my_position.x - neighbour_position.x) , 2.0) +
				pow( (my_position.y - neighbour_position.y) ,2.0) - pow(range, 2.0);
		if (a == 0.0)	{a = 0.000000001;}
		delta = pow(b, 2.0) - (4.0*a*c);


		if (delta > 0.0) {
			x_1 = (-b + pow( delta , 0.5))/(2.0*a);
			x_2 = (-b - pow( delta , 0.5))/(2.0*a);
			if (x_1 > 0.0) {
				let = x_1;
			} else if (x_2 > 0.0) {
				let = x_2;
			}
		} else if (delta == 0.0) {
			let = (-b)/(2.0*a);
		} else {
			let = 100.0;
		}
	}

	//limite superior do let. no caso de os veículos terem mobilidades semelhantes.
	//exemplo: dois carros se movendo na mesma direção e com as mesmas velocidades.
	if(let > 100.0) { let = 100.0; }
	if(distance >= range) { let = 0.0; } //se estiver igual ou além do range, zera o let

	return let;
}

// Pega o tempo de início do processo
void GetTempoIni() {
	tempoIni =  Simulator::Now().GetSeconds ();
	std::cout << "[SISTEMA] Tempo inicial é ==> "<< tempoIni << std::endl;
	os << "[SISTEMA] Tempo inicial é ==> "<< tempoIni << std::endl;
}

void checkOffloadingSuccess(bool checkInFinish) {
	//std::string filename = "results/" + scenario + "-" + density + "-w" + std::to_string(workload) + "-" + algorithm + ".tr";
	std::string filename = "results/" + scenario + "-" + density + "-" + cellcoverage + "-w" + std::to_string(workload) + "-" + algorithm + ".tr";
	//std::string filename = "results/" + scenario + "-" + density + "-kr" + pknownRoutes + "-w" + std::to_string(workload) +
	//		"-" + algorithm + ".tr";
	const char* fname = filename.c_str();
	fp = fopen(fname, "a+");  //arquivo com resultados
	char offlSuc[ 4 ]; //apenas transformando uint32_t em char* para o fprintf
	sprintf(offlSuc,"%u", OffloadSuccess); //apenas transformando uint32_t em char* para o fprintf

	if(checkInFinish == true){  //para checar apenas no final
		std::time_t realTime = std::time(nullptr);
		std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
		os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
		if(idxOfProvidersActionedInClient==0){ //não ativou nenhum provider no cliente... isto é, n encontrou ninguém
			tasksCounted = tasksOnlyLocal + tasksOffloadedSuc + tasksRecovered;
			//não houve envio do workload pelo cliente
			std::cout << "[SISTEMA] DNF - Nenhum Offloading Realizado =(" << std::endl; //detectado no fim (DNF)
			os << "[SISTEMA] DNF - Nenhum Offloading Realizado =(" << std::endl;
			fprintf(fp,"N;%s;0.0;%lu;%lu;%lu;%.3f;%.3f;%i;%i;%i;%i;%.3f;%i;%lu\n", offlSuc, (unsigned long) numberOfSurrogates,
				(unsigned long) idxOfProvidersActionedInClient, (unsigned long) numberOfRecoveries, onlyLocalTime,
				variation, tasksOnlyLocal, tasksOffloadedSuc, tasksRecovered, tasksCounted, elapsedTime, numberOfEnergyViolations,
				(unsigned long) run);
		}
		if(idxOfProvidersActionedInClient>0){ //acionou providers p enviar workloads, mas o offloading falhou
			if((numberOfSurrogates > 0) && (OffloadSuccess < numberOfSurrogates)){
				tasksCounted = tasksOnlyLocal + tasksOffloadedSuc + tasksRecovered;
				std::cout << "[SISTEMA] Offloading FAIL =(" << std::endl;
				os << "[SISTEMA] Offloading FAIL =(" << std::endl;
				//deu certo;qtd de sucessos;tempo;nsd;p qtos tentou enviar;número de recuperações
				//não contabilizo o tempo quando existe falha, porque não terminou a execução
				fprintf(fp,"F;%s;0.0;%lu;%lu;%lu;%.3f;%.3f;%i;%i;%i;%i;%.3f;%i;%lu\n", offlSuc, (unsigned long) numberOfSurrogates,
						(unsigned long) idxOfProvidersActionedInClient, (unsigned long) numberOfRecoveries, onlyLocalTime,
						variation, tasksOnlyLocal, tasksOffloadedSuc, tasksRecovered, tasksCounted, elapsedTime, numberOfEnergyViolations,
						(unsigned long) run);
			}
		}
		fclose(fp);
		exit(0);
	} //fim da chegagem no fim da simulação
	if(checkInFinish == false){   //para checar durante a simulação
		//if(idxOfProvidersActionedInClient==0 && numberOfSurrogates == 0){ //se depois de tentativas, não achou servidores
		if(idxOfProvidersActionedInClient==0){ //se depois de tentativas, não achou servidores
			std::time_t realTime = std::time(nullptr);
			std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
			os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;

			std::cout << "[SISTEMA] DAF - Nenhum Offloading Realizado =(" << std::endl; //detectado antes do fim (DAF)
			os << "[SISTEMA] DAF - Nenhum Offloading Realizado =(" << std::endl;

			tasksOnlyLocal = numberOfTasks; //tudo vai ser executado localmente
			tasksOffloadedSuc=0; //tudo vai ser executado localmente
			tasksRecovered=0; //tudo vai ser executado localmente
			tasksCounted = tasksOnlyLocal + tasksOffloadedSuc + tasksRecovered;
			//tempo de execução todo local... não estou contando que o tempo passou e a fila diminuiu
			double currentTime = Simulator::Now().GetSeconds ();
			localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
			localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], wSumCpuReq);

			variation = ((localTime/onlyLocalTime)-1.0)*100.0; //variação em relação ao onlyLocalTime

			fprintf(fp,"N;%s;%.3f;%lu;%lu;%lu;%.3f;%.3f;%i;%i;%i;%i;%.3f;%i;%lu\n", offlSuc, localTime, (unsigned long) numberOfSurrogates,
					(unsigned long) idxOfProvidersActionedInClient, (unsigned long) numberOfRecoveries, onlyLocalTime,
					variation, tasksOnlyLocal, tasksOffloadedSuc, tasksRecovered, tasksCounted, elapsedTime, numberOfEnergyViolations,
					(unsigned long) run);
			fclose(fp);
			exit(0);
		}
		else if ((OffloadSuccess == numberOfSurrogates)) {
			std::time_t realTime = std::time(nullptr);
			std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
			os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;

			tasksCounted = tasksOnlyLocal + tasksOffloadedSuc + tasksRecovered;
			double tempoFin = Simulator::Now().GetSeconds ();
			std::cout << "[SISTEMA] Offloading OK =)" << std::endl;
			os << "[SISTEMA] Offloading OK =)" << std::endl;
			std::cout << "[SISTEMA] Tempo total do offloading: "<< (tempoFin-tempoIni) << std::endl;
			os << "[SISTEMA] Tempo total do offloading: "<< (tempoFin-tempoIni) << std::endl;
			std::cout << "[SISTEMA] Tempo do local: "<< localTime << std::endl;
			os << "[SISTEMA] Tempo do local: "<< localTime << std::endl;

			double tempoResultante = std::max((tempoFin-tempoIni),localTime);
			variation = ((tempoResultante/onlyLocalTime)-1.0)*100.0; //variação em relação ao onlyLocalTime
			//imprime no arquivo de resultados
			//deu certo;qtd de sucessos;tempo;nsd;p qtos tentou enviar, # de recuperações de falhas
			fprintf(fp,"T;%s;%.3f;%lu;%lu;%lu;%.3f;%.3f;%i;%i;%i;%i;%.3f;%i;%lu\n", offlSuc, tempoResultante,(unsigned long)numberOfSurrogates,
					(unsigned long)idxOfProvidersActionedInClient, (unsigned long) numberOfRecoveries, onlyLocalTime,
					variation, tasksOnlyLocal, tasksOffloadedSuc, tasksRecovered, tasksCounted, elapsedTime, numberOfEnergyViolations,
					(unsigned long) run);
			fclose(fp);
			exit(0);
		}
	}
}

//função para enviar dados maiores que o buffer do socket TCP
void ClientWriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
	//while (client_tcp_send[localSocket] < packetSize && localSocket->GetTxAvailable () > 0)
	//if (client_tcp_send[localSocket] < packetSize && localSocket->GetTxAvailable () > 0)
	if (client_tcp_send[localSocket] < auxProvidersPktToRcv[localSocket] && localSocket->GetTxAvailable () > 0)
	{
		uint32_t tcpsegment = 1500;
		uint32_t left = auxProvidersPktToRcv[localSocket] - client_tcp_send[localSocket];
		uint32_t toSend = std::min (left, localSocket->GetTxAvailable ());
		toSend = std::min (toSend, tcpsegment);
		Ptr<Packet> p = Create<Packet> (toSend);
		//NS_LOG_DEBUG ("Source send data=\"" << GetString (p) << "\"");

		int sent = localSocket->Send (p, 0);
		//InetSocketAddress dst = InetSocketAddress(ClientSocket_ServerIP[localSocket],5555);
		//int sent = localSocket->SendTo(p, 0, dst);

		//NS_TEST_EXPECT_MSG_EQ ((sent != -1), true, "Error during send ?");
		if(sent < 0)
		{
			// we will be called again when new tx space becomes available.
			return;
		}
		client_tcp_send[localSocket] += sent;
	}
	//else if (client_tcp_send[localSocket] >= packetSize ) {
	if (client_tcp_send[localSocket] >= auxProvidersPktToRcv[localSocket] ) {
		return;
	}
	//descobrir a interface do socket para pegar o data rate
	Ptr<NetDevice> curDevice = localSocket->GetBoundNetDevice(); //interface de rede deste socket (se wave ou 5g)
	TypeId tid1 = curDevice->GetInstanceTypeId();
	std::string tidName = tid1.GetName(); //pega o nome da interface de rede
	double datarate = 0.0;
	if(tidName == "ns3::MmWaveUeNetDevice"){
		datarate = datarateMmWave;
	}
	if(tidName == "ns3::WifiNetDevice"){
		datarate = datarateWave;
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
	if (server_tcp_send[localSocket] < resultPacketSize && localSocket->GetTxAvailable () > 0)
	{
		uint32_t tcpsegment = 1500;
		uint32_t left = resultPacketSize - server_tcp_send[localSocket];
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
	if(server_tcp_send[localSocket] == resultPacketSize) {
	//if(server_tcp_send[localSocket] >= packetSize) { //temporário
		//	  tcp_send[localSocket] = 0; //zera depois de transmitir tudo
		//localSocket->Close ();
		return;
	}
	//descobrir a interface do socket para pegar o data rate
	Ptr<NetDevice> curDevice = localSocket->GetBoundNetDevice(); //interface de rede deste socket (se wave ou 5g)
	TypeId tid1 = curDevice->GetInstanceTypeId();
	std::string tidName = tid1.GetName(); //pega o nome da interface de rede
	double datarate = 0.0;
	if(tidName == "ns3::MmWaveUeNetDevice"){
		datarate = datarateMmWave;
	}
	if(tidName == "ns3::WifiNetDevice"){
		datarate = datarateWave;
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
	Ptr<Node> curNode = socket->GetNode(); //nó atual deste socket
	Ipv4Address ipv4To = getIPFromServer(curNode);

	//distância
	Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
	Ptr<MobilityModel> model2 = curNode->GetObject<MobilityModel>(); //pega o modelo de mobilidade deste server
	double distance = GetDistance(model1,model2);

	std::cout << "[SERVIDOR] " << ipv4To << ". Dist.: " << distance << ". Enviando o resultado em " <<
			Simulator::Now().GetSeconds () << "." << std::endl;
	os << "[SERVIDOR] " << ipv4To << ". Dist.: " << distance << ". Enviando o resultado em " <<
			Simulator::Now().GetSeconds () << "." << std::endl;

	//@socket->Send(Create<Packet> (packetSize),0);
	StartFlow(socket,ipv4From, portFrom,"download");
	//socket->Close();
}

//Recebe os dados do cliente, processa (depois de sleepTime) e chama SendAfter para enviar os resultados
void serverHandler(Ptr<Socket> socket) {
	//@Ptr<Packet> pk = socket->Recv(2288,0); //recebe os dados do cliente  //@TODO: por que é 2014?
	//Ptr<Packet> pk = socket->Recv(packetSize,0); //@TODO: como garante que recebeu o pacote todo?

	while(socket->GetRxAvailable() > 0){
		Address from;
		Ptr<Packet> pack2 = socket->RecvFrom (from);
		//std::cout << "[SERVIDOR] Pacote recebido " << std::endl;
		uint32_t packet2 = pack2->GetSize();
		server_tcp_recv[socket] += packet2;  //@TODO: garantir para zerar depois esse map
		Ipv4Address ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();  //ipv4 do cliente
		uint16_t portFrom = InetSocketAddress::ConvertFrom(from).GetPort(); //porta do cliente

		//server: qual meu IP?
		Ipv4Address ipv4To; //endereço do nó atual (na interface atual) que está recebendo
		uint32_t iface = 0; //interface do socket (mmwave ou wave)
		Ptr<Node> curNode = socket->GetNode(); //nó atual deste socket
		Ptr<NetDevice> curDevice = socket->GetBoundNetDevice(); //interface de rede deste socket (se wave ou 5g)
		TypeId tid1 = curDevice->GetInstanceTypeId();
		std::string tidName = tid1.GetName(); //pega o nome da interface de rede
		if(tidName == "ns3::MmWaveUeNetDevice"){
			iface = 1;   //servidor só tem 1 interface, a 0 é loopback
		}
		if(tidName == "ns3::WifiNetDevice"){
			iface = 1;
		}
		//iface = 1;
		ipv4To = curNode->GetObject<Ipv4>()->GetAddress (iface, 0).GetLocal();

		//PEGAR DISTÂNCIA
		uint32_t curId = curNode->GetId();
		Ptr<MobilityModel> model1;
		std::string firstOctet = getDecimalFromOctet(ipv4To, 0);
		if(firstOctet == "7"){
			model1 = enbNodes.Get(0)->GetObject<MobilityModel>(); //pega o modelo de mobilidade do enb associado
		}
		else{
			model1 = c.Get(curId)->GetObject<MobilityModel>();
		}
		Ptr<MobilityModel> model2 = clients.Get(0)->GetObject<MobilityModel>();
		double distance = GetDistance(model1,model2);

		std::cout << "[SERVIDOR] "<< ipv4To << " recebendo dados" << "Tempo ==> " << Simulator::Now().GetSeconds () <<
				". Distance ==> " << distance << std::endl;

		Ptr<Socket> clientSocket = ServerIP_ClientSocket[ipv4To]; //pega o socket do cliente q está enviando p ver o qto tem q receber

		if (server_tcp_recv[socket]>=auxProvidersPktToRcv[clientSocket]) {
			Ptr<Node> curNode = socket->GetNode(); //nó atual deste socket
			uint32_t curId = curNode->GetId();
			//qual interface está limitada neste socket?
			Ptr<NetDevice> curDevice = socket->GetBoundNetDevice(); //interface de rede deste socket (se wave ou 5g)
			TypeId tid1 = curDevice->GetInstanceTypeId();
			std::string tidName = tid1.GetName(); //pega o nome da interface de rede
			//std::string tidName = "ns3::WifiNetDevice";

			//uint32_t cpuBusy = 0; //variável para pegar a cpufree do nó
			double cpuCap = 0; //pega a capacity de cpu do nó
			double cpuQueue = 0; //pega a capacity de cpu do nó
			if(tidName == "ns3::MmWaveUeNetDevice"){
				cpuCap = edgeNodeCPUCap[idxFromMoreClosestEnb];
				cpuQueue = edgeNodeCPUQueue[idxFromMoreClosestEnb];
				//sleepTime = sleepTimeCloudnet2(auxProvidersPktToRcv[clientSocket], //par: tamanho do pkt rcvd
				//		cpuBusy , 1); //par: cpuBusy do server, iface 1 (5g)
				sleepTime = calcSleepTime(cpuCap, cpuQueue , auxProvidersTaskToProc[clientSocket]);
			}
			if(tidName == "ns3::WifiNetDevice"){
				//Qual o id e posição dele no container?
				uint32_t positionInC = 0;
				for(uint32_t i=0; i<c.GetN();i++){
					if(curId == c.Get(i)->GetId()){
						positionInC = i;
						break;
					}
				}
				cpuCap = carNodeCPUCap[positionInC];
				cpuQueue = carNodeCPUQueue[positionInC];
				//sleepTime = sleepTimeCloudnet2(auxProvidersPktToRcv[clientSocket], //par: tamanho do pkt rcvd
				//		cpuBusy, 2); //par: cpuBusy do server, iface 2 (wave)
				sleepTime = calcSleepTime(cpuCap, cpuQueue, auxProvidersTaskToProc[clientSocket]);
			}

			std::cout << "[SERVIDOR] " << ipv4To << " com CpuCap ==> " << cpuCap << " e CpuQueue ==> " << cpuQueue <<
					". Tarefa recebida. Tempo antes de processar ==> " << Simulator::Now().GetSeconds () << ". Processando..." << std::endl;
			os << "[SERVIDOR] " << ipv4To << " com CpuCap ==> " << cpuCap << " e CpuQueue ==> " << cpuQueue <<
					". Tarefa recebida. Tempo antes de processar ==> " << Simulator::Now().GetSeconds () << ". Processando..." << std::endl;

			for(uint32_t j=0; j<providers.size();j++){ //percorrer lista de providers
				if(curNode == providers[j]){ //se o servidor ainda consta na lista de providers -> não é falha
					Simulator::Schedule(Seconds(sleepTime),&serverSendAfter,
						socket, ipv4From, portFrom); //processa (num tempo sleepTime) e chama o sendAfter p enviar os resultados
				}
			}
		}
	}
}


// Aceita a conexão
void serverAccept(Ptr<Socket> socket,const ns3::Address& from)
{
	Ptr<Node> curNode = socket->GetNode(); //nó atual deste socket
	Ipv4Address ipv4To = getIPFromServer(curNode);

	std::cout <<"[SERVIDOR] "<< ipv4To << ": Conexão aceita"<< std::endl;
	os <<"[SERVIDOR] "<< ipv4To << ": Conexão aceita"<< std::endl;
	//auxProvidersPktToRcv[socket] = sizesOfPackets[idxOfServerAccept];
	//idxOfServerAccept++;
	socket->SetRecvCallback (MakeCallback (&serverHandler));
}

//Cria o socket servidor e coloca-o para escutar e depois responder
void serverSide() {
	if (OffloadSuccess >= numberOfSurrogates) {
		exit(0); //termina o programa
	}

	TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
	//for(; idxOfProvidersActionedInServer < providers.GetN(); idxOfProvidersActionedInServer++){ //cria um socket TCP para cada provider
	if(idxOfProvidersActionedInServer < numberOfSurrogates){ //cria um socket TCP para cada provider
		//server_side = Socket::CreateSocket(edgeNodes.Get(0),tid);

		//qual interface este provider tem? 5G ou WAVE? Lembrando que o provider só tem 1 interface
		//Ptr<Node> curNode = providers.Get(idxOfProvidersActionedInServer);

		Ptr<Node> curNode = providers[idxOfProvidersActionedInServer];

		std::cout << "[SERVIDOR] Servidor " << idxOfProvidersActionedInServer << " (" << getIPFromServer(curNode) << ") ativo!" << std::endl;
		os << "[SERVIDOR] Servidor " << idxOfProvidersActionedInServer << " (" << getIPFromServer(curNode) << ") ativo!" << std::endl;

		Ptr<NetDevice> curDevice = curNode->GetDevice(0);
		TypeId tid1 = curDevice->GetInstanceTypeId();
		std::string tidName = tid1.GetName(); //pega o nome da interface de rede
		if(tidName == "ns3::LoopbackNetDevice"){ //tenta pegar uma interface q n seja loopback
			curDevice = curNode->GetDevice(1);
			tid1 = curDevice->GetInstanceTypeId();
			tidName = tid1.GetName(); //pega o nome da interface de rede
		}

		//server_side = Socket::CreateSocket(providers.Get(idxOfProvidersActionedInServer),tid);
		server_side = Socket::CreateSocket(providers[idxOfProvidersActionedInServer],tid);
		if(tidName == "ns3::MmWaveUeNetDevice"){
			Ptr<NetDevice> edgeNetDevCell = edgeNetDev.Get(0); //interface 5G do cliente; só vai ter um edge server
			server_side->BindToNetDevice (edgeNetDevCell); //garante que este socket esteja atrelado à interface 5G do cliente
		}
		if(tidName == "ns3::WifiNetDevice"){
			for(uint32_t i=0; i<devices.GetN();i++){
				//encontrar o índice certo para pegar o NetDevice; lembrando que os carros servidores só têm WAVE
				//if(providers.Get(idxOfProvidersActionedInServer)->GetDevice(1) == devices.Get(i)){
				if(providers[idxOfProvidersActionedInServer]->GetDevice(1) == devices.Get(i)){
					Ptr<NetDevice> carNetDevWave = devices.Get(i);
					server_side->BindToNetDevice (carNetDevWave);
					break;
				}
			}
		}
		InetSocketAddress listen = InetSocketAddress (Ipv4Address::GetAny (), 55555);
		server_side->Bind(listen);
		server_side->Listen();
		//auxProvidersPktToRcv[server_side] = sizesOfPackets[index];
		server_side->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>,const Address &> (),MakeCallback(&serverAccept));
		idxOfProvidersActionedInServer++;
	}
}

// Função para receber solicitações dos clientes e responder
void serverListenForRequests(Ptr<Socket> socket) {
	//@TODO: O que prova que recebeu pacote?
	Address from;
	Ptr<Packet> packet= socket->RecvFrom(from);
	Ipv4Address ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();

	uint8_t *buff = new uint8_t[packet->GetSize()];
	packet->CopyData(buff,packet->GetSize());
	std::string data = std::string((char*)buff);

	Ptr<Node> curNode = socket->GetNode(); //nó atual deste socket
	Ipv4Address ipv4To = getIPFromServer(curNode);

	/*std::stringbuf sbuf;
	std::ostream sIp(&sbuf);
	ipv4To.Print(sIp);
	std::string stringIp = sbuf.str();
	if(stringIp == "10.1.0.23"){*/
	//std::cout << "[SERVIDOR] " << ipv4To << ". Recebeu a mensagem: <"<< data << "> de " << ipv4From << " em "<< Simulator::Now().GetSeconds () <<
	//		"s. Enviando a Resposta..." << std::endl;
	std::cout << "[SERVIDOR] Rcvd Request de: " << ipv4From << " em " << Simulator::Now().GetSeconds () <<
			"s. Server: " << ipv4To << ". Enviando Reply..." << std::endl;
	os << "[SERVIDOR] Rcvd Request de: " << ipv4From << " em " << Simulator::Now().GetSeconds () <<
			"s. Server: " << ipv4To << ". Enviando Reply..." << std::endl;

	std::string firstOctet = getDecimalFromOctet(ipv4To, 0);
	bool hasEnoughEnergy = true;
	uint32_t curId = curNode->GetId();
	if(firstOctet != "7"){ //análise do carServer pq edge sempre tem energia suficiente
		if((carEnergyLevel[curId] <= carMinimalEnergy[curId]) && (algorithm=="abc")){
			hasEnoughEnergy = false;  //BCV: não tem energia suficiente p participar do processo de offloading
		}
	}
	if(hasEnoughEnergy == true){ //só responde se o servidor tiver energia suficiente; o edge sempre vai ter; o carro nem sempre.
		InetSocketAddress dst = InetSocketAddress(ipv4From,80);
		socket->Connect(dst);											 //REPLY DIRETO
		socket->SendTo (Create<Packet> (60), 0, dst);
		socket->Close();
	}

	/*serverRcvRequestSocket[curNode] = socket;
	uint32_t curId = curNode->GetId();
	Ptr<MobilityModel> model1;                                    //REPLY PELA DISTÂNCIA APENAS COM VEÍCULOS
	std::string firstOctet = getDecimalFromOctet(ipv4To, 0);
	bool hasEnoughEnergy = true;
	if(firstOctet == "7"){
		//model1 = enbNodes.Get(0)->GetObject<MobilityModel>(); //pega o modelo de mobilidade do enb associado
		socket->Connect(dst);											 //REPLY DIRETO
		socket->SendTo (Create<Packet> (60), 0, dst);
		socket->Close();
	}
	else{
		model1 = c.Get(curId)->GetObject<MobilityModel>();
		if(carEnergyLevel[curId] <= carMinimalEnergy[curId]){
			hasEnoughEnergy = false;  //não tem energia suficiente p participar do processo de offloading
		}
		Ptr<MobilityModel> model2 = clients.Get(0)->GetObject<MobilityModel>();
		double distance = GetDistance(model1,model2);
		double replyTime = distance/1000.0;

		if(hasEnoughEnergy == true){ //só responde se o servidor tiver energia suficiente; o edge sempre vai ter; o carro nem sempre.
			Simulator::Schedule(Seconds(replyTime),&serverSendReply, socket, dst);
		}
	}*/

}

void serverSendReply(Ptr<Socket> replySocket, InetSocketAddress dst){
	replySocket->Connect(dst);
	replySocket->SendTo (Create<Packet> (60), 0, dst);
	replySocket->Close();
}

//recebe resultados dos offloadings e imprime no arquivo de resultados
void clientHandler(Ptr<Socket> socket) {
	//FILE *fp = fopen("/tmp/results", "a+");  //arquivo com resultados   "results/resultados.tr"
	//@FILE *fp = fopen("results/results.tr", "a+");  //arquivo com resultados

	Address from;
	Ptr<Packet> pack = socket->RecvFrom (from);
	Ipv4Address ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();
	//std::cout << "[CLIENTE] Pacote recebido " << std::endl;
	uint32_t packet = pack->GetSize();
	client_tcp_recv[socket] += packet;

	Ptr<Node> serverNode = getNodeFromIP(ipv4From); //pegar o nó servidor a partir do ip

	//@TODO: fazer um laço para percorrer cada socket e verificar se todos receberam os pacotes completos
	//@while (socket->Recv(1024,0)) { //cliente recebendo o resultado do offloading
	//@while (socket->Recv(packetSize,0)) { //@TODO: verificar se não é apenas ACK
	//@	std::cout << "[CLIENTE] Pacote recebido " << std::endl; //@TODO: ainda precisa garantir que não é retransmissão ou pacote duplicado
	//@}

	if (client_tcp_recv[socket]>=resultPacketSize) {
		//recebeu de um servidor que ainda está em providers?
		for(uint32_t j=0; j<providers.size();j++){ //percorrer lista de providers
			if(serverNode == providers[j]){ //se o servidor ainda consta na lista de providers -> não é falha
				clientRcvdResult[socket] = true; //atualizar map dizendo que o socket atual recebeu os dados
				OffloadSuccess++; //@TODO: garantir que é o pacote correto que foi recebido e que ele está com o tamanho certo

				std::cout << "[CLIENTE] Resultados recebidos de: " << ipv4From << " em " << Simulator::Now().GetSeconds ()
						<< "s. OffloadSuccess ==> " << OffloadSuccess << std::endl;
				os << "[CLIENTE] Resultados recebidos de: " << ipv4From << " em " << Simulator::Now().GetSeconds ()
								<< "s. OffloadSuccess ==> " << OffloadSuccess << std::endl;
				break;
			}
		}
	}

	checkOffloadingSuccess(false); //verifica se já encerraram os offloadings
}

//Cria socket para se comunicar com o servidor e envia o pacote
void clientSide() {
	TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");	
	//std::cout << "[SISTEMA] Número de providers ==> "<< providers.GetN() << std::endl;
	//os << "[SISTEMA] Número de providers ==> "<< providers.GetN() << std::endl;
	std::cout << "[SISTEMA] Número de providers ==> "<< providers.size() << std::endl;
	os << "[SISTEMA] Número de providers ==> "<< providers.size() << std::endl;
	if (OffloadSuccess >= numberOfSurrogates) {
		exit(0); //termina o programa
	}

	//for(; idxOfProvidersActionedInClient < providers.GetN(); idxOfProvidersActionedInClient++){ //cria um socket TCP para cada um dos providers
	if(idxOfProvidersActionedInClient < numberOfSurrogates){ //cria um socket TCP para cada um dos providers
		client_side = Socket::CreateSocket(clients.Get(0),tid);
		
		//qual interface este provider tem? 5G ou WAVE? Lembrando que o provider só tem 1 interface
		Ptr<Node> curNode = providers[idxOfProvidersActionedInClient];
		Ptr<NetDevice> curDevice = curNode->GetDevice(0); //o 0 é o loopback
		TypeId tid1 = curDevice->GetInstanceTypeId();
		std::string tidName = tid1.GetName(); //pega o nome da interface de rede
		if(tidName == "ns3::LoopbackNetDevice"){ //tenta pegar uma interface q n seja loopback
			curDevice = curNode->GetDevice(1);
			tid1 = curDevice->GetInstanceTypeId();
			tidName = tid1.GetName(); //pega o nome da interface de rede
		}

		//Ipv4Address serverIP = edgeIpIface.GetAddress(0);
		//Ipv4Address serverIP = providers.Get(index)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
		Ipv4Address serverIP;
		if(tidName == "ns3::MmWaveUeNetDevice"){
			//serverIP = providers.Get(idxOfProvidersActionedInClient)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(); //o provider só tem uma interface de rede
			serverIP = providers[idxOfProvidersActionedInClient]->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(); //o provider só tem uma interface de rede
			Ptr<NetDevice> clientNetDevCell = clientNetDevMmWave.Get(0); //interface 5G do cliente
			client_side->BindToNetDevice (clientNetDevCell); //garante que este socket esteja atrelado à interface 5G do cliente
		}
		if(tidName == "ns3::WifiNetDevice"){
			//serverIP = providers.Get(idxOfProvidersActionedInClient)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(); //o provider só tem uma interface de rede
			serverIP = providers[idxOfProvidersActionedInClient]->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(); //o provider só tem uma interface de rede
			Ptr<NetDevice> clientNetDevWave = devices.Get(clientId); //interface WAVE do cliente
			client_side->BindToNetDevice (clientNetDevWave); //garante que este socket esteja atrelado à interface WAVE do cliente
		}

		InetSocketAddress end = InetSocketAddress (serverIP, 55555);
		uint32_t status = client_side->Connect(end);

		std::cout << "[CLIENTE] Conectando-se a ==> "<< idxOfProvidersActionedInClient << "(" << serverIP << ")"
				<<	". Status da conexão ==> "<< status << ". Tempo antes de enviar => "<< Simulator::Now().GetSeconds()
				<<	". Enviando tarefas..." << std::endl;
		os << "[CLIENTE] Conectando-se a ==> "<< idxOfProvidersActionedInClient << "(" << serverIP << ")"
				<<	". Status da conexão ==> "<< status << ". Tempo antes de enviar => "<< Simulator::Now().GetSeconds()
				<<	". Enviando tarefas..." << std::endl;

		//IP do provider/tamanho do pacote q ele deve receber
		//auxProvidersPktToRcv[client_side] = sizesOfPackets[idxOfProvidersActionedInClient];
		//se houver borda ela sempre deverá estar no índice zero
		auxProvidersPktToRcv[client_side] = scheduleMatrix[idxOfProvidersActionedInClient].at(0); //pkt size
		auxProvidersTaskToProc[client_side] = scheduleMatrix[idxOfProvidersActionedInClient].at(1); //quanto processar?
		auxProvidersNumberOfTasksToProc[client_side] = scheduleMatrix[idxOfProvidersActionedInClient].at(2); //quantas tasks processar

		//momento esperado para receber o resultado
		if((algorithm=="gtt") || (algorithm=="gcf2")){
			//clientExpectedResultTime[client_side] = Simulator::Now().GetSeconds () + 3*expectedResultTime[serverIP]; //tempo atual + 3*esperado
			clientExpectedResultTime[client_side] = 30.0;
		} else {
			/*uint32_t curId = curNode->GetId();
			double comTime = 0.0; //tempo gasto com upload e download
			double queueProcTime = 0.0; //tempo gasto com processamento
			//saber se usa 5g ou wave
			std::string firstOctet = getDecimalFromOctet(serverIP, 0);
			if(firstOctet == "7"){ //é da rede dos edge servers, então usa o trange do mmWave
				comTime = (auxProvidersPktToRcv[client_side] + 1000)/(datarateMmWave*0.25) + 0.05; //1000 é a volta do resultado do processamento
				queueProcTime = calcSleepTime(edgeNodeCPUCap[idxFromMoreClosestEnb], edgeNodeCPUQueue[idxFromMoreClosestEnb],
						auxProvidersTaskToProc[client_side]);
			}
			else { // é carro server/WAVE
				comTime = (auxProvidersPktToRcv[client_side] + 1000)/(datarateWave*0.25) + 0.05; //1000 é a volta do resultado do processamento
				uint32_t positionInC = 0;
				for(uint32_t i=0; i<c.GetN();i++){
					if(curId == c.Get(i)->GetId()){
						positionInC = i;
						break;
					}
				}
				queueProcTime = calcSleepTime(carNodeCPUCap[positionInC], carNodeCPUQueue[positionInC], auxProvidersTaskToProc[client_side]);
			}
			double howLongTake = comTime + queueProcTime;
			double expectedTime = Simulator::Now().GetSeconds () + 3*howLongTake; //tempo atual + 3*esperado
			clientExpectedResultTime[client_side] = expectedTime; //tempo esperado para receber os resultados daquele socket */
			clientExpectedResultTime[client_side] = 30.0;
		}

		clientRcvdResult[client_side] = false; //obviamente ainda n recebeu o resultado do servidor correspondente
		ServerIP_ClientSocket[serverIP] = client_side; //associar IP do servidor ao socket do cliente
		ClientSocket_ServerIP[client_side] = serverIP; //associar IP do servidor ao socket do cliente
		StartFlow(client_side,serverIP, 55555,"upload");
		//@client_side->Send(Create<Packet> (packetSize),0); // Envia o pedaco da matriz
		//client_side->Bind(end);
		client_side->SetRecvCallback(MakeCallback(&clientHandler)); //espera para receber o resultado
		idxOfProvidersActionedInClient++;
	}
}

// Cliente faz solicitação
void clientRequest(Ptr<Socket> socket) {
	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	std::cout << "[CLIENTE] Enviando solicitação em "<< Simulator::Now().GetSeconds () << " segundos" << std::endl;
	os << "[CLIENTE] Enviando solicitação em "<< Simulator::Now().GetSeconds () << " segundos" << std::endl;
	//std::cerr << "[DEBUG] => "<< Simulator::Now().GetSeconds () << std::endl;
	std::stringstream msgx;
	msgx << "Quero fazer offloading!";
	Ptr<Packet> pkt = Create<Packet>((uint8_t*) msgx.str().c_str(), 256);
	socket->Send(pkt); //envia pacote pelo socket
	Simulator::Schedule(Seconds(2.0),&checkOffloadingSuccess, false); //se n tiver encontrado ngm em 2s, é para finalizar e executar local
}

//função para verificar periodicamente se as distâncias com os providers estão ok
//se não, aloca para executar local
void checkConnectivity(){
	double currentTime = Simulator::Now().GetSeconds ();
	Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>(); //modelo de mobilidade do cliente
	//verificar a distância entre o cliente e os providers
	//seria melhor ficar verificando o lifetime e qto tempo ainda falta p terminar a tarefa p antecipar o erro?
	for(uint32_t i = 0; i < providers.size(); i++){
		Ptr<MobilityModel> model2 = providers[i]->GetObject<MobilityModel>();
		double distance = GetDistance(model1,model2);
		Ipv4Address ipServer = getIPFromServer(providers[i]); //ip do servidor
		Ptr<Socket> clientSock = ServerIP_ClientSocket[ipServer]; //socket do cliente
		double curTrange = 260.0; //o transmission range default é 250.0 (coloquei margem de 260), mas pode n ser no caso do mmWave
		std::string firstOctet = getDecimalFromOctet(ipServer, 0);
		if(firstOctet == "7"){ //é da rede dos edge servers, então usa o trange do mmWave
			curTrange = 230.0; //o range é 220 metros, mas coloquei margem de 230 metros
		}
		//@TODO: pode ficar atualizando o tempo de vida e comparando quanto ainda falta para processar p se antecipar à falha
		//@TODO: usar as mensagens beacons p detectar falhas
		if(((distance > curTrange) || (currentTime > clientExpectedResultTime[clientSock]))
				&& (clientRcvdResult[clientSock]== false)){ //já perdeu a conectividade e ainda n recebeu os resultados
			//@TODO: verificar se n é melhor transferir para outro servidor remoto
			std::cout << "[SISTEMA] Falha em " << currentTime << "s. " <<  ipServer << " está distante "<<
					distance << " metros do cliente!" << std::endl;
			os << "[SISTEMA] Falha em " << currentTime << "s. " <<  ipServer << " está distante "<<
					distance << " metros do cliente!" << std::endl;

			numberOfSurrogates--; //diminui um surrogate já q vai ser executado localmente
			providers.erase(providers.begin() + i); //deletar o defeituoso do providers
			numberOfRecoveries++; //incrementa o número de recuperações de falhas
			double cpuReqOfLostPkt = 0.0; //cpu required da tarefa perdida
			cpuReqOfLostPkt = auxProvidersTaskToProc[clientSock];

			//double numberOfRecoveredTasks = cpuReqOfLostPkt/wAvgCpuReq;
			//int nrctasks = (int)numberOfRecoveredTasks;
			//tasksRecovered = tasksRecovered + nrctasks; //adiciona ao número de tarefas recuperadas
			//tasksOffloadedSuc = tasksOffloadedSuc - nrctasks; //diminui do número de tarefas offloadadas com sucesso
			tasksRecovered = tasksRecovered + auxProvidersNumberOfTasksToProc[clientSock]; //adiciona ao número de tarefas recuperadas
			tasksOffloadedSuc = tasksOffloadedSuc - auxProvidersNumberOfTasksToProc[clientSock]; //diminui da qtd de tarefas offloadadas com sucesso

			//executa aquela tarefa localmente
			if(localTime < 1.0){ //se nenhuma tarefa foi alocada aqui neste programa para o local do cliente
				    double sleepLocalTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], cpuReqOfLostPkt);
					localTime = (currentTime - tempoIni) + sleepLocalTime; //atualiza o localTime como se ele tivesse executado a tarefa perdida
			} else { //alguma tarefa foi alocada neste programa para o local do cliente
					if(currentTime > (localTime + tempoIni)){ //se já executou a tarefa alocada
						double sleepLocalTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], cpuReqOfLostPkt);
						localTime = currentTime + sleepLocalTime; //atualiza o localTime como se ele tivesse executado a tarefa perdida
					} else { //se já executou a tarefa alocada
						double sleepLocalTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], cpuReqOfLostPkt);
						localTime = localTime + sleepLocalTime; //atualiza o localTime como se ele tivesse executado a tarefa perdida
					}
			}
		}
	}
	checkOffloadingSuccess(false); //verifica se já encerraram os offloadings
	Simulator::Schedule(Seconds(1.0),checkConnectivity); //fica chamando a função de 1 em 1s
}

/*void testMobility(){
	Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
	Vector my_velocity = model1->GetVelocity();
	Vector my_position = model1->GetPosition();
	double mySpeed = GetSpeed(model1);
	std::cout << "[SISTEMA] Velocidade do cliente: " << mySpeed << ". Posição do cliente: [" << my_position.x << "," << my_position.y <<
				"]. Vetor velocidade: [" << my_velocity.x << "," << my_velocity.y << "]." << std::endl;
	Simulator::Schedule(Seconds(1.0),testMobility); //fica chamando a função de 1 em 1s
}
*/

// Recebe os pacotes de resposta e faz a escolha dos substitutos - todos com capacidade são favoráveis ao offloading
void clientRecRepPkt(Ptr<Socket> socket) {
	Address from;
	Ptr<Packet> packet= socket->RecvFrom(from);
	Ipv4Address ipv4From = InetSocketAddress::ConvertFrom(from).GetIpv4();
	std::cout << "[CLIENTE] Rcvd Reply em "<< Simulator::Now().GetSeconds () << "s. Server: " << ipv4From << ". ";
	os << "[CLIENTE] Rcvd Reply em "<< Simulator::Now().GetSeconds () << "s. Server: " << ipv4From << ". ";

	double currentTime = Simulator::Now().GetSeconds ();
	if(currentTime - tempoIni <= 0.5){
		nReplies += 1; //contador de replies em até 0.5s... recebeu mais um resposta
	}
	Ipv4Address ipv4To; //endereço do nó atual (na interface atual) que está recebendo
	uint32_t iface = 0; //interface do socket (mmwave ou wave)
	Ptr<Node> curNode = socket->GetNode(); //nó atual deste socket
	Ptr<NetDevice> curDevice = socket->GetBoundNetDevice(); //interface de rede deste socket (se wave ou 5g)
	TypeId tid1 = curDevice->GetInstanceTypeId();
	std::string tidName = tid1.GetName(); //pega o nome da interface de rede
	//Ipv4Header ipv4header;
	//packet->RemoveHeader(ipv4header);
	//Ipv4Address ipv4To = ipv4header.GetDestination();
	if(tidName == "ns3::MmWaveUeNetDevice"){
		iface = 1;
	}
	if(tidName == "ns3::WifiNetDevice"){
		iface = 2;
	}
	ipv4To = curNode->GetObject<Ipv4>()->GetAddress (iface, 0).GetLocal();
	//std::cout << "[CLIENTE] Recebeu resposta para ==> "<< ipv4To << std::endl;
	//os << "[CLIENTE] Recebeu resposta para ==> "<< ipv4To << std::endl;

	if(algorithm == "random2"){
		random2Decision(ipv4From, iface);
	}
	if(algorithm == "hvc"){
		hvcDecision(ipv4From, iface);
	}
	if(algorithm == "gcf2"){
		gcf2Decision(ipv4From, iface);
	}
	if(algorithm == "gtt"){
		gttDecision(ipv4From, iface);
	}
	if(algorithm == "abc"){
		abcDecision(ipv4From, iface);
	}
	if(algorithm == "mdo"){
		mdoDecision(ipv4From, iface);
	}
}

void initializeWave () {
	//Camadas de Enlace e Física WAVE
	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default (); //camada física
	YansWifiChannelHelper wifiChannel; //canal da camada física
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel"); //atraso de propagação do canal
	//wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
	//wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange",DoubleValue(600));
	wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel", //modelo de rádio propagação
			"Frequency", DoubleValue (5.9e9),
			"HeightAboveZ", DoubleValue (1.5));

	std::string phyMode ("OfdmRate27MbpsBW10MHz"); //padrão do WAVE é 6Mbps
	wifiPhy.Set ("TxGain", DoubleValue(2.0) );
	wifiPhy.Set ("RxGain", DoubleValue (2.0) );
	wifiPhy.Set ("TxPowerLevels", UintegerValue(1));
	wifiPhy.Set("EnergyDetectionThreshold",DoubleValue(-95.0));
	//trange = 251.0;
	//ConfigureTransmissionRangeTRG(trange); //configura a camada física para garantir o alcance
	wifiPhy.Set ("TxPowerStart", DoubleValue(txpower)); //16.7 p 250/251m; 22.35 p 500m; 28.55 p 750m
	wifiPhy.Set ("TxPowerEnd", DoubleValue(txpower)); //16.7 p 250/251m; 22.35 p 500m; 28.55 p 750m
	wifiPhy.SetChannel (wifiChannel.Create ()); //seta o canal configurado na camada física
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11); // suporte para gerar um trace pcap

	NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default (); //camada de enlace 802.11p com QoS
	Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default (); //helper da camada de enlace 802.11p com QoS
	wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",   //usa a mesma taxa de dados para cada pacote
			"DataMode",StringValue (phyMode), //configura o modo de dados
			"ControlMode",StringValue (phyMode)); //configura o modo de controle
	devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c); //instala a camada física e enlace nas placas de redes dos nós

	//internet.Install (c); //instala a pilha de internet nos carros -- não pode ser acionado sem ter placa de rede instalada
	Ipv4AddressHelper ipv4; // serviço de DHCP
	NS_LOG_INFO ("Atribuindo endereços IP.");
	ipv4.SetBase ("10.1.0.0", "255.255.0.0"); //Seta o número base da rede, máscara e endereço
	ips = ipv4.Assign (devices); //atribui os IPs nas placas de rede dos nós
	std::cout << "[SISTEMA] IP-WAVE do cliente ==> "<< ips.GetAddress(clientId) << std::endl;
	os << "[SISTEMA] IP-WAVE do cliente ==> "<< ips.GetAddress(clientId) << std::endl;

	//broadcast para descoberta em WAVE
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory"); //pega o tipo da API para criar instâncias de sockets UDP
	wReqBcSock = Socket::CreateSocket (c.Get (clientId), tid); //cria socket UDP do cliente para o broadcast
	Ptr<NetDevice> clientNetDevWave = devices.Get(clientId); //interface WAVE do cliente
	wReqBcSock->BindToNetDevice (clientNetDevWave); //garante que este socket esteja atrelado à interface WAVE do cliente
	InetSocketAddress port = InetSocketAddress (Ipv4Address::GetAny (), 80); //usado para receber as respostas das solicitações em broadcast
	InetSocketAddress broadcast = InetSocketAddress (Ipv4Address("255.255.255.255"), 80); //usado para enviar as solicitações em broadcast
	wReqBcSock->SetAllowBroadcast(true); //configura se transmissões de datagramas broadcast são permitidos
	wReqBcSock->Connect (broadcast); //Inicia uma conexão com o hospedeiro remoto (broadcast)
	wReqBcSock->Bind(port); //Aloca um endpoint para este socket para escutar respostas das solicitações em broadcast
	wReqBcSock->SetRecvCallback(MakeCallback(&clientRecRepPkt)); //Notifica quando novos dados estão disponíveis, recebe respostas

	//p servidor do carro receber solicitações em broadcast e responder
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80); //usado para escutar solicitações do cliente
	for (uint32_t x = 0; x < c.GetN(); x++) {
		if (x != clientId) { //que o substituto não seja o próprio cliente
			surrogates.Add(c.Get(x)); //adiciona todos os nós restantes no container de substitutos
			//@armazenaIndexSurrogates[x] = x; //armazena o índice dos substitutos
			Ptr<Socket> wSinkServer = Socket::CreateSocket (c.Get (x), tid); //cria socket UDP nos substitutos para escutar solicitações do cliente
			Ptr<NetDevice> carNetDevWave = devices.Get(x);
			wSinkServer->BindToNetDevice (carNetDevWave);
			wSinkServer->Bind(local); //aloca um endpoint para este socket para escutar solicitações do cliente
			wSinkServer->SetRecvCallback(MakeCallback(&serverListenForRequests)); //recebe solicitações e responde para o cliente
		}
	}
}

void initializeMmWave () {
	//configurações do mmwave
	Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024*1024)); //tentei tb=16777216
	Config::SetDefault ("ns3::MmWavePhyMacCommon::ResourceBlockNum", UintegerValue(1));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::ChunkPerRB", UintegerValue(72));
	bool rlcAmEnabled = true;
	Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue (1024*1024)); //tentei tb=16777216
	Config::SetDefault ("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue(rlcAmEnabled));
	Config::SetDefault ("ns3::LteRlcAm::PollRetransmitTimer", TimeValue(MilliSeconds(0.4)));
	Config::SetDefault ("ns3::LteRlcAm::StatusProhibitTimer", TimeValue(MilliSeconds(1.0)));
	Config::SetDefault ("ns3::MmWavePhyMacCommon::TbDecodeLatency", UintegerValue(2.0));
	Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (Seconds(0)));
	Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1apLinkDelay", TimeValue (Seconds(0)));
	Config::SetDefault ("ns3::MmWaveFlexTtiMacScheduler::CqiTimerThreshold", UintegerValue(100000));
	//Config::SetDefault ("ns3::MmWavePropagationLossModel::ChannelStates", StringValue ("n"));
	Config::SetDefault ("ns3::MmWavePropagationLossModel::FixedLossTst", BooleanValue (false));
	Config::SetDefault ("ns3::MmWavePropagationLossModel::LossFixedDb", DoubleValue (100.0));

	//outras configurações
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1500)); // seta o tamanho do segmento TCP
	Config::SetDefault ("ns3::TcpSocket::TcpNoDelay", BooleanValue (true));
	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (131072*50)); //tentei tb=16777216
	Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (131072*50)); //tentei tb=16777216

	//configurando periodicidade das mensagens 5G
	//Config::SetDefault ("ns3::MmWavePhyMacCommon::SubframePeriod", DoubleValue(10000000.0));
	//Config::SetDefault ("ns3::MmWaveBeamforming::LongTermUpdatePeriod", TimeValue (Seconds (2.0)));
	Config::SetDefault ("ns3::MmWaveEnbPhy::UpdateSinrEstimatePeriod", IntegerValue (25600));
	//Config::SetDefault ("ns3::MmWaveUeMac::UpdateUeSinrEstimatePeriod", DoubleValue (0));

	//cria os helpers do mmwave
	ptr_mmWave = CreateObject<MmWaveHelper> ();
	ptr_mmWave->SetSchedulerType ("ns3::MmWaveFlexTtiMacScheduler");
	epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
	ptr_mmWave->SetEpcHelper (epcHelper);
	ptr_mmWave->Initialize();

	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults();

	Ptr<Node> pgw = epcHelper->GetPgwNode ();
	MobilityHelper pgwmobility;
	Ptr<ListPositionAllocator> pgwPositionAlloc = CreateObject<ListPositionAllocator> ();
	pgwPositionAlloc->Add (Vector (1000.0, 1000.0, 3.0));
	pgwmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	pgwmobility.SetPositionAllocator(pgwPositionAlloc);
	pgwmobility.Install (pgw);

	Ptr<Node> mme = epcHelper->GetMmeNode();
	MobilityHelper mmemobility;
	Ptr<ListPositionAllocator> mmePositionAlloc = CreateObject<ListPositionAllocator> ();
	mmePositionAlloc->Add (Vector (1000.0 + 10.0, 1000.0 + 10.0, 3.0));
	mmemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mmemobility.SetPositionAllocator(mmePositionAlloc);
	mmemobility.Install (mme);

	//instala o mmwave nos nós
	edgeNetDev = ptr_mmWave->InstallUeDevice (edgeNodes);
	enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
	clientNetDevMmWave = ptr_mmWave->InstallUeDevice (clients);

	//instala a pilha da internet nos EUs que agem como servidores de borda
	internet.Install (edgeNodes);
	edgeIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (edgeNetDev));

	internet.Install (c); //instala a pilha de internet nos carros -- não pode ser acionado sem ter placa de rede instalada
	//atribuição de IP na interface 5G do cliente
	clientIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (clientNetDevMmWave));
	std::cout << "[SISTEMA] IP-mmWAVE do cliente ==> "<< clientIpIface.GetAddress(0) << std::endl;
	os << "[SISTEMA] IP-mmWAVE do cliente ==> "<< clientIpIface.GetAddress(0) << std::endl;

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	// Set the default gateway for the SUEs (edge nodes) and associate with your eNB
	for(uint32_t i=0; i<edgeNodes.GetN(); i++){
		Ptr<Node> edgeNode = edgeNodes.Get (i);
		Ptr<Ipv4StaticRouting> edgeNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (edgeNode->GetObject<Ipv4> ());
		edgeNodeStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

		uint32_t idEnb = enbNodes.Get(i)->GetId();
		uint32_t idEdgeNode = edgeNode->GetId();
		ipEdgeEnb[idEnb] = edgeIpIface.GetAddress (i);
		//ipEdgeEnb[idEnb] = edgeNode->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal();
		idEdgeEnb[idEnb] = idEdgeNode;
	}

	// Set the default gateway for the o carro cliente
	Ptr<Node> clientNode = clients.Get (0);
	Ptr<Ipv4StaticRouting> clientStaticRouting = ipv4RoutingHelper.GetStaticRouting (clientNode->GetObject<Ipv4> ());
	clientStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

	//liga os equipamentos 5G ao eNB mais próximo
	ptr_mmWave->AttachToClosestEnb (edgeNetDev, enbNetDev);
	ptr_mmWave->AttachToClosestEnb (clientNetDevMmWave, enbNetDev);
	ptr_mmWave->EnableTraces();

	//como pegar o enb associado ao carro cliente
	Ptr<MmWaveUeNetDevice> teste = clientNetDevMmWave.Get(0)->GetObject<MmWaveUeNetDevice>();
	Ptr<MmWaveEnbNetDevice> targetEnb = teste->GetTargetEnb();
	uint32_t idTargetEnb = targetEnb->GetNode()->GetId();
	std::cerr << "[SISTEMA] ID do eNB associado ==> " << idTargetEnb << " Tempo: ==> "<< Simulator::Now().GetSeconds() << std::endl;
	os << "[SISTEMA] ID do eNB associado ==> " << idTargetEnb << " Tempo: ==> "<< Simulator::Now().GetSeconds() << std::endl;

	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory"); //pega o tipo da API para criar instâncias de sockets UDP
	//hello em unicast para o eNB
	mmwReqUncSock = Socket::CreateSocket (c.Get(clientId), tid);
	InetSocketAddress port = InetSocketAddress (Ipv4Address::GetAny (), 80); //usado para receber as respostas das solicitações em broadcast
	Ptr<NetDevice> clientNetDevCell = clientNetDevMmWave.Get(0); //interface 5G do cliente
	mmwReqUncSock->BindToNetDevice (clientNetDevCell); //garante que este socket esteja atrelado à interface 5G do cliente
	InetSocketAddress remoteEdge = InetSocketAddress (ipEdgeEnb[idTargetEnb], 80);
	mmwReqUncSock->Connect (remoteEdge);
	mmwReqUncSock->Bind(port);
	mmwReqUncSock->SetRecvCallback (MakeCallback (&clientRecRepPkt));

	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80); //usado para escutar solicitações do cliente
	//p servidores da edge receberem solicitações em unicast e responder
	for (uint32_t y = 0; y < edgeNodes.GetN(); y++) {
		Ptr<Socket> mmwSinkEdge = Socket::CreateSocket (edgeNodes.Get (y), tid); //cria socket UDP nos edgeNodes para escutar solicitações do cliente
		Ptr<NetDevice> edgeNetDevCell = edgeNetDev.Get(y);
		mmwSinkEdge->BindToNetDevice (edgeNetDevCell);
		mmwSinkEdge->Bind(local); //aloca um endpoint para este socket para escutar solicitações do cliente
		mmwSinkEdge->SetRecvCallback(MakeCallback(&serverListenForRequests)); //recebe solicitações e responde para o cliente
	}
}

std::vector<double> clientPosition () {
	//std::ifstream infile(tracePath + traceFile, std::ios::in);
	std::ifstream infile(tracePath + "current.tcl", std::ios::in);  //lembrar que houve corte em buildTcl()
	if(!infile)
	{
	    std::cerr<<"File could not be opend"<<std::endl;
	}
	// I have eight different variables, representing each column, but some lines have less columns.
	std::string a; std::string b; std::string e; std::string h;
	std::string c; //terceira coluna: tempo
	std::string d; //quarta coluna: node
	double f; //sexta coluna: posição x
	double g; //sétima coluna: posição y

	//std::string sInitAction = std::to_string(initAction);
	//sInitAction = sInitAction + ".0";
	//std::string sInitAction = std::to_string(0.0);
	std::string sInitAction = "2.0"; //lembrar que depois do corte em buildTcl(), as simulações começam por 2s
	std::string sClientId = std::to_string(clientId);

	/*string str;
	while (getline(infile, str)) { // Read the entire line
	    stringstream ss(str);
	    ss>>a>>b>>c>>d>>e>>f>>g;
	    getline(ss, h);
	    ... // The rest of your code
	}*/

	std::vector<double> clientCoord;
	//while((infile>>a>>b>>c>>d>>e>>f>>g>>h) && getline(infile, h))
	//while(infile>>a>>b>>c>>d>>e>>f>>g>>h)
	std::string str;
	while (std::getline(infile, str)) // Read the entire line
	{
		std::stringstream ss(str);
		//ss>>a>>b>>c>>d>>e>>f>>g;
		//if(ss.rdstate() == std::ios::failbit){ //se falhou em pegar as 8 colunas
		//if(a.size() > 5){ //primeira coluna começa com $node_
		if(!(ss>>a>>b>>c>>d>>e>>f>>g)){  //se falhou em pegar as 8 colunas
			ss.clear();
			ss.seekg(std::ios::beg);
			ss>>a>>b>>c>>d;
			e="EMPTY";f=0.0;g=0.0;

			std::string cutA = a;
			std::string curnode = cutA.substr(cutA.find("(") + 1, (cutA.size()-7)); //o 7 é de $node_(
			curnode = curnode.substr (0, curnode.length() - 1);
			if(curnode == sClientId) {    //garante logo pegar a posição inicial
				if(c == "X_"){
					//converte d de string para double
					double xPoint = std::stod(d);
					clientCoord.push_back(xPoint);
				}
				if(c == "Y_"){
					//converte d de string para double
					double yPoint = std::stod(d);
					clientCoord.push_back(yPoint);
				}
			}
		}

		//if(a.size() < 5){ //primeira coluna começa com $ns_
		else {    //se conseguiu pegar as 8 colunas
			ss.clear();
			ss.seekg(std::ios::beg);
			ss>>a>>b>>c>>d>>e>>f>>g;

			std::string curnode = d.substr(d.find("(") + 1, (d.size()-8)); //o 8 é de "$node_(
			curnode = curnode.substr (0, curnode.length() - 1);
			if(c == sInitAction) {
				if(curnode == sClientId) {
					if(clientCoord.size()>0){
						clientCoord.clear(); //
					}
					clientCoord.push_back(f);
					clientCoord.push_back(g);
					break;
				}
			}
		}
	}
	infile.close();
	return clientCoord;
}

void createStaticMobility () {
	std::vector<double> clientCoord = clientPosition(); //pega a posição do cliente em initAction

	//verifica qual o enb mais próximo dessa posição
	idxFromMoreClosestEnb = 0;
	double shortestDistance = 999999999999.0;
	for(uint32_t i=0; i < enbXPositions.size(); i++){
		double dist = sqrt(pow((enbXPositions[i] - clientCoord[0]), 2.0) + pow((enbYPositions[i] - clientCoord[1]), 2.0));
		if(dist < shortestDistance){
			shortestDistance = dist;
			idxFromMoreClosestEnb = i;
		}
	}


	//cria mobilidade constante para os eNBs
	MobilityHelper enbmobility;
	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
	//for(uint32_t i=0;i<numberOfEdgeServers;i++){
	//	enbPositionAlloc->Add (Vector (enbXPositions[i], enbYPositions[i], 0.0));
	//}
	//enbPositionAlloc->Add (Vector (3384.51, 1965.47, 0.0)); //eNB0
	enbPositionAlloc->Add (Vector (enbXPositions[idxFromMoreClosestEnb], enbYPositions[idxFromMoreClosestEnb], 0.0)); //eNB0
	enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	enbmobility.SetPositionAllocator(enbPositionAlloc);
	enbmobility.Install (enbNodes);

	//cria mobilidade constante para os edge nodes
	MobilityHelper edgeMobility;
	Ptr<ListPositionAllocator> edgePositionAlloc = CreateObject<ListPositionAllocator> ();
	//for(uint32_t i=0;i<numberOfEdgeServers;i++){
	//	edgePositionAlloc->Add (Vector (enbXPositions[i] + 10.0, enbYPositions[i], 0.0));
	//}
	//edgePositionAlloc->Add (Vector (3384.51 + 10.0, 1965.47, 0.0)); //SUE0
	edgePositionAlloc->Add (Vector (enbXPositions[idxFromMoreClosestEnb]+10.0, enbYPositions[idxFromMoreClosestEnb], 0.0)); //eNB0
	edgeMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	edgeMobility.SetPositionAllocator(edgePositionAlloc);
	edgeMobility.Install (edgeNodes);

	//adiciona um ponto adicional p aumentar a bounding box do ns-3 e n gerar conflito com o mmwave
	if(scenario == "urban"){
		NodeContainer limitPoint;
		limitPoint.Create(1);
		MobilityHelper lpmobility;
		Ptr<ListPositionAllocator> lpPositionAlloc = CreateObject<ListPositionAllocator> ();
		lpPositionAlloc->Add (Vector (4000.0, 3000.0, 0.0));
		lpmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		lpmobility.SetPositionAllocator(lpPositionAlloc);
		lpmobility.Install (limitPoint);
	}
	if(scenario == "highway"){
		NodeContainer limitPoint;
		limitPoint.Create(1);
		MobilityHelper lpmobility;
		Ptr<ListPositionAllocator> lpPositionAlloc = CreateObject<ListPositionAllocator> ();
		lpPositionAlloc->Add (Vector (3000.0, 12000.0, 0.0));
		lpmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		lpmobility.SetPositionAllocator(lpPositionAlloc);
		lpmobility.Install (limitPoint);
	}
}

void getEnbAndEdgePoints () {
	//configura a quantidade de edges, torres 5G (eNBs) e suas posições
	if((cellcoverage == "100") && (scenario == "highway")){
		numberOfEdgeServers = 23;
		readLinesFromFile("inputs/enbs/highway-100-5g-coverage-xpoints", "enb-xpoints");
		readLinesFromFile("inputs/enbs/highway-100-5g-coverage-ypoints", "enb-ypoints");
	}
	if((cellcoverage == "50interleaved") && (scenario == "highway")){
		numberOfEdgeServers = 12;
		readLinesFromFile("inputs/enbs/highway-50-5g-coverage-interleaved-xpoints", "enb-xpoints");
		readLinesFromFile("inputs/enbs/highway-50-5g-coverage-interleaved-ypoints", "enb-ypoints");
	}
	if((cellcoverage == "50half") && (scenario == "highway")){
		numberOfEdgeServers = 11;
		readLinesFromFile("inputs/enbs/highway-50-5g-coverage-half-xpoints", "enb-xpoints");
		readLinesFromFile("inputs/enbs/highway-50-5g-coverage-half-ypoints", "enb-ypoints");
	}
	if((cellcoverage == "100") && (scenario == "urban")){
		numberOfEdgeServers = 16;
		readLinesFromFile("inputs/enbs/urban-100-5g-coverage-xpoints", "enb-xpoints");
		readLinesFromFile("inputs/enbs/urban-100-5g-coverage-ypoints", "enb-ypoints");
	}
	if((cellcoverage == "50interleaved") && (scenario == "urban")){
		numberOfEdgeServers = 8;
		readLinesFromFile("inputs/enbs/urban-50-5g-coverage-interleaved-xpoints", "enb-xpoints");
		readLinesFromFile("inputs/enbs/urban-50-5g-coverage-interleaved-ypoints", "enb-ypoints");
	}
	if((cellcoverage == "50half") && (scenario == "urban")){
		numberOfEdgeServers = 8;
		readLinesFromFile("inputs/enbs/urban-50-5g-coverage-half-xpoints", "enb-xpoints");
		readLinesFromFile("inputs/enbs/urban-50-5g-coverage-half-ypoints", "enb-ypoints");
	}
}

void configureTimeAndNumberOfNodes() {
	//quantidade de nós e tempo inicial e final da simulação depende do cenário
	numberOfNodes=0;
	startTime=0.0;
	finishTime=0.0;
	if(traceFile == "highway-low.tcl"){
		startTime = 10.0;
		finishTime = 160.0;
		numberOfNodes = 56;
	}
	if(traceFile == "highway-medium.tcl"){
		startTime = 50.0;
		finishTime = 200.0;
		numberOfNodes = 298;
	}
	if(traceFile == "highway-high.tcl"){
		startTime = 180.0;
		finishTime = 330.0;
		numberOfNodes = 604;
	}
	if(traceFile == "urban-low.tcl"){
		startTime = 10.0;
		finishTime = 160.0;
		//numberOfNodes = 50; //old
		numberOfNodes = 51; //old - vehcom
		//numberOfNodes = 51+10; //thesis - acrescentei 10 estacionados
	}
	if(traceFile == "urban-medium.tcl"){
		startTime = 10.0;
		finishTime = 160.0;
		numberOfNodes = 276; //old - vehcom
		//numberOfNodes = 276+30; //thesis - acrescentei 30 estacionados
	}
	if(traceFile == "urban-high.tcl"){
		startTime = 10.0;
		finishTime = 160.0;
		//numberOfNodes = 509; //old
		numberOfNodes = 553; //old - vehcom2020
		//numberOfNodes = 602+50; //thesis - acrescentei 50 estacionados
	}
}

// Função principal
int main (int argc, char *argv[])
{
	//std::cout << "C++ " << __cplusplus << std::endl; //descobrir a versão do C++

	CommandLine cmd; // análise de atributos de linha de comando
	cmd.AddValue ("run", "Run index (for setting repeatable seeds)", run);
	cmd.AddValue ("tracePath", "trace de mobilidade do ns-2", tracePath);
	cmd.AddValue ("scenario", "urban ou highway", scenario);
	cmd.AddValue ("density", "low, medium ou high", density);
	cmd.AddValue ("cellcoverage", "tipo de cobertura das torres 5G", cellcoverage);
	cmd.AddValue ("knownroutes", "rotas conhecidas", pknownRoutes);
	cmd.AddValue ("workload", "workload a ser executado", workload);
	//cmd.AddValue ("trange", "alcance de comunicação", trange);
	//cmd.AddValue ("txPower", "potência de transmissão", txpower);
	cmd.AddValue ("algorithm", "algoritmo", algorithm);
	//cmd.AddValue ("numberOfCycles", "número de ciclos do abc", numberOfCycles);
	//cmd.AddValue ("foodSources", "fontes de comida do abc", foodSources);
	cmd.Parse (argc,argv);

	//pknownRoutes="50";
	txpower = 16.7;
	//cellcoverage = "100"; //100% de cobertura 5G - configuração das posições das torres celulares (enbs)
	trange = 251.0; //alcance / cobertura / range padrão
	traceFile = scenario+"-"+density+".tcl";
	//logFile = "logs/" + scenario + "-" + density + "-w" + std::to_string(workload)
	//		+ "-" + algorithm + "-" + std::to_string(run) + ".log";
	logFile = "logs/" + scenario + "-" + density + "-" + cellcoverage + "-w" + std::to_string(workload)
			+ "-" + algorithm + "-" + std::to_string(run) + ".log";
	//logFile = "logs/" + scenario + "-" + density + "-kr" + pknownRoutes + "-w" + std::to_string(workload) + "-" +
	//		algorithm + "-" + std::to_string(run) + ".log";

	if (traceFile.empty () || logFile.empty ()) // verifica os argumentos de linha de comando
	{
		std::cout << "Uso do " << argv[0] << " :\n\n"
				"./waf --run \"olvanets  --traceFile=... --logFile=...\" \n\n";
		return 0;
	}

	RngSeedManager::SetSeed (1);
	RngSeedManager::SetRun (run);
	//srand(time(NULL)); //faz uso do relógio interno do computador para controlar a escolha da semente (seed)

	readLinesFromFile("inputs/workloads/workload" + std::to_string(workload) +".txt", "workload"); //pega as tarefas do workload
	backupWorkloadMatrix = workloadMatrix;
	numberOfTasks = workloadMatrix.size();
	for(uint32_t k=0; k<workloadMatrix.size(); k++){
		wSumCpuReq = wSumCpuReq + workloadMatrix[k].at(1);
	}
	wAvgCpuReq = wSumCpuReq/numberOfTasks; //média de cpu required ds tasks do workload

	//procDeadline = 10.0 + (workload-1.0)*2.0;
	//procDeadline = floor(((wSumCpuReq/1.5)+5.0)/2.0); //1.5 é a média de capacidades de cpu e 5.0 é a média das filas
	//procDeadline = wSumCpuReq/2.0; //deadline é a metade da soma dos gigaciclos (no caso, por segundo... ex.: 12gcps/2)
	//low:  ( (wSum/1.5)+5.0+(#tasks/numberOfWorkload) )/2.0; medium-highway: low - (numberOfWorkload/2.0)
	//high-highway: medium - (numberOfWorkload/2.0); //medium-urban: low; //high-urban: medium-highway

	configureTimeAndNumberOfNodes(); //configura tempo inicial e final e número de carros (em movimento)
	getEnbAndEdgePoints(); //pega as coordenadas dos edge servers e enbs para cada cenário
	getEnergyValues(); //pega os valores de energia

	//std::string delimiter = "-"; //pegando o tamanho e o tempo de uma imagem
	//std::string imgS = oneImage.substr(0, oneImage.find(delimiter));
	//imgSize = stoi(imgS);
	//std::string imgT = oneImage.substr(oneImage.find(delimiter)+1, oneImage.length());
	//imgTime = stod(imgT);
	//pktAndSleep(imgSize, imgTime); //calcula o sleepTime e o packetSize

	ns3::PacketMetadata::Enable (); //Registrar metadados, cabeçalhos etc dos pacotes; registra cada operação no buffer do pacote e analisa seu conteúdo
	double interval = 1.0; // segundos
	Time interPacketInterval = Seconds (interval);

	//numberOfNodes = 2;
	c.Create(numberOfNodes); // cria a quantidade de nós no container c. Veículos parados em 4 estacionamentos: 52-61, 276-305, 602-651

	os.open (logFile.c_str (), std::ofstream::out); //abre arquivo para colocar os logs

	//escolhe qual carro vai ser o cliente
	Ptr<UniformRandomVariable> xrv = CreateObject<UniformRandomVariable> ();
	clientId = xrv->GetInteger(0, numberOfNodes);
	while(carEnergyLevel[clientId] < carMinimalEnergy[clientId]){ //roda até achar um cliente que tenha energia suficiente
		clientId = xrv->GetInteger(0, numberOfNodes);
	}
	//clientId = 241;
	//clientId = 0;
	//clientId = rand() % numberOfNodes + 0; //escolhe aleatoriamente (dentre todos os nós) um nó para ser o cliente

	//initializeSleepTimes(); //matriz de cpu, packet size e tempo

	//pega o tempo inicial aleatoriamente segundo seed/run
	Ptr<UniformRandomVariable> yrv = CreateObject<UniformRandomVariable> ();
	initAction = yrv->GetInteger(startTime, startTime + 80);
	//initAction = 0;
	//int initAction2 = 0; //começa sempre em 2s, pq o arquivo de mobilidade já foi cortado
	//uint32_t initAction = rand() % 80 + startTime; //agenda a execução da ação entre 10 e 90s da simulação
	//uint32_t initAction = 10;

	buildTcl(); //corta o arquivo de mobilidade e atualiza-o para agilizar a simulação

	MobilityHelper mobility; // Habilitando a mobilidade do ns2 que utiliza os traces gerados pelo SUMO.
	//Ns2MobilityHelper ns2 = Ns2MobilityHelper (tracePath + traceFile); //pegando o arquivo de mobilidade e instanciando
	//Ns2MobilityHelper ns2 = Ns2MobilityHelper ("/home/alisson/ns-3.29/mobilityTraces/teste2.tcl");
	Ns2MobilityHelper ns2 = Ns2MobilityHelper (tracePath + "current.tcl"); //pegando o arquivo de mobilidade e instanciando
	ns2.Install (); // instala a mobilidade por traces

	clients.Add(c.Get(clientId)); //adiciona o cliente escolhido no containter de clientes em movimento

	readLinesFromFile("inputs/edgeCpu.txt", "edgeCPU"); //configura a capacidade de CPU dos edge nodes, pegando números aleatórios de arquivo
	readLinesFromFile("inputs/edgeQueue.txt", "edgeQueue"); //configura o tempo na fila nos edge nodes, pegando números aleatórios de arquivo
	readLinesFromFile("inputs/carCpu.txt", "carCPU"); //configura a capacidade de CPU dos carros, pegando números aleatórios de arquivo
	readLinesFromFile("inputs/carQueue.txt", "carQueue"); //configura o tempo na fila nos carros, pegando números aleatórios de arquivo
	if(scenario=="urban"){
		//rotas conhecidas no low: do 52-61, no medium: 276-305, no high: 602-651 //carros estacionados
		readLinesFromFile("inputs/knownRoutes/carKnownRoutes-"+pknownRoutes+"-urban-"+density+".txt", "carKnownRoutes"); //configura a % de rotas conhecidas
	}
	else{
		readLinesFromFile("inputs/knownRoutes/carKnownRoutes-"+pknownRoutes+".txt", "carKnownRoutes"); //configura a % de rotas conhecidas
	}

	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	std::cout << "[SISTEMA] ID do cliente ==> "<< clientId << ". Com CpuCap ==> "<< carNodeCPUCap[clientId] <<
			" e CpuQueue ==> " <<	carNodeCPUQueue[clientId] << std::endl;
	os << "[SISTEMA] ID do cliente ==> "<< clientId << ". Com CpuCap ==> "<< carNodeCPUCap[clientId] <<
			" e CpuQueue ==> " <<	carNodeCPUQueue[clientId] << std::endl;
	onlyLocalTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], wSumCpuReq) - tempoIni; //tempo unicamente local
	std::cout << "[SISTEMA] onlyLocalTime ==> "<< onlyLocalTime << "." << std::endl;
	os << "[SISTEMA] onlyLocalTime ==> "<< onlyLocalTime << "." << std::endl;

	numberOfCycles=50;
	foodSources=50;
	if(carNodeCPUCap[clientId] == 0.5){
		numberOfCycles=20;
	}
	variation=0.0; //inicializa a variação em relação ao onlyLocalTime

	//cria os enbs e os computadores de borda
	//enbNodes.Create (numberOfEdgeServers);
	enbNodes.Create (1);
	//edgeNodes.Create (numberOfEdgeServers);
	edgeNodes.Create (1);

	createStaticMobility(); //cria a mobilidade estática dos enbs, edge servers e ponto limite
	//createStaticMobilityPV(); //cria a mobilidade estática dos veículos estacionados

	initializeMmWave(); //inicializa as funções do mmwave
	initializeWave(); //inicializa as funções do wave

	double initAction2 = 2.0; //esse vai ser sempre o tempo inicial, pq o initAction já vai ter cortado o mobility trace
	//double initAction2 = 0.1; //esse vai ser sempre o tempo inicial, pq o initAction já vai ter cortado o mobility trace
	//configura finishTime p n demorar demais na simulação
	double finishTime2 = initAction2 + 70.0;
	finishTime = std::min (finishTime - initAction, finishTime2);

	//configura a aplicação de mensagens beacons (BSM - Basic Safety Messages)
	WaveBsmHelper waveBsm;
	m_txSafetyRanges.push_back(251.0);
	WaveBsmHelper::GetNodesMoving().resize(numberOfNodes,1); //inicialmente supõe que todos os nós estão se movendo
	waveBsm.Install(ips, Seconds (finishTime), 200, Seconds(3), 40,	m_txSafetyRanges,
			0, MilliSeconds(10)); //coloca os beacons para começar

	std::cout << "[SISTEMA] Número de substitutos ==> "<< surrogates.GetN() << std::endl;
	os << "[SISTEMA] Número de substitutos ==> "<< surrogates.GetN() << std::endl;

	//AnimationInterface anim ("vanets-animation.xml"); //animação do netanim gerada para análise
	//anim.SetMaxPktsPerTraceFile(5000000000);

	//AsciiTraceHelper ascii; //usado para tracefile
	//wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("results/resultados.tr")); //habilita tracefile
	//wifiPhy.EnablePcap ("client-80211p", devices); //habilita a geração de pcap por placa de rede
	//wifiPhy.EnablePcapAll("vanets_cap"); //habilita a geração de pcap para tudo

	std::cout << "[SISTEMA] Execução inicia em ==> "<< initAction << " segundos" << std::endl;
	os << "[SISTEMA] Execução inicia em ==> "<< initAction << " segundos" << std::endl;


	std::cout << "[SISTEMA] Depois do corte, inicia em ==> "<< initAction2 << " segundos" << std::endl;
	os << "[SISTEMA] Depois do corte, inicia em ==> "<< initAction2 << " segundos" << std::endl;
	Simulator::Schedule(Seconds(initAction2),&GetTempoIni); //pega o tempo inicial

	//adiciona o cliente/local às listas de auxProviders
	auxProvidersQueue[c.Get(clientId)] = carNodeCPUQueue[clientId];
	auxProvidersCap[c.Get(clientId)] = carNodeCPUCap[clientId];
	auxProvidersDist[c.Get(clientId)] = 0.0; //distância é zero.... é ele mesmo
	auxProvidersCpuConsumption[c.Get(clientId)] = carCpuConsumption[clientId]; //parâmetros de energia
	auxProvidersMinimalEnergy[c.Get(clientId)] = carMinimalEnergy[clientId];
	auxProvidersEnergyLevel[c.Get(clientId)] = carEnergyLevel[clientId];

	if(algorithm == "random2"){ //o random2 usa 5G e WAVE
		tasksToLocal(clientId); //o que deve ser executado localmente e quantos surrogates
		Simulator::Schedule(Seconds(initAction2),clientRequest,mmwReqUncSock); //cliente envia pacote em unicast para enb
		Simulator::Schedule(Seconds(initAction2),clientRequest,wReqBcSock); //cliente envia pacote em broadcast para descoberta
		//testMobility();
	}
	if(algorithm == "hvc"){
		Simulator::Schedule(Seconds(initAction2),clientRequest,mmwReqUncSock); //cliente envia pacote em unicast para enb
		Simulator::Schedule(Seconds(initAction2 + 0.1), hvcDecisionNoMmWave); //se passar 0.05s e n receber resposta do 5G...
	}
	if(algorithm == "gcf2"){
		//adiciona o cliente/local às listas de auxProviders
		//auxProvidersQueue[c.Get(clientId)] = carNodeCPUQueue[clientId];
		//auxProvidersDist[c.Get(clientId)] = 0.0; //distância é zero.... é ele mesmo

		Simulator::Schedule(Seconds(initAction2),clientRequest,wReqBcSock); //cliente envia pacote em broadcast para descoberta
		Simulator::Schedule(Seconds(initAction2),clientRequest,mmwReqUncSock); //cliente envia pacote em unicast para enb
		//testeGcf2();
	}
	if(algorithm == "gtt"){ //greedy task by task
		//adiciona o cliente/local às listas de auxProviders
		//auxProvidersQueue[c.Get(clientId)] = carNodeCPUQueue[clientId];
		//auxProvidersCap[c.Get(clientId)] = carNodeCPUCap[clientId];
		//auxProvidersDist[c.Get(clientId)] = 0.0; //distância é zero.... é ele mesmo

		Simulator::Schedule(Seconds(initAction2),clientRequest,wReqBcSock); //cliente envia pacote em broadcast para descoberta
		Simulator::Schedule(Seconds(initAction2),clientRequest,mmwReqUncSock); //cliente envia pacote em unicast para enb
	}
	if(algorithm == "abc"){ //artifical bee colony
		//adiciona o cliente/local às listas de auxProviders
		//auxProvidersQueue[c.Get(clientId)] = carNodeCPUQueue[clientId];
		//auxProvidersCap[c.Get(clientId)] = carNodeCPUCap[clientId];
		//auxProvidersDist[c.Get(clientId)] = 0.0; //distância é zero.... é ele mesmo
		//auxProvidersCpuConsumption[c.Get(clientId)] = carCpuConsumption[clientId]; //parâmetros de energia
		//auxProvidersMinimalEnergy[c.Get(clientId)] = carMinimalEnergy[clientId];
		//auxProvidersEnergyLevel[c.Get(clientId)] = carEnergyLevel[clientId];

		Simulator::Schedule(Seconds(initAction2),clientRequest,wReqBcSock); //cliente envia pacote em broadcast para descoberta
		Simulator::Schedule(Seconds(initAction2),clientRequest,mmwReqUncSock); //cliente envia pacote em unicast para enb
	}
	if(algorithm == "mdo"){ //multi-decision offloading do paper do context-aware, só usa V2V
		Simulator::Schedule(Seconds(initAction2),clientRequest,wReqBcSock); //cliente envia pacote em broadcast para descoberta
	}

	Simulator::Schedule(Seconds(finishTime-0.1),&checkOffloadingSuccess, true);
	Simulator::Stop (Seconds (finishTime)); //simulação para em 170s
	Simulator::Run (); //executa a simulação
	Simulator::Destroy (); //destroi a simulação
	os.close (); //fecha o arquivo de log
	return 0; //termina o programa
}
