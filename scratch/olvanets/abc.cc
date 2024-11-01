/*
*  Alisson Barbosa, Fevereiro/2021. E-mail: alisson@ufc.br
*  This code realizes the packets exchange between vehicles (V2V) and the edge (V2I), simulating
*  the send of subtasks to be executed in the surrogate vehicles and in the edge of the network.
*
*  This code is protected by copyright and intellectual property right laws.
*  Do not use or distribute without the author's authorization.
*  This is true even for modified versions or snippets of this code.
*
*/

#include "scratch/olvanets/abc.h"
//#include <stdlib.h>

class Solution {
public:
	double fitness; //fitness da solução
	int cycles; //duração de ciclos dessa solução
	double maxTime; //tempo máximo de execução da solução

	std::vector< Ptr<Node>> nodes; //vetor p armazenar apenas a solucao atual
	std::vector< double> execTimes; //vetor p armazenar a qtd de tempo de processamento esperado
	std::vector< double> howMuchProc; //vetor p armazenar a qtd de processamento p esse server
	std::vector< double> transTimes; //vetor p armazenar a qtd de tempo de transmissao esperado
	std::vector< double> howManyBytes; //vetor p armazenar a qtd de bytes q esse server irá receber
	std::vector< double> execEnergies; //vetor p armazenar a qtd de energia gasta esperada no processamento
	std::vector< double> transEnergies; //vetor p armazenar a qtd de energia gasta esperada na transmissão

	//maps p acumular qtds por nós
	std::map <Ptr<Node> ,double> abcHowMuchProcServer; //node / quanto o nó deve processar; temporario p o algoritmo abc
	std::map <Ptr<Node> ,double> abcHowMuchDataForServer; //node / qual o tamanho do pacote q deve receber; temporario p o algoritmo abc
	//std::map <Ptr<Node> ,double> abcHowManyTasksForServer; //node / quantas tasks deve receber; temporario p o algoritmo abc
	std::map< Ptr<Node> ,double> abcConsumedEnergyPerNode; //qtd de energia consumida por cada nó
	std::map< Ptr<Node> ,double> expectedTransTimePerNode; //qtd de tempo de transmissao esperado para cada nó
	std::map< Ptr<Node> ,double> expectedProcTimePerNode; //qtd de tempo de processamento esperado para cada nó

	void clear(){
		fitness = 0.0;
		cycles = 0.0; //seta q a solução ainda não passou por nenhum ciclo
		maxTime = 0.0;
		nodes.clear(); //limpar a sequencia de nós da solucao atual
		execTimes.clear(); //limpa os tempos esperados da solução
		howMuchProc.clear();
		transTimes.clear(); //limpa os tempos esperados da solução
		howManyBytes.clear();
		execEnergies.clear(); //limpa as energias gastas no processamento da solução
		transEnergies.clear(); //limpa as energias gastas pelo cliente na transmissao das tasks da soluçãos
		abcHowMuchProcServer.clear();
		abcHowMuchDataForServer.clear();
		//abcHowManyTasksForServer.clear();
		abcConsumedEnergyPerNode.clear();
		expectedTransTimePerNode.clear();
		expectedProcTimePerNode.clear();
	}
};

bool isViable; //diz se uma alocação de task para um nó é viável ou não

int abandonmentLimit = 2; //se tiver mais q 2 e estiver na metade pior das soluções, a abelha vai tentar abandonar
int countLimitScouts; //contar limite p chamar ou n as scout bees

//uint32_t foodSources = 50; //qtd de soluções ou food sources
//uint32_t numberOfCycles = 20; //número de ciclos
uint32_t bestSolutionPosition; //posição da melhor solução no vetor de soluções... inicialmente é zero
uint32_t initSecondHalfScout; //scout bees: vamos avaliar apenas a segunda metade do vetor ordenado de soluções
uint32_t indexOfChampionSolutionOnlooker; //onlooker bees: índice da solução com melhor fitness dentre as aleatoriamente escolhidas
uint32_t amountOnlooker; //qtas onlooker bees
uint32_t amountTournament; //onlooker bees: quantas soluções aleatórias pegar
uint32_t randomPos; //inteiro pegue aleatoriamente de um intervalo p representar a posição em um vetor
uint32_t bestPos; //usada na função bestSolution
uint32_t initSecondHalfNeighbFood;
uint32_t abcAvoidInfLoop;

double nearFoodSourceFactor = 0.5; //fator que determina uma food source próxima é 0.5; uma food source vizinha vai manter 50% dos números iniciais
double abandonmentFactor = 0.5; //fator que determina uma food source próxima é 0.5; uma food source vizinha vai manter 50% dos números iniciais
double onlookerFactor = 0.5; //onlookers bees são metade da qtd de food sources
double tournamentFactor = 0.1; //pega aleatoriamente apenas 10% das soluções
double maxFitness; //usada na função bestSolution
double maxTimeInitialize;
double fitnessInitialize;
double fitness; //p função do cálculo de fitness
double totalConsumedEnergy; //energia total gasta da solução
double noqueueProcTime; //calcular tempo sem fila apenas dessa task (depois vou somar o acumulado e a fila na funcao de fitness)
double procTime; //calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
double energyPerSecond; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
double expectedCsmdEnergy; //calcular consumo de energia esperado
double comTime; //tempo gasto com upload e download
double comTimeCurTask; //tempo gasto com upload e download apenas dessa task
double energyTransmissionPerSecond5G;
double energyTransmissionPerSecondWave;
double expectedCsmdEnergyTransmission; //energia gasta pelo cliente para transmitir apenas essa task
double howLongTake; //quanto tempo leva para processar uma task no servidor
double maxTimeCalc; //da função calculaMaxTime
double abcCurrentTime; //p pegar o tempo atual do simulador
//calculateMaxTime
double clientTotalTime;
double edgeTotalTime;
double carServersTotalTime;
double lastTransTimesCarServer;
double addWeightAbc;

Ptr<UniformRandomVariable> onlookerUrv; //variável aleatória p as onlooker bees
Ptr<UniformRandomVariable> urvNewFood; //variável aleatória p new food sources
Ptr<UniformRandomVariable> urvInitialize; //variável aleatória p initializePopulation
Ptr<UniformRandomVariable> urvNeighbFood; //variável aleatória p neighboringFood...

//Ipv4Address ipFromServer;
//std::string firstOctet;

Solution curSolution; //cria um objeto Solution p ficar sendo usado como solução temporária

