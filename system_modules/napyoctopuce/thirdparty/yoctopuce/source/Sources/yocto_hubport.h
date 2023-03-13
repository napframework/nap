/*********************************************************************
 *
 * $Id: yocto_hubport.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindHubPort(), the high-level API for HubPort functions
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


#ifndef YOCTO_HUBPORT_H
#define YOCTO_HUBPORT_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YHubPort return codes)
//--- (end of YHubPort return codes)
//--- (YHubPort definitions)
class YHubPort; // forward declaration

typedef void (*YHubPortValueCallback)(YHubPort *func, const string& functionValue);
#ifndef _Y_ENABLED_ENUM
#define _Y_ENABLED_ENUM
typedef enum {
    Y_ENABLED_FALSE = 0,
    Y_ENABLED_TRUE = 1,
    Y_ENABLED_INVALID = -1,
} Y_ENABLED_enum;
#endif
#ifndef _Y_PORTSTATE_ENUM
#define _Y_PORTSTATE_ENUM
#ifdef Y_PORTSTATE_INVALID
#undef Y_PORTSTATE_INVALID
#endif
typedef enum {
    Y_PORTSTATE_OFF = 0,
    Y_PORTSTATE_OVRLD = 1,
    Y_PORTSTATE_ON = 2,
    Y_PORTSTATE_RUN = 3,
    Y_PORTSTATE_PROG = 4,
    Y_PORTSTATE_INVALID = -1,
} Y_PORTSTATE_enum;
#endif
#define Y_BAUDRATE_INVALID              (YAPI_INVALID_UINT)
//--- (end of YHubPort definitions)

//--- (YHubPort declaration)
/**
 * YHubPort Class: Yocto-hub port interface
 *
 * YHubPort objects provide control over the power supply for every
 * YoctoHub port and provide information about the device connected to it.
 * The logical name of a YHubPort is always automatically set to the
 * unique serial number of the Yoctopuce device connected to it.
 */
