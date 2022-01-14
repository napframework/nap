/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "gpioservice.h"
#include "gpiopin.h"

// Third party includes
#include <wiringPi.h>

#include <nap/logger.h>
#include <utility/stringutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::pipins::GpioService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

using namespace nap::pipins;

namespace nap
{
    GpioService::GpioService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


    bool GpioService::init(nap::utility::ErrorState& errorState)
    {
        mRun.store(true);
        mUpdateTask = std::async(std::launch::async, [this]
        {
            wiringPiSetupGpio();

            thread();
        });

        return true;
    }


    void GpioService::setPwmRange(int range)
    {
        mQueue.enqueue([range]()
        {
            pwmSetRange(range);
        });
    }


    void GpioService::setPwmMode(EPWMMode mode)
    {
        mQueue.enqueue([mode]()
        {
            pwmSetMode(mode);
        });
    }


    void GpioService::setPwmClock(int divisor)
    {
        mQueue.enqueue([divisor]()
        {
            pwmSetClock(divisor);
        });
    }


    void GpioService::shutdown()
    {
        mRun.store(false);
        if (mUpdateTask.valid())
        {
            mUpdateTask.wait();
        }
    }


    void GpioService::registerObjectCreators(rtti::Factory& factory)
    {
        factory.addObjectCreator(std::make_unique<GpioPinObjectCreator>(*this));
    }


    bool GpioService::registerPin(const GpioPin* pin, utility::ErrorState& errorState)
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


    void GpioService::removePin(const GpioPin* pin)
    {
        auto it = mPins.find(pin->mPin);
        assert(it != mPins.end());
        mPins.erase(it);
    }


    void GpioService::setDigitalWrite(int pin, EPinValue value)
    {
        mQueue.enqueue([pin, value]()
        {
            digitalWrite(pin, static_cast<int>(value));
        });
    }


    void GpioService::setAnalogWrite(int pin, int value)
    {
        mQueue.enqueue([pin, value]()
        {
            analogWrite(pin, value);
        });
    }


    void GpioService::setPinMode(int pin, EPinMode mode)
    {
        mQueue.enqueue([pin, mode]()
        {
            pinMode(pin, static_cast<int>(mode));
        });
    }


    void GpioService::setPullUpDnControl(int pin, int pud)
    {
        mQueue.enqueue([pin, pud]()
        {
            pullUpDnControl(pin, pud);
        });
    }


    EPinValue GpioService::getDigitalRead(int pin)
    {
        std::lock_guard<std::mutex> l(mMutex);
        return static_cast<EPinValue>(digitalRead(pin));
    }


    int GpioService::getAnalogRead(int pin)
    {
        std::lock_guard<std::mutex> l(mMutex);
        return analogRead(pin);
    }


    void GpioService::setPwmValue(int pin, int value)
    {
        mQueue.enqueue([pin, value]()
        {
            pwmWrite(pin, value);
        });
    }


    void GpioService::thread()
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