//vetores globais permanentes
std::vector <Ptr<Node>> availableServers; //servidores que responderam
std::vector <Solution> solutions; //vetor permanente de soluções ou food sources

//vetores temporários
std::vector<double> tempVec;

//maps globais permanentes
std::map <Ptr<Node> , std::vector<double>> finalMap; //chave: nó, valor: vetor[0]: howManyBytes, vetor[1]: howMuchProc, vetor[2]: howManyTasks
//std::map <Ptr<Node> ,double> serverDist; //map auxiliar para ajudar na ordenação dos providers; node / distância para o cliente
std::map <Ptr<Node> ,double> serverLlt; //map auxiliar para ajudar na ordenação dos providers; node / llt

double calculateMaxTime(Solution &slt){

	clientTotalTime = 0.0;
	edgeTotalTime = 0.0;
	carServersTotalTime = 0.0;
	lastTransTimesCarServer = 0.0; //soma dos tempos de transmissão dos carServers anteriores

	for(std::map< Ptr<Node> , double>::iterator it = slt.expectedProcTimePerNode.begin();
	  it != slt.expectedProcTimePerNode.end(); ++it){

		slt.expectedProcTimePerNode[it->first] = slt.expectedProcTimePerNode[it->first] + auxProvidersQueue[it->first]; //somar c a fila pré-existente

		if(it->first == c.Get(clientId)){ //armazenar o tempo total do cliente
			clientTotalTime = slt.expectedProcTimePerNode[it->first];
		}
		else if(auxProvidersCap[it->first] >= 1.5){ //armazenar o tempo total do edgeNode
			edgeTotalTime = slt.expectedTransTimePerNode[it->first] + slt.expectedProcTimePerNode[it->first];
		}
		else{ //calcular o tempo de cada car server e tempo total gasto com os car servers
			//(ideia:)
			//carserver0: trans0+proc0
			//carserver1: trans0+trans1+proc1
			//carserver2: trans0+trans1+proc2 e assim por diante
			slt.expectedProcTimePerNode[it->first] = lastTransTimesCarServer + slt.expectedTransTimePerNode[it->first] +
					slt.expectedProcTimePerNode[it->first];
			if(slt.expectedProcTimePerNode[it->first] > carServersTotalTime){
				carServersTotalTime = slt.expectedProcTimePerNode[it->first];
			}
			lastTransTimesCarServer = lastTransTimesCarServer + slt.expectedTransTimePerNode[it->first];
		}

	}

	maxTimeCalc = std::max(clientTotalTime,edgeTotalTime); //cliente, edge e (veículos) rodam em paralelo)
	maxTimeCalc = std::max(maxTimeCalc,carServersTotalTime); //tempo total estimado da solução

	return maxTimeCalc;
	//@TODO: função de avaliação: 0.9 p tempo e 0.1 p energia?
}

double calculateEnergies(Solution &slt){

	//energia gasta pelo cliente nesta solução: consumedEnergyPerNode[c.Get(clientId)]

	totalConsumedEnergy = 0.0; //energia total gasta da solução
	for(std::map< Ptr<Node> , double>::iterator jt = slt.abcConsumedEnergyPerNode.begin();
	  jt != slt.abcConsumedEnergyPerNode.end(); ++jt){
		totalConsumedEnergy = totalConsumedEnergy + jt->second;
	}

	return totalConsumedEnergy; //se quiser retornar a energia gasta apenas pelo cliente: consumedEnergyPerNode[c.Get(clientId)]
}

double calculateFitness(double mxTm){
	fitness = (1.0/mxTm)*100.0; //é o inverso de maxTime: ou seja em qto menos tempo executar, maior a fitness
	fitness = fitness*fitness; //potencializar as melhores soluções e tender a esquecer as piores soluções
	return fitness;
}

