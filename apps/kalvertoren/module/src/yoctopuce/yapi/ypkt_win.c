/*********************************************************************
 *
 * $Id: ypkt_win.c 29389 2017-12-07 08:57:39Z seb $
 *
 * OS-specific USB packet layer, Windows version
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

#define __FILE_ID__  "ypkt_win"
#include "yapi.h"
#if defined(WINDOWS_API) && !defined(WINCE)
#include "yproto.h"
#include <TlHelp32.h>
#ifdef LOG_DEVICE_PATH
#define DP(PATH) (PATH)
#else
#define DP(PATH) "..."
#endif

#ifndef _MSC_VER
#define SPDRP_INSTALL_STATE (0x00000022)
#define _TRUNCATE ((size_t)-1)
#endif

#define yWinSetErr(iface,errmsg)  yWinSetErrEx(__LINE__,iface,GetLastError(),"",errmsg)

static int yWinSetErrEx(u32 line, yInterfaceSt *iface, DWORD err, const char *msg, char *errmsg)
{
    int len;
    if (errmsg == NULL)
        return YAPI_IO_ERROR;
    if (iface) {
        YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "%s:%d(%s:%d): %s(%d)", iface->serial, iface->ifaceno, __FILE_ID__, line, msg, (u32)err);
    } else {
        YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "%s:%d: %s(%d)", __FILE_ID__, line, msg, (u32)err);
    }
    len = YSTRLEN(errmsg);
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)(errmsg + len),
        YOCTO_ERRMSG_LEN - len, NULL);

    return YAPI_IO_ERROR;
}


#if 0
static void yWinPushEx(u32 line, yInterfaceSt *iface, pktQueue  *q, DWORD err)
{
    int len;
    char errmsg[YOCTO_ERRMSG_LEN];

    YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "%s:%d(%s:%d): (%d)", iface->serial, iface->ifaceno, __FILE_ID__, line, (u32)err);
    len = YSTRLEN(errmsg);
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)(errmsg + len),
        YOCTO_ERRMSG_LEN - len, NULL);
    yPktQueueSetError(q, YAPI_IO_ERROR, errmsg);
}

#endif


static u32 decodeHex(const char *p, int nbdigit)
{
    u32 ret = 0;
    int i;
    for (i = nbdigit - 1; i >= 0; i--, p++) {
        u32 digit;
        if (*p >= 'a' && *p <= 'f') {
            digit = 10 + *p - 'a';
        } else if (*p >= 'A' && *p <= 'F') {
            digit = 10 + *p - 'A';
        } else if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        } else {
            return 0;
        }
        ret += digit << (4 * i);
    }
    return ret;
}



#define FIRST_OF        1
#define FIRST_NOT_OF    2

static char* findDelim(char *str, const char *delimiters, const int nbdelim, const int mode)
{
    int d;

    while (*str) {
        for (d = 0; d < nbdelim; d++) {
            if (*str == delimiters[d]) {
                break;
            }
        }
        if (mode == FIRST_OF) {
            if (d < nbdelim) {
                return str;
            }
        } else {
            if (d == nbdelim) {
                return str;
            }
        }
        str++;
    }
    return str;
}


void DecodeHardwareid(char *str, u32 *vendorid, u32 *deviceid, u32 *release, u32 *iface)
{
    const char *delim = "\\&?";
    char *token_start, *token_stop;
    token_start = findDelim(str, delim, 4, FIRST_NOT_OF);
    token_stop = findDelim(token_start, delim, 4, FIRST_OF);
    *vendorid = *deviceid = *release = *iface = 0;
    while (token_start != token_stop) {
        if (YSTRNICMP(token_start, "VID_", 4) == 0) {
            *vendorid = decodeHex(token_start + 4, 4);
        } else if (YSTRNICMP(token_start, "PID_", 4) == 0) {
            *deviceid = decodeHex(token_start + 4, 4);
        } else if (YSTRNICMP(token_start, "REV_", 4) == 0) {
            *release = decodeHex(token_start + 4, 4);
        } else if (YSTRNICMP(token_start, "MI_", 3) == 0) {
            *iface = decodeHex(token_start + 3, 2);
        }
        token_start = findDelim(token_stop, delim, 2, FIRST_NOT_OF);
        token_stop = findDelim(token_start, delim, 2, FIRST_OF);
    }
}



static int getProcName(char *buffer, int buffer_size)
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    DWORD pid = GetCurrentProcessId();
    *buffer = 0;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        dbglog("CreateToolhelp32Snapshot (of processes)\n");
        return pid;
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32)) {
        dbglog("Process32First error\n"); // show cause of failure
        CloseHandle(hProcessSnap);          // clean the snapshot object
        return pid;
    }

    // Now walk the snapshot of processes, and
    // display information about each process in turn
    do {
        if (pid == pe32.th32ProcessID) {
#ifndef  UNICODE
            YSTRCPY(buffer, buffer_size, pe32.szExeFile);
#else
#if defined(_MSC_VER) && (_MSC_VER > MSC_VS2003)
            {
                size_t          len;
                wcstombs_s(&len, buffer, buffer_size, (wchar_t*)pe32.szExeFile, _TRUNCATE);
            }
#else
            wcstombs(buffer, (wchar_t*)pe32.szExeFile, buffer_size);
#endif
#endif

            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));
    buffer[buffer_size - 1] = 0;
    CloseHandle(hProcessSnap);
    return pid;
}



#define LEGACY_YOCTOPUCE_KEY "Software\\Yoctopuce\\"
#define USB_LOCK_KEY "Software\\Yoctopuce\\usb_lock\\"
#ifndef KEY_WOW64_32KEY
// dirty hack to work with mingw32
#define KEY_WOW64_32KEY 0
#endif

static int yConvertUSBLockKey(yContextSt *ctx, int deletekey)
{
    HKEY key;
    LONG res;
    res = ctx->registry.yRegOpenKeyEx(HKEY_LOCAL_MACHINE, LEGACY_YOCTOPUCE_KEY, 0, KEY_WRITE | KEY_READ | KEY_WOW64_32KEY, &key);
    if (res == ERROR_SUCCESS) {
        //save information
        char process_name[512];
        char buffer[32];
        DWORD value_length = 32;
        res = ctx->registry.yRegQueryValueEx(key, "process_id", NULL, NULL, (LPBYTE)buffer, &value_length);
        if (res != ERROR_SUCCESS) {
            return -1;
        }
        value_length = 512;
        res = ctx->registry.yRegQueryValueEx(key, "process_name", NULL, NULL, (LPBYTE)process_name, &value_length);
        if (res != ERROR_SUCCESS) {
            return -1;
        }
        if (deletekey) {
            // delete key
            ctx->registry.yRegCloseKey(key);
            res = ctx->registry.yRegDeleteKeyEx(HKEY_LOCAL_MACHINE, LEGACY_YOCTOPUCE_KEY, KEY_WOW64_32KEY, 0);
        } else {
            ctx->registry.yRegDeleteValue(key, "process_id");
            ctx->registry.yRegDeleteValue(key, "process_name");
            ctx->registry.yRegCloseKey(key);
        }
        // create new one
        res = ctx->registry.yRegCreateKeyEx(HKEY_LOCAL_MACHINE, USB_LOCK_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ | KEY_WOW64_32KEY, NULL, &key, NULL);
        // save value in new key
        if (ctx->registry.yRegSetValueEx(key, "process_id", 0, REG_SZ, (BYTE*)buffer, YSTRLEN(buffer)) != ERROR_SUCCESS) {
            dbglog("Unable to set registry value yapi_process");
        }
        if (ctx->registry.yRegSetValueEx(key, "process_name", 0, REG_SZ, (BYTE*)process_name, YSTRLEN(process_name)) != ERROR_SUCCESS) {
            dbglog("Unable to set registry value yapi_process");
        }
        ctx->registry.yRegCloseKey(key);
        return 1;
    } else {
        return -1;
    }
}

// return 1 if we can reserve access to the device 0 if the device
// is already reserved
static int yReserveGlobalAccess(yContextSt *ctx, char * errmsg)
{
    int has_reg_key = 1;
    char process_name[512];
    char buffer[32];
    DWORD value_length = 512;
    int retval;
    s64 pid;
    HKEY key = NULL;
    LONG res;

    if (ctx->registry.hREG != NULL) {
        res = ctx->registry.yRegCreateKeyEx(HKEY_LOCAL_MACHINE, USB_LOCK_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ | KEY_WOW64_32KEY, NULL, &key, NULL);
        if (res == ERROR_CHILD_MUST_BE_VOLATILE) {
            yConvertUSBLockKey(ctx, 1);
            res = ctx->registry.yRegCreateKeyEx(HKEY_LOCAL_MACHINE, USB_LOCK_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ | KEY_WOW64_32KEY, NULL, &key, NULL);
            if (res != ERROR_SUCCESS) {
                has_reg_key = 0;
            }
        } else if (res == ERROR_SUCCESS) {
            yConvertUSBLockKey(ctx, 0);
        } else {
            has_reg_key = 0;
        }
    } else {
        has_reg_key = 0;
    }

    ctx->apiLock = CreateMailslotA("\\\\.\\mailslot\\yoctopuce_yapi", 8, 0, NULL);
    ctx->nameLock = CreateMailslotA("\\\\.\\mailslot\\yoctopuce_yapi_name", 8, 0, NULL);
    if (ctx->apiLock == INVALID_HANDLE_VALUE) {
        // unable to create lock -> another instance is already using the device
        retval = YAPI_DOUBLE_ACCES;
        YERRMSG(YAPI_DOUBLE_ACCES, "Another process is already using yAPI");
        if (has_reg_key && ctx->nameLock == INVALID_HANDLE_VALUE) {
            pid = -1;
            res = ctx->registry.yRegQueryValueEx(key, "process_id", NULL, NULL, (LPBYTE)buffer, &value_length);
            if (res == ERROR_SUCCESS) {
                pid = atoi(buffer);
            }
            value_length = 512;
            res = ctx->registry.yRegQueryValueEx(key, "process_name", NULL, NULL, (LPBYTE)process_name, &value_length);
            if (res == ERROR_SUCCESS && pid >= 0) {
                char current_name[512];
                getProcName(current_name, 512);
                if (YSTRCMP(process_name, current_name)) {
                    YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "Another process named %s (pid %"FMTs64") is already using yAPI", process_name, pid);
                } else {
                    YSPRINTF(errmsg, YOCTO_ERRMSG_LEN, "Another %s (pid %"FMTs64") is already using yAPI", process_name, pid);
                }
            }
        }
    } else {
        retval = YAPI_SUCCESS;
        if (has_reg_key) {
            int pid = getProcName(process_name, 512);
            YSPRINTF(buffer, 32, "%d", pid);
            if (ctx->registry.yRegSetValueEx(key, "process_id", 0, REG_SZ, (BYTE*)buffer, YSTRLEN(buffer)) != ERROR_SUCCESS) {
                dbglog("Unable to set registry value yapi_process");
            }
            if (ctx->registry.yRegSetValueEx(key, "process_name", 0, REG_SZ, (BYTE*)process_name, YSTRLEN(process_name)) != ERROR_SUCCESS) {
                dbglog("Unable to set registry value yapi_process");
            }
        }
    }
    if (has_reg_key) {
        ctx->registry.yRegCloseKey(key);
    }

    return retval;
}

static void yReleaseGlobalAccess(yContextSt *ctx)
{
    CloseHandle(ctx->apiLock);
    ctx->apiLock = INVALID_HANDLE_VALUE;
    CloseHandle(ctx->nameLock);
    ctx->nameLock = INVALID_HANDLE_VALUE;
}



int yyyUSB_init(yContextSt *ctx, char *errmsg)
{
    ctx->registry.hREG = LoadLibraryA("Advapi32.dll");
    if (ctx->registry.hREG != NULL) {
        //Update the pointers:
        ctx->registry.yRegCreateKeyEx = (PYRegCreateKeyEx)GetProcAddress(ctx->registry.hREG, "RegCreateKeyExA");
        ctx->registry.yRegOpenKeyEx = (PYRegOpenKeyEx)GetProcAddress(ctx->registry.hREG, "RegOpenKeyExA");
        ctx->registry.yRegSetValueEx = (PYRegSetValueEx)GetProcAddress(ctx->registry.hREG, "RegSetValueExA");
        ctx->registry.yRegQueryValueEx = (PYRegQueryValueEx)GetProcAddress(ctx->registry.hREG, "RegQueryValueExA");
        ctx->registry.yRegDeleteValue = (PYRegDeleteValue)GetProcAddress(ctx->registry.hREG, "RegDeleteValueA");
        ctx->registry.yRegCloseKey = (PYRegCloseKey)GetProcAddress(ctx->registry.hREG, "RegCloseKey");
        ctx->registry.yRegDeleteKeyEx = (PYRegDeleteKeyEx)GetProcAddress(ctx->registry.hREG, "RegDeleteKeyExA");
    }


    YPROPERR(yReserveGlobalAccess(ctx, errmsg));
    ctx->hid.hHID = LoadLibraryA("HID.DLL");
    if (ctx->hid.hHID == NULL) {
        return yWinSetErr(NULL, errmsg);
    }
    //Update the pointers:
    ctx->hid.GetHidGuid = (PHidD_GetHidGuid)GetProcAddress(ctx->hid.hHID, "HidD_GetHidGuid");
    ctx->hid.GetAttributes = (PHidD_GetAttributes)GetProcAddress(ctx->hid.hHID, "HidD_GetAttributes");
    ctx->hid.GetManufacturerString = (PHidD_GetManufacturerString)GetProcAddress(ctx->hid.hHID, "HidD_GetManufacturerString");
    ctx->hid.GetProductString = (PHidD_GetProductString)GetProcAddress(ctx->hid.hHID, "HidD_GetProductString");
    ctx->hid.GetSerialNumberString = (PHidD_GetSerialNumberString)GetProcAddress(ctx->hid.hHID, "HidD_GetSerialNumberString");
    ctx->hid.SetNumInputBuffers = (PHidD_SetNumInputBuffers)GetProcAddress(ctx->hid.hHID, "HidD_SetNumInputBuffers");
    yInitializeCriticalSection(&ctx->prevEnum_cs);

    return YAPI_SUCCESS;
}


int yyyUSB_stop(yContextSt *ctx, char *errmsg)
{
    yDeleteCriticalSection(&ctx->prevEnum_cs);
    if (ctx->prevEnum) {
        yFree(ctx->prevEnum);
        ctx->prevEnum = NULL;
    }
    ctx->prevEnumCnt = 0;
    yReleaseGlobalAccess(ctx);
    return YAPI_SUCCESS;
}
/*****************************************************************
* yPacket API without cycling logic
*****************************************************************/


