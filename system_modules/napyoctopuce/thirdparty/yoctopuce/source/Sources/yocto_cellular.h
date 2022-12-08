/*********************************************************************
 *
 * $Id: yocto_cellular.h 28748 2017-10-03 08:23:39Z seb $
 *
 * Declares yFindCellular(), the high-level API for Cellular functions
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


#ifndef YOCTO_CELLULAR_H
#define YOCTO_CELLULAR_H

#include "yocto_api.h"
#include <cfloat>
#include <cmath>
#include <map>

//--- (generated code: YCellular return codes)
//--- (end of generated code: YCellular return codes)
//--- (generated code: YCellular definitions)
class YCellular; // forward declaration

typedef void (*YCellularValueCallback)(YCellular *func, const string& functionValue);
#ifndef _Y_CELLTYPE_ENUM
#define _Y_CELLTYPE_ENUM
typedef enum {
    Y_CELLTYPE_GPRS = 0,
    Y_CELLTYPE_EGPRS = 1,
    Y_CELLTYPE_WCDMA = 2,
    Y_CELLTYPE_HSDPA = 3,
    Y_CELLTYPE_NONE = 4,
    Y_CELLTYPE_CDMA = 5,
    Y_CELLTYPE_INVALID = -1,
} Y_CELLTYPE_enum;
#endif
#ifndef _Y_AIRPLANEMODE_ENUM
#define _Y_AIRPLANEMODE_ENUM
typedef enum {
    Y_AIRPLANEMODE_OFF = 0,
    Y_AIRPLANEMODE_ON = 1,
    Y_AIRPLANEMODE_INVALID = -1,
} Y_AIRPLANEMODE_enum;
#endif
#ifndef _Y_ENABLEDATA_ENUM
#define _Y_ENABLEDATA_ENUM
typedef enum {
    Y_ENABLEDATA_HOMENETWORK = 0,
    Y_ENABLEDATA_ROAMING = 1,
    Y_ENABLEDATA_NEVER = 2,
    Y_ENABLEDATA_NEUTRALITY = 3,
    Y_ENABLEDATA_INVALID = -1,
} Y_ENABLEDATA_enum;
#endif
#define Y_LINKQUALITY_INVALID           (YAPI_INVALID_UINT)
#define Y_CELLOPERATOR_INVALID          (YAPI_INVALID_STRING)
#define Y_CELLIDENTIFIER_INVALID        (YAPI_INVALID_STRING)
#define Y_IMSI_INVALID                  (YAPI_INVALID_STRING)
#define Y_MESSAGE_INVALID               (YAPI_INVALID_STRING)
#define Y_PIN_INVALID                   (YAPI_INVALID_STRING)
#define Y_LOCKEDOPERATOR_INVALID        (YAPI_INVALID_STRING)
#define Y_APN_INVALID                   (YAPI_INVALID_STRING)
#define Y_APNSECRET_INVALID             (YAPI_INVALID_STRING)
#define Y_PINGINTERVAL_INVALID          (YAPI_INVALID_UINT)
#define Y_DATASENT_INVALID              (YAPI_INVALID_UINT)
#define Y_DATARECEIVED_INVALID          (YAPI_INVALID_UINT)
#define Y_COMMAND_INVALID               (YAPI_INVALID_STRING)
//--- (end of generated code: YCellular definitions)

//--- (generated code: YCellRecord definitions)
//--- (end of generated code: YCellRecord definitions)

//--- (generated code: YCellRecord declaration)
/**
 * YCellRecord Class: Description of a cellular antenna
 *
 *
 */
class YOCTO_CLASS_EXPORT YCellRecord {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YCellRecord declaration)
    //--- (generated code: YCellRecord attributes)
    // Attributes (function value cache)
    string          _oper;
    int             _mcc;
    int             _mnc;
    int             _lac;
    int             _cid;
    int             _dbm;
    int             _tad;
    //--- (end of generated code: YCellRecord attributes)
    //--- (generated code: YCellRecord constructor)

    //--- (end of generated code: YCellRecord constructor)
    //--- (generated code: YCellRecord initialization)
    //--- (end of generated code: YCellRecord initialization)

public:
    YCellRecord(int mcc,int mnc,int lac,int cellId,int dbm,int tad,const string &oper);
    //--- (generated code: YCellRecord accessors declaration)


    virtual string      get_cellOperator(void);

    virtual int         get_mobileCountryCode(void);

    virtual int         get_mobileNetworkCode(void);

    virtual int         get_locationAreaCode(void);

    virtual int         get_cellId(void);

    virtual int         get_signalStrength(void);

    virtual int         get_timingAdvance(void);

