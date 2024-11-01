/*
*  Alisson Barbosa, Outubro/2020. E-mail: alisson@ufc.br
*  This code realizes the packets exchange between vehicles (V2V) and the edge (V2I), simulating
*  the send of subtasks to be executed in the surrogate vehicles and in the edge of the network.
*
*  This code is protected by copyright and intellectual property right laws.
*  Do not use or distribute without the author's authorization.
*  This is true even for modified versions or snippets of this code.
*
*/

#include "scratch/olvanets/gtt.h"
//#include <stdlib.h>

std::map< Ptr<Node> , double> tempProvidersGtt;
double howMuchProcLocalGtt;
std::map <Ptr<Node> ,double> auxProvidersDistGttOriginal; //map auxiliar p guardar a distância original

//void testeGcf2(){
//	std::cout << "initAction: " << initAction << std::endl;
//}

//a ideia é passar o tempo e o nó e a função retornar as posições x e y num vetor
std::vector<double> estimatedPositionGtt(uint32_t estTime, uint32_t analyzedNode){
	FILE *arq;
	char a[100];char b[100];char c[100];char d[100];char e[100];char f[100];char g[100];char h[100];
	//float c;
	//tempo: converter uint32_t em char
	std::string sEstTime = std::to_string(estTime) + ".0";
	char cEstTime[100];
	strcpy(cEstTime, sEstTime.c_str());
	//sprintf(cEstTime, "%lu", (unsigned long) estTime);

	//node: converter uint32_t em string
	std::stringstream tempStr;
	std::string sAnalyzedNode;
	tempStr << analyzedNode;
	tempStr >> sAnalyzedNode;
	//vetor de retorno
	std::vector<double> rEstPos;

	std::string fnameComplete = tracePath + "current.tcl";
	arq = fopen(fnameComplete.c_str(), "rt");
	if (arq == NULL){	printf("Problemas na abertura do arquivo\n");	}

	while (!feof(arq)) {
		if(fscanf(arq, "%s", a)){
			if(strcmp(a, "$ns_") == 0){
				if(fscanf(arq, "%s %s %s", b, c, d)){
					if ((strcmp (c, cEstTime) == 0) && (strcmp (a, "$ns_") == 0)){ //comparei o tempo e $ns
						//pegando o node
						std::string cutD = d;
						std::string curnode = cutD.substr(cutD.find("(") + 1, (cutD.size()-8)); //o 7 é de "$node_(
						curnode = curnode.substr (0, curnode.length() - 1);
						if(curnode == sAnalyzedNode){ //comparando o nó
							if(fscanf(arq, "%s %s %s %s",e,f,g,h)){
								//retornar as posições x e y
								double cutF = std::stod(f);
								double cutG = std::stod(g);
								rEstPos.push_back(cutF);
								rEstPos.push_back(cutG);
							}
						}
					}
				}
			}

		}
	}
	fclose(arq);
	return rEstPos;
}

//a ideia é passar o tempo e o nó e a função retornar as posições x e y num vetor
std::vector<double> lastKnownPositionGtt(uint32_t estTime, uint32_t analyzedNode){
	FILE *arq;
	char a[100];char b[100];char c[100];char d[100];char e[100];char f[100];char g[100];char h[100];
	//float c;
	//tempo: converter uint32_t em char
	std::string sEstTime = std::to_string(estTime) + ".0";
	char cEstTime[100];
	strcpy(cEstTime, sEstTime.c_str());
	//sprintf(cEstTime, "%lu", (unsigned long) estTime);

	//node: converter uint32_t em string
	std::stringstream tempStr;
	std::string sAnalyzedNode;
	tempStr << analyzedNode;
	tempStr >> sAnalyzedNode;
	//vetor de retorno
	std::vector<double> rEstPos;

	std::string fnameComplete = tracePath + "current.tcl";
	arq = fopen(fnameComplete.c_str(), "rt");
	if (arq == NULL){	printf("Problemas na abertura do arquivo\n");	}

	while (!feof(arq)) {
		if(fscanf(arq, "%s", a)){
			if(strcmp(a, "$ns_") == 0){
				if(fscanf(arq, "%s %s %s", b, c, d)){
					std::string cutC = c;
					uint32_t ic = std::stoi(cutC);
					if ((ic <= estTime) && (strcmp (a, "$ns_") == 0)){ //comparei o $ns
						//pegando o node
						std::string cutD = d;
						std::string curnode = cutD.substr(cutD.find("(") + 1, (cutD.size()-8)); //o 7 é de "$node_(
						curnode = curnode.substr (0, curnode.length() - 1);
						if(curnode == sAnalyzedNode){ //comparando o nó
							if(fscanf(arq, "%s %s %s %s",e,f,g,h)){
								rEstPos.clear(); //zerar o vetor primeiro
								//retornar as posições x e y
								double cutF = std::stod(f);
								double cutG = std::stod(g);
								rEstPos.push_back(cutF);
								rEstPos.push_back(cutG);
							}
						}
					}
				}
			}
			else if(strstr(a,"$node")){ //armazenar as posições iniciais
				if(fscanf(arq, "%s %s %s", b, d, e)){
					//pegando o node
					std::string cutA = a;
					std::string curnode = cutA.substr(cutA.find("(") + 1, (cutA.size()-7)); //o 7 é de $node_(
					curnode = curnode.substr (0, curnode.length() - 1);
					if(curnode == sAnalyzedNode){ //comparando o nó
						//convertendo char em string
						std::string sd = d;
						std::string se = e;
						rEstPos.clear(); //zerar o vetor primeiro
						if(sd == "X_"){
							double de = std::stod(se);
							rEstPos.push_back(de);
						}
						if(sd == "Y_"){
							double de = std::stod(se);
							rEstPos.push_back(de);
						}
					}
				}
			}

		}
	}
	fclose(arq);
	return rEstPos;
}