bool isViableFunction(uint32_t posTask, Ptr<Node> nodeServer){

	isViable = false; //valor de retorno

	if(nodeServer == c.Get(clientId)){ //se o servidor escolhido é o próprio cliente

		//maps... se n der certo, tirar
		curSolution.abcHowMuchProcServer[nodeServer] = curSolution.abcHowMuchProcServer[nodeServer] + workloadMatrix[posTask].at(1);

		noqueueProcTime = calcSleepTime(auxProvidersCap[nodeServer], 0.0,
			workloadMatrix[posTask].at(1)); //calcular tempo sem fila apenas dessa task (depois vou somar o acumulado e a fila na funcao de fitness)

		//calcular tempo com fila e com os novos processamentos acumulados p saber se a energia vai ser suficiente
		procTime = calcSleepTime(auxProvidersCap[nodeServer], auxProvidersQueue[nodeServer], curSolution.abcHowMuchProcServer[nodeServer]);
		energyPerSecond = auxProvidersCpuConsumption[nodeServer]/3600.0; //energia gasta em 1s (em Watt ou Watt-hour?????????????)
		expectedCsmdEnergy = procTime*energyPerSecond; //calcular consumo de energia esperado

		if((auxProvidersEnergyLevel[nodeServer] - expectedCsmdEnergy) > auxProvidersMinimalEnergy[nodeServer]){
			isViable = true; //é viável pq n vai sair do alcance e a energia gasta n vai passar do mínimo da bateria
			//double queueProcTime = calcSleepTime(auxProvidersCap[nodeServer], 0.0, abcHowMuchProcServer[nodeServer]);

			//vetores
			curSolution.transTimes.push_back(0.0);
			curSolution.howManyBytes.push_back(workloadMatrix[posTask].at(0));
			curSolution.execTimes.push_back(noqueueProcTime); //o tempo esperado é apenas de processamento para o cliente
			curSolution.howMuchProc.push_back(workloadMatrix[posTask].at(1));
			curSolution.execEnergies.push_back(expectedCsmdEnergy);
			curSolution.transEnergies.push_back(0.0); //não gasta nada com transmissão já q vai executar localmente

			//maps
			curSolution.abcHowMuchDataForServer[nodeServer] = curSolution.abcHowMuchDataForServer[nodeServer]
				  + workloadMatrix[posTask].at(0); //n precisa transferir a task pq é o próprio cliente, mas armazeno mesmo assim
			//curSolution.abcHowManyTasksForServer[nodeServer] = curSolution.abcHowManyTasksForServer[nodeServer] + 1.0;
			curSolution.abcConsumedEnergyPerNode[c.Get(clientId)] = curSolution.abcConsumedEnergyPerNode[nodeServer]
					 + expectedCsmdEnergy; //energia gasta estimada em processamento pelo cliente (só processamento)
			curSolution.expectedTransTimePerNode[nodeServer] = 0.0; //cliente n transmite p ele mesmo

			if(carNodeCPUCap[clientId] == 1){
				addWeightAbc = 0.5;
			}
			else{
				addWeightAbc = 3.0;
			}
			curSolution.expectedProcTimePerNode[nodeServer] = curSolution.expectedProcTimePerNode[nodeServer]
					 + noqueueProcTime //tempo esperado para cada nó processar suas tasks
					 + addWeightAbc; //extra só p n escolher demais o local
		}
		else{
			curSolution.abcHowMuchProcServer[nodeServer] = curSolution.abcHowMuchProcServer[nodeServer] - workloadMatrix[posTask].at(1);
			if(curSolution.abcHowMuchProcServer[nodeServer] < 0.00001){ //se n tiver nada para processar, sai do map
				curSolution.abcHowMuchProcServer.erase(nodeServer);
			}
		}
	}
	else { //se o servidor é remoto, é preciso avaliar se ele n vai sair do alcance

		//@TODO: se n der certo, tirars
		curSolution.abcHowMuchProcServer[nodeServer] = curSolution.abcHowMuchProcServer[nodeServer] + workloadMatrix[posTask].at(1);
		curSolution.abcHowMuchDataForServer[nodeServer] = curSolution.abcHowMuchDataForServer[nodeServer] + workloadMatrix[posTask].at(0);

		//VERIFICA SE ESTÁ OK OFFLOADAR... analisa a viabilidade de processar a task...
		//quanto tempo levaria para processar a task? tempo de upload + download + tempo de espera na fila + tempo de processamento
		if(auxProvidersCap[nodeServer] >= 1.5){ //se tem capacidade de cpu >= 1.5 é edge server/5G
			comTime = (curSolution.abcHowMuchDataForServer[nodeServer] + 1000)/(datarateMmWave*0.25) + 0.05; //1000 é a volta do resultado do proc
			//@TODO: esse 1000 é para cada task... entao se forem enviadas 7 tasks, o valor deveria ser 7000
			comTimeCurTask = (workloadMatrix[posTask].at(0) + 1000)/(datarateMmWave*0.25) + 0.05;
			expectedCsmdEnergyTransmission = energyTransmissionPerSecond5G * comTimeCurTask; //energia gasta no cliente p transmitir só essa task
		}
		else { // é carro server/WAVE
			comTime = (curSolution.abcHowMuchDataForServer[nodeServer] + 1000)/(datarateWave*0.25) + 0.05; //1000 é a volta do resultado do proc
			//@TODO: esse 1000 é para cada task... entao se forem enviadas 7 tasks, o valor deveria ser 7000
			comTimeCurTask = (workloadMatrix[posTask].at(0) + 1000)/(datarateWave*0.25) + 0.05;
			expectedCsmdEnergyTransmission = energyTransmissionPerSecondWave * comTimeCurTask; //energia gasta no cliente p transmitir só essa task
		}
		//como as cargas anteriores já foram adicionadas à fila, aqui deveríamos adicionar só a nova task... mas do jeito q está, fica como margem
		procTime = calcSleepTime(auxProvidersCap[nodeServer], auxProvidersQueue[nodeServer], curSolution.abcHowMuchProcServer[nodeServer]);
		howLongTake = comTime + procTime;
		//expectedResultTime[getIPFromServer(nodeServer)] = howLongTake; //armazena essa informação por servidor

		//p cálculo posterior na função de fitness
		noqueueProcTime = calcSleepTime(auxProvidersCap[nodeServer], 0.0,
			workloadMatrix[posTask].at(1)); //calcular tempo sem fila apenas dessa task (depois vou somar o acumulado e a fila na funcao de fitness)

		energyPerSecond = auxProvidersCpuConsumption[nodeServer]/3600.0; //energia gasta em 1s (em Watt)
		expectedCsmdEnergy = procTime*energyPerSecond; //calcular consumo de energia esperado com fila e novo processamento acumulado

		if( (howLongTake < 25.0) && (howLongTake <= (auxProvidersLlt[nodeServer]-2.2)) ) { //se estarão conectados ainda
			if((auxProvidersEnergyLevel[nodeServer] - expectedCsmdEnergy) > auxProvidersMinimalEnergy[nodeServer]){ //verifica energia
				isViable = true;

				//vetores
				curSolution.transTimes.push_back(comTimeCurTask);
				curSolution.howManyBytes.push_back(workloadMatrix[posTask].at(0));
				curSolution.execTimes.push_back(noqueueProcTime); //adiciona o tempo esperado por servidor; depois é pegue o tempo max da solucao
				curSolution.howMuchProc.push_back(workloadMatrix[posTask].at(1));
				curSolution.execEnergies.push_back(expectedCsmdEnergy); //energia gasta no processamento dessa task
				curSolution.transEnergies.push_back(expectedCsmdEnergyTransmission); //energia gasta pelo cliente na transmissão dessa task

				//maps
				//curSolution.abcHowManyTasksForServer[nodeServer] = curSolution.abcHowManyTasksForServer[nodeServer] + 1.0;
				curSolution.abcConsumedEnergyPerNode[c.Get(clientId)] = curSolution.abcConsumedEnergyPerNode[c.Get(clientId)]
						 + expectedCsmdEnergyTransmission; //somar a energia gasta estimada em transmissões pelo cliente
				curSolution.abcConsumedEnergyPerNode[nodeServer] = curSolution.abcConsumedEnergyPerNode[nodeServer]
						 + expectedCsmdEnergy; //energia gasta estimada em processamento pelo servidor
				//tempo esperado para o cliente transmitir suas tasks p os servers:
				curSolution.expectedTransTimePerNode[nodeServer] = curSolution.expectedTransTimePerNode[nodeServer] + comTimeCurTask; //tempo p trans
				curSolution.expectedProcTimePerNode[nodeServer] = curSolution.expectedProcTimePerNode[nodeServer] + noqueueProcTime //tempo p proc
						 + auxProvidersDist[nodeServer]/trange
						 + auxProvidersQueue[nodeServer]/10.0
						 + 0.5/auxProvidersCap[nodeServer]; //adiciono uma penalidade pela distância, fila e cap
			}
		}
		else { //retirar o q tinha sido adicionado
			curSolution.abcHowMuchProcServer[nodeServer] = curSolution.abcHowMuchProcServer[nodeServer] - workloadMatrix[posTask].at(1);
			curSolution.abcHowMuchDataForServer[nodeServer] = curSolution.abcHowMuchDataForServer[nodeServer] - workloadMatrix[posTask].at(0);
			if(curSolution.abcHowMuchProcServer[nodeServer] < 0.00001){ //se n tiver nada para processar, sai do map
				curSolution.abcHowMuchProcServer.erase(nodeServer);
				curSolution.abcHowMuchDataForServer.erase(nodeServer);
			}
		}

	}

	return isViable;

}

