/*********************************************************************
 *
 * $Id: yocto_api.h 29669 2018-01-19 08:25:56Z seb $
 *
 * High-level programming interface, common to all modules
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
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
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

#ifndef YOCTO_API_H
#define YOCTO_API_H

#include "yapi/ydef.h"
#include "yapi/yjson.h"
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <stdexcept>
#include <cfloat>
#include <cmath>

#if defined(WINDOWS_API)
#if defined(GENERATE_DLL) || defined(YOCTOPUCEDLL_EXPORTS)
#define YOCTO_CLASS_EXPORT __declspec(dllexport)
#pragma warning(disable: 4251)
#else
#define YOCTO_CLASS_EXPORT
#endif
#else
#define YOCTO_CLASS_EXPORT
#endif

using std::string;
using std::vector;
using std::queue;
using std::map;

#define YAPI_INVALID_STRING         "!INVALID!"
#define YAPI_INVALID_INT            (0x7FFFFFFF)
#define YAPI_INVALID_UINT           (-1)
#define YAPI_INVALID_LONG           (0x7FFFFFFFFFFFFFFFLL)
#define YAPI_INVALID_DOUBLE         (-DBL_MAX)


//--- (generated code: YFunction definitions)
class YFunction; // forward declaration

typedef void (*YFunctionValueCallback)(YFunction *func, const string& functionValue);
#define Y_LOGICALNAME_INVALID           (YAPI_INVALID_STRING)
#define Y_ADVERTISEDVALUE_INVALID       (YAPI_INVALID_STRING)
//--- (end of generated code: YFunction definitions)


//--- (generated code: YModule definitions)
class YModule; // forward declaration

typedef void (*YModuleLogCallback)(YModule *module, const string& logline);
typedef void (*YModuleValueCallback)(YModule *func, const string& functionValue);
#ifndef _Y_PERSISTENTSETTINGS_ENUM
#define _Y_PERSISTENTSETTINGS_ENUM
typedef enum {
    Y_PERSISTENTSETTINGS_LOADED = 0,
    Y_PERSISTENTSETTINGS_SAVED = 1,
    Y_PERSISTENTSETTINGS_MODIFIED = 2,
    Y_PERSISTENTSETTINGS_INVALID = -1,
} Y_PERSISTENTSETTINGS_enum;
#endif
#ifndef _Y_BEACON_ENUM
#define _Y_BEACON_ENUM
typedef enum {
    Y_BEACON_OFF = 0,
    Y_BEACON_ON = 1,
    Y_BEACON_INVALID = -1,
} Y_BEACON_enum;
#endif
#define Y_PRODUCTNAME_INVALID           (YAPI_INVALID_STRING)
#define Y_SERIALNUMBER_INVALID          (YAPI_INVALID_STRING)
#define Y_PRODUCTID_INVALID             (YAPI_INVALID_UINT)
#define Y_PRODUCTRELEASE_INVALID        (YAPI_INVALID_UINT)
#define Y_FIRMWARERELEASE_INVALID       (YAPI_INVALID_STRING)
#define Y_LUMINOSITY_INVALID            (YAPI_INVALID_UINT)
#define Y_UPTIME_INVALID                (YAPI_INVALID_LONG)
#define Y_USBCURRENT_INVALID            (YAPI_INVALID_UINT)
#define Y_REBOOTCOUNTDOWN_INVALID       (YAPI_INVALID_INT)
#define Y_USERVAR_INVALID               (YAPI_INVALID_INT)
//--- (end of generated code: YModule definitions)

class YMeasure; // forward declaration
//--- (generated code: YSensor definitions)
class YSensor; // forward declaration

typedef void (*YSensorValueCallback)(YSensor *func, const string& functionValue);
class YMeasure; // forward declaration
typedef void (*YSensorTimedReportCallback)(YSensor *func, YMeasure measure);
#ifndef _Y_ADVMODE_ENUM
#define _Y_ADVMODE_ENUM
typedef enum {
    Y_ADVMODE_IMMEDIATE = 0,
    Y_ADVMODE_PERIOD_AVG = 1,
    Y_ADVMODE_PERIOD_MIN = 2,
    Y_ADVMODE_PERIOD_MAX = 3,
    Y_ADVMODE_INVALID = -1,
} Y_ADVMODE_enum;
#endif
#define Y_UNIT_INVALID                  (YAPI_INVALID_STRING)
#define Y_CURRENTVALUE_INVALID          (YAPI_INVALID_DOUBLE)
#define Y_LOWESTVALUE_INVALID           (YAPI_INVALID_DOUBLE)
#define Y_HIGHESTVALUE_INVALID          (YAPI_INVALID_DOUBLE)
#define Y_CURRENTRAWVALUE_INVALID       (YAPI_INVALID_DOUBLE)
#define Y_LOGFREQUENCY_INVALID          (YAPI_INVALID_STRING)
#define Y_REPORTFREQUENCY_INVALID       (YAPI_INVALID_STRING)
#define Y_CALIBRATIONPARAM_INVALID      (YAPI_INVALID_STRING)
#define Y_RESOLUTION_INVALID            (YAPI_INVALID_DOUBLE)
#define Y_SENSORSTATE_INVALID           (YAPI_INVALID_INT)
//--- (end of generated code: YSensor definitions)


//--- (generated code: YDataStream definitions)
//--- (end of generated code: YDataStream definitions)

//--- (generated code: YFirmwareUpdate definitions)
//--- (end of generated code: YFirmwareUpdate definitions)


//--- (generated code: YMeasure definitions)
//--- (end of generated code: YMeasure definitions)


//--- (generated code: YDataLogger definitions)
class YDataLogger; // forward declaration

typedef void (*YDataLoggerValueCallback)(YDataLogger *func, const string& functionValue);
#ifndef _Y_RECORDING_ENUM
#define _Y_RECORDING_ENUM
typedef enum {
    Y_RECORDING_OFF = 0,
    Y_RECORDING_ON = 1,
    Y_RECORDING_PENDING = 2,
    Y_RECORDING_INVALID = -1,
} Y_RECORDING_enum;
#endif
#ifndef _Y_AUTOSTART_ENUM
#define _Y_AUTOSTART_ENUM
typedef enum {
    Y_AUTOSTART_OFF = 0,
    Y_AUTOSTART_ON = 1,
    Y_AUTOSTART_INVALID = -1,
} Y_AUTOSTART_enum;
#endif
#ifndef _Y_BEACONDRIVEN_ENUM
#define _Y_BEACONDRIVEN_ENUM
typedef enum {
    Y_BEACONDRIVEN_OFF = 0,
    Y_BEACONDRIVEN_ON = 1,
    Y_BEACONDRIVEN_INVALID = -1,
} Y_BEACONDRIVEN_enum;
#endif
#ifndef _Y_CLEARHISTORY_ENUM
#define _Y_CLEARHISTORY_ENUM
typedef enum {
    Y_CLEARHISTORY_FALSE = 0,
    Y_CLEARHISTORY_TRUE = 1,
    Y_CLEARHISTORY_INVALID = -1,
} Y_CLEARHISTORY_enum;
#endif
#define Y_CURRENTRUNINDEX_INVALID       (YAPI_INVALID_UINT)
#define Y_TIMEUTC_INVALID               (YAPI_INVALID_LONG)
//--- (end of generated code: YDataLogger definitions)


// yInitAPI argument
const int Y_DETECT_NONE = 0;
const int Y_DETECT_USB = 1;
const int Y_DETECT_NET = 2;
const int Y_RESEND_MISSING_PKT = 4;
const int Y_DETECT_ALL = (Y_DETECT_USB | Y_DETECT_NET);

// Forward-declaration
class YDataSet;
class YFunction;

/// prototype of the log callback
typedef void (*yLogFunction)(const string& msg);

/// prototype of the device arrival/update/removal callback
typedef void (*yDeviceUpdateCallback)(YModule *module);

/// prototype of the Hub discoverycallback
typedef void (*YHubDiscoveryCallback)(const string& serial, const string& url);



/// prototype of the value calibration handlers
typedef vector<double>  floatArr;
typedef vector<int>     intArr;
typedef double (*yCalibrationHandler)(double rawValue, int calibType, vector<int> params, vector<double> rawValues, vector<double> refValues);

typedef YAPI_DEVICE     YDEV_DESCR;
typedef YAPI_FUNCTION   YFUN_DESCR;
#define Y_FUNCTIONDESCRIPTOR_INVALID    (-1)
#define Y_HARDWAREID_INVALID            (YAPI_INVALID_STRING)
#define Y_FUNCTIONID_INVALID            (YAPI_INVALID_STRING)
#define Y_FRIENDLYNAME_INVALID          (YAPI_INVALID_STRING)

#define Y_DATA_INVALID                  (-DBL_MAX)
#define Y_DURATION_INVALID              (-1)

//
// Class used to report exceptions within Yocto-API
// Do not instantiate directly
//
class YAPI_Exception : public std::runtime_error {
public:
    YRETCODE errorType;
    explicit YAPI_Exception(YRETCODE errType, string errMsg) : std::runtime_error(errMsg), errorType(errType) { }
};

typedef enum {
    YAPI_DEV_ARRIVAL,
    YAPI_DEV_REMOVAL,
    YAPI_DEV_CHANGE,
    YAPI_DEV_LOG,
	YAPI_HUB_DISCOVER
} yapiGlobalEventType;

typedef struct{
	yapiGlobalEventType    type;
    union{
        YModule		*module;
		struct {
			char serial[YOCTO_SERIAL_LEN];
			char url[64];
		};
    };
}yapiGlobalEvent;



typedef enum {
	YAPI_FUN_UPDATE,
	YAPI_FUN_VALUE,
	YAPI_FUN_TIMEDREPORT,
	YAPI_FUN_REFRESH
} yapiDataEventType;

typedef struct{
	yapiDataEventType    type;
	union{
		struct {
			YFunction   *fun;
			char        value[YOCTO_PUBVAL_LEN];
		};
		struct {
			YSensor    *sensor;
			double      timestamp;
			int         len;
			int			report[18];
		};
	};
}yapiDataEvent;


// internal helper function
s64 yatoi(const char *c);
int _ystrpos(const string& haystack, const string& needle);
vector<string> _strsplit(const string& str, char delimiter);

typedef enum {
    STRING,
    NUMBER,
    ARRAY,
    OBJECT
} YJSONType;

typedef enum {
    JSTART,
    JWAITFORNAME,
    JWAITFORENDOFNAME,
    JWAITFORCOLON,
    JWAITFORDATA,
    JWAITFORNEXTSTRUCTMEMBER,
    JWAITFORNEXTARRAYITEM,
    JWAITFORSTRINGVALUE,
    JWAITFORSTRINGVALUE_ESC,
    JWAITFORINTVALUE,
    JWAITFORBOOLVALUE
} Tjstate;

class YJSONObject;

class YJSONContent
{
    public:
        string _data;
        int _data_start;
        int _data_len;
        int _data_boundary;
        YJSONType _type;
        static YJSONContent* ParseJson(const string& data, int start, int stop);
        YJSONContent(const string& data, int start, int stop, YJSONType type);
        YJSONContent(YJSONContent *ref);
        YJSONContent(YJSONType type);
        virtual ~YJSONContent();
        YJSONType getJSONType();
        virtual int parse()=0;
        static int SkipGarbage(const string& data, int start, int stop);
        string FormatError(const string& errmsg, int cur_pos);
        virtual string toJSON()=0;
        virtual string toString()=0;
};

class YJSONArray : public YJSONContent
{
        vector<YJSONContent*> _arrayValue;
    public:
        YJSONArray(const string& data, int start, int stop);
        YJSONArray(const string& data);
        YJSONArray(YJSONArray *ref);
        YJSONArray();
        virtual ~YJSONArray();
        int length();
        virtual int parse();
        YJSONObject* getYJSONObject(int i);
        string getString(int i);
        YJSONContent* get(int i);
        YJSONArray* getYJSONArray(int i);
        int getInt(int i);
        s64 getLong(int i);
        void put(const string& flatAttr);
        virtual string toJSON();
        virtual string toString();
};

class YJSONString : public YJSONContent
{
        string _stringValue;
    public:
        YJSONString(const string& data, int start, int stop);
        YJSONString(YJSONString *ref);
        YJSONString();

        virtual ~YJSONString() { }

    virtual int parse();
        virtual string toJSON();
        string getString();
        virtual string toString();
        void setContent(const string& value);
};


class YJSONNumber : public YJSONContent
{
        s64 _intValue;
        double _doubleValue;
        bool _isFloat;
    public:
        YJSONNumber(const string& data, int start, int stop);
        YJSONNumber(YJSONNumber *ref);

        virtual ~YJSONNumber()    { }

    virtual int parse();
        virtual string toJSON();
        s64 getLong();
        int getInt();
        double getDouble();
        virtual string toString();
};


class YJSONObject : public YJSONContent
{
    map<string, YJSONContent*> _parsed;
    vector<string> _keys;
    void convert(YJSONObject* reference, YJSONArray* newArray);
public:
    YJSONObject(const string& data);
    YJSONObject(const string& data, int start, int len);
    YJSONObject(YJSONObject *ref);
    virtual ~YJSONObject();

    virtual int parse();
    bool has(const string& key);
    YJSONObject* getYJSONObject(const string& key);
    YJSONString* getYJSONString(const string& key);
    YJSONArray* getYJSONArray(const string& key);
    vector<string> keys();
    YJSONNumber* getYJSONNumber(const string& key);
    string getString(const string& key);
    int getInt(const string& key);
    YJSONContent* get(const string& key);
    s64 getLong(const string& key);
    double getDouble(const string& key);
    virtual string toJSON();
    virtual string toString();
    void parseWithRef(YJSONObject* reference);
    string getKeyFromIdx(int i);
};



//
// YAPI Context
//
// This class provides C++-style entry points to lowlevcel functions defined to yapi.h
// Could be implemented by a singleton, we use static methods insead
//
class YOCTO_CLASS_EXPORT YAPI {
private:
    static  queue<yapiGlobalEvent>  _plug_events;
    static  queue<yapiDataEvent>    _data_events;
    static  YHubDiscoveryCallback   _HubDiscoveryCallback;
    static  u64                 _nextEnum;

    static  map<int,yCalibrationHandler> _calibHandlers;
    static  void        _yapiLogFunctionFwd(const char *log, u32 loglen);
    static  void        _yapiDeviceArrivalCallbackFwd(YDEV_DESCR devdesc);
    static  void        _yapiDeviceRemovalCallbackFwd(YDEV_DESCR devdesc);
    static  void        _yapiDeviceChangeCallbackFwd(YDEV_DESCR devdesc);
    static  void        _yapiDeviceLogCallbackFwd(YDEV_DESCR devdesc, const char* line);
    static  void        _yapiFunctionTimedReportCallbackFwd(YAPI_FUNCTION fundesc, double timestamp, const u8 *bytes, u32 len);
	static  void        _yapiHubDiscoveryCallbackFwd(const char *serial, const char *url);

public:
    static  void        _yapiFunctionUpdateCallbackFwd(YFUN_DESCR fundesc, const char *value);
    static  double      _decimalToDouble(s16 val);
    static  s16         _doubleToDecimal(double val);
    static  yCalibrationHandler _getCalibrationHandler(int calibType);
    static  vector<int> _decodeWords(string s);
    static  vector<int> _decodeFloats(string sdat);
    static  string      _bin2HexStr(const string& data);
    static  string      _hexStr2Bin(const string& str);
    static  string      _flattenJsonStruct(string jsonbuffer);
    static  string      _checkFirmware(const string& serial, const string& rev, const string& path);

    static  int         DefaultCacheValidity;
    static  bool        ExceptionsDisabled;
    static  const string      INVALID_STRING;
    static  const int         INVALID_INT = YAPI_INVALID_INT;
    static  const int         INVALID_UINT = YAPI_INVALID_UINT;
    static  const double      INVALID_DOUBLE;
    static  const s64         INVALID_LONG = YAPI_INVALID_LONG;
    static  bool                _apiInitialized;
    static  yCRITICAL_SECTION   _global_cs;

    // Callback functions
    static  yLogFunction            LogFunction;
    static  yDeviceUpdateCallback   DeviceArrivalCallback;
    static  yDeviceUpdateCallback   DeviceRemovalCallback;
    static  yDeviceUpdateCallback   DeviceChangeCallback;

    static const u32 DETECT_NONE        = 0;
    static const u32 DETECT_USB         = 1;
    static const u32 DETECT_NET         = 2;
    static const u32 RESEND_MISSING_PKT = 4;
    static const u32 DETECT_ALL  = (Y_DETECT_USB | Y_DETECT_NET);

//--- (generated code: YFunction return codes)
    static const int SUCCESS               = 0;       // everything worked all right
    static const int NOT_INITIALIZED       = -1;      // call yInitAPI() first !
    static const int INVALID_ARGUMENT      = -2;      // one of the arguments passed to the function is invalid
    static const int NOT_SUPPORTED         = -3;      // the operation attempted is (currently) not supported
    static const int DEVICE_NOT_FOUND      = -4;      // the requested device is not reachable
    static const int VERSION_MISMATCH      = -5;      // the device firmware is incompatible with this API version
    static const int DEVICE_BUSY           = -6;      // the device is busy with another task and cannot answer
    static const int TIMEOUT               = -7;      // the device took too long to provide an answer
    static const int IO_ERROR              = -8;      // there was an I/O problem while talking to the device
    static const int NO_MORE_DATA          = -9;      // there is no more data to read from
    static const int EXHAUSTED             = -10;     // you have run out of a limited resource, check the documentation
    static const int DOUBLE_ACCES          = -11;     // you have two process that try to access to the same device
    static const int UNAUTHORIZED          = -12;     // unauthorized access to password-protected device
    static const int RTC_NOT_READY         = -13;     // real-time clock has not been initialized (or time was lost)
    static const int FILE_NOT_FOUND        = -14;     // the file is not found
//--- (end of generated code: YFunction return codes)


    /**
     * Returns the version identifier for the Yoctopuce library in use.
     * The version is a string in the form "Major.Minor.Build",
     * for instance "1.01.5535". For languages using an external
     * DLL (for instance C#, VisualBasic or Delphi), the character string
     * includes as well the DLL version, for instance
     * "1.01.5535 (1.01.5439)".
     *
     * If you want to verify in your code that the library version is
     * compatible with the version that you have used during development,
     * verify that the major number is strictly equal and that the minor
     * number is greater or equal. The build number is not relevant
     * with respect to the library compatibility.
     *
     * @return a character string describing the library version.
     */
    static string GetAPIVersion(void);


    /**
     * Initializes the Yoctopuce programming library explicitly.
     * It is not strictly needed to call yInitAPI(), as the library is
     * automatically  initialized when calling yRegisterHub() for the
     * first time.
     *
     * When Y_DETECT_NONE is used as detection mode,
     * you must explicitly use yRegisterHub() to point the API to the
     * VirtualHub on which your devices are connected before trying to access them.
     *
     * @param mode : an integer corresponding to the type of automatic
     *         device detection to use. Possible values are
     *         Y_DETECT_NONE, Y_DETECT_USB, Y_DETECT_NET,
     *         and Y_DETECT_ALL.
     * @param errmsg : a string passed by reference to receive any error message.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    static  YRETCODE    InitAPI(int mode, string& errmsg);

    /**
     * Frees dynamically allocated memory blocks used by the Yoctopuce library.
     * It is generally not required to call this function, unless you
     * want to free all dynamically allocated memory blocks in order to
     * track a memory leak for instance.
     * You should not call any other library function after calling
     * yFreeAPI(), or your program will crash.
     */
    static  void        FreeAPI(void);

    /**
     * Disables the use of exceptions to report runtime errors.
     * When exceptions are disabled, every function returns a specific
     * error value which depends on its type and which is documented in
     * this reference manual.
     */
    static  void        DisableExceptions(void);

    /**
     * Re-enables the use of exceptions for runtime error handling.
     * Be aware than when exceptions are enabled, every function that fails
     * triggers an exception. If the exception is not caught by the user code,
     * it  either fires the debugger or aborts (i.e. crash) the program.
     * On failure, throws an exception or returns a negative error code.
     */
    static  void        EnableExceptions(void);

    /**
     * Registers a log callback function. This callback will be called each time
     * the API have something to say. Quite useful to debug the API.
     *
     * @param logfun : a procedure taking a string parameter, or NULL
     *         to unregister a previously registered  callback.
     */
    static  void        RegisterLogFunction(yLogFunction logfun);

    /**
     * Register a callback function, to be called each time
     * a device is plugged. This callback will be invoked while yUpdateDeviceList
     * is running. You will have to call this function on a regular basis.
     *
     * @param arrivalCallback : a procedure taking a YModule parameter, or NULL
     *         to unregister a previously registered  callback.
     */
    static  void        RegisterDeviceArrivalCallback(yDeviceUpdateCallback arrivalCallback);

    /**
     * Register a callback function, to be called each time
     * a device is unplugged. This callback will be invoked while yUpdateDeviceList
     * is running. You will have to call this function on a regular basis.
     *
     * @param removalCallback : a procedure taking a YModule parameter, or NULL
     *         to unregister a previously registered  callback.
     */
    static  void        RegisterDeviceRemovalCallback(yDeviceUpdateCallback removalCallback);

    /**
     * Register a callback function, to be called each time an Network Hub send
     * an SSDP message. The callback has two string parameter, the first one
     * contain the serial number of the hub and the second contain the URL of the
     * network hub (this URL can be passed to RegisterHub). This callback will be invoked
     * while yUpdateDeviceList is running. You will have to call this function on a regular basis.
     *
     * @param hubDiscoveryCallback : a procedure taking two string parameter, the serial
     *         number and the hub URL. Use NULL to unregister a previously registered  callback.
     */
    static  void        RegisterHubDiscoveryCallback(YHubDiscoveryCallback hubDiscoveryCallback);

    /**
     * Force a hub discovery, if a callback as been registered with yRegisterHubDiscoveryCallback it
     * will be called for each net work hub that will respond to the discovery.
     *
     * @param errmsg : a string passed by reference to receive any error message.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *         On failure, throws an exception or returns a negative error code.
     */
	static  YRETCODE    TriggerHubDiscovery(string& errmsg);


    static  void        RegisterDeviceChangeCallback(yDeviceUpdateCallback changeCallback);

    // Register a new value calibration handler for a given calibration type
    //
    static void         RegisterCalibrationHandler(int calibrationType, yCalibrationHandler calibrationHandler);

    // Standard value calibration handler (n-point linear error correction)
    //
    static double       LinearCalibrationHandler(double rawValue, int calibType, intArr params, floatArr rawValues, floatArr refValues);

    /**
     * Test if the hub is reachable. This method do not register the hub, it only test if the
     * hub is usable. The url parameter follow the same convention as the RegisterHub
     * method. This method is useful to verify the authentication parameters for a hub. It
     * is possible to force this method to return after mstimeout milliseconds.
     *
     * @param url : a string containing either "usb","callback" or the
     *         root URL of the hub to monitor
     * @param mstimeout : the number of millisecond available to test the connection.
     * @param errmsg : a string passed by reference to receive any error message.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure returns a negative error code.
     */
    static  YRETCODE    TestHub(const string& url, int mstimeout, string& errmsg);
    /**
     * Setup the Yoctopuce library to use modules connected on a given machine. The
     * parameter will determine how the API will work. Use the following values:
     *
     * <b>usb</b>: When the usb keyword is used, the API will work with
     * devices connected directly to the USB bus. Some programming languages such a Javascript,
     * PHP, and Java don't provide direct access to USB hardware, so usb will
     * not work with these. In this case, use a VirtualHub or a networked YoctoHub (see below).
     *
     * <b><i>x.x.x.x</i></b> or <b><i>hostname</i></b>: The API will use the devices connected to the
     * host with the given IP address or hostname. That host can be a regular computer
     * running a VirtualHub, or a networked YoctoHub such as YoctoHub-Ethernet or
     * YoctoHub-Wireless. If you want to use the VirtualHub running on you local
     * computer, use the IP address 127.0.0.1.
     *
     * <b>callback</b>: that keyword make the API run in "<i>HTTP Callback</i>" mode.
     * This a special mode allowing to take control of Yoctopuce devices
     * through a NAT filter when using a VirtualHub or a networked YoctoHub. You only
     * need to configure your hub to call your server script on a regular basis.
     * This mode is currently available for PHP and Node.JS only.
     *
     * Be aware that only one application can use direct USB access at a
     * given time on a machine. Multiple access would cause conflicts
     * while trying to access the USB modules. In particular, this means
     * that you must stop the VirtualHub software before starting
     * an application that uses direct USB access. The workaround
     * for this limitation is to setup the library to use the VirtualHub
     * rather than direct USB access.
     *
     * If access control has been activated on the hub, virtual or not, you want to
     * reach, the URL parameter should look like:
     *
     * http://username:password@address:port
     *
     * You can call <i>RegisterHub</i> several times to connect to several machines.
     *
     * @param url : a string containing either "usb","callback" or the
     *         root URL of the hub to monitor
     * @param errmsg : a string passed by reference to receive any error message.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    static  YRETCODE    RegisterHub(const string& url, string& errmsg);

    /**
     * Fault-tolerant alternative to RegisterHub(). This function has the same
     * purpose and same arguments as RegisterHub(), but does not trigger
     * an error when the selected hub is not available at the time of the function call.
     * This makes it possible to register a network hub independently of the current
     * connectivity, and to try to contact it only when a device is actively needed.
     *
     * @param url : a string containing either "usb","callback" or the
     *         root URL of the hub to monitor
     * @param errmsg : a string passed by reference to receive any error message.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    static  YRETCODE    PreregisterHub(const string& url, string& errmsg);

    /**
     * Setup the Yoctopuce library to no more use modules connected on a previously
     * registered machine with RegisterHub.
     *
     * @param url : a string containing either "usb" or the
     *         root URL of the hub to monitor
     */
    static  void        UnregisterHub(const string& url);

    /**
     * Triggers a (re)detection of connected Yoctopuce modules.
     * The library searches the machines or USB ports previously registered using
     * yRegisterHub(), and invokes any user-defined callback function
     * in case a change in the list of connected devices is detected.
     *
     * This function can be called as frequently as desired to refresh the device list
     * and to make the application aware of hot-plug events.
     *
     * @param errmsg : a string passed by reference to receive any error message.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    static  YRETCODE    UpdateDeviceList(string& errmsg);
    /**
     * Maintains the device-to-library communication channel.
     * If your program includes significant loops, you may want to include
     * a call to this function to make sure that the library takes care of
     * the information pushed by the modules on the communication channels.
     * This is not strictly necessary, but it may improve the reactivity
     * of the library for the following commands.
     *
     * This function may signal an error in case there is a communication problem
     * while contacting a module.
     *
     * @param errmsg : a string passed by reference to receive any error message.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    static  YRETCODE    HandleEvents(string& errmsg);
    /**
     * Pauses the execution flow for a specified duration.
     * This function implements a passive waiting loop, meaning that it does not
     * consume CPU cycles significantly. The processor is left available for
     * other threads and processes. During the pause, the library nevertheless
     * reads from time to time information from the Yoctopuce modules by
     * calling yHandleEvents(), in order to stay up-to-date.
     *
     * This function may signal an error in case there is a communication problem
     * while contacting a module.
     *
     * @param ms_duration : an integer corresponding to the duration of the pause,
     *         in milliseconds.
     * @param errmsg : a string passed by reference to receive any error message.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    static  YRETCODE    Sleep(unsigned ms_duration, string& errmsg);
    /**
     * Returns the current value of a monotone millisecond-based time counter.
     * This counter can be used to compute delays in relation with
     * Yoctopuce devices, which also uses the millisecond as timebase.
     *
     * @return a long integer corresponding to the millisecond counter.
     */
    static  u64         GetTickCount(void);
    /**
     * Checks if a given string is valid as logical name for a module or a function.
     * A valid logical name has a maximum of 19 characters, all among
     * A..Z, a..z, 0..9, _, and -.
     * If you try to configure a logical name with an incorrect string,
     * the invalid characters are ignored.
     *
     * @param name : a string containing the name to check.
     *
     * @return true if the name is valid, false otherwise.
     */
    static  bool        CheckLogicalName(const string& name);
};


