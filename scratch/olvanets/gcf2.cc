/*
*  Alisson Barbosa, Julho/2020. E-mail: alisson@ufc.br
*  This code realizes the packets exchange between vehicles (V2V) and the edge (V2I), simulating
*  the send of subtasks to be executed in the surrogate vehicles and in the edge of the network.
*
*  This code is protected by copyright and intellectual property right laws.
*  Do not use or distribute without the author's authorization.
*  This is true even for modified versions or snippets of this code.
*
*/

#include "scratch/olvanets/gcf2.h"

//variáveis da função gcfRemainder()
std::map< Ptr<Node> , double> tempProvidersGcf;
double currentTimeGcf;
int nrtasksGcf; //tasks n alocadas p serem executadas no cliente
double whatProcessGcf;
bool hasBreakGcf; //diz se caiu no break
double sleepLocalTimeGcf;
double queueProcTimeGcf;
double energyPerSecondGcf; //energia gasta em 1s (em Watt)
double expectedCsmdEnergyGcf; //consumo de energia esperado c fila e novo processamento acumulado
double howMuchProcLocalGcf; //final de quanto o cliente vai processar

//void testeGcf2(){
//	std::cout << "initAction: " << initAction << std::endl;
//}

//função para executar a rebarba localmente
void gcfRemainder(){
	currentTimeGcf = Simulator::Now().GetSeconds ();
	nrtasksGcf = 0; //tasks n alocadas p serem executadas no cliente
	//executar o restante de scheduleMatrix localmente
	whatProcessGcf = 0.0;
	hasBreakGcf = false; //diz se caiu no break
	while(workloadMatrix.size()>0){
		whatProcessGcf = whatProcessGcf + workloadMatrix[0].at(1);

		queueProcTimeGcf = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], whatProcessGcf);
		energyPerSecondGcf = auxProvidersCpuConsumption[c.Get(clientId)]/3600.0; //energia gasta em 1s (em Watt)
		expectedCsmdEnergyGcf = queueProcTimeGcf*energyPerSecondGcf; //consumo de energia esperado c fila e novo processamento acumulado
		if((auxProvidersEnergyLevel[c.Get(clientId)] - expectedCsmdEnergyGcf) > auxProvidersMinimalEnergy[c.Get(clientId)]){ //verifica energia
			nrtasksGcf += 1; //incrementa em 1
			workloadMatrix.erase(workloadMatrix.begin()); //deletar tarefas q já estão sendo processadas localmente
		}
		else{
			hasBreakGcf = true;
			break; //sair do laço while
		}
	}
	if(hasBreakGcf == true){
		whatProcessGcf = whatProcessGcf - workloadMatrix[0].at(1); //deleta a última task a ser processada localmente pq o cliente n aguentou
	}
	if(whatProcessGcf <= 0.0){
		return;
	}
	//double numberOfRemainingTasks = whatProcess/wAvgCpuReq; //@TODO: atenção aqui se forem tarefas de tamanhos diferentes
	//int nrtasks = (int)numberOfRemainingTasks;
	howMuchProcLocalGcf = howMuchProcLocalGcf + whatProcessGcf; //só p imprimir correto na decisão
	tasksOnlyLocal = tasksOnlyLocal + nrtasksGcf; //outras tarefas serão executadas localmente desde o início
	//executa as tarefas desalocadas localmente
	if(localTime < 1.0){ //se nenhuma tarefa foi alocada aqui neste programa para o local do cliente
			sleepLocalTimeGcf = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], whatProcessGcf);
			localTime = (currentTimeGcf - tempoIni) + sleepLocalTimeGcf;
	} else { //alguma tarefa foi alocada neste programa para o local do cliente
			if(currentTimeGcf > (localTime + tempoIni)){ //se já executou a tarefa alocada
				sleepLocalTimeGcf = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], whatProcessGcf);
				localTime = currentTimeGcf + sleepLocalTimeGcf;
			} else { //se não executou a tarefa alocada
				sleepLocalTimeGcf = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], whatProcessGcf);
				localTime = localTime + sleepLocalTimeGcf;
			}
	}
}

