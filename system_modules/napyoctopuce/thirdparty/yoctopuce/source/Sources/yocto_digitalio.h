/*********************************************************************
 *
 * $Id: yocto_digitalio.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindDigitalIO(), the high-level API for DigitalIO functions
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


#ifndef YOCTO_DIGITALIO_H
#define YOCTO_DIGITALIO_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (YDigitalIO return codes)
//--- (end of YDigitalIO return codes)
//--- (YDigitalIO definitions)
class YDigitalIO; // forward declaration

typedef void (*YDigitalIOValueCallback)(YDigitalIO *func, const string& functionValue);
#ifndef _Y_OUTPUTVOLTAGE_ENUM
#define _Y_OUTPUTVOLTAGE_ENUM
typedef enum {
    Y_OUTPUTVOLTAGE_USB_5V = 0,
    Y_OUTPUTVOLTAGE_USB_3V = 1,
    Y_OUTPUTVOLTAGE_EXT_V = 2,
    Y_OUTPUTVOLTAGE_INVALID = -1,
} Y_OUTPUTVOLTAGE_enum;
#endif
#define Y_PORTSTATE_INVALID             (YAPI_INVALID_UINT)
#define Y_PORTDIRECTION_INVALID         (YAPI_INVALID_UINT)
#define Y_PORTOPENDRAIN_INVALID         (YAPI_INVALID_UINT)
#define Y_PORTPOLARITY_INVALID          (YAPI_INVALID_UINT)
#define Y_PORTDIAGS_INVALID             (YAPI_INVALID_UINT)
#define Y_PORTSIZE_INVALID              (YAPI_INVALID_UINT)
#define Y_COMMAND_INVALID               (YAPI_INVALID_STRING)
//--- (end of YDigitalIO definitions)

//--- (YDigitalIO declaration)
/**
 * YDigitalIO Class: Digital IO function interface
 *
 * The Yoctopuce application programming interface allows you to switch the state of each
 * bit of the I/O port. You can switch all bits at once, or one by one. The library
 * can also automatically generate short pulses of a determined duration. Electrical behavior
 * of each I/O can be modified (open drain and reverse polarity).
 */
