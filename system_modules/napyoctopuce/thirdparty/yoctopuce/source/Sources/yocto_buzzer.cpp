/*********************************************************************
 *
 * $Id: yocto_buzzer.cpp 28748 2017-10-03 08:23:39Z seb $
 *
 * Implements yFindBuzzer(), the high-level API for Buzzer functions
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


#define _CRT_SECURE_NO_DEPRECATE //do not use windows secure crt
#include "yocto_buzzer.h"
#include "yapi/yjson.h"
#include "yapi/yapi.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define  __FILE_ID__  "buzzer"

YBuzzer::YBuzzer(const string& func): YFunction(func)
//--- (YBuzzer initialization)
    ,_frequency(FREQUENCY_INVALID)
    ,_volume(VOLUME_INVALID)
    ,_playSeqSize(PLAYSEQSIZE_INVALID)
    ,_playSeqMaxSize(PLAYSEQMAXSIZE_INVALID)
    ,_playSeqSignature(PLAYSEQSIGNATURE_INVALID)
    ,_command(COMMAND_INVALID)
    ,_valueCallbackBuzzer(NULL)
//--- (end of YBuzzer initialization)
{
    _className="Buzzer";
}

YBuzzer::~YBuzzer()
{
//--- (YBuzzer cleanup)
//--- (end of YBuzzer cleanup)
}
//--- (YBuzzer implementation)
// static attributes
const double YBuzzer::FREQUENCY_INVALID = YAPI_INVALID_DOUBLE;
const string YBuzzer::COMMAND_INVALID = YAPI_INVALID_STRING;

int YBuzzer::_parseAttr(YJSONObject* json_val)
{
    if(json_val->has("frequency")) {
        _frequency =  floor(json_val->getDouble("frequency") * 1000.0 / 65536.0 + 0.5) / 1000.0;
    }
    if(json_val->has("volume")) {
        _volume =  json_val->getInt("volume");
    }
    if(json_val->has("playSeqSize")) {
        _playSeqSize =  json_val->getInt("playSeqSize");
    }
    if(json_val->has("playSeqMaxSize")) {
        _playSeqMaxSize =  json_val->getInt("playSeqMaxSize");
    }
    if(json_val->has("playSeqSignature")) {
        _playSeqSignature =  json_val->getInt("playSeqSignature");
    }
    if(json_val->has("command")) {
        _command =  json_val->getString("command");
    }
    return YFunction::_parseAttr(json_val);
}


/**
 * Changes the frequency of the signal sent to the buzzer. A zero value stops the buzzer.
 *
 * @param newval : a floating point number corresponding to the frequency of the signal sent to the buzzer
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YBuzzer::set_frequency(double newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf,"%d", (int)floor(newval * 65536.0 + 0.5)); rest_val = string(buf);
        res = _setAttr("frequency", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the  frequency of the signal sent to the buzzer/speaker.
 *
 * @return a floating point number corresponding to the  frequency of the signal sent to the buzzer/speaker
 *
 * On failure, throws an exception or returns Y_FREQUENCY_INVALID.
 */
double YBuzzer::get_frequency(void)
{
    double res = 0.0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YBuzzer::FREQUENCY_INVALID;
                }
            }
        }
        res = _frequency;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the volume of the signal sent to the buzzer/speaker.
 *
 * @return an integer corresponding to the volume of the signal sent to the buzzer/speaker
 *
 * On failure, throws an exception or returns Y_VOLUME_INVALID.
 */
int YBuzzer::get_volume(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YBuzzer::VOLUME_INVALID;
                }
            }
        }
        res = _volume;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Changes the volume of the signal sent to the buzzer/speaker.
 *
 * @param newval : an integer corresponding to the volume of the signal sent to the buzzer/speaker
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
int YBuzzer::set_volume(int newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        char buf[32]; sprintf(buf, "%d", newval); rest_val = string(buf);
        res = _setAttr("volume", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the current length of the playing sequence.
 *
 * @return an integer corresponding to the current length of the playing sequence
 *
 * On failure, throws an exception or returns Y_PLAYSEQSIZE_INVALID.
 */
int YBuzzer::get_playSeqSize(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YBuzzer::PLAYSEQSIZE_INVALID;
                }
            }
        }
        res = _playSeqSize;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

/**
 * Returns the maximum length of the playing sequence.
 *
 * @return an integer corresponding to the maximum length of the playing sequence
 *
 * On failure, throws an exception or returns Y_PLAYSEQMAXSIZE_INVALID.
 */
int YBuzzer::get_playSeqMaxSize(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration == 0) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YBuzzer::PLAYSEQMAXSIZE_INVALID;
                }
            }
        }
        res = _playSeqMaxSize;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

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
int YBuzzer::get_playSeqSignature(void)
{
    int res = 0;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YBuzzer::PLAYSEQSIGNATURE_INVALID;
                }
            }
        }
        res = _playSeqSignature;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