// Wrappers to yapi low-level API
class YapiWrapper {
public:
    static  u16         getAPIVersion(string& version, string& date);
    static  YDEV_DESCR  getDevice(const string& device_str, string& errmsg);
    static  int         getAllDevices(vector<YDEV_DESCR>& buffer, string& errmsg);
    static  YRETCODE    getDeviceInfo(YDEV_DESCR devdesc, yDeviceSt& infos, string& errmsg);
    static  YFUN_DESCR  getFunction(const string& class_str, const string& function_str, string& errmsg);
    static  int         getFunctionsByClass(const string& class_str, YFUN_DESCR prevfundesc, vector<YFUN_DESCR>& buffer, int maxsize, string& errmsg);
    static  int         getFunctionsByDevice(YDEV_DESCR devdesc, YFUN_DESCR prevfundesc, vector<YFUN_DESCR>& buffer, int maxsize, string& errmsg);
    static  YDEV_DESCR  getDeviceByFunction(YFUN_DESCR fundesc, string& errmsg);
    static  YRETCODE    getFunctionInfo(YFUN_DESCR fundesc, YDEV_DESCR& devdescr, string& serial, string& funcId, string& funcName, string& funcVal, string& errmsg);
    static  YRETCODE    getFunctionInfoEx(YFUN_DESCR fundesc, YDEV_DESCR& devdescr, string& serial, string& funcId, string& baseType, string& funcName, string& funcVal, string& errmsg);
    // pure yapi mapper
    static  YRETCODE    updateDeviceList(bool forceupdate, string& errmsg);
    static  YRETCODE    handleEvents(string& errmsg);
    static  string      ysprintf(const char *fmt, ...);
};


//--- (generated code: YFirmwareUpdate declaration)
/**
 * YFirmwareUpdate Class: Control interface for the firmware update process
 *
 * The YFirmwareUpdate class let you control the firmware update of a Yoctopuce
 * module. This class should not be instantiate directly, instead the method
 * updateFirmware should be called to get an instance of YFirmwareUpdate.
 */
class YOCTO_CLASS_EXPORT YFirmwareUpdate {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YFirmwareUpdate declaration)
protected:
    //--- (generated code: YFirmwareUpdate attributes)
    // Attributes (function value cache)
    string          _serial;
    string          _settings;
    string          _firmwarepath;
    string          _progress_msg;
    int             _progress_c;
    int             _progress;
    int             _restore_step;
    bool            _force;
    //--- (end of generated code: YFirmwareUpdate attributes)