class YOCTO_CLASS_EXPORT YDigitalIO: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of YDigitalIO declaration)
protected:
    //--- (YDigitalIO attributes)
    // Attributes (function value cache)
    int             _portState;
    int             _portDirection;
    int             _portOpenDrain;
    int             _portPolarity;
    int             _portDiags;
    int             _portSize;
    Y_OUTPUTVOLTAGE_enum _outputVoltage;
    string          _command;
    YDigitalIOValueCallback _valueCallbackDigitalIO;

    friend YDigitalIO *yFindDigitalIO(const string& func);
    friend YDigitalIO *yFirstDigitalIO(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindDigitalIO factory function to instantiate
    YDigitalIO(const string& func);
    //--- (end of YDigitalIO attributes)

public:
    ~YDigitalIO();
    //--- (YDigitalIO accessors declaration)

    static const int PORTSTATE_INVALID = YAPI_INVALID_UINT;
    static const int PORTDIRECTION_INVALID = YAPI_INVALID_UINT;
    static const int PORTOPENDRAIN_INVALID = YAPI_INVALID_UINT;
    static const int PORTPOLARITY_INVALID = YAPI_INVALID_UINT;
    static const int PORTDIAGS_INVALID = YAPI_INVALID_UINT;
    static const int PORTSIZE_INVALID = YAPI_INVALID_UINT;
    static const Y_OUTPUTVOLTAGE_enum OUTPUTVOLTAGE_USB_5V = Y_OUTPUTVOLTAGE_USB_5V;
    static const Y_OUTPUTVOLTAGE_enum OUTPUTVOLTAGE_USB_3V = Y_OUTPUTVOLTAGE_USB_3V;
    static const Y_OUTPUTVOLTAGE_enum OUTPUTVOLTAGE_EXT_V = Y_OUTPUTVOLTAGE_EXT_V;
    static const Y_OUTPUTVOLTAGE_enum OUTPUTVOLTAGE_INVALID = Y_OUTPUTVOLTAGE_INVALID;
    static const string COMMAND_INVALID;

    /**
     * Returns the digital IO port state: bit 0 represents input 0, and so on.
     *
     * @return an integer corresponding to the digital IO port state: bit 0 represents input 0, and so on
     *
     * On failure, throws an exception or returns Y_PORTSTATE_INVALID.
     */
    int                 get_portState(void);

    inline int          portState(void)
    { return this->get_portState(); }

    /**
     * Changes the digital IO port state: bit 0 represents input 0, and so on. This function has no effect
     * on bits configured as input in portDirection.
     *
     * @param newval : an integer corresponding to the digital IO port state: bit 0 represents input 0, and so on
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_portState(int newval);
    inline int      setPortState(int newval)
    { return this->set_portState(newval); }

    /**
     * Returns the IO direction of all bits of the port: 0 makes a bit an input, 1 makes it an output.
     *
     * @return an integer corresponding to the IO direction of all bits of the port: 0 makes a bit an
     * input, 1 makes it an output
     *
     * On failure, throws an exception or returns Y_PORTDIRECTION_INVALID.
     */
    int                 get_portDirection(void);

    inline int          portDirection(void)
    { return this->get_portDirection(); }

    /**
     * Changes the IO direction of all bits of the port: 0 makes a bit an input, 1 makes it an output.
     * Remember to call the saveToFlash() method  to make sure the setting is kept after a reboot.
     *
     * @param newval : an integer corresponding to the IO direction of all bits of the port: 0 makes a bit
     * an input, 1 makes it an output
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_portDirection(int newval);
    inline int      setPortDirection(int newval)
    { return this->set_portDirection(newval); }

    /**
     * Returns the electrical interface for each bit of the port. For each bit set to 0  the matching I/O
     * works in the regular,
     * intuitive way, for each bit set to 1, the I/O works in reverse mode.
     *
     * @return an integer corresponding to the electrical interface for each bit of the port
     *
     * On failure, throws an exception or returns Y_PORTOPENDRAIN_INVALID.
     */
    int                 get_portOpenDrain(void);

    inline int          portOpenDrain(void)
    { return this->get_portOpenDrain(); }

    /**
     * Changes the electrical interface for each bit of the port. 0 makes a bit a regular input/output, 1 makes
     * it an open-drain (open-collector) input/output. Remember to call the
     * saveToFlash() method  to make sure the setting is kept after a reboot.
     *
     * @param newval : an integer corresponding to the electrical interface for each bit of the port
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_portOpenDrain(int newval);
    inline int      setPortOpenDrain(int newval)
    { return this->set_portOpenDrain(newval); }

    /**
     * Returns the polarity of all the bits of the port.  For each bit set to 0, the matching I/O works the regular,
     * intuitive way; for each bit set to 1, the I/O works in reverse mode.
     *
     * @return an integer corresponding to the polarity of all the bits of the port
     *
     * On failure, throws an exception or returns Y_PORTPOLARITY_INVALID.
     */
    int                 get_portPolarity(void);

    inline int          portPolarity(void)
    { return this->get_portPolarity(); }

    /**
     * Changes the polarity of all the bits of the port: For each bit set to 0, the matching I/O works the regular,
     * intuitive way; for each bit set to 1, the I/O works in reverse mode.
     * Remember to call the saveToFlash() method  to make sure the setting will be kept after a reboot.
     *
     * @param newval : an integer corresponding to the polarity of all the bits of the port: For each bit
     * set to 0, the matching I/O works the regular,
     *         intuitive way; for each bit set to 1, the I/O works in reverse mode
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_portPolarity(int newval);
    inline int      setPortPolarity(int newval)
    { return this->set_portPolarity(newval); }

    /**
     * Returns the port state diagnostics (Yocto-IO and Yocto-MaxiIO-V2 only). Bit 0 indicates a shortcut on
     * output 0, etc. Bit 8 indicates a power failure, and bit 9 signals overheating (overcurrent).
     * During normal use, all diagnostic bits should stay clear.
     *
     * @return an integer corresponding to the port state diagnostics (Yocto-IO and Yocto-MaxiIO-V2 only)
     *
     * On failure, throws an exception or returns Y_PORTDIAGS_INVALID.
     */
    int                 get_portDiags(void);

    inline int          portDiags(void)
    { return this->get_portDiags(); }

    /**
     * Returns the number of bits implemented in the I/O port.
     *
     * @return an integer corresponding to the number of bits implemented in the I/O port
     *
     * On failure, throws an exception or returns Y_PORTSIZE_INVALID.
     */
    int                 get_portSize(void);

    inline int          portSize(void)
    { return this->get_portSize(); }

    /**
     * Returns the voltage source used to drive output bits.
     *
     * @return a value among Y_OUTPUTVOLTAGE_USB_5V, Y_OUTPUTVOLTAGE_USB_3V and Y_OUTPUTVOLTAGE_EXT_V
     * corresponding to the voltage source used to drive output bits
     *
     * On failure, throws an exception or returns Y_OUTPUTVOLTAGE_INVALID.
     */
    Y_OUTPUTVOLTAGE_enum get_outputVoltage(void);

    inline Y_OUTPUTVOLTAGE_enum outputVoltage(void)
    { return this->get_outputVoltage(); }

    /**
     * Changes the voltage source used to drive output bits.
     * Remember to call the saveToFlash() method  to make sure the setting is kept after a reboot.
     *
     * @param newval : a value among Y_OUTPUTVOLTAGE_USB_5V, Y_OUTPUTVOLTAGE_USB_3V and
     * Y_OUTPUTVOLTAGE_EXT_V corresponding to the voltage source used to drive output bits
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_outputVoltage(Y_OUTPUTVOLTAGE_enum newval);
    inline int      setOutputVoltage(Y_OUTPUTVOLTAGE_enum newval)
    { return this->set_outputVoltage(newval); }

    string              get_command(void);

    inline string       command(void)
    { return this->get_command(); }

    int             set_command(const string& newval);
    inline int      setCommand(const string& newval)
    { return this->set_command(newval); }

    /**
     * Retrieves a digital IO port for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the digital IO port is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YDigitalIO.isOnline() to test if the digital IO port is
     * indeed online at a given time. In case of ambiguity when looking for
     * a digital IO port by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the digital IO port
     *
     * @return a YDigitalIO object allowing you to drive the digital IO port.
     */
    static YDigitalIO*  FindDigitalIO(string func);

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
    virtual int         registerValueCallback(YDigitalIOValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Sets a single bit of the I/O port.
     *
     * @param bitno : the bit number; lowest bit has index 0
     * @param bitstate : the state of the bit (1 or 0)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_bitState(int bitno,int bitstate);

    /**
     * Returns the state of a single bit of the I/O port.
     *
     * @param bitno : the bit number; lowest bit has index 0
     *
     * @return the bit state (0 or 1)
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         get_bitState(int bitno);

    /**
     * Reverts a single bit of the I/O port.
     *
     * @param bitno : the bit number; lowest bit has index 0
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         toggle_bitState(int bitno);

    /**
     * Changes  the direction of a single bit from the I/O port.
     *
     * @param bitno : the bit number; lowest bit has index 0
     * @param bitdirection : direction to set, 0 makes the bit an input, 1 makes it an output.
     *         Remember to call the   saveToFlash() method to make sure the setting is kept after a reboot.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_bitDirection(int bitno,int bitdirection);

    /**
     * Returns the direction of a single bit from the I/O port (0 means the bit is an input, 1  an output).
     *
     * @param bitno : the bit number; lowest bit has index 0
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         get_bitDirection(int bitno);

    /**
     * Changes the polarity of a single bit from the I/O port.
     *
     * @param bitno : the bit number; lowest bit has index 0.
     * @param bitpolarity : polarity to set, 0 makes the I/O work in regular mode, 1 makes the I/O  works
     * in reverse mode.
     *         Remember to call the   saveToFlash() method to make sure the setting is kept after a reboot.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_bitPolarity(int bitno,int bitpolarity);

    /**
     * Returns the polarity of a single bit from the I/O port (0 means the I/O works in regular mode, 1
     * means the I/O  works in reverse mode).
     *
     * @param bitno : the bit number; lowest bit has index 0
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         get_bitPolarity(int bitno);

    /**
     * Changes  the electrical interface of a single bit from the I/O port.
     *
     * @param bitno : the bit number; lowest bit has index 0
     * @param opendrain : 0 makes a bit a regular input/output, 1 makes
     *         it an open-drain (open-collector) input/output. Remember to call the
     *         saveToFlash() method to make sure the setting is kept after a reboot.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_bitOpenDrain(int bitno,int opendrain);

    /**
     * Returns the type of electrical interface of a single bit from the I/O port. (0 means the bit is an
     * input, 1  an output).
     *
     * @param bitno : the bit number; lowest bit has index 0
     *
     * @return   0 means the a bit is a regular input/output, 1 means the bit is an open-drain
     *         (open-collector) input/output.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         get_bitOpenDrain(int bitno);

    /**
     * Triggers a pulse on a single bit for a specified duration. The specified bit
     * will be turned to 1, and then back to 0 after the given duration.
     *
     * @param bitno : the bit number; lowest bit has index 0
     * @param ms_duration : desired pulse duration in milliseconds. Be aware that the device time
     *         resolution is not guaranteed up to the millisecond.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         pulse(int bitno,int ms_duration);

    /**
     * Schedules a pulse on a single bit for a specified duration. The specified bit
     * will be turned to 1, and then back to 0 after the given duration.
     *
     * @param bitno : the bit number; lowest bit has index 0
     * @param ms_delay : waiting time before the pulse, in milliseconds
     * @param ms_duration : desired pulse duration in milliseconds. Be aware that the device time
     *         resolution is not guaranteed up to the millisecond.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         delayedPulse(int bitno,int ms_delay,int ms_duration);


    inline static YDigitalIO* Find(string func)
    { return YDigitalIO::FindDigitalIO(func); }

    /**
     * Continues the enumeration of digital IO ports started using yFirstDigitalIO().
     *
     * @return a pointer to a YDigitalIO object, corresponding to
     *         a digital IO port currently online, or a NULL pointer
     *         if there are no more digital IO ports to enumerate.
     */
           YDigitalIO      *nextDigitalIO(void);
    inline YDigitalIO      *next(void)
    { return this->nextDigitalIO();}

    /**
     * Starts the enumeration of digital IO ports currently accessible.
     * Use the method YDigitalIO.nextDigitalIO() to iterate on
     * next digital IO ports.
     *
     * @return a pointer to a YDigitalIO object, corresponding to
     *         the first digital IO port currently online, or a NULL pointer
     *         if there are none.
     */
           static YDigitalIO* FirstDigitalIO(void);
    inline static YDigitalIO* First(void)
    { return YDigitalIO::FirstDigitalIO();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of YDigitalIO accessors declaration)
};

//--- (YDigitalIO functions declaration)

/**
 * Retrieves a digital IO port for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the digital IO port is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YDigitalIO.isOnline() to test if the digital IO port is
 * indeed online at a given time. In case of ambiguity when looking for
 * a digital IO port by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the digital IO port
 *
 * @return a YDigitalIO object allowing you to drive the digital IO port.
 */
inline YDigitalIO* yFindDigitalIO(const string& func)
{ return YDigitalIO::FindDigitalIO(func);}
/**
 * Starts the enumeration of digital IO ports currently accessible.
 * Use the method YDigitalIO.nextDigitalIO() to iterate on
 * next digital IO ports.
 *
 * @return a pointer to a YDigitalIO object, corresponding to
 *         the first digital IO port currently online, or a NULL pointer
 *         if there are none.
 */
inline YDigitalIO* yFirstDigitalIO(void)
{ return YDigitalIO::FirstDigitalIO();}

//--- (end of YDigitalIO functions declaration)

#endif
