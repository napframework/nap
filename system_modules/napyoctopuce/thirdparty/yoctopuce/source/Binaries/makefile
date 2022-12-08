# *********************************************************************
#
#  $Id: makefile 29480 2017-12-21 08:16:02Z seb $
#
#  makefile for C++ library
#  - - - - - - - - - License information: - - - - - - - - -
#
#  Copyright (C) 2011 and beyond by Yoctopuce Sarl, Switzerland.
#
#  1) If you have obtained this file from www.yoctopuce.com using
#     a valid customer account established in your proper name,
#     Yoctopuce Sarl (hereafter Licensor) licenses to you (hereafter
#     Licensee) the right to use, modify, copy, and integrate this
#     source file into your own solution for the sole purpose of
#     interfacing a Yoctopuce product integrated into Licensee's
#     solution.
#
#     You should refer to the license agreement accompanying this
#     Software for additional information regarding your rights
#     and obligations.
#
#     THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
#     WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
#     WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS
#     FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO
#     EVENT SHALL LICENSOR BE LIABLE FOR ANY INCIDENTAL, SPECIAL,
#     INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
#     COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR
#     SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT
#     LIMITED TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR
#     CONTRIBUTION, OR OTHER SIMILAR COSTS, WHETHER ASSERTED ON THE
#     BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE), BREACH OF
#     WARRANTY, OR OTHERWISE.
#
#  2) If you have obtained this file from any other source, you
#     are not entitled to use it, read it or create any derived
#     material. You should delete this file immediately.
#
# ********************************************************************
.SILENT:

WINDOWS_SDK_32 = $(WINDOWSSDKDIR)

LINKWITH32 =  /LIBPATH:"$(VCINSTALLDIR)lib" /LIBPATH:"$(VCINSTALLDIR)atlmfc\lib" /LIBPATH:"$(WINDOWS_SDK_32)lib"

!IF !EXISTS(c:\Program Files\Microsoft SDKs\Windows\v7.1\)
WINDOWS_SDK_64 = $(WINDOWSSDKDIR)
CC64 = "$(VCINSTALLDIR)bin\x86_amd64\cl"
LINKER64 = "$(VCINSTALLDIR)bin\x86_amd64\link"
LINKWITH64 = /MACHINE:X64 /LIBPATH:"$(VCINSTALLDIR)lib\amd64" /LIBPATH:"$(VCINSTALLDIR)atlmfc\lib\amd64" /LIBPATH:"$(WINDOWS_SDK_64)Lib\$(WINDOWSSDKLIBVERSION)ucrt\x64" /LIBPATH:"$(WINDOWS_SDK_64)\Lib\$(WINDOWSSDKLIBVERSION)um\x64"
!ELSE
WINDOWS_SDK_64 = c:\Program Files\Microsoft SDKs\Windows\v7.1
CC64 = "$(VCINSTALLDIR)bin\amd64\cl"
LINKWITH64 =  /LIBPATH:"$(VCINSTALLDIR)lib\amd64" /LIBPATH:"$(VCINSTALLDIR)atlmfc\lib\amd64" /LIBPATH:"$(WINDOWS_SDK_64)\lib\x64"
!ENDIF


!IFDEF JAVA_HOME
JNI_OPTION = /D YAPI_WITH_JNI /I"$(JAVA_HOME)\include" /I"$(JAVA_HOME)\include\win32"
!ELSE
JNI_OPTION =
!ENDIF

OUTDIR32 = windows

OUTDIR64 = $(OUTDIR32)\amd64

OBJDIR32 = $(OUTDIR32)\obj_dir

OBJDIR_DLL32 = $(OUTDIR32)\obj_dir_dll

OBJDIR64 = $(OUTDIR64)\obj_dir

OBJDIR_DLL64 = $(OUTDIR64)\obj_dir_dll

OD = arch_type

YOCTO_API_DIR = ..\Sources

