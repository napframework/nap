/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "pigpiopin.h"

RTTI_BEGIN_ENUM(nap::pigpio::EPinMode)
    RTTI_ENUM_VALUE(nap::pigpio::EPinMode::OUTPUT, "Output"),
    RTTI_ENUM_VALUE(nap::pigpio::EPinMode::INPUT, "Input"),
    RTTI_ENUM_VALUE(nap::pigpio::EPinMode::PWM_OUTPUT, "PWM Output"),
    RTTI_ENUM_VALUE(nap::pigpio::EPinMode::GPIO_CLOCK, "GPIO Clock")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::pigpio::PiGPIOPin)
    RTTI_PROPERTY("Pin", &nap::pigpio::PiGPIOPin::mPin, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Mode", &nap::pigpio::PiGPIOPin::mMode, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

using namespace nap::pigpio;

namespace nap
{
    PiGPIOPin::PiGPIOPin(PiGPIOService& service) : mService(service){}


    bool PiGPIOPin::init(utility::ErrorState& errorState)
    {
        if(!mService.registerPin(this, errorState))
        {
            return false;
        }

        mService.setPinMode(mPin, mMode);

        return true;
    }


    void PiGPIOPin::onDestroy()
    {
        mService.removePin(this);
    }


    void PiGPIOPin::setPinMode(EPinMode mode)
    {
        mService.setPinMode(mPin, mode);
    }


    void PiGPIOPin::setPwmValue(int value)
    {
        mService.setPwmValue(mPin, value);
    }


    void PiGPIOPin::setPullUpDnControl(int pud)
    {
        mService.setPullUpDnControl(mPin, pud);
    }


    EPinValue PiGPIOPin::getDigitalRead()
    {
        return mService.getDigitalRead(mPin);
    }


    void PiGPIOPin::setDigitalWrite(EPinValue value)
    {
        mService.setDigitalWrite(mPin, value);
    }


    int PiGPIOPin::getAnalogRead()
    {
        return mService.getAnalogRead(mPin);
    }


    void PiGPIOPin::setAnalogWrite(int value)
    {
        mService.setAnalogWrite(mPin, value);
    }
}