/*********************************************************************
 *
 * $Id: yocto_buzzer.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindBuzzer(), the high-level API for Buzzer functions
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


#ifndef YOCTO_BUZZER_H
#define YOCTO_BUZZER_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YBuzzer return codes)
//--- (end of YBuzzer return codes)
//--- (YBuzzer definitions)
class YBuzzer; // forward declaration

typedef void (*YBuzzerValueCallback)(YBuzzer *func, const string& functionValue);
#define Y_FREQUENCY_INVALID             (YAPI_INVALID_DOUBLE)
#define Y_VOLUME_INVALID                (YAPI_INVALID_UINT)
#define Y_PLAYSEQSIZE_INVALID           (YAPI_INVALID_UINT)
#define Y_PLAYSEQMAXSIZE_INVALID        (YAPI_INVALID_UINT)
#define Y_PLAYSEQSIGNATURE_INVALID      (YAPI_INVALID_UINT)
#define Y_COMMAND_INVALID               (YAPI_INVALID_STRING)
//--- (end of YBuzzer definitions)

//--- (YBuzzer declaration)
/**
 * YBuzzer Class: Buzzer function interface
 *
 * The Yoctopuce application programming interface allows you to
 * choose the frequency and volume at which the buzzer must sound.
 * You can also pre-program a play sequence.
 */
