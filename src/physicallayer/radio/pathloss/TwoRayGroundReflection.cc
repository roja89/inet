/***************************************************************************
 * author:      Andreas Kuntz
 *
 * copyright:   (c) 2008 Institute of Telematics, University of Karlsruhe (TH)
 *
 * author:      Alfonso Ariza
 *              Malaga university
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************/

#include "TwoRayGroundReflection.h"

Register_Class(TwoRayGroundReflection);

TwoRayGroundReflection::TwoRayGroundReflection() :
    ht(m(0)),
    hr(m(0))
{
}

void TwoRayGroundReflection::initialize(int stage)
{
    FreeSpacePathLoss::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        ht = m(par("TransmiterAntennaHigh"));
        hr = m(par("ReceiverAntennaHigh"));
    }
}

void TwoRayGroundReflection::printToStream(std::ostream &stream) const
{
    stream << "two ray ground reflection, "
           << "alpha = " << alpha << ", "
           << "system loss = " << systemLoss << ", "
           << "transmitter antenna high = " << ht << ", "
           << "receiver antenna high = " << hr;
}

double TwoRayGroundReflection::computePathLoss(mps propagationSpeed, Hz carrierFrequency, m distance) const
{
    m waveLength = propagationSpeed / carrierFrequency;
    /**
     * At the cross over distance two ray model and free space model predict the same power
     *
     *                        4 * pi * hr * ht
     *   crossOverDistance = ------------------
     *                           waveLength
     */
    m crossOverDistance = (4 * M_PI * ht * hr) / waveLength;
    if (distance < crossOverDistance)
        return computeFreeSpacePathLoss(waveLength, distance, alpha, systemLoss);
    else
        /**
         * Two-ray ground reflection model.
         *
         *         (ht ^ 2 * hr ^ 2)
         * loss = ---------------
         *            d ^ 4 * L
         *
         * To be consistent with the free space equation, L is added here.
         * The original equation in Rappaport's book assumes L = 1.
         */
        return unit((ht * ht * hr * hr) / (distance * distance * distance * distance * systemLoss)).get();
}
