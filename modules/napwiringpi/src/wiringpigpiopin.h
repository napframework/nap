/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Internal Includes
#include "wiringpiservice.h"

// External Includes
#include <nap/resource.h>

namespace nap
{
    class NAPAPI WiringPiGPIOPin : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        WiringPiGPIOPin(WiringPiService& service);

        bool init(utility::ErrorState& errorState) override;

        void setPinMode(wiringpi::EPinMode mode);

        void setPwmValue(int value);

        void setPwmFreq(int freq);

        wiringpi::EPinValue getDigitalRead();

        void setDigitalWrite(wiringpi::EPinValue value);

        // properties
        int mPin = 0;
    private:
        WiringPiService& mService;
    };

    using WiringPiGPIOPinObjectCreator = rtti::ObjectCreator<WiringPiGPIOPin, WiringPiService>;
}
