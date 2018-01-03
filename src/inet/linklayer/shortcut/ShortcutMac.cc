//
// Copyright (C) OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
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

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/shortcut/ShortcutMac.h"

namespace inet {

std::map<MacAddress, ShortcutMac *> ShortcutMac::shortcutMacs;

Define_Module(ShortcutMac);

void ShortcutMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        bitrate = par("bitrate");
        propagationDelay = &par("propagationDelay");
        lengthOverhead = &par("lengthOverhead");
        durationOverhead = &par("durationOverhead");
        packetLoss = &par("packetLoss");
        initializeMacAddress();
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerInterface();
    }
}

void ShortcutMac::initializeMacAddress()
{
    const char *addressString = par("address");
    if (!strcmp(addressString, "auto"))
        address = MacAddress::generateAutoAddress();
    else
        address.setAddress(addressString);
    shortcutMacs[address] = this;
}

InterfaceEntry *ShortcutMac::createInterfaceEntry()
{
    auto interfaceEntry = getContainingNicModule(this);
    interfaceEntry->setDatarate(bitrate);
    interfaceEntry->setMACAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
    interfaceEntry->setMulticast(false);
    interfaceEntry->setBroadcast(true);
    return interfaceEntry;
}

void ShortcutMac::handleMessageWhenUp(cMessage *message)
{
    if (message->getArrivalGate() == gate("peerIn"))
        receiveFromPeer(check_and_cast<Packet *>(message));
    else
        MacProtocolBase::handleMessageWhenUp(message);
}

void ShortcutMac::handleUpperPacket(Packet *packet)
{
    auto destination = packet->_getTag<MacAddressReq>()->getDestAddress();
    if (destination.isBroadcast()) {
        for (auto it : shortcutMacs)
            if (it.second != this)
                sendToPeer(packet->dup(), it.second);
        delete packet;
    }
    else {
        auto peer = findPeer(destination);
        if (peer != nullptr)
            sendToPeer(packet, peer);
        else
            throw cRuntimeError("ShortcutMac not found");
    }
}

void ShortcutMac::handleLowerPacket(Packet *packet)
{
    throw cRuntimeError("Received lower packet is unexpected");
}

ShortcutMac *ShortcutMac::findPeer(MacAddress address)
{
    auto it = shortcutMacs.find(address);
    if (it == shortcutMacs.end())
        return nullptr;
    else
        return it->second;
}

void ShortcutMac::sendToPeer(Packet *packet, ShortcutMac *peer)
{
    if (dblrand() < packetLoss->doubleValue()) {
        EV_WARN << "Packet " << packet << " lost.";
        delete packet;
    }
    else {
        auto length = b(lengthOverhead->intValue());
        if (length != b(0))
            packet->insertHeader(makeShared<BitCountChunk>(length));
        simtime_t transmissionDuration = packet->getBitLength() / bitrate + durationOverhead->doubleValue();
        packet->setDuration(transmissionDuration);
        sendDirect(packet, propagationDelay->doubleValue(), transmissionDuration, peer->gate("peerIn"));
    }
}

void ShortcutMac::receiveFromPeer(Packet *packet)
{
    auto protocol = packet->_getTag<PacketProtocolTag>()->getProtocol();
    packet->_clearTags();
    packet->_addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    packet->_addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    packet->popHeader<BitCountChunk>();
    sendUp(packet);
}

} // namespace inet