void initializePopulation(){

	//solução: lista de size==workloadMatrix.size(); cada posição da lista==task de workloadMatrix; cada valor==node que processará a task

	for(uint32_t itSol=0; itSol < foodSources; itSol++){

		curSolution.clear(); //método p limpar todos os vetores e maps do objeto
		auxProvidersEnergyLevel[c.Get(clientId)] = carEnergyLevel[clientId]; //restaura o nível de energia real
		for(uint32_t itTask=0; itTask<workloadMatrix.size(); itTask++){ //atribuir um node para cada task; //criar uma solução: task1: nodex, task2: nodey ...
			isViable = false;
			abcAvoidInfLoop = 0;
			while(isViable==false){ //enqto a solucao n for viavel, continuar gerando solucoes
				//p cada posição de solution colocar um node aleatório de availableServers (pode ser com repetição; escolhido com base na posicao)
				randomPos = urvInitialize->GetInteger(0, availableServers.size()-1);
				//std::cout << "randomPos ==> "<< randomPos << std::endl;
				isViable = isViableFunction(itTask, availableServers[randomPos]); //chama funcao para verificar viabilidade desse par: node/task
				if(abcAvoidInfLoop >= 20){ //c 20 tries e nada, liberamos p executar o restante na marra no local
					auxProvidersEnergyLevel[c.Get(clientId)] = 999999; //liberar local na marra
				}
				abcAvoidInfLoop += 1; //incrementa p evitar loop infinito
			}
			curSolution.nodes.push_back(availableServers[randomPos]); //tempo e energia são adicionados na funcao isViable
		}
		maxTimeInitialize = calculateMaxTime(curSolution);
		curSolution.maxTime = maxTimeInitialize;
		fitnessInitialize = calculateFitness(maxTimeInitialize); //calcula a fitness da solução
		curSolution.fitness = fitnessInitialize;
		solutions.push_back(curSolution); //adiciona curSolution no vetor de soluções
		//solutions[itSol] = curSolution; //copia curSolution p o vetor de soluções
	}
}

//função para escolher a fonte de comida vizinha (para employed e onlooker bees)
//void neighboringFoodSource(uint32_t originalPosition, std::vector< Ptr<Node>> &sltNodes){
void neighboringFoodSource(uint32_t originalPosition, Solution &slt){

	//já tenho a solução/food source... agora preciso da food source vizinha

	//vamos percorrer a primeira metade da solução apenas p calcular abcHowMuchProcServer etc
	curSolution.clear();
	auxProvidersEnergyLevel[c.Get(clientId)] = carEnergyLevel[clientId]; //restaura o nível de energia real
	//primeira metade da solução / já é conhecida pq já está em sltNodes / é uma repetição da primeira metade de sltNodes (a food source original)
	for(uint32_t itTask=0; itTask<initSecondHalfNeighbFood; itTask++){ //atribuir node p cada task (solução: task1: nodex, task2: nodey)
		isViable = false;
		while(isViable==false){ //enqto a solucao n for viavel, continuar escolhendo nodes
			//essa chamada é só p preencher os vetores temporários
			isViable = isViableFunction(itTask, slt.nodes[itTask]); //chama funcao para verificar viabilidade desse par: node/task
		}
		curSolution.nodes.push_back(slt.nodes[itTask]); //tempo e energia são adicionados na funcao isViable
	}

	//segunda metade da solução / vamos adicionar nodes aleatórios para as abelhas explorarem food sources vizinhas
	for(uint32_t jtTask=initSecondHalfNeighbFood; jtTask<workloadMatrix.size(); jtTask++){ //node p cada task; (solução: task1: nodex, task2: nodey)
		isViable = false;
		abcAvoidInfLoop = 0;
		while(isViable==false){ //enqto a solucao n for viavel, continuar escolhendo nodes
			//p cada posição de solution colocar um node aleatório de availableServers (pode ser com repetição; escolhido com base na posicao)
			randomPos = urvNeighbFood->GetInteger(0, availableServers.size()-1);
			//std::cout << "randomPos ==> "<< randomPos << std::endl;
			isViable = isViableFunction(jtTask, availableServers[randomPos]); //chama funcao para verificar viabilidade desse par: node/task
			if(abcAvoidInfLoop >= 20){ //c 50 tries e nada, liberamos p executar o restante na marra no local
				auxProvidersEnergyLevel[c.Get(clientId)] = 999999; //liberar local na marra
			}
			abcAvoidInfLoop += 1; //incrementa p evitar loop infinito
		}
		curSolution.nodes.push_back(availableServers[randomPos]); //tempo e energia são adicionados na funcao isViable
	}

	//já tenho a food source original e a food source vizinha / nova solução (curSolution)
	//agora preciso comparar as funções de fitness... quem tiver a maior, vai ficar na posição do vetor permanente de soluções

	curSolution.maxTime = calculateMaxTime(curSolution); //calcula a fitness da solução que está na posição originalPosition
	curSolution.fitness = calculateFitness(curSolution.maxTime);

	if(curSolution.fitness > solutions[originalPosition].fitness){ //se for maior (mais aptidão), vai substituir a food source original
		solutions[originalPosition] = curSolution; //adiciona curSolution no vetor de soluções
		solutions[originalPosition].cycles = -1; //valor inicial de ciclos da nova solução
	}
}

