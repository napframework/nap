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
#include "pigpioenums.h"

namespace nap
{

namespace pigpio
{
    class PiGPIOPin;

    class NAPAPI PiGPIOService : public nap::Service
    {
        friend class PiGPIOPin;

        RTTI_ENABLE(nap::Service)
    public:
        /**
         * Constructor
         * @param configuration service configuration
         */
        PiGPIOService(ServiceConfiguration* configuration);

        // Initialization
        bool init(nap::utility::ErrorState& errorState) override;

        /**
         * The PWM generator can run in 2 modes – “balanced” and “mark:space”.
         * The mark:space mode is traditional, however the default mode in the Pi is “balanced”.
         * You can switch modes by supplying the parameter: PWM_MODE_BAL or PWM_MODE_MS.
         * @param mode
         */
        void setPwmMode(EPWMMode mode);

        /**
         * This sets the range register in the PWM generator. The default is 1024.
         * @param range value between 0-1024
         */
        void setPwmRange(int range);

        /**
         * This sets the divisor for the PWM clock.
         * @param divisor
         */
        void setPwmClock(int divisor);
    protected:
        /**
         * Registers the object creator for PiGPIOPin
         * @param factory
         */
        void registerObjectCreators(rtti::Factory& factory) override final;

        /**
         * Shutdown is called on exit before deconstruction
         */
        void shutdown() override;

    private:
        /**
         * Registers a PiGPIOPin resource and checks if one is not already declared
         * @param pin const pointer to PiGPIOPin resource
         * @param errorState errorState to write error in
         * @return true on success
         */
        bool registerPin(const PiGPIOPin* pin, utility::ErrorState& errorState);

        /**
         * Removes a registered PiGPIO pin resource
         */
        void removePin(const PiGPIOPin* pin);

        /**
         * Writes the value HIGH or LOW (1 or 0) to the given pin which must have been previously set as an output.
         * @param pin the pin
         * @param value the value HIGH or LOW (1 or 0) depending on the logic level at the pin.
         */
        void setDigitalWrite(int pin, EPinValue value);

        /**
         * This writes the given value to the supplied analog pin.
         * @param pin the pin
         * @param value the value
         */
        void setAnalogWrite(int pin, int value);

        /**
         * This sets the mode of a pin to either INPUT, OUTPUT, PWM_OUTPUT or GPIO_CLOCK.
         * Note that only wiringPi pin 1 (BCM_GPIO 18) supports PWM output and only wiringPi pin 7 (BCM_GPIO 4) supports CLOCK output modes.
         * @param pin the pin
         * @param mode the mode
         */
        void setPinMode(int pin, EPinMode mode);

        /**
         * This sets the pull-up or pull-down resistor mode on the given pin, which should be set as an input.
         * Unlike the Arduino, the BCM2835 has both pull-up an down internal resistors.
         * The parameter pud should be; PUD_OFF, (no pull up/down), PUD_DOWN (pull to ground) or PUD_UP (pull to 3.3v)
         * The internal pull up/down resistors have a value of approximately 50KΩ on the Raspberry Pi.
         * @param pin the pin
         * @param pud PUD_OFF, (no pull up/down), PUD_DOWN (pull to ground) or PUD_UP (pull to 3.3v)
         */
        void setPullUpDnControl(int pin, int pud);

        /**
         * Writes the value to the PWM register for the given pin.
         * The Raspberry Pi has one on-board PWM pin, pin 1 (BMC_GPIO 18, Phys 12) and the range is 0-1024.
         * Other PWM devices may have other PWM ranges.
         * @param pin the pin
         * @param value the value
         */
        void setPwmValue(int pin, int value);

        /**
         * This function returns the value read at the given pin.
         * It will be HIGH or LOW (1 or 0) depending on the logic level at the pin.
         * @param pin the value
         * @return will be HIGH or LOW (1 or 0) depending on the logic level at the pin.
         */
        EPinValue getDigitalRead(int pin);

        /**
         * This returns the value read on the supplied analog input pin.
         * @param pin the pin
         * @return the value of the analog pin
         */
        int getAnalogRead(int pin);

        /**
         * The threaded function
         */
        void thread();

        // mutex
        std::mutex mMutex;

        // queue can of actions that will be handled on thread
        moodycamel::ConcurrentQueue<std::function<void()>> mQueue;

        // the task
        std::future<void> mUpdateTask;

        // boolean set to false to make pigpio thread stop running
        std::atomic_bool mRun;

        // map of registered pins with pin number as key value
        std::unordered_map<int, const PiGPIOPin*> mPins;
    };
} // end namespace pigpio

} // end namespace nap
