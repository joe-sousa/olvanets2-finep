/*
*  Alisson Barbosa, Junho/2021. E-mail: alisson@ufc.br
*  This code realizes the packets exchange between vehicles (V2V) and the edge (V2I), simulating
*  the send of subtasks to be executed in the surrogate vehicles and in the edge of the network.
*
*  This code is protected by copyright and intellectual property right laws.
*  Do not use or distribute without the author's authorization.
*  This is true even for modified versions or snippets of this code.
*
*/

#include "scratch/olvanets/mdo.h"
//#include <stdlib.h>

double howMuchProcLocalMdo;
std::map <Ptr<Node> ,double> auxProvidersDistMdoOriginal; //map auxiliar p guardar a distância original
double maxExecTime; //pior tempo de execução estimado de um veículo/task
std::map <Ptr<Node> ,double> auxProvidersEstTrans; //node / o que iria gastar para receber as tasks
std::map <Ptr<Node> ,double> auxProvidersEstTtotal; //node / o que iria gastar para processar as tasks, contando transmissao tb

void calcMaxTime(){
	maxExecTime = 0.0;
	double curExecTime = 0.0;
	double comTime = 0.0;
	for(std::map< Ptr<Node> , double>::iterator it = auxProvidersQueue.begin(); //percorre o map da fila
	  it != auxProvidersQueue.end(); ++it){
		if(it->first != c.Get(clientId)){ //se n é o cliente
			comTime = (workloadMatrix[0].at(0) + 1000)/(datarateWave*0.25) + 0.05 + auxProvidersEstTrans[it->first]; //ttrans + o q ja tem
		}
		//+ tempo de processamento + tempo de fila
		double queueProcTime = calcSleepTime(auxProvidersCap[it->first], auxProvidersQueue[it->first], workloadMatrix[0].at(1));
		curExecTime = comTime + queueProcTime;
		if(curExecTime > maxExecTime){
			maxExecTime = curExecTime;
		}
	}
}


void sortByQueueCapDistMdo(){

	for(std::map< Ptr<Node> , double>::iterator it = auxProvidersQueue.begin(); //percorre o map da fila
	  it != auxProvidersQueue.end(); ++it){
		if(it->first != c.Get(clientId)){ //se n é o cliente
			double comTime = (workloadMatrix[0].at(0) + 1000)/(datarateWave*0.25) + 0.05 + auxProvidersEstTrans[it->first]; //ttrans + o q ja tem
			double queueProcTime = calcSleepTime(auxProvidersCap[it->first], auxProvidersQueue[it->first], workloadMatrix[0].at(1));
			auxProvidersEstTtotal[it->first] = comTime + queueProcTime; //coloca a estimativa de tempo por node
		}
	}

	// copia os pares chave-valor do map
	std::copy(auxProvidersEstTtotal.begin(),
			auxProvidersEstTtotal.end(),
			std::back_inserter<std::vector<pair>>(sortAuxProviders));
	// ordene o vetor em ordem crescente pelo segundo valor do pair
	// se o segundo valor for igual, ordene pelo primeiro valor do par
	std::sort(sortAuxProviders.begin(), sortAuxProviders.end(),
			[](const pair& l, const pair& r) {
		if (l.second != r.second)
			return l.second < r.second;
		return l.first < r.first;
	});

}


