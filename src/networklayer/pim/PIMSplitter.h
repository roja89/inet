//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#ifndef __INET_PIMSPLITTER_H
#define __INET_PIMSPLITTER_H

#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IRoutingTable.h"

#include <omnetpp.h>
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "InterfaceTableAccess.h"
#include "InterfaceTable.h"
#include "NotifierConsts.h"
#include "IPv4Address.h"
#include "IPv4RoutingTable.h"
#include "PIMNeighborTable.h"
#include "PIMInterfaceTable.h"


#define HT 30.0										/**< Hello Timer = 30s. */

/**
 * @brief Class implements PIM Splitter, which splits PIM messages to correct PIM module.
 * @details This module is needed because we cannot distinguish PIM mode on layer 3, all of
 * them have same protocol number (103). PIM Splitter can resend PIM message to correct
 * PIM module according to configuration saved in PimInterfaceTable. Splitter also manages
 * PimNeighborTable.
 */
class PIMSplitter : public cSimpleModule, protected cListener
{
	private:
		IIPv4RoutingTable           	*rt;           	/**< Pointer to routing table. */
	    IInterfaceTable         	*ift;          	/**< Pointer to interface table. */

	    PIMInterfaceTable			*pimIft;		/**< Pointer to table of PIM interfaces. */
	    PIMNeighborTable			*pimNbt;		/**< Pointer to table of PIM neighbors. */

	    const char *				hostname;      	/**< Router hostname. */

	   void processPIMPkt(PIMPacket *pkt);

	   // methods for Hello packets
	   PIMHello* createHelloPkt(int iftID);
	   void sendHelloPkt();
	   void processHelloPkt(PIMPacket *pkt);

	   // process notification
       virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
	   virtual void newMulticast(IPv4Address destAddr, IPv4Address srcAddr);

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);

	public:
		PIMSplitter(){};
};


#endif


/**
 * @mainpage Multicast Routing Modelling in OMNeT++ Documentation
 *
 * This is programming documentation to thesis Multicast Routing Modelling in OMNeT++ by Veronika Rybova.
 * In this documentation you can see whole C++ programming part of the thesis. You do not find here description
 * of NED files and classes which are autogenerated from files PIMTimer.msg and PIMPacket.msg. The reason is that
 * the output of these files is big amount of classes and documentation would be overwhelmed.
 *
 * The thesis is part of project ANSA, which takes place at the Faculty of Information Technology, Brno University
 * of Technology. For more information visit: https://nes.fit.vutbr.cz/ansa/pmwiki.php
 */
