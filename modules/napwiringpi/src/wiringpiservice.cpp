/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "wiringpiservice.h"

// Third party includes
#include <wiringPi.h>

#include <nap/logger.h>
#include <utility/stringutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WiringPiService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
    WiringPiService::WiringPiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

    bool WiringPiService::init(nap::utility::ErrorState& errorState)
    {
        mRun.store(true);
        mUpdateTask = std::async(std::launch::async, [this]
        {
            wiringPiSetupGpio() ;

            thread();
        });

        return true;
    }

    void WiringPiService::shutdown()
    {
        mRun.store(false);
        if (mUpdateTask.valid())
        {
            mUpdateTask.wait();
        }
    }
    
    
    void WiringPiService::registerObjectCreators(rtti::Factory& factory)
    {
    }


    void WiringPiService::setPwmMode(wiringpi::EPWMMode mode)
    {
        mQueue.enqueue([this, mode]()
        {
            pwmSetMode(mode);
        });
    }


    void WiringPiService::setPinMode(int pin, wiringpi::EPinMode mode)
    {
        mQueue.enqueue([this, pin, mode]()
        {
            pinMode(pin, (int) mode);
        });
    }


    void WiringPiService::setPwmFreq(int pin, int freq)
    {
        mQueue.enqueue([this, pin, freq]()
        {
            pwmToneWrite(pin, freq);
        });
    }


    wiringpi::EPinValue WiringPiService::getDigitalRead(int pin)
    {
        std::lock_guard<std::mutex> l(mMutex);
        return static_cast<wiringpi::EPinValue>(digitalRead(pin));
    }


    void WiringPiService::setDigitalWrite(int pin, wiringpi::EPinValue value)
    {
        mQueue.enqueue([this, pin, value]()
        {
            digitalWrite(pin, (int) value);
        });
    }


    void WiringPiService::setPwmValue(int pin, int value)
    {
        mQueue.enqueue([this, pin, value]()
        {

            pwmWrite(pin, value);
        });
    }


    void WiringPiService::thread()
    {
        while(mRun.load())
        {
            std::function<void()> action;
            while(mQueue.try_dequeue(action))
            {
                std::lock_guard<std::mutex> l(mMutex);
                action();
            }
        }
    }

}
