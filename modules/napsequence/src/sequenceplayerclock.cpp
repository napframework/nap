/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayerclock.h"
#include "sequenceservice.h"

RTTI_BEGIN_CLASS(nap::SequencePlayerIndependentClock)
        RTTI_PROPERTY("Frequency", &nap::SequencePlayerIndependentClock::mFrequency, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerStandardClock)
RTTI_END_CLASS

namespace nap
{
    SequencePlayerStandardClock::SequencePlayerStandardClock(SequenceService &service)
        : mService(service)
    {
    }


    void SequencePlayerStandardClock::start(Slot<double> &updateSlot)
    {
        mSlot = updateSlot;
        mService.registerStandardClock(this);
    }


    void SequencePlayerStandardClock::stop()
    {
        mService.unregisterStandardClock(this);
    }


    void SequencePlayerStandardClock::update(double deltaTime)
    {
        mSlot.trigger(deltaTime);
    }


    bool SequencePlayerIndependentClock::init(utility::ErrorState &errorState)
    {
        if(!errorState.check(mFrequency > 0.0f, "Frequency must be bigger then zero!"))
        {
            return false;
        }

        return true;
    }


    void SequencePlayerIndependentClock::start(Slot<double> &updateSlot)
    {
        mRunning.store(true);
        mSlot = updateSlot;
        mUpdateTask = std::async(std::launch::async, [this]
        {
            onUpdate();
        });
    }


    void SequencePlayerIndependentClock::stop()
    {
        // stop running thread
        mRunning.store(false);
        if(mUpdateTask.valid())
        {
            mUpdateTask.wait();
        }
    }


    void SequencePlayerIndependentClock::onUpdate()
    {
        // Compute sleep time in microseconds
        float sleep_time_microf = 1000.0f / static_cast<float>(mFrequency);
        long sleep_time_micro = static_cast<long>(sleep_time_microf * 1000.0f);

        while(mRunning.load())
        {
            // advance time
            SteadyTimeStamp now = SteadyClock::now();
            double delta_time = std::chrono::duration<double>(now - mBefore).count();
            mBefore = now;

            mSlot.trigger(delta_time);

            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_micro));
        }
    }
}