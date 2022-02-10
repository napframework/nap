/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "gpiopin.h"

#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::pipins::EPinMode)
    RTTI_ENUM_VALUE(nap::pipins::EPinMode::OUTPUT, "Output"),
    RTTI_ENUM_VALUE(nap::pipins::EPinMode::INPUT, "Input"),
    RTTI_ENUM_VALUE(nap::pipins::EPinMode::PWM_OUTPUT, "PWM Output"),
    RTTI_ENUM_VALUE(nap::pipins::EPinMode::GPIO_CLOCK, "GPIO Clock")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::pipins::GpioPin)
    RTTI_PROPERTY("Pin", &nap::pipins::GpioPin::mPin, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Mode", &nap::pipins::GpioPin::mMode, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

using namespace nap::pipins;

namespace nap
{
    GpioPin::GpioPin(GpioService& service) : mService(service){}


    bool GpioPin::init(utility::ErrorState& errorState)
    {
        if(!mService.registerPin(this, errorState))
        {
            return false;
        }

        mService.setPinMode(mPin, mMode);

        return true;
    }


    void GpioPin::onDestroy()
    {
        mService.removePin(this);
    }


    void GpioPin::setPinMode(EPinMode mode)
    {
        mService.setPinMode(mPin, mode);
    }


    void GpioPin::setPwmValue(int value)
    {
        mService.setPwmValue(mPin, value);
    }


    void GpioPin::setPullUpDnControl(EPUDMode pud)
    {
        mService.setPullUpDnControl(mPin, pud);
    }


    EDigitalPinValue GpioPin::getDigitalRead()
    {
        return mService.getDigitalRead(mPin);
    }


    void GpioPin::setDigitalWrite(EDigitalPinValue value)
    {
        mService.setDigitalWrite(mPin, value);
    }


    int GpioPin::getAnalogRead()
    {
        return mService.getAnalogRead(mPin);
    }


    void GpioPin::setAnalogWrite(int value)
    {
        value = math::clamp<int>(value, 0, 255);
        mService.setAnalogWrite(mPin, value);
    }
}
