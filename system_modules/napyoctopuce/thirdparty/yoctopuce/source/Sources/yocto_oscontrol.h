/*********************************************************************
 *
 * $Id: yocto_oscontrol.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindOsControl(), the high-level API for OsControl functions
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


#ifndef YOCTO_OSCONTROL_H
#define YOCTO_OSCONTROL_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YOsControl return codes)
//--- (end of YOsControl return codes)
//--- (YOsControl definitions)
class YOsControl; // forward declaration

typedef void (*YOsControlValueCallback)(YOsControl *func, const string& functionValue);
#define Y_SHUTDOWNCOUNTDOWN_INVALID     (YAPI_INVALID_UINT)
//--- (end of YOsControl definitions)

//--- (YOsControl declaration)
/**
 * YOsControl Class: OS control
 *
 * The OScontrol object allows some control over the operating system running a VirtualHub.
 * OsControl is available on the VirtualHub software only. This feature must be activated at the VirtualHub
 * start up with -o option.
 */
class YOCTO_CLASS_EXPORT YOsControl: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YOsControl declaration)
protected:
    //--- (YOsControl attributes)
    // Attributes (function value cache)
    int             _shutdownCountdown;
    YOsControlValueCallback _valueCallbackOsControl;

    friend YOsControl *yFindOsControl(const string& func);
    friend YOsControl *yFirstOsControl(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindOsControl factory function to instantiate
    YOsControl(const string& func);
    //--- (end of YOsControl attributes)

public:
    ~YOsControl();
    //--- (YOsControl accessors declaration)

    static const int SHUTDOWNCOUNTDOWN_INVALID = YAPI_INVALID_UINT;

    /**
     * Returns the remaining number of seconds before the OS shutdown, or zero when no
     * shutdown has been scheduled.
     *
     * @return an integer corresponding to the remaining number of seconds before the OS shutdown, or zero when no
     *         shutdown has been scheduled
     *
     * On failure, throws an exception or returns Y_SHUTDOWNCOUNTDOWN_INVALID.
     */
    int                 get_shutdownCountdown(void);

    inline int          shutdownCountdown(void)
    { return this->get_shutdownCountdown(); }

    int             set_shutdownCountdown(int newval);
    inline int      setShutdownCountdown(int newval)
    { return this->set_shutdownCountdown(newval); }

    /**
     * Retrieves OS control for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the OS control is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YOsControl.isOnline() to test if the OS control is
     * indeed online at a given time. In case of ambiguity when looking for
     * OS control by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the OS control
     *
     * @return a YOsControl object allowing you to drive the OS control.
     */
    static YOsControl*  FindOsControl(string func);

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
    virtual int         registerValueCallback(YOsControlValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Schedules an OS shutdown after a given number of seconds.
     *
     * @param secBeforeShutDown : number of seconds before shutdown
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         shutdown(int secBeforeShutDown);


    inline static YOsControl* Find(string func)
    { return YOsControl::FindOsControl(func); }

    /**
     * Continues the enumeration of OS control started using yFirstOsControl().
     *
     * @return a pointer to a YOsControl object, corresponding to
     *         OS control currently online, or a NULL pointer
     *         if there are no more OS control to enumerate.
     */
           YOsControl      *nextOsControl(void);
    inline YOsControl      *next(void)
    { return this->nextOsControl();}

    /**
     * Starts the enumeration of OS control currently accessible.
     * Use the method YOsControl.nextOsControl() to iterate on
     * next OS control.
     *
     * @return a pointer to a YOsControl object, corresponding to
     *         the first OS control currently online, or a NULL pointer
     *         if there are none.
     */
           static YOsControl* FirstOsControl(void);
    inline static YOsControl* First(void)
    { return YOsControl::FirstOsControl();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YOsControl accessors declaration)
};

//--- (YOsControl functions declaration)

/**
 * Retrieves OS control for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the OS control is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YOsControl.isOnline() to test if the OS control is
 * indeed online at a given time. In case of ambiguity when looking for
 * OS control by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the OS control
 *
 * @return a YOsControl object allowing you to drive the OS control.
 */
inline YOsControl* yFindOsControl(const string& func)
{ return YOsControl::FindOsControl(func);}
/**
 * Starts the enumeration of OS control currently accessible.
 * Use the method YOsControl.nextOsControl() to iterate on
 * next OS control.
 *
 * @return a pointer to a YOsControl object, corresponding to
 *         the first OS control currently online, or a NULL pointer
 *         if there are none.
 */
inline YOsControl* yFirstOsControl(void)
{ return YOsControl::FirstOsControl();}

//--- (end of YOsControl functions declaration)

#endif