void SortAndInitTransferMdo() {

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
	double energyPerSecond; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
	double expectedCsmdEnergy; //calcular consumo de energia esperado
	bool alreadyHad;
	double comTime; //tempo gasto com upload e download
	double queueProcTime;
	double howLongTake;
	bool isOkToOffload;
	std::vector<double> tempVec;


	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;

	startElapsedTime = std::chrono::high_resolution_clock::now();

	sortByQueueCapDistMdo(); //ordena os possíveis servidores

	//while((workloadMatrix.size()> 0) && (i<workloadMatrix.size())){ //percorrer as tarefas
	while((workloadMatrix.size()> 0)){ //percorrer as tarefas
		if(avoidInfiniteLoop>=50){
			break;
		}
		//jogar tarefa para o primeiro servidor da lista
		if(sortAuxProviders.size() > 0){
			curNode = sortAuxProviders[j].first; //nó atual... sempre pega o primeiro da lista... não é o cliente... é um servidor remoto (carro)
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
			double curRange = 250.0; //current range, muda apenas se for para edge server
			//uint32_t curId = curNode->GetId(); //pegar ID do carro servidor
			//quanto tempo levaria para processar a task? tempo de upload + download + tempo de espera na fila + tempo de processamento
			comTime = 0.0; //tempo gasto com upload e download ... é carro server/WAVE
			comTime = (howMuchDataForServer[curNode] + 1000*howManyTasksForServer[curNode])/(datarateWave*0.25) + 0.05; //1000 é a volta do result
			//como as cargas anteriores já foram adicionadas à fila, aqui deveríamos adicionar só a nova task... mas do jeito q está, fica como margem
			queueProcTime = calcSleepTime(auxProvidersCap[curNode], auxProvidersQueue[curNode], howMuchProcServer[curNode]);
			//@TODO: esse tempo está dobrado pq adicionamos à fila e e adicionamos em howMuchProcServer?
			howLongTake = comTime + queueProcTime;
			//expectedResultTime[getIPFromServer(curNode)] = howLongTake; //armazena essa informação por servidor

			double let = 0.0; //tempo de vida será calculado se a rota n for conhecida
			Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>(); //calcular o tempo de vida tb
			Ptr<MobilityModel> model2 = curNode->GetObject<MobilityModel>(); //pega o modelo de mobilidade do servidor atual
			let = linkEstimatedLifeTime(model1,model2,curRange); // Tempo de vida do enlace; 'let'

			isOkToOffload = false;

			energyPerSecond = auxProvidersCpuConsumption[curNode]/3600.0; //energia gasta em 1s (em Watt)
			expectedCsmdEnergy = queueProcTime*energyPerSecond; //consumo de energia esperado c fila e novo processamento acumulado
			//se as rotas são conhecidas e se estarão conectados ainda
			if( (howLongTake < 30.0) && (howLongTake <= (let-2.0)) ) {
			//if( (howLongTake < 30.0) && (howLongTake <= (auxProvidersLlt[curNode]-2.0)) ) { //se estarão conectados ainda
				isOkToOffload = true;
				if((auxProvidersEnergyLevel[curNode] - expectedCsmdEnergy) < auxProvidersMinimalEnergy[curNode]){ //verifica energia
					numberOfEnergyViolations += 1;
				}
			}
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			if(isOkToOffload==true){
				//atualizar nova carga; somar o processamento à fila do cliente
				auxProvidersQueue[curNode] = auxProvidersQueue[curNode] + workloadMatrix[i].at(1)/auxProvidersCap[curNode];
				auxProvidersEstTrans[curNode] = comTime;
				tasksOffloadedSuc += 1;
				workloadMatrix.erase(workloadMatrix.begin() + j); //deletar tarefas q já estão sendo processadas localmente
				//reordenar a lista
				sortAuxProviders.clear();
				sortByQueueCapDistMdo();
			}
			else {
				//apaga tudo da tentativa desse último servidor
				howMuchProcServer[curNode] = howMuchProcServer[curNode] - workloadMatrix[j].at(1);
				howMuchDataForServer[curNode] = howMuchDataForServer[curNode] - workloadMatrix[j].at(0);
				howManyTasksForServer[curNode] -= 1.0; //decrementa em 1
				//fazer o elemento cair na lista de ordenação
				auxProvidersDist[curNode] = auxProvidersDist[curNode] + 987654321.0; //adiciona esse tempo grande só p descer na list

				//executa local
				howMuchProcLocal = howMuchProcLocal + workloadMatrix[i].at(1); //quanto o client deve processar... jogar a tarefa para o cliente
				//atualizar nova carga; somar o processamento à fila do cliente
				auxProvidersQueue[c.Get(clientId)] = auxProvidersQueue[c.Get(clientId)] + workloadMatrix[i].at(1)/auxProvidersCap[c.Get(clientId)];
				tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
				workloadMatrix.erase(workloadMatrix.begin() + i); //deletar tarefas q já estão sendo processadas localmente

				//reordenar a lista
				sortAuxProviders.clear();
				sortByQueueCapDistMdo();
			}

		}
		avoidInfiniteLoop += 1;
	}

	//%%%%%%%%%%%%%%%%%%%% CLIENTE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
	howMuchProcLocalMdo = howMuchProcLocal; //contabilizar p impressãos
	if(howMuchProcLocal > 0){ //tem algo para processar no cliente/local
		localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProcLocal); //processamento do cliente
	}

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
		printDecisionMdo();
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

		printDecisionMdo();
		//checkOffloadingSuccess(false); //verifica para finalizar a simulação sem offloading
	}
}

void printDecisionMdo(){
	if(providers.size() > 0){
		std::cout << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		os << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		double timetoproc = howMuchProcLocalMdo/auxProvidersCap[c.Get(clientId)]; //apenas p ajudar a imprimir
		std::cout << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] - timetoproc << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalMdo << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
		os << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] - timetoproc << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalMdo << "Gc. Transfering: " << 0 <<
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
			   "Bytes. Distance: " << auxProvidersDistMdoOriginal[providers[i]] << std::endl;
			os << ". cpuQueue: " << auxProvidersQueue[providers[i]] - timetoproc << ". cpuCap: " << auxProvidersCap[providers[i]]
			   << ". Processing: " << scheduleMatrix[i][1] << "Gc. Transfering: " << scheduleMatrix[i][0] <<
			   "Bytes. Distance: " << auxProvidersDistMdoOriginal[providers[i]] << std::endl;
		}
	}
	else{ //se n deu certo, executa tudo localmente
		howMuchProcLocalMdo = 0.0;
		for(uint32_t j=0; j<workloadMatrix.size(); j++){
			howMuchProcLocalMdo = howMuchProcLocalMdo + workloadMatrix[j].at(1);
		}
		std::cout << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		os << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		std::cout << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalMdo << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
		os << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalMdo << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
	}
}

void mdoDecision (Ipv4Address ipv4From, uint32_t iface) {
	if(iface == 2){
		for (size_t i = 0; i < c.GetN(); i++) {
			if (ipv4From == c.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //se o surrogate escolhido respondeu
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = c.Get(i)->GetObject<MobilityModel>();
				double distance = GetDistance(model1,model2);
				double let = linkEstimatedLifeTime(model1,model2,240.0); // Tempo de vida do enlace; 'let', botei range menor
				std::cout << "[CLIENTE] MDO: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
						carNodeCPUCap[i] << " e CpuQueue ==> " << carNodeCPUQueue[i] << std::endl;
				os << "[CLIENTE] MDO: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
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
							auxProvidersDistMdoOriginal[c.Get(i)] = distance;
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
		Simulator::Schedule(Seconds(0.5),SortAndInitTransferMdo); //só dá prosseguimento após 0.5s
		alreadySentToSort = true;
	}
}