// no check on reentrance or initializations since we are only called
// by the yUpdateDeviceList witch take care of all this stuff
// the caller is responsible of freeing the ifaces buffer (if not set to NULL)
int yyyUSBGetInterfaces(yInterfaceSt **ifaces, int *nbifaceDetect, char *errmsg)
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA_A    pDetailedInterfaceData;
    int         index = 0;
    GUID        InterfaceClassGuid;
    DWORD       needsize;
    HDEVINFO    DeviceInfoTable = INVALID_HANDLE_VALUE;
    int         nbifaceAlloc;
    char        buffer[WIN_DEVICE_PATH_LEN];//buffer forp DetailedInterfaceData

    *ifaces = NULL;
    //setup some windows stuff
    yContext->hid.GetHidGuid(&InterfaceClassGuid);
    DeviceInfoTable = SetupDiGetClassDevs(&InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (DeviceInfoTable == INVALID_HANDLE_VALUE) {
        return yWinSetErr(NULL, errmsg);
    }
    yEnterCriticalSection(&yContext->prevEnum_cs);
    pDetailedInterfaceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)buffer;
    // allocate buffer for detected interfaces
    *nbifaceDetect = 0;
    nbifaceAlloc = yContext->prevEnumCnt + 8;
    *ifaces = (yInterfaceSt*)yMalloc(nbifaceAlloc * sizeof(yInterfaceSt));
    memset(*ifaces, 0, nbifaceAlloc * sizeof(yInterfaceSt));

    //Now look through the list we just populated.  We are trying to see if any of them match our device.
    for (;; index++) {
        u32             vendorid, deviceid, release, ifaceno, inst_state;
        SP_DEVINFO_DATA DevInfoData;

        DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        if (!SetupDiEnumDeviceInfo(DeviceInfoTable, index, &DevInfoData)) {
            //no more device
            break;
        }
        // get device hardwareid string
        SetupDiGetDeviceRegistryPropertyA(DeviceInfoTable, &DevInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)buffer, 512, NULL);
        // skip 4 first char ("HID/")
        DecodeHardwareid(buffer + 4, &vendorid, &deviceid, &release, &ifaceno);
        // get installation status see:http://msdn.microsoft.com/en-us/library/ff543130
        SetupDiGetDeviceRegistryProperty(DeviceInfoTable, &DevInfoData, SPDRP_INSTALL_STATE, NULL, (PBYTE)&inst_state, sizeof(inst_state), &needsize);
        if (vendorid == YOCTO_VENDORID && inst_state == 0) {
            SP_DEVICE_INTERFACE_DATA    InterfaceData;
            yInterfaceSt    *iface;
            int             find, retry = 16;

            //ensure the buffer of detected interface is big enought
            if (*nbifaceDetect == nbifaceAlloc) {
                yInterfaceSt    *tmp;
                u32 newsize = nbifaceAlloc * 2 * sizeof(yInterfaceSt);
                tmp = (yInterfaceSt*)yMalloc(newsize);
                memset(tmp, 0, newsize);
                yMemcpy(tmp, *ifaces, nbifaceAlloc * sizeof(yInterfaceSt));
                yFree(*ifaces);
                *ifaces = tmp;
                nbifaceAlloc *= 2;
            }
            iface = *ifaces + *nbifaceDetect;
            iface->vendorid = (u16)vendorid;
            iface->deviceid = (u16)deviceid;
            iface->ifaceno = (u16)ifaceno;

            InterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
            if (!SetupDiEnumDeviceInterfaces(DeviceInfoTable, &DevInfoData, &InterfaceClassGuid, 0, &InterfaceData)) {
                dbglog("Fail to retrieve DeviceInterfaces");
                continue;
            }
            SetupDiGetDeviceInterfaceDetailA(DeviceInfoTable, &InterfaceData, NULL, 0, &needsize, NULL);
            if (WIN_DEVICE_PATH_LEN < needsize) {
                dbglog("buffer too small");
                continue;
            }
            pDetailedInterfaceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
            SetupDiGetDeviceInterfaceDetailA(DeviceInfoTable, &InterfaceData, pDetailedInterfaceData, needsize, NULL, NULL);
            YSTRNCPY(iface->devicePath, WIN_DEVICE_PATH_LEN, pDetailedInterfaceData->DevicePath, WIN_DEVICE_PATH_LEN);
            for (find = 0; find < yContext->prevEnumCnt; find++) {
                if (YSTRCMP(iface->devicePath, yContext->prevEnum[find].devicePath) == 0) break;
            }
            if (find < yContext->prevEnumCnt) {
                yMemcpy(iface->serial, yContext->prevEnum[find].serial, YOCTO_SERIAL_LEN * 2);
            } else {
                HANDLE          control;
                HALLOG("Get serial for %s\n", pDetailedInterfaceData->DevicePath);
                control = CreateFileA(pDetailedInterfaceData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
                if (control == INVALID_HANDLE_VALUE) {
                    dbglog("Unable to open device %s to get serial\n", pDetailedInterfaceData->DevicePath);
                    continue;
                }
                while (!yContext->hid.GetSerialNumberString(control, buffer, WIN_DEVICE_PATH_LEN) && --retry >= 0) {
                    dbglog("Unable to get serial for %s (%d), retrying (%d)\n", pDetailedInterfaceData->DevicePath, GetLastError(), retry);
                    Sleep(17);
                }
                if (retry < 0) {
                    dbglog("Unable to get serial for %s (%d), giving up\n", pDetailedInterfaceData->DevicePath, GetLastError());
                    CloseHandle(control);
                    continue;
                }
#if defined(_MSC_VER) && (_MSC_VER > MSC_VS2003)
                {
                    size_t          len;
                    wcstombs_s(&len, iface->serial, YOCTO_SERIAL_LEN * 2, (wchar_t*)buffer, _TRUNCATE);
                }
#else
                wcstombs(iface->serial, (wchar_t*)buffer, YOCTO_SERIAL_LEN * 2);
#endif
                CloseHandle(control);
            }
            (*nbifaceDetect)++;
            if (deviceid > YOCTO_DEVID_BOOTLOADER) {
                HALENUMLOG("----Running Dev %x:%x:%d:%s ---\n", vendorid, deviceid, ifaceno, iface->serial);
#ifdef LOG_DEVICE_PATH
                HALENUMLOG("----DevicePath %s ---\n", iface->devicePath);
#endif
            } else {
                HALENUMLOG("----Running Firm %x:%x:%d:%s ---\n", vendorid, deviceid, ifaceno, iface->serial);
            }
        } else {
            //HALLOG("Drop device vendorid=%x inst_state=%x\n", vendorid, inst_state);
        }
    }
    // unallocate Device infos
    if (!SetupDiDestroyDeviceInfoList(DeviceInfoTable)) {
        HALLOG("Unable to unallocated Device Info Table  (%d)", GetLastError());
    }
    // save enumeration result to prevent later USB packets to redetect serial
    if (yContext->prevEnum) yFree(yContext->prevEnum);
    yContext->prevEnumCnt = *nbifaceDetect;
    if (*nbifaceDetect > 0) {
        yContext->prevEnum = (yInterfaceSt*)yMalloc(*nbifaceDetect * sizeof(yInterfaceSt));
        yMemcpy(yContext->prevEnum, *ifaces, *nbifaceDetect * sizeof(yInterfaceSt));
    } else {
        yContext->prevEnum = NULL;
    }
    yLeaveCriticalSection(&yContext->prevEnum_cs);

    return YAPI_SUCCESS;
}