//função para escolher uma nova fonte de comida (para scouts bees)
void newFoodSource(uint32_t originalPosition, Solution &slt){

	curSolution.clear();
	auxProvidersEnergyLevel[c.Get(clientId)] = carEnergyLevel[clientId]; //restaura o nível de energia real
	//vamos adicionar nodes aleatórios para as abelhas explorarem new food sources
	for(uint32_t itTask=0; itTask<workloadMatrix.size(); itTask++){ //atribuir um node para cada task; //criar uma solução: task1: nodex, task2: nodey ...
		isViable = false;
		abcAvoidInfLoop = 0;
		while(isViable==false){ //enqto a solucao n for viavel, continuar escolhendo nodes
			//p cada posição de solution colocar um node aleatório de availableServers (pode ser com repetição; escolhido com base na posicao)
			randomPos = urvNewFood->GetInteger(0, availableServers.size()-1);
			//std::cout << "randomPos ==> "<< randomPos << std::endl;
			isViable = isViableFunction(itTask, availableServers[randomPos]); //chama funcao para verificar viabilidade desse par: node/task
			if(abcAvoidInfLoop >= 20){ //c 50 tries e nada, liberamos p executar o restante na marra no local
				auxProvidersEnergyLevel[c.Get(clientId)] = 999999; //liberar local na marra
			}
			abcAvoidInfLoop += 1; //incrementa p evitar loop infinito
		}
		curSolution.nodes.push_back(availableServers[randomPos]); //tempo e energia são adicionados na funcao isViable
	}

	curSolution.maxTime = calculateMaxTime(curSolution); //calcula a fitness da solução que está na posição originalPosition
	curSolution.fitness = calculateFitness(curSolution.maxTime);

	if(curSolution.fitness > solutions[originalPosition].fitness){ //se for maior (mais aptidão), vai substituir a food source original
		solutions[originalPosition] = curSolution; //adiciona curSolution no vetor de soluções
		solutions[originalPosition].cycles = -1; //valor inicial de ciclos da nova solução
	}
}

void employedBees(){
	for(uint32_t i=0; i<foodSources;i++){ //percorrer o vetor de soluções
		//ver uma solução vizinha para cada solução original (mantendo alguns números iguais e modificando outros)
		neighboringFoodSource(i, solutions[i]); //i é a posição original no vetor permanente de soluções
	}
}

void onlookerBees(){

	//torneio com base na fitness das soluções
	for(uint32_t i=0; i<amountOnlooker;i++){ //esse loop é para todas as abelhas espectadoras (q são metade da qtd de food sources)
		randomPos = onlookerUrv->GetInteger(0, foodSources-1); //0 a 99... escolhe entre todas as food sources
		indexOfChampionSolutionOnlooker = randomPos; //inicializa com a primeira posição aleatória escolhida
		for(uint32_t j=1; j<amountTournament; j++){ //começa em 1 pq a 0 foi antes do loop
			randomPos = onlookerUrv->GetInteger(0, foodSources-1); //0 a 99... escolhe entre todas as food sources
			if(solutions[randomPos].fitness > solutions[indexOfChampionSolutionOnlooker].fitness){ //se fitness da solução da nova randpos é melhor
				indexOfChampionSolutionOnlooker = randomPos;
			}
		}
		//faz isso para cada onlooker bee
		neighboringFoodSource(indexOfChampionSolutionOnlooker, solutions[indexOfChampionSolutionOnlooker]); //originalPos no vetor perm de sols

	}
}

void scoutBees(){

	std::sort(solutions.begin(), solutions.end(), [](const Solution& lhs, const Solution& rhs){ //ordenar vetor de soluções em ordem decrescente
		return lhs.fitness > rhs.fitness;
	});

	for(uint32_t i=initSecondHalfScout; i<solutions.size();i++){ //avaliar se a metade pior das soluções serão abandonadas
		if(solutions[i].cycles > abandonmentLimit){ //está entre as 50 piores soluções e já teve mais de 5 ciclos p melhorar e n melhorou
			newFoodSource(i, solutions[i]); //vai explorar outra solução
		}
	}
}

void bestSolutionAndCycles(){
	maxFitness = solutions[0].fitness; //supomos q o primeiro elemento é o maior... dps vamos ver se é isso mesmo
	solutions[0].cycles++; //já incrementa os ciclos aqui
	bestPos = 0;
	for(uint32_t i=1; i<solutions.size();i++){
		solutions[i].cycles++; //já incrementa os ciclos aqui
		if(solutions[i].fitness > maxFitness){
			maxFitness = solutions[i].fitness;
			bestPos = i;
		}
	}
	bestSolutionPosition = bestPos; //bestSolutionPosition é global
}

void printDecision(Solution &slt){
	double printProcClient; //só p imprimir o processamento do cliente
	bool hasClient = false; //supoe q o cliente n está no finalMap
	//for(std::map< Ptr<Node> , std::vector<double>>::iterator it = finalMap.begin(); //percorrer o map final
	//  it != finalMap.end(); ++it){
	for(std::map< Ptr<Node> , double>::iterator it = slt.abcHowMuchProcServer.begin(); //percorrer o map final
	  it != slt.abcHowMuchProcServer.end(); ++it){
		if(it->first == c.Get(clientId)){
			hasClient = true;
		}
	}
	if(hasClient == true){ //se tem algo no finalMap sobre o cliente
		printProcClient = slt.abcHowMuchProcServer[c.Get(clientId)];
	}
	else{
		printProcClient = 0.0;
	}
	std::cout << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
	os << "[DECISION] ClientLocal: " << getIPFromServer(c.Get(clientId));
	std::cout << ". Queue: " << auxProvidersQueue[c.Get(clientId)] << ". Cap: " << auxProvidersCap[c.Get(clientId)]
	   << ". Proc.: " << printProcClient << "Gc. Transf.: " << 0 << "KB. Dist.: " << auxProvidersDist[c.Get(clientId)]
	   << std::endl;
	os << ". Queue: " << auxProvidersQueue[c.Get(clientId)] << ". Cap: " << auxProvidersCap[c.Get(clientId)]
	   << ". Proc.: " << printProcClient << "Gc. Transf.: " << 0 << "KB. Dist.: " << auxProvidersDist[c.Get(clientId)]
	   << std::endl;
	for(uint32_t i=0; i<providers.size(); i++){
		if(auxProvidersCap[providers[i]] >= 1.5){ //se tem capacidade de cpu >= 1.5 é edge server/5G
			std::cout << "[DECISION] Edge: " << getIPFromServer(providers[i]);
			os << "[DECISION] Edge: " << getIPFromServer(providers[i]);
		}
		else{
			std::cout << "[DECISION] CarServer: " << getIPFromServer(providers[i]);
			os << "[DECISION] CarServer: " << getIPFromServer(providers[i]);
		}
		std::cout << ". Queue: " << auxProvidersQueue[providers[i]] << ". Cap: " << auxProvidersCap[providers[i]]
		   << ". Proc.: " << scheduleMatrix[i][1] << "Gc. Transf.: " << (scheduleMatrix[i][0])/1000 <<
		   "KB. Dist.: " << auxProvidersDist[providers[i]] << ". LLT.: " << auxProvidersLlt[providers[i]] << "." << std::endl;
		os << ". Queue: " << auxProvidersQueue[providers[i]] << ". Cap: " << auxProvidersCap[providers[i]]
		   << ". Proc.: " << scheduleMatrix[i][1] << "Gc. Transf.: " << (scheduleMatrix[i][0])/1000 <<
		   "KB. Dist.: " << auxProvidersDist[providers[i]] << ". LLT.: " << auxProvidersLlt[providers[i]] << "." << std::endl;
	}
}

