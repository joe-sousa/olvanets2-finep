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

#include "scratch/olvanets/hvc.h"

double procTimeHvc;
double energyPerSecondHvc; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
double expectedCsmdEnergyHvc; //calcular consumo de energia esperado
std::vector<double> tempVecHvc;
uint32_t howManyTasksToEdgeHvc;
double howMuchProcLocalHvc; //final de quanto o cliente vai processar
uint32_t idxProvidersHvc = 0;
bool printedLocalHvc = false; //só p dizer se já imprimiu a decisão do cliente

//calcula o número de surrogates necessários e escalona o modo de transferir as tarefas
void surrogatesAndScheduleHvc(){
	numberOfSurrogates = numberOfSurrogates + workloadMatrix.size(); //cada surrogate vai ficar com 1 tarefa
	tempVecHvc.clear();
	for(uint32_t i=0; i< workloadMatrix.size();i++){
		tasksOffloadedSuc += 1; //número de tarefas offloadadas
		tempVecHvc.push_back(workloadMatrix[i].at(0)); //pkt size
		tempVecHvc.push_back(workloadMatrix[i].at(1)); //cpu required
		tempVecHvc.push_back(1.0); //qtd de tasks por carServer
		scheduleMatrix.push_back(tempVecHvc); //adiciona tempVec para scheduleMatrix
		tempVecHvc.clear(); //zera tempVec
	}
	workloadMatrix.clear(); //zero workloadMatrix
}

