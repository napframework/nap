/*********************************************************************
 *
 * $Id: yocto_daisychain.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindDaisyChain(), the high-level API for DaisyChain functions
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


#ifndef YOCTO_DAISYCHAIN_H
#define YOCTO_DAISYCHAIN_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YDaisyChain return codes)
//--- (end of YDaisyChain return codes)
//--- (YDaisyChain definitions)
class YDaisyChain; // forward declaration

typedef void (*YDaisyChainValueCallback)(YDaisyChain *func, const string& functionValue);
#ifndef _Y_DAISYSTATE_ENUM
#define _Y_DAISYSTATE_ENUM
typedef enum {
    Y_DAISYSTATE_READY = 0,
    Y_DAISYSTATE_IS_CHILD = 1,
    Y_DAISYSTATE_FIRMWARE_MISMATCH = 2,
    Y_DAISYSTATE_CHILD_MISSING = 3,
    Y_DAISYSTATE_CHILD_LOST = 4,
    Y_DAISYSTATE_INVALID = -1,
} Y_DAISYSTATE_enum;
#endif
#define Y_CHILDCOUNT_INVALID            (YAPI_INVALID_UINT)
#define Y_REQUIREDCHILDCOUNT_INVALID    (YAPI_INVALID_UINT)
//--- (end of YDaisyChain definitions)

//--- (YDaisyChain declaration)
/**
 * YDaisyChain Class: DaisyChain function interface
 *
 * The YDaisyChain interface can be used to verify that devices that
 * are daisy-chained directly from device to device, without a hub,
 * are detected properly.
 */
class YOCTO_CLASS_EXPORT YDaisyChain: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YDaisyChain declaration)
protected:
    //--- (YDaisyChain attributes)
    // Attributes (function value cache)
    Y_DAISYSTATE_enum _daisyState;
    int             _childCount;
    int             _requiredChildCount;
    YDaisyChainValueCallback _valueCallbackDaisyChain;

    friend YDaisyChain *yFindDaisyChain(const string& func);
    friend YDaisyChain *yFirstDaisyChain(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindDaisyChain factory function to instantiate
    YDaisyChain(const string& func);
    //--- (end of YDaisyChain attributes)

public:
    ~YDaisyChain();
    //--- (YDaisyChain accessors declaration)

    static const Y_DAISYSTATE_enum DAISYSTATE_READY = Y_DAISYSTATE_READY;
    static const Y_DAISYSTATE_enum DAISYSTATE_IS_CHILD = Y_DAISYSTATE_IS_CHILD;
    static const Y_DAISYSTATE_enum DAISYSTATE_FIRMWARE_MISMATCH = Y_DAISYSTATE_FIRMWARE_MISMATCH;
    static const Y_DAISYSTATE_enum DAISYSTATE_CHILD_MISSING = Y_DAISYSTATE_CHILD_MISSING;
    static const Y_DAISYSTATE_enum DAISYSTATE_CHILD_LOST = Y_DAISYSTATE_CHILD_LOST;
    static const Y_DAISYSTATE_enum DAISYSTATE_INVALID = Y_DAISYSTATE_INVALID;
    static const int CHILDCOUNT_INVALID = YAPI_INVALID_UINT;
    static const int REQUIREDCHILDCOUNT_INVALID = YAPI_INVALID_UINT;

    /**
     * Returns the state of the daisy-link between modules.
     *
     * @return a value among Y_DAISYSTATE_READY, Y_DAISYSTATE_IS_CHILD, Y_DAISYSTATE_FIRMWARE_MISMATCH,
     * Y_DAISYSTATE_CHILD_MISSING and Y_DAISYSTATE_CHILD_LOST corresponding to the state of the daisy-link
     * between modules
     *
     * On failure, throws an exception or returns Y_DAISYSTATE_INVALID.
     */
    Y_DAISYSTATE_enum   get_daisyState(void);

    inline Y_DAISYSTATE_enum daisyState(void)
    { return this->get_daisyState(); }

    /**
     * Returns the number of child nodes currently detected.
     *
     * @return an integer corresponding to the number of child nodes currently detected
     *
     * On failure, throws an exception or returns Y_CHILDCOUNT_INVALID.
     */
    int                 get_childCount(void);

    inline int          childCount(void)
    { return this->get_childCount(); }

    /**
     * Returns the number of child nodes expected in normal conditions.
     *
     * @return an integer corresponding to the number of child nodes expected in normal conditions
     *
     * On failure, throws an exception or returns Y_REQUIREDCHILDCOUNT_INVALID.
     */
    int                 get_requiredChildCount(void);

    inline int          requiredChildCount(void)
    { return this->get_requiredChildCount(); }

    /**
     * Changes the number of child nodes expected in normal conditions.
     * If the value is zero, no check is performed. If it is non-zero, the number
     * child nodes is checked on startup and the status will change to error if
     * the count does not match.
     *
     * @param newval : an integer corresponding to the number of child nodes expected in normal conditions
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_requiredChildCount(int newval);
    inline int      setRequiredChildCount(int newval)
    { return this->set_requiredChildCount(newval); }

    /**
     * Retrieves a module chain for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the module chain is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YDaisyChain.isOnline() to test if the module chain is
     * indeed online at a given time. In case of ambiguity when looking for
     * a module chain by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the module chain
     *
     * @return a YDaisyChain object allowing you to drive the module chain.
     */
    static YDaisyChain* FindDaisyChain(string func);

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
    virtual int         registerValueCallback(YDaisyChainValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YDaisyChain* Find(string func)
    { return YDaisyChain::FindDaisyChain(func); }

    /**
     * Continues the enumeration of module chains started using yFirstDaisyChain().
     *
     * @return a pointer to a YDaisyChain object, corresponding to
     *         a module chain currently online, or a NULL pointer
     *         if there are no more module chains to enumerate.
     */
           YDaisyChain     *nextDaisyChain(void);
    inline YDaisyChain     *next(void)
    { return this->nextDaisyChain();}

    /**
     * Starts the enumeration of module chains currently accessible.
     * Use the method YDaisyChain.nextDaisyChain() to iterate on
     * next module chains.
     *
     * @return a pointer to a YDaisyChain object, corresponding to
     *         the first module chain currently online, or a NULL pointer
     *         if there are none.
     */
           static YDaisyChain* FirstDaisyChain(void);
    inline static YDaisyChain* First(void)
    { return YDaisyChain::FirstDaisyChain();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YDaisyChain accessors declaration)
};

//--- (YDaisyChain functions declaration)

/**
 * Retrieves a module chain for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the module chain is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YDaisyChain.isOnline() to test if the module chain is
 * indeed online at a given time. In case of ambiguity when looking for
 * a module chain by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the module chain
 *
 * @return a YDaisyChain object allowing you to drive the module chain.
 */
inline YDaisyChain* yFindDaisyChain(const string& func)
{ return YDaisyChain::FindDaisyChain(func);}
/**
 * Starts the enumeration of module chains currently accessible.
 * Use the method YDaisyChain.nextDaisyChain() to iterate on
 * next module chains.
 *
 * @return a pointer to a YDaisyChain object, corresponding to
 *         the first module chain currently online, or a NULL pointer
 *         if there are none.
 */
inline YDaisyChain* yFirstDaisyChain(void)
{ return YDaisyChain::FirstDaisyChain();}

//--- (end of YDaisyChain functions declaration)

#endif
