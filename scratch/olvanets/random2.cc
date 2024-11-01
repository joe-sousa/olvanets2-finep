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

#include "scratch/olvanets/random2.h"

double procTimeFifo;
double energyPerSecondFifo; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
double expectedCsmdEnergyFifo; //calcular consumo de energia esperado
std::vector<double> tempVecFifo;
double howMuchProcLocalFifo; //final de quanto o cliente vai processar

//calcula o número de surrogates necessários e escalona o modo de transferir as tarefas
void surrogatesAndScheduleFifo(){
	numberOfSurrogates = numberOfSurrogates + workloadMatrix.size(); //cada surrogate vai ficar com 1 tarefa
	tempVecFifo.clear();
	for(uint32_t i=0; i< workloadMatrix.size();i++){
		tasksOffloadedSuc += 1; //número de tarefas offloadadas
		tempVecFifo.push_back(workloadMatrix[i].at(0)); //pkt size
		tempVecFifo.push_back(workloadMatrix[i].at(1)); //cpu required
		tempVecFifo.push_back(1.0); //qtd de tasks por carServer
		scheduleMatrix.push_back(tempVecFifo); //adiciona tempVec para scheduleMatrix
		tempVecFifo.clear(); //zera tempVec
	}
	workloadMatrix.clear(); //zero workloadMatrix
}

//calcula packetsize, localTime, numberOfSurrogates
void tasksToLocal(uint32_t clientId){
	double currentTime = Simulator::Now().GetSeconds ();
	double howMuchProc = 0.0; //quanto de processamento deverá ser feito localmente
	numberOfSurrogates = 0; //zera o número de surrogates

	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	startElapsedTime = std::chrono::high_resolution_clock::now();

	if(carNodeCPUQueue[clientId] <= 2){ //se o cliente tem um tempo de fila de 2s pra baixo //o cliente tenta processar duas tarefas localmente
		for(uint32_t i=0; i<2; i++){ //@TODO: caso o workload possua duas tarefas ou mais
			localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
			howMuchProc = howMuchProc + workloadMatrix[0].at(1); //acrescenta os tempos de processamento de cada tarefa

			//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
			procTimeFifo = calcSleepTime(auxProvidersCap[c.Get(clientId)], auxProvidersQueue[c.Get(clientId)], howMuchProc);
			energyPerSecondFifo = auxProvidersCpuConsumption[c.Get(clientId)]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
			expectedCsmdEnergyFifo = procTimeFifo*energyPerSecondFifo; //calcular consumo de energia esperado

			if((auxProvidersEnergyLevel[c.Get(clientId)] - expectedCsmdEnergyFifo) > auxProvidersMinimalEnergy[c.Get(clientId)]){
				tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
				workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já estão sendo processadas localmente
			}
			else{
				howMuchProc = howMuchProc - workloadMatrix[0].at(1);
				break;
			}
		}
		if(howMuchProc > 0.0){
			localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc);
		}
		surrogatesAndScheduleFifo();
	}
	if((carNodeCPUQueue[clientId] >= 3) && (carNodeCPUQueue[clientId] <= 6)){ //se o cliente tem um tempo de fila entre 3 e 6
		localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
		howMuchProc = howMuchProc + workloadMatrix[0].at(1); //acrescenta os tempos de processamento de cada tarefa

		//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
		procTimeFifo = calcSleepTime(auxProvidersCap[c.Get(clientId)], auxProvidersQueue[c.Get(clientId)], howMuchProc);
		energyPerSecondFifo = auxProvidersCpuConsumption[c.Get(clientId)]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
		expectedCsmdEnergyFifo = procTimeFifo*energyPerSecondFifo; //calcular consumo de energia esperado

		if((auxProvidersEnergyLevel[c.Get(clientId)] - expectedCsmdEnergyFifo) > auxProvidersMinimalEnergy[c.Get(clientId)]){
			//o cliente vai processar uma tarefa localmente
			tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
			workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já estão sendo processadas localmente
			localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc);
		}
		surrogatesAndScheduleFifo();
	}
	if(carNodeCPUQueue[clientId] >= 7){ //se o cliente tem um tempo de fila de 7s pra cima
		localTime = 0.0 //o carro não processa nenhuma tarefa localmente
				+ (currentTime - tempoIni);  //tempo até chegar aqui desde o initAction2
		surrogatesAndScheduleFifo();
	}
	howMuchProcLocalFifo = howMuchProc;
	/******************** MEDIÇÃO DE TEMPO ************************************/
	endElapsedTime = std::chrono::high_resolution_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
	elapsedTime *= 1e-9;
	std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
	os << "[ELAPSED TIME] " << elapsedTime << std::endl;
	realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
}

