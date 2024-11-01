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

#ifndef MDO_H_
#define MDO_H_

#include <algorithm>
#include "ns3/internet-stack-helper.h"
#include "ns3/mobility-model.h"
#include "scratch/olvanets/olvanets.h"
#include "scratch/olvanets/globals.h"

using namespace ns3;



//ordena os possíveis carros servidores por tamanho da fila, capacidade de cpu e distância
void sortByQueueCapDistMdo();
//ordena os substitutos pelas menores CPUsBusys, corta para ficar apenas a quantidade desejada de surrogates
//e adiciona-os ao providers
void SortAndInitTransferMdo();
void printDecisionMdo();
void mdoDecision (Ipv4Address ipv4From, uint32_t iface);

#endif /* MDO_H_ */