#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YCellRecord accessors declaration)
};


//--- (generated code: YCellular declaration)
/**
 * YCellular Class: Cellular function interface
 *
 * YCellular functions provides control over cellular network parameters
 * and status for devices that are GSM-enabled.
 */
class YOCTO_CLASS_EXPORT YCellular: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YCellular declaration)
protected:
    //--- (generated code: YCellular attributes)
    // Attributes (function value cache)
    int             _linkQuality;
    string          _cellOperator;
    string          _cellIdentifier;
    Y_CELLTYPE_enum _cellType;
    string          _imsi;
    string          _message;
    string          _pin;
    string          _lockedOperator;
    Y_AIRPLANEMODE_enum _airplaneMode;
    Y_ENABLEDATA_enum _enableData;
    string          _apn;
    string          _apnSecret;
    int             _pingInterval;
    int             _dataSent;
    int             _dataReceived;
    string          _command;
    YCellularValueCallback _valueCallbackCellular;

    friend YCellular *yFindCellular(const string& func);
    friend YCellular *yFirstCellular(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindCellular factory function to instantiate
    YCellular(const string& func);
    //--- (end of generated code: YCellular attributes)

public:
    ~YCellular();
    //--- (generated code: YCellular accessors declaration)

    static const int LINKQUALITY_INVALID = YAPI_INVALID_UINT;
    static const string CELLOPERATOR_INVALID;
    static const string CELLIDENTIFIER_INVALID;
    static const Y_CELLTYPE_enum CELLTYPE_GPRS = Y_CELLTYPE_GPRS;
    static const Y_CELLTYPE_enum CELLTYPE_EGPRS = Y_CELLTYPE_EGPRS;
    static const Y_CELLTYPE_enum CELLTYPE_WCDMA = Y_CELLTYPE_WCDMA;
    static const Y_CELLTYPE_enum CELLTYPE_HSDPA = Y_CELLTYPE_HSDPA;
    static const Y_CELLTYPE_enum CELLTYPE_NONE = Y_CELLTYPE_NONE;
    static const Y_CELLTYPE_enum CELLTYPE_CDMA = Y_CELLTYPE_CDMA;
    static const Y_CELLTYPE_enum CELLTYPE_INVALID = Y_CELLTYPE_INVALID;
    static const string IMSI_INVALID;
    static const string MESSAGE_INVALID;
    static const string PIN_INVALID;
    static const string LOCKEDOPERATOR_INVALID;
    static const Y_AIRPLANEMODE_enum AIRPLANEMODE_OFF = Y_AIRPLANEMODE_OFF;
    static const Y_AIRPLANEMODE_enum AIRPLANEMODE_ON = Y_AIRPLANEMODE_ON;
    static const Y_AIRPLANEMODE_enum AIRPLANEMODE_INVALID = Y_AIRPLANEMODE_INVALID;
    static const Y_ENABLEDATA_enum ENABLEDATA_HOMENETWORK = Y_ENABLEDATA_HOMENETWORK;
    static const Y_ENABLEDATA_enum ENABLEDATA_ROAMING = Y_ENABLEDATA_ROAMING;
    static const Y_ENABLEDATA_enum ENABLEDATA_NEVER = Y_ENABLEDATA_NEVER;
    static const Y_ENABLEDATA_enum ENABLEDATA_NEUTRALITY = Y_ENABLEDATA_NEUTRALITY;
    static const Y_ENABLEDATA_enum ENABLEDATA_INVALID = Y_ENABLEDATA_INVALID;
    static const string APN_INVALID;
    static const string APNSECRET_INVALID;
    static const int PINGINTERVAL_INVALID = YAPI_INVALID_UINT;
    static const int DATASENT_INVALID = YAPI_INVALID_UINT;
    static const int DATARECEIVED_INVALID = YAPI_INVALID_UINT;
    static const string COMMAND_INVALID;

    /**
     * Returns the link quality, expressed in percent.
     *
     * @return an integer corresponding to the link quality, expressed in percent
     *
     * On failure, throws an exception or returns Y_LINKQUALITY_INVALID.
     */
    int                 get_linkQuality(void);

    inline int          linkQuality(void)
    { return this->get_linkQuality(); }

    /**
     * Returns the name of the cell operator currently in use.
     *
     * @return a string corresponding to the name of the cell operator currently in use
     *
     * On failure, throws an exception or returns Y_CELLOPERATOR_INVALID.
     */
    string              get_cellOperator(void);

    inline string       cellOperator(void)
    { return this->get_cellOperator(); }

    /**
     * Returns the unique identifier of the cellular antenna in use: MCC, MNC, LAC and Cell ID.
     *
     * @return a string corresponding to the unique identifier of the cellular antenna in use: MCC, MNC,
     * LAC and Cell ID
     *
     * On failure, throws an exception or returns Y_CELLIDENTIFIER_INVALID.
     */
    string              get_cellIdentifier(void);

    inline string       cellIdentifier(void)
    { return this->get_cellIdentifier(); }

    /**
     * Active cellular connection type.
     *
     * @return a value among Y_CELLTYPE_GPRS, Y_CELLTYPE_EGPRS, Y_CELLTYPE_WCDMA, Y_CELLTYPE_HSDPA,
     * Y_CELLTYPE_NONE and Y_CELLTYPE_CDMA
     *
     * On failure, throws an exception or returns Y_CELLTYPE_INVALID.
     */
    Y_CELLTYPE_enum     get_cellType(void);

    inline Y_CELLTYPE_enum cellType(void)
    { return this->get_cellType(); }

    /**
     * Returns an opaque string if a PIN code has been configured in the device to access
     * the SIM card, or an empty string if none has been configured or if the code provided
     * was rejected by the SIM card.
     *
     * @return a string corresponding to an opaque string if a PIN code has been configured in the device to access
     *         the SIM card, or an empty string if none has been configured or if the code provided
     *         was rejected by the SIM card
     *
     * On failure, throws an exception or returns Y_IMSI_INVALID.
     */
    string              get_imsi(void);

    inline string       imsi(void)
    { return this->get_imsi(); }

    /**
     * Returns the latest status message from the wireless interface.
     *
     * @return a string corresponding to the latest status message from the wireless interface
     *
     * On failure, throws an exception or returns Y_MESSAGE_INVALID.
     */
    string              get_message(void);

    inline string       message(void)
    { return this->get_message(); }

    /**
     * Returns an opaque string if a PIN code has been configured in the device to access
     * the SIM card, or an empty string if none has been configured or if the code provided
     * was rejected by the SIM card.
     *
     * @return a string corresponding to an opaque string if a PIN code has been configured in the device to access
     *         the SIM card, or an empty string if none has been configured or if the code provided
     *         was rejected by the SIM card
     *
     * On failure, throws an exception or returns Y_PIN_INVALID.
     */
    string              get_pin(void);

    inline string       pin(void)
    { return this->get_pin(); }

    /**
     * Changes the PIN code used by the module to access the SIM card.
     * This function does not change the code on the SIM card itself, but only changes
     * the parameter used by the device to try to get access to it. If the SIM code
     * does not work immediately on first try, it will be automatically forgotten
     * and the message will be set to "Enter SIM PIN". The method should then be
     * invoked again with right correct PIN code. After three failed attempts in a row,
     * the message is changed to "Enter SIM PUK" and the SIM card PUK code must be
     * provided using method sendPUK.
     *
     * Remember to call the saveToFlash() method of the module to save the
     * new value in the device flash.
     *
     * @param newval : a string corresponding to the PIN code used by the module to access the SIM card
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_pin(const string& newval);
    inline int      setPin(const string& newval)
    { return this->set_pin(newval); }

    /**
     * Returns the name of the only cell operator to use if automatic choice is disabled,
     * or an empty string if the SIM card will automatically choose among available
     * cell operators.
     *
     * @return a string corresponding to the name of the only cell operator to use if automatic choice is disabled,
     *         or an empty string if the SIM card will automatically choose among available
     *         cell operators
     *
     * On failure, throws an exception or returns Y_LOCKEDOPERATOR_INVALID.
     */
    string              get_lockedOperator(void);

    inline string       lockedOperator(void)
    { return this->get_lockedOperator(); }

    /**
     * Changes the name of the cell operator to be used. If the name is an empty
     * string, the choice will be made automatically based on the SIM card. Otherwise,
     * the selected operator is the only one that will be used.
     *
     * @param newval : a string corresponding to the name of the cell operator to be used
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_lockedOperator(const string& newval);
    inline int      setLockedOperator(const string& newval)
    { return this->set_lockedOperator(newval); }

    /**
     * Returns true if the airplane mode is active (radio turned off).
     *
     * @return either Y_AIRPLANEMODE_OFF or Y_AIRPLANEMODE_ON, according to true if the airplane mode is
     * active (radio turned off)
     *
     * On failure, throws an exception or returns Y_AIRPLANEMODE_INVALID.
     */
    Y_AIRPLANEMODE_enum get_airplaneMode(void);

    inline Y_AIRPLANEMODE_enum airplaneMode(void)
    { return this->get_airplaneMode(); }

    /**
     * Changes the activation state of airplane mode (radio turned off).
     *
     * @param newval : either Y_AIRPLANEMODE_OFF or Y_AIRPLANEMODE_ON, according to the activation state
     * of airplane mode (radio turned off)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_airplaneMode(Y_AIRPLANEMODE_enum newval);
    inline int      setAirplaneMode(Y_AIRPLANEMODE_enum newval)
    { return this->set_airplaneMode(newval); }

    /**
     * Returns the condition for enabling IP data services (GPRS).
     * When data services are disabled, SMS are the only mean of communication.
     *
     * @return a value among Y_ENABLEDATA_HOMENETWORK, Y_ENABLEDATA_ROAMING, Y_ENABLEDATA_NEVER and
     * Y_ENABLEDATA_NEUTRALITY corresponding to the condition for enabling IP data services (GPRS)
     *
     * On failure, throws an exception or returns Y_ENABLEDATA_INVALID.
     */
    Y_ENABLEDATA_enum   get_enableData(void);

    inline Y_ENABLEDATA_enum enableData(void)
    { return this->get_enableData(); }

    /**
     * Changes the condition for enabling IP data services (GPRS).
     * The service can be either fully deactivated, or limited to the SIM home network,
     * or enabled for all partner networks (roaming). Caution: enabling data services
     * on roaming networks may cause prohibitive communication costs !
     *
     * When data services are disabled, SMS are the only mean of communication.
     *
     * @param newval : a value among Y_ENABLEDATA_HOMENETWORK, Y_ENABLEDATA_ROAMING, Y_ENABLEDATA_NEVER
     * and Y_ENABLEDATA_NEUTRALITY corresponding to the condition for enabling IP data services (GPRS)
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_enableData(Y_ENABLEDATA_enum newval);
    inline int      setEnableData(Y_ENABLEDATA_enum newval)
    { return this->set_enableData(newval); }

    /**
     * Returns the Access Point Name (APN) to be used, if needed.
     * When left blank, the APN suggested by the cell operator will be used.
     *
     * @return a string corresponding to the Access Point Name (APN) to be used, if needed
     *
     * On failure, throws an exception or returns Y_APN_INVALID.
     */
    string              get_apn(void);

    inline string       apn(void)
    { return this->get_apn(); }

    /**
     * Returns the Access Point Name (APN) to be used, if needed.
     * When left blank, the APN suggested by the cell operator will be used.
     *
     * @param newval : a string
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_apn(const string& newval);
    inline int      setApn(const string& newval)
    { return this->set_apn(newval); }

    /**
     * Returns an opaque string if APN authentication parameters have been configured
     * in the device, or an empty string otherwise.
     * To configure these parameters, use set_apnAuth().
     *
     * @return a string corresponding to an opaque string if APN authentication parameters have been configured
     *         in the device, or an empty string otherwise
     *
     * On failure, throws an exception or returns Y_APNSECRET_INVALID.
     */
    string              get_apnSecret(void);

    inline string       apnSecret(void)
    { return this->get_apnSecret(); }

    int             set_apnSecret(const string& newval);
    inline int      setApnSecret(const string& newval)
    { return this->set_apnSecret(newval); }

    /**
     * Returns the automated connectivity check interval, in seconds.
     *
     * @return an integer corresponding to the automated connectivity check interval, in seconds
     *
     * On failure, throws an exception or returns Y_PINGINTERVAL_INVALID.
     */
    int                 get_pingInterval(void);

    inline int          pingInterval(void)
    { return this->get_pingInterval(); }

    /**
     * Changes the automated connectivity check interval, in seconds.
     *
     * @param newval : an integer corresponding to the automated connectivity check interval, in seconds
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_pingInterval(int newval);
    inline int      setPingInterval(int newval)
    { return this->set_pingInterval(newval); }

    /**
     * Returns the number of bytes sent so far.
     *
     * @return an integer corresponding to the number of bytes sent so far
     *
     * On failure, throws an exception or returns Y_DATASENT_INVALID.
     */
    int                 get_dataSent(void);

    inline int          dataSent(void)
    { return this->get_dataSent(); }

    /**
     * Changes the value of the outgoing data counter.
     *
     * @param newval : an integer corresponding to the value of the outgoing data counter
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_dataSent(int newval);
    inline int      setDataSent(int newval)
    { return this->set_dataSent(newval); }

    /**
     * Returns the number of bytes received so far.
     *
     * @return an integer corresponding to the number of bytes received so far
     *
     * On failure, throws an exception or returns Y_DATARECEIVED_INVALID.
     */
    int                 get_dataReceived(void);

    inline int          dataReceived(void)
    { return this->get_dataReceived(); }

    /**
     * Changes the value of the incoming data counter.
     *
     * @param newval : an integer corresponding to the value of the incoming data counter
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_dataReceived(int newval);
    inline int      setDataReceived(int newval)
    { return this->set_dataReceived(newval); }

    string              get_command(void);

    inline string       command(void)
    { return this->get_command(); }

    int             set_command(const string& newval);
    inline int      setCommand(const string& newval)
    { return this->set_command(newval); }

    /**
     * Retrieves a cellular interface for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the cellular interface is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YCellular.isOnline() to test if the cellular interface is
     * indeed online at a given time. In case of ambiguity when looking for
     * a cellular interface by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the cellular interface
     *
     * @return a YCellular object allowing you to drive the cellular interface.
     */
    static YCellular*   FindCellular(string func);

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
    virtual int         registerValueCallback(YCellularValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Sends a PUK code to unlock the SIM card after three failed PIN code attempts, and
     * setup a new PIN into the SIM card. Only ten consecutives tentatives are permitted:
     * after that, the SIM card will be blocked permanently without any mean of recovery
     * to use it again. Note that after calling this method, you have usually to invoke
     * method set_pin() to tell the YoctoHub which PIN to use in the future.
     *
     * @param puk : the SIM PUK code
     * @param newPin : new PIN code to configure into the SIM card
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         sendPUK(string puk,string newPin);

    /**
     * Configure authentication parameters to connect to the APN. Both
     * PAP and CHAP authentication are supported.
     *
     * @param username : APN username
     * @param password : APN password
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_apnAuth(string username,string password);

    /**
     * Clear the transmitted data counters.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         clearDataCounters(void);

    /**
     * Sends an AT command to the GSM module and returns the command output.
     * The command will only execute when the GSM module is in standard
     * command state, and should leave it in the exact same state.
     * Use this function with great care !
     *
     * @param cmd : the AT command to execute, like for instance: "+CCLK?".
     *
     * @return a string with the result of the commands. Empty lines are
     *         automatically removed from the output.
     */
    virtual string      _AT(string cmd);

    /**
     * Returns the list detected cell operators in the neighborhood.
     * This function will typically take between 30 seconds to 1 minute to
     * return. Note that any SIM card can usually only connect to specific
     * operators. All networks returned by this function might therefore
     * not be available for connection.
     *
     * @return a list of string (cell operator names).
     */
    virtual vector<string> get_availableOperators(void);

    /**
     * Returns a list of nearby cellular antennas, as required for quick
     * geolocation of the device. The first cell listed is the serving
     * cell, and the next ones are the neighboor cells reported by the
     * serving cell.
     *
     * @return a list of YCellRecords.
     */
    virtual vector<YCellRecord> quickCellSurvey(void);


    inline static YCellular* Find(string func)
    { return YCellular::FindCellular(func); }

    /**
     * Continues the enumeration of cellular interfaces started using yFirstCellular().
     *
     * @return a pointer to a YCellular object, corresponding to
     *         a cellular interface currently online, or a NULL pointer
     *         if there are no more cellular interfaces to enumerate.
     */
           YCellular       *nextCellular(void);
    inline YCellular       *next(void)
    { return this->nextCellular();}

    /**
     * Starts the enumeration of cellular interfaces currently accessible.
     * Use the method YCellular.nextCellular() to iterate on
     * next cellular interfaces.
     *
     * @return a pointer to a YCellular object, corresponding to
     *         the first cellular interface currently online, or a NULL pointer
     *         if there are none.
     */
           static YCellular* FirstCellular(void);
    inline static YCellular* First(void)
    { return YCellular::FirstCellular();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YCellular accessors declaration)
};

//--- (generated code: YCellular functions declaration)

/**
 * Retrieves a cellular interface for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the cellular interface is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YCellular.isOnline() to test if the cellular interface is
 * indeed online at a given time. In case of ambiguity when looking for
 * a cellular interface by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the cellular interface
 *
 * @return a YCellular object allowing you to drive the cellular interface.
 */
inline YCellular* yFindCellular(const string& func)
{ return YCellular::FindCellular(func);}
/**
 * Starts the enumeration of cellular interfaces currently accessible.
 * Use the method YCellular.nextCellular() to iterate on
 * next cellular interfaces.
 *
 * @return a pointer to a YCellular object, corresponding to
 *         the first cellular interface currently online, or a NULL pointer
 *         if there are none.
 */
inline YCellular* yFirstCellular(void)
{ return YCellular::FirstCellular();}

//--- (end of generated code: YCellular functions declaration)

#endif