void printAlgorithmTime(){
	std::string filename = "results/abcTime-" + scenario + "-" + density + "-" + std::to_string(workload) + ".tr";
	const char* fname = filename.c_str();
	fp = fopen(fname, "a+");  //arquivo com resultados
	fprintf(fp,"%lu;%lu;%.3f;%lu\n", (unsigned long) numberOfCycles, (unsigned long) foodSources,
			elapsedTime, (unsigned long) run);
	fclose(fp);
	exit(0);
}

//função para adaptar a melhor solução ao problema e dar continuidade ao offloading
void adaptSolution(uint32_t pos, Solution &slt){

	abcCurrentTime = Simulator::Now().GetSeconds ();

	serverLlt.clear(); //map auxiliar para ajudar na ordenação dos providers; node / llt

	finalMap.clear(); //chave: nó, valor: vetor[0]: howManyBytes, vetor[1]: howMuchProc
	for(uint32_t i=0; i<slt.nodes.size();i++){
		if(slt.nodes[i] == c.Get(clientId)){
			tasksOnlyLocal += 1; //conta o número de tasks executadas localmente desde o início
		}
		else{
			tasksOffloadedSuc += 1; //conta o número de tasks que devem ser executadas pelos servers
		}

		//transformar o vector de soluções em um map
		if(finalMap[slt.nodes[i]].size() ==3){ //já fez o push_back inicial
			finalMap[slt.nodes[i]][0] = finalMap[slt.nodes[i]][0] + slt.howManyBytes[i]; //quantos bytes cada server vai receber
			finalMap[slt.nodes[i]][1] = finalMap[slt.nodes[i]][1] + slt.howMuchProc[i]; //quantos gigaciclos cada server vai processar
			finalMap[slt.nodes[i]][2] += 1.0; //quantas tasks cada server vai processar
		}
		else{ //ainda n fez o push_back inicial
			tempVec.clear();
			tempVec.push_back(0.0);
			tempVec.push_back(0.0);
			tempVec.push_back(0.0);
			finalMap[slt.nodes[i]] = tempVec; //pronto... fez o push_back inicial

			finalMap[slt.nodes[i]][0] = finalMap[slt.nodes[i]][0] + slt.howManyBytes[i]; //quantos bytes cada server vai receber
			finalMap[slt.nodes[i]][1] = finalMap[slt.nodes[i]][1] + slt.howMuchProc[i]; //quantos gigaciclos cada server vai processar
			finalMap[slt.nodes[i]][2] += 1.0; //quantas tasks cada server vai processar
		}
	}

	for(std::map< Ptr<Node> , std::vector<double>>::iterator it = finalMap.begin(); //percorrer o map final
	  it != finalMap.end(); ++it){
	//for(std::map< Ptr<Node> , double>::iterator it = slt.abcHowMuchProcServer.begin(); //percorrer o map final
	//  it != slt.abcHowMuchProcServer.end(); ++it){

		if(it->first == c.Get(clientId)){ //se é o cliente, já coloca para executar
			//%%%%%%%%%%%%%%%%%%%% CLIENTE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			//tasksOnlyLocal = slt.abcHowManyTasksForServer[it->first]; //qtd de tasks executadas apenas pelo cliente
			localTime = abcCurrentTime - tempoIni; //tempo até chegar aqui desde o initAction2
			if(it->second[1] > 0){ //tem algo para processar no cliente/local
			//if(it->second > 0){ //tem algo para processar no cliente/local
				localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], it->second[1]); //processamento do cliente
				//localTime = localTime + calcSleepTime(carNodeCPUCap[clientId], carNodeCPUQueue[clientId], it->second); //processamento do cliente
			}
			//%%%%%%%%%%%%%%%%%%%% CLIENTE %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		}
		else{
			if(auxProvidersCap[it->first] >= 1.5){ //se for edge, coloca para ser o primeiro server a receber as tasks
				//tasksOffloadedSuc = tasksOffloadedSuc + slt.abcHowManyTasksForServer[it->first]; //conta a qtd de tasks executadas remotamente
				providers.push_back(it->first);
				tempVec.clear();
				tempVec.push_back(it->second[0]);
				tempVec.push_back(it->second[1]);
				tempVec.push_back(it->second[2]);
				//tempVec.push_back(slt.abcHowMuchDataForServer[it->first]); //usa o node q estamos percorrendo p pegar do outro map
				//tempVec.push_back(it->second); //já é o valor do map q estamos percorrendo
				//tempVec.push_back(slt.abcHowManyTasksForServer[it->first]); //qtd de tasks por server
				scheduleMatrix.push_back(tempVec);
				numberOfSurrogates++;
			}
			else{ //se é car Server
				//tasksOffloadedSuc = tasksOffloadedSuc + slt.abcHowManyTasksForServer[it->first]; //conta a qtd de tasks executadas remotamente
				serverLlt[it->first] = auxProvidersLlt[it->first]; //só p n percorrer o auxProviders todo
			}
		}
	}

	//VOU ENVIAR PARA OS OUTROS SERVIDORES (CAR SERRVERS) DOS MAIS DISTANTES PARA OS MENOS DISTANTES
	std::vector <pair> sortCarServers; //vetor de pares usado para ordenar os carServers
	//agora só tem carServers: ordenar com os primeiros sendo os mais distantes
	// copia os pares chave-valor do map com a soma queue+cap+distance (normalizada/equalizada)
	std::copy(serverLlt.begin(),
			serverLlt.end(),
			std::back_inserter<std::vector<pair>>(sortCarServers));
	// ordene o vetor em ordem decrescente pelo segundo valor do pair
	// se o segundo valor for igual, ordene pelo primeiro valor do par
	std::sort(sortCarServers.begin(), sortCarServers.end(),
			[](const pair& l, const pair& r) {
		if (l.second != r.second)
			return l.second < r.second;
		return l.first < r.first;
	});

	for(uint32_t j=0; j<sortCarServers.size();j++){ //agora vou adicionando os carServers à solução
		if(auxProvidersCap[sortCarServers[j].first] < 1.5){ //carServer
			providers.push_back(sortCarServers[j].first);
			tempVec.clear();
			tempVec.push_back(finalMap[sortCarServers[j].first][0]);
			tempVec.push_back(finalMap[sortCarServers[j].first][1]);
			tempVec.push_back(finalMap[sortCarServers[j].first][2]);
			//tempVec.push_back(slt.abcHowMuchDataForServer[sortCarServers[j].first]);
			//tempVec.push_back(slt.abcHowMuchProcServer[sortCarServers[j].first]);
			//tempVec.push_back(slt.abcHowManyTasksForServer[sortCarServers[j].first]);
			scheduleMatrix.push_back(tempVec);
			numberOfSurrogates++;
		}
	}

	endElapsedTime = std::chrono::high_resolution_clock::now();

	elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endElapsedTime - startElapsedTime).count();
	elapsedTime *= 1e-9;
	if(auxProvidersCap[c.Get(clientId)]==0.5){
		elapsedTime = elapsedTime*2.0;
	}

	//elapsedTime = elapsedTime/5.0;
	//elapsedTime = 0.384;

	std::cout << "[ELAPSED TIME] " << elapsedTime << std::endl;
	os << "[ELAPSED TIME] " << elapsedTime << std::endl;

	localTime = localTime + elapsedTime; //só começa a executar no local dps da decisão

	printDecision(slt); //imprimir a decisão do algoritmo
	//printAlgorithmTime(); //imprimir o tempo de execução do algoritmo

	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	//dá prosseguimento ao offloading
	if ((providers.size() == numberOfSurrogates) && (alreadySent == false)) {
		for(uint32_t k = 0; k < providers.size(); k++){
			//Simulator::Schedule(Seconds(0.0),serverSide);
			//Simulator::Schedule(Seconds(0.0),clientSide);
			////Simulator::Schedule(Seconds(k*0.001),clientSide);
			Simulator::Schedule(Seconds(elapsedTime),serverSide);
			Simulator::Schedule(Seconds(elapsedTime),clientSide);
		}
		alreadySent = true;
		//Simulator::Schedule(Seconds(1.0),checkConnectivity); //dá início as verificações periódicas de conectividade
		Simulator::Schedule(Seconds(1.0 + elapsedTime),checkConnectivity); //dá início as verificações periódicas de conectividade
	}
}