YOCTO_API_OBJS =  $(OD)\yocto_api.obj $(OD)\yocto_anbutton.obj $(OD)\yocto_carbondioxide.obj $(OD)\yocto_colorled.obj $(OD)\yocto_current.obj \
	$(OD)\yocto_datalogger.obj $(OD)\yocto_display.obj $(OD)\yocto_dualpower.obj $(OD)\yocto_files.obj $(OD)\yocto_genericsensor.obj \
	$(OD)\yocto_hubport.obj $(OD)\yocto_humidity.obj $(OD)\yocto_led.obj $(OD)\yocto_lightsensor.obj $(OD)\yocto_network.obj \
	$(OD)\yocto_oscontrol.obj $(OD)\yocto_power.obj $(OD)\yocto_pressure.obj $(OD)\yocto_realtimeclock.obj $(OD)\yocto_relay.obj \
	$(OD)\yocto_servo.obj $(OD)\yocto_temperature.obj $(OD)\yocto_voc.obj $(OD)\yocto_voltage.obj \
	$(OD)\yocto_wakeupmonitor.obj $(OD)\yocto_wakeupschedule.obj $(OD)\yocto_watchdog.obj $(OD)\yocto_wireless.obj $(OD)\yocto_digitalio.obj \
	$(OD)\yocto_accelerometer.obj $(OD)\yocto_magnetometer.obj $(OD)\yocto_refFrame.obj $(OD)\yocto_tilt.obj $(OD)\yocto_compass.obj \
	$(OD)\yocto_gyro.obj $(OD)\yocto_pwmoutput.obj $(OD)\yocto_pwmpowersource.obj $(OD)\yocto_altitude.obj $(OD)\yocto_motor.obj \
	$(OD)\yocto_serialport.obj $(OD)\yocto_pwminput.obj $(OD)\yocto_segmenteddisplay.obj $(OD)\yocto_buzzer.obj $(OD)\yocto_gps.obj \
	$(OD)\yocto_cellular.obj $(OD)\yocto_groundspeed.obj $(OD)\yocto_latitude.obj $(OD)\yocto_longitude.obj  $(OD)\yocto_poweroutput.obj \
	$(OD)\yocto_quadraturedecoder.obj $(OD)\yocto_currentloopoutput.obj $(OD)\yocto_colorledcluster.obj $(OD)\yocto_spiport.obj \
	$(OD)\yocto_steppermotor.obj $(OD)\yocto_multiaxiscontroller.obj $(OD)\yocto_proximity.obj $(OD)\yocto_rangefinder.obj \
	$(OD)\yocto_daisychain.obj $(OD)\yocto_weighscale.obj $(OD)\yocto_voltageoutput.obj $(OD)\yocto_powersupply.obj $(OD)\yocto_multicellweighscale.obj

YOCTO_API_OBJS32 = $(YOCTO_API_OBJS:arch_type=windows\obj_dir)

YOCTO_API_OBJS64 = $(YOCTO_API_OBJS:arch_type=windows\amd64\obj_dir)

YOCTO_API_OBJS_DLL32 = $(YOCTO_API_OBJS:arch_type=windows\obj_dir_dll)

YOCTO_API_OBJS_DLL64 = $(YOCTO_API_OBJS:arch_type=windows\amd64\obj_dir_dll)

YAPI_DIR = $(YOCTO_API_DIR)\yapi

YAPI_OBJS = $(OD)\yapi.obj $(OD)\ystream.obj $(OD)\yprog.obj  $(OD)\yjni.obj $(OD)\ypkt_lin.obj $(OD)\ypkt_win.obj $(OD)\ypkt_osx.obj $(OD)\yfifo.obj $(OD)\ykey.obj $(OD)\yhash.obj $(OD)\yjson.obj $(OD)\ytcp.obj $(OD)\ymemory.obj $(OD)\ythread.obj

YAPI_OBJS32 = $(YAPI_OBJS:arch_type=windows\obj_dir)

YAPI_OBJS64 = $(YAPI_OBJS:arch_type=windows\amd64\obj_dir)

