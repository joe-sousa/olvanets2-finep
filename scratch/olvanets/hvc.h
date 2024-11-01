/*
*  Alisson Barbosa, Janeiro/2021. E-mail: alisson@ufc.br
*  This code realizes the packets exchange between vehicles (V2V) and the edge (V2I), simulating
*  the send of subtasks to be executed in the surrogate vehicles and in the edge of the network.
*
*  This code is protected by copyright and intellectual property right laws.
*  Do not use or distribute without the author's authorization.
*  This is true even for modified versions or snippets of this code.
*
*/

#ifndef HVC_H_
#define HVC_H_

#include <algorithm>
#include "ns3/internet-stack-helper.h"
#include "ns3/mobility-model.h"
#include "scratch/olvanets/olvanets.h"
#include "scratch/olvanets/globals.h"

using namespace ns3;

void tasksToLocalHvc(uint32_t clientId);
void hvcTimeoutToFind();
void hvcDecisionNoMmWave();
void hvcDecision(Ipv4Address ipv4From, uint32_t iface);

#endif /* HVC_H_ */
