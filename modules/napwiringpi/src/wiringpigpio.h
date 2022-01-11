/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>

namespace nap
{
    namespace wiringpi
    {
        enum EPWMMode
        {
            PWM_MODE_MS	= 0,
            PWM_MODE_BAL = 1
        };

        enum EPinMode
        {
            INPUT = 0,
            OUTPUT = 1,
            PWM_OUTPUT = 2,
            GPIO_CLOCK = 3,
            SOFT_PWM_OUTPUT = 4,
            SOFT_TONE_OUTPUT = 5,
            PWM_TONE_OUTPUT = 6
        };

        enum EPinValue
        {
            LOW = 0,
            HIGH = 1
        };
    }

    class NAPAPI WiringPiGPIO : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        virtual bool init(utility::ErrorState& errorState) override;
    private:
    };
}