void calculateLltAndWithinRange(){

	double currentTime = Simulator::Now().GetSeconds ();
	uint32_t futureTime = ceil(currentTime); //tempo futuro inicial, arrendondar para cima... mobility file tem posições em tempos do tipo int
	uint32_t limitTime = floor(finishTime); //tempo final p verificar se está dentro do alcance

	//calcular tempo de vida e withinRange para cada um dos availableServers
	for(uint32_t i=0; i<availableServers.size(); i++){
		futureTime = ceil(currentTime);
		if(availableServers[i] == c.Get(clientId)){ //se é o cliente
			auxProvidersLlt[availableServers[i]] = 100; //tempo de vida infinito (é ele mesmo)
		}
		else{
			if(auxProvidersCap[availableServers[i]] >= 1.5){ //se é o edgeNode
				auxProvidersKr[availableServers[i]] = true; //edge node ta parado, então tem rota conhecidas
				//calcular tempo de vida com withinRange
				uint32_t curId = availableServers[i]->GetId(); //pegar ID do servidor
				bool withinrange = true; //no começo estão dentro do alcance um do outro
				while(withinrange == true){ //ou chegar no fim do arquivo
					withinrange = withinRangeAbc(futureTime,clientId,curId,180.0); //os dois nós ainda estarão dentro do alcance um do outro?
					futureTime++;
					if(futureTime >= limitTime){
						withinrange = false; //p sair do loops
					}
				}
				futureTime--; //último tempo em que os dois nós estavam dentro do alcance um do outro
				auxProvidersLlt[availableServers[i]] = futureTime - currentTime;
			}
			else{ //se é carServer
				uint32_t curId = availableServers[i]->GetId(); //pegar ID do carro servidor
				uint32_t positionInC = 0; //verificar se a rota do carServer é conhecida
				for(uint32_t j=0; j<c.GetN();j++){
					if(curId == c.Get(j)->GetId()){
						positionInC = j;
						break;
					}
				}
				auxProvidersKr[availableServers[i]] = carNodeKnownRoutes[positionInC]; //1 é true e 0 é false

				if(auxProvidersKr[availableServers[i]] == false){ //se a rota não é conhecida, calcular let do modo tradicional
					Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
					Ptr<MobilityModel> model2 = availableServers[i]->GetObject<MobilityModel>();
					//double distance = GetDistance(model1,model2);
					double let = linkEstimatedLifeTime(model1,model2,240.0); // Tempo de vida do enlace; 'let', botei range menor
					auxProvidersLlt[availableServers[i]] = let;
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
					auxProvidersLlt[availableServers[i]] = futureTime - currentTime;
				}

			}
		}

	}
}

void preInitialize(){
	urvInitialize = CreateObject<UniformRandomVariable> (); //p initializePopulation
	initSecondHalfNeighbFood = workloadMatrix.size()*nearFoodSourceFactor; //vamos alterar apenas a segunda metade da atual solução
	urvNeighbFood = CreateObject<UniformRandomVariable> ();
	onlookerUrv = CreateObject<UniformRandomVariable> ();
	amountOnlooker = foodSources*onlookerFactor; //qtas onlooker bees existem
	amountTournament = foodSources*tournamentFactor; //onlooker bees: quantas soluções aleatórias pegar
	initSecondHalfScout = solutions.size()*abandonmentFactor; //scout bees: vamos avaliar apenas a segunda metade do vetor ordenado de soluções
	urvNewFood = CreateObject<UniformRandomVariable> (); //new food sources
	energyTransmissionPerSecond5G = 1.0/3600.0; //1.0 Watt é uma constante q definimos p trasmissoes 5G
	energyTransmissionPerSecondWave = 0.046/3600.0; //0.046 Watt é uma constante q definimos p transmissoes WAVE
	countLimitScouts = 0;
	abcAvoidInfLoop = 0;

	for(std::map< Ptr<Node> , double>::iterator it = auxProvidersQueue.begin(); //percorrer servers that replied (auxProvidersQueue ou cap ou dist)
	  it != auxProvidersQueue.end(); ++it){
		availableServers.push_back(it->first);
	}

	calculateLltAndWithinRange();

	//inicializar vetor de soluções com soluções vazias p evitar criar no initialize
	//curSolution.clear();
	//for(uint32_t itSol=0; itSol < foodSources; itSol++){
	//	solutions.push_back(curSolution); //adiciona curSolution no vetor de soluções
	//}

}