//a ideia é verificar se os dois nós ainda estarão dentro do alcance um do outro no tempo futuro (tempo q durará o offloading)
bool withinRangeGtt(uint32_t estTime, uint32_t analyzedNode1, uint32_t analyzedNode2, double range){
	estTime = estTime - 3; //no traces de mobilidade, para saber a posição no tempo x, temos q olhar o setdest setado na posição x-3
	estTime = estTime + 2; //tb tenho somar 2 pq o current.tcl é construído desde 2s antes
	std::vector<double> rEstPos1 = estimatedPositionGtt(estTime, analyzedNode1); //posição do node1 no tempo estTime
	if(rEstPos1.empty()){ //se o vetor estiver vazio
		rEstPos1 = lastKnownPositionGtt(estTime, analyzedNode1); //pegar a última posição conhecida do nó
	}
	std::vector<double> rEstPos2; //posição do node2 no tempo estTime
	if(range==220.0){ //se é edge server, pegar a posição fixa do enb
		rEstPos2.push_back(enbXPositions[idxFromMoreClosestEnb]);
		rEstPos2.push_back(enbYPositions[idxFromMoreClosestEnb]);
	} else { //se é carro server
		rEstPos2 = estimatedPositionGtt(estTime, analyzedNode2);
	}
	if(rEstPos2.empty()){ //se o vetor estiver vazio
		rEstPos2 = lastKnownPositionGtt(estTime, analyzedNode2); //pegar a última posição conhecida do nó
	}
	double dist = sqrt(pow((rEstPos1[0] - rEstPos2[0]), 2.0) +	pow((rEstPos1[1] - rEstPos2[1]), 2.0)); //distância entre os nodes
	//Ptr<UniformRandomVariable> drv = CreateObject<UniformRandomVariable> (); //dar uma aleatoriezada, segundo a seed/run
	//int randomDist = drv->GetInteger(0, 5); //variação entre 0 a 5 metros
	//dist = dist + 1.0*randomDist; //distância + variação aleatoriezada
	bool wrange=false;
	if(dist <= range){ //verificar se está dentro do alcance
		wrange = true;
	}
	return wrange;
}

