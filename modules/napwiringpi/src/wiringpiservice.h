/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <set>
#include <future>

// Nap includes
#include <nap/service.h>
#include <concurrentqueue.h>

// Internal Includes
#include "wiringpienums.h"

namespace nap
{
    class NAPAPI WiringPiService : public nap::Service
    {
        friend class WiringPiGPIOPin;

        RTTI_ENABLE(nap::Service)

    public:
        WiringPiService(ServiceConfiguration* configuration);
        
        // Initialization
        bool init(nap::utility::ErrorState& errorState) override;

        void setPwmMode(wiringpi::EPWMMode mode);
    protected:
        void registerObjectCreators(rtti::Factory& factory) override final;

        void shutdown() override;
    private:
        void setPinMode(int pin, wiringpi::EPinMode mode);

        void setPwmValue(int pin, int value);

        void setPwmFreq(int pin, int freq);

        wiringpi::EPinValue getDigitalRead(int pin);

        void setDigitalWrite(int pin, wiringpi::EPinValue value);

        void thread();

        std::mutex mMutex;
        moodycamel::ConcurrentQueue<std::function<void()>> mQueue;

        std::future<void> mUpdateTask;

        std::atomic_bool mRun;
    };
        
}