string YBuzzer::get_command(void)
{
    string res;
    yEnterCriticalSection(&_this_cs);
    try {
        if (_cacheExpiration <= YAPI::GetTickCount()) {
            if (this->_load_unsafe(YAPI::DefaultCacheValidity) != YAPI_SUCCESS) {
                {
                    yLeaveCriticalSection(&_this_cs);
                    return YBuzzer::COMMAND_INVALID;
                }
            }
        }
        res = _command;
    } catch (std::exception) {
        yLeaveCriticalSection(&_this_cs);
        throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

int YBuzzer::set_command(const string& newval)
{
    string rest_val;
    int res;
    yEnterCriticalSection(&_this_cs);
    try {
        rest_val = newval;
        res = _setAttr("command", rest_val);
    } catch (std::exception) {
         yLeaveCriticalSection(&_this_cs);
         throw;
    }
    yLeaveCriticalSection(&_this_cs);
    return res;
}

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
YBuzzer* YBuzzer::FindBuzzer(string func)
{
    YBuzzer* obj = NULL;
    int taken = 0;
    if (YAPI::_apiInitialized) {
        yEnterCriticalSection(&YAPI::_global_cs);
        taken = 1;
    }try {
        obj = (YBuzzer*) YFunction::_FindFromCache("Buzzer", func);
        if (obj == NULL) {
            obj = new YBuzzer(func);
            YFunction::_AddToCache("Buzzer", func, obj);
        }
    } catch (std::exception) {
        if (taken) yLeaveCriticalSection(&YAPI::_global_cs);
        throw;
    }
    if (taken) yLeaveCriticalSection(&YAPI::_global_cs);
    return obj;
}

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
int YBuzzer::registerValueCallback(YBuzzerValueCallback callback)
{
    string val;
    if (callback != NULL) {
        YFunction::_UpdateValueCallbackList(this, true);
    } else {
        YFunction::_UpdateValueCallbackList(this, false);
    }
    _valueCallbackBuzzer = callback;
    // Immediately invoke value callback with current value
    if (callback != NULL && this->isOnline()) {
        val = _advertisedValue;
        if (!(val == "")) {
            this->_invokeValueCallback(val);
        }
    }
    return 0;
}

int YBuzzer::_invokeValueCallback(string value)
{
    if (_valueCallbackBuzzer != NULL) {
        _valueCallbackBuzzer(this, value);
    } else {
        YFunction::_invokeValueCallback(value);
    }
    return 0;
}

int YBuzzer::sendCommand(string command)
{
    return this->set_command(command);
}

/**
 * Adds a new frequency transition to the playing sequence.
 *
 * @param freq    : desired frequency when the transition is completed, in Hz
 * @param msDelay : duration of the frequency transition, in milliseconds.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YBuzzer::addFreqMoveToPlaySeq(int freq,int msDelay)
{
    return this->sendCommand(YapiWrapper::ysprintf("A%d,%d",freq,msDelay));
}

/**
 * Adds a pulse to the playing sequence.
 *
 * @param freq : pulse frequency, in Hz
 * @param msDuration : pulse duration, in milliseconds.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YBuzzer::addPulseToPlaySeq(int freq,int msDuration)
{
    return this->sendCommand(YapiWrapper::ysprintf("B%d,%d",freq,msDuration));
}

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
int YBuzzer::addVolMoveToPlaySeq(int volume,int msDuration)
{
    return this->sendCommand(YapiWrapper::ysprintf("C%d,%d",volume,msDuration));
}

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
int YBuzzer::addNotesToPlaySeq(string notes)
{
    int tempo = 0;
    int prevPitch = 0;
    int prevDuration = 0;
    int prevFreq = 0;
    int note = 0;
    int num = 0;
    int typ = 0;
    string ascNotes;
    int notesLen = 0;
    int i = 0;
    int ch = 0;
    int dNote = 0;
    int pitch = 0;
    int freq = 0;
    int ms = 0;
    int ms16 = 0;
    int rest = 0;
    tempo = 100;
    prevPitch = 3;
    prevDuration = 4;
    prevFreq = 110;
    note = -99;
    num = 0;
    typ = 3;
    ascNotes = notes;
    notesLen = (int)(ascNotes).size();
    i = 0;
    while (i < notesLen) {
        ch = ((u8)ascNotes[i]);
        // A (note))
        if (ch == 65) {
            note = 0;
        }
        // B (note)
        if (ch == 66) {
            note = 2;
        }
        // C (note)
        if (ch == 67) {
            note = 3;
        }
        // D (note)
        if (ch == 68) {
            note = 5;
        }
        // E (note)
        if (ch == 69) {
            note = 7;
        }
        // F (note)
        if (ch == 70) {
            note = 8;
        }
        // G (note)
        if (ch == 71) {
            note = 10;
        }
        // '#' (sharp modifier)
        if (ch == 35) {
            note = note + 1;
        }
        // 'b' (flat modifier)
        if (ch == 98) {
            note = note - 1;
        }
        // ' (octave up)
        if (ch == 39) {
            prevPitch = prevPitch + 12;
        }
        // , (octave down)
        if (ch == 44) {
            prevPitch = prevPitch - 12;
        }
        // R (rest)
        if (ch == 82) {
            typ = 0;
        }
        // ! (staccato modifier)
        if (ch == 33) {
            typ = 1;
        }
        // ^ (short modifier)
        if (ch == 94) {
            typ = 2;
        }
        // _ (legato modifier)
        if (ch == 95) {
            typ = 4;
        }
        // - (glissando modifier)
        if (ch == 45) {
            typ = 5;
        }
        // % (tempo change)
        if ((ch == 37) && (num > 0)) {
            tempo = num;
            num = 0;
        }
        if ((ch >= 48) && (ch <= 57)) {
            // 0-9 (number)
            num = (num * 10) + (ch - 48);
        }
        if (ch == 46) {
            // . (duration modifier)
            num = ((num * 2) / (3));
        }
        if (((ch == 32) || (i+1 == notesLen)) && ((note > -99) || (typ != 3))) {
            if (num == 0) {
                num = prevDuration;
            } else {
                prevDuration = num;
            }
            ms = (int) floor(320000.0 / (tempo * num)+0.5);
            if (typ == 0) {
                this->addPulseToPlaySeq(0, ms);
            } else {
                dNote = note - (((prevPitch) % (12)));
                if (dNote > 6) {
                    dNote = dNote - 12;
                }
                if (dNote <= -6) {
                    dNote = dNote + 12;
                }
                pitch = prevPitch + dNote;
                freq = (int) floor(440 * exp(pitch * 0.05776226504666)+0.5);
                ms16 = ((ms) >> (4));
                rest = 0;
                if (typ == 3) {
                    rest = 2 * ms16;
                }
                if (typ == 2) {
                    rest = 8 * ms16;
                }
                if (typ == 1) {
                    rest = 12 * ms16;
                }
                if (typ == 5) {
                    this->addPulseToPlaySeq(prevFreq, ms16);
                    this->addFreqMoveToPlaySeq(freq, 8 * ms16);
                    this->addPulseToPlaySeq(freq, ms - 9 * ms16);
                } else {
                    this->addPulseToPlaySeq(freq, ms - rest);
                    if (rest > 0) {
                        this->addPulseToPlaySeq(0, rest);
                    }
                }
                prevFreq = freq;
                prevPitch = pitch;
            }
            note = -99;
            num = 0;
            typ = 3;
        }
        i = i + 1;
    }
    return YAPI_SUCCESS;
}

/**
 * Starts the preprogrammed playing sequence. The sequence
 * runs in loop until it is stopped by stopPlaySeq or an explicit
 * change. To play the sequence only once, use oncePlaySeq().
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YBuzzer::startPlaySeq(void)
{
    return this->sendCommand("S");
}

/**
 * Stops the preprogrammed playing sequence and sets the frequency to zero.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YBuzzer::stopPlaySeq(void)
{
    return this->sendCommand("X");
}

/**
 * Resets the preprogrammed playing sequence and sets the frequency to zero.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YBuzzer::resetPlaySeq(void)
{
    return this->sendCommand("Z");
}

/**
 * Starts the preprogrammed playing sequence and run it once only.
 *
 * @return YAPI_SUCCESS if the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
int YBuzzer::oncePlaySeq(void)
{
    return this->sendCommand("s");
}

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
int YBuzzer::pulse(int frequency,int duration)
{
    return this->set_command(YapiWrapper::ysprintf("P%d,%d",frequency,duration));
}

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
int YBuzzer::freqMove(int frequency,int duration)
{
    return this->set_command(YapiWrapper::ysprintf("F%d,%d",frequency,duration));
}

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
int YBuzzer::volumeMove(int volume,int duration)
{
    return this->set_command(YapiWrapper::ysprintf("V%d,%d",volume,duration));
}

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
int YBuzzer::playNotes(string notes)
{
    this->resetPlaySeq();
    this->addNotesToPlaySeq(notes);
    return this->oncePlaySeq();
}

YBuzzer *YBuzzer::nextBuzzer(void)
{
    string  hwid;

    if(YISERR(_nextFunction(hwid)) || hwid=="") {
        return NULL;
    }
    return YBuzzer::FindBuzzer(hwid);
}

YBuzzer* YBuzzer::FirstBuzzer(void)
{
    vector<YFUN_DESCR>   v_fundescr;
    YDEV_DESCR             ydevice;
    string              serial, funcId, funcName, funcVal, errmsg;

    if(YISERR(YapiWrapper::getFunctionsByClass("Buzzer", 0, v_fundescr, sizeof(YFUN_DESCR), errmsg)) ||
       v_fundescr.size() == 0 ||
       YISERR(YapiWrapper::getFunctionInfo(v_fundescr[0], ydevice, serial, funcId, funcName, funcVal, errmsg))) {
        return NULL;
    }
    return YBuzzer::FindBuzzer(serial+"."+funcId);
}

//--- (end of YBuzzer implementation)

//--- (YBuzzer functions)
//--- (end of YBuzzer functions)
