/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "wiringpigpiopin.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WiringPiGPIOPin)
    RTTI_PROPERTY("Pin", &nap::WiringPiGPIOPin::mPin, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    WiringPiGPIOPin::WiringPiGPIOPin(WiringPiService& service) : mService(service){}


    bool WiringPiGPIOPin::init(utility::ErrorState& errorState)
    {
        // TODO : check if pin is not already declared
        return true;
    }


    void WiringPiGPIOPin::setPinMode(wiringpi::EPinMode mode)
    {
        mService.setPinMode(mPin, mode);
    }


    void WiringPiGPIOPin::setPwmValue(int value)
    {
        mService.setPwmValue(mPin, value);
    }


    void WiringPiGPIOPin::setPwmFreq(int freq)
    {
        mService.setPwmFreq(mPin, freq);
    }


    wiringpi::EPinValue WiringPiGPIOPin::getDigitalRead()
    {
        return mService.getDigitalRead(mPin);
    }


    void WiringPiGPIOPin::setDigitalWrite(wiringpi::EPinValue value)
    {
        mService.setDigitalWrite(mPin, value);
    }
}