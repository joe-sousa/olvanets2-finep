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

#ifndef ABC_H_
#define ABC_H_

#include <algorithm>
#include "ns3/internet-stack-helper.h"
#include "ns3/mobility-model.h"
#include "scratch/olvanets/olvanets.h"
#include "scratch/olvanets/globals.h"

using namespace ns3;


void abcMain();
void abcDecision(Ipv4Address ipv4From, uint32_t iface);
std::vector<double> estimatedPositionAbc(uint32_t estTime, uint32_t analyzedNode);
std::vector<double> lastKnownPositionAbc(uint32_t estTime, uint32_t analyzedNode);
bool withinRangeAbc(uint32_t estTime, uint32_t analyzedNode1, uint32_t analyzedNode2, double range);

#endif /* ABC_H_ */