public:
    YFirmwareUpdate(string serialNumber, string path, string settings);
    YFirmwareUpdate(string serialNumber, string path, string settings, bool force);
    YFirmwareUpdate();

    //--- (generated code: YFirmwareUpdate accessors declaration)


    virtual int         _processMore(int newupdate);

    /**
     * Returns a list of all the modules in "firmware update" mode. Only devices
     * connected over USB are listed. For devices connected to a YoctoHub, you
     * must connect yourself to the YoctoHub web interface.
     *
     * @return an array of strings containing the serial numbers of devices in "firmware update" mode.
     */
    static vector<string> GetAllBootLoaders(void);

    /**
     * Test if the byn file is valid for this module. It is possible to pass a directory instead of a file.
     * In that case, this method returns the path of the most recent appropriate byn file. This method will
     * ignore any firmware older than minrelease.
     *
     * @param serial : the serial number of the module to update
     * @param path : the path of a byn file or a directory that contains byn files
     * @param minrelease : a positive integer
     *
     * @return : the path of the byn file to use, or an empty string if no byn files matches the requirement
     *
     * On failure, returns a string that starts with "error:".
     */
    static string       CheckFirmware(string serial,string path,int minrelease);

    /**
     * Returns the progress of the firmware update, on a scale from 0 to 100. When the object is
     * instantiated, the progress is zero. The value is updated during the firmware update process until
     * the value of 100 is reached. The 100 value means that the firmware update was completed
     * successfully. If an error occurs during the firmware update, a negative value is returned, and the
     * error message can be retrieved with get_progressMessage.
     *
     * @return an integer in the range 0 to 100 (percentage of completion)
     *         or a negative error code in case of failure.
     */
    virtual int         get_progress(void);

    /**
     * Returns the last progress message of the firmware update process. If an error occurs during the
     * firmware update process, the error message is returned
     *
     * @return a string  with the latest progress message, or the error message.
     */
    virtual string      get_progressMessage(void);

    /**
     * Starts the firmware update process. This method starts the firmware update process in background. This method
     * returns immediately. You can monitor the progress of the firmware update with the get_progress()
     * and get_progressMessage() methods.
     *
     * @return an integer in the range 0 to 100 (percentage of completion),
     *         or a negative error code in case of failure.
     *
     * On failure returns a negative error code.
     */
    virtual int         startUpdate(void);

#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YFirmwareUpdate accessors declaration)
};



//--- (generated code: YDataStream declaration)
/**
 * YDataStream Class: Unformatted data sequence
 *
 * YDataStream objects represent bare recorded measure sequences,
 * exactly as found within the data logger present on Yoctopuce
 * sensors.
 *
 * In most cases, it is not necessary to use YDataStream objects
 * directly, as the YDataSet objects (returned by the
 * get_recordedData() method from sensors and the
 * get_dataSets() method from the data logger) provide
 * a more convenient interface.
 */
class YOCTO_CLASS_EXPORT YDataStream {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YDataStream declaration)
protected:
    //--- (generated code: YDataStream attributes)
    // Attributes (function value cache)
    YFunction*      _parent;
    int             _runNo;
    s64             _utcStamp;
    int             _nCols;
    int             _nRows;
    int             _duration;
    vector<string>  _columnNames;
    string          _functionId;
    bool            _isClosed;
    bool            _isAvg;
    bool            _isScal;
    bool            _isScal32;
    int             _decimals;
    double          _offset;
    double          _scale;
    int             _samplesPerHour;
    double          _minVal;
    double          _avgVal;
    double          _maxVal;
    double          _decexp;
    int             _caltyp;
    vector<int>     _calpar;
    vector<double>  _calraw;
    vector<double>  _calref;
    vector< vector<double> > _values;
    //--- (end of generated code: YDataStream attributes)

    yCalibrationHandler _calhdl;

public:
    YDataStream(YFunction *parent);
    YDataStream(YFunction *parent, YDataSet &dataset, const vector<int>& encoded);

    virtual ~YDataStream();

    static const double DATA_INVALID;
    static const int    DURATION_INVALID = -1;

    //--- (generated code: YDataStream accessors declaration)


    virtual int         _initFromDataSet(YDataSet* dataset,vector<int> encoded);

    virtual int         _parseStream(string sdata);

    virtual string      _get_url(void);

    virtual int         loadStream(void);

    virtual double      _decodeVal(int w);

    virtual double      _decodeAvg(int dw,int count);

    virtual bool        isClosed(void);

    /**
     * Returns the run index of the data stream. A run can be made of
     * multiple datastreams, for different time intervals.
     *
     * @return an unsigned number corresponding to the run index.
     */
    virtual int         get_runIndex(void);

    /**
     * Returns the relative start time of the data stream, measured in seconds.
     * For recent firmwares, the value is relative to the present time,
     * which means the value is always negative.
     * If the device uses a firmware older than version 13000, value is
     * relative to the start of the time the device was powered on, and
     * is always positive.
     * If you need an absolute UTC timestamp, use get_startTimeUTC().
     *
     * @return an unsigned number corresponding to the number of seconds
     *         between the start of the run and the beginning of this data
     *         stream.
     */
    virtual int         get_startTime(void);

    /**
     * Returns the start time of the data stream, relative to the Jan 1, 1970.
     * If the UTC time was not set in the datalogger at the time of the recording
     * of this data stream, this method returns 0.
     *
     * @return an unsigned number corresponding to the number of seconds
     *         between the Jan 1, 1970 and the beginning of this data
     *         stream (i.e. Unix time representation of the absolute time).
     */
    virtual s64         get_startTimeUTC(void);

    /**
     * Returns the number of milliseconds between two consecutive
     * rows of this data stream. By default, the data logger records one row
     * per second, but the recording frequency can be changed for
     * each device function
     *
     * @return an unsigned number corresponding to a number of milliseconds.
     */
    virtual int         get_dataSamplesIntervalMs(void);

    virtual double      get_dataSamplesInterval(void);

    /**
     * Returns the number of data rows present in this stream.
     *
     * If the device uses a firmware older than version 13000,
     * this method fetches the whole data stream from the device
     * if not yet done, which can cause a little delay.
     *
     * @return an unsigned number corresponding to the number of rows.
     *
     * On failure, throws an exception or returns zero.
     */
    virtual int         get_rowCount(void);

    /**
     * Returns the number of data columns present in this stream.
     * The meaning of the values present in each column can be obtained
     * using the method get_columnNames().
     *
     * If the device uses a firmware older than version 13000,
     * this method fetches the whole data stream from the device
     * if not yet done, which can cause a little delay.
     *
     * @return an unsigned number corresponding to the number of columns.
     *
     * On failure, throws an exception or returns zero.
     */
    virtual int         get_columnCount(void);

    /**
     * Returns the title (or meaning) of each data column present in this stream.
     * In most case, the title of the data column is the hardware identifier
     * of the sensor that produced the data. For streams recorded at a lower
     * recording rate, the dataLogger stores the min, average and max value
     * during each measure interval into three columns with suffixes _min,
     * _avg and _max respectively.
     *
     * If the device uses a firmware older than version 13000,
     * this method fetches the whole data stream from the device
     * if not yet done, which can cause a little delay.
     *
     * @return a list containing as many strings as there are columns in the
     *         data stream.
     *
     * On failure, throws an exception or returns an empty array.
     */
    virtual vector<string> get_columnNames(void);

    /**
     * Returns the smallest measure observed within this stream.
     * If the device uses a firmware older than version 13000,
     * this method will always return Y_DATA_INVALID.
     *
     * @return a floating-point number corresponding to the smallest value,
     *         or Y_DATA_INVALID if the stream is not yet complete (still recording).
     *
     * On failure, throws an exception or returns Y_DATA_INVALID.
     */
    virtual double      get_minValue(void);

    /**
     * Returns the average of all measures observed within this stream.
     * If the device uses a firmware older than version 13000,
     * this method will always return Y_DATA_INVALID.
     *
     * @return a floating-point number corresponding to the average value,
     *         or Y_DATA_INVALID if the stream is not yet complete (still recording).
     *
     * On failure, throws an exception or returns Y_DATA_INVALID.
     */
    virtual double      get_averageValue(void);

    /**
     * Returns the largest measure observed within this stream.
     * If the device uses a firmware older than version 13000,
     * this method will always return Y_DATA_INVALID.
     *
     * @return a floating-point number corresponding to the largest value,
     *         or Y_DATA_INVALID if the stream is not yet complete (still recording).
     *
     * On failure, throws an exception or returns Y_DATA_INVALID.
     */
    virtual double      get_maxValue(void);

    /**
     * Returns the approximate duration of this stream, in seconds.
     *
     * @return the number of seconds covered by this stream.
     *
     * On failure, throws an exception or returns Y_DURATION_INVALID.
     */
    virtual int         get_duration(void);

    /**
     * Returns the whole data set contained in the stream, as a bidimensional
     * table of numbers.
     * The meaning of the values present in each column can be obtained
     * using the method get_columnNames().
     *
     * This method fetches the whole data stream from the device,
     * if not yet done.
     *
     * @return a list containing as many elements as there are rows in the
     *         data stream. Each row itself is a list of floating-point
     *         numbers.
     *
     * On failure, throws an exception or returns an empty array.
     */
    virtual vector< vector<double> > get_dataRows(void);

    /**
     * Returns a single measure from the data stream, specified by its
     * row and column index.
     * The meaning of the values present in each column can be obtained
     * using the method get_columnNames().
     *
     * This method fetches the whole data stream from the device,
     * if not yet done.
     *
     * @param row : row index
     * @param col : column index
     *
     * @return a floating-point number
     *
     * On failure, throws an exception or returns Y_DATA_INVALID.
     */
    virtual double      get_data(int row,int col);

#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YDataStream accessors declaration)
};

//--- (generated code: YMeasure declaration)
/**
 * YMeasure Class: Measured value
 *
 * YMeasure objects are used within the API to represent
 * a value measured at a specified time. These objects are
 * used in particular in conjunction with the YDataSet class.
 */
class YOCTO_CLASS_EXPORT YMeasure {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YMeasure declaration)
protected:
    //--- (generated code: YMeasure attributes)
    // Attributes (function value cache)
    double          _start;
    double          _end;
    double          _minVal;
    double          _avgVal;
    double          _maxVal;
    //--- (end of generated code: YMeasure attributes)


    time_t              _startTime_t;
    time_t              _stopTime_t;
public:
    YMeasure(double start, double end, double minVal, double avgVal, double maxVal);
    YMeasure();

    time_t*        get_startTimeUTC_asTime_t(time_t *time);
    time_t*        get_endTimeUTC_asTime_t(time_t *time);

    //--- (generated code: YMeasure accessors declaration)


    /**
     * Returns the start time of the measure, relative to the Jan 1, 1970 UTC
     * (Unix timestamp). When the recording rate is higher then 1 sample
     * per second, the timestamp may have a fractional part.
     *
     * @return an floating point number corresponding to the number of seconds
     *         between the Jan 1, 1970 UTC and the beginning of this measure.
     */
    virtual double      get_startTimeUTC(void);

    /**
     * Returns the end time of the measure, relative to the Jan 1, 1970 UTC
     * (Unix timestamp). When the recording rate is higher than 1 sample
     * per second, the timestamp may have a fractional part.
     *
     * @return an floating point number corresponding to the number of seconds
     *         between the Jan 1, 1970 UTC and the end of this measure.
     */
    virtual double      get_endTimeUTC(void);

    /**
     * Returns the smallest value observed during the time interval
     * covered by this measure.
     *
     * @return a floating-point number corresponding to the smallest value observed.
     */
    virtual double      get_minValue(void);

    /**
     * Returns the average value observed during the time interval
     * covered by this measure.
     *
     * @return a floating-point number corresponding to the average value observed.
     */
    virtual double      get_averageValue(void);

    /**
     * Returns the largest value observed during the time interval
     * covered by this measure.
     *
     * @return a floating-point number corresponding to the largest value observed.
     */
    virtual double      get_maxValue(void);

#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YMeasure accessors declaration)
};



//--- (generated code: YDataSet declaration)
/**
 * YDataSet Class: Recorded data sequence
 *
 * YDataSet objects make it possible to retrieve a set of recorded measures
 * for a given sensor and a specified time interval. They can be used
 * to load data points with a progress report. When the YDataSet object is
 * instantiated by the get_recordedData()  function, no data is
 * yet loaded from the module. It is only when the loadMore()
 * method is called over and over than data will be effectively loaded
 * from the dataLogger.
 *
 * A preview of available measures is available using the function
 * get_preview() as soon as loadMore() has been called
 * once. Measures themselves are available using function get_measures()
 * when loaded by subsequent calls to loadMore().
 *
 * This class can only be used on devices that use a recent firmware,
 * as YDataSet objects are not supported by firmwares older than version 13000.
 */
class YOCTO_CLASS_EXPORT YDataSet {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YDataSet declaration)
protected:
    //--- (generated code: YDataSet attributes)
    // Attributes (function value cache)
    YFunction*      _parent;
    string          _hardwareId;
    string          _functionId;
    string          _unit;
    s64             _startTime;
    s64             _endTime;
    int             _progress;
    vector<int>     _calib;
    vector<YDataStream*> _streams;
    YMeasure        _summary;
    vector<YMeasure> _preview;
    vector<YMeasure> _measures;
    //--- (end of generated code: YDataSet attributes)

public:
    YDataSet(YFunction *parent, const string& functionId, const string& unit, s64 startTime, s64 endTime);
    YDataSet(YFunction *parent);
    int _parse(const string& json);

    //--- (generated code: YDataSet accessors declaration)


    virtual vector<int> _get_calibration(void);

    virtual int         processMore(int progress,string data);

    virtual vector<YDataStream*> get_privateDataStreams(void);

    /**
     * Returns the unique hardware identifier of the function who performed the measures,
     * in the form SERIAL.FUNCTIONID. The unique hardware identifier is composed of the
     * device serial number and of the hardware identifier of the function
     * (for example THRMCPL1-123456.temperature1)
     *
     * @return a string that uniquely identifies the function (ex: THRMCPL1-123456.temperature1)
     *
     * On failure, throws an exception or returns  Y_HARDWAREID_INVALID.
     */
    virtual string      get_hardwareId(void);

    /**
     * Returns the hardware identifier of the function that performed the measure,
     * without reference to the module. For example temperature1.
     *
     * @return a string that identifies the function (ex: temperature1)
     */
    virtual string      get_functionId(void);

    /**
     * Returns the measuring unit for the measured value.
     *
     * @return a string that represents a physical unit.
     *
     * On failure, throws an exception or returns  Y_UNIT_INVALID.
     */
    virtual string      get_unit(void);

    /**
     * Returns the start time of the dataset, relative to the Jan 1, 1970.
     * When the YDataSet is created, the start time is the value passed
     * in parameter to the get_dataSet() function. After the
     * very first call to loadMore(), the start time is updated
     * to reflect the timestamp of the first measure actually found in the
     * dataLogger within the specified range.
     *
     * @return an unsigned number corresponding to the number of seconds
     *         between the Jan 1, 1970 and the beginning of this data
     *         set (i.e. Unix time representation of the absolute time).
     */
    virtual s64         get_startTimeUTC(void);

    /**
     * Returns the end time of the dataset, relative to the Jan 1, 1970.
     * When the YDataSet is created, the end time is the value passed
     * in parameter to the get_dataSet() function. After the
     * very first call to loadMore(), the end time is updated
     * to reflect the timestamp of the last measure actually found in the
     * dataLogger within the specified range.
     *
     * @return an unsigned number corresponding to the number of seconds
     *         between the Jan 1, 1970 and the end of this data
     *         set (i.e. Unix time representation of the absolute time).
     */
    virtual s64         get_endTimeUTC(void);

    /**
     * Returns the progress of the downloads of the measures from the data logger,
     * on a scale from 0 to 100. When the object is instantiated by get_dataSet,
     * the progress is zero. Each time loadMore() is invoked, the progress
     * is updated, to reach the value 100 only once all measures have been loaded.
     *
     * @return an integer in the range 0 to 100 (percentage of completion).
     */
    virtual int         get_progress(void);

    /**
     * Loads the the next block of measures from the dataLogger, and updates
     * the progress indicator.
     *
     * @return an integer in the range 0 to 100 (percentage of completion),
     *         or a negative error code in case of failure.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         loadMore(void);

    /**
     * Returns an YMeasure object which summarizes the whole
     * DataSet. In includes the following information:
     * - the start of a time interval
     * - the end of a time interval
     * - the minimal value observed during the time interval
     * - the average value observed during the time interval
     * - the maximal value observed during the time interval
     *
     * This summary is available as soon as loadMore() has
     * been called for the first time.
     *
     * @return an YMeasure object
     */
    virtual YMeasure    get_summary(void);

    /**
     * Returns a condensed version of the measures that can
     * retrieved in this YDataSet, as a list of YMeasure
     * objects. Each item includes:
     * - the start of a time interval
     * - the end of a time interval
     * - the minimal value observed during the time interval
     * - the average value observed during the time interval
     * - the maximal value observed during the time interval
     *
     * This preview is available as soon as loadMore() has
     * been called for the first time.
     *
     * @return a table of records, where each record depicts the
     *         measured values during a time interval
     *
     * On failure, throws an exception or returns an empty array.
     */
    virtual vector<YMeasure> get_preview(void);

    /**
     * Returns the detailed set of measures for the time interval corresponding
     * to a given condensed measures previously returned by get_preview().
     * The result is provided as a list of YMeasure objects.
     *
     * @param measure : condensed measure from the list previously returned by
     *         get_preview().
     *
     * @return a table of records, where each record depicts the
     *         measured values during a time interval
     *
     * On failure, throws an exception or returns an empty array.
     */
    virtual vector<YMeasure> get_measuresAt(YMeasure measure);

    /**
     * Returns all measured values currently available for this DataSet,
     * as a list of YMeasure objects. Each item includes:
     * - the start of the measure time interval
     * - the end of the measure time interval
     * - the minimal value observed during the time interval
     * - the average value observed during the time interval
     * - the maximal value observed during the time interval
     *
     * Before calling this method, you should call loadMore()
     * to load data from the device. You may have to call loadMore()
     * several time until all rows are loaded, but you can start
     * looking at available data rows before the load is complete.
     *
     * The oldest measures are always loaded first, and the most
     * recent measures will be loaded last. As a result, timestamps
     * are normally sorted in ascending order within the measure table,
     * unless there was an unexpected adjustment of the datalogger UTC
     * clock.
     *
     * @return a table of records, where each record depicts the
     *         measured value for a given time interval
     *
     * On failure, throws an exception or returns an empty array.
     */
    virtual vector<YMeasure> get_measures(void);

