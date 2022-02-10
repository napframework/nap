/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Internal Includes
#include "gpioservice.h"

// External Includes
#include <nap/resource.h>

namespace nap
{

namespace pipins
{
    /**
     * GpioPin can be used to read or write to a GPIO pin on the Raspberry PI. You can set the pin number by setting
     * the "Pin" property and set its mode by setting the "Mode" property.
     * The GpioService uses "wiringPiSetupGpio()" which means the service uses the Broadcom GPIO pin numbers directly
     * with no re-mapping.
     * Write calls will always be asynchronous and not block the calling thread. Read calls are synchronous and perform
     * a mutex lock to ensure thread-safety.
     * See https://pinout.xyz/ to help with mapping.
     * The PiGPIOPin wraps the core functionality of wiringPi, see : http://wiringpi.com/reference/
     */
    class NAPAPI GpioPin : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        /**
         * Constructor
         * @param service reference to GpioService
         */
        GpioPin(GpioService& service);

        // Initialization
        bool init(utility::ErrorState& errorState) override;

        /**
         * Called upon destruction before destructor
         */
        void onDestroy() override;

        /**
         * This sets the mode of a pin to either INPUT, OUTPUT, PWM_OUTPUT or GPIO_CLOCK.
         * Note that only wiringPi pin 1 (BCM_GPIO 18) supports PWM output and only wiringPi pin 7 (BCM_GPIO 4) supports CLOCK output modes.
         * @param mode the mode
         */
        void setPinMode(EPinMode mode);

        /**
         * This sets the pull-up or pull-down resistor mode on the given pin, which should be set as an input.
         * Unlike the Arduino, the BCM2835 has both pull-up an down internal resistors.
         * The parameter pud should be; PUD_OFF, (no pull up/down), PUD_DOWN (pull to ground) or PUD_UP (pull to 3.3v)
         * The internal pull up/down resistors have a value of approximately 50KΩ on the Raspberry Pi.
         * @param pud PUD_OFF, (no pull up/down), PUD_DOWN (pull to ground) or PUD_UP (pull to 3.3v)
         */
        void setPullUpDnControl(EPUDMode pud);

        /**
         * Writes the value to the PWM register for the given pin.
         * The Raspberry Pi has one on-board PWM pin, pin 1 (BMC_GPIO 18, Phys 12) and the range is 0-1024.
         * Other PWM devices may have other PWM ranges.
         * @param value the value between 0-1024
         */
        void setPwmValue(int value);

        /**
         * This function returns the value read at the given pin.
         * It will be HIGH or LOW (1 or 0) depending on the logic level at the pin.
         * @return will be HIGH or LOW (1 or 0) depending on the logic level at the pin.
         */
        EDigitalPinValue getDigitalRead();

        /**
         * This returns the value read on the supplied analog input pin.
         * The analog input and output are 8-bit devices, so have a range of 0 to 255 when we read/write them.
         * @return the value of the analog pin between 0-255
         */
        int getAnalogRead();

        /**
         * Writes the value HIGH or LOW (1 or 0) to the given pin which must have been previously set as an output.
         * @param value the value HIGH or LOW (1 or 0) depending on the logic level at the pin.
         */
        void setDigitalWrite(EDigitalPinValue value);

        /**
         * This writes the given value to the supplied analog pin.
         * The analog input and output are 8-bit devices, so have a range of 0 to 255 when we read/write them.
         * @param value the value between 0-255
         */
        void setAnalogWrite(int value);

        // properties
        int mPin = 0; ///< Property: 'Pin' the Pin number
        EPinMode mMode = EPinMode::OUTPUT; ///< Property: 'Mode' Pin mode
    private:
        GpioService& mService;
    };

    using GpioPinObjectCreator = rtti::ObjectCreator<GpioPin, GpioService>;
} // end namespace pipins

} // end namespace nap