//queue -> disponibilidade de CPU
void sortByQueueAndDistance(){
	//preciso fazer uma soma do valores (já normalizados) dos nós nos três maps e jogar no map temporário

	for(std::map< Ptr<Node> , double>::iterator it = auxProvidersQueue.begin(); //percorre o map da fila
		it != auxProvidersQueue.end(); ++it){
		for(std::map< Ptr<Node> , double>::iterator jt = auxProvidersDist.begin(); //percorre o map da distância
			jt != auxProvidersDist.end(); ++jt){
			if(it->first == jt->first){ //se são os mesmos nós
				//preciso da média da cpu required do workloads
				tempProvidersGcf[it->first] = it->second + (jt->second/trange); //soma equalizada
			}
		}
	}

	// copia os pares chave-valor do map com a soma cap+distance (normalizada/equalizada)
	std::copy(tempProvidersGcf.begin(),
			tempProvidersGcf.end(),
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

//calcula todos os tempos de vida antes de iniciar a decisão
void calculateLltGcf(){

	//calcular tempo de vida e withinRange para cada um dos availableServers
	for(std::map< Ptr<Node> , double>::iterator it = auxProvidersQueue.begin(); //percorrer servers that replied (auxProvidersQueue ou cap ou dist)
	  it != auxProvidersQueue.end(); ++it){
		if(it->first == c.Get(clientId)){ //se é o cliente
			auxProvidersLlt[it->first] = 100; //tempo de vida infinito (é ele mesmo)
		}
		else{
			if(auxProvidersCap[it->first] >= 1.5){ //se é o edgeNode
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = it->first->GetObject<MobilityModel>();
				double let = linkEstimatedLifeTime(model1,model2,220.0); // Tempo de vida do enlace; 'let', botei range menor
				auxProvidersLlt[it->first] = let;
			}
			else{ //se é carServer
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = it->first->GetObject<MobilityModel>();
				double let = linkEstimatedLifeTime(model1,model2,240.0); // Tempo de vida do enlace; 'let', botei range menor
				auxProvidersLlt[it->first] = let;
			}
		}
	}
}

void SortAndInitTransfer2() {
	numberOfSurrogates = 0; //zera o número de surrogates por enqto
	double currentTime = Simulator::Now().GetSeconds ();
	//inicialmente o cliente não processa algo localmente
	localTime = 0 //o cliente não processa local
			+ (currentTime - tempoIni);  //tempo até chegar aqui desde o initAction2
	uint32_t numberOfTasksToBeProcessed=0; //número de tarefas a ser processada
	Ptr<Node> curNode;
	Ptr<MobilityModel> model1;
	Ptr<MobilityModel> model2;
	double howMuchProc = 0.0; //quanto este servidor deve processar
	double totalTaskSize = 0.0; //qual o tamanho total do pacote para transferir para esse servidor
	uint32_t j = 0; //indexador do loop das tarefas
	uint32_t ntbp = 0; //indexador do número de tarefas a serem processadas
	uint32_t k = 0; //indexador do loop das tarefas
	uint32_t m = 0; //indexador do loop das tarefas
	double let;
	double curRange;
	double comTime;
	double queueProcTime;
	double energyPerSecond; //energia gasta em 1s (em Watt)
	double expectedCsmdEnergy; //consumo de energia esperado c fila e novo processamento acumulado
	double howLongTake;
	bool isOkToOffload;
	std::vector<double> tempVec;

	//calculateLltGcf(); //calcula logo os tempos de vida dos possíveis servidores

	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;

	startElapsedTime = std::chrono::high_resolution_clock::now();

	sortByQueueAndDistance(); //ordena os possíveis carros servidores


	if(workloadMatrix.size() > 0){ //se ainda houver tasks desalocadas no workload
		for(uint32_t i=0; i< sortAuxProviders.size(); i++){ //percorrer os possíveis servidores remotos
			curNode = sortAuxProviders[i].first; //nó atual
			//uint32_t curId = curNode->GetId(); //pegar ID do carro servidor
			if(auxProvidersCap[sortAuxProviders[i].first] >= 1.5){ //verifica primeiro o edge server... /edge n tem restrição de energia
				numberOfTasksToBeProcessed=0; //número de tarefas a ser processada
				if(auxProvidersQueue[sortAuxProviders[i].first]<=2){
					if(workloadMatrix.size()>4){
						numberOfTasksToBeProcessed=6;
					}
					else {
						numberOfTasksToBeProcessed=4;
					}
				} else if((auxProvidersQueue[sortAuxProviders[i].first]>2)&&(auxProvidersQueue[sortAuxProviders[i].first]<=6)){
					numberOfTasksToBeProcessed=4;
				} else if(auxProvidersQueue[sortAuxProviders[i].first]>6){
					numberOfTasksToBeProcessed=2;
				}
				//percorre cada task do workload
				howMuchProc = 0.0; //quanto este servidor deve processar
				totalTaskSize = 0.0; //qual o tamanho total do pacote para transferir para esse servidor
				j = 0; //indexador do loop das tarefas
				ntbp = 0; //indexador do número de tarefas a serem processadas
				while((workloadMatrix.size()> 0) && (j<workloadMatrix.size()) && (ntbp<numberOfTasksToBeProcessed)){
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% VERIFICA SE ESTÁ OK OFFLOADAR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//analisa a viabilidade de processar a task... quanto o servidor vai processar?
					howMuchProc = howMuchProc + workloadMatrix[j].at(1);
					totalTaskSize = totalTaskSize + workloadMatrix[j].at(0);
					curRange = 250.0; //current range, muda apenas se for para edge server
					//quanto tempo levaria para processar a task? tempo de upload + download + tempo de espera na fila + tempo de processamento
					comTime = 0.0; //tempo gasto com upload e download
					comTime = (totalTaskSize + 1000)/(datarateMmWave*0.25) + 0.05; //1000 é a volta do resultado do processamento
					curRange = 220.0; //muda o current range
					queueProcTime = calcSleepTime(auxProvidersCap[sortAuxProviders[i].first], auxProvidersQueue[sortAuxProviders[i].first],
							howMuchProc);
					howLongTake = comTime + queueProcTime;
					expectedResultTime[getIPFromServer(curNode)] = howLongTake; //armazena essa informação por servidor

					let = 0.0;
					model1 = clients.Get(0)->GetObject<MobilityModel>(); //calcular o tempo de vida tb
					model2 = curNode->GetObject<MobilityModel>(); //pega o modelo de mobilidade do servidor atual
					let = linkEstimatedLifeTime(model1,model2,curRange); // Tempo de vida do enlace; 'let'

					isOkToOffload = false;

					if(howLongTake <= let){         //(howLongTake <= procDeadline) &&
					//if(howLongTake <= auxProvidersLlt[curNode]){         //(howLongTake <= procDeadline) &&
						isOkToOffload = true;
					}
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

					if(isOkToOffload==true){
						ntbp += 1;
						tasksOffloadedSuc += 1;
						workloadMatrix.erase(workloadMatrix.begin() + j); //deletar tarefas q já estão alocadas p serem processadas
					}
					else {
						howMuchProc = howMuchProc - workloadMatrix[j].at(1);
						totalTaskSize = totalTaskSize - workloadMatrix[j].at(0);
						j += 1; //só incrementa se n tiver deletado o primeiro elemento da lista
					}
				} //fim do while das tasks
				if((howMuchProc > 0) && (totalTaskSize > 0)){ //se esse servidor vai receber algo p processar
					providers.push_back(sortAuxProviders[i].first);
					tempVec.clear();
					tempVec.push_back(totalTaskSize);
					tempVec.push_back(howMuchProc);
					tempVec.push_back(ntbp*1.0);
					scheduleMatrix.push_back(tempVec);
					numberOfSurrogates += 1;
					tempVec.clear();
				}
				/*std::cout << "[DECISION] Edge: " << getIPFromServer(sortAuxProviders[i].first);
				os << "[DECISION] Edge: " << getIPFromServer(sortAuxProviders[i].first);
				std::cout << ". cpuQueue: " << auxProvidersQueue[sortAuxProviders[i].first] << ". cpuCap: " << auxProvidersCap[sortAuxProviders[i].first]
					   << ". Processing: " << howMuchProc << "Gc. Transfering: " << totalTaskSize << "Bytes" << std::endl;
				os << ". cpuQueue: " << auxProvidersQueue[sortAuxProviders[i].first] << ". cpuCap: " << auxProvidersCap[sortAuxProviders[i].first]
						<< ". Processing: " << howMuchProc << "Gc. Transfering: " << totalTaskSize << "Bytes" << std::endl;*/
				j=0; //zera o contador
				howMuchProc = 0.0; //zera o quanto que tem de processar
				totalTaskSize = 0.0; //zera o total do tamanho do pacote
			}
			else if(curNode == c.Get(clientId)){ //verifica se é o próprio cliente
				numberOfTasksToBeProcessed=0; //número de tarefas a ser processada
				if(attempts==0){
					if(auxProvidersQueue[sortAuxProviders[i].first]<=3){
						numberOfTasksToBeProcessed=1;
					} else if((auxProvidersQueue[sortAuxProviders[i].first]>3) && (auxProvidersQueue[sortAuxProviders[i].first]<=6)){
						numberOfTasksToBeProcessed=0;
					} else {
						numberOfTasksToBeProcessed=0;
					}
				}
				if(attempts==1){
					if(auxProvidersQueue[sortAuxProviders[i].first]<=4){
						numberOfTasksToBeProcessed=2;
					} else if((auxProvidersQueue[sortAuxProviders[i].first]>4) && (auxProvidersQueue[sortAuxProviders[i].first]<=6)){
						numberOfTasksToBeProcessed=1;
					} else {
						numberOfTasksToBeProcessed=0;
					}
				}
				//checar quanto o local pode fazer
				localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
				if(workloadMatrix.size() > 0){ //se houver tasks desalocadas no workload
					//percorre cada task do workload
					howMuchProc = 0.0; //quanto o client deve processar
					k = 0; //indexador do loop das tarefas
					ntbp = 0; //indexador do número de tarefas a serem processadas
					while((workloadMatrix.size()> 0) && (k<workloadMatrix.size()) && (ntbp<numberOfTasksToBeProcessed)){
						//analisa a viabilidade de processar a task... quanto o cliente vai processar?
						howMuchProc = howMuchProc + workloadMatrix[k].at(1);

						queueProcTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc);
						energyPerSecond = auxProvidersCpuConsumption[curNode]/3600.0; //energia gasta em 1s (em Watt)
						expectedCsmdEnergy = queueProcTime*energyPerSecond; //consumo de energia esperado c fila e novo processamento acumulado

						//quanto tempo levaria para processar a task? tempo de espera na fila + tempo de processamento
						//double howLongTake = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc);

						//if(howLongTake <= procDeadline){ //verifica se esse tempo é menor que o deadline
						if((auxProvidersEnergyLevel[curNode] - expectedCsmdEnergy) > auxProvidersMinimalEnergy[curNode]){ //verifica energia
							ntbp += 1;
							tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
							workloadMatrix.erase(workloadMatrix.begin() + k); //deletar tarefas q já estão sendo processadas localmente
						}
						else {
							howMuchProc = howMuchProc - workloadMatrix[k].at(1);
							k +=1; //só incrementa se n tiver deletado o primeiro elemento da lista
						}
					}
					if(howMuchProc>0.0){
						localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc); //processamento do cliente
					}
					howMuchProcLocalGcf = howMuchProc;
					/*std::cout << "[DECISION] ClientLocal: " << getIPFromServer(clients.Get(0)) << ". cpuQueue: " << carNodeCPUQueue[clientId] <<
							". cpuCap: " << carNodeCPUCap[clientId] << ". Processing: " << howMuchProc << "Gc," << std::endl;
					os << "[DECISION] ClientLocal: " << getIPFromServer(clients.Get(0)) << ". cpuQueue: " << carNodeCPUQueue[clientId] <<
							". cpuCap: " << carNodeCPUCap[clientId] << ". Processing: " << howMuchProc << "Gc," << std::endl;*/
					k=0; //zera o contador
					howMuchProc = 0.0; //zera o quanto que tem de processar
				}
			}
			else { //percorre os carros servers
				numberOfTasksToBeProcessed=0; //número de tarefas a ser processada
				if(attempts==0){
					if(auxProvidersQueue[sortAuxProviders[i].first]<=3){
						numberOfTasksToBeProcessed=1;
					} else if((auxProvidersQueue[sortAuxProviders[i].first]>3) && (auxProvidersQueue[sortAuxProviders[i].first]<=5)){
						numberOfTasksToBeProcessed=0;
					} else {
						numberOfTasksToBeProcessed=0;
					}
				}
				if(attempts==1){
					if(auxProvidersQueue[sortAuxProviders[i].first]<=2){
						numberOfTasksToBeProcessed=2;
					} else if((auxProvidersQueue[sortAuxProviders[i].first]>2) && (auxProvidersQueue[sortAuxProviders[i].first]<=5)){
						numberOfTasksToBeProcessed=1;
					} else {
						numberOfTasksToBeProcessed=0;
					}
				}
				//percorre cada task do workload
				howMuchProc = 0.0; //quanto este servidor deve processar
				totalTaskSize = 0.0; //qual o tamanho total do pacote para transferir para esse servidor
				m = 0; //indexador do loop das tarefas
				ntbp = 0; //indexador do número de tarefas a serem processadas
				while((workloadMatrix.size()> 0) && (m<workloadMatrix.size()) && (ntbp<numberOfTasksToBeProcessed)){
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% VERIFICA SE ESTÁ OK OFFLOADAR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//analisa a viabilidade de processar a task... quanto o servidor vai processar?
					howMuchProc = howMuchProc + workloadMatrix[m].at(1);
					totalTaskSize = totalTaskSize + workloadMatrix[m].at(0);
					curRange = 250.0; //current range, muda apenas se for para edge server
					//quanto tempo levaria para processar a task? tempo de upload + download + tempo de espera na fila + tempo de processamento
					comTime = 0.0; //tempo gasto com upload e download
					comTime = (totalTaskSize + 1000)/(datarateWave*0.25) + 0.05; //1000 é a volta do resultado do processamento
					queueProcTime = calcSleepTime(auxProvidersCap[sortAuxProviders[i].first], auxProvidersQueue[sortAuxProviders[i].first],
							howMuchProc);
					howLongTake = comTime + queueProcTime;
					expectedResultTime[getIPFromServer(curNode)] = howLongTake; //armazena essa informação por servidor

					let = 0.0; //tempo de vida será calculado se a rota n for conhecida
					model1 = clients.Get(0)->GetObject<MobilityModel>(); //calcular o tempo de vida tb
					model2 = curNode->GetObject<MobilityModel>(); //pega o modelo de mobilidade do servidor atual
					let = linkEstimatedLifeTime(model1,model2,curRange); // Tempo de vida do enlace; 'let'

					isOkToOffload = false;
					energyPerSecond = auxProvidersCpuConsumption[curNode]/3600.0; //energia gasta em 1s (em Watt)
					expectedCsmdEnergy = queueProcTime*energyPerSecond; //consumo de energia esperado c fila e novo processamento acumulado

					if(howLongTake <= let){       //(howLongTake <= procDeadline) &&
					//if(howLongTake <= auxProvidersLlt[curNode]){       //(howLongTake <= procDeadline) &&
						if((auxProvidersEnergyLevel[curNode] - expectedCsmdEnergy) > auxProvidersMinimalEnergy[curNode]){ //verifica energia
							isOkToOffload = true;
						}
					}
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
					//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

					if(isOkToOffload==true){
						tasksOffloadedSuc += 1;
						ntbp += 1;
						workloadMatrix.erase(workloadMatrix.begin() + m); //deletar tarefas q já estão sendo processadas
					}
					else {
						howMuchProc = howMuchProc - workloadMatrix[m].at(1);
						totalTaskSize = totalTaskSize - workloadMatrix[m].at(0);
						m += 1; //só incrementa se n tiver deletado o primeiro elemento da lista
					}
				} //fim do while das tasks
				if((howMuchProc > 0) && (totalTaskSize > 0)){ //se esse servidor vai receber algo p processar
					providers.push_back(sortAuxProviders[i].first);
					tempVec.clear();
					tempVec.push_back(totalTaskSize);
					tempVec.push_back(howMuchProc);
					tempVec.push_back(ntbp*1.0);
					scheduleMatrix.push_back(tempVec);
					numberOfSurrogates += 1;
					tempVec.clear();
				}
				/*std::cout << "[DECISION] CarServer: " << getIPFromServer(sortAuxProviders[i].first);
				os << "[DECISION] CarServer: " << getIPFromServer(sortAuxProviders[i].first);
				std::cout << ". cpuQueue: " << auxProvidersQueue[sortAuxProviders[i].first] << ". cpuCap: " << auxProvidersCap[sortAuxProviders[i].first]
					   << ". Processing: " << howMuchProc << "Gc. Transfering: " << totalTaskSize << "Bytes" << std::endl;
				os << ". cpuQueue: " << auxProvidersQueue[sortAuxProviders[i].first] << ". cpuCap: " << auxProvidersCap[sortAuxProviders[i].first]
						<< ". Processing: " << howMuchProc << "Gc. Transfering: " << totalTaskSize << "Bytes" << std::endl;*/
				m=0; //zera o contador
				howMuchProc = 0.0; //zera o quanto que tem de processar
				totalTaskSize = 0.0; //zera o total do tamanho do pacote
			}
		} //fim do for dos possíveis servidores
	}

	//o q sobrar de rebarba executa localmente
	if((workloadMatrix.size()>0) && (attempts==1)){
		gcfRemainder();
	}

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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

		printDecisionGcf();

		for(uint32_t i = 0; i < providers.size(); i++){
			Simulator::Schedule(Seconds(0.0),serverSide);
			Simulator::Schedule(Seconds(0.0),clientSide);
		}
		alreadySent = true;
		Simulator::Schedule(Seconds(1.0),checkConnectivity); //dá início as verificações periódicas de conectividade
	}
	else if ((alreadySent == false) && (workloadMatrix.size()>0)) { //vamos tentar de novo em 0.1s para dar tempo chegarem novos providers
		sortAuxProviders.clear(); //zera tudo
		providers.clear();
		scheduleMatrix.clear();
		workloadMatrix.clear();
		workloadMatrix = backupWorkloadMatrix; //resgatar workloadMatrix
		numberOfSurrogates = 0;
		tasksOnlyLocal=0;
		tasksOffloadedSuc=0;
		if(attempts < 1){ //tentar no máximo 2 vezes (0 e 1)
			Simulator::Schedule(Seconds(0.25),SortAndInitTransfer2);
			attempts += 1; //vai para a tentativa seguinte
		}
	}
}

void printDecisionGcf(){
	std::cout << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
	os << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
	std::cout << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
	   << ". Processing: " << howMuchProcLocalGcf << "Gc. Transfering: " << 0 <<
	   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
	os << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
	   << ". Processing: " << howMuchProcLocalGcf << "Gc. Transfering: " << 0 <<
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
		std::cout << ". cpuQueue: " << auxProvidersQueue[providers[i]] << ". cpuCap: " << auxProvidersCap[providers[i]]
		   << ". Processing: " << scheduleMatrix[i][1] << "Gc. Transfering: " << scheduleMatrix[i][0] <<
		   "Bytes. Distance: " << auxProvidersDist[providers[i]] << std::endl;
		os << ". cpuQueue: " << auxProvidersQueue[providers[i]] << ". cpuCap: " << auxProvidersCap[providers[i]]
		   << ". Processing: " << scheduleMatrix[i][1] << "Gc. Transfering: " << scheduleMatrix[i][0] <<
		   "Bytes. Distance: " << auxProvidersDist[providers[i]] << std::endl;
	}
}

void gcf2Decision (Ipv4Address ipv4From, uint32_t iface) {
	if(iface==1){ //se recebeu resposta pela interface 5G; sempre vai receber primeiro por aqui...
		for (size_t i = 0; i < edgeNodes.GetN(); i++) {
			if (ipv4From == edgeNodes.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //a borda só tem interface 5G

				enbNotFound = false; //enb foi encontrado
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = enbNodes.Get(i)->GetObject<MobilityModel>(); //pega o modelo de mobilidade do enb associado
				double distance = GetDistance(model1,model2);
				double let = linkEstimatedLifeTime(model1,model2,220.0); // Tempo de vida do enlace; 'let'
				std::cout << "[CLIENTE] GCF: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
						edgeNodeCPUCap[idxFromMoreClosestEnb] << " e CpuQueue ==> " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << std::endl;
				os << "[CLIENTE] GCF: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
						edgeNodeCPUCap[idxFromMoreClosestEnb] << " e CpuQueue ==> " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << std::endl;


				double procTime = calcSleepTime(edgeNodeCPUCap[idxFromMoreClosestEnb], edgeNodeCPUQueue[idxFromMoreClosestEnb],
						2*3); //3-cpuRequired médio; @TODO: melhorar isso aqui
				double transferTime = (2*1200000 + 1000)/(datarateMmWave*0.25) + 0.05; //1200000-taskSize médio; @TODO: melhorar isso aqui
				double possibleTime = transferTime + procTime;

				if(let >= possibleTime){
					if(alreadySent == false){ //enquanto ainda não foi enviado o workload, pode continuar adicionando possíveis providers
						if(alreadyDecided == false){ //se ainda n decidiu qtos mandar
							auxProvidersDist[edgeNodes.Get(i)] = distance;
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
				double let = linkEstimatedLifeTime(model1,model2,250.0); // Tempo de vida do enlace; 'let'
				std::cout << "[CLIENTE] GCF: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
						carNodeCPUCap[i] << " e CpuQueue ==> " << carNodeCPUQueue[i] << std::endl;
				os << "[CLIENTE] GCF: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
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
	//após 0.1s (tempo necessário para receber mais respostas dos substitutos), chama addProviders
	if((alreadySentToSort == false) && (alreadyDecided == false)){ //e o número de providers já é suficiente
		Simulator::Schedule(Seconds(0.25),SortAndInitTransfer2); //ou 0.05 ou 0.03
		alreadySentToSort = true;
	}
}
