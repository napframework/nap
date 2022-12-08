/*********************************************************************
 *
 * $Id: yocto_pressure.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindPressure(), the high-level API for Pressure functions
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


#ifndef YOCTO_PRESSURE_H
#define YOCTO_PRESSURE_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YPressure return codes)
//--- (end of YPressure return codes)
//--- (YPressure definitions)
class YPressure; // forward declaration

typedef void (*YPressureValueCallback)(YPressure *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YPressureTimedReportCallback)(YPressure *func, YMeasure measure);
//--- (end of YPressure definitions)

//--- (YPressure declaration)
/**
 * YPressure Class: Pressure function interface
 *
 * The Yoctopuce class YPressure allows you to read and configure Yoctopuce pressure
 * sensors. It inherits from YSensor class the core functions to read measurements,
 * to register callback functions, to access the autonomous datalogger.
 */
class YOCTO_CLASS_EXPORT YPressure: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YPressure declaration)
protected:
    //--- (YPressure attributes)
    // Attributes (function value cache)
    YPressureValueCallback _valueCallbackPressure;
    YPressureTimedReportCallback _timedReportCallbackPressure;

    friend YPressure *yFindPressure(const string& func);
    friend YPressure *yFirstPressure(void);

    // Constructor is protected, use yFindPressure factory function to instantiate
    YPressure(const string& func);
    //--- (end of YPressure attributes)

public:
    ~YPressure();
    //--- (YPressure accessors declaration)


    /**
     * Retrieves a pressure sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the pressure sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YPressure.isOnline() to test if the pressure sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a pressure sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the pressure sensor
     *
     * @return a YPressure object allowing you to drive the pressure sensor.
     */
    static YPressure*   FindPressure(string func);

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
    virtual int         registerValueCallback(YPressureValueCallback callback);
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
    virtual int         registerTimedReportCallback(YPressureTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YPressure* Find(string func)
    { return YPressure::FindPressure(func); }

    /**
     * Continues the enumeration of pressure sensors started using yFirstPressure().
     *
     * @return a pointer to a YPressure object, corresponding to
     *         a pressure sensor currently online, or a NULL pointer
     *         if there are no more pressure sensors to enumerate.
     */
           YPressure       *nextPressure(void);
    inline YPressure       *next(void)
    { return this->nextPressure();}

    /**
     * Starts the enumeration of pressure sensors currently accessible.
     * Use the method YPressure.nextPressure() to iterate on
     * next pressure sensors.
     *
     * @return a pointer to a YPressure object, corresponding to
     *         the first pressure sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YPressure* FirstPressure(void);
    inline static YPressure* First(void)
    { return YPressure::FirstPressure();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YPressure accessors declaration)
};

//--- (YPressure functions declaration)

/**
 * Retrieves a pressure sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the pressure sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YPressure.isOnline() to test if the pressure sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a pressure sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the pressure sensor
 *
 * @return a YPressure object allowing you to drive the pressure sensor.
 */
inline YPressure* yFindPressure(const string& func)
{ return YPressure::FindPressure(func);}
/**
 * Starts the enumeration of pressure sensors currently accessible.
 * Use the method YPressure.nextPressure() to iterate on
 * next pressure sensors.
 *
 * @return a pointer to a YPressure object, corresponding to
 *         the first pressure sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YPressure* yFirstPressure(void)
{ return YPressure::FirstPressure();}

//--- (end of YPressure functions declaration)

#endif
