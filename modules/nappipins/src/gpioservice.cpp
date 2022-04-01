/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "gpioservice.h"
#include "gpiopin.h"

// Third party includes
#include <wiringPi.h>

// External includes
#include <stdlib.h>

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
        // we need the application to be run as root
        if(!errorState.check(!getuid(), "You are not running as root! GpioService needs root access. Try running as root or sudo..."))
            return false;

        // set environment variables so wiring pi setup returns an exit code, see following Gordon's comment
        /**
         * Note: wiringPi version 1 returned an error code if these functions failed for whatever reason.
         * Version 2 returns always returns zero. After discussions and inspection of many programs written by users of
         * wiringPi and observing that many people don’t bother checking the return code, I took the stance that should
         * one of the wiringPi setup functions fail, then it would be considered a fatal program fault and the program
         * execution will be terminated at that point with an error message printed on the terminal.
         * If you want to restore the v1 behaviour, then you need to set the environment variable: WIRINGPI_CODES (to
         * any value, it just needs to exist)
         */
        setenv("WIRINGPI_CODES", "1", 1);

        /**
         * This initialises wiringPi and assumes that the calling program is going to be using the broadcom pin numbering scheme.
         */
        int exit_code = wiringPiSetupGpio();
        if(!errorState.check(exit_code==0, "wiringPiSetup failed with exit code %i", exit_code))
            return false;

        // start thread
        mRun.store(true);
        mUpdateTask = std::async(std::launch::async, [this]
        {
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


    void GpioService::setDigitalWrite(int pin, EDigitalPinValue value)
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


    void GpioService::setPullUpDnControl(int pin, EPUDMode pud)
    {
        mQueue.enqueue([pin, pud]()
        {
            pullUpDnControl(pin, static_cast<int>(pud));
        });
    }


    EDigitalPinValue GpioService::getDigitalRead(int pin)
    {
        std::lock_guard<std::mutex> l(mMutex);
        return static_cast<EDigitalPinValue>(digitalRead(pin));
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