//localTime, numberOfSurrogates
void tasksToLocalHvc(uint32_t clientId){
	double currentTime = Simulator::Now().GetSeconds ();
	double howMuchProc = 0.0; //quanto de processamento deverá ser feito localmente
	numberOfSurrogates = 0; //zera o número de surrogates

	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;

	startElapsedTime = std::chrono::high_resolution_clock::now();

	if(enbNotFound == true){ //se não foi encontrado eNB
		if(carNodeCPUQueue[clientId] <= 6){ //se o tempo de fila no carro é de 6 para baixo
			localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
			howMuchProc = howMuchProc + workloadMatrix[0].at(1); //acrescenta os tempos de processamento de cada tarefa

			//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
			procTimeHvc = calcSleepTime(auxProvidersCap[c.Get(clientId)], auxProvidersQueue[c.Get(clientId)], howMuchProc);
			energyPerSecondHvc = auxProvidersCpuConsumption[c.Get(clientId)]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
			expectedCsmdEnergyHvc = procTimeHvc*energyPerSecondHvc; //calcular consumo de energia esperado

			if((auxProvidersEnergyLevel[c.Get(clientId)] - expectedCsmdEnergyHvc) > auxProvidersMinimalEnergy[c.Get(clientId)]){
				//o cliente vai processar uma tarefa localmente
				tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
				workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já estão sendo processadas localmente
				localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc);
			}
			else{
				howMuchProc = howMuchProc - workloadMatrix[0].at(1);
			}
			howMuchProcLocalHvc = howMuchProc; //contabilizar p impressão da decisão
			surrogatesAndScheduleHvc(); //uma tarefa para cada carro servidor diferente
			/******************** MEDIÇÃO DE TEMPO ************************************/
			endElapsedTime = std::chrono::high_resolution_clock::now();
			elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
			elapsedTime *= 1e-9;
			std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
			os << "[ELAPSED TIME] " << elapsedTime << std::endl;
			realTime = std::time(nullptr);
			std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
			os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
			return;
		}
		if(carNodeCPUQueue[clientId] >= 7){ //se o tempo de fila no carro é de 7 para cima
			//o cliente não processa tarefas... vai tudo para servidores remotos
			howMuchProcLocalHvc = 0.0;
			localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
			surrogatesAndScheduleHvc(); //uma tarefa para cada carro servidor diferente
			/******************** MEDIÇÃO DE TEMPO ************************************/
			endElapsedTime = std::chrono::high_resolution_clock::now();
			elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
			elapsedTime *= 1e-9;
			std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
			os << "[ELAPSED TIME] " << elapsedTime << std::endl;
			realTime = std::time(nullptr);
			std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
			os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
			return;
		}
	}
	if(enbNotFound == false){ //se foi encontrado algum eNB       	EDGE N TEM RESTRIÇÃO DE ENERGIA
		tempVecHvc.clear();
		tempVecHvc.push_back(0.0); tempVecHvc.push_back(0.0);
		if(edgeNodeCPUQueue[idxFromMoreClosestEnb] <= 2){ //se a borda está com tempo de fila de 2 para baixo
			localTime = 0 //o carro não processa nada; envia tudo para a borda
					+ (currentTime - tempoIni);  //tempo até chegar aqui desde o initAction2
			//todas as tarefas vão para a borda
			//agora informar o que será processado pelo edge server
			howManyTasksToEdgeHvc = workloadMatrix.size();
			if(howManyTasksToEdgeHvc>8){
				howManyTasksToEdgeHvc = 8; //limita a enviar 8 tarefas para a borda
			}
			for(uint32_t i=0; i< howManyTasksToEdgeHvc;i++){
				tasksOffloadedSuc += 1; //número de tarefas offloadadas
				tempVecHvc[0] = tempVecHvc[0] + workloadMatrix[i].at(0); //pkt size para a borda
				tempVecHvc[1] = tempVecHvc[1] + workloadMatrix[i].at(1); //cpu required p a borda
			}
			tempVecHvc.push_back(howManyTasksToEdgeHvc*1.0); //qtd de tasks p a edge
			scheduleMatrix.push_back(tempVecHvc); //para o edge já está escalonado
			numberOfSurrogates = 1; //todas as tasks, por enqto, vão para a borda
			//agora deletar as tarefas que já foram escalonadas para o edge
			for(uint32_t i=0; i< howManyTasksToEdgeHvc;i++){
				workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já serão processadas pelo edge server
			}
			if(workloadMatrix.size()>0){ //se ainda restarem tarefas
				if(carNodeCPUQueue[clientId] <= 6){ //se o tempo de fila no cliente é de 6 para baixo
					localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
					howMuchProc = howMuchProc + workloadMatrix[0].at(1); //acrescenta os tempos de processamento de cada tarefa

					//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
					procTimeHvc = calcSleepTime(auxProvidersCap[c.Get(clientId)], auxProvidersQueue[c.Get(clientId)], howMuchProc);
					energyPerSecondHvc = auxProvidersCpuConsumption[c.Get(clientId)]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
					expectedCsmdEnergyHvc = procTimeHvc*energyPerSecondHvc; //calcular consumo de energia esperado

					if((auxProvidersEnergyLevel[c.Get(clientId)] - expectedCsmdEnergyHvc) > auxProvidersMinimalEnergy[c.Get(clientId)]){
						//o cliente vai processar uma tarefa localmente
						tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
						workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já estão sendo processadas localmente
						localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc);
					}
					else{
						howMuchProc = howMuchProc - workloadMatrix[0].at(1);
					}
				}
				howMuchProcLocalHvc = howMuchProc;
				surrogatesAndScheduleHvc(); //aloca o restante das tarefas para outros servidores
			}
			/******************** MEDIÇÃO DE TEMPO ************************************/
			endElapsedTime = std::chrono::high_resolution_clock::now();
			elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
			elapsedTime *= 1e-9;
			std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
			os << "[ELAPSED TIME] " << elapsedTime << std::endl;
			realTime = std::time(nullptr);
			std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
			os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
			return;
		}
		if((edgeNodeCPUQueue[idxFromMoreClosestEnb] >= 3) && (edgeNodeCPUQueue[idxFromMoreClosestEnb] <= 6)){ //edge: tempo de fila entre [3,6]
			if(carNodeCPUQueue[clientId] <= 6){ //se o tempo de fila no carro é de 6 para baixo
				localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
				howMuchProc = howMuchProc + workloadMatrix[0].at(1); //acrescenta os tempos de processamento de cada tarefa

				//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
				procTimeHvc = calcSleepTime(auxProvidersCap[c.Get(clientId)], auxProvidersQueue[c.Get(clientId)], howMuchProc);
				energyPerSecondHvc = auxProvidersCpuConsumption[c.Get(clientId)]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
				expectedCsmdEnergyHvc = procTimeHvc*energyPerSecondHvc; //calcular consumo de energia esperado

				if((auxProvidersEnergyLevel[c.Get(clientId)] - expectedCsmdEnergyHvc) > auxProvidersMinimalEnergy[c.Get(clientId)]){
					//o cliente vai processar uma tarefa localmente
					tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
					workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já estão sendo processadas localmente
					localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc);
				}
				else{
					howMuchProc = howMuchProc - workloadMatrix[0].at(1); //acrescenta os tempos de processamento de cada tarefa
				}
				howMuchProcLocalHvc = howMuchProc; //contabilizar qtd de processamento do cliente

				//metade das tarefas que sobraram vão para a borda e a outra metade vai um para cada carro servidor
				//agora informar o que será processado pelo edge server
				howManyTasksToEdgeHvc = ceil(workloadMatrix.size()/2);
				for(uint32_t i=0; i< howManyTasksToEdgeHvc;i++){
					tasksOffloadedSuc += 1; //número de tarefas offloadadas
					tempVecHvc[0] = tempVecHvc[0] + workloadMatrix[i].at(0); //pkt size para a borda
					tempVecHvc[1] = tempVecHvc[1] + workloadMatrix[i].at(1); //cpu required p a borda
				}
				tempVecHvc.push_back(howManyTasksToEdgeHvc*1.0);
				scheduleMatrix.push_back(tempVecHvc); //para o edge já está escalonado
				//agora deletar as tarefas que já foram escalonadas para o edge
				for(uint32_t i=0; i< howManyTasksToEdgeHvc;i++){
					workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já serão processadas pelo edge server
				}
				numberOfSurrogates = 1; //por enqto é 1 pq só tem o edge de surrogate
				surrogatesAndScheduleHvc(); //aloca o restante das tarefas para outros servidores
				/******************** MEDIÇÃO DE TEMPO ************************************/
				endElapsedTime = std::chrono::high_resolution_clock::now();
				elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
				elapsedTime *= 1e-9;
				std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
				os << "[ELAPSED TIME] " << elapsedTime << std::endl;
				realTime = std::time(nullptr);
				std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
				os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
				return;
			}
			if(carNodeCPUQueue[clientId] >= 7){ //se o tempo de fila no cliente é de 7 para cima
				localTime = 0
						+ (currentTime - tempoIni);  //tempo até chegar aqui desde o initAction2
				howMuchProcLocalHvc = 0.0; //contabilizando processamento do cliente
				//metade das tarefas vão para a borda e a outra metade vai um para cada carro servidor
				numberOfSurrogates = workloadMatrix.size() - ceil(workloadMatrix.size()/2) + 1; //mais 1 é a borda
				//agora informar o que será processado pelo edge server
				howManyTasksToEdgeHvc = ceil(workloadMatrix.size()/2);
				for(uint32_t i=0; i< howManyTasksToEdgeHvc;i++){
					tasksOffloadedSuc += 1; //número de tarefas offloadadas
					tempVecHvc[0] = tempVecHvc[0] + workloadMatrix[i].at(0);
					tempVecHvc[1] = tempVecHvc[1] + workloadMatrix[i].at(1);
				}
				tempVecHvc.push_back(howManyTasksToEdgeHvc*1.0);
				scheduleMatrix.push_back(tempVecHvc); //para o edge já está escalonado
				//agora deletar as tarefas que já foram designadas para o edge
				for(uint32_t i=0; i< howManyTasksToEdgeHvc;i++){
					workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já serão processadas pelo edge server
				}
				numberOfSurrogates = 1; //por enqto é 1 pq só tem o edge de surrogate
				surrogatesAndScheduleHvc(); //aloca o restante das tarefas para outros servidores
				/******************** MEDIÇÃO DE TEMPO ************************************/
				endElapsedTime = std::chrono::high_resolution_clock::now();
				elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
				elapsedTime *= 1e-9;
				std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
				os << "[ELAPSED TIME] " << elapsedTime << std::endl;
				realTime = std::time(nullptr);
				std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
				os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
				return;
			}
		}
		if(edgeNodeCPUQueue[idxFromMoreClosestEnb] >= 7){ //se a borda está com tempo de fila de 7 para cima
			if(carNodeCPUQueue[clientId] <= 6){ //se o tempo de fila no cliente é de 6 para baixo
				localTime = currentTime - tempoIni; //tempo até chegar aqui desde o initAction2
				howMuchProc = howMuchProc + workloadMatrix[0].at(1); //acrescenta os tempos de processamento de cada tarefa

				//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
				procTimeHvc = calcSleepTime(auxProvidersCap[c.Get(clientId)], auxProvidersQueue[c.Get(clientId)], howMuchProc);
				energyPerSecondHvc = auxProvidersCpuConsumption[c.Get(clientId)]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
				expectedCsmdEnergyHvc = procTimeHvc*energyPerSecondHvc; //calcular consumo de energia esperado

				if((auxProvidersEnergyLevel[c.Get(clientId)] - expectedCsmdEnergyHvc) > auxProvidersMinimalEnergy[c.Get(clientId)]){
					//o cliente vai processar uma tarefa localmente
					tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
					workloadMatrix.erase(workloadMatrix.begin() + 0); //deletar tarefas q já estão sendo processadas localmente
					localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], howMuchProc);
				}
				else{
					howMuchProc = howMuchProc - workloadMatrix[0].at(1);
				}
				howMuchProcLocalHvc = howMuchProc;
				//a borda não processa; cada tarefa restante vai um para cada carro servidor
				surrogatesAndScheduleHvc(); //uma tarefa para cada carro servidor diferente
				/******************** MEDIÇÃO DE TEMPO ************************************/
				endElapsedTime = std::chrono::high_resolution_clock::now();
				elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
				elapsedTime *= 1e-9;
				std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
				os << "[ELAPSED TIME] " << elapsedTime << std::endl;
				realTime = std::time(nullptr);
				std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
				os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
				return;
			}
			if(carNodeCPUQueue[clientId] >= 7){ //se o carro está com tempo de fila de 7 para cima
				localTime = 0.0 //o cliente não processa
						+ (currentTime - tempoIni);  //tempo até chegar aqui desde o initAction2
				howMuchProcLocalHvc = 0.0;
				//a borda não processa; cada tarefa restante vai um para cada carro servidor
				surrogatesAndScheduleHvc(); //uma tarefa para cada carro servidor diferente
				/******************** MEDIÇÃO DE TEMPO ************************************/
				endElapsedTime = std::chrono::high_resolution_clock::now();
				elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
				elapsedTime *= 1e-9;
				std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
				os << "[ELAPSED TIME] " << elapsedTime << std::endl;
				realTime = std::time(nullptr);
				std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
				os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
				return;
			}
		}
	}
}

