/*********************************************************************
 *
 * $Id: yocto_audioout.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindAudioOut(), the high-level API for AudioOut functions
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


#ifndef YOCTO_AUDIOOUT_H
#define YOCTO_AUDIOOUT_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YAudioOut return codes)
//--- (end of YAudioOut return codes)
//--- (YAudioOut definitions)
class YAudioOut; // forward declaration

typedef void (*YAudioOutValueCallback)(YAudioOut *func, const string& functionValue);
#ifndef _Y_MUTE_ENUM
#define _Y_MUTE_ENUM
typedef enum {
    Y_MUTE_FALSE = 0,
    Y_MUTE_TRUE = 1,
    Y_MUTE_INVALID = -1,
} Y_MUTE_enum;
#endif
#define Y_VOLUME_INVALID                (YAPI_INVALID_UINT)
#define Y_VOLUMERANGE_INVALID           (YAPI_INVALID_STRING)
#define Y_SIGNAL_INVALID                (YAPI_INVALID_INT)
#define Y_NOSIGNALFOR_INVALID           (YAPI_INVALID_INT)
//--- (end of YAudioOut definitions)

//--- (YAudioOut declaration)
/**
 * YAudioOut Class: AudioOut function interface
 *
 * The Yoctopuce application programming interface allows you to configure the volume of the outout.
 */
class YOCTO_CLASS_EXPORT YAudioOut: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YAudioOut declaration)
protected:
    //--- (YAudioOut attributes)
    // Attributes (function value cache)
    int             _volume;
    Y_MUTE_enum     _mute;
    string          _volumeRange;
    int             _signal;
    int             _noSignalFor;
    YAudioOutValueCallback _valueCallbackAudioOut;

    friend YAudioOut *yFindAudioOut(const string& func);
    friend YAudioOut *yFirstAudioOut(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindAudioOut factory function to instantiate
    YAudioOut(const string& func);
    //--- (end of YAudioOut attributes)

public:
    ~YAudioOut();
    //--- (YAudioOut accessors declaration)

    static const int VOLUME_INVALID = YAPI_INVALID_UINT;
    static const Y_MUTE_enum MUTE_FALSE = Y_MUTE_FALSE;
    static const Y_MUTE_enum MUTE_TRUE = Y_MUTE_TRUE;
    static const Y_MUTE_enum MUTE_INVALID = Y_MUTE_INVALID;
    static const string VOLUMERANGE_INVALID;
    static const int SIGNAL_INVALID = YAPI_INVALID_INT;
    static const int NOSIGNALFOR_INVALID = YAPI_INVALID_INT;

    /**
     * Returns audio output volume, in per cents.
     *
     * @return an integer corresponding to audio output volume, in per cents
     *
     * On failure, throws an exception or returns Y_VOLUME_INVALID.
     */
    int                 get_volume(void);

    inline int          volume(void)
    { return this->get_volume(); }

    /**
     * Changes audio output volume, in per cents.
     *
     * @param newval : an integer corresponding to audio output volume, in per cents
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_volume(int newval);
    inline int      setVolume(int newval)
    { return this->set_volume(newval); }

    /**
     * Returns the state of the mute function.
     *
     * @return either Y_MUTE_FALSE or Y_MUTE_TRUE, according to the state of the mute function
     *
     * On failure, throws an exception or returns Y_MUTE_INVALID.
     */
    Y_MUTE_enum         get_mute(void);

    inline Y_MUTE_enum  mute(void)
    { return this->get_mute(); }

    /**
     * Changes the state of the mute function. Remember to call the matching module
     * saveToFlash() method to save the setting permanently.
     *
     * @param newval : either Y_MUTE_FALSE or Y_MUTE_TRUE, according to the state of the mute function
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_mute(Y_MUTE_enum newval);
    inline int      setMute(Y_MUTE_enum newval)
    { return this->set_mute(newval); }

    /**
     * Returns the supported volume range. The low value of the
     * range corresponds to the minimal audible value. To
     * completely mute the sound, use set_mute()
     * instead of the set_volume().
     *
     * @return a string corresponding to the supported volume range
     *
     * On failure, throws an exception or returns Y_VOLUMERANGE_INVALID.
     */
    string              get_volumeRange(void);

    inline string       volumeRange(void)
    { return this->get_volumeRange(); }

    /**
     * Returns the detected output current level.
     *
     * @return an integer corresponding to the detected output current level
     *
     * On failure, throws an exception or returns Y_SIGNAL_INVALID.
     */
    int                 get_signal(void);

    inline int          signal(void)
    { return this->get_signal(); }

    /**
     * Returns the number of seconds elapsed without detecting a signal.
     *
     * @return an integer corresponding to the number of seconds elapsed without detecting a signal
     *
     * On failure, throws an exception or returns Y_NOSIGNALFOR_INVALID.
     */
    int                 get_noSignalFor(void);

    inline int          noSignalFor(void)
    { return this->get_noSignalFor(); }

    /**
     * Retrieves an audio output for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the audio output is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YAudioOut.isOnline() to test if the audio output is
     * indeed online at a given time. In case of ambiguity when looking for
     * an audio output by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the audio output
     *
     * @return a YAudioOut object allowing you to drive the audio output.
     */
    static YAudioOut*   FindAudioOut(string func);

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
    virtual int         registerValueCallback(YAudioOutValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);


    inline static YAudioOut* Find(string func)
    { return YAudioOut::FindAudioOut(func); }

    /**
     * Continues the enumeration of audio outputs started using yFirstAudioOut().
     *
     * @return a pointer to a YAudioOut object, corresponding to
     *         an audio output currently online, or a NULL pointer
     *         if there are no more audio outputs to enumerate.
     */
           YAudioOut       *nextAudioOut(void);
    inline YAudioOut       *next(void)
    { return this->nextAudioOut();}

    /**
     * Starts the enumeration of audio outputs currently accessible.
     * Use the method YAudioOut.nextAudioOut() to iterate on
     * next audio outputs.
     *
     * @return a pointer to a YAudioOut object, corresponding to
     *         the first audio output currently online, or a NULL pointer
     *         if there are none.
     */
           static YAudioOut* FirstAudioOut(void);
    inline static YAudioOut* First(void)
    { return YAudioOut::FirstAudioOut();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YAudioOut accessors declaration)
};

//--- (YAudioOut functions declaration)

/**
 * Retrieves an audio output for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the audio output is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YAudioOut.isOnline() to test if the audio output is
 * indeed online at a given time. In case of ambiguity when looking for
 * an audio output by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the audio output
 *
 * @return a YAudioOut object allowing you to drive the audio output.
 */
inline YAudioOut* yFindAudioOut(const string& func)
{ return YAudioOut::FindAudioOut(func);}
/**
 * Starts the enumeration of audio outputs currently accessible.
 * Use the method YAudioOut.nextAudioOut() to iterate on
 * next audio outputs.
 *
 * @return a pointer to a YAudioOut object, corresponding to
 *         the first audio output currently online, or a NULL pointer
 *         if there are none.
 */
inline YAudioOut* yFirstAudioOut(void)
{ return YAudioOut::FirstAudioOut();}

//--- (end of YAudioOut functions declaration)

#endif
