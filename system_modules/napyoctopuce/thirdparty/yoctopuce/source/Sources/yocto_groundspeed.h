/*********************************************************************
 *
 * $Id: yocto_groundspeed.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindGroundSpeed(), the high-level API for GroundSpeed functions
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


#ifndef YOCTO_GROUNDSPEED_H
#define YOCTO_GROUNDSPEED_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YGroundSpeed return codes)
//--- (end of YGroundSpeed return codes)
//--- (YGroundSpeed definitions)
class YGroundSpeed; // forward declaration

typedef void (*YGroundSpeedValueCallback)(YGroundSpeed *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YGroundSpeedTimedReportCallback)(YGroundSpeed *func, YMeasure measure);
//--- (end of YGroundSpeed definitions)

//--- (YGroundSpeed declaration)
/**
 * YGroundSpeed Class: GroundSpeed function interface
 *
 * The Yoctopuce class YGroundSpeed allows you to read the ground speed from Yoctopuce
 * geolocalization sensors. It inherits from the YSensor class the core functions to
 * read measurements, register callback functions, access the autonomous
 * datalogger.
 */
class YOCTO_CLASS_EXPORT YGroundSpeed: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YGroundSpeed declaration)
protected:
    //--- (YGroundSpeed attributes)
    // Attributes (function value cache)
    YGroundSpeedValueCallback _valueCallbackGroundSpeed;
    YGroundSpeedTimedReportCallback _timedReportCallbackGroundSpeed;

    friend YGroundSpeed *yFindGroundSpeed(const string& func);
    friend YGroundSpeed *yFirstGroundSpeed(void);

    // Constructor is protected, use yFindGroundSpeed factory function to instantiate
    YGroundSpeed(const string& func);
    //--- (end of YGroundSpeed attributes)

public:
    ~YGroundSpeed();
    //--- (YGroundSpeed accessors declaration)


    /**
     * Retrieves a ground speed sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the ground speed sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YGroundSpeed.isOnline() to test if the ground speed sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a ground speed sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the ground speed sensor
     *
     * @return a YGroundSpeed object allowing you to drive the ground speed sensor.
     */
    static YGroundSpeed* FindGroundSpeed(string func);

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
    virtual int         registerValueCallback(YGroundSpeedValueCallback callback);
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
    virtual int         registerTimedReportCallback(YGroundSpeedTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YGroundSpeed* Find(string func)
    { return YGroundSpeed::FindGroundSpeed(func); }

    /**
     * Continues the enumeration of ground speed sensors started using yFirstGroundSpeed().
     *
     * @return a pointer to a YGroundSpeed object, corresponding to
     *         a ground speed sensor currently online, or a NULL pointer
     *         if there are no more ground speed sensors to enumerate.
     */
           YGroundSpeed    *nextGroundSpeed(void);
    inline YGroundSpeed    *next(void)
    { return this->nextGroundSpeed();}

    /**
     * Starts the enumeration of ground speed sensors currently accessible.
     * Use the method YGroundSpeed.nextGroundSpeed() to iterate on
     * next ground speed sensors.
     *
     * @return a pointer to a YGroundSpeed object, corresponding to
     *         the first ground speed sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YGroundSpeed* FirstGroundSpeed(void);
    inline static YGroundSpeed* First(void)
    { return YGroundSpeed::FirstGroundSpeed();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YGroundSpeed accessors declaration)
};

//--- (YGroundSpeed functions declaration)

/**
 * Retrieves a ground speed sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the ground speed sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YGroundSpeed.isOnline() to test if the ground speed sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a ground speed sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the ground speed sensor
 *
 * @return a YGroundSpeed object allowing you to drive the ground speed sensor.
 */
inline YGroundSpeed* yFindGroundSpeed(const string& func)
{ return YGroundSpeed::FindGroundSpeed(func);}
/**
 * Starts the enumeration of ground speed sensors currently accessible.
 * Use the method YGroundSpeed.nextGroundSpeed() to iterate on
 * next ground speed sensors.
 *
 * @return a pointer to a YGroundSpeed object, corresponding to
 *         the first ground speed sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YGroundSpeed* yFirstGroundSpeed(void)
{ return YGroundSpeed::FirstGroundSpeed();}

//--- (end of YGroundSpeed functions declaration)

#endif