void printDecisionHvc(){
	if(printedLocalHvc == false){
		std::cout << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		os << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
		std::cout << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalHvc << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
		os << ". cpuQueue: " << auxProvidersQueue[c.Get(clientId)] << ". cpuCap: " << auxProvidersCap[c.Get(clientId)]
		   << ". Processing: " << howMuchProcLocalHvc << "Gc. Transfering: " << 0 <<
		   "Bytes. Distance: " << auxProvidersDist[c.Get(clientId)] << std::endl;
		printedLocalHvc = true;
	}

	for(uint32_t i=idxProvidersHvc; i<providers.size(); i++){
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

//função que, qdo estoura o tempo, executa o resto das tarefas desalocadas localmente
void hvcTimeoutToFind(){
	if(hvcTimeout == false){
		double currentTime = Simulator::Now().GetSeconds ();
		hvcTimeout = true; //já estourou o tempo; depois disso, n adicionar mais providers
		int nrtasks = 0; //tasks n alocadas p serem executadas no cliente
		if(idxOfProvidersActionedInClient<numberOfSurrogates){
			numberOfSurrogates = idxOfProvidersActionedInClient; //ficam de surrogates só os q já estão
			//executar o restante de scheduleMatrix localmente
			double whatProcess = 0.0;
			bool hasBreak = false; //diz se caiu no break
			uint32_t iCount;
			for(uint32_t i = idxOfProvidersActionedInClient; i < scheduleMatrix.size(); i++){
				iCount = i;
				whatProcess = whatProcess + scheduleMatrix[i].at(1);

				double queueProcTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], whatProcess);
				energyPerSecondHvc = auxProvidersCpuConsumption[c.Get(clientId)]/3600.0; //energia gasta em 1s (em Watt)
				expectedCsmdEnergyHvc = queueProcTime*energyPerSecondHvc; //consumo de energia esperado c fila e novo processamento acumulado
				if((auxProvidersEnergyLevel[c.Get(clientId)] - expectedCsmdEnergyHvc) > auxProvidersMinimalEnergy[c.Get(clientId)]){ //verifica energia
					nrtasks += 1; //incrementa em 1
				}
				else{
					hasBreak = true;
					break; //sair do laço while
				}
			}
			if(hasBreak == true){
				whatProcess = whatProcess - scheduleMatrix[iCount].at(1); //deleta a última task a ser processada localmente pq o cliente n aguentou
			}
			if(whatProcess <= 0.0){
				return;
			}
			//double numberOfRemainingTasks = whatProcess/wAvgCpuReq; //@TODO: atenção aqui se forem tarefas de tamanhos diferentes
			//int nrtasks = (int)numberOfRemainingTasks;
			howMuchProcLocalHvc = howMuchProcLocalHvc + whatProcess; //contabilizando o q vai ser processado pelo cliente
			printedLocalHvc = false; //p imprimir a reconsideração de processamento do clientes
			printDecisionHvc();
			tasksOnlyLocal = tasksOnlyLocal + nrtasks; //outras tarefas serão executadas localmente desde o início
			tasksOffloadedSuc = tasksOffloadedSuc - nrtasks; //menos serão offloadadas
			//tasksOffloadedSuc = tasksOffloadedSuc - (numberOfSurrogates - idxOfProvidersActionedInClient); //menos serão offloadadas
			//executa as tarefas desalocadas localmente
			if(localTime < 1.0){ //se nenhuma tarefa foi alocada aqui neste programa para o local do cliente
					double sleepLocalTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], whatProcess);
					localTime = (currentTime - tempoIni) + sleepLocalTime;
			} else { //alguma tarefa foi alocada neste programa para o local do cliente
					if(currentTime > (localTime + tempoIni)){ //se já executou a tarefa alocada
						double sleepLocalTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], whatProcess);
						localTime = currentTime + sleepLocalTime;
					} else { //se não executou a tarefa alocada
						double sleepLocalTime = calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], whatProcess);
						localTime = localTime + sleepLocalTime;
					}
			}
		}
	}
}