void sortByQueueCapDistGtt(){

	//preciso fazer uma soma do valores (já normalizados) dos nós nos três maps e jogar no map temporário

	for(std::map< Ptr<Node> , double>::iterator it = auxProvidersQueue.begin(); //percorre o map da fila
		it != auxProvidersQueue.end(); ++it){
		for(std::map< Ptr<Node> , double>::iterator jt = auxProvidersDist.begin(); //percorre o map da distância
			jt != auxProvidersDist.end(); ++jt){
			for(std::map< Ptr<Node> , double>::iterator kt = auxProvidersCap.begin(); //percorre o map da capacidade de cpu
				kt != auxProvidersCap.end(); ++kt){
				if((it->first == jt->first) && (it->first == kt->first)){ //se são os mesmos nós
					//preciso da média da cpu required do workloads
					tempProvidersGtt[it->first] = (it->second)*wAvgCpuReq*2.0 + ((jt->second)/trange)*10
							+ (wAvgCpuReq/(kt->second))*wAvgCpuReq; //soma equalizada
				}
			}
		}
	}

	// copia os pares chave-valor do map com a soma queue+cap+distance (normalizada/equalizada)
	std::copy(tempProvidersGtt.begin(),
			tempProvidersGtt.end(),
			std::back_inserter<std::vector<pair>>(sortAuxProviders));
	// ordene o vetor em ordem crescente pelo segundo valor do pair
	// se o segundo valor for igual, ordene pelo primeiro valor do par
	std::sort(sortAuxProviders.begin(), sortAuxProviders.end(),
			[](const pair& l, const pair& r) {
		if (l.second != r.second)
			return l.second < r.second;
		return l.first < r.first;
	});

	// imprime a lista
	//std::cout << "Lista de ordenada de sortAuxProviders" << std::endl;
	//os << "Lista de ordenada de sortAuxProviders" << std::endl;
	//for (uint i=0; i<sortAuxProviders.size();i++) {
	//	std::cout << '{' << getIPFromServer(sortAuxProviders[i].first) << "," << sortAuxProviders[i].second << '}' << std::endl;
	//	os << '{' << getIPFromServer(sortAuxProviders[i].first) << "," << sortAuxProviders[i].second << '}' << std::endl;
	//}
}

void calculateLltAndWithinRangeGtt(){

	double currentTime = Simulator::Now().GetSeconds ();
	uint32_t futureTime = ceil(currentTime); //tempo futuro inicial, arrendondar para cima... mobility file tem posições em tempos do tipo int
	uint32_t limitTime = floor(finishTime); //tempo final p verificar se está dentro do alcance

	//calcular tempo de vida e withinRange para cada um dos availableServers
	for(std::map< Ptr<Node> , double>::iterator it = auxProvidersQueue.begin(); //percorrer servers that replied (auxProvidersQueue ou cap ou dist)
	  it != auxProvidersQueue.end(); ++it){
		if(it->first == c.Get(clientId)){ //se é o cliente
			auxProvidersLlt[it->first] = 100; //tempo de vida infinito (é ele mesmo)
		}
		else{
			if(auxProvidersCap[it->first] >= 1.5){ //se é o edgeNode
				auxProvidersKr[it->first] = true; //edge node ta parado, então tem rota conhecidas
				//calcular tempo de vida com withinRange
				uint32_t curId = it->first->GetId(); //pegar ID do servidor
				bool withinrange = true; //no começo estão dentro do alcance um do outro
				while(withinrange == true){ //ou chegar no fim do arquivo
					withinrange = withinRangeAbc(futureTime,clientId,curId,220.0); //os dois nós ainda estarão dentro do alcance um do outro?
					futureTime++;
					if(futureTime >= limitTime){
						withinrange = false; //p sair do loops
					}
				}
				futureTime--; //último tempo em que os dois nós estavam dentro do alcance um do outro
				auxProvidersLlt[it->first] = futureTime - currentTime; //precisa converter futureTime p double antes?
			}
			else{ //se é carServer
				uint32_t curId = it->first->GetId(); //pegar ID do carro servidor
				uint32_t positionInC = 0; //verificar se a rota do carServer é conhecida
				for(uint32_t j=0; j<c.GetN();j++){
					if(curId == c.Get(j)->GetId()){
						positionInC = j;
						break;
					}
				}
				auxProvidersKr[it->first] = carNodeKnownRoutes[positionInC]; //1 é true e 0 é false

				if(auxProvidersKr[it->first] == false){ //se a rota não é conhecida, calcular let do modo tradicional
					Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
					Ptr<MobilityModel> model2 = it->first->GetObject<MobilityModel>();
					//double distance = GetDistance(model1,model2);
					double let = linkEstimatedLifeTime(model1,model2,240.0); // Tempo de vida do enlace; 'let', botei range menor
					auxProvidersLlt[it->first] = let;
				}
				else{ //se a rota é conhecida, calcular let com withinRange
					bool withinrange = true; //no começo estão dentro do alcance um do outro
					while(withinrange == true){ //ou chegar no fim do arquivo
						withinrange = withinRangeAbc(futureTime,clientId,curId,240.0); //os dois nós ainda estarão dentro do alcance um do outro?
						futureTime++;
						if(futureTime >= limitTime){
							withinrange = false; //p sair do loops
						}
					}
					futureTime--; //último tempo em que os dois nós estavam dentro do alcance um do outro
					auxProvidersLlt[it->first] = futureTime - currentTime; //precisa converter futureTime p double antes?
				}

			}
		}

	}
}

