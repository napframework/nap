#pragma once

namespace nap
{

namespace pipins
{
    /**
     * This sets the pull-up or pull-down resistor mode on the given pin, which should be set as an input.
     * Unlike the Arduino, the BCM2835 has both pull-up an down internal resistors.
     * The parameter pud should be; PUD_OFF, (no pull up/down), PUD_DOWN (pull to ground) or PUD_UP (pull to 3.3v)
     * The internal pull up/down resistors have a value of approximately 50KΩ on the Raspberry Pi.
     */
    enum EPUDMode
    {
        PUD_OFF     = 0,
        PUD_DOWN    = 1,
        PUD_UP      = 2
    };

    /**
     * The PWM generator can run in 2 modes – “balanced” and “mark:space”.
     * The mark:space mode is traditional, however the default mode in the Pi is “balanced”.
     * You can switch modes by supplying the parameter: PWM_MODE_BAL or PWM_MODE_MS.
     */
    enum EPWMMode
    {
        PWM_MODE_MS	    = 0,
        PWM_MODE_BAL    = 1
    };

    /**
     * Enum of available pi pin modes.
     * Can be either INPUT, OUTPUT, PWM_OUTPUT or GPIO_CLOCK.
     */
    enum EPinMode
    {
        INPUT           = 0,
        OUTPUT          = 1,
        PWM_OUTPUT      = 2,
        GPIO_CLOCK      = 3
    };

    /**
     * Value for digital pin is HIGH or LOW (1 or 0).
     * WiringPi treats any non-zero number as HIGH, however 0 is the only representation of LOW.
     */
    enum EDigitalPinValue
    {
        LOW = 0,
        HIGH = 1
    };
}

}