class YOCTO_CLASS_EXPORT YBuzzer: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YBuzzer declaration)
protected:
    //--- (YBuzzer attributes)
    // Attributes (function value cache)
    double          _frequency;
    int             _volume;
    int             _playSeqSize;
    int             _playSeqMaxSize;
    int             _playSeqSignature;
    string          _command;
    YBuzzerValueCallback _valueCallbackBuzzer;

    friend YBuzzer *yFindBuzzer(const string& func);
    friend YBuzzer *yFirstBuzzer(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindBuzzer factory function to instantiate
    YBuzzer(const string& func);
    //--- (end of YBuzzer attributes)

public:
    ~YBuzzer();
    //--- (YBuzzer accessors declaration)

    static const double FREQUENCY_INVALID;
    static const int VOLUME_INVALID = YAPI_INVALID_UINT;
    static const int PLAYSEQSIZE_INVALID = YAPI_INVALID_UINT;
    static const int PLAYSEQMAXSIZE_INVALID = YAPI_INVALID_UINT;
    static const int PLAYSEQSIGNATURE_INVALID = YAPI_INVALID_UINT;
    static const string COMMAND_INVALID;

    /**
     * Changes the frequency of the signal sent to the buzzer. A zero value stops the buzzer.
     *
     * @param newval : a floating point number corresponding to the frequency of the signal sent to the buzzer
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_frequency(double newval);
    inline int      setFrequency(double newval)
    { return this->set_frequency(newval); }

    /**
     * Returns the  frequency of the signal sent to the buzzer/speaker.
     *
     * @return a floating point number corresponding to the  frequency of the signal sent to the buzzer/speaker
     *
     * On failure, throws an exception or returns Y_FREQUENCY_INVALID.
     */
    double              get_frequency(void);

    inline double       frequency(void)
    { return this->get_frequency(); }

    /**
     * Returns the volume of the signal sent to the buzzer/speaker.
     *
     * @return an integer corresponding to the volume of the signal sent to the buzzer/speaker
     *
     * On failure, throws an exception or returns Y_VOLUME_INVALID.
     */
    int                 get_volume(void);

    inline int          volume(void)
    { return this->get_volume(); }

    /**
     * Changes the volume of the signal sent to the buzzer/speaker.
     *
     * @param newval : an integer corresponding to the volume of the signal sent to the buzzer/speaker
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_volume(int newval);
    inline int      setVolume(int newval)
    { return this->set_volume(newval); }

    /**
     * Returns the current length of the playing sequence.
     *
     * @return an integer corresponding to the current length of the playing sequence
     *
     * On failure, throws an exception or returns Y_PLAYSEQSIZE_INVALID.
     */
    int                 get_playSeqSize(void);

    inline int          playSeqSize(void)
    { return this->get_playSeqSize(); }

    /**
     * Returns the maximum length of the playing sequence.
     *
     * @return an integer corresponding to the maximum length of the playing sequence
     *
     * On failure, throws an exception or returns Y_PLAYSEQMAXSIZE_INVALID.
     */
    int                 get_playSeqMaxSize(void);

    inline int          playSeqMaxSize(void)
    { return this->get_playSeqMaxSize(); }

    /**
     * Returns the playing sequence signature. As playing
     * sequences cannot be read from the device, this can be used
     * to detect if a specific playing sequence is already
     * programmed.
     *
     * @return an integer corresponding to the playing sequence signature
     *
     * On failure, throws an exception or returns Y_PLAYSEQSIGNATURE_INVALID.
     */
    int                 get_playSeqSignature(void);

    inline int          playSeqSignature(void)
    { return this->get_playSeqSignature(); }

    string              get_command(void);

    inline string       command(void)
    { return this->get_command(); }

    int             set_command(const string& newval);
    inline int      setCommand(const string& newval)
    { return this->set_command(newval); }

    /**
     * Retrieves a buzzer for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the buzzer is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YBuzzer.isOnline() to test if the buzzer is
     * indeed online at a given time. In case of ambiguity when looking for
     * a buzzer by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the buzzer
     *
     * @return a YBuzzer object allowing you to drive the buzzer.
     */
    static YBuzzer*     FindBuzzer(string func);

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
    virtual int         registerValueCallback(YBuzzerValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    virtual int         sendCommand(string command);

    /**
     * Adds a new frequency transition to the playing sequence.
     *
     * @param freq    : desired frequency when the transition is completed, in Hz
     * @param msDelay : duration of the frequency transition, in milliseconds.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         addFreqMoveToPlaySeq(int freq,int msDelay);

    /**
     * Adds a pulse to the playing sequence.
     *
     * @param freq : pulse frequency, in Hz
     * @param msDuration : pulse duration, in milliseconds.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         addPulseToPlaySeq(int freq,int msDuration);

    /**
     * Adds a new volume transition to the playing sequence. Frequency stays untouched:
     * if frequency is at zero, the transition has no effect.
     *
     * @param volume    : desired volume when the transition is completed, as a percentage.
     * @param msDuration : duration of the volume transition, in milliseconds.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         addVolMoveToPlaySeq(int volume,int msDuration);

    /**
     * Adds notes to the playing sequence. Notes are provided as text words, separated by
     * spaces. The pitch is specified using the usual letter from A to G. The duration is
     * specified as the divisor of a whole note: 4 for a fourth, 8 for an eight note, etc.
     * Some modifiers are supported: # and b to alter a note pitch,
     * ' and , to move to the upper/lower octave, . to enlarge
     * the note duration.
     *
     * @param notes : notes to be played, as a text string.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         addNotesToPlaySeq(string notes);

    /**
     * Starts the preprogrammed playing sequence. The sequence
     * runs in loop until it is stopped by stopPlaySeq or an explicit
     * change. To play the sequence only once, use oncePlaySeq().
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         startPlaySeq(void);

    /**
     * Stops the preprogrammed playing sequence and sets the frequency to zero.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         stopPlaySeq(void);

    /**
     * Resets the preprogrammed playing sequence and sets the frequency to zero.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         resetPlaySeq(void);

    /**
     * Starts the preprogrammed playing sequence and run it once only.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         oncePlaySeq(void);

    /**
     * Activates the buzzer for a short duration.
     *
     * @param frequency : pulse frequency, in hertz
     * @param duration : pulse duration in millseconds
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         pulse(int frequency,int duration);

    /**
     * Makes the buzzer frequency change over a period of time.
     *
     * @param frequency : frequency to reach, in hertz. A frequency under 25Hz stops the buzzer.
     * @param duration :  pulse duration in millseconds
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         freqMove(int frequency,int duration);

    /**
     * Makes the buzzer volume change over a period of time, frequency  stays untouched.
     *
     * @param volume : volume to reach in %
     * @param duration : change duration in millseconds
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         volumeMove(int volume,int duration);

    /**
     * Immediately play a note sequence. Notes are provided as text words, separated by
     * spaces. The pitch is specified using the usual letter from A to G. The duration is
     * specified as the divisor of a whole note: 4 for a fourth, 8 for an eight note, etc.
     * Some modifiers are supported: # and b to alter a note pitch,
     * ' and , to move to the upper/lower octave, . to enlarge
     * the note duration.
     *
     * @param notes : notes to be played, as a text string.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
    virtual int         playNotes(string notes);


    inline static YBuzzer* Find(string func)
    { return YBuzzer::FindBuzzer(func); }

    /**
     * Continues the enumeration of buzzers started using yFirstBuzzer().
     *
     * @return a pointer to a YBuzzer object, corresponding to
     *         a buzzer currently online, or a NULL pointer
     *         if there are no more buzzers to enumerate.
     */
           YBuzzer         *nextBuzzer(void);
    inline YBuzzer         *next(void)
    { return this->nextBuzzer();}

    /**
     * Starts the enumeration of buzzers currently accessible.
     * Use the method YBuzzer.nextBuzzer() to iterate on
     * next buzzers.
     *
     * @return a pointer to a YBuzzer object, corresponding to
     *         the first buzzer currently online, or a NULL pointer
     *         if there are none.
     */
           static YBuzzer* FirstBuzzer(void);
    inline static YBuzzer* First(void)
    { return YBuzzer::FirstBuzzer();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YBuzzer accessors declaration)
};

//--- (YBuzzer functions declaration)

/**
 * Retrieves a buzzer for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the buzzer is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YBuzzer.isOnline() to test if the buzzer is
 * indeed online at a given time. In case of ambiguity when looking for
 * a buzzer by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the buzzer
 *
 * @return a YBuzzer object allowing you to drive the buzzer.
 */
inline YBuzzer* yFindBuzzer(const string& func)
{ return YBuzzer::FindBuzzer(func);}
/**
 * Starts the enumeration of buzzers currently accessible.
 * Use the method YBuzzer.nextBuzzer() to iterate on
 * next buzzers.
 *
 * @return a pointer to a YBuzzer object, corresponding to
 *         the first buzzer currently online, or a NULL pointer
 *         if there are none.
 */
inline YBuzzer* yFirstBuzzer(void)
{ return YBuzzer::FirstBuzzer();}

//--- (end of YBuzzer functions declaration)

#endif