void printNumberOfReplies(){
	std::string filename = "results/" + scenario + "-" + density + "-w" + std::to_string(workload) + "-" + std::to_string(txpower)
		+ "-" + algorithm + ".tr";
	const char* fname = filename.c_str();
	fp = fopen(fname, "a+");  //arquivo com resultados
	fprintf(fp,"%i;%lu\n", nReplies, (unsigned long) run);
	fclose(fp);
	exit(0);
}

void SortAndInitTransferGtt() {

	//printNumberOfReplies();
	numberOfSurrogates = 0; //zera o número de surrogates por enqto
	double currentTime = Simulator::Now().GetSeconds ();
	//inicialmente o cliente não processa algo localmente
	localTime = 0 //o cliente não processa local
			+ (currentTime - tempoIni);  //tempo até chegar aqui desde o initAction2
	double howMuchProcLocal = 0.0; //quanto o client deve processar
	std::map< Ptr<Node> , double> howMuchProcServer; //node / quanto o nó deve processar
	std::map< Ptr<Node> , double> howMuchDataForServer; //node / qual o tamanho do pacote q deve receber
	std::map< Ptr<Node> , double> howManyTasksForServer; //node / quantas tasks o server recebe (p contar eventuais tarefas recuperadas)
	uint32_t i = 0; //indexador do loop das tarefas
	uint32_t j = 0; //indexador do loop dos servidores
	uint32_t avoidInfiniteLoop = 0; //evitar loop infinito
	Ptr<Node> curNode;
	double procTime;
	double energyPerSecond; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
	double expectedCsmdEnergy; //calcular consumo de energia esperado
	bool alreadyHad;
	double comTime; //tempo gasto com upload e download
	double queueProcTime;
	double howLongTake;
	bool isOkToOffload;
	std::vector<double> tempVec;

	//calculateLltAndWithinRangeGtt(); //calcular previamente os tempos de vida (se for possível com withinRange)

	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;

	startElapsedTime = std::chrono::high_resolution_clock::now();

	sortByQueueCapDistGtt(); //ordena os possíveis servidores

	//while((workloadMatrix.size()> 0) && (i<workloadMatrix.size())){ //percorrer as tarefas
	while((workloadMatrix.size()> 0)){ //percorrer as tarefas
		if(avoidInfiniteLoop>=50){
			break;
		}
		//jogar tarefa para o primeiro servidor da lista
		if(sortAuxProviders.size() > 0){
			curNode = sortAuxProviders[j].first; //nó atual... sempre pega o primeiro da lista

			if(curNode == c.Get(clientId)){ //verifica se é o próprio cliente
				//se n der certo, tirar
				howMuchProcLocal = howMuchProcLocal + workloadMatrix[i].at(1); //quanto o client deve processar... jogar a tarefa para o cliente

				//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
				procTime = calcSleepTime(auxProvidersCap[curNode], auxProvidersQueue[curNode], howMuchProcLocal);
				energyPerSecond = auxProvidersCpuConsumption[curNode]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
				expectedCsmdEnergy = procTime*energyPerSecond; //calcular consumo de energia esperado

				if((auxProvidersEnergyLevel[curNode] - expectedCsmdEnergy) > auxProvidersMinimalEnergy[curNode]){
					//atualizar nova carga; somar o processamento à fila do cliente
					auxProvidersQueue[curNode] = auxProvidersQueue[curNode] + workloadMatrix[i].at(1)/auxProvidersCap[curNode];
					tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
					workloadMatrix.erase(workloadMatrix.begin() + i); //deletar tarefas q já estão sendo processadas localmente
					sortAuxProviders.clear();
					sortByQueueCapDistGtt(); //reordenar a lista
				}
				else{
					howMuchProcLocal = howMuchProcLocal - workloadMatrix[i].at(1); //se n der certo, retira o q ia ser processado
				}
			}
			else { //não é o próprio cliente... é um servidor remoto (carro ou edge)
				//verificar se já foi atribuída alguma tarefa; e se a chave/node já estão em howMuchProcServer
				alreadyHad = false; //se o nó já estava no map
				for(std::map< Ptr<Node> , double>::iterator it = howMuchProcServer.begin(); //percorre o map de quanto cada nó deve processar
				  it != howMuchProcServer.end(); ++it){
					if((it->first == curNode)){ //se o nó atual já está no map
						alreadyHad=true; //já tinha o elemento no map
					}
				}
				if(alreadyHad==false){ //se ainda n tinha o servidor na lista do q vai ser processado
					howMuchProcServer[curNode] = workloadMatrix[i].at(1);
					howMuchDataForServer[curNode] = workloadMatrix[i].at(0);
					howManyTasksForServer[curNode] = 1.0; //por enqto, uma task
				}
				else { //se já tinha o servidor na lista do q vai ser processado
					howMuchProcServer[curNode] = howMuchProcServer[curNode] + workloadMatrix[i].at(1);
					howMuchDataForServer[curNode] = howMuchDataForServer[curNode] + workloadMatrix[i].at(0);
					howManyTasksForServer[curNode] += 1.0; //incrementa em 1
				}
				//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% VERIFICA SE ESTÁ OK OFFLOADAR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				//analisa a viabilidade de processar a task... quanto o servidor vai processar?
				bool knownRoute = false; //verificar se o servidor possui rota conhecida
				double curRange = 250.0; //current range, muda apenas se for para edge server
				uint32_t curId = curNode->GetId(); //pegar ID do carro servidor
				//quanto tempo levaria para processar a task? tempo de upload + download + tempo de espera na fila + tempo de processamento
				comTime = 0.0; //tempo gasto com upload e download
				//saber se usa 5g ou wave
				if(auxProvidersCap[curNode] >= 1.5){ //se tem capacidade de cpu >= 1.5 é edge server/5G
					comTime = (howMuchDataForServer[curNode] + 1000)/(datarateMmWave*0.25) + 0.05; //1000 é a volta do resultado do processamento
					knownRoute = true; //o edge server sempre está parado
					curRange = 220.0; //muda o current range
				}
				else { // é carro server/WAVE
					comTime = (howMuchDataForServer[curNode] + 1000)/(datarateWave*0.25) + 0.05; //1000 é a volta do resultado do processamento
					//verificar se a rota do carro server é conhecida
					uint32_t positionInC = 0;
					for(uint32_t i=0; i<c.GetN();i++){
						if(curId == c.Get(i)->GetId()){
							positionInC = i;
							break;
						}
					}
					knownRoute = carNodeKnownRoutes[positionInC]; //1 é true e 0 é false
				}
				//como as cargas anteriores já foram adicionadas à fila, aqui deveríamos adicionar só a nova task... mas do jeito q está, fica como margem
				queueProcTime = calcSleepTime(auxProvidersCap[curNode], auxProvidersQueue[curNode], howMuchProcServer[curNode]);
				//@TODO: esse tempo está dobrado pq adicionamos à fila e e adicionamos em howMuchProcServer?
				howLongTake = comTime + queueProcTime;
				//expectedResultTime[getIPFromServer(curNode)] = howLongTake; //armazena essa informação por servidor

				bool withinrange = false; //estarão dentro do alcance um do outro?
				double let = 0.0; //tempo de vida será calculado se a rota n for conhecida
				if(knownRoute==true){
					double futureTime = Simulator::Now().GetSeconds () + howLongTake; //pegar o tempo a ser analisado
					uint32_t fTime = ceil(futureTime); //arrendondar para cima
					withinrange = withinRangeGtt(fTime,clientId,curId,curRange); //os dois nós ainda estarão dentro do alcance um do outro?
				} else {
					Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>(); //calcular o tempo de vida tb
					Ptr<MobilityModel> model2 = curNode->GetObject<MobilityModel>(); //pega o modelo de mobilidade do servidor atual
					let = linkEstimatedLifeTime(model1,model2,curRange); // Tempo de vida do enlace; 'let'
				}

				isOkToOffload = false;

				energyPerSecond = auxProvidersCpuConsumption[curNode]/3600.0; //energia gasta em 1s (em Watt)
				expectedCsmdEnergy = queueProcTime*energyPerSecond; //consumo de energia esperado c fila e novo processamento acumulado
				//se as rotas são conhecidas e se estarão conectados ainda
				if( (howLongTake < 30.0) && (((knownRoute==false) && (howLongTake <= (let-2.0))) || (knownRoute==true && withinrange==true))) {
				//if( (howLongTake < 30.0) && (howLongTake <= (auxProvidersLlt[curNode]-2.0)) ) { //se estarão conectados ainda

					//if((auxProvidersEnergyLevel[curNode] - expectedCsmdEnergy) > auxProvidersMinimalEnergy[curNode]){ //verifica energia
						isOkToOffload = true;
					//}
					if((auxProvidersEnergyLevel[curNode] - expectedCsmdEnergy) < auxProvidersMinimalEnergy[curNode]){ //podem haver violações
						numberOfEnergyViolations += 1;
					}
				}
				//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				if(isOkToOffload==true){
					//atualizar nova carga; somar o processamento à fila do cliente
					auxProvidersQueue[curNode] = auxProvidersQueue[curNode] + workloadMatrix[i].at(1)/auxProvidersCap[curNode];
					tasksOffloadedSuc += 1;
					workloadMatrix.erase(workloadMatrix.begin() + j); //deletar tarefas q já estão sendo processadas localmente
					//reordenar a lista
					sortAuxProviders.clear();
					sortByQueueCapDistGtt();
				}
				else {
					howMuchProcServer[curNode] = howMuchProcServer[curNode] - workloadMatrix[j].at(1);
					howMuchDataForServer[curNode] = howMuchDataForServer[curNode] - workloadMatrix[j].at(0);
					howManyTasksForServer[curNode] -= 1.0; //decrementa em 1
					//fazer o elemento cair na lista de ordenação
					auxProvidersDist[curNode] = auxProvidersDist[curNode] + 987654321.0; //adiciona esse tempo grande só p descer na list
					//reordenar a lista
					sortAuxProviders.clear();
					sortByQueueCapDistGtt();
					//j++; //vai tentar com o servidor seguinte
					//if(j>= sortAuxProviders.size()){ //já percorreu toda a lista de servidores
					//	break;
					//}
				}
			}
		}
		avoidInfiniteLoop += 1;
	}

	//%%%%%%%%%%%%%%%%%%%% CLIENTE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
	howMuchProcLocalGtt = howMuchProcLocal; //contabilizar p impressãos
	if(howMuchProcLocal > 0){ //tem algo para processar no cliente/local
		localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProcLocal); //processamento do cliente
	}
	//std::cout << "[DECISION] ClientLocal: " << getIPFromServer(clients.Get(0)) << ". cpuQueue: " << carNodeCPUQueue[clientId] <<
	//		". cpuCap: " << carNodeCPUCap[clientId] << ". Processing: " << howMuchProcLocal << "Gc," << std::endl;
	//os << "[DECISION] ClientLocal: " << getIPFromServer(clients.Get(0)) << ". cpuQueue: " << carNodeCPUQueue[clientId] <<
	//		". cpuCap: " << carNodeCPUCap[clientId] << ". Processing: " << howMuchProcLocal << "Gc," << std::endl;
	//%%%%%%%%%%%%%%%%%%%% CLIENTE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


	//%%%%%%%%%%%%%%%%%%%% SERVIDORES %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	for(std::map< Ptr<Node> , double>::iterator it = howMuchProcServer.begin(); //percorre o map de quanto cada nó deve processar
	  it != howMuchProcServer.end(); ++it){
		Ptr<Node> curNode = it->first; //nó atual
		if((howMuchProcServer[curNode] > 0) && (howMuchDataForServer[curNode] > 0)){ //se esse servidor vai receber algo p processar
			providers.push_back(curNode);
			tempVec.clear();
			tempVec.push_back(howMuchDataForServer[curNode]);
			tempVec.push_back(howMuchProcServer[curNode]);
			tempVec.push_back(howManyTasksForServer[curNode]);
			scheduleMatrix.push_back(tempVec);
			numberOfSurrogates += 1;
			tempVec.clear();
		}
		//if(auxProvidersCap[curNode] >= 1.5){ //se tem capacidade de cpu >= 1.5 é edge server/5G
		//	std::cout << "[DECISION] Edge: " << getIPFromServer(curNode);
		//	os << "[DECISION] Edge: " << getIPFromServer(curNode);
		//}
		//else { // é carro server/WAVE
		//	std::cout << "[DECISION] CarServer: " << getIPFromServer(curNode);
		//	os << "[DECISION] CarServer: " << getIPFromServer(curNode);
		//}
		//double timetoproc = howMuchProcServer[curNode]/auxProvidersCap[curNode]; //apenas p ajudar a imprimir
		//std::cout << ". cpuQueue: " << auxProvidersQueue[curNode] - timetoproc << ". cpuCap: " << auxProvidersCap[curNode]
		//	   << ". Processing: " << howMuchProcServer[curNode] << "Gc. Transfering: " << howMuchDataForServer[curNode] << "Bytes. Distance: " <<
		//	   auxProvidersDist[curNode] << std::endl;
		//os << ". cpuQueue: " << auxProvidersQueue[curNode] - timetoproc << ". cpuCap: " << auxProvidersCap[curNode]
		//	   << ". Processing: " << howMuchProcServer[curNode] << "Gc. Transfering: " << howMuchDataForServer[curNode] << "Bytes. Distance: " <<
		//	   auxProvidersDist[curNode] << std::endl;
	}
	//%%%%%%%%%%%%%%%%%%%% SERVIDORES %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	//timestamps e elapsedTime... para saber o qto demora o corpo do algoritmo de decisão
	endElapsedTime = std::chrono::high_resolution_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
	elapsedTime *= 1e-9;
	std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
	os << "[ELAPSED TIME] " << elapsedTime << std::endl;
	realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;

	//dá prosseguimento ao offloading
	if ((providers.size() == numberOfSurrogates) && (alreadySent == false) && (workloadMatrix.size()==0)) {
		printDecisionGtt();
		for(uint32_t i = 0; i < providers.size(); i++){
			Simulator::Schedule(Seconds(0.0),serverSide);
			Simulator::Schedule(Seconds(0.0),clientSide);
			//Simulator::Schedule(Seconds(i*0.001),clientSide);
		}
		alreadySent = true;
		Simulator::Schedule(Seconds(1.0),checkConnectivity); //dá início as verificações periódicas de conectividade
	}
	else if ((alreadySent == false) && (workloadMatrix.size()>0)) { //não tenta de novo
		sortAuxProviders.clear(); //zera tudo
		providers.clear();
		scheduleMatrix.clear();
		workloadMatrix.clear();
		workloadMatrix = backupWorkloadMatrix; //resgatar workloadMatrix
		numberOfSurrogates = 0;
		tasksOnlyLocal=0;
		tasksOffloadedSuc=0;

		printDecisionGtt();
		//checkOffloadingSuccess(false); //verifica para finalizar a simulação sem offloading
	}
}

