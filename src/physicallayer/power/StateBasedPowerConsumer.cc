//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "StateBasedPowerConsumer.h"
#include "ModuleAccess.h"

namespace inet {

namespace physicallayer {

Define_Module(StateBasedPowerConsumer);

StateBasedPowerConsumer::StateBasedPowerConsumer() :
    offPowerConsumption(W(sNaN)),
    sleepPowerConsumption(W(sNaN)),
    switchingPowerConsumption(W(sNaN)),
    receiverIdlePowerConsumption(W(sNaN)),
    receiverBusyPowerConsumption(W(sNaN)),
    receiverSynchronizingPowerConsumption(W(sNaN)),
    receiverReceivingPowerConsumption(W(sNaN)),
    transmitterIdlePowerConsumption(W(sNaN)),
    transmitterTransmittingPowerConsumption(W(sNaN)),
    radio(NULL),
    powerSource(NULL),
    powerConsumerId(-1)
{
}

void StateBasedPowerConsumer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV << "Initializing RadioPowerConsumer, stage = " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        offPowerConsumption = W(par("offPowerConsumption"));
        sleepPowerConsumption = W(par("sleepPowerConsumption"));
        switchingPowerConsumption = W(par("switchingPowerConsumption"));
        receiverIdlePowerConsumption = W(par("receiverIdlePowerConsumption"));
        receiverBusyPowerConsumption = W(par("receiverBusyPowerConsumption"));
        receiverSynchronizingPowerConsumption = W(par("receiverSynchronizingPowerConsumption"));
        receiverReceivingPowerConsumption = W(par("receiverReceivingPowerConsumption"));
        transmitterIdlePowerConsumption = W(par("transmitterIdlePowerConsumption"));
        transmitterTransmittingPowerConsumption = W(par("transmitterTransmittingPowerConsumption"));
        cModule *radioModule = getParentModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
        const char *powerSourceModule = par("powerSourceModule");
        powerSource = dynamic_cast<IPowerSource *>(getModuleByPath(powerSourceModule));
        if (!powerSource)
            throw cRuntimeError("Cannot find power source");
        powerConsumerId = powerSource->addPowerConsumer(this);
    }
}

void StateBasedPowerConsumer::receiveSignal(cComponent *source, simsignal_t signalID, long value)
{
    if (signalID == IRadio::radioModeChangedSignal || signalID == IRadio::receptionStateChangedSignal || signalID == IRadio::transmissionStateChangedSignal)
        powerSource->setPowerConsumption(powerConsumerId, getPowerConsumption());
}

W StateBasedPowerConsumer::getPowerConsumption()
{
    IRadio::RadioMode radioMode = radio->getRadioMode();
    if (radioMode == IRadio::RADIO_MODE_OFF)
        return offPowerConsumption;
    else if (radioMode == IRadio::RADIO_MODE_SLEEP)
        return sleepPowerConsumption;
    else if (radioMode == IRadio::RADIO_MODE_SWITCHING)
        return switchingPowerConsumption;
    W powerConsumption = W(0);
    IRadio::ReceptionState receptionState = radio->getReceptionState();
    IRadio::TransmissionState transmissionState = radio->getTransmissionState();
    if (radioMode == IRadio::RADIO_MODE_RECEIVER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (receptionState == IRadio::RECEPTION_STATE_IDLE)
            powerConsumption += receiverIdlePowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_BUSY)
            powerConsumption += receiverBusyPowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_SYNCHRONIZING)
            powerConsumption += receiverSynchronizingPowerConsumption;
        else if (receptionState == IRadio::RECEPTION_STATE_RECEIVING)
            powerConsumption += receiverReceivingPowerConsumption;
        else if (receptionState != IRadio::RECEPTION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio reception state");
    }
    if (radioMode == IRadio::RADIO_MODE_TRANSMITTER || radioMode == IRadio::RADIO_MODE_TRANSCEIVER) {
        if (transmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            powerConsumption += transmitterIdlePowerConsumption;
        else if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING)
            powerConsumption += transmitterTransmittingPowerConsumption;
        else if (transmissionState != IRadio::TRANSMISSION_STATE_UNDEFINED)
            throw cRuntimeError("Unknown radio transmission state");
    }
    return powerConsumption;
}

} // namespace physicallayer

} // namespace inet