// return 1 if OS hdl are identicals
//        0 if any of the interface has changed
int yyyOShdlCompare(yPrivDeviceSt *dev, yInterfaceSt *newiface)
{
    if (dev->infos.nbinbterfaces != 1) {
        HALLOG("bad number of inteface for %s (%d)\n", dev->infos.serial, dev->infos.nbinbterfaces);
        return 0;
    }
    if (YSTRCMP(dev->iface.devicePath, newiface->devicePath) != 0) {
        HALLOG("devref has changed for %s (%s)\n", dev->infos.serial, DP(dev->iface.devicePath));
        return 0;
    }
    return 1;
}



/*****************************************************************
* Window implementation of yyypacket functions
*****************************************************************/





static int OpenWriteHandles(yInterfaceSt    *iface)
{
    int res;
    iface->wrHDL = INVALID_HANDLE_VALUE;
    //open blocking write handle
    iface->wrHDL = CreateFileA(iface->devicePath, GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
    if (iface->wrHDL == INVALID_HANDLE_VALUE) {
        char  errmsg[YOCTO_ERRMSG_LEN];
        res = yWinSetErr(iface, errmsg);
        HALLOG("OpenWriteHandles error of %s:%d (%s)\n", iface->serial, iface->ifaceno, errmsg);
        return res;
    }
    return YAPI_SUCCESS;
}


static void CloseWriteHandles(yInterfaceSt    *iface)
{
    if (iface->wrHDL != INVALID_HANDLE_VALUE) {
        CloseHandle(iface->wrHDL);
        iface->wrHDL = INVALID_HANDLE_VALUE;
    }
}



static int OpenReadHandles(yInterfaceSt    *iface)
{
    char  errmsg[YOCTO_ERRMSG_LEN];
    int res;
    BOOLEAN setbuffres;
    //open non blocking read handle
    iface->rdHDL = CreateFileA(iface->devicePath, GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (iface->rdHDL == INVALID_HANDLE_VALUE) {
        res = yWinSetErr(iface, errmsg);
        HALLOG("OpenReadHandles of %s error %d of %s:%d (%s)\n", DP(iface->devicePath), res, iface->serial, iface->ifaceno, errmsg);
        return res;
    }
    // since Win Xp HID buffer sizw is 32 by default and can be up to 512
    setbuffres = yContext->hid.SetNumInputBuffers(iface->rdHDL, 256);
    if (!setbuffres) {
        res = yWinSetErr(iface, errmsg);
        dbglog("SetNumInputBuffers of %s error %d of %s:%d (%s)\n", DP(iface->devicePath), res, iface->serial, iface->ifaceno, errmsg);
    }
    // create Event for non blocking read
    iface->EV[YWIN_EVENT_READ] = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (iface->EV[YWIN_EVENT_READ] == NULL) {
        res = yWinSetErr(iface, errmsg);
        HALLOG("OpenReadHandles error %d of %s:%d (%s)\n", res, iface->serial, iface->ifaceno, errmsg);
        return res;
    }
    return YAPI_SUCCESS;
}


static void CloseReadHandles(yInterfaceSt    *iface)
{
    DWORD readed;

    if (iface->rdpending && iface->rdHDL != INVALID_HANDLE_VALUE) {
        if (CancelIo(iface->rdHDL) == 0) {
            HALLOG("CancelIo failed with %d\n", GetLastError());
        } else {
            if (GetOverlappedResult(iface->rdHDL, &iface->rdOL, &readed, TRUE)) {
                //finished
                if (readed != sizeof(OS_USB_Packet)) {
                    HALLOG("invalid packet size read %d  %d\n", iface->ifaceno, readed);
                } else {
                    yPktQueuePushD2H(iface, &iface->tmpd2hpkt.pkt, NULL);
                }
                ySetEvent(&yContext->exitSleepEvent);
            } else {
                u32 error = GetLastError();
                if (error != ERROR_OPERATION_ABORTED) {
                    HALLOG("Error when stoping read IO on %s:%d\n", iface->serial, iface->ifaceno);
                }
            }
        }
    }
    if (iface->EV[YWIN_EVENT_READ] != NULL) {
        CloseHandle(iface->EV[YWIN_EVENT_READ]);
        iface->EV[YWIN_EVENT_READ] = NULL;
    }
    if (iface->rdHDL != INVALID_HANDLE_VALUE) {
        CloseHandle(iface->rdHDL);
        iface->rdHDL = INVALID_HANDLE_VALUE;
    }
    iface->rdpending = 0;
}


static int StartReadIO(yInterfaceSt *iface, char *errmsg)
{
    DWORD readed;
    u32   retrycount = 0;
retry:
    YASSERT(iface->rdpending == 0);
    memset(&iface->rdOL, 0, sizeof(iface->rdOL));
    //check if we need that : if(!SetEvent(iface->rdEV)) return yWinSetErr(errmsg);
    iface->rdOL.hEvent = iface->EV[YWIN_EVENT_READ];
    if (!ReadFile(iface->rdHDL, &iface->tmpd2hpkt, sizeof(OS_USB_Packet), &readed, &iface->rdOL)) {
        u32 error = GetLastError();
        if (error != ERROR_IO_PENDING) {
            return yWinSetErrEx(__LINE__, iface, error, "", errmsg);
        }
        iface->rdpending = 1;
    } else {
        yPktQueuePushD2H(iface, &iface->tmpd2hpkt.pkt, NULL);
        ySetEvent(&yContext->exitSleepEvent);
        // TODO: add some kind of timeout to be able to send reset packet
        // if device become crazy
        retrycount++;
        goto retry;
    }
    return YAPI_SUCCESS;
}





//Look if we have new packet arrived
static int yyyyRead(yInterfaceSt *iface, char *errmsg)
{
    DWORD           readed;
    int             res;
    int             retrycount = 1;
retry:
    if (iface->rdpending == 0) {
        // no IO started -> start a new one
        res = StartReadIO(iface, errmsg);
        if (YISERR(res)) {
            if (retrycount--) {
                CloseReadHandles(iface);
                if (YISERR(OpenReadHandles(iface))) {
                    HALLOG("Open handles restared failed %s:%d\n", iface->serial, iface->ifaceno);
                    return res;
                }
                //seep a bit to let the OS restart thing correctly
                yApproximateSleep(1);
                goto retry;
            }
            HALLOG("Read IO error %s:%d (%s/%s)\n", iface->serial, iface->ifaceno, errmsg, DP(iface->devicePath));
            return res;
        }
    }
    if (GetOverlappedResult(iface->rdHDL, &iface->rdOL, &readed, 0)) {
        iface->rdpending = 0;
        if (readed != sizeof(OS_USB_Packet)) {
            HALLOG("drop invalid packet on %s:%d (invalid size %d)\n", iface->serial, iface->ifaceno, readed);
        } else {
            yPktQueuePushD2H(iface, &iface->tmpd2hpkt.pkt, NULL);
            ySetEvent(&yContext->exitSleepEvent);
        }
        res = StartReadIO(iface, errmsg);
        if (YISERR(res)) {
            if (retrycount--) {
                CloseReadHandles(iface);
                if (YISERR(OpenReadHandles(iface))) {
                    HALLOG("Open handles restared failed %s:%d\n", iface->serial, iface->ifaceno);
                    return res;
                }
                //seep a bit to let the OS restart thing correctly
                yApproximateSleep(1);
                goto retry;
            }
            HALLOG("Read IO error %s:%d (%s/%s)\n", iface->serial, iface->ifaceno, errmsg, DP(iface->devicePath));
            return res;
        }
    } else {
        u32 error = GetLastError();
        if (error != ERROR_IO_INCOMPLETE) {
            iface->rdpending = 0;
            res = yWinSetErrEx(__LINE__, iface, error, "", errmsg);
            if (retrycount--) {
                CloseReadHandles(iface);
                if (YISERR(OpenReadHandles(iface))) {
                    HALLOG("Open handles restared failed %s:%d\n", iface->serial, iface->ifaceno);
                    return res;
                }
                //seep a bit to let the OS restart thing correctly
                yApproximateSleep(1);
                goto retry;
            }
            HALLOG("Read IO error %s:%d (%s/%s)\n", iface->serial, iface->ifaceno, errmsg, DP(iface->devicePath));
            return res;
        }
    }
    if (retrycount == 0) {
        HALLOG("Read IO needed 1 retry %s:%d (%s/%s)\n", iface->serial, iface->ifaceno, errmsg, DP(iface->devicePath));
    }
    return YAPI_SUCCESS;
}


static int yyyyWrite(yInterfaceSt *iface, pktItem *pktItem)
{
    char            errmsg[YOCTO_ERRMSG_LEN];
    DWORD           written;
    OS_USB_Packet   winpkt;
    int             retrycount = 1;

    winpkt.dummy = 0;
    memcpy(&winpkt.pkt, &pktItem->pkt, sizeof(USB_Packet));
retry:
    if (!WriteFile(iface->wrHDL, &winpkt, sizeof(OS_USB_Packet), &written, NULL)) {
        YRETCODE code = yWinSetErr(iface, errmsg);
        if (retrycount--) {
            // reset handles
            CloseWriteHandles(iface);
            if (YISERR(OpenWriteHandles(iface))) {
                HALLOG("Write handles restared failed %s:%d\n", iface->serial, iface->ifaceno);
                return code;
            }
            //seep a bit to let the OS restart thing correctly
            yApproximateSleep(1);
            goto retry;
        }
        HALLOG("Write IO error %s:%d (%s/%s)\n", iface->serial, iface->ifaceno, errmsg, DP(iface->devicePath));
        return code;
    }
    if (retrycount == 0) {
        HALLOG("Write IO needed 1 retry %s:%d (%s/%s)\n", iface->serial, iface->ifaceno, errmsg, DP(iface->devicePath));
    }
    return YAPI_SUCCESS;
}



static void* yyyUsbIoThread(void* thread_void)
{
    u32             i;
    char            errmsg[YOCTO_ERRMSG_LEN];
    DWORD           dwEvent;
    yThread         *thread = (yThread*)thread_void;
    yInterfaceSt    *iface = (yInterfaceSt*)thread->ctx;


    iface->wrHDL = INVALID_HANDLE_VALUE;
    iface->rdHDL = INVALID_HANDLE_VALUE;
    for (i = 0; i < 2; i++) {
        iface->EV[i] = NULL;
    }

    //open blocking write handle
    if (YISERR(OpenWriteHandles(iface))) {
        yThreadSignalStart(thread);
        goto exitThread;
    }
    //open blocking write handle
    if (YISERR(OpenReadHandles(iface))) {
        yThreadSignalStart(thread);
        goto exitThread;
    }
    // create Event for breaking wait
    iface->EV[YWIN_EVENT_INTERRUPT] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (iface->EV[YWIN_EVENT_INTERRUPT] == NULL) {
        yWinSetErr(iface, errmsg);
        HALLOG("IO error %s:%d (%s)\n", iface->serial, iface->ifaceno, errmsg);
        yThreadSignalStart(thread);
        goto exitThread;
    }


    HALLOG("yyyReady I%x wr=%x rd=%x se=%s\n", iface->ifaceno, iface->wrHDL, iface->rdHDL, iface->serial);
    yThreadSignalStart(thread);
    //CloseHandle(iface->EV[YWIN_EVENT_INTERRUPT]);


    if (yyyyRead(iface, errmsg) != YAPI_SUCCESS) {
        HALLOG("Read error  %s:%d (%s)\n", iface->serial, iface->ifaceno, errmsg);
        goto exitThread;
    }


    HALLOG("----IoThread start of %s:%d (%s)  ---\n", iface->serial, iface->ifaceno, DP(iface->devicePath));

    while (!yThreadMustEnd(thread)) {
        pktItem *pktItem;
        yPktQueuePeekH2D(iface, &pktItem);
        //first write pending out packet
        while (pktItem) {
#ifdef DEBUG_PKT_TIMING
            u64     timeBeforeWrite, timeAfterWrite, stop;
            YASSERT(pktItem->time);
            timeBeforeWrite = yapiGetTickCount() - pktItem->time;
            YASSERT(timeBeforeWrite >= 0 && timeBeforeWrite < 50);
#endif
            if (YISERR(yyyyWrite(iface, pktItem))) {
                goto exitThread;
            }
            yPktQueuePopH2D(iface, &pktItem);
#ifdef DEBUG_PKT_TIMING
            stop = yapiGetTickCount();
            timeAfterWrite = stop - pktItem->time;
            if (pktItem->next) {
                u64 tmp = yapiGetTickCount() - pktItem->time;
                printf("outpkt no %llu %llu -> %llu (%llu=>%llu) (next=no %llu %llu)\n", pktItem->ospktno, timeBeforeWrite, timeAfterWrite, pktItem->time, stop, pktItem->next->ospktno, tmp);
            } else {
                printf("outpkt no %llu %llu -> %llu (%llu=>%llu) (no next)\n",
                    pktItem->ospktno, timeBeforeWrite, timeAfterWrite, pktItem->time, stop);
            }
            YASSERT(timeAfterWrite >= 0 && timeAfterWrite < 50);
#endif
            yFree(pktItem);
            yPktQueuePeekH2D(iface, &pktItem);
        }

        // Wait for the thread to signal one of the event objects
        dwEvent = WaitForMultipleObjects(
            2,          // number of objects in array
            iface->EV,  // array of objects
            FALSE,      // wait for any object
            50);        // wait for max 50 ms
        if (dwEvent == WAIT_FAILED) {
            YRETCODE code = yWinSetErr(iface, errmsg);
            HALLOG("Wait error %s:%d (%s)\n", iface->serial, iface->ifaceno, errmsg);
            yPktQueueSetError(&iface->txQueue, code, errmsg);
            yPktQueueSetError(&iface->rxQueue, code, errmsg);
            yApproximateSleep(2);
            continue;
        }
        if (dwEvent != WAIT_TIMEOUT) {
            if (yyyyRead(iface, errmsg) != YAPI_SUCCESS) {
                HALLOG("Read error %s:%d (%s)\n", iface->serial, iface->ifaceno, errmsg);
                yPktQueueSetError(&iface->rxQueue, YAPI_IO_ERROR, errmsg);
                break;
            }
        }
            }

exitThread:
    HALLOG("----IoThread end of  %s:%d (%s)  ---\n", iface->serial, iface->ifaceno, DP(iface->devicePath));
    if (iface->EV[YWIN_EVENT_INTERRUPT] != NULL) {
        CloseHandle(iface->EV[YWIN_EVENT_INTERRUPT]);
        iface->EV[YWIN_EVENT_INTERRUPT] = NULL;
    }
    CloseWriteHandles(iface);
    CloseReadHandles(iface);
    yThreadSignalEnd(thread);
    return NULL;
        }


int yyySetup(yInterfaceSt *iface, char *errmsg)
{
    HALLOG("yyySetup %p\n", iface);
    yPktQueueInit(&iface->rxQueue);
    yPktQueueInit(&iface->txQueue);
    memset(&iface->io_thread, 0, sizeof(yThread));
    if (yThreadCreate(&iface->io_thread, yyyUsbIoThread, (void*)iface) < 0) {
        return YERRMSG(YAPI_IO_ERROR, "Unable to start USB IO thread");
    }
    return YAPI_SUCCESS;
}

int yyySignalOutPkt(yInterfaceSt *iface, char *errmsg)
{
    ySetEvent(&iface->EV[YWIN_EVENT_INTERRUPT]);
    return YAPI_SUCCESS;
}

void yyyPacketShutdown(yInterfaceSt *iface)
{
    HALLOG("yyyPacketShutdown %p\n", iface);
    if (yThreadIsRunning(&iface->io_thread)) {
        u64 timeout;
        yThreadRequestEnd(&iface->io_thread);
        timeout = yapiGetTickCount() + YIO_DEFAULT_USB_TIMEOUT;
        while (yThreadIsRunning(&iface->io_thread) && timeout >= yapiGetTickCount()) {
            yApproximateSleep(10);
        }
        yThreadKill(&iface->io_thread);
    }
    yPktQueueFree(&iface->rxQueue);
    yPktQueueFree(&iface->txQueue);
}

#endif


#ifdef WINCE
#include "yproto.h"


int yUSB_init(yContextSt *ctx, char *errmsg)
{
    return YAPI_SUCCESS;
}


int yUSB_stop(yContextSt *ctx, char *errmsg)
{
    return 0;
}



int yUSBGetInterfaces(yInterfaceSt **ifaces, int *nbifaceDetect, char *errmsg)
{
    *nbifaceDetect = 0;
    return 0;
}



int yyyTestOShdl(yPrivDeviceSt *dev, DevEnum *newdev)
{
    return 1;
}




int yyySetup(yInterfaceSt *iface, char *errmsg)
{
    return YERR(YAPI_NOT_SUPPORTED);
}

int yyySignalOutPkt(yInterfaceSt *iface)
{
    return YAPI_SUCCESS;
}

void yyyPacketShutdown(yInterfaceSt  *iface)
{}

#endif