#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YDataSet accessors declaration)
};

//
// YDevice Class (used internally)
//
// This class is used to cache device-level information
//
// In order to regroup multiple function queries on the same physical device,
// this class implements a device-wide API string cache (agnostic of API content).
// This is in addition to the function-specific cache implemented in YFunction.
//

class YDevice;

typedef void (*HTTPRequestCallback)(YDevice *device,void *context,YRETCODE returnval, const string& result,string& errmsg);

class YDevice
{
private:
    // Static device-based JSON string cache
    static vector<YDevice*> _devCache;

    // Device cache entries
    YDEV_DESCR          _devdescr;
    u64                 _cacheStamp; // used only by requestAPI method
    YJSONObject*        _cacheJson;  // used only by requestAPI method
    vector<YFUN_DESCR>  _functions;
    char                _rootdevice[YOCTO_SERIAL_LEN];
    char                *_subpath;
    yCRITICAL_SECTION   _lock;
    // Constructor is private, use getDevice factory method
    YDevice(YDEV_DESCR devdesc);
    ~YDevice();
    YRETCODE   HTTPRequestPrepare(const string& request, string& fullrequest, char *errbuff);
    YRETCODE   HTTPRequest_unsafe(int channel, const string& request, string& buffer, yapiRequestProgressCallback progress_cb, void *progress_ctx, string& errmsg);

public:
    static void ClearCache();
    static YDevice *getDevice(YDEV_DESCR devdescr);
    YRETCODE    HTTPRequestAsync(int channel, const string& request, HTTPRequestCallback callback, void *context, string& errmsg);
    YRETCODE    HTTPRequest(int channel, const string& request, string& buffer, yapiRequestProgressCallback progress_cb, void *progress_ctx, string& errmsg);
    YRETCODE    requestAPI(YJSONObject*& apires, string& errmsg);
    void        clearCache(bool clearSubpath);
    YRETCODE    getFunctions(vector<YFUN_DESCR> **functions, string& errmsg);
    string      getHubSerial(void);

};

//--- (generated code: YFunction declaration)
/**
 * YFunction Class: Common function interface
 *
 * This is the parent class for all public objects representing device functions documented in
 * the high-level programming API. This abstract class does all the real job, but without
 * knowledge of the specific function attributes.
 *
 * Instantiating a child class of YFunction does not cause any communication.
 * The instance simply keeps track of its function identifier, and will dynamically bind
 * to a matching device at the time it is really being used to read or set an attribute.
 * In order to allow true hot-plug replacement of one device by another, the binding stay
 * dynamic through the life of the object.
 *
 * The YFunction class implements a generic high-level cache for the attribute values of
 * the specified function, pre-parsed from the REST API string.
 */
class YOCTO_CLASS_EXPORT YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YFunction declaration)
protected:
    // Protected attributes
    string      _className;
    string      _func;
    YRETCODE    _lastErrorType;
    string      _lastErrorMsg;
    YFUN_DESCR  _fundescr;
    yCRITICAL_SECTION _this_cs;
    std::map<string,YDataStream*> _dataStreams;
    void*                   _userData;
    //--- (generated code: YFunction attributes)
    // Attributes (function value cache)
    string          _logicalName;
    string          _advertisedValue;
    YFunctionValueCallback _valueCallbackFunction;
    u64             _cacheExpiration;
    string          _serial;
    string          _funId;
    string          _hwId;

    friend YFunction *yFindFunction(const string& func);
    friend YFunction *yFirstFunction(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindFunction factory function to instantiate
    YFunction(const string& func);
    //--- (end of generated code: YFunction attributes)
    static  std::map<string,YFunction*> _cache;


    // Method used to retrieve our unique function descriptor (may trigger a hub scan)
    YRETCODE    _getDescriptor(YFUN_DESCR& fundescr, string& errMsg);

    // Method used to retrieve our device object (may trigger a hub scan)
    YRETCODE    _getDevice(YDevice*& dev, string& errMsg);

    // Method used to find the next instance of our function
    YRETCODE    _nextFunction(string &hwId);

    int         _parse(YJSONObject* j);

    string      _escapeAttr(const string& changeval);
    YRETCODE    _buildSetRequest(const string& changeattr, const string  *changeval, string& request, string& errmsg);

    // Method used to change attributes
    YRETCODE    _setAttr(string attrname, string newvalue);
    YRETCODE    _load_unsafe(int msValidity);

    static void _UpdateValueCallbackList(YFunction* func, bool add);
    static void _UpdateTimedReportCallbackList(YFunction* func, bool add);

    // function cache methods
    static YFunction*  _FindFromCache(const string& classname, const string& func);
    static void        _AddToCache(const string& classname, const string& func, YFunction *obj);

public:
    virtual ~YFunction();

    // clear cache of all YFunction object (use only on YAPI::FreeAPI)
    static void _ClearCache(void);

    // Method used to throw exceptions or save error type/message
    void        _throw(YRETCODE errType, string errMsg);

    // Method used to send http request to the device (not the function)
    string      _request(const string& request);
    string      _requestEx(int tcpchan, const string& request, yapiRequestProgressCallback callback, void *context);
    string      _download(const string& url);

    // Method used to upload a file to the device
    YRETCODE    _uploadWithProgress(const string& path, const string& content, yapiRequestProgressCallback callback, void *context);
    YRETCODE    _upload(const string& path, const string& content);

    // Method used to parse a string in JSON data (low-level)
    string      _json_get_key(const string& json, const string& data);
    string      _json_get_string(const string& json);
    vector<string> _json_get_array(const string& json);
    string      _get_json_path(const string& json, const string& path);
    string      _decode_json_string(const string& json);
    string      _parseString(yJsonStateMachine& j);
    int         _parseEx(yJsonStateMachine& j);



    // Method used to cache DataStream objects (new DataLogger)
    YDataStream *_findDataStream(YDataSet& dataset, const string& def);
    // Method used to clear cache of DataStream object (undocumented)
    void _clearDataStreamCache();



    static const YFUN_DESCR FUNCTIONDESCRIPTOR_INVALID = Y_FUNCTIONDESCRIPTOR_INVALID;
    static const string     HARDWAREID_INVALID;
    static const string     FUNCTIONID_INVALID;
    static const string     FRIENDLYNAME_INVALID;



    string get_hubSerial();


    //--- (generated code: YFunction accessors declaration)

    static const string LOGICALNAME_INVALID;
    static const string ADVERTISEDVALUE_INVALID;

    /**
     * Returns the logical name of the function.
     *
     * @return a string corresponding to the logical name of the function
     *
     * On failure, throws an exception or returns Y_LOGICALNAME_INVALID.
     */
    string              get_logicalName(void);

    inline string       logicalName(void)
    { return this->get_logicalName(); }

    /**
     * Changes the logical name of the function. You can use yCheckLogicalName()
     * prior to this call to make sure that your parameter is valid.
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @param newval : a string corresponding to the logical name of the function
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_logicalName(const string& newval);
    inline int      setLogicalName(const string& newval)
    { return this->set_logicalName(newval); }

    /**
     * Returns a short string representing the current state of the function.
     *
     * @return a string corresponding to a short string representing the current state of the function
     *
     * On failure, throws an exception or returns Y_ADVERTISEDVALUE_INVALID.
     */
    string              get_advertisedValue(void);

    inline string       advertisedValue(void)
    { return this->get_advertisedValue(); }

    int             set_advertisedValue(const string& newval);
    inline int      setAdvertisedValue(const string& newval)
    { return this->set_advertisedValue(newval); }

    /**
     * Retrieves a function for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the function is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YFunction.isOnline() to test if the function is
     * indeed online at a given time. In case of ambiguity when looking for
     * a function by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the function
     *
     * @return a YFunction object allowing you to drive the function.
     */
    static YFunction*   FindFunction(string func);

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
    virtual int         registerValueCallback(YFunctionValueCallback callback);

    virtual int         _invokeValueCallback(string value);

    /**
     * Disables the propagation of every new advertised value to the parent hub.
     * You can use this function to save bandwidth and CPU on computers with limited
     * resources, or to prevent unwanted invocations of the HTTP callback.
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         muteValueCallbacks(void);

    /**
     * Re-enables the propagation of every new advertised value to the parent hub.
     * This function reverts the effect of a previous call to muteValueCallbacks().
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         unmuteValueCallbacks(void);

    /**
     * Returns the current value of a single function attribute, as a text string, as quickly as
     * possible but without using the cached value.
     *
     * @param attrName : the name of the requested attribute
     *
     * @return a string with the value of the the attribute
     *
     * On failure, throws an exception or returns an empty string.
     */
    virtual string      loadAttribute(string attrName);

    virtual int         _parserHelper(void);


    inline static YFunction* Find(string func)
    { return YFunction::FindFunction(func); }

    /**
     * comment from .yc definition
     */
           YFunction       *nextFunction(void);
    inline YFunction       *next(void)
    { return this->nextFunction();}

    /**
     * comment from .yc definition
     */
           static YFunction* FirstFunction(void);
    inline static YFunction* First(void)
    { return YFunction::FirstFunction();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YFunction accessors declaration)



    /**
     * Returns a short text that describes unambiguously the instance of the function in the form
     * TYPE(NAME)=SERIAL&#46;FUNCTIONID.
     * More precisely,
     * TYPE       is the type of the function,
     * NAME       it the name used for the first access to the function,
     * SERIAL     is the serial number of the module if the module is connected or "unresolved", and
     * FUNCTIONID is  the hardware identifier of the function if the module is connected.
     * For example, this method returns Relay(MyCustomName.relay1)=RELAYLO1-123456.relay1 if the
     * module is already connected or Relay(BadCustomeName.relay1)=unresolved if the module has
     * not yet been connected. This method does not trigger any USB or TCP transaction and can therefore be used in
     * a debugger.
     *
     * @return a string that describes the function
     *         (ex: Relay(MyCustomName.relay1)=RELAYLO1-123456.relay1)
     */
    string      describe(void);

    /**
     * Returns a global identifier of the function in the format MODULE_NAME&#46;FUNCTION_NAME.
     * The returned string uses the logical names of the module and of the function if they are defined,
     * otherwise the serial number of the module and the hardware identifier of the function
     * (for example: MyCustomName.relay1)
     *
     * @return a string that uniquely identifies the function using logical names
     *         (ex: MyCustomName.relay1)
     *
     * On failure, throws an exception or returns  Y_FRIENDLYNAME_INVALID.
     */
     virtual string      get_friendlyName(void);

    /**
     * Returns the unique hardware identifier of the function in the form SERIAL.FUNCTIONID.
     * The unique hardware identifier is composed of the device serial
     * number and of the hardware identifier of the function (for example RELAYLO1-123456.relay1).
     *
     * @return a string that uniquely identifies the function (ex: RELAYLO1-123456.relay1)
     *
     * On failure, throws an exception or returns  Y_HARDWAREID_INVALID.
     */
    string      get_hardwareId(void);

    /**
     * Returns the hardware identifier of the function, without reference to the module. For example
     * relay1
     *
     * @return a string that identifies the function (ex: relay1)
     *
     * On failure, throws an exception or returns  Y_FUNCTIONID_INVALID.
     */
    string      get_functionId(void);


    /**
     * Returns the numerical error code of the latest error with the function.
     * This method is mostly useful when using the Yoctopuce library with
     * exceptions disabled.
     *
     * @return a number corresponding to the code of the latest error that occurred while
     *         using the function object
     */
           YRETCODE    get_errorType(void);
    inline YRETCODE    errorType(void)
    {return this->get_errorType();}
    inline YRETCODE    errType(void)
    {return this->get_errorType();}

    /**
     * Returns the error message of the latest error with the function.
     * This method is mostly useful when using the Yoctopuce library with
     * exceptions disabled.
     *
     * @return a string corresponding to the latest error message that occured while
     *         using the function object
     */
           string      get_errorMessage(void);
    inline string      errorMessage(void)
    {return this->get_errorMessage();}
    inline string      errMessage(void)
    {return this->get_errorMessage();}

    /**
     * Checks if the function is currently reachable, without raising any error.
     * If there is a cached value for the function in cache, that has not yet
     * expired, the device is considered reachable.
     * No exception is raised if there is an error while trying to contact the
     * device hosting the function.
     *
     * @return true if the function can be reached, and false otherwise
     */
    bool        isOnline(void);

    /**
     * Preloads the function cache with a specified validity duration.
     * By default, whenever accessing a device, all function attributes
     * are kept in cache for the standard duration (5 ms). This method can be
     * used to temporarily mark the cache as valid for a longer period, in order
     * to reduce network traffic for instance.
     *
     * @param msValidity : an integer corresponding to the validity attributed to the
     *         loaded function parameters, in milliseconds
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    YRETCODE    load(int msValidity);

    /**
     * Invalidates the cache. Invalidates the cache of the function attributes. Forces the
     * next call to get_xxx() or loadxxx() to use values that come from the device.
     *
     * @noreturn
     */
    void    clearCache();

    /**
     * Gets the YModule object for the device on which the function is located.
     * If the function cannot be located on any module, the returned instance of
     * YModule is not shown as on-line.
     *
     * @return an instance of YModule
     */
           YModule     *get_module(void);
    inline YModule     *module(void)
    {return this->get_module();}

    /**
     * Returns a unique identifier of type YFUN_DESCR corresponding to the function.
     * This identifier can be used to test if two instances of YFunction reference the same
     * physical function on the same physical device.
     *
     * @return an identifier of type YFUN_DESCR.
     *
     * If the function has never been contacted, the returned value is Y_FUNCTIONDESCRIPTOR_INVALID.
     */
           YFUN_DESCR     get_functionDescriptor(void);
    inline YFUN_DESCR     functionDescriptor(void)
    {return this->get_functionDescriptor();}

    /**
     * Returns the value of the userData attribute, as previously stored using method
     * set_userData.
     * This attribute is never touched directly by the API, and is at disposal of the caller to
     * store a context.
     *
     * @return the object stored previously by the caller.
     */
           void        *get_userData(void);
    inline void        *userData(void)
    {return this->get_userData();}

    /**
     * Stores a user context provided as argument in the userData attribute of the function.
     * This attribute is never touched by the API, and is at disposal of the caller to store a context.
     *
     * @param data : any kind of object to be stored
     * @noreturn
     */
           void        set_userData(void* data);
    inline void        setUserData(void* data)
    { this->set_userData(data);}

};


