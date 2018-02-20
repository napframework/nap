/*********************************************************************
 *
 * $Id: yocto_lightsensor.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindLightSensor(), the high-level API for LightSensor functions
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


#ifndef YOCTO_LIGHTSENSOR_H
#define YOCTO_LIGHTSENSOR_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YLightSensor return codes)
//--- (end of YLightSensor return codes)
//--- (YLightSensor definitions)
class YLightSensor; // forward declaration

typedef void (*YLightSensorValueCallback)(YLightSensor *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YLightSensorTimedReportCallback)(YLightSensor *func, YMeasure measure);
#ifndef _Y_MEASURETYPE_ENUM
#define _Y_MEASURETYPE_ENUM
typedef enum {
    Y_MEASURETYPE_HUMAN_EYE = 0,
    Y_MEASURETYPE_WIDE_SPECTRUM = 1,
    Y_MEASURETYPE_INFRARED = 2,
    Y_MEASURETYPE_HIGH_RATE = 3,
    Y_MEASURETYPE_HIGH_ENERGY = 4,
    Y_MEASURETYPE_INVALID = -1,
} Y_MEASURETYPE_enum;
#endif
//--- (end of YLightSensor definitions)

//--- (YLightSensor declaration)
/**
 * YLightSensor Class: LightSensor function interface
 *
 * The Yoctopuce class YLightSensor allows you to read and configure Yoctopuce light
 * sensors. It inherits from YSensor class the core functions to read measurements,
 * to register callback functions, to access the autonomous datalogger.
 * This class adds the ability to easily perform a one-point linear calibration
 * to compensate the effect of a glass or filter placed in front of the sensor.
 * For some light sensors with several working modes, this class can select the
 * desired working mode.
 */
class YOCTO_CLASS_EXPORT YLightSensor: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YLightSensor declaration)
protected:
    //--- (YLightSensor attributes)
    // Attributes (function value cache)
    Y_MEASURETYPE_enum _measureType;
    YLightSensorValueCallback _valueCallbackLightSensor;
    YLightSensorTimedReportCallback _timedReportCallbackLightSensor;

    friend YLightSensor *yFindLightSensor(const string& func);
    friend YLightSensor *yFirstLightSensor(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindLightSensor factory function to instantiate
    YLightSensor(const string& func);
    //--- (end of YLightSensor attributes)

public:
    ~YLightSensor();
    //--- (YLightSensor accessors declaration)

    static const Y_MEASURETYPE_enum MEASURETYPE_HUMAN_EYE = Y_MEASURETYPE_HUMAN_EYE;
    static const Y_MEASURETYPE_enum MEASURETYPE_WIDE_SPECTRUM = Y_MEASURETYPE_WIDE_SPECTRUM;
    static const Y_MEASURETYPE_enum MEASURETYPE_INFRARED = Y_MEASURETYPE_INFRARED;
    static const Y_MEASURETYPE_enum MEASURETYPE_HIGH_RATE = Y_MEASURETYPE_HIGH_RATE;
    static const Y_MEASURETYPE_enum MEASURETYPE_HIGH_ENERGY = Y_MEASURETYPE_HIGH_ENERGY;
    static const Y_MEASURETYPE_enum MEASURETYPE_INVALID = Y_MEASURETYPE_INVALID;

    int             set_currentValue(double newval);
    inline int      setCurrentValue(double newval)
    { return this->set_currentValue(newval); }

    /**
     * Changes the sensor-specific calibration parameter so that the current value
     * matches a desired target (linear scaling).
     *
     * @param calibratedVal : the desired target value.
     *
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             calibrate(double calibratedVal);

    /**
     * Returns the type of light measure.
     *
     * @return a value among Y_MEASURETYPE_HUMAN_EYE, Y_MEASURETYPE_WIDE_SPECTRUM, Y_MEASURETYPE_INFRARED,
     * Y_MEASURETYPE_HIGH_RATE and Y_MEASURETYPE_HIGH_ENERGY corresponding to the type of light measure
     *
     * On failure, throws an exception or returns Y_MEASURETYPE_INVALID.
     */
    Y_MEASURETYPE_enum  get_measureType(void);

    inline Y_MEASURETYPE_enum measureType(void)
    { return this->get_measureType(); }

    /**
     * Changes the light sensor type used in the device. The measure can either
     * approximate the response of the human eye, focus on a specific light
     * spectrum, depending on the capabilities of the light-sensitive cell.
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @param newval : a value among Y_MEASURETYPE_HUMAN_EYE, Y_MEASURETYPE_WIDE_SPECTRUM,
     * Y_MEASURETYPE_INFRARED, Y_MEASURETYPE_HIGH_RATE and Y_MEASURETYPE_HIGH_ENERGY corresponding to the
     * light sensor type used in the device
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_measureType(Y_MEASURETYPE_enum newval);
    inline int      setMeasureType(Y_MEASURETYPE_enum newval)
    { return this->set_measureType(newval); }

    /**
     * Retrieves a light sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the light sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YLightSensor.isOnline() to test if the light sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a light sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the light sensor
     *
     * @return a YLightSensor object allowing you to drive the light sensor.
     */
    static YLightSensor* FindLightSensor(string func);

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
    virtual int         registerValueCallback(YLightSensorValueCallback callback);
    using YSensor::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Registers the callback function that is invoked on every periodic timed notification.
     * The callback is invoked only during the execution of ySleep or yHandleEvents.
     * This provides control over the time when the callback is triggered. For good responsiveness, remember to call
     * one of these two functions periodically. To unregister a callback, pass a NULL pointer as argument.
     *
     * @param callback : the callback function to call, or a NULL pointer. The callback function should take two
     *         arguments: the function object of which the value has changed, and an YMeasure object describing
     *         the new advertised value.
     * @noreturn
     */
    virtual int         registerTimedReportCallback(YLightSensorTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YLightSensor* Find(string func)
    { return YLightSensor::FindLightSensor(func); }

    /**
     * Continues the enumeration of light sensors started using yFirstLightSensor().
     *
     * @return a pointer to a YLightSensor object, corresponding to
     *         a light sensor currently online, or a NULL pointer
     *         if there are no more light sensors to enumerate.
     */
           YLightSensor    *nextLightSensor(void);
    inline YLightSensor    *next(void)
    { return this->nextLightSensor();}

    /**
     * Starts the enumeration of light sensors currently accessible.
     * Use the method YLightSensor.nextLightSensor() to iterate on
     * next light sensors.
     *
     * @return a pointer to a YLightSensor object, corresponding to
     *         the first light sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YLightSensor* FirstLightSensor(void);
    inline static YLightSensor* First(void)
    { return YLightSensor::FirstLightSensor();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YLightSensor accessors declaration)
};

//--- (YLightSensor functions declaration)

/**
 * Retrieves a light sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the light sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YLightSensor.isOnline() to test if the light sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a light sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the light sensor
 *
 * @return a YLightSensor object allowing you to drive the light sensor.
 */
inline YLightSensor* yFindLightSensor(const string& func)
{ return YLightSensor::FindLightSensor(func);}
/**
 * Starts the enumeration of light sensors currently accessible.
 * Use the method YLightSensor.nextLightSensor() to iterate on
 * next light sensors.
 *
 * @return a pointer to a YLightSensor object, corresponding to
 *         the first light sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YLightSensor* yFirstLightSensor(void)
{ return YLightSensor::FirstLightSensor();}

//--- (end of YLightSensor functions declaration)

#endif