void printDecisionFifo(){
	std::cout << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
	os << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
	std::cout << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
	   << ". Processing: " << howMuchProcLocalFifo << "Gc. Transfering: " << 0 <<
	   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
	os << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
	   << ". Processing: " << howMuchProcLocalFifo << "Gc. Transfering: " << 0 <<
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

void random2Decision(Ipv4Address ipv4From, uint32_t iface) {
	if(providers.size() < numberOfSurrogates){
		if(iface==1){ //se recebeu resposta pela interface 5G
			for (size_t i = 0; i < edgeNodes.GetN(); i++) {
				if (ipv4From == edgeNodes.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //a borda só tem interface 5G
					std::cout << "[CLIENTE] Random2: O IP do substituto é ==> "<< ipv4From << std::endl;
					os << "[CLIENTE] Random2: O IP do substituto é ==> "<< ipv4From << std::endl;
					providers.push_back(edgeNodes.Get(i));

					Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
					Ptr<MobilityModel> model2 = enbNodes.Get(i)->GetObject<MobilityModel>(); //pega o modelo de mobilidade do enb associado
					double distance = GetDistance(model1,model2);
					auxProvidersDist[edgeNodes.Get(i)] = distance;
					auxProvidersQueue[edgeNodes.Get(i)] = edgeNodeCPUQueue[idxFromMoreClosestEnb];
					auxProvidersCap[edgeNodes.Get(i)] = edgeNodeCPUCap[idxFromMoreClosestEnb];
					auxProvidersCpuConsumption[edgeNodes.Get(i)] = edgeCpuConsumption[idxFromMoreClosestEnb]; //parâmetros de energia
					auxProvidersMinimalEnergy[edgeNodes.Get(i)] = -999.0;
					auxProvidersEnergyLevel[edgeNodes.Get(i)] = 999999;

					break;
				}
			}
		}
		if(iface==2){
			for (size_t i = 0; i < c.GetN(); i++) { //@TODO: como sei que o surrogates tem alguma coisa?
				if (ipv4From == c.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //se o surrogate escolhido respondeu
					//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
					procTimeFifo = calcSleepTime(carNodeCPUCap[i], carNodeCPUQueue[i], 3); //3-cpuRequired médio
					energyPerSecondFifo = carCpuConsumption[i]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour??????)
					expectedCsmdEnergyFifo = procTimeFifo*energyPerSecondFifo; //calcular consumo de energia esperado

					//if((carEnergyLevel[i] - expectedCsmdEnergyFifo) > carMinimalEnergy[i]){ //se tem energia
						std::cout << "[CLIENTE] Random2: O IP do substituto é ==> "<< ipv4From << std::endl;
						os << "[CLIENTE] Random2: O IP do substituto é ==> "<< ipv4From << std::endl;
						providers.push_back(c.Get(i));

						Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
						Ptr<MobilityModel> model2 = c.Get(i)->GetObject<MobilityModel>();
						double distance = GetDistance(model1,model2);
						auxProvidersQueue[c.Get(i)] = carNodeCPUQueue[i];
						auxProvidersCap[c.Get(i)] = carNodeCPUCap[i];
						auxProvidersDist[c.Get(i)] = distance;
						auxProvidersCpuConsumption[c.Get(i)] = carCpuConsumption[i]; //parâmetros de energia
						auxProvidersMinimalEnergy[c.Get(i)] = carMinimalEnergy[i];
						auxProvidersEnergyLevel[c.Get(i)] = carEnergyLevel[i];
					//}
						if((carEnergyLevel[i] - expectedCsmdEnergyFifo) < carMinimalEnergy[i]){ //podem haver violações
							numberOfEnergyViolations += 1;
						}
					break;
				}
			}
		}
	}
	if ((providers.size() == numberOfSurrogates) && (alreadySent == false)) {
		std::cout << "[CLIENTE] Recebeu respostas suficientes! Dando prosseguimento =)" << std::endl;
		os << "[CLIENTE] Recebeu respostas suficientes! Dando prosseguimento =)" << std::endl;
		printDecisionFifo();
		for(uint32_t i = 0; i < providers.size(); i++){
			Simulator::Schedule(Seconds(0.0),serverSide);
			Simulator::Schedule(Seconds(0.0),clientSide);
		}
		alreadySent = true;
		Simulator::Schedule(Seconds(1.0),checkConnectivity); //dá início as verificações periódicas de conectividade
	} else if (providers.size() < numberOfSurrogates) {
		std::cout << "[CLIENTE] Não recebeu respostas suficientes!" << std::endl;
		os << "[CLIENTE] Não recebeu respostas suficientes!" << std::endl;
		//Simulator::Schedule(Seconds(1.0),&checkOffloadingSuccess, false); //se n tiver encontrado ngm em 1s, é para finalizar
	}
}