typedef void(*YModuleLogCallback)(YModule *module, const string& log);

//--- (generated code: YModule declaration)
/**
 * YModule Class: Module control interface
 *
 * This interface is identical for all Yoctopuce USB modules.
 * It can be used to control the module global parameters, and
 * to enumerate the functions provided by each module.
 */
class YOCTO_CLASS_EXPORT YModule: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YModule declaration)
protected:
    //--- (generated code: YModule attributes)
    // Attributes (function value cache)
    string          _productName;
    string          _serialNumber;
    int             _productId;
    int             _productRelease;
    string          _firmwareRelease;
    Y_PERSISTENTSETTINGS_enum _persistentSettings;
    int             _luminosity;
    Y_BEACON_enum   _beacon;
    s64             _upTime;
    int             _usbCurrent;
    int             _rebootCountdown;
    int             _userVar;
    YModuleValueCallback _valueCallbackModule;
    YModuleLogCallback _logCallback;

    friend YModule *yFindModule(const string& func);
    friend YModule *yFirstModule(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindModule factory function to instantiate
    YModule(const string& func);
    //--- (end of generated code: YModule attributes)
    //--- (generated code: YModule initialization)
    //--- (end of generated code: YModule initialization)

    // Method used to retrieve details of the nth function of our device
    YRETCODE        _getFunction(int idx, string& serial, string& funcId, string& baseType, string& funcName, string& funcVal, string& errMsg);

public:
    ~YModule();

    /**
     * Returns a global identifier of the function in the format MODULE_NAME&#46;FUNCTION_NAME.
     * The returned string uses the logical names of the module and of the function if they are defined,
     * otherwise the serial number of the module and the hardware identifier of the function
     * (for example: MyCustomName.relay1)
     *
     * @return a string that uniquely identifies the function using logical names
     *         (ex: MyCustomName.relay1)
     *
     * On failure, throws an exception or returns  Y_FRIENDLYNAME_INVALID.
     */
    virtual string      get_friendlyName(void);


    /**
     * Returns the number of functions (beside the "module" interface) available on the module.
     *
     * @return the number of functions on the module
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             functionCount();

    /**
     * Retrieves the hardware identifier of the <i>n</i>th function on the module.
     *
     * @param functionIndex : the index of the function for which the information is desired, starting at
     * 0 for the first function.
     *
     * @return a string corresponding to the unambiguous hardware identifier of the requested module function
     *
     * On failure, throws an exception or returns an empty string.
     */
    string          functionId(int functionIndex);

    /**
     * Retrieves the logical name of the <i>n</i>th function on the module.
     *
     * @param functionIndex : the index of the function for which the information is desired, starting at
     * 0 for the first function.
     *
     * @return a string corresponding to the logical name of the requested module function
     *
     * On failure, throws an exception or returns an empty string.
     */
    string          functionName(int functionIndex);

    /**
     * Retrieves the advertised value of the <i>n</i>th function on the module.
     *
     * @param functionIndex : the index of the function for which the information is desired, starting at
     * 0 for the first function.
     *
     * @return a short string (up to 6 characters) corresponding to the advertised value of the requested
     * module function
     *
     * On failure, throws an exception or returns an empty string.
     */
    string          functionValue(int functionIndex);

    /**
     * Retrieves the type of the <i>n</i>th function on the module.
     *
     * @param functionIndex : the index of the function for which the information is desired, starting at
     * 0 for the first function.
     *
     * @return a string corresponding to the type of the function
     *
     * On failure, throws an exception or returns an empty string.
     */
    string          functionType(int functionIndex);


    /**
     * Retrieves the base type of the <i>n</i>th function on the module.
     * For instance, the base type of all measuring functions is "Sensor".
     *
     * @param functionIndex : the index of the function for which the information is desired, starting at
     * 0 for the first function.
     *
     * @return a string corresponding to the base type of the function
     *
     * On failure, throws an exception or returns an empty string.
     */
    string          functionBaseType(int functionIndex);


    void            setImmutableAttributes(yDeviceSt *infos);

    /**
     * Registers a device log callback function. This callback will be called each time
     * that a module sends a new log message. Mostly useful to debug a Yoctopuce module.
     *
     * @param callback : the callback function to call, or a NULL pointer. The callback function should take two
     *         arguments: the module object that emitted the log message, and the character string containing the log.
     * @noreturn
     */
    void            registerLogCallback(YModuleLogCallback callback);


    YModuleLogCallback get_logCallback();



    //--- (generated code: YModule accessors declaration)

    static const string PRODUCTNAME_INVALID;
    static const string SERIALNUMBER_INVALID;
    static const int PRODUCTID_INVALID = YAPI_INVALID_UINT;
    static const int PRODUCTRELEASE_INVALID = YAPI_INVALID_UINT;
    static const string FIRMWARERELEASE_INVALID;
    static const Y_PERSISTENTSETTINGS_enum PERSISTENTSETTINGS_LOADED = Y_PERSISTENTSETTINGS_LOADED;
    static const Y_PERSISTENTSETTINGS_enum PERSISTENTSETTINGS_SAVED = Y_PERSISTENTSETTINGS_SAVED;
    static const Y_PERSISTENTSETTINGS_enum PERSISTENTSETTINGS_MODIFIED = Y_PERSISTENTSETTINGS_MODIFIED;
    static const Y_PERSISTENTSETTINGS_enum PERSISTENTSETTINGS_INVALID = Y_PERSISTENTSETTINGS_INVALID;
    static const int LUMINOSITY_INVALID = YAPI_INVALID_UINT;
    static const Y_BEACON_enum BEACON_OFF = Y_BEACON_OFF;
    static const Y_BEACON_enum BEACON_ON = Y_BEACON_ON;
    static const Y_BEACON_enum BEACON_INVALID = Y_BEACON_INVALID;
    static const s64 UPTIME_INVALID = YAPI_INVALID_LONG;
    static const int USBCURRENT_INVALID = YAPI_INVALID_UINT;
    static const int REBOOTCOUNTDOWN_INVALID = YAPI_INVALID_INT;
    static const int USERVAR_INVALID = YAPI_INVALID_INT;

    /**
     * Returns the commercial name of the module, as set by the factory.
     *
     * @return a string corresponding to the commercial name of the module, as set by the factory
     *
     * On failure, throws an exception or returns Y_PRODUCTNAME_INVALID.
     */
    string              get_productName(void);

    inline string       productName(void)
    { return this->get_productName(); }

    /**
     * Returns the serial number of the module, as set by the factory.
     *
     * @return a string corresponding to the serial number of the module, as set by the factory
     *
     * On failure, throws an exception or returns Y_SERIALNUMBER_INVALID.
     */
    string              get_serialNumber(void);

    inline string       serialNumber(void)
    { return this->get_serialNumber(); }

    /**
     * Returns the USB device identifier of the module.
     *
     * @return an integer corresponding to the USB device identifier of the module
     *
     * On failure, throws an exception or returns Y_PRODUCTID_INVALID.
     */
    int                 get_productId(void);

    inline int          productId(void)
    { return this->get_productId(); }

    /**
     * Returns the hardware release version of the module.
     *
     * @return an integer corresponding to the hardware release version of the module
     *
     * On failure, throws an exception or returns Y_PRODUCTRELEASE_INVALID.
     */
    int                 get_productRelease(void);

    inline int          productRelease(void)
    { return this->get_productRelease(); }

    /**
     * Returns the version of the firmware embedded in the module.
     *
     * @return a string corresponding to the version of the firmware embedded in the module
     *
     * On failure, throws an exception or returns Y_FIRMWARERELEASE_INVALID.
     */
    string              get_firmwareRelease(void);

    inline string       firmwareRelease(void)
    { return this->get_firmwareRelease(); }

    /**
     * Returns the current state of persistent module settings.
     *
     * @return a value among Y_PERSISTENTSETTINGS_LOADED, Y_PERSISTENTSETTINGS_SAVED and
     * Y_PERSISTENTSETTINGS_MODIFIED corresponding to the current state of persistent module settings
     *
     * On failure, throws an exception or returns Y_PERSISTENTSETTINGS_INVALID.
     */
    Y_PERSISTENTSETTINGS_enum get_persistentSettings(void);

    inline Y_PERSISTENTSETTINGS_enum persistentSettings(void)
    { return this->get_persistentSettings(); }

    int             set_persistentSettings(Y_PERSISTENTSETTINGS_enum newval);
    inline int      setPersistentSettings(Y_PERSISTENTSETTINGS_enum newval)
    { return this->set_persistentSettings(newval); }

    /**
     * Returns the luminosity of the  module informative leds (from 0 to 100).
     *
     * @return an integer corresponding to the luminosity of the  module informative leds (from 0 to 100)
     *
     * On failure, throws an exception or returns Y_LUMINOSITY_INVALID.
     */
    int                 get_luminosity(void);

    inline int          luminosity(void)
    { return this->get_luminosity(); }

    /**
     * Changes the luminosity of the module informative leds. The parameter is a
     * value between 0 and 100.
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @param newval : an integer corresponding to the luminosity of the module informative leds
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_luminosity(int newval);
    inline int      setLuminosity(int newval)
    { return this->set_luminosity(newval); }

    /**
     * Returns the state of the localization beacon.
     *
     * @return either Y_BEACON_OFF or Y_BEACON_ON, according to the state of the localization beacon
     *
     * On failure, throws an exception or returns Y_BEACON_INVALID.
     */
    Y_BEACON_enum       get_beacon(void);

    inline Y_BEACON_enum beacon(void)
    { return this->get_beacon(); }

    /**
     * Turns on or off the module localization beacon.
     *
     * @param newval : either Y_BEACON_OFF or Y_BEACON_ON
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_beacon(Y_BEACON_enum newval);
    inline int      setBeacon(Y_BEACON_enum newval)
    { return this->set_beacon(newval); }

    /**
     * Returns the number of milliseconds spent since the module was powered on.
     *
     * @return an integer corresponding to the number of milliseconds spent since the module was powered on
     *
     * On failure, throws an exception or returns Y_UPTIME_INVALID.
     */
    s64                 get_upTime(void);

    inline s64          upTime(void)
    { return this->get_upTime(); }

    /**
     * Returns the current consumed by the module on the USB bus, in milli-amps.
     *
     * @return an integer corresponding to the current consumed by the module on the USB bus, in milli-amps
     *
     * On failure, throws an exception or returns Y_USBCURRENT_INVALID.
     */
    int                 get_usbCurrent(void);

    inline int          usbCurrent(void)
    { return this->get_usbCurrent(); }

    /**
     * Returns the remaining number of seconds before the module restarts, or zero when no
     * reboot has been scheduled.
     *
     * @return an integer corresponding to the remaining number of seconds before the module restarts, or zero when no
     *         reboot has been scheduled
     *
     * On failure, throws an exception or returns Y_REBOOTCOUNTDOWN_INVALID.
     */
    int                 get_rebootCountdown(void);

    inline int          rebootCountdown(void)
    { return this->get_rebootCountdown(); }

    int             set_rebootCountdown(int newval);
    inline int      setRebootCountdown(int newval)
    { return this->set_rebootCountdown(newval); }

    /**
     * Returns the value previously stored in this attribute.
     * On startup and after a device reboot, the value is always reset to zero.
     *
     * @return an integer corresponding to the value previously stored in this attribute
     *
     * On failure, throws an exception or returns Y_USERVAR_INVALID.
     */
    int                 get_userVar(void);

    inline int          userVar(void)
    { return this->get_userVar(); }

    /**
     * Stores a 32 bit value in the device RAM. This attribute is at programmer disposal,
     * should he need to store a state variable.
     * On startup and after a device reboot, the value is always reset to zero.
     *
     * @param newval : an integer
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_userVar(int newval);
    inline int      setUserVar(int newval)
    { return this->set_userVar(newval); }

    /**
     * Allows you to find a module from its serial number or from its logical name.
     *
     * This function does not require that the module is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YModule.isOnline() to test if the module is
     * indeed online at a given time. In case of ambiguity when looking for
     * a module by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string containing either the serial number or
     *         the logical name of the desired module
     *
     * @return a YModule object allowing you to drive the module
     *         or get additional information on the module.
     */
    static YModule*     FindModule(string func);

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
    virtual int         registerValueCallback(YModuleValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Saves current settings in the nonvolatile memory of the module.
     * Warning: the number of allowed save operations during a module life is
     * limited (about 100000 cycles). Do not call this function within a loop.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         saveToFlash(void);

    /**
     * Reloads the settings stored in the nonvolatile memory, as
     * when the module is powered on.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         revertFromFlash(void);

    /**
     * Schedules a simple module reboot after the given number of seconds.
     *
     * @param secBeforeReboot : number of seconds before rebooting
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         reboot(int secBeforeReboot);

    /**
     * Schedules a module reboot into special firmware update mode.
     *
     * @param secBeforeReboot : number of seconds before rebooting
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         triggerFirmwareUpdate(int secBeforeReboot);

    /**
     * Tests whether the byn file is valid for this module. This method is useful to test if the module
     * needs to be updated.
     * It is possible to pass a directory as argument instead of a file. In this case, this method returns
     * the path of the most recent
     * appropriate .byn file. If the parameter onlynew is true, the function discards firmwares that are older or
     * equal to the installed firmware.
     *
     * @param path : the path of a byn file or a directory that contains byn files
     * @param onlynew : returns only files that are strictly newer
     *
     * @return the path of the byn file to use or a empty string if no byn files matches the requirement
     *
     * On failure, throws an exception or returns a string that start with "error:".
     */
    virtual string      checkFirmware(string path,bool onlynew);

    /**
     * Prepares a firmware update of the module. This method returns a YFirmwareUpdate object which
     * handles the firmware update process.
     *
     * @param path : the path of the .byn file to use.
     * @param force : true to force the firmware update even if some prerequisites appear not to be met
     *
     * @return a YFirmwareUpdate object or NULL on error.
     */
    virtual YFirmwareUpdate updateFirmwareEx(string path,bool force);

    /**
     * Prepares a firmware update of the module. This method returns a YFirmwareUpdate object which
     * handles the firmware update process.
     *
     * @param path : the path of the .byn file to use.
     *
     * @return a YFirmwareUpdate object or NULL on error.
     */
    virtual YFirmwareUpdate updateFirmware(string path);

    /**
     * Returns all the settings and uploaded files of the module. Useful to backup all the
     * logical names, calibrations parameters, and uploaded files of a device.
     *
     * @return a binary buffer with all the settings.
     *
     * On failure, throws an exception or returns an binary object of size 0.
     */
    virtual string      get_allSettings(void);

    virtual int         loadThermistorExtra(string funcId,string jsonExtra);

    virtual int         set_extraSettings(string jsonExtra);

    /**
     * Restores all the settings and uploaded files to the module.
     * This method is useful to restore all the logical names and calibrations parameters,
     * uploaded files etc. of a device from a backup.
     * Remember to call the saveToFlash() method of the module if the
     * modifications must be kept.
     *
     * @param settings : a binary buffer with all the settings.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_allSettingsAndFiles(string settings);

    /**
     * Tests if the device includes a specific function. This method takes a function identifier
     * and returns a boolean.
     *
     * @param funcId : the requested function identifier
     *
     * @return true if the device has the function identifier
     */
    virtual bool        hasFunction(string funcId);

    /**
     * Retrieve all hardware identifier that match the type passed in argument.
     *
     * @param funType : The type of function (Relay, LightSensor, Voltage,...)
     *
     * @return an array of strings.
     */
    virtual vector<string> get_functionIds(string funType);

    virtual string      _flattenJsonStruct(string jsoncomplex);

    virtual int         calibVersion(string cparams);

    virtual int         calibScale(string unit_name,string sensorType);

    virtual int         calibOffset(string unit_name);

    virtual string      calibConvert(string param,string currentFuncValue,string unit_name,string sensorType);

    /**
     * Restores all the settings of the device. Useful to restore all the logical names and calibrations parameters
     * of a module from a backup.Remember to call the saveToFlash() method of the module if the
     * modifications must be kept.
     *
     * @param settings : a binary buffer with all the settings.
     *
     * @return YAPI_SUCCESS when the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         set_allSettings(string settings);

    /**
     * Downloads the specified built-in file and returns a binary buffer with its content.
     *
     * @param pathname : name of the new file to load
     *
     * @return a binary buffer with the file content
     *
     * On failure, throws an exception or returns  YAPI_INVALID_STRING.
     */
    virtual string      download(string pathname);

    /**
     * Returns the icon of the module. The icon is a PNG image and does not
     * exceeds 1536 bytes.
     *
     * @return a binary buffer with module icon, in png format.
     *         On failure, throws an exception or returns  YAPI_INVALID_STRING.
     */
    virtual string      get_icon2d(void);

    /**
     * Returns a string with last logs of the module. This method return only
     * logs that are still in the module.
     *
     * @return a string with last logs of the module.
     *         On failure, throws an exception or returns  YAPI_INVALID_STRING.
     */
    virtual string      get_lastLogs(void);

    /**
     * Adds a text message to the device logs. This function is useful in
     * particular to trace the execution of HTTP callbacks. If a newline
     * is desired after the message, it must be included in the string.
     *
     * @param text : the string to append to the logs.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         log(string text);

    /**
     * Returns a list of all the modules that are plugged into the current module.
     * This method only makes sense when called for a YoctoHub/VirtualHub.
     * Otherwise, an empty array will be returned.
     *
     * @return an array of strings containing the sub modules.
     */
    virtual vector<string> get_subDevices(void);

    /**
     * Returns the serial number of the YoctoHub on which this module is connected.
     * If the module is connected by USB, or if the module is the root YoctoHub, an
     * empty string is returned.
     *
     * @return a string with the serial number of the YoctoHub or an empty string
     */
    virtual string      get_parentHub(void);

    /**
     * Returns the URL used to access the module. If the module is connected by USB, the
     * string 'usb' is returned.
     *
     * @return a string with the URL of the module.
     */
    virtual string      get_url(void);


    inline static YModule* Find(string func)
    { return YModule::FindModule(func); }

    /**
     * Continues the module enumeration started using yFirstModule().
     *
     * @return a pointer to a YModule object, corresponding to
     *         the next module found, or a NULL pointer
     *         if there are no more modules to enumerate.
     */
           YModule         *nextModule(void);
    inline YModule         *next(void)
    { return this->nextModule();}

    /**
     * Starts the enumeration of modules currently accessible.
     * Use the method YModule.nextModule() to iterate on the
     * next modules.
     *
     * @return a pointer to a YModule object, corresponding to
     *         the first module currently online, or a NULL pointer
     *         if there are none.
     */
           static YModule* FirstModule(void);
    inline static YModule* First(void)
    { return YModule::FirstModule();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YModule accessors declaration)
};




//--- (generated code: YSensor declaration)
/**
 * YSensor Class: Sensor function interface
 *
 * The YSensor class is the parent class for all Yoctopuce sensors. It can be
 * used to read the current value and unit of any sensor, read the min/max
 * value, configure autonomous recording frequency and access recorded data.
 * It also provide a function to register a callback invoked each time the
 * observed value changes, or at a predefined interval. Using this class rather
 * than a specific subclass makes it possible to create generic applications
 * that work with any Yoctopuce sensor, even those that do not yet exist.
 * Note: The YAnButton class is the only analog input which does not inherit
 * from YSensor.
 */
class YOCTO_CLASS_EXPORT YSensor: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YSensor declaration)
protected:
    //--- (generated code: YSensor attributes)
    // Attributes (function value cache)
    string          _unit;
    double          _currentValue;
    double          _lowestValue;
    double          _highestValue;
    double          _currentRawValue;
    string          _logFrequency;
    string          _reportFrequency;
    Y_ADVMODE_enum  _advMode;
    string          _calibrationParam;
    double          _resolution;
    int             _sensorState;
    YSensorValueCallback _valueCallbackSensor;
    YSensorTimedReportCallback _timedReportCallbackSensor;
    double          _prevTimedReport;
    double          _iresol;
    double          _offset;
    double          _scale;
    double          _decexp;
    bool            _isScal;
    bool            _isScal32;
    int             _caltyp;
    vector<int>     _calpar;
    vector<double>  _calraw;
    vector<double>  _calref;
    yCalibrationHandler _calhdl;

    friend YSensor *yFindSensor(const string& func);
    friend YSensor *yFirstSensor(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindSensor factory function to instantiate
    YSensor(const string& func);
    //--- (end of generated code: YSensor attributes)

    //--- (generated code: YSensor initialization)
    //--- (end of generated code: YSensor initialization)

public:
    ~YSensor();
    //--- (generated code: YSensor accessors declaration)

    static const string UNIT_INVALID;
    static const double CURRENTVALUE_INVALID;
    static const double LOWESTVALUE_INVALID;
    static const double HIGHESTVALUE_INVALID;
    static const double CURRENTRAWVALUE_INVALID;
    static const string LOGFREQUENCY_INVALID;
    static const string REPORTFREQUENCY_INVALID;
    static const Y_ADVMODE_enum ADVMODE_IMMEDIATE = Y_ADVMODE_IMMEDIATE;
    static const Y_ADVMODE_enum ADVMODE_PERIOD_AVG = Y_ADVMODE_PERIOD_AVG;
    static const Y_ADVMODE_enum ADVMODE_PERIOD_MIN = Y_ADVMODE_PERIOD_MIN;
    static const Y_ADVMODE_enum ADVMODE_PERIOD_MAX = Y_ADVMODE_PERIOD_MAX;
    static const Y_ADVMODE_enum ADVMODE_INVALID = Y_ADVMODE_INVALID;
    static const string CALIBRATIONPARAM_INVALID;
    static const double RESOLUTION_INVALID;
    static const int SENSORSTATE_INVALID = YAPI_INVALID_INT;

    /**
     * Returns the measuring unit for the measure.
     *
     * @return a string corresponding to the measuring unit for the measure
     *
     * On failure, throws an exception or returns Y_UNIT_INVALID.
     */
    string              get_unit(void);

    inline string       unit(void)
    { return this->get_unit(); }

    /**
     * Returns the current value of the measure, in the specified unit, as a floating point number.
     *
     * @return a floating point number corresponding to the current value of the measure, in the specified
     * unit, as a floating point number
     *
     * On failure, throws an exception or returns Y_CURRENTVALUE_INVALID.
     */
    double              get_currentValue(void);

    inline double       currentValue(void)
    { return this->get_currentValue(); }

    /**
     * Changes the recorded minimal value observed. Can be used to reset the value returned
     * by get_lowestValue().
     *
     * @param newval : a floating point number corresponding to the recorded minimal value observed
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_lowestValue(double newval);
    inline int      setLowestValue(double newval)
    { return this->set_lowestValue(newval); }

    /**
     * Returns the minimal value observed for the measure since the device was started.
     * Can be reset to an arbitrary value thanks to set_lowestValue().
     *
     * @return a floating point number corresponding to the minimal value observed for the measure since
     * the device was started
     *
     * On failure, throws an exception or returns Y_LOWESTVALUE_INVALID.
     */
    double              get_lowestValue(void);

    inline double       lowestValue(void)
    { return this->get_lowestValue(); }

    /**
     * Changes the recorded maximal value observed. Can be used to reset the value returned
     * by get_lowestValue().
     *
     * @param newval : a floating point number corresponding to the recorded maximal value observed
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_highestValue(double newval);
    inline int      setHighestValue(double newval)
    { return this->set_highestValue(newval); }

    /**
     * Returns the maximal value observed for the measure since the device was started.
     * Can be reset to an arbitrary value thanks to set_highestValue().
     *
     * @return a floating point number corresponding to the maximal value observed for the measure since
     * the device was started
     *
     * On failure, throws an exception or returns Y_HIGHESTVALUE_INVALID.
     */
    double              get_highestValue(void);

    inline double       highestValue(void)
    { return this->get_highestValue(); }

    /**
     * Returns the uncalibrated, unrounded raw value returned by the sensor, in the specified unit, as a
     * floating point number.
     *
     * @return a floating point number corresponding to the uncalibrated, unrounded raw value returned by
     * the sensor, in the specified unit, as a floating point number
     *
     * On failure, throws an exception or returns Y_CURRENTRAWVALUE_INVALID.
     */
    double              get_currentRawValue(void);

    inline double       currentRawValue(void)
    { return this->get_currentRawValue(); }

    /**
     * Returns the datalogger recording frequency for this function, or "OFF"
     * when measures are not stored in the data logger flash memory.
     *
     * @return a string corresponding to the datalogger recording frequency for this function, or "OFF"
     *         when measures are not stored in the data logger flash memory
     *
     * On failure, throws an exception or returns Y_LOGFREQUENCY_INVALID.
     */
    string              get_logFrequency(void);

    inline string       logFrequency(void)
    { return this->get_logFrequency(); }

    /**
     * Changes the datalogger recording frequency for this function.
     * The frequency can be specified as samples per second,
     * as sample per minute (for instance "15/m") or in samples per
     * hour (eg. "4/h"). To disable recording for this function, use
     * the value "OFF".
     *
     * @param newval : a string corresponding to the datalogger recording frequency for this function
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_logFrequency(const string& newval);
    inline int      setLogFrequency(const string& newval)
    { return this->set_logFrequency(newval); }

    /**
     * Returns the timed value notification frequency, or "OFF" if timed
     * value notifications are disabled for this function.
     *
     * @return a string corresponding to the timed value notification frequency, or "OFF" if timed
     *         value notifications are disabled for this function
     *
     * On failure, throws an exception or returns Y_REPORTFREQUENCY_INVALID.
     */
    string              get_reportFrequency(void);

    inline string       reportFrequency(void)
    { return this->get_reportFrequency(); }

    /**
     * Changes the timed value notification frequency for this function.
     * The frequency can be specified as samples per second,
     * as sample per minute (for instance "15/m") or in samples per
     * hour (eg. "4/h"). To disable timed value notifications for this
     * function, use the value "OFF".
     *
     * @param newval : a string corresponding to the timed value notification frequency for this function
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_reportFrequency(const string& newval);
    inline int      setReportFrequency(const string& newval)
    { return this->set_reportFrequency(newval); }

    /**
     * Returns the measuring mode used for the advertised value pushed to the parent hub.
     *
     * @return a value among Y_ADVMODE_IMMEDIATE, Y_ADVMODE_PERIOD_AVG, Y_ADVMODE_PERIOD_MIN and
     * Y_ADVMODE_PERIOD_MAX corresponding to the measuring mode used for the advertised value pushed to the parent hub
     *
     * On failure, throws an exception or returns Y_ADVMODE_INVALID.
     */
    Y_ADVMODE_enum      get_advMode(void);

    inline Y_ADVMODE_enum advMode(void)
    { return this->get_advMode(); }

    /**
     * Changes the measuring mode used for the advertised value pushed to the parent hub.
     *
     * @param newval : a value among Y_ADVMODE_IMMEDIATE, Y_ADVMODE_PERIOD_AVG, Y_ADVMODE_PERIOD_MIN and
     * Y_ADVMODE_PERIOD_MAX corresponding to the measuring mode used for the advertised value pushed to the parent hub
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_advMode(Y_ADVMODE_enum newval);
    inline int      setAdvMode(Y_ADVMODE_enum newval)
    { return this->set_advMode(newval); }

    string              get_calibrationParam(void);

    inline string       calibrationParam(void)
    { return this->get_calibrationParam(); }

    int             set_calibrationParam(const string& newval);
    inline int      setCalibrationParam(const string& newval)
    { return this->set_calibrationParam(newval); }

    /**
     * Changes the resolution of the measured physical values. The resolution corresponds to the numerical precision
     * when displaying value. It does not change the precision of the measure itself.
     *
     * @param newval : a floating point number corresponding to the resolution of the measured physical values
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_resolution(double newval);
    inline int      setResolution(double newval)
    { return this->set_resolution(newval); }

    /**
     * Returns the resolution of the measured values. The resolution corresponds to the numerical precision
     * of the measures, which is not always the same as the actual precision of the sensor.
     *
     * @return a floating point number corresponding to the resolution of the measured values
     *
     * On failure, throws an exception or returns Y_RESOLUTION_INVALID.
     */
    double              get_resolution(void);

    inline double       resolution(void)
    { return this->get_resolution(); }

    /**
     * Returns the sensor health state code, which is zero when there is an up-to-date measure
     * available or a positive code if the sensor is not able to provide a measure right now.
     *
     * @return an integer corresponding to the sensor health state code, which is zero when there is an
     * up-to-date measure
     *         available or a positive code if the sensor is not able to provide a measure right now
     *
     * On failure, throws an exception or returns Y_SENSORSTATE_INVALID.
     */
    int                 get_sensorState(void);

    inline int          sensorState(void)
    { return this->get_sensorState(); }

    /**
     * Retrieves a sensor for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the sensor is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YSensor.isOnline() to test if the sensor is
     * indeed online at a given time. In case of ambiguity when looking for
     * a sensor by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the sensor
     *
     * @return a YSensor object allowing you to drive the sensor.
     */
    static YSensor*     FindSensor(string func);

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
    virtual int         registerValueCallback(YSensorValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    virtual int         _parserHelper(void);

    /**
     * Checks if the sensor is currently able to provide an up-to-date measure.
     * Returns false if the device is unreachable, or if the sensor does not have
     * a current measure to transmit. No exception is raised if there is an error
     * while trying to contact the device hosting $THEFUNCTION$.
     *
     * @return true if the sensor can provide an up-to-date measure, and false otherwise
     */
    virtual bool        isSensorReady(void);

    /**
     * Returns the YDatalogger object of the device hosting the sensor. This method returns an object of
     * class YDatalogger that can control global parameters of the data logger. The returned object
     * should not be freed.
     *
     * @return an YDataLogger object or NULL on error.
     */
    virtual YDataLogger* get_dataLogger(void);

    /**
     * Starts the data logger on the device. Note that the data logger
     * will only save the measures on this sensor if the logFrequency
     * is not set to "OFF".
     *
     * @return YAPI_SUCCESS if the call succeeds.
     */
    virtual int         startDataLogger(void);

    /**
     * Stops the datalogger on the device.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     */
    virtual int         stopDataLogger(void);

    /**
     * Retrieves a DataSet object holding historical data for this
     * sensor, for a specified time interval. The measures will be
     * retrieved from the data logger, which must have been turned
     * on at the desired time. See the documentation of the DataSet
     * class for information on how to get an overview of the
     * recorded data, and how to load progressively a large set
     * of measures from the data logger.
     *
     * This function only works if the device uses a recent firmware,
     * as DataSet objects are not supported by firmwares older than
     * version 13000.
     *
     * @param startTime : the start of the desired measure time interval,
     *         as a Unix timestamp, i.e. the number of seconds since
     *         January 1, 1970 UTC. The special value 0 can be used
     *         to include any meaasure, without initial limit.
     * @param endTime : the end of the desired measure time interval,
     *         as a Unix timestamp, i.e. the number of seconds since
     *         January 1, 1970 UTC. The special value 0 can be used
     *         to include any meaasure, without ending limit.
     *
     * @return an instance of YDataSet, providing access to historical
     *         data. Past measures can be loaded progressively
     *         using methods from the YDataSet object.
     */
    virtual YDataSet    get_recordedData(s64 startTime,s64 endTime);

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
    virtual int         registerTimedReportCallback(YSensorTimedReportCallback callback);

    virtual int         _invokeTimedReportCallback(YMeasure value);

    /**
     * Configures error correction data points, in particular to compensate for
     * a possible perturbation of the measure caused by an enclosure. It is possible
     * to configure up to five correction points. Correction points must be provided
     * in ascending order, and be in the range of the sensor. The device will automatically
     * perform a linear interpolation of the error correction between specified
     * points. Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * For more information on advanced capabilities to refine the calibration of
     * sensors, please contact support@yoctopuce.com.
     *
     * @param rawValues : array of floating point numbers, corresponding to the raw
     *         values returned by the sensor for the correction points.
     * @param refValues : array of floating point numbers, corresponding to the corrected
     *         values for the correction points.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         calibrateFromPoints(vector<double> rawValues,vector<double> refValues);

    /**
     * Retrieves error correction data points previously entered using the method
     * calibrateFromPoints.
     *
     * @param rawValues : array of floating point numbers, that will be filled by the
     *         function with the raw sensor values for the correction points.
     * @param refValues : array of floating point numbers, that will be filled by the
     *         function with the desired values for the correction points.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         loadCalibrationPoints(vector<double>& rawValues,vector<double>& refValues);

    virtual string      _encodeCalibrationPoints(vector<double> rawValues,vector<double> refValues);

    virtual double      _applyCalibration(double rawValue);

    virtual YMeasure    _decodeTimedReport(double timestamp,vector<int> report);

    virtual double      _decodeVal(int w);

    virtual double      _decodeAvg(int dw);


    inline static YSensor* Find(string func)
    { return YSensor::FindSensor(func); }

    /**
     * Continues the enumeration of sensors started using yFirstSensor().
     *
     * @return a pointer to a YSensor object, corresponding to
     *         a sensor currently online, or a NULL pointer
     *         if there are no more sensors to enumerate.
     */
           YSensor         *nextSensor(void);
    inline YSensor         *next(void)
    { return this->nextSensor();}

    /**
     * Starts the enumeration of sensors currently accessible.
     * Use the method YSensor.nextSensor() to iterate on
     * next sensors.
     *
     * @return a pointer to a YSensor object, corresponding to
     *         the first sensor currently online, or a NULL pointer
     *         if there are none.
     */
           static YSensor* FirstSensor(void);
    inline static YSensor* First(void)
    { return YSensor::FirstSensor();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YSensor accessors declaration)
};

//--- (generated code: YSensor functions declaration)

/**
 * Retrieves a sensor for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the sensor is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YSensor.isOnline() to test if the sensor is
 * indeed online at a given time. In case of ambiguity when looking for
 * a sensor by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the sensor
 *
 * @return a YSensor object allowing you to drive the sensor.
 */
inline YSensor* yFindSensor(const string& func)
{ return YSensor::FindSensor(func);}
/**
 * Starts the enumeration of sensors currently accessible.
 * Use the method YSensor.nextSensor() to iterate on
 * next sensors.
 *
 * @return a pointer to a YSensor object, corresponding to
 *         the first sensor currently online, or a NULL pointer
 *         if there are none.
 */
inline YSensor* yFirstSensor(void)
{ return YSensor::FirstSensor();}

//--- (end of generated code: YSensor functions declaration)


inline string yGetAPIVersion()
{ return YAPI::GetAPIVersion(); }



/**
 * Initializes the Yoctopuce programming library explicitly.
 * It is not strictly needed to call yInitAPI(), as the library is
 * automatically  initialized when calling yRegisterHub() for the
 * first time.
 *
 * When Y_DETECT_NONE is used as detection mode,
 * you must explicitly use yRegisterHub() to point the API to the
 * VirtualHub on which your devices are connected before trying to access them.
 *
 * @param mode : an integer corresponding to the type of automatic
 *         device detection to use. Possible values are
 *         Y_DETECT_NONE, Y_DETECT_USB, Y_DETECT_NET,
 *         and Y_DETECT_ALL.
 * @param errmsg : a string passed by reference to receive any error message.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
inline YRETCODE yInitAPI(int mode, string& errmsg)
{ return YAPI::InitAPI(mode,errmsg); }

/**
 * Frees dynamically allocated memory blocks used by the Yoctopuce library.
 * It is generally not required to call this function, unless you
 * want to free all dynamically allocated memory blocks in order to
 * track a memory leak for instance.
 * You should not call any other library function after calling
 * yFreeAPI(), or your program will crash.
 */
inline void yFreeAPI()
{ YAPI::FreeAPI(); }

/**
 * Disables the use of exceptions to report runtime errors.
 * When exceptions are disabled, every function returns a specific
 * error value which depends on its type and which is documented in
 * this reference manual.
 */
inline void yDisableExceptions(void)
{ YAPI::DisableExceptions(); }

/**
 * Re-enables the use of exceptions for runtime error handling.
 * Be aware than when exceptions are enabled, every function that fails
 * triggers an exception. If the exception is not caught by the user code,
 * it  either fires the debugger or aborts (i.e. crash) the program.
 * On failure, throws an exception or returns a negative error code.
 */
inline void yEnableExceptions(void)
{ YAPI::EnableExceptions(); }

/**
 * Registers a log callback function. This callback will be called each time
 * the API have something to say. Quite useful to debug the API.
 *
 * @param logfun : a procedure taking a string parameter, or NULL
 *         to unregister a previously registered  callback.
 */
inline void yRegisterLogFunction(yLogFunction logfun)
{ YAPI::RegisterLogFunction(logfun); }

/**
 * Register a callback function, to be called each time
 * a device is plugged. This callback will be invoked while yUpdateDeviceList
 * is running. You will have to call this function on a regular basis.
 *
 * @param arrivalCallback : a procedure taking a YModule parameter, or NULL
 *         to unregister a previously registered  callback.
 */
inline void yRegisterDeviceArrivalCallback(yDeviceUpdateCallback arrivalCallback)
{ YAPI::RegisterDeviceArrivalCallback(arrivalCallback); }

/**
 * Register a callback function, to be called each time
 * a device is unplugged. This callback will be invoked while yUpdateDeviceList
 * is running. You will have to call this function on a regular basis.
 *
 * @param removalCallback : a procedure taking a YModule parameter, or NULL
 *         to unregister a previously registered  callback.
 */
inline void yRegisterDeviceRemovalCallback(yDeviceUpdateCallback removalCallback)
{ YAPI::RegisterDeviceRemovalCallback(removalCallback); }

inline void yRegisterDeviceChangeCallback(yDeviceUpdateCallback removalCallback)
{ YAPI::RegisterDeviceChangeCallback(removalCallback); }


/**
 * Register a callback function, to be called each time an Network Hub send
 * an SSDP message. The callback has two string parameter, the first one
 * contain the serial number of the hub and the second contain the URL of the
 * network hub (this URL can be passed to RegisterHub). This callback will be invoked
 * while yUpdateDeviceList is running. You will have to call this function on a regular basis.
 *
 * @param hubDiscoveryCallback : a procedure taking two string parameter, the serial
 *         number and the hub URL. Use NULL to unregister a previously registered  callback.
 */
inline void yRegisterHubDiscoveryCallback(YHubDiscoveryCallback hubDiscoveryCallback)
{
	YAPI::RegisterHubDiscoveryCallback(hubDiscoveryCallback);
}

/**
 * Force a hub discovery, if a callback as been registered with yRegisterHubDiscoveryCallback it
 * will be called for each net work hub that will respond to the discovery.
 *
 * @param errmsg : a string passed by reference to receive any error message.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *         On failure, throws an exception or returns a negative error code.
 */
inline YRETCODE yTriggerHubDiscovery(string& errmsg)
{
	return YAPI::TriggerHubDiscovery(errmsg);
}


// Register a new value calibration handler for a given calibration type
//
inline void yRegisterCalibrationHandler(int calibrationType, yCalibrationHandler calibrationHandler)
{ YAPI::RegisterCalibrationHandler(calibrationType, calibrationHandler); }

/**
 * Setup the Yoctopuce library to use modules connected on a given machine. The
 * parameter will determine how the API will work. Use the following values:
 *
 * <b>usb</b>: When the usb keyword is used, the API will work with
 * devices connected directly to the USB bus. Some programming languages such a Javascript,
 * PHP, and Java don't provide direct access to USB hardware, so usb will
 * not work with these. In this case, use a VirtualHub or a networked YoctoHub (see below).
 *
 * <b><i>x.x.x.x</i></b> or <b><i>hostname</i></b>: The API will use the devices connected to the
 * host with the given IP address or hostname. That host can be a regular computer
 * running a VirtualHub, or a networked YoctoHub such as YoctoHub-Ethernet or
 * YoctoHub-Wireless. If you want to use the VirtualHub running on you local
 * computer, use the IP address 127.0.0.1.
 *
 * <b>callback</b>: that keyword make the API run in "<i>HTTP Callback</i>" mode.
 * This a special mode allowing to take control of Yoctopuce devices
 * through a NAT filter when using a VirtualHub or a networked YoctoHub. You only
 * need to configure your hub to call your server script on a regular basis.
 * This mode is currently available for PHP and Node.JS only.
 *
 * Be aware that only one application can use direct USB access at a
 * given time on a machine. Multiple access would cause conflicts
 * while trying to access the USB modules. In particular, this means
 * that you must stop the VirtualHub software before starting
 * an application that uses direct USB access. The workaround
 * for this limitation is to setup the library to use the VirtualHub
 * rather than direct USB access.
 *
 * If access control has been activated on the hub, virtual or not, you want to
 * reach, the URL parameter should look like:
 *
 * http://username:password@address:port
 *
 * You can call <i>RegisterHub</i> several times to connect to several machines.
 *
 * @param url : a string containing either "usb","callback" or the
 *         root URL of the hub to monitor
 * @param errmsg : a string passed by reference to receive any error message.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
inline YRETCODE yRegisterHub(const string& url, string& errmsg)
{ return YAPI::RegisterHub(url,errmsg); }

/**
 * Fault-tolerant alternative to RegisterHub(). This function has the same
 * purpose and same arguments as RegisterHub(), but does not trigger
 * an error when the selected hub is not available at the time of the function call.
 * This makes it possible to register a network hub independently of the current
 * connectivity, and to try to contact it only when a device is actively needed.
 *
 * @param url : a string containing either "usb","callback" or the
 *         root URL of the hub to monitor
 * @param errmsg : a string passed by reference to receive any error message.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
inline YRETCODE yPreregisterHub(const string& url, string& errmsg)
{ return YAPI::PreregisterHub(url,errmsg); }

/**
 * Setup the Yoctopuce library to no more use modules connected on a previously
 * registered machine with RegisterHub.
 *
 * @param url : a string containing either "usb" or the
 *         root URL of the hub to monitor
 */
inline void yUnregisterHub(const string& url)
{ YAPI::UnregisterHub(url); }

/**
 * Test if the hub is reachable. This method do not register the hub, it only test if the
 * hub is usable. The url parameter follow the same convention as the RegisterHub
 * method. This method is useful to verify the authentication parameters for a hub. It
 * is possible to force this method to return after mstimeout milliseconds.
 *
 * @param url : a string containing either "usb","callback" or the
 *         root URL of the hub to monitor
 * @param mstimeout : the number of millisecond available to test the connection.
 * @param errmsg : a string passed by reference to receive any error message.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure returns a negative error code.
 */
inline YRETCODE yTestHub(const string& url, int mstimeout, string& errmsg)
{ return YAPI::TestHub(url, mstimeout, errmsg); }

/**
 * Triggers a (re)detection of connected Yoctopuce modules.
 * The library searches the machines or USB ports previously registered using
 * yRegisterHub(), and invokes any user-defined callback function
 * in case a change in the list of connected devices is detected.
 *
 * This function can be called as frequently as desired to refresh the device list
 * and to make the application aware of hot-plug events.
 *
 * @param errmsg : a string passed by reference to receive any error message.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
inline YRETCODE yUpdateDeviceList(string& errmsg)
{ return YAPI::UpdateDeviceList(errmsg); }

/**
 * Maintains the device-to-library communication channel.
 * If your program includes significant loops, you may want to include
 * a call to this function to make sure that the library takes care of
 * the information pushed by the modules on the communication channels.
 * This is not strictly necessary, but it may improve the reactivity
 * of the library for the following commands.
 *
 * This function may signal an error in case there is a communication problem
 * while contacting a module.
 *
 * @param errmsg : a string passed by reference to receive any error message.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
inline YRETCODE yHandleEvents(string& errmsg)
{ return YAPI::HandleEvents(errmsg); }

/**
 * Pauses the execution flow for a specified duration.
 * This function implements a passive waiting loop, meaning that it does not
 * consume CPU cycles significantly. The processor is left available for
 * other threads and processes. During the pause, the library nevertheless
 * reads from time to time information from the Yoctopuce modules by
 * calling yHandleEvents(), in order to stay up-to-date.
 *
 * This function may signal an error in case there is a communication problem
 * while contacting a module.
 *
 * @param ms_duration : an integer corresponding to the duration of the pause,
 *         in milliseconds.
 * @param errmsg : a string passed by reference to receive any error message.
 *
 * @return YAPI_SUCCESS when the call succeeds.
 *
 * On failure, throws an exception or returns a negative error code.
 */
inline YRETCODE ySleep(unsigned ms_duration, string& errmsg)
{ return YAPI::Sleep(ms_duration, errmsg); }

/**
 * Returns the current value of a monotone millisecond-based time counter.
 * This counter can be used to compute delays in relation with
 * Yoctopuce devices, which also uses the millisecond as timebase.
 *
 * @return a long integer corresponding to the millisecond counter.
 */
inline u64 yGetTickCount(void)
{ return YAPI::GetTickCount(); }

/**
 * Checks if a given string is valid as logical name for a module or a function.
 * A valid logical name has a maximum of 19 characters, all among
 * A..Z, a..z, 0..9, _, and -.
 * If you try to configure a logical name with an incorrect string,
 * the invalid characters are ignored.
 *
 * @param name : a string containing the name to check.
 *
 * @return true if the name is valid, false otherwise.
 */
inline bool yCheckLogicalName(const string& name)
{ return YAPI::CheckLogicalName(name); }

//--- (generated code: YModule functions declaration)

/**
 * Allows you to find a module from its serial number or from its logical name.
 *
 * This function does not require that the module is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YModule.isOnline() to test if the module is
 * indeed online at a given time. In case of ambiguity when looking for
 * a module by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string containing either the serial number or
 *         the logical name of the desired module
 *
 * @return a YModule object allowing you to drive the module
 *         or get additional information on the module.
 */
inline YModule* yFindModule(const string& func)
{ return YModule::FindModule(func);}
/**
 * Starts the enumeration of modules currently accessible.
 * Use the method YModule.nextModule() to iterate on the
 * next modules.
 *
 * @return a pointer to a YModule object, corresponding to
 *         the first module currently online, or a NULL pointer
 *         if there are none.
 */
inline YModule* yFirstModule(void)
{ return YModule::FirstModule();}

//--- (end of generated code: YModule functions declaration)



/**
 * YOldDataStream Class: Sequence of measured data, returned by the data logger
 *
 * A data stream is a small collection of consecutive measures for a set
 * of sensors. A few properties are available directly from the object itself
 * (they are preloaded at instantiation time), while most other properties and
 * the actual data are loaded on demand when accessed for the first time.
 *
 * This is the old version of the YDataStream class, used for backward-compatibility
 * with devices with firmware < 13000
 */
class YOldDataStream: public YDataStream {
protected:
    // Data preloaded on object instantiation
    YDataLogger     *_dataLogger;
    unsigned        _timeStamp;
    unsigned        _interval;

public:
    YOldDataStream(YDataLogger *parent, unsigned run,
                   unsigned stamp, unsigned utc, unsigned itv);

    // override new version with backward-compatible code
    int loadStream(void);

    /**
     * Returns the relative start time of the data stream, measured in seconds.
     * For recent firmwares, the value is relative to the present time,
     * which means the value is always negative.
     * If the device uses a firmware older than version 13000, value is
     * relative to the start of the time the device was powered on, and
     * is always positive.
     * If you need an absolute UTC timestamp, use get_startTimeUTC().
     *
     * @return an unsigned number corresponding to the number of seconds
     *         between the start of the run and the beginning of this data
     *         stream.
     */
           int          get_startTime(void);
    inline int          startTime(void)
    { return this->get_startTime(); }

    /**
     * Returns the number of seconds elapsed between  two consecutive
     * rows of this data stream. By default, the data logger records one row
     * per second, but there might be alternative streams at lower resolution
     * created by summarizing the original stream for archiving purposes.
     *
     * This method does not cause any access to the device, as the value
     * is preloaded in the object at instantiation time.
     *
     * @return an unsigned number corresponding to a number of seconds.
     */
           double       get_dataSamplesInterval(void);
    inline double       dataSamplesInterval(void)
    { return this->get_dataSamplesInterval(); }
};

//--- (generated code: YDataLogger declaration)
/**
 * YDataLogger Class: DataLogger function interface
 *
 * Yoctopuce sensors include a non-volatile memory capable of storing ongoing measured
 * data automatically, without requiring a permanent connection to a computer.
 * The DataLogger function controls the global parameters of the internal data
 * logger.
 */
class YOCTO_CLASS_EXPORT YDataLogger: public YFunction {
#ifdef __BORLANDC__
#pragma option push -w-8022
#endif
//--- (end of generated code: YDataLogger declaration)
    //--- (generated code: YDataLogger attributes)
    // Attributes (function value cache)
    int             _currentRunIndex;
    s64             _timeUTC;
    Y_RECORDING_enum _recording;
    Y_AUTOSTART_enum _autoStart;
    Y_BEACONDRIVEN_enum _beaconDriven;
    Y_CLEARHISTORY_enum _clearHistory;
    YDataLoggerValueCallback _valueCallbackDataLogger;

    friend YDataLogger *yFindDataLogger(const string& func);
    friend YDataLogger *yFirstDataLogger(void);

    // Function-specific method for parsing of JSON output and caching result
    virtual int     _parseAttr(YJSONObject* json_val);

    // Constructor is protected, use yFindDataLogger factory function to instantiate
    YDataLogger(const string& func);
    //--- (end of generated code: YDataLogger attributes)
    //--- (generated code: YDataLogger initialization)
    //--- (end of generated code: YDataLogger initialization)

    // device-specific URL to access the datalogger
    string          dataLoggerURL;

    // DataLogger-specific method to retrieve and pre-parse recorded data
    int             getData(unsigned runIdx, unsigned timeIdx, string &buffer, yJsonStateMachine &j);

    // YOldDataStream loadStream() will use our method getData()
    friend int YOldDataStream::loadStream(void);

public:
    ~YDataLogger();

    /**
     * Builds a list of all data streams hold by the data logger (legacy method).
     * The caller must pass by reference an empty array to hold YDataStream
     * objects, and the function fills it with objects describing available
     * data sequences.
     *
     * This is the old way to retrieve data from the DataLogger.
     * For new applications, you should rather use get_dataSets()
     * method, or call directly get_recordedData() on the
     * sensor object.
     *
     * @param v : an array of YDataStream objects to be filled in
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             get_dataStreams(vector<YDataStream *>& v);
    inline int dataStreams(vector<YDataStream *>& v)
    { return this->get_dataStreams(v);}


    //--- (generated code: YDataLogger accessors declaration)

    static const int CURRENTRUNINDEX_INVALID = YAPI_INVALID_UINT;
    static const s64 TIMEUTC_INVALID = YAPI_INVALID_LONG;
    static const Y_RECORDING_enum RECORDING_OFF = Y_RECORDING_OFF;
    static const Y_RECORDING_enum RECORDING_ON = Y_RECORDING_ON;
    static const Y_RECORDING_enum RECORDING_PENDING = Y_RECORDING_PENDING;
    static const Y_RECORDING_enum RECORDING_INVALID = Y_RECORDING_INVALID;
    static const Y_AUTOSTART_enum AUTOSTART_OFF = Y_AUTOSTART_OFF;
    static const Y_AUTOSTART_enum AUTOSTART_ON = Y_AUTOSTART_ON;
    static const Y_AUTOSTART_enum AUTOSTART_INVALID = Y_AUTOSTART_INVALID;
    static const Y_BEACONDRIVEN_enum BEACONDRIVEN_OFF = Y_BEACONDRIVEN_OFF;
    static const Y_BEACONDRIVEN_enum BEACONDRIVEN_ON = Y_BEACONDRIVEN_ON;
    static const Y_BEACONDRIVEN_enum BEACONDRIVEN_INVALID = Y_BEACONDRIVEN_INVALID;
    static const Y_CLEARHISTORY_enum CLEARHISTORY_FALSE = Y_CLEARHISTORY_FALSE;
    static const Y_CLEARHISTORY_enum CLEARHISTORY_TRUE = Y_CLEARHISTORY_TRUE;
    static const Y_CLEARHISTORY_enum CLEARHISTORY_INVALID = Y_CLEARHISTORY_INVALID;

    /**
     * Returns the current run number, corresponding to the number of times the module was
     * powered on with the dataLogger enabled at some point.
     *
     * @return an integer corresponding to the current run number, corresponding to the number of times the module was
     *         powered on with the dataLogger enabled at some point
     *
     * On failure, throws an exception or returns Y_CURRENTRUNINDEX_INVALID.
     */
    int                 get_currentRunIndex(void);

    inline int          currentRunIndex(void)
    { return this->get_currentRunIndex(); }

    /**
     * Returns the Unix timestamp for current UTC time, if known.
     *
     * @return an integer corresponding to the Unix timestamp for current UTC time, if known
     *
     * On failure, throws an exception or returns Y_TIMEUTC_INVALID.
     */
    s64                 get_timeUTC(void);

    inline s64          timeUTC(void)
    { return this->get_timeUTC(); }

    /**
     * Changes the current UTC time reference used for recorded data.
     *
     * @param newval : an integer corresponding to the current UTC time reference used for recorded data
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_timeUTC(s64 newval);
    inline int      setTimeUTC(s64 newval)
    { return this->set_timeUTC(newval); }

    /**
     * Returns the current activation state of the data logger.
     *
     * @return a value among Y_RECORDING_OFF, Y_RECORDING_ON and Y_RECORDING_PENDING corresponding to the
     * current activation state of the data logger
     *
     * On failure, throws an exception or returns Y_RECORDING_INVALID.
     */
    Y_RECORDING_enum    get_recording(void);

    inline Y_RECORDING_enum recording(void)
    { return this->get_recording(); }

    /**
     * Changes the activation state of the data logger to start/stop recording data.
     *
     * @param newval : a value among Y_RECORDING_OFF, Y_RECORDING_ON and Y_RECORDING_PENDING corresponding
     * to the activation state of the data logger to start/stop recording data
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_recording(Y_RECORDING_enum newval);
    inline int      setRecording(Y_RECORDING_enum newval)
    { return this->set_recording(newval); }

    /**
     * Returns the default activation state of the data logger on power up.
     *
     * @return either Y_AUTOSTART_OFF or Y_AUTOSTART_ON, according to the default activation state of the
     * data logger on power up
     *
     * On failure, throws an exception or returns Y_AUTOSTART_INVALID.
     */
    Y_AUTOSTART_enum    get_autoStart(void);

    inline Y_AUTOSTART_enum autoStart(void)
    { return this->get_autoStart(); }

    /**
     * Changes the default activation state of the data logger on power up.
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @param newval : either Y_AUTOSTART_OFF or Y_AUTOSTART_ON, according to the default activation state
     * of the data logger on power up
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_autoStart(Y_AUTOSTART_enum newval);
    inline int      setAutoStart(Y_AUTOSTART_enum newval)
    { return this->set_autoStart(newval); }

    /**
     * Returns true if the data logger is synchronised with the localization beacon.
     *
     * @return either Y_BEACONDRIVEN_OFF or Y_BEACONDRIVEN_ON, according to true if the data logger is
     * synchronised with the localization beacon
     *
     * On failure, throws an exception or returns Y_BEACONDRIVEN_INVALID.
     */
    Y_BEACONDRIVEN_enum get_beaconDriven(void);

    inline Y_BEACONDRIVEN_enum beaconDriven(void)
    { return this->get_beaconDriven(); }

    /**
     * Changes the type of synchronisation of the data logger.
     * Remember to call the saveToFlash() method of the module if the
     * modification must be kept.
     *
     * @param newval : either Y_BEACONDRIVEN_OFF or Y_BEACONDRIVEN_ON, according to the type of
     * synchronisation of the data logger
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    int             set_beaconDriven(Y_BEACONDRIVEN_enum newval);
    inline int      setBeaconDriven(Y_BEACONDRIVEN_enum newval)
    { return this->set_beaconDriven(newval); }

    Y_CLEARHISTORY_enum get_clearHistory(void);

    inline Y_CLEARHISTORY_enum clearHistory(void)
    { return this->get_clearHistory(); }

    int             set_clearHistory(Y_CLEARHISTORY_enum newval);
    inline int      setClearHistory(Y_CLEARHISTORY_enum newval)
    { return this->set_clearHistory(newval); }

    /**
     * Retrieves a data logger for a given identifier.
     * The identifier can be specified using several formats:
     * <ul>
     * <li>FunctionLogicalName</li>
     * <li>ModuleSerialNumber.FunctionIdentifier</li>
     * <li>ModuleSerialNumber.FunctionLogicalName</li>
     * <li>ModuleLogicalName.FunctionIdentifier</li>
     * <li>ModuleLogicalName.FunctionLogicalName</li>
     * </ul>
     *
     * This function does not require that the data logger is online at the time
     * it is invoked. The returned object is nevertheless valid.
     * Use the method YDataLogger.isOnline() to test if the data logger is
     * indeed online at a given time. In case of ambiguity when looking for
     * a data logger by logical name, no error is notified: the first instance
     * found is returned. The search is performed first by hardware name,
     * then by logical name.
     *
     * If a call to this object's is_online() method returns FALSE although
     * you are certain that the matching device is plugged, make sure that you did
     * call registerHub() at application initialization time.
     *
     * @param func : a string that uniquely characterizes the data logger
     *
     * @return a YDataLogger object allowing you to drive the data logger.
     */
    static YDataLogger* FindDataLogger(string func);

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
    virtual int         registerValueCallback(YDataLoggerValueCallback callback);
    using YFunction::registerValueCallback;

    virtual int         _invokeValueCallback(string value);

    /**
     * Clears the data logger memory and discards all recorded data streams.
     * This method also resets the current run index to zero.
     *
     * @return YAPI_SUCCESS if the call succeeds.
     *
     * On failure, throws an exception or returns a negative error code.
     */
    virtual int         forgetAllDataStreams(void);

    /**
     * Returns a list of YDataSet objects that can be used to retrieve
     * all measures stored by the data logger.
     *
     * This function only works if the device uses a recent firmware,
     * as YDataSet objects are not supported by firmwares older than
     * version 13000.
     *
     * @return a list of YDataSet object.
     *
     * On failure, throws an exception or returns an empty list.
     */
    virtual vector<YDataSet> get_dataSets(void);

    virtual vector<YDataSet> parse_dataSets(string json);


    inline static YDataLogger* Find(string func)
    { return YDataLogger::FindDataLogger(func); }

    /**
     * Continues the enumeration of data loggers started using yFirstDataLogger().
     *
     * @return a pointer to a YDataLogger object, corresponding to
     *         a data logger currently online, or a NULL pointer
     *         if there are no more data loggers to enumerate.
     */
           YDataLogger     *nextDataLogger(void);
    inline YDataLogger     *next(void)
    { return this->nextDataLogger();}

    /**
     * Starts the enumeration of data loggers currently accessible.
     * Use the method YDataLogger.nextDataLogger() to iterate on
     * next data loggers.
     *
     * @return a pointer to a YDataLogger object, corresponding to
     *         the first data logger currently online, or a NULL pointer
     *         if there are none.
     */
           static YDataLogger* FirstDataLogger(void);
    inline static YDataLogger* First(void)
    { return YDataLogger::FirstDataLogger();}
#ifdef __BORLANDC__
#pragma option pop
#endif
    //--- (end of generated code: YDataLogger accessors declaration)
};


//--- (generated code: YDataLogger functions declaration)

/**
 * Retrieves a data logger for a given identifier.
 * The identifier can be specified using several formats:
 * <ul>
 * <li>FunctionLogicalName</li>
 * <li>ModuleSerialNumber.FunctionIdentifier</li>
 * <li>ModuleSerialNumber.FunctionLogicalName</li>
 * <li>ModuleLogicalName.FunctionIdentifier</li>
 * <li>ModuleLogicalName.FunctionLogicalName</li>
 * </ul>
 *
 * This function does not require that the data logger is online at the time
 * it is invoked. The returned object is nevertheless valid.
 * Use the method YDataLogger.isOnline() to test if the data logger is
 * indeed online at a given time. In case of ambiguity when looking for
 * a data logger by logical name, no error is notified: the first instance
 * found is returned. The search is performed first by hardware name,
 * then by logical name.
 *
 * If a call to this object's is_online() method returns FALSE although
 * you are certain that the matching device is plugged, make sure that you did
 * call registerHub() at application initialization time.
 *
 * @param func : a string that uniquely characterizes the data logger
 *
 * @return a YDataLogger object allowing you to drive the data logger.
 */
inline YDataLogger* yFindDataLogger(const string& func)
{ return YDataLogger::FindDataLogger(func);}
/**
 * Starts the enumeration of data loggers currently accessible.
 * Use the method YDataLogger.nextDataLogger() to iterate on
 * next data loggers.
 *
 * @return a pointer to a YDataLogger object, corresponding to
 *         the first data logger currently online, or a NULL pointer
 *         if there are none.
 */
inline YDataLogger* yFirstDataLogger(void)
{ return YDataLogger::FirstDataLogger();}

//--- (end of generated code: YDataLogger functions declaration)


#endif
