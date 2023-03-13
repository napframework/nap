/*********************************************************************
 *
 * $Id: yocto_anbutton.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindAnButton(), the high-level API for AnButton functions
 *
 * - - - - - - - - - License information: - - - - - - - - -
 *
 *  Copyright (C) 2011 and beyond by Yoctopuce Sarl, Switzerland.
 *
 *  Yoctopuce Sarl (hereafter Licensor) grants to you a perpetual
 *  non-exclusive license to use, modify, copy and integrate this
 *  file into your software for the sole purpose of interfacing
 *  with Yoctopuce products.
 *
 *  You may reproduce and distribute copies of this file in
 *  source or object form, as long as the sole purpose of this
 *  code is to interface with Yoctopuce products. You must retain
 *  this notice in the distributed source file.
 *
 *  You should refer to Yoctopuce General Terms and Conditions
 *  for additional information regarding your rights and
 *  obligations.
 *
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED 'AS IS' WITHOUT
 *  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 *  WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO
 *  EVENT SHALL LICENSOR BE LIABLE FOR ANY INCIDENTAL, SPECIAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 *  COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR
 *  SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT
 *  LIMITED TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR
 *  CONTRIBUTION, OR OTHER SIMILAR COSTS, WHETHER ASSERTED ON THE
 *  BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE), BREACH OF
 *  WARRANTY, OR OTHERWISE.
 *
 *********************************************************************/


#ifndef YOCTO_ANBUTTON_H
#define YOCTO_ANBUTTON_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YAnButton return codes)
//--- (end of YAnButton return codes)
//--- (YAnButton definitions)
class YAnButton; // forward declaration

typedef void (*YAnButtonValueCallback)(YAnButton *func, const string& functionValue);
#ifndef _Y_ANALOGCALIBRATION_ENUM
#define _Y_ANALOGCALIBRATION_ENUM
typedef enum {
    Y_ANALOGCALIBRATION_OFF = 0,
    Y_ANALOGCALIBRATION_ON = 1,
    Y_ANALOGCALIBRATION_INVALID = -1,
} Y_ANALOGCALIBRATION_enum;
#endif
#ifndef _Y_ISPRESSED_ENUM
#define _Y_ISPRESSED_ENUM
typedef enum {
    Y_ISPRESSED_FALSE = 0,
    Y_ISPRESSED_TRUE = 1,
    Y_ISPRESSED_INVALID = -1,
} Y_ISPRESSED_enum;
#endif
#define Y_CALIBRATEDVALUE_INVALID       (YAPI_INVALID_UINT)
#define Y_RAWVALUE_INVALID              (YAPI_INVALID_UINT)
#define Y_CALIBRATIONMAX_INVALID        (YAPI_INVALID_UINT)
#define Y_CALIBRATIONMIN_INVALID        (YAPI_INVALID_UINT)
#define Y_SENSITIVITY_INVALID           (YAPI_INVALID_UINT)
#define Y_LASTTIMEPRESSED_INVALID       (YAPI_INVALID_LONG)
#define Y_LASTTIMERELEASED_INVALID      (YAPI_INVALID_LONG)
#define Y_PULSECOUNTER_INVALID          (YAPI_INVALID_LONG)
#define Y_PULSETIMER_INVALID            (YAPI_INVALID_LONG)
//--- (end of YAnButton definitions)

//--- (YAnButton declaration)
/**
 * YAnButton Class: AnButton function interface
 *
 * Yoctopuce application programming interface allows you to measure the state
 * of a simple button as well as to read an analog potentiometer (variable resistance).
 * This can be use for instance with a continuous rotating knob, a throttle grip
 * or a joystick. The module is capable to calibrate itself on min and max values,
 * in order to compute a calibrated value that varies proportionally with the
 * potentiometer position, regardless of its total resistance.
 */