YAPI_OBJS_DLL32 = $(YAPI_OBJS:arch_type=windows\obj_dir_dll)

YAPI_OBJS_DLL64 = $(YAPI_OBJS:arch_type=windows\amd64\obj_dir_dll)

ALL_OBJS32		= $(YAPI_OBJS32) $(YOCTO_API_OBJS32)

ALL_OBJS_DLL32	= $(YAPI_OBJS_DLL32) $(YOCTO_API_OBJS_DLL32)

ALL_OBJS64		= $(YAPI_OBJS64) $(YOCTO_API_OBJS64)

ALL_OBJS_DLL64	= $(YAPI_OBJS_DLL64) $(YOCTO_API_OBJS_DLL64)

CC32 = "$(VCINSTALLDIR)\bin\cl"

{$(YAPI_DIR)\}.c{$(OBJDIR32)\}.obj:
	$(CC32) $(CFLAGS) /nologo /Zl /Fo$@ /c $<

{$(YOCTO_API_DIR)\}.cpp{$(OBJDIR32)\}.obj:
	$(CC32) $(CPPFLAGS) /nologo /EHsc /Zl /Fo$@ /c $<

{$(YAPI_DIR)\}.c{$(OBJDIR_DLL32)\}.obj:
	$(CC32) $(CFLAGS) /D GENERATE_DLL $(JNI_OPTION) /W3 /WX- /O2 /Oi /Oy- /Gy /GL /MT  /nologo /Fo$@ /c $<

{$(YOCTO_API_DIR)\}.cpp{$(OBJDIR_DLL32)\}.obj:
	$(CC32) $(CPPFLAGS) /D GENERATE_DLL /nologo /EHsc  /Fo$@ /c $<

{$(YAPI_DIR)\}.c{$(OBJDIR64)\}.obj:
	$(CC64) $(CFLAGS) /nologo /Zl /Fo$@ /c $<

{$(YOCTO_API_DIR)\}.cpp{$(OBJDIR64)\}.obj:
	$(CC64) $(CPPFLAGS) /nologo /EHsc /Zl /Fo$@ /c $<

{$(YAPI_DIR)\}.c{$(OBJDIR_DLL64)\}.obj:
	$(CC64) $(CFLAGS) /D GENERATE_DLL $(JNI_OPTION) /W3 /WX- /O2 /Oi /Oy- /Gy /GL /MT  /nologo /Fo$@ /c $<

{$(YOCTO_API_DIR)\}.cpp{$(OBJDIR_DLL64)\}.obj:
	$(CC64) $(CPPFLAGS) /D GENERATE_DLL /nologo /EHsc  /Fo$@ /c $<

{$(YAPI_DIR)\}.rc{$(YAPI_DIR)\}.res:
	$(RC) $(RFLAGS) $<

{$(YOCTO_API_DIR)\}.rc{$(YOCTO_API_DIR)\}.res:
	$(RC) $(RFLAGS) $<

YAPIOUT32 = $(OUTDIR32)\yapi

YAPIOUT64 = $(OUTDIR32)\yapi\amd64\

TARGET = $(OUTDIR32)\yocto-static.lib $(OUTDIR32)\yocto.dll $(YAPIOUT32)\yapi-static.lib $(YAPIOUT32)\yapi.dll

b32:   $(OBJDIR32) $(OBJDIR_DLL32) $(YAPIOUT32) $(OUTDIR32)\yocto-static.lib $(OUTDIR32)\yocto.dll $(YAPIOUT32)\yapi-static.lib $(YAPIOUT32)\yapi.dll

b64:   $(OBJDIR64) $(OBJDIR_DLL64) $(YAPIOUT64) $(OUTDIR64)\yocto-static.lib $(OUTDIR64)\yocto.dll $(YAPIOUT64)\yapi-static.lib $(YAPIOUT64)\yapi.dll

all:  b32 b64