void abcMain(){

	preInitialize();

	std::time_t realTime = std::time(nullptr);
	std::cout << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;
	os << "[DATA E HORA]" << std::asctime(std::localtime(&realTime)) << std::endl;

	startElapsedTime = std::chrono::high_resolution_clock::now();

	initializePopulation(); //gerar população inicial de soluções
	for(uint32_t j=0; j< numberOfCycles;j++){ //ciclos de execução ou @TODO: verificar quando as soluções convergem
		employedBees(); //chama as abelhas empregadas
		onlookerBees(); //chama as abelhas espectadoras
		if(countLimitScouts >= abandonmentLimit){ //fica chamando as scouts depois de passados 2 ciclos (0 e 1, chama no 2)
			scoutBees(); //chama as abelhas exploradoras
			countLimitScouts = 0; //zera p recomeçar...
		}

		bestSolutionAndCycles(); //memorizar a melhor solução
		countLimitScouts += 1; //incrementa p ficar contando os limites relacionados às scouts
	}

	adaptSolution(bestSolutionPosition, solutions[bestSolutionPosition]);

}

void abcDecision(Ipv4Address ipv4From, uint32_t iface){
	if(iface==1){ //se recebeu resposta pela interface 5G; sempre vai receber primeiro por aqui...
		for (size_t i = 0; i < edgeNodes.GetN(); i++) {
			if (ipv4From == edgeNodes.Get(i)->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal()){ //a borda só tem interface 5G

				enbNotFound = false; //enb foi encontrado
				Ptr<MobilityModel> model1 = clients.Get(0)->GetObject<MobilityModel>();
				Ptr<MobilityModel> model2 = enbNodes.Get(i)->GetObject<MobilityModel>(); //pega o modelo de mobilidade do enb associado
				double distance = GetDistance(model1,model2);
				double let = linkEstimatedLifeTime(model1,model2,210.0); // Tempo de vida do enlace; 'let', botei range menor
				//std::cout << "[CLIENTE] ABC: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
				//		edgeNodeCPUCap[idxFromMoreClosestEnb] << " e CpuQueue ==> " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << std::endl;
				std::cout << "LLT: " << let << ". Cap.: " << edgeNodeCPUCap[idxFromMoreClosestEnb] <<
						". Queue: " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << "." << std::endl;
				os << "LLT: " << let << ". Cap.: " << edgeNodeCPUCap[idxFromMoreClosestEnb] <<
						". Queue: " << edgeNodeCPUQueue[idxFromMoreClosestEnb] << "." << std::endl;

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
							//parte de energia
							auxProvidersCpuConsumption[edgeNodes.Get(i)] = edgeCpuConsumption[idxFromMoreClosestEnb];
							auxProvidersMinimalEnergy[edgeNodes.Get(i)] = -999; //edge n tem mínimo de energia pq é ligado à operadora de energia
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
				//std::cout << "[CLIENTE] ABC: Servidor ==> "<< ipv4From << ". Tempo de vida do enlace ==> " << let << ". CpuCap ==> " <<
				//		carNodeCPUCap[i] << " e CpuQueue ==> " << carNodeCPUQueue[i] << std::endl;
				std::cout << "LLT.: " << let << ". Cap.: " << carNodeCPUCap[i] << ". Queue: " << carNodeCPUQueue[i] << "." << std::endl;
				os << "LLT.: " << let << ". Cap.: " << carNodeCPUCap[i] << ". Queue: " << carNodeCPUQueue[i] << "." << std::endl;

				double procTime = calcSleepTime(carNodeCPUCap[i], carNodeCPUQueue[i], 3); //3-cpuRequired médio; @TODO: média das tasks?
				double transferTime = (1200000 + 1000)/(datarateWave*0.25) + 0.05; //1200000-taskSize médio;
				double possibleTime = transferTime + procTime;

				if (let >= possibleTime){
					if(alreadySent == false){ //enquanto ainda não foi enviado o workload, pode continuar adicionando possíveis providers
						if(alreadyDecided == false){ //se ainda n decidiu qtos mandar
							auxProvidersQueue[c.Get(i)] = carNodeCPUQueue[i];
							auxProvidersCap[c.Get(i)] = carNodeCPUCap[i];
							auxProvidersDist[c.Get(i)] = distance;
							//parte de energia
							auxProvidersCpuConsumption[c.Get(i)] = carCpuConsumption[i];
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
		Simulator::Schedule(Seconds(0.5),abcMain); //só dá prosseguimento após 0.5s
		alreadySentToSort = true;
	}
}

//a ideia é passar o tempo e o nó e a função retornar as posições x e y num vetor
std::vector<double> estimatedPositionAbc(uint32_t estTime, uint32_t analyzedNode){
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
std::vector<double> lastKnownPositionAbc(uint32_t estTime, uint32_t analyzedNode){
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
bool withinRangeAbc(uint32_t estTime, uint32_t analyzedNode1, uint32_t analyzedNode2, double range){
	estTime = estTime - 3; //no traces de mobilidade, para saber a posição no tempo x, temos q olhar o setdest setado na posição x-3
	estTime = estTime + 2; //tb tenho somar 2 pq o current.tcl é construído desde 2s antes
	std::vector<double> rEstPos1 = estimatedPositionAbc(estTime, analyzedNode1); //posição do node1 no tempo estTime
	if(rEstPos1.empty()){ //se o vetor estiver vazio
		rEstPos1 = lastKnownPositionAbc(estTime, analyzedNode1); //pegar a última posição conhecida do nó
	}
	std::vector<double> rEstPos2; //posição do node2 no tempo estTime
	if(range==180.0){ //se é edge server, pegar a posição fixa do enb
		rEstPos2.push_back(enbXPositions[idxFromMoreClosestEnb]);
		rEstPos2.push_back(enbYPositions[idxFromMoreClosestEnb]);
	} else { //se é carro server
		rEstPos2 = estimatedPositionAbc(estTime, analyzedNode2);
	}
	if(rEstPos2.empty()){ //se o vetor estiver vazio
		rEstPos2 = lastKnownPositionAbc(estTime, analyzedNode2); //pegar a última posição conhecida do nó
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