void printDecisionGtt(){
	if(providers.size() > 0){
		std::cout << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		os << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		double timetoproc = howMuchProcLocalGtt/auxProvidersCap[c.Get(clientId)]; //apenas p ajudar a imprimir
		std::cout << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] - timetoproc << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalGtt << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
		os << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] - timetoproc << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalGtt << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;

		for(uint32_t i=0; i<providers.size(); i++){
			if(auxProvidersCap[providers[i]] >= 1.5){ //se tem capacidade de cpu >= 1.5 é edge server/5G
				std::cout << "[DECISION] Edge: " << getIPFromServer(providers[i]);
				os << "[DECISION] Edge: " << getIPFromServer(providers[i]);
			}
			else{
				std::cout << "[DECISION] CarServer: " << getIPFromServer(providers[i]);
				os << "[DECISION] CarServer: " << getIPFromServer(providers[i]);
			}
			double timetoproc = scheduleMatrix[i][1]/auxProvidersCap[providers[i]]; //apenas p ajudar a imprimir
			std::cout << ". cpuQueue: " << auxProvidersQueue[providers[i]] - timetoproc << ". cpuCap: " << auxProvidersCap[providers[i]]
			   << ". Processing: " << scheduleMatrix[i][1] << "Gc. Transfering: " << scheduleMatrix[i][0] <<
			   "Bytes. Distance: " << auxProvidersDistGttOriginal[providers[i]] << std::endl;
			os << ". cpuQueue: " << auxProvidersQueue[providers[i]] - timetoproc << ". cpuCap: " << auxProvidersCap[providers[i]]
			   << ". Processing: " << scheduleMatrix[i][1] << "Gc. Transfering: " << scheduleMatrix[i][0] <<
			   "Bytes. Distance: " << auxProvidersDistGttOriginal[providers[i]] << std::endl;
		}
	}
	else{ //se n deu certo, executa tudo localmente
		howMuchProcLocalGtt = 0.0;
		for(uint32_t j=0; j<workloadMatrix.size(); j++){
			howMuchProcLocalGtt = howMuchProcLocalGtt + workloadMatrix[j].at(1);
		}
		std::cout << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		os << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		std::cout << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalGtt << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
		os << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalGtt << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
	}
}

