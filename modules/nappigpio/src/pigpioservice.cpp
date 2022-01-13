/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "pigpioservice.h"
#include "pigpiopin.h"

// Third party includes
#include <wiringPi.h>

#include <nap/logger.h>
#include <utility/stringutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::pigpio::PiGPIOService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

using namespace nap::pigpio;

namespace nap
{
    PiGPIOService::PiGPIOService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


    bool PiGPIOService::init(nap::utility::ErrorState& errorState)
    {
        mRun.store(true);
        mUpdateTask = std::async(std::launch::async, [this]
        {
            wiringPiSetupGpio();

            thread();
        });

        return true;
    }


    void PiGPIOService::setPwmRange(int range)
    {
        mQueue.enqueue([range]()
        {
            pwmSetRange(range);
        });
    }


    void PiGPIOService::setPwmMode(EPWMMode mode)
    {
        mQueue.enqueue([mode]()
        {
            pwmSetMode(mode);
        });
    }


    void PiGPIOService::setPwmClock(int divisor)
    {
        mQueue.enqueue([divisor]()
        {
            pwmSetClock(divisor);
        });
    }


    void PiGPIOService::shutdown()
    {
        mRun.store(false);
        if (mUpdateTask.valid())
        {
            mUpdateTask.wait();
        }
    }


    void PiGPIOService::registerObjectCreators(rtti::Factory& factory)
    {
        factory.addObjectCreator(std::make_unique<PiGPIOPinObjectCreator>(*this));
    }


    bool PiGPIOService::registerPin(const PiGPIOPin* pin, utility::ErrorState& errorState)
    {
        auto it = mPins.find(pin->mPin);
        if(it != mPins.end())
        {
            errorState.fail("pin number %i already registered", pin->mPin);
            return false;
        }

        mPins.emplace(pin->mPin, pin);
        return true;
    }


    void PiGPIOService::removePin(const PiGPIOPin* pin)
    {
        auto it = mPins.find(pin->mPin);
        assert(it != mPins.end());
        mPins.erase(it);
    }


    void PiGPIOService::setDigitalWrite(int pin, EPinValue value)
    {
        mQueue.enqueue([pin, value]()
        {
            digitalWrite(pin, static_cast<int>(value));
        });
    }


    void PiGPIOService::setAnalogWrite(int pin, int value)
    {
        mQueue.enqueue([pin, value]()
        {
            analogWrite(pin, value);
        });
    }


    void PiGPIOService::setPinMode(int pin, EPinMode mode)
    {
        mQueue.enqueue([pin, mode]()
        {
            pinMode(pin, static_cast<int>(mode));
        });
    }


    void PiGPIOService::setPullUpDnControl(int pin, int pud)
    {
        mQueue.enqueue([pin, pud]()
        {
            pullUpDnControl(pin, pud);
        });
    }


    EPinValue PiGPIOService::getDigitalRead(int pin)
    {
        std::lock_guard<std::mutex> l(mMutex);
        return static_cast<EPinValue>(digitalRead(pin));
    }


    int PiGPIOService::getAnalogRead(int pin)
    {
        std::lock_guard<std::mutex> l(mMutex);
        return analogRead(pin);
    }


    void PiGPIOService::setPwmValue(int pin, int value)
    {
        mQueue.enqueue([pin, value]()
        {
            pwmWrite(pin, value);
        });
    }


    void PiGPIOService::thread()
    {
        while(mRun.load())
        {
            while(mQueue.size_approx()>0)
            {
                std::function<void()> action;
                if(mQueue.try_dequeue(action))
                {
                    std::lock_guard<std::mutex> l(mMutex);
                    action();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
