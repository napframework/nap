/*********************************************************************
 *
 * $Id: yocto_current.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindCurrent(), the high-level API for Current functions
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


#ifndef YOCTO_CURRENT_H
#define YOCTO_CURRENT_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YCurrent return codes)
//--- (end of YCurrent return codes)
//--- (YCurrent definitions)
class YCurrent; // forward declaration

typedef void (*YCurrentValueCallback)(YCurrent *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YCurrentTimedReportCallback)(YCurrent *func, YMeasure measure);
#ifndef _Y_ENABLED_ENUM
#define _Y_ENABLED_ENUM
typedef enum {
    Y_ENABLED_FALSE = 0,
    Y_ENABLED_TRUE = 1,
    Y_ENABLED_INVALID = -1,
} Y_ENABLED_enum;
#endif
//--- (end of YCurrent definitions)

//--- (YCurrent declaration)
/**
 * YCurrent Class: Current function interface
 *
 * The Yoctopuce class YCurrent allows you to read and configure Yoctopuce current
 * sensors. It inherits from YSensor class the core functions to read measurements,
 * to register callback functions, to access the autonomous datalogger.
 */
class YOCTO_CLASS_EXPORT YCurrent: public YSensor {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YCurrent declaration)
protected:
    //--- (YCurrent attributes)
    // Attributes (function value cache)
    Y_ENABLED_enum  _enabled;
    YCurrentValueCallback _valueCallbackCurrent;
    YCurrentTimedReportCallback _timedReportCallbackCurrent;

    friend YCurrent *yFindCurrent(const string& func);
    friend YCurrent *yFirstCurrent(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindCurrent factory function to instantiate
    YCurrent(const string& func);
    //--- (end of YCurrent attributes)

public:
    ~YCurrent();
    //--- (YCurrent accessors declaration)

    static const Y_ENABLED_enum ENABLED_FALSE = Y_ENABLED_FALSE;
    static const Y_ENABLED_enum ENABLED_TRUE = Y_ENABLED_TRUE;
    static const Y_ENABLED_enum ENABLED_INVALID = Y_ENABLED_INVALID;

    Y_ENABLED_enum      get_enabled(void);

    inline Y_ENABLED_enum enabled(void)
    { return this->get_enabled(); }

    int             set_enabled(Y_ENABLED_enum newval);
    inline int      setEnabled(Y_ENABLED_enum newval)
    { return this->set_enabled(newval); }

    /**
     * Retrieves a current sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the current sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YCurrent.isOnline() to test if the current sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a current sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the current sensor
     *
     * @return a YCurrent object allowing you to drive the current sensor.
     */
    static YCurrent*    FindCurrent(string func);

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
    virtual int         registerValueCallback(YCurrentValueCallback callback);
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
    virtual int         registerTimedReportCallback(YCurrentTimedReportCallback callback);
    using YSensor::registerTimedReportCallback;

    virtual int         _invokeTimedReportCallback(YMeasure value);


    inline static YCurrent* Find(string func)
    { return YCurrent::FindCurrent(func); }

    /**
     * Continues the enumeration of current sensors started using yFirstCurrent().
     *
     * @return a pointer to a YCurrent object, corresponding to
     *         a current sensor currently online, or a NULL pointer
     *         if there are no more current sensors to enumerate.
     */
           YCurrent        *nextCurrent(void);
    inline YCurrent        *next(void)
    { return this->nextCurrent();}

    /**
     * Starts the enumeration of current sensors currently accessible.
     * Use the method YCurrent.nextCurrent() to iterate on
     * next current sensors.
     *
     * @return a pointer to a YCurrent object, corresponding to
     *         the first current sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YCurrent* FirstCurrent(void);
    inline static YCurrent* First(void)
    { return YCurrent::FirstCurrent();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YCurrent accessors declaration)
};

//--- (YCurrent functions declaration)

/**
 * Retrieves a current sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the current sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YCurrent.isOnline() to test if the current sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a current sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the current sensor
 *
 * @return a YCurrent object allowing you to drive the current sensor.
 */
inline YCurrent* yFindCurrent(const string& func)
{ return YCurrent::FindCurrent(func);}
/**
 * Starts the enumeration of current sensors currently accessible.
 * Use the method YCurrent.nextCurrent() to iterate on
 * next current sensors.
 *
 * @return a pointer to a YCurrent object, corresponding to
 *         the first current sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YCurrent* yFirstCurrent(void)
{ return YCurrent::FirstCurrent();}

//--- (end of YCurrent functions declaration)

#endif
