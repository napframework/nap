/*********************************************************************
 *
 * $Id: yocto_segmenteddisplay.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindSegmentedDisplay(), the high-level API for SegmentedDisplay functions
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


#ifndef YOCTO_SEGMENTEDDISPLAY_H
#define YOCTO_SEGMENTEDDISPLAY_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YSegmentedDisplay return codes)
//--- (end of YSegmentedDisplay return codes)
//--- (YSegmentedDisplay definitions)
class YSegmentedDisplay; // forward declaration

typedef void (*YSegmentedDisplayValueCallback)(YSegmentedDisplay *func, const string& functionValue);
#ifndef _Y_DISPLAYMODE_ENUM
#define _Y_DISPLAYMODE_ENUM
typedef enum {
    Y_DISPLAYMODE_DISCONNECTED = 0,
    Y_DISPLAYMODE_MANUAL = 1,
    Y_DISPLAYMODE_AUTO1 = 2,
    Y_DISPLAYMODE_AUTO60 = 3,
    Y_DISPLAYMODE_INVALID = -1,
} Y_DISPLAYMODE_enum;
#endif
#define Y_DISPLAYEDTEXT_INVALID         (YAPI_INVALID_STRING)
//--- (end of YSegmentedDisplay definitions)

//--- (YSegmentedDisplay declaration)
/**
 * YSegmentedDisplay Class: SegmentedDisplay function interface
 *
 * The SegmentedDisplay class allows you to drive segmented displays.
 */
class YOCTO_CLASS_EXPORT YSegmentedDisplay: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YSegmentedDisplay declaration)
protected:
    //--- (YSegmentedDisplay attributes)
    // Attributes (function value cache)
    string          _displayedText;
    Y_DISPLAYMODE_enum _displayMode;
    YSegmentedDisplayValueCallback _valueCallbackSegmentedDisplay;

    friend YSegmentedDisplay *yFindSegmentedDisplay(const string& func);
    friend YSegmentedDisplay *yFirstSegmentedDisplay(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindSegmentedDisplay factory function to instantiate
    YSegmentedDisplay(const string& func);
    //--- (end of YSegmentedDisplay attributes)

public:
    ~YSegmentedDisplay();
    //--- (YSegmentedDisplay accessors declaration)

    static const string DISPLAYEDTEXT_INVALID;
    static const Y_DISPLAYMODE_enum DISPLAYMODE_DISCONNECTED = Y_DISPLAYMODE_DISCONNECTED;
    static const Y_DISPLAYMODE_enum DISPLAYMODE_MANUAL = Y_DISPLAYMODE_MANUAL;
    static const Y_DISPLAYMODE_enum DISPLAYMODE_AUTO1 = Y_DISPLAYMODE_AUTO1;
    static const Y_DISPLAYMODE_enum DISPLAYMODE_AUTO60 = Y_DISPLAYMODE_AUTO60;
    static const Y_DISPLAYMODE_enum DISPLAYMODE_INVALID = Y_DISPLAYMODE_INVALID;

    /**
     * Returns the text currently displayed on the screen.
     *
     * @return a string corresponding to the text currently displayed on the screen
     *
     * On failure, throws an exception or returns Y_DISPLAYEDTEXT_INVALID.
     */
    string              get_displayedText(void);

    inline string       displayedText(void)
    { return this->get_displayedText(); }

    /**
     * Changes the text currently displayed on the screen.
     *
     * @param newval : a string corresponding to the text currently displayed on the screen
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_displayedText(const string& newval);
    inline int      setDisplayedText(const string& newval)
    { return this->set_displayedText(newval); }

    Y_DISPLAYMODE_enum  get_displayMode(void);

    inline Y_DISPLAYMODE_enum displayMode(void)
    { return this->get_displayMode(); }

    int             set_displayMode(Y_DISPLAYMODE_enum newval);
    inline int      setDisplayMode(Y_DISPLAYMODE_enum newval)
    { return this->set_displayMode(newval); }

    /**
     * Retrieves a segmented display for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the segmented displays is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YSegmentedDisplay.isOnline() to test if the segmented displays is
     * indeed online at a given time. In case of ambiguity when looking for
     * a segmented display by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the segmented displays
     *
     * @return a YSegmentedDisplay object allowing you to drive the segmented displays.
     */
    static YSegmentedDisplay* FindSegmentedDisplay(string func);

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
    virtual int         registerValueCallback(YSegmentedDisplayValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YSegmentedDisplay* Find(string func)
    { return YSegmentedDisplay::FindSegmentedDisplay(func); }

    /**
     * Continues the enumeration of segmented displays started using yFirstSegmentedDisplay().
     *
     * @return a pointer to a YSegmentedDisplay object, corresponding to
     *         a segmented display currently online, or a NULL pointer
     *         if there are no more segmented displays to enumerate.
     */
           YSegmentedDisplay *nextSegmentedDisplay(void);
    inline YSegmentedDisplay *next(void)
    { return this->nextSegmentedDisplay();}

    /**
     * Starts the enumeration of segmented displays currently accessible.
     * Use the method YSegmentedDisplay.nextSegmentedDisplay() to iterate on
     * next segmented displays.
     *
     * @return a pointer to a YSegmentedDisplay object, corresponding to
     *         the first segmented displays currently online, or a NULL pointer
     *         if there are none.
     */
           static YSegmentedDisplay* FirstSegmentedDisplay(void);
    inline static YSegmentedDisplay* First(void)
    { return YSegmentedDisplay::FirstSegmentedDisplay();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YSegmentedDisplay accessors declaration)
};

//--- (YSegmentedDisplay functions declaration)

/**
 * Retrieves a segmented display for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the segmented displays is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YSegmentedDisplay.isOnline() to test if the segmented displays is
 * indeed online at a given time. In case of ambiguity when looking for
 * a segmented display by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the segmented displays
 *
 * @return a YSegmentedDisplay object allowing you to drive the segmented displays.
 */
inline YSegmentedDisplay* yFindSegmentedDisplay(const string& func)
{ return YSegmentedDisplay::FindSegmentedDisplay(func);}
/**
 * Starts the enumeration of segmented displays currently accessible.
 * Use the method YSegmentedDisplay.nextSegmentedDisplay() to iterate on
 * next segmented displays.
 *
 * @return a pointer to a YSegmentedDisplay object, corresponding to
 *         the first segmented displays currently online, or a NULL pointer
 *         if there are none.
 */
inline YSegmentedDisplay* yFirstSegmentedDisplay(void)
{ return YSegmentedDisplay::FirstSegmentedDisplay();}

//--- (end of YSegmentedDisplay functions declaration)

#endif