yapi:  $(OBJDIR32) $(OBJDIR_DLL32) $(YAPIOUT32) $(YAPIOUT32)\yapi.dll $(OBJDIR64) $(OBJDIR_DLL64) $(YAPIOUT64) $(YAPIOUT64)\yapi.dll

release :  all clean_obj

$(OBJDIR32) $(OBJDIR_DLL32) $(YAPIOUT32) $(OBJDIR64) $(OBJDIR_DLL64) $(YAPIOUT64):
	IF NOT EXIST $@ mkdir $@

$(OUTDIR32)\yocto-static.lib: $(ALL_OBJS32)
	@echo building lib ($@)
    @lib /OUT:$@ $**

$(OUTDIR32)\yocto.dll: $(ALL_OBJS_DLL32) $(YOCTO_API_DIR)\winresource.res
	@echo building dll $@
	$(CC32) $(CFLAGS) $** OleAut32.lib /nologo /Fe$@ /Fa(R) /link /DLL

$(YAPIOUT32)\yapi-static.lib: $(YAPI_OBJS32)
	@echo building lib ($@)
    @lib /OUT:$@ $**

$(YAPIOUT32)\yapi.dll: $(YAPI_OBJS_DLL32) $(YAPI_DIR)\yapiresource.res
	@echo building dll $@
	@$(CC32) $(CFLAGS) $** /W3 /WX- /O2 /Oi /Oy- /MT /Gy /GL /nologo /Fe$@ /Fa(R) /link /DLL /DEF:$(YAPI_DIR)\yapi.def

$(OUTDIR64)\yocto-static.lib: $(ALL_OBJS64)
	@echo building lib ($@)
    @lib /OUT:$@ $**

$(OUTDIR64)\yocto.dll: $(ALL_OBJS_DLL64) $(YOCTO_API_DIR)\winresource64.res
	@echo building dll $@
	@$(CC64) $(CFLAGS) $** /nologo /MD /Fe$@ /Fa(R) /link  $(LINKWITH64)  /DLL

$(YAPIOUT64)\yapi-static.lib: $(YAPI_OBJS64)
	@echo building lib ($@)
    @lib /OUT:$@ $**

$(YAPIOUT64)\yapi.dll: $(YAPI_OBJS_DLL64) $(YAPI_DIR)\yapiresource64.res
	@echo building dll $@
	@$(CC64) $(CFLAGS) $**  /W3 /WX- /O2 /Oi /Oy- /MD /Gy /GL /nologo /Fe$@ /Fa(R) /link $(LINKWITH64) /DLL

$(ALL_OBJS32) $(ALL_OBJS_DLL32) $(YAPI_OBJS_DLL32) $(YAPI_OBJS_DLL32): $(YAPI_DIR)\*.h  $(YOCTO_API_DIR)\*.h

$(ALL_OBJS64) $(ALL_OBJS_DLL64) $(YAPI_OBJS_DLL64) $(YAPI_OBJS_DLL64): $(YAPI_DIR)\*.h  $(YOCTO_API_DIR)\*.h

clean: clean32 clean64

clean_obj: clean_obj32 clean_obj64

clean32: clean_obj32
	-del /Q /F $(OUTDIR32)  $(YAPIOUT32)

clean_obj32:
	-del /Q /F $(OBJDIR32)
	-del /Q /F $(OBJDIR_DLL32)
	-del /Q /F $(YAPI_DIR)\yapiresource.res
	-del /Q /F $(YOCTO_API_DIR)\winresource.res
	-rmdir $(OBJDIR32)
	-rmdir $(OBJDIR_DLL32)

clean64: clean_obj64
	-del /Q /F $(OUTDIR64)  $(YAPIOUT64)

clean_obj64:
	-del /Q /F $(OBJDIR64)
	-del /Q /F $(OBJDIR_DLL64)
	-del /Q /F $(YAPI_DIR)\yapiresource64.res
	-del /Q /F $(YOCTO_API_DIR)\winresource64.res
	-rmdir $(OBJDIR64)
	-rmdir $(OBJDIR_DLL64)