void hvcDecisionNoMmWave(){
	if((idxOfProvidersActionedInClient == 0) && (alreadyDecided == false)){ //não enviou nada ainda pra torre
		enbNotFound = true; //enb não encontrado
		//pktHvc(oneImage, clientId); //envia tudo por carro
		tasksToLocalHvc(clientId); //envia tudo por carro
		alreadyDecided = true;
		Simulator::Schedule(Seconds(0.0),clientRequest,wReqBcSock); //cliente envia pacote em broadcast para descoberta
	}
}

void hvcDecision(Ipv4Address ipv4From, uint32_t iface){
	if(hvcTimeout == false){
		Simulator::Schedule(Seconds(0.5),hvcTimeoutToFind); //após 0.5s estoura o timeout
	}
	if(iface==1){ //se recebeu resposta pela interface 5G; sempre vai receber primeiro por aqui...
		for (size_t i = 0; i < edgeNodes.GetN(); i++) {
			if (ipv4From == edgeNodes.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //a borda só tem interface 5G
				enbNotFound = false; //enb foi encontrado
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = enbNodes.Get(i)->GetObject<MobilityModel>(); //pega o modelo de mobilidade do enb associado
				double distance = GetDistance(model1,model2);
				double let = linkEstimatedLifeTime(model1,model2,220.0); // Tempo de vida do enlace; 'let'
				std::cout << "[CLIENTE] HVC: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let <<  ". CpuCap ==> "
						<< edgeNodeCPUCap[idxFromMoreClosestEnb] << " e CpuQueue ==> " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << std::endl;
				os << "[CLIENTE] HVC: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let <<  ". CpuCap ==> "
						<< edgeNodeCPUCap[idxFromMoreClosestEnb] << " e CpuQueue ==> " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << std::endl;


				//uint32_t possibleSleepTime = sleepTimeCloudnet2(4*oneImage, 0, 1); //par: tamanho do pkt rcvd, cpuBusy do server, iface 1 (5g)
				double possibleSleepTime = calcSleepTime(edgeNodeCPUCap[idxFromMoreClosestEnb], edgeNodeCPUQueue[idxFromMoreClosestEnb],
						4*3); //3-cpuRequired médio; par: edgeCap, edgeQueue, 4*firstTask-requiredCpu
				//dou um desconto p ficar apenas 25% do data rate e imagina que enviará as 4 imagens
				//em Bytes; 2 é por conta de ser upload e download
				double possibleTime = ((4*1200000)/(datarateMmWave*0.25)) + possibleSleepTime+0.05; //1200000-taskSize médio; 4*4*firstTask-pktSize
				//se a borda tiver tempo de fila de 2 ou menos e o link lifetime der certo
				if((edgeNodeCPUQueue[idxFromMoreClosestEnb]<=2) && (let >= possibleTime)){
					if((alreadyDecided == false) && (hvcTimeout == false)){ //se ainda n decidiu qtos mandar
						providers.push_back(edgeNodes.Get(i));

						auxProvidersDist[edgeNodes.Get(i)] = distance;
						auxProvidersQueue[edgeNodes.Get(i)] = edgeNodeCPUQueue[idxFromMoreClosestEnb];
						auxProvidersCap[edgeNodes.Get(i)] = edgeNodeCPUCap[idxFromMoreClosestEnb];
						auxProvidersCpuConsumption[edgeNodes.Get(i)] = edgeCpuConsumption[idxFromMoreClosestEnb]; //parâmetros de energia
						auxProvidersMinimalEnergy[edgeNodes.Get(i)] = -999.0;
						auxProvidersEnergyLevel[edgeNodes.Get(i)] = 999999;

						tasksToLocalHvc(clientId); //configura para enviar logo tudo para a borda
						alreadyDecided = true;
						printDecisionHvc();
						idxProvidersHvc++; //incrementa o índice q monitora os providers p imprimi-los
						Simulator::Schedule(Seconds(0.0),serverSide); //já envia para a borda executar
						Simulator::Schedule(Seconds(0.0),clientSide); //já envia para a borda executar
						alreadySent = true;
						if(alreadyStartedCheckConnectivity == false){
							alreadyStartedCheckConnectivity = true;
							Simulator::Schedule(Seconds(1.0),checkConnectivity); //dá início as verificações periódicas de conectividade
						}
					}
				}

				//possibleSleepTime = sleepTimeCloudnet2(2*oneImage, 50, 1); //par: tamanho do pkt rcvd, cpuBusy do server, iface 1 (5g)
				possibleSleepTime = calcSleepTime(edgeNodeCPUCap[idxFromMoreClosestEnb], edgeNodeCPUQueue[idxFromMoreClosestEnb],
						2*3); //3-cpuRequired médio; par: edgeCap, edgeQueue, 2*firstTask-requiredCpu
				possibleTime = ((2*1200000)/(datarateMmWave*0.25)) + possibleSleepTime+0.05; //1200000-taskSize médio; imagina que enviará apenas 2 tasks
				//a borda está como tempo de fila entre [3,6] e o link lifetime der certo
				if( ((edgeNodeCPUQueue[idxFromMoreClosestEnb]>=3) && (edgeNodeCPUQueue[idxFromMoreClosestEnb]<=6)) && (let >= possibleTime)){
					if((alreadyDecided == false) && (hvcTimeout == false)){ //se ainda n decidiu qtos mandar
						providers.push_back(edgeNodes.Get(i));

						auxProvidersDist[edgeNodes.Get(i)] = distance;
						auxProvidersQueue[edgeNodes.Get(i)] = edgeNodeCPUQueue[idxFromMoreClosestEnb];
						auxProvidersCap[edgeNodes.Get(i)] = edgeNodeCPUCap[idxFromMoreClosestEnb];
						auxProvidersCpuConsumption[edgeNodes.Get(i)] = edgeCpuConsumption[idxFromMoreClosestEnb]; //parâmetros de energia
						auxProvidersMinimalEnergy[edgeNodes.Get(i)] = -999.0;
						auxProvidersEnergyLevel[edgeNodes.Get(i)] = 999999;

						tasksToLocalHvc(clientId);
						alreadyDecided = true;
						printDecisionHvc();
						idxProvidersHvc++; //incrementa o índice q monitora os providers p imprimi-los
						Simulator::Schedule(Seconds(0.0),serverSide); //já envia para a borda executar
						Simulator::Schedule(Seconds(0.0),clientSide); //já envia para a borda executar
						Simulator::Schedule(Seconds(0.0),clientRequest,wReqBcSock); //inicia req/rep wave
						if(alreadyStartedCheckConnectivity == false){
							alreadyStartedCheckConnectivity = true;
							Simulator::Schedule(Seconds(1.0),checkConnectivity); //dá início as verificações periódicas de conectividade
						}
					}
				}

				//se a borda estiver com tempo de fila de 7 para cima ou linklifetime n der certo
				if((let < possibleTime) || (edgeNodeCPUQueue[idxFromMoreClosestEnb]>=7)) {
					if(hvcTimeout == false){
						edgeNodeCPUQueue[idxFromMoreClosestEnb]=7; //forçar p ele n processar nada
						enbNotFound = true; //finge que o enb n foi encontrado... pq nem adianta já q tem muita espera na fila da borda
						tasksToLocalHvc(clientId);
						alreadyDecided = true;
						printDecisionHvc();
						idxProvidersHvc++; //incrementa o índice q monitora os providers p imprimi-los
						Simulator::Schedule(Seconds(0.0),clientRequest,wReqBcSock); //inicia req/rep wave
					}
				}

				break;
			}
		}
	}
	if(iface==2){ //se recebeu resposta pela interface WAVE
		for (size_t i = 0; i < c.GetN(); i++) { //@TODO: como sei que o surrogates tem alguma coisa?
			if (ipv4From == c.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //se o surrogate escolhido respondeu
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = c.Get(i)->GetObject<MobilityModel>(); //pega o modelo de mobilidade do outro carro
				double distance = GetDistance(model1,model2);
				double let = linkEstimatedLifeTime(model1,model2,250.0); // Tempo de vida do enlace; 'let'
				std::cout << "[CLIENTE] HVC: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let <<
						". CpuCap ==> " << carNodeCPUCap[i] << " e CpuQueue ==> " << carNodeCPUQueue[i] << std::endl;
				os << "[CLIENTE] HVC: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let <<
						". CpuCap ==> " << carNodeCPUCap[i] << " e CpuQueue ==> " << carNodeCPUQueue[i] << std::endl;

				//uint32_t possibleSleepTime = sleepTimeCloudnet2(oneImage, 0, 1); //par: tamanho do pkt rcvd, cpuBusy do server, iface 1 (5g)
				double possibleSleepTime = calcSleepTime(carNodeCPUCap[i], carNodeCPUQueue[i],
						3); //3-cpuRequired médio; par:carCap,carQueue,reqCpu
				//dou um desconto p ficar apenas 25% do data rate e imagina que enviará uma imagem
				//double possibleTime = oneImage/(datarateMmWave*0.25) + possibleSleepTime+0.05; //em Bytes; 2 é por conta de ser upload e download
				double possibleTime = 1200000/(datarateMmWave*0.25) + possibleSleepTime+0.05; //1200000-taskSize médio; em Bytes; 2 é por conta de ser upload e download

				if(hvcTimeout == false){ //se ainda n tiver estourado o tempo
					//se o carro estiver com tempo de fila abaixo de 7 e o link lifetime der certo
					if((carNodeCPUQueue[i] < 7) && (let >= possibleTime)){
						if(providers.size() < numberOfSurrogates){
							//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
							procTimeHvc = calcSleepTime(carNodeCPUCap[i], carNodeCPUQueue[i], 3); //3-cpuRequired médio
							energyPerSecondHvc = carCpuConsumption[i]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour??????)
							expectedCsmdEnergyHvc = procTimeHvc*energyPerSecondHvc; //calcular consumo de energia esperado

							//if((carEnergyLevel[i] - expectedCsmdEnergyHvc) > carMinimalEnergy[i]){ //se tem energia
								providers.push_back(c.Get(i));

								auxProvidersQueue[c.Get(i)] = carNodeCPUQueue[i];
								auxProvidersCap[c.Get(i)] = carNodeCPUCap[i];
								auxProvidersDist[c.Get(i)] = distance;
								auxProvidersCpuConsumption[c.Get(i)] = carCpuConsumption[i]; //parâmetros de energia
								auxProvidersMinimalEnergy[c.Get(i)] = carMinimalEnergy[i];
								auxProvidersEnergyLevel[c.Get(i)] = carEnergyLevel[i];

								printDecisionHvc();
								idxProvidersHvc++; //incrementa o índice q monitora os providers p imprimi-los
								Simulator::Schedule(Seconds(0.0),serverSide); //já envia para o outro carro executar
								Simulator::Schedule(Seconds(0.0),clientSide); //já envia para o outro carro executar
								if(alreadyStartedCheckConnectivity == false){
									alreadyStartedCheckConnectivity = true;
									Simulator::Schedule(Seconds(1.0),checkConnectivity); //dá início as verificações periódicas de conectividade
								}
							//}
								if((carEnergyLevel[i] - expectedCsmdEnergyHvc) < carMinimalEnergy[i]){ //podem haver violações
									numberOfEnergyViolations += 1;
								}
						}
						if(providers.size() >= numberOfSurrogates) { //é porque já adicionou e enviou pra todos
							alreadySent = true; //já enviou pra todos
						}
					}
				}
				break;
			}
		}
	}
}