void gttDecision (Ipv4Address ipv4From, uint32_t iface) {
	if(iface==1){ //se recebeu resposta pela interface 5G; sempre vai receber primeiro por aqui...
		for (size_t i = 0; i < edgeNodes.GetN(); i++) {
			if (ipv4From == edgeNodes.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //a borda só tem interface 5G

				enbNotFound = false; //enb foi encontrado
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = enbNodes.Get(i)->GetObject<MobilityModel>(); //pega o modelo de mobilidade do enb associado
				double distance = GetDistance(model1,model2);
				double let = linkEstimatedLifeTime(model1,model2,210.0); // Tempo de vida do enlace; 'let', botei range menor
				std::cout << "[CLIENTE] GTT: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
						edgeNodeCPUCap[idxFromMoreClosestEnb] << " e CpuQueue ==> " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << std::endl;
				os << "[CLIENTE] GTT: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
						edgeNodeCPUCap[idxFromMoreClosestEnb] << " e CpuQueue ==> " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << std::endl;


				double procTime = calcSleepTime(edgeNodeCPUCap[idxFromMoreClosestEnb], edgeNodeCPUQueue[idxFromMoreClosestEnb],
						2*3); //3-cpuRequired médio; @TODO: melhorar isso aqui
				double transferTime = (2*1200000 + 1000)/(datarateMmWave*0.25) + 0.05; //1200000-taskSize médio; @TODO: melhorar isso aqui
				double possibleTime = transferTime + procTime;

				if(let >= possibleTime){
					if(alreadySent == false){ //enquanto ainda não foi enviado o workload, pode continuar adicionando possíveis providers
						if(alreadyDecided == false){ //se ainda n decidiu qtos mandar
							auxProvidersDist[edgeNodes.Get(i)] = distance;
							auxProvidersDistGttOriginal[edgeNodes.Get(i)] = distance;
							auxProvidersQueue[edgeNodes.Get(i)] = edgeNodeCPUQueue[idxFromMoreClosestEnb];
							auxProvidersCap[edgeNodes.Get(i)] = edgeNodeCPUCap[idxFromMoreClosestEnb];
							auxProvidersCpuConsumption[edgeNodes.Get(i)] = edgeCpuConsumption[idxFromMoreClosestEnb]; //parâmetros de energia
							auxProvidersMinimalEnergy[edgeNodes.Get(i)] = -999.0;
							auxProvidersEnergyLevel[edgeNodes.Get(i)] = 999999;
						}
					}
				}
				break;
			}
		}
	}
	if(iface == 2){
		for (size_t i = 0; i < c.GetN(); i++) {
			if (ipv4From == c.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //se o surrogate escolhido respondeu
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = c.Get(i)->GetObject<MobilityModel>();
				double distance = GetDistance(model1,model2);
				double let = linkEstimatedLifeTime(model1,model2,240.0); // Tempo de vida do enlace; 'let', botei range menor
				std::cout << "[CLIENTE] GTT: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
						carNodeCPUCap[i] << " e CpuQueue ==> " << carNodeCPUQueue[i] << std::endl;
				os << "[CLIENTE] GTT: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
						carNodeCPUCap[i] << " e CpuQueue ==> " << carNodeCPUQueue[i] << std::endl;

				double procTime = calcSleepTime(carNodeCPUCap[i], carNodeCPUQueue[i], 3); //3-cpuRequired médio; @TODO: média das tasks?
				double transferTime = (1200000 + 1000)/(datarateWave*0.25) + 0.05; //1200000-taskSize médio;
				double possibleTime = transferTime + procTime;

				if (let >= possibleTime){
					if(alreadySent == false){ //enquanto ainda não foi enviado o workload, pode continuar adicionando possíveis providers
						if(alreadyDecided == false){ //se ainda n decidiu qtos mandar
							auxProvidersQueue[c.Get(i)] = carNodeCPUQueue[i];
							auxProvidersCap[c.Get(i)] = carNodeCPUCap[i];
							auxProvidersDist[c.Get(i)] = distance;
							auxProvidersDistGttOriginal[c.Get(i)] = distance;
							auxProvidersCpuConsumption[c.Get(i)] = carCpuConsumption[i]; //parâmetros de energia
							auxProvidersMinimalEnergy[c.Get(i)] = carMinimalEnergy[i];
							auxProvidersEnergyLevel[c.Get(i)] = carEnergyLevel[i];
						}
					}
				}
				break;
			}
		}
	}
	//após 0.5s (tempo necessário para receber mais respostas dos substitutos), chama addProviders
	if((alreadySentToSort == false) && (alreadyDecided == false)){ //e o número de providers já é suficiente
		Simulator::Schedule(Seconds(0.5),SortAndInitTransferGtt); //só dá prosseguimento após 0.5s
		alreadySentToSort = true;
	}
}