class YOCTO_CLASS_EXPORT YAnButton: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YAnButton declaration)
protected:
    //--- (YAnButton attributes)
    // Attributes (function value cache)
    int             _calibratedValue;
    int             _rawValue;
    Y_ANALOGCALIBRATION_enum _analogCalibration;
    int             _calibrationMax;
    int             _calibrationMin;
    int             _sensitivity;
    Y_ISPRESSED_enum _isPressed;
    s64             _lastTimePressed;
    s64             _lastTimeReleased;
    s64             _pulseCounter;
    s64             _pulseTimer;
    YAnButtonValueCallback _valueCallbackAnButton;

    friend YAnButton *yFindAnButton(const string& func);
    friend YAnButton *yFirstAnButton(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindAnButton factory function to instantiate
    YAnButton(const string& func);
    //--- (end of YAnButton attributes)

public:
    ~YAnButton();
    //--- (YAnButton accessors declaration)

    static const int CALIBRATEDVALUE_INVALID = YAPI_INVALID_UINT;
    static const int RAWVALUE_INVALID = YAPI_INVALID_UINT;
    static const Y_ANALOGCALIBRATION_enum ANALOGCALIBRATION_OFF = Y_ANALOGCALIBRATION_OFF;
    static const Y_ANALOGCALIBRATION_enum ANALOGCALIBRATION_ON = Y_ANALOGCALIBRATION_ON;
    static const Y_ANALOGCALIBRATION_enum ANALOGCALIBRATION_INVALID = Y_ANALOGCALIBRATION_INVALID;
    static const int CALIBRATIONMAX_INVALID = YAPI_INVALID_UINT;
    static const int CALIBRATIONMIN_INVALID = YAPI_INVALID_UINT;
    static const int SENSITIVITY_INVALID = YAPI_INVALID_UINT;
    static const Y_ISPRESSED_enum ISPRESSED_FALSE = Y_ISPRESSED_FALSE;
    static const Y_ISPRESSED_enum ISPRESSED_TRUE = Y_ISPRESSED_TRUE;
    static const Y_ISPRESSED_enum ISPRESSED_INVALID = Y_ISPRESSED_INVALID;
    static const s64 LASTTIMEPRESSED_INVALID = YAPI_INVALID_LONG;
    static const s64 LASTTIMERELEASED_INVALID = YAPI_INVALID_LONG;
    static const s64 PULSECOUNTER_INVALID = YAPI_INVALID_LONG;
    static const s64 PULSETIMER_INVALID = YAPI_INVALID_LONG;

    /**
     * Returns the current calibrated input value (between 0 and 1000, included).
     *
     * @return an integer corresponding to the current calibrated input value (between 0 and 1000, included)
     *
     * On failure, throws an exception or returns Y_CALIBRATEDVALUE_INVALID.
     */
    int                 get_calibratedValue(void);

    inline int          calibratedValue(void)
    { return this->get_calibratedValue(); }

    /**
     * Returns the current measured input value as-is (between 0 and 4095, included).
     *
     * @return an integer corresponding to the current measured input value as-is (between 0 and 4095, included)
     *
     * On failure, throws an exception or returns Y_RAWVALUE_INVALID.
     */
    int                 get_rawValue(void);

    inline int          rawValue(void)
    { return this->get_rawValue(); }

    /**
     * Tells if a calibration process is currently ongoing.
     *
     * @return either Y_ANALOGCALIBRATION_OFF or Y_ANALOGCALIBRATION_ON
     *
     * On failure, throws an exception or returns Y_ANALOGCALIBRATION_INVALID.
     */
    Y_ANALOGCALIBRATION_enum get_analogCalibration(void);

    inline Y_ANALOGCALIBRATION_enum analogCalibration(void)
    { return this->get_analogCalibration(); }

    /**
     * Starts or stops the calibration process. Remember to call the saveToFlash()
     * method of the module at the end of the calibration if the modification must be kept.
     *
     * @param newval : either Y_ANALOGCALIBRATION_OFF or Y_ANALOGCALIBRATION_ON
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_analogCalibration(Y_ANALOGCALIBRATION_enum newval);
    inline int      setAnalogCalibration(Y_ANALOGCALIBRATION_enum newval)
    { return this->set_analogCalibration(newval); }

    /**
     * Returns the maximal value measured during the calibration (between 0 and 4095, included).
     *
     * @return an integer corresponding to the maximal value measured during the calibration (between 0
     * and 4095, included)
     *
     * On failure, throws an exception or returns Y_CALIBRATIONMAX_INVALID.
     */
    int                 get_calibrationMax(void);

    inline int          calibrationMax(void)
    { return this->get_calibrationMax(); }

    /**
     * Changes the maximal calibration value for the input (between 0 and 4095, included), without actually
     * starting the automated calibration.  Remember to call the saveToFlash()
     * method of the module if the modification must be kept.
     *
     * @param newval : an integer corresponding to the maximal calibration value for the input (between 0
     * and 4095, included), without actually
     *         starting the automated calibration
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_calibrationMax(int newval);
    inline int      setCalibrationMax(int newval)
    { return this->set_calibrationMax(newval); }

    /**
     * Returns the minimal value measured during the calibration (between 0 and 4095, included).
     *
     * @return an integer corresponding to the minimal value measured during the calibration (between 0
     * and 4095, included)
     *
     * On failure, throws an exception or returns Y_CALIBRATIONMIN_INVALID.
     */
    int                 get_calibrationMin(void);

    inline int          calibrationMin(void)
    { return this->get_calibrationMin(); }

    /**
     * Changes the minimal calibration value for the input (between 0 and 4095, included), without actually
     * starting the automated calibration.  Remember to call the saveToFlash()
     * method of the module if the modification must be kept.
     *
     * @param newval : an integer corresponding to the minimal calibration value for the input (between 0
     * and 4095, included), without actually
     *         starting the automated calibration
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_calibrationMin(int newval);
    inline int      setCalibrationMin(int newval)
    { return this->set_calibrationMin(newval); }

    /**
     * Returns the sensibility for the input (between 1 and 1000) for triggering user callbacks.
     *
     * @return an integer corresponding to the sensibility for the input (between 1 and 1000) for
     * triggering user callbacks
     *
     * On failure, throws an exception or returns Y_SENSITIVITY_INVALID.
     */
    int                 get_sensitivity(void);

    inline int          sensitivity(void)
    { return this->get_sensitivity(); }

    /**
     * Changes the sensibility for the input (between 1 and 1000) for triggering user callbacks.
     * The sensibility is used to filter variations around a fixed value, but does not preclude the
     * transmission of events when the input value evolves constantly in the same direction.
     * Special case: when the value 1000 is used, the callback will only be thrown when the logical state
     * of the input switches from pressed to released and back.
     * Remember to call the saveToFlash() method of the module if the modification must be kept.
     *
     * @param newval : an integer corresponding to the sensibility for the input (between 1 and 1000) for
     * triggering user callbacks
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_sensitivity(int newval);
    inline int      setSensitivity(int newval)
    { return this->set_sensitivity(newval); }

    /**
     * Returns true if the input (considered as binary) is active (closed contact), and false otherwise.
     *
     * @return either Y_ISPRESSED_FALSE or Y_ISPRESSED_TRUE, according to true if the input (considered as
     * binary) is active (closed contact), and false otherwise
     *
     * On failure, throws an exception or returns Y_ISPRESSED_INVALID.
     */
    Y_ISPRESSED_enum    get_isPressed(void);

    inline Y_ISPRESSED_enum isPressed(void)
    { return this->get_isPressed(); }

    /**
     * Returns the number of elapsed milliseconds between the module power on and the last time
     * the input button was pressed (the input contact transitioned from open to closed).
     *
     * @return an integer corresponding to the number of elapsed milliseconds between the module power on
     * and the last time
     *         the input button was pressed (the input contact transitioned from open to closed)
     *
     * On failure, throws an exception or returns Y_LASTTIMEPRESSED_INVALID.
     */
    s64                 get_lastTimePressed(void);

    inline s64          lastTimePressed(void)
    { return this->get_lastTimePressed(); }

    /**
     * Returns the number of elapsed milliseconds between the module power on and the last time
     * the input button was released (the input contact transitioned from closed to open).
     *
     * @return an integer corresponding to the number of elapsed milliseconds between the module power on
     * and the last time
     *         the input button was released (the input contact transitioned from closed to open)
     *
     * On failure, throws an exception or returns Y_LASTTIMERELEASED_INVALID.
     */
    s64                 get_lastTimeReleased(void);

    inline s64          lastTimeReleased(void)
    { return this->get_lastTimeReleased(); }

    /**
     * Returns the pulse counter value. The value is a 32 bit integer. In case
     * of overflow (>=2^32), the counter will wrap. To reset the counter, just
     * call the resetCounter() method.
     *
     * @return an integer corresponding to the pulse counter value
     *
     * On failure, throws an exception or returns Y_PULSECOUNTER_INVALID.
     */
    s64                 get_pulseCounter(void);

    inline s64          pulseCounter(void)
    { return this->get_pulseCounter(); }

    int             set_pulseCounter(s64 newval);
    inline int      setPulseCounter(s64 newval)
    { return this->set_pulseCounter(newval); }

    /**
     * Returns the timer of the pulses counter (ms).
     *
     * @return an integer corresponding to the timer of the pulses counter (ms)
     *
     * On failure, throws an exception or returns Y_PULSETIMER_INVALID.
     */
    s64                 get_pulseTimer(void);

    inline s64          pulseTimer(void)
    { return this->get_pulseTimer(); }

    /**
     * Retrieves an analog input for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the analog input is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YAnButton.isOnline() to test if the analog input is
     * indeed online at a given time. In case of ambiguity when looking for
     * an analog input by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the analog input
     *
     * @return a YAnButton object allowing you to drive the analog input.
     */
    static YAnButton*   FindAnButton(string func);

    /**
     * Registers the callback function that is invoked on every change of advertised value.
     * The callback is invoked only during the execution of ySleep or yHandleEvents.
     * This provides control over the time when the callback is triggered. For good responsiveness, remember to call
     * one of these two functions periodically. To unregister a callback, pass a NULL pointer as argument.
     *
     * @param callback : the callback function to call, or a NULL pointer. The callback function should take two
     *         arguments: the function object of which the value has changed, and the character string describing
     *         the new advertised value.
     * @noreturn
     */
    virtual int         registerValueCallback(YAnButtonValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Returns the pulse counter value as well as its timer.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         resetCounter(void);


    inline static YAnButton* Find(string func)
    { return YAnButton::FindAnButton(func); }

    /**
     * Continues the enumeration of analog inputs started using yFirstAnButton().
     *
     * @return a pointer to a YAnButton object, corresponding to
     *         an analog input currently online, or a NULL pointer
     *         if there are no more analog inputs to enumerate.
     */
           YAnButton       *nextAnButton(void);
    inline YAnButton       *next(void)
    { return this->nextAnButton();}

    /**
     * Starts the enumeration of analog inputs currently accessible.
     * Use the method YAnButton.nextAnButton() to iterate on
     * next analog inputs.
     *
     * @return a pointer to a YAnButton object, corresponding to
     *         the first analog input currently online, or a NULL pointer
     *         if there are none.
     */
           static YAnButton* FirstAnButton(void);
    inline static YAnButton* First(void)
    { return YAnButton::FirstAnButton();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YAnButton accessors declaration)
};

//--- (YAnButton functions declaration)

/**
 * Retrieves an analog input for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the analog input is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YAnButton.isOnline() to test if the analog input is
 * indeed online at a given time. In case of ambiguity when looking for
 * an analog input by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the analog input
 *
 * @return a YAnButton object allowing you to drive the analog input.
 */
inline YAnButton* yFindAnButton(const string& func)
{ return YAnButton::FindAnButton(func);}
/**
 * Starts the enumeration of analog inputs currently accessible.
 * Use the method YAnButton.nextAnButton() to iterate on
 * next analog inputs.
 *
 * @return a pointer to a YAnButton object, corresponding to
 *         the first analog input currently online, or a NULL pointer
 *         if there are none.
 */
inline YAnButton* yFirstAnButton(void)
{ return YAnButton::FirstAnButton();}

//--- (end of YAnButton functions declaration)

#endif