class YOCTO_CLASS_EXPORT YHubPort: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YHubPort declaration)
protected:
    //--- (YHubPort attributes)
    // Attributes (function value cache)
    Y_ENABLED_enum  _enabled;
    Y_PORTSTATE_enum _portState;
    int             _baudRate;
    YHubPortValueCallback _valueCallbackHubPort;

    friend YHubPort *yFindHubPort(const string& func);
    friend YHubPort *yFirstHubPort(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindHubPort factory function to instantiate
    YHubPort(const string& func);
    //--- (end of YHubPort attributes)

public:
    ~YHubPort();
    //--- (YHubPort accessors declaration)

    static const Y_ENABLED_enum ENABLED_FALSE = Y_ENABLED_FALSE;
    static const Y_ENABLED_enum ENABLED_TRUE = Y_ENABLED_TRUE;
    static const Y_ENABLED_enum ENABLED_INVALID = Y_ENABLED_INVALID;
    static const Y_PORTSTATE_enum PORTSTATE_OFF = Y_PORTSTATE_OFF;
    static const Y_PORTSTATE_enum PORTSTATE_OVRLD = Y_PORTSTATE_OVRLD;
    static const Y_PORTSTATE_enum PORTSTATE_ON = Y_PORTSTATE_ON;
    static const Y_PORTSTATE_enum PORTSTATE_RUN = Y_PORTSTATE_RUN;
    static const Y_PORTSTATE_enum PORTSTATE_PROG = Y_PORTSTATE_PROG;
    static const Y_PORTSTATE_enum PORTSTATE_INVALID = Y_PORTSTATE_INVALID;
    static const int BAUDRATE_INVALID = YAPI_INVALID_UINT;

    /**
     * Returns true if the Yocto-hub port is powered, false otherwise.
     *
     * @return either Y_ENABLED_FALSE or Y_ENABLED_TRUE, according to true if the Yocto-hub port is
     * powered, false otherwise
     *
     * On failure, throws an exception or returns Y_ENABLED_INVALID.
     */
    Y_ENABLED_enum      get_enabled(void);

    inline Y_ENABLED_enum enabled(void)
    { return this->get_enabled(); }

    /**
     * Changes the activation of the Yocto-hub port. If the port is enabled, the
     * connected module is powered. Otherwise, port power is shut down.
     *
     * @param newval : either Y_ENABLED_FALSE or Y_ENABLED_TRUE, according to the activation of the Yocto-hub port
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_enabled(Y_ENABLED_enum newval);
    inline int      setEnabled(Y_ENABLED_enum newval)
    { return this->set_enabled(newval); }

    /**
     * Returns the current state of the Yocto-hub port.
     *
     * @return a value among Y_PORTSTATE_OFF, Y_PORTSTATE_OVRLD, Y_PORTSTATE_ON, Y_PORTSTATE_RUN and
     * Y_PORTSTATE_PROG corresponding to the current state of the Yocto-hub port
     *
     * On failure, throws an exception or returns Y_PORTSTATE_INVALID.
     */
    Y_PORTSTATE_enum    get_portState(void);

    inline Y_PORTSTATE_enum portState(void)
    { return this->get_portState(); }

    /**
     * Returns the current baud rate used by this Yocto-hub port, in kbps.
     * The default value is 1000 kbps, but a slower rate may be used if communication
     * problems are encountered.
     *
     * @return an integer corresponding to the current baud rate used by this Yocto-hub port, in kbps
     *
     * On failure, throws an exception or returns Y_BAUDRATE_INVALID.
     */
    int                 get_baudRate(void);

    inline int          baudRate(void)
    { return this->get_baudRate(); }

    /**
     * Retrieves a Yocto-hub port for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the Yocto-hub port is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YHubPort.isOnline() to test if the Yocto-hub port is
     * indeed online at a given time. In case of ambiguity when looking for
     * a Yocto-hub port by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the Yocto-hub port
     *
     * @return a YHubPort object allowing you to drive the Yocto-hub port.
     */
    static YHubPort*    FindHubPort(string func);

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
    virtual int         registerValueCallback(YHubPortValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YHubPort* Find(string func)
    { return YHubPort::FindHubPort(func); }

    /**
     * Continues the enumeration of Yocto-hub ports started using yFirstHubPort().
     *
     * @return a pointer to a YHubPort object, corresponding to
     *         a Yocto-hub port currently online, or a NULL pointer
     *         if there are no more Yocto-hub ports to enumerate.
     */
           YHubPort        *nextHubPort(void);
    inline YHubPort        *next(void)
    { return this->nextHubPort();}

    /**
     * Starts the enumeration of Yocto-hub ports currently accessible.
     * Use the method YHubPort.nextHubPort() to iterate on
     * next Yocto-hub ports.
     *
     * @return a pointer to a YHubPort object, corresponding to
     *         the first Yocto-hub port currently online, or a NULL pointer
     *         if there are none.
     */
           static YHubPort* FirstHubPort(void);
    inline static YHubPort* First(void)
    { return YHubPort::FirstHubPort();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YHubPort accessors declaration)
};

//--- (YHubPort functions declaration)

/**
 * Retrieves a Yocto-hub port for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the Yocto-hub port is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YHubPort.isOnline() to test if the Yocto-hub port is
 * indeed online at a given time. In case of ambiguity when looking for
 * a Yocto-hub port by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the Yocto-hub port
 *
 * @return a YHubPort object allowing you to drive the Yocto-hub port.
 */
inline YHubPort* yFindHubPort(const string& func)
{ return YHubPort::FindHubPort(func);}
/**
 * Starts the enumeration of Yocto-hub ports currently accessible.
 * Use the method YHubPort.nextHubPort() to iterate on
 * next Yocto-hub ports.
 *
 * @return a pointer to a YHubPort object, corresponding to
 *         the first Yocto-hub port currently online, or a NULL pointer
 *         if there are none.
 */
inline YHubPort* yFirstHubPort(void)
{ return YHubPort::FirstHubPort();}

//--- (end of YHubPort functions declaration)

#endif
